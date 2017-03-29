################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include usbdrv/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
LSS += \
USBTIR.lss \

FLASH_IMAGE += \
USBTIR.hex \

EEPROM_IMAGE += \
USBTIR.eep \

SIZEDUMMY += \
sizedummy \


# All Target
all: USBTIR.elf secondary-outputs

# Tool invocations
USBTIR.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: AVR C Linker'
	avr-gcc -Wl,-Map,USBTIR.map -mmcu=atmega8 -o "USBTIR.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

USBTIR.lss: USBTIR.elf
	@echo 'Invoking: AVR Create Extended Listing'
	-avr-objdump -h -S USBTIR.elf  >"USBTIR.lss"
	@echo 'Finished building: $@'
	@echo ' '

USBTIR.hex: USBTIR.elf
	@echo 'Create Flash image (ihex format)'
	-avr-objcopy -R .eeprom -O ihex USBTIR.elf  "USBTIR.hex"
	@echo 'Finished building: $@'
	@echo ' '

USBTIR.eep: USBTIR.elf
	@echo 'Create eeprom image (ihex format)'
	-avr-objcopy -j .eeprom --no-change-warnings --change-section-lma .eeprom=0 -O ihex USBTIR.elf  "USBTIR.eep"
	@echo 'Finished building: $@'
	@echo ' '

sizedummy: USBTIR.elf
	@echo 'Invoking: Print Size'
	-avr-size --format=avr --mcu=atmega8 USBTIR.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(ASM_DEPS)$(EEPROM_IMAGE)$(FLASH_IMAGE)$(ELFS)$(LSS)$(S_DEPS)$(SIZEDUMMY)$(S_UPPER_DEPS) USBTIR.elf
	-@echo ' '

secondary-outputs: $(LSS) $(FLASH_IMAGE) $(EEPROM_IMAGE) $(SIZEDUMMY)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
