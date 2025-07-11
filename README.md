# Gigasound MIDI Polyphonic Keyboard

A USB-connected MPE MIDI keyboard using piezoresistive sensors for expressive, pressure-sensitive input, built around the STM32F401 microcontroller.

<img width="2439" height="2252" alt="Case" src="https://github.com/user-attachments/assets/f3a12c76-3afd-41f1-8cbc-73c5987d0764" />

## Source Tree

```
├── sprite/     # Assets in aseprite format
├── assets_src/ # Assets in bitmap format 
├── convert_asset.py # Asset convertion tool
├── emulator/
│   ├── emulator.h
│   └── Makefile
├── gigasound-hw/ # PCB KiCad files
├── include/
│   ├── adc.h
│   ├── assets.h
│   ├── bootloader.h
│   ├── calibrate.h
│   ├── config.h
│   ├── eeprom.h
│   ├── init.h
│   ├── input.h
│   ├── led.h
│   ├── main.h
│   ├── midi.h
│   ├── scale.h
│   ├── ssd1306.h
│   ├── stm32f4xx_it.h
│   ├── tusb_config.h
│   ├── uart.h
│   └── ui/
├── lib
│   └── gigagl # embedded graphics library
├── platformio.ini 
└── src/
    ├── adc.c
    ├── assets
    ├── calibrate.c
    ├── config.c
    ├── eeprom.c
    ├── init.c
    ├── intput.c
    ├── led.c
    ├── main.c
    ├── midi.c
    ├── scale.c
    ├── stm32f4xx_hal_msp.c
    ├── stm32f4xx_it.c
    ├── uart.c
    ├── ui/               
    └── usb_descriptors.c
```


## References

MCU: [STM32F0401RE](https://www.st.com/resource/en/datasheet/stm32f401re.pdf)
Nucleo: [Schematic](https://www.st.com/resource/en/schematic_pack/mb1136-default-c04_schematic.pdf)
Piezoresistive sensor: [datasheet](https://cdn2.hubspot.net/hubfs/3899023/Interlinkelectronics%20November2017/Docs/Datasheet_FSR.pdf)
MIDI: [docs](file:///home/nik/Downloads/M1-100-UM_v1-1_MIDI_Polyphonic_Expression_Specification.pdf)

## Testing

[MPE Monitor](https://studiocode.dev/mpe-monitor/)