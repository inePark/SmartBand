################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include emlib/subdir.mk
-include bglib/subdir.mk
-include CMSIS/EFM32HG/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: gesture_bt121\ (2).axf

# Tool invocations
gesture_bt121\ (2).axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GNU ARM C Linker'
	arm-none-eabi-gcc -g -gdwarf-2 -mcpu=cortex-m0plus -mthumb -T "gesture_bt121 (2).ld" -Xlinker --gc-sections -Xlinker -Map="gesture_bt121 (2).map" --specs=nano.specs -o "gesture_bt121 (2).axf" "./CMSIS/EFM32HG/startup_gcc_efm32hg.o" "./CMSIS/EFM32HG/system_efm32hg.o" "./bglib/bglib_txrx.o" "./emlib/em_assert.o" "./emlib/em_cmu.o" "./emlib/em_emu.o" "./emlib/em_gpio.o" "./emlib/em_i2c.o" "./emlib/em_int.o" "./emlib/em_leuart.o" "./emlib/em_system.o" "./emlib/em_timer.o" "./emlib/em_usart.o" "./src/InitDevice.o" "./src/gestureRecognition.o" "./src/gesture_bt121_main.o" "./src/key_input.o" "./src/sensor.o" "./src/user_delay.o" "./src/user_usart.o" -Wl,--start-group -lgcc -lc -lnosys -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

	@echo 'Building hex file: gesture_bt121 (2).hex'
	arm-none-eabi-objcopy -O ihex "gesture_bt121 (2).axf" "gesture_bt121 (2).hex"
	@echo ' '

	@echo 'Building bin file: gesture_bt121 (2).bin'
	arm-none-eabi-objcopy -O binary "gesture_bt121 (2).axf" "gesture_bt121 (2).bin"
	@echo ' '

	@echo 'Building s37 file: gesture_bt121 (2).s37'
	arm-none-eabi-objcopy -O srec "gesture_bt121 (2).axf" "gesture_bt121 (2).s37"
	@echo ' '

	@echo 'Running size tool'
	arm-none-eabi-size "gesture_bt121 (2).axf"
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) "gesture_bt121 (2).axf"
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
