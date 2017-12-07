################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include emlib/subdir.mk
-include CMSIS/efm32hg/subdir.mk
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
all: 9axis_sensors.axf

# Tool invocations
9axis_sensors.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GNU ARM C Linker'
	arm-none-eabi-gcc -g -gdwarf-2 -mcpu=cortex-m0plus -mthumb -T "9axis_sensors.ld" -Xlinker --gc-sections -Xlinker -Map="9axis_sensors.map" --specs=nano.specs -o 9axis_sensors.axf "./src/InitDevice.o" "./src/gestureRecognition.o" "./src/main.o" "./emlib/em_cmu.o" "./emlib/em_emu.o" "./emlib/em_gpio.o" "./emlib/em_i2c.o" "./emlib/em_system.o" "./emlib/em_usart.o" "./CMSIS/efm32hg/startup_gcc_efm32hg.o" "./CMSIS/efm32hg/system_efm32hg.o" "-lm" -Wl,--start-group -lgcc -lc -lnosys -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

	@echo 'Building hex file: 9axis_sensors.hex'
	arm-none-eabi-objcopy -O ihex "9axis_sensors.axf" "9axis_sensors.hex"
	@echo ' '

	@echo 'Building bin file: 9axis_sensors.bin'
	arm-none-eabi-objcopy -O binary "9axis_sensors.axf" "9axis_sensors.bin"
	@echo ' '

	@echo 'Running size tool'
	arm-none-eabi-size "9axis_sensors.axf"
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) 9axis_sensors.axf
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
