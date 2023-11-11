# Pioneer Abstract Layer
The PAL Project is an embedded SDK for the Pioneer Board, which use STM32F103RCT6 as microcontroller.
Because the Dependence Library of this project is the HAL(Hardware Abstract Layer) library published by STMicroelectronics.
This project is built with C language, and we will upgrade this project to C++ in few months, but the C Version will also be alive in a branch(not major).

Although the PAL is developed for Pioneer with STM32F103RCT6(also we got lots of versions of the hardware),
 you can still use this lib in other platforms, like other boards with RCT6, other STM32F1xx chips, even F4 or F7,H7 etc.
By the way, if you want to use PAL in other platform, please check whether it fits your project.

## How to install and use
Because many function in this lib depends on the MACRO defined in certain head files or your private source files, we do not publish static lib version.
You need to compile your project with these codes(of course the part you need) so that you can get the .hex .bin .elf binary executable file and download to your mcu.

### Step 1. Generate codes with HAL firmware
If you want to use PAL, you must use HAL at first. Because in many head files we use
```c
#include "stm32f1xx_hal.h"
```
or sth. in HAL firmware, you need to ensure when you compile PAL source codes the compiler could find these HAL things.
For example, if you generate codes with STM32CubeMX (recommend), the cubeMX will add these source codes and head files in CMakeList.txt or sth. accepted by your IDE.

### Step 2. Load PAL Sources and Headers
After the code generation, your IDE and build tool may not recognize PAL codes so that you'll fail during build process. 
Before you add our codes to your CMakeList.txt, it may be written by cubeMX as:
```cmake
include_directories(Inc Drivers/STM32F1xx_HAL_Driver/Inc Drivers/STM32F1xx_HAL_Driver/Inc/Legacy Middlewares/ST/STM32_USB_Device_Library/Core/Inc Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc Drivers/CMSIS/Device/ST/STM32F1xx/Include Drivers/CMSIS/Include)
#...
file(GLOB_RECURSE SOURCES "Startup/*.*" "Src/*.*" "Middlewares/*.*" "Drivers/*.*")
```
You need to add PAL/xx.h to include_directories() and PAL/xx/xx.c(cpp) to file() so that the cmake will recognize our codes, like this:
```cmake
include_directories(PAL Inc Drivers/STM32F1xx_HAL_Driver/Inc Drivers/STM32F1xx_HAL_Driver/Inc/Legacy Middlewares/ST/STM32_USB_Device_Library/Core/Inc Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc Drivers/CMSIS/Device/ST/STM32F1xx/Include Drivers/CMSIS/Include)

file(GLOB_RECURSE SOURCES "PAL/*/*.*" "Startup/*.*" "Src/*.*" "Middlewares/*.*" "Drivers/*.*")
```
### Step 3. Build your project and enjoy

## Functions in Directories and files
|   Directory   |                   Function Category                    | Contents |
|:-------------:|:------------------------------------------------------:|:--------:|
|    utils/     |                      public tools                      |    1     |
| connectivity/ |       enterprise connectivity of many protocols        |    1     |
|   display/    | drivers of screens or other things which can show sth. |    2     |
|   storage/    |              drivers of flash or sd-card               |    1     |
|  extension/   |     drivers of chip can extend pins and functions      |    1     |

### Things Built( Main Function)

- UART Convenient: Improve Hardware UART functions (no DMA)
- W25QXX: Driver for spi-flash chip (W25QXX by Winbond)
- PCA9685: Driver for i2c-pwm chip (PCA9685 by NXP) 
- SSD1306: SSD1306 OLED driver (need modify)