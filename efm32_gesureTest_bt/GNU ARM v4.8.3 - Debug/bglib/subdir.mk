################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../bglib/bglib_txrx.c 

OBJS += \
./bglib/bglib_txrx.o 

C_DEPS += \
./bglib/bglib_txrx.d 


# Each subdirectory must supply rules for building sources it contributes
bglib/bglib_txrx.o: ../bglib/bglib_txrx.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -mcpu=cortex-m0plus -mthumb '-DEFM32HG322F64=1' -I"D:/Project/4.Smartkey_Wearable/Development/9axis_sensors_BT/inc" -I"D:/Project/4.Smartkey_Wearable/Development/9axis_sensors_BT/bglib" -I"C:/SiliconLabs/SimplicityStudio/v3/developer/sdks/efm32/v2/kits/SLSTK3400A_EFM32HG/config" -I"C:/SiliconLabs/SimplicityStudio/v3/developer/sdks/efm32/v2/CMSIS/Include" -I"C:/SiliconLabs/SimplicityStudio/v3/developer/sdks/efm32/v2/kits/common/bsp" -I"C:/SiliconLabs/SimplicityStudio/v3/developer/sdks/efm32/v2/emlib/inc" -I"C:/SiliconLabs/SimplicityStudio/v3/developer/sdks/efm32/v2/Device/SiliconLabs/EFM32HG/Include" -I"C:/SiliconLabs/SimplicityStudio/v3/developer/sdks/efm32/v2/kits/common/drivers" -O0 -Wall -c -fmessage-length=0 -mno-sched-prolog -fno-builtin -ffunction-sections -fdata-sections -std=c99 -MMD -MP -MF"bglib/bglib_txrx.d" -MT"bglib/bglib_txrx.o" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


