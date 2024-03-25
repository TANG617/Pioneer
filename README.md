# Bootloader For STM32F103(RCT6)

## Overview
ST doesn't offer official supported USB-DFU bootloader, though they mentioned it in [the documents](https://www.st.com/resource/en/application_note/an3156-usb-dfu-protocol-used-in-the-stm32-bootloader-stmicroelectronics.pdf).

This bootloader has NO compatibility of Arduino but only CubeMx Programmer.

The USB-DFU should be enabled in CubeMX.

Other chip in STM32F103 series should work in theory. Just replace `ioc` file and re-gengrate the project before build and flash.

## Instruction

- Use `Make` to compile the project with `arm-gcc` compiler.
- When STM32 boots, if there is no button(GPIO_B_1) pushed, the app code(start at address `0x08006000`) starts. Otherwise, USB-DFU is triggered to start with LED(GPIO_B_1) lit.
- Make sure to flash the app code start to `0x08006000`. Choose the address when using tools like CubeMX Prog or modify the `xxx_FLASH.ld` in the generated project.
    ```bash
    /* Specify the memory areas */
    MEMORY
    {
    RAM (xrw)      : ORIGIN = 0x20000000, LENGTH = 20K
    FLASH (rx)      : ORIGIN = 0x8000000, LENGTH = 128K --> ORIGIN = 0x8006000, LENGTH = 128K
    } 
    ```
- Add `Shell Scripts` of `STM32_Programmer_CLI -c port=usb1 -w cmake-build-debug/$PROJECT_NAME.bin 0x08006000` to CLion and add the script after `build` process.


