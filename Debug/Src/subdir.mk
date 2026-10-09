################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/esp8266_mqtt.c \
../Src/fsm.c \
../Src/main.c \
../Src/relay.c \
../Src/sensor.c \
../Src/spi1_lcd_st7789.c \
../Src/syscalls.c \
../Src/sysmem.c \
../Src/timer.c \
../Src/uart.c \
../Src/w25q64.c 

OBJS += \
./Src/esp8266_mqtt.o \
./Src/fsm.o \
./Src/main.o \
./Src/relay.o \
./Src/sensor.o \
./Src/spi1_lcd_st7789.o \
./Src/syscalls.o \
./Src/sysmem.o \
./Src/timer.o \
./Src/uart.o \
./Src/w25q64.o 

C_DEPS += \
./Src/esp8266_mqtt.d \
./Src/fsm.d \
./Src/main.d \
./Src/relay.d \
./Src/sensor.d \
./Src/spi1_lcd_st7789.d \
./Src/syscalls.d \
./Src/sysmem.d \
./Src/timer.d \
./Src/uart.d \
./Src/w25q64.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o Src/%.su Src/%.cyclo: ../Src/%.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DSTM32 -DSTM32F1 -DSTM32F103C8Tx -DSTM32F103xB -c -I../Inc -I"C:/Users/PHU/Downloads/stm32cubef1/STM32Cube_FW_F1_V1.8.0/Drivers/CMSIS/Device/ST/STM32F1xx/Include" -I"C:/Users/PHU/Downloads/stm32cubef1/STM32Cube_FW_F1_V1.8.0/Drivers/CMSIS/Include" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Src

clean-Src:
	-$(RM) ./Src/esp8266_mqtt.cyclo ./Src/esp8266_mqtt.d ./Src/esp8266_mqtt.o ./Src/esp8266_mqtt.su ./Src/fsm.cyclo ./Src/fsm.d ./Src/fsm.o ./Src/fsm.su ./Src/main.cyclo ./Src/main.d ./Src/main.o ./Src/main.su ./Src/relay.cyclo ./Src/relay.d ./Src/relay.o ./Src/relay.su ./Src/sensor.cyclo ./Src/sensor.d ./Src/sensor.o ./Src/sensor.su ./Src/spi1_lcd_st7789.cyclo ./Src/spi1_lcd_st7789.d ./Src/spi1_lcd_st7789.o ./Src/spi1_lcd_st7789.su ./Src/syscalls.cyclo ./Src/syscalls.d ./Src/syscalls.o ./Src/syscalls.su ./Src/sysmem.cyclo ./Src/sysmem.d ./Src/sysmem.o ./Src/sysmem.su ./Src/timer.cyclo ./Src/timer.d ./Src/timer.o ./Src/timer.su ./Src/uart.cyclo ./Src/uart.d ./Src/uart.o ./Src/uart.su ./Src/w25q64.cyclo ./Src/w25q64.d ./Src/w25q64.o ./Src/w25q64.su

.PHONY: clean-Src

