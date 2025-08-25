################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Common/config_app.c \
../Core/Common/configuration.c \
../Core/Common/control.c \
../Core/Common/flash_app.c \
../Core/Common/flash_conf.c \
../Core/Common/reg_map.c \
../Core/Common/system_msp.c 

OBJS += \
./Core/Common/config_app.o \
./Core/Common/configuration.o \
./Core/Common/control.o \
./Core/Common/flash_app.o \
./Core/Common/flash_conf.o \
./Core/Common/reg_map.o \
./Core/Common/system_msp.o 

C_DEPS += \
./Core/Common/config_app.d \
./Core/Common/configuration.d \
./Core/Common/control.d \
./Core/Common/flash_app.d \
./Core/Common/flash_conf.d \
./Core/Common/reg_map.d \
./Core/Common/system_msp.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Common/%.o Core/Common/%.su Core/Common/%.cyclo: ../Core/Common/%.c Core/Common/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32F072xB -c -I../Core/Inc -I../Core/Board -I../Core/Serial -I../Drivers/STM32F0xx_HAL_Driver/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F0xx/Include -I../Drivers/CMSIS/Include -I../Core/Common -Og -ffunction-sections -fdata-sections -Wall -Wfatal-errors -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Common

clean-Core-2f-Common:
	-$(RM) ./Core/Common/config_app.cyclo ./Core/Common/config_app.d ./Core/Common/config_app.o ./Core/Common/config_app.su ./Core/Common/configuration.cyclo ./Core/Common/configuration.d ./Core/Common/configuration.o ./Core/Common/configuration.su ./Core/Common/control.cyclo ./Core/Common/control.d ./Core/Common/control.o ./Core/Common/control.su ./Core/Common/flash_app.cyclo ./Core/Common/flash_app.d ./Core/Common/flash_app.o ./Core/Common/flash_app.su ./Core/Common/flash_conf.cyclo ./Core/Common/flash_conf.d ./Core/Common/flash_conf.o ./Core/Common/flash_conf.su ./Core/Common/reg_map.cyclo ./Core/Common/reg_map.d ./Core/Common/reg_map.o ./Core/Common/reg_map.su ./Core/Common/system_msp.cyclo ./Core/Common/system_msp.d ./Core/Common/system_msp.o ./Core/Common/system_msp.su

.PHONY: clean-Core-2f-Common

