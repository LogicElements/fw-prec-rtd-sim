################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Board/MAX7300.c \
../Core/Board/PCA9505.c \
../Core/Board/RTD_Handle.c \
../Core/Board/RTD_lib.c \
../Core/Board/SHT20.c \
../Core/Board/led.c \
../Core/Board/switch.c 

OBJS += \
./Core/Board/MAX7300.o \
./Core/Board/PCA9505.o \
./Core/Board/RTD_Handle.o \
./Core/Board/RTD_lib.o \
./Core/Board/SHT20.o \
./Core/Board/led.o \
./Core/Board/switch.o 

C_DEPS += \
./Core/Board/MAX7300.d \
./Core/Board/PCA9505.d \
./Core/Board/RTD_Handle.d \
./Core/Board/RTD_lib.d \
./Core/Board/SHT20.d \
./Core/Board/led.d \
./Core/Board/switch.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Board/%.o Core/Board/%.su Core/Board/%.cyclo: ../Core/Board/%.c Core/Board/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32F072xB -c -I../Core/Inc -I../Core/Board -I../Core/Serial -I../Drivers/STM32F0xx_HAL_Driver/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F0xx/Include -I../Drivers/CMSIS/Include -I../Core/Common -Og -ffunction-sections -fdata-sections -Wall -Wfatal-errors -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Board

clean-Core-2f-Board:
	-$(RM) ./Core/Board/MAX7300.cyclo ./Core/Board/MAX7300.d ./Core/Board/MAX7300.o ./Core/Board/MAX7300.su ./Core/Board/PCA9505.cyclo ./Core/Board/PCA9505.d ./Core/Board/PCA9505.o ./Core/Board/PCA9505.su ./Core/Board/RTD_Handle.cyclo ./Core/Board/RTD_Handle.d ./Core/Board/RTD_Handle.o ./Core/Board/RTD_Handle.su ./Core/Board/RTD_lib.cyclo ./Core/Board/RTD_lib.d ./Core/Board/RTD_lib.o ./Core/Board/RTD_lib.su ./Core/Board/SHT20.cyclo ./Core/Board/SHT20.d ./Core/Board/SHT20.o ./Core/Board/SHT20.su ./Core/Board/led.cyclo ./Core/Board/led.d ./Core/Board/led.o ./Core/Board/led.su ./Core/Board/switch.cyclo ./Core/Board/switch.d ./Core/Board/switch.o ./Core/Board/switch.su

.PHONY: clean-Core-2f-Board

