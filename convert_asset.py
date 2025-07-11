#!/usr/bin/env python3
import sys
import os
import time

CHAR_WIDTH = 5
CHAR_HEIGHT = 11
CHAR_SPACING = 2
NUMBER_OF_CHARS = 62

def read_bmp_file(filename):
    with open(filename, 'rb') as f:
        return f.read()

def write_output_file(filename, content):
    with open(filename, 'w') as f:
        f.write(content)

def parse_bmp_header(data):
    return {
        "width": int.from_bytes(data[18:22], 'little'),
        "height": int.from_bytes(data[22:26], 'little'),
        "bit_depth": int.from_bytes(data[28:30], 'little'),
        "pixel_data_offset": int.from_bytes(data[10:14], 'little'),
    }

def get_palette(data, size=1024):
    return data[54:54+size]

def get_palette_color(palette, index):
    base = index * 4
    return palette[base:base+3]  # BGR

def extract_pixel(data, bit_depth, palette, offset):
    if bit_depth == 24:
        pixel = data[offset:offset+3]
        return pixel == b'\x00\x00\x00'
    elif bit_depth == 8:
        pixel_index = data[offset]
        color = get_palette_color(palette, pixel_index)
        return color == b'\x00\x00\x00'
    else:
        return False

def convert(input_path, output_path):
    name = os.path.basename(input_path).split('.')[0]
    bmp_data = read_bmp_file(input_path)

    header = parse_bmp_header(bmp_data)
    width, height = header["width"], header["height"]
    bit_depth = header["bit_depth"]
    pixel_data_offset = header["pixel_data_offset"]

    palette = get_palette(bmp_data) if bit_depth == 8 else None
    row_size = {
        24: ((width * 3 + 3) & ~3),
        8:  ((width + 3) & ~3),
    }.get(bit_depth)

    if row_size is None:
        print(f"Unsupported bit depth: {bit_depth}")
        sys.exit(1)

    output_lines = []
    output_lines.append(f'#include "gigagl.h"')
    output_lines.append(f'// Converted from {input_path}')
    output_lines.append(f'const uint8_t __{name}_data[] = {{')

    for y in range(height):
        line = "\t"
        for x in range(0, width, 8):
            byte = 0
            for i in range(8):
                if x + i >= width:
                    bit = 1  # padding bit as white
                else:
                    offset = pixel_data_offset + (height - 1 - y) * row_size + (x + i) * (3 if bit_depth == 24 else 1)
                    bit = 0 if extract_pixel(bmp_data, bit_depth, palette, offset) else 1
                byte = (byte << 1) | bit
            line += f"0b{byte:08b}, "
        output_lines.append(line)
    output_lines.append("};\n")

    output_lines.append(f"""const ggl_icon_t {name}_icon = {{
    .width = {width},
    .height = {height},
    .data = __{name}_data,
}};""")

    write_output_file(output_path, '\n'.join(output_lines))
    print(f"Conversion complete: {output_path}")

def convert_font(input_path, output_path):
    bmp_data = read_bmp_file(input_path)
    header = parse_bmp_header(bmp_data)
    width, height = header["width"], header["height"]
    bit_depth = header["bit_depth"]
    pixel_data_offset = header["pixel_data_offset"]

    palette = get_palette(bmp_data) if bit_depth == 8 else None
    row_size = {
        24: ((width * 3 + 3) & ~3),
        8:  ((width + 3) & ~3),
    }.get(bit_depth)

    if row_size is None:
        print(f"Unsupported bit depth: {bit_depth}")
        sys.exit(1)

    output_lines = [f'#include "gigagl.h"', f'// Font converted from {input_path}']
    output_lines.append(f"const uint8_t font_data[{NUMBER_OF_CHARS}][{CHAR_HEIGHT}] = "+"{")

    rows = [
        "ABCDEFGHIJKLMNO",
        "PQRSTUVWXYZ",
        "abcdefghijklmno",
        "pqrstuvwxyz",
        "0123456789",
    ]

    for row_index, row_chars in enumerate(rows):
        for i, char in enumerate(row_chars):
            output_lines.append(f"\t// '{char}'")
            output_lines.append("\t{")
            for y in range(CHAR_HEIGHT):
                bits = 0
                for x in range(CHAR_WIDTH):
                    px = i * (CHAR_WIDTH + CHAR_SPACING) + x
                    py = row_index * CHAR_HEIGHT + y
                    offset = pixel_data_offset + (height - 1 - py) * row_size + px * (3 if bit_depth == 24 else 1)
                    bit = 0 if extract_pixel(bmp_data, bit_depth, palette, offset) else 1
                    bits = (bits << 1) | bit
                # Pad bits to left (MSB)
                padded_byte = bits << (8 - CHAR_WIDTH)
                output_lines.append(f"\t\t0b{padded_byte:08b},")
            output_lines.append("\t},")

    output_lines.append("};")
    write_output_file(output_path, '\n'.join(output_lines))
    print(f"Font conversion complete: {output_path}")


if __name__ == "__main__":

    assets_src_dir = 'assets_src'

    src_output_dir = os.path.join("src", "assets")
    os.makedirs(src_output_dir, exist_ok=True)

    this_py = os.path.abspath(__file__)

    for filename in os.listdir(assets_src_dir):
        if not filename.endswith(".bmp"):
            continue
        input_path = os.path.join(assets_src_dir, filename)
        asset_name = filename[:-4]
        src_output_path = os.path.join(src_output_dir, f"{asset_name}.c")

        input_mtime = os.path.getmtime(input_path)
        py_mtime = os.path.getmtime(this_py)
        rebuild = True
        if os.path.exists(src_output_path):
            out_mtime = os.path.getmtime(src_output_path)
            if out_mtime >= max(input_mtime, py_mtime):
                rebuild = False

        if rebuild:
            if filename == "font.bmp":
                convert_font(input_path, src_output_path)
            else:
                convert(input_path, src_output_path)
        else:
            print(f"Skipping {src_output_path}, up to date.")

    # Generate assets.h
    assets_include_path = os.path.join("include", "assets.h")
    asset_decls = ['#ifndef ASSETS_H', '#define ASSETS_H', '#include "gigagl.h"', '']

    for filename in os.listdir(assets_src_dir):
        if filename.endswith(".bmp"):
            asset_name = filename[:-4]
            if filename == "font.bmp":
                asset_decls.append(f'extern const uint8_t font_data[{NUMBER_OF_CHARS}][{CHAR_HEIGHT}];')
            else:
                asset_decls.append(f'extern const ggl_icon_t {asset_name}_icon;')

    asset_decls.append('\n#endif // ASSETS_H')
    
    with open(assets_include_path, "w") as f:
        f.write('\n'.join(asset_decls) + '\n')
