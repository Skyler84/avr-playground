#pragma once
#include <stdint.h>

#define PHA_BIT 4
#define PHB_BIT 5
#define ENCODER_PORT PORTE
#define ENCODER_DDR DDRE
#define ENCODER_PIN PINE
#define PHA_VECTOR INT4_vect
#define PHB_VECTOR INT5_vect

extern volatile int8_t encoder_dt;

void encoder_init();
