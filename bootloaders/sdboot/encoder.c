#include "encoder.h"
#include <avr/io.h>
#include <avr/interrupt.h>

int8_t encoder_dt_internal;


#define PHA_BIT 4
#define PHB_BIT 5
#define ENCODER_PORT PORTE
#define ENCODER_DDR DDRE
#define ENCODER_PIN PINE
#define PHA_VECTOR INT4_vect
#define PHB_VECTOR INT5_vect


void __attribute__((noinline)) encoder_irq() {
  cli();
  static uint8_t last_state = 0;
  uint8_t state = (ENCODER_PIN & (_BV(PHA_BIT) | _BV(PHB_BIT))) >> PHA_BIT;

  static const __flash1 int8_t lut[16] = {
     0,  1, -1,  0,
    -1,  0,  0,  1,
     1,  0,  0, -1,
     0, -1,  1,  0
  };
  encoder_dt_internal += lut[state | (last_state << 2)];
  last_state = state;
  sei();
}

ISR(PHA_VECTOR) {
  encoder_irq();
}
ISR(PHB_VECTOR) {
  encoder_irq();
}

int8_t encoder_dt(int8_t dt) {
  cli();
  int8_t _dt = encoder_dt_internal;
  encoder_dt_internal -= dt;
  sei();
  return _dt-dt;
}

void encoder_init() {
  encoder_dt_internal = 0;
  
  ENCODER_DDR &= ~(_BV(PHA_BIT) | _BV(PHB_BIT)); // Set encoder pins as input
  ENCODER_PORT |= _BV(PHA_BIT) | _BV(PHB_BIT); // Enable pull-up on encoder pins
  EICRB |= _BV(ISC40) | _BV(ISC50);    // Trigger on any edge
  EICRB &= ~(_BV(ISC41) | _BV(ISC51)); // Trigger on any edge
  EIMSK |= _BV(INT4) | _BV(INT5); // Enable INT4 and INT5

  sei();
  
}