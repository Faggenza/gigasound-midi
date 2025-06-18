#!/usr/bin/env python3
import sys

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

def convert(input_path, output_path):
    name = input_path.split('/')[-1].split('.')[0]
    bmp_data = read_bmp_file(input_path)

    header = parse_bmp_header(bmp_data)
    width, height = header["width"], header["height"]
    bit_depth = header["bit_depth"]
    pixel_data_offset = header["pixel_data_offset"]

    if width % 8 != 0:
        print(f"Error: Width of file {input_path},  must be a multiple of 8 for 1bpp conversion.")
        sys.exit(1)

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
            byte_bits = []
            for i in range(8):
                if bit_depth == 24:
                    offset = pixel_data_offset + (height - 1 - y) * row_size + (x + i) * 3
                    pixel = bmp_data[offset:offset+3]
                    is_black = pixel == b'\x00\x00\x00'
                elif bit_depth == 8:
                    offset = pixel_data_offset + (height - 1 - y) * row_size + (x + i)
                    pixel_index = bmp_data[offset]
                    color = get_palette_color(palette, pixel_index)
                    is_black = color == b'\x00\x00\x00'
                byte_bits.append('0' if is_black else '1')
            byte_val = ''.join(byte_bits)
            line += f"0b{byte_val}, "
        output_lines.append(line)
    output_lines.append("};\n")

    # if height == 16 and width == 16:
    output_lines.append(f"""const ggl_icon_t {name}_icon = {{
    .stride = {width // 8},
    .height = {height},
    .data = __{name}_data,
}};""")


    write_output_file(output_path, '\n'.join(output_lines))
    print(f"Conversion complete: {output_path}")

if __name__ == "__main__":
    # Create the include/assets/ directory if it doesn't exist
    import os
    assets_src_dir = 'assets_src'
    assets_include_dir = 'include/assets'
    os.makedirs(assets_include_dir, exist_ok=True)

    # Iterate over all the files in assets_src/ and convert them into include/assets/
    for filename in os.listdir(assets_src_dir):
        input_path = os.path.join(assets_src_dir, filename)
        output_path = os.path.join(assets_include_dir, f"{filename[:-4]}.h")
        convert(input_path, output_path)