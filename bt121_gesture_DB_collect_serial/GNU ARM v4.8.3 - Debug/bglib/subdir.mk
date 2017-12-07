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
	arm-none-eabi-gcc -g -gdwarf-2 -mcpu=cortex-m0plus -mthumb -std=c99 '-DDEBUG=1' '-DEFM32HG322F64=1' -I"C:\work\EFM32\v4\gesture_bt121_forDB\inc" -I"C:/work/EFM32/v4/gesture_bt121_forDB/inc" -I"C:/SiliconLabs/SimplicityStudio/v4/developer/sdks/exx32/v4.4.1//kits/common/bsp" -I"C:/SiliconLabs/SimplicityStudio/v4/developer/sdks/exx32/v4.4.1//Device/SiliconLabs/EFM32HG/Include" -I"C:/SiliconLabs/SimplicityStudio/v4/developer/sdks/exx32/v4.4.1//emlib/inc" -I"C:/SiliconLabs/SimplicityStudio/v4/developer/sdks/exx32/v4.4.1//kits/common/drivers" -I"C:/SiliconLabs/SimplicityStudio/v4/developer/sdks/exx32/v4.4.1//CMSIS/Include" -I"C:\work\EFM32\v4\gesture_bt121 (2)\bglib" -I"C:\work\EFM32\v4\gesture_bt121 (2)\driver" -O0 -Wall -c -fmessage-length=0 -mno-sched-prolog -fno-builtin -ffunction-sections -fdata-sections -MMD -MP -MF"bglib/bglib_txrx.d" -MT"bglib/bglib_txrx.o" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


