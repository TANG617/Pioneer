# Bootloader for STM32F103 (RCT6)

## Overview
While STMicroelectronics mentions the USB-DFU (Device Firmware Upgrade) protocol in [their documentation](https://www.st.com/resource/en/application_note/an3156-usb-dfu-protocol-used-in-the-stm32-bootloader-stmicroelectronics.pdf), they do not provide an official bootloader supporting USB-DFU for the STM32F103 series. This custom bootloader is designed to fill that gap, providing USB-DFU capabilities specifically tailored for the STM32F103 (RCT6) microcontroller and compatible with STM32CubeMX Programmer.

It's important to note that this bootloader is not compatible with the Arduino platform; it is intended for use with the CubeMX environment. The USB-DFU mode must be activated within CubeMX before use.

Although this bootloader is specifically for the STM32F103RCT6 model, it should theoretically work with other chips in the STM32F103 series. Users are advised to replace the `.ioc` file and regenerate the project using CubeMX before proceeding with the build and flash process.

## Instructions

1. **Compilation**:
   Use the `Make` command to compile the project. Ensure that the `arm-gcc` compiler is correctly installed and configured on your system.

2. **Boot Process**:
  - During the boot process of the STM32F103, the bootloader checks the state of a user-defined button (connected to GPIO_B_1).
  - If the button is not pressed, the bootloader will initiate the user application, starting from memory address `0x08006000`.
  - If the button is pressed, the device enters USB-DFU mode, indicated by the illumination of an LED (also connected to GPIO_B_1).

3. **Flashing the Application**:
  - Ensure that the starting address for the application code is set to `0x08006000`.
  - When flashing the application, select this start address in tools like CubeMX Programmer, or adjust the `xxx_FLASH.ld` linker script in the auto-generated project. Below is an example modification for the linker script:
    ```bash
    /* Specify the memory areas */
    MEMORY
    {
    RAM (xrw)      : ORIGIN = 0x20000000, LENGTH = 20K
    FLASH (rx)     : ORIGIN = 0x08006000, LENGTH = 122K
    } 
    ```
  - Note the adjustment in the `FLASH` section's `ORIGIN` and `LENGTH` attributes to accommodate the bootloader.

4. **Integration with Development Environments**:
  - For integration with CLion or similar IDEs, add a shell script to the project settings that utilizes `STM32_Programmer_CLI` for flashing the compiled binary to the correct address.
  - An example shell script command might look like this:
    ```bash
    STM32_Programmer_CLI -c port=USB1 -w cmake-build-debug/$PROJECT_NAME.bin 0x08006000
    ```
  - Configure this script to run post-build for seamless development and testing cycles.

---

This branch contains usage of the following peripheral:

Perpheral| Status |
--|--------|
STM32 Basic| ✅
GPIO| ✅
UART| ✅
SPI Screen| ⭕️
IIC PCA9685|⭕️
IIC MPU6050|⭕️
CAN|⭕️
SDIO SD card | ⭕️
SPI Flash | ⭕️

✅Done,  ⭕️Not In this Branch


This branch contains usage of the following application:

Apps| Status |
--|--------|
FreeRTOS| ⭕️
LVGL | ⭕️
Pioneer Car | ⭕️
SPI PS2 Controller | ⭕️
UART DualSense Controller| ⭕️
FAT_FS| ⭕️



✅Done,  ⭕️Not In this Branch





