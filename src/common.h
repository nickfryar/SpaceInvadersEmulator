#ifndef COMMON_H
#define COMMON_H

#include <cstdint>

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t

#define ACC i8080.registers[7]
#define PC i8080.pc
#define SP i8080.sp
#define SIGN i8080.status[0]
#define ZERO i8080.status[1]
#define CARRY i8080.status[3]
#define AUX_CARRY i8080.status[4]
#define PARITY i8080.status[2]
#define H_L ((i8080.registers[4] << 8) | (i8080.registers[5]))

#endif /*COMMON_H*/
