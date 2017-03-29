################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ir.c \
../main.c \
../uart.c \
../usb.c 

OBJS += \
./ir.o \
./main.o \
./uart.o \
./usb.o 

C_DEPS += \
./ir.d \
./main.d \
./uart.d \
./usb.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: AVR Compiler'
	avr-gcc -I"/home/sergio/electronica/proyectos/workspaceAVR/USBTIR/usbdrv" -I"/home/sergio/electronica/proyectos/workspaceAVR/USBTIR" -I/usr/lib/avr/include -I/usr/lib/avr/include/avr -I/usr/lib/avr/include/compat -I/usr/lib/avr/include/util -Wall -Os -fpack-struct -fshort-enums -std=gnu99 -funsigned-char -funsigned-bitfields -mmcu=atmega8 -DF_CPU=12000000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


