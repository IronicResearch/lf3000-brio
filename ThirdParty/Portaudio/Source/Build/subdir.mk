################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../pa_allocation.c \
../pa_converters.c \
../pa_cpuload.c \
../pa_debugprint.c \
../pa_dither.c \
../pa_front.c \
../pa_process.c \
../pa_skeleton.c \
../pa_stream.c \
../pa_trace.c \
../pa_unix_hostapis.c \
../pa_unix_oss.c \
../pa_unix_util.c 

OBJS += \
./pa_allocation.o \
./pa_converters.o \
./pa_cpuload.o \
./pa_debugprint.o \
./pa_dither.o \
./pa_front.o \
./pa_process.o \
./pa_skeleton.o \
./pa_stream.o \
./pa_trace.o \
./pa_unix_hostapis.o \
./pa_unix_oss.o \
./pa_unix_util.o 

C_DEPS += \
./pa_allocation.d \
./pa_converters.d \
./pa_cpuload.d \
./pa_debugprint.d \
./pa_dither.d \
./pa_front.d \
./pa_process.d \
./pa_skeleton.d \
./pa_stream.d \
./pa_trace.d \
./pa_unix_hostapis.d \
./pa_unix_oss.d \
./pa_unix_util.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -DPA_LITTLE_ENDIAN -DPA_ENABLE_DEBUG_OUTPUT -DPA_LOG_API_CALLS -DHAVE_NANOSLEEP=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_UNISTD_H=1 -DHAVE_STRING_H=1 -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_DLFCN_H=1 -DPA_USE_OSS=1 -DSIZEOF_INT=4 -DSIZEOF_SHORT=2 -DSIZEOF_LONG=4 -I"/home/darren/workspace/Brio2/ThirdParty/Portaudio/Source/pa_headers" -O0 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


