#include "encoder.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

int8_t encoder_dt_internal;


#define PHA_BIT 4
#define PHB_BIT 5
#define ENCODER_PORT PORTE
#define ENCODER_DDR DDRE
#define ENCODER_PIN PINE
#define PHA_VECTOR INT4_vect
#define PHB_VECTOR INT5_vect

#if 1
#define PAUI() uint8_t tmp = SREG; cli()
#define RESI() SREG = tmp
#else
#define PAUI() cli()
#define RESI() sei()
#endif

void __attribute__((noinline)) encoder_irq() {
  PAUI();
  // static uint8_t last_state = 0;
  // uint8_t state = (ENCODER_PIN & (_BV(PHA_BIT) | _BV(PHB_BIT))) >> PHA_BIT;

  // static const int8_t lut[16] PROGMEM = {
  //    0,  1, -1,  0,
  //   -1,  0,  0,  1,
  //    1,  0,  0, -1,
  //    0, -1,  1,  0
  // };
  // encoder_dt_internal += pgm_read_byte_far(pgm_get_far_address(lut) + (state | (last_state << 2)));
  // last_state = state;
  RESI();
}

ISR(PHA_VECTOR) {
  // encoder_irq();
}
ISR_ALIAS(PHB_VECTOR, PHA_VECTOR);

int8_t encoder_dt(int8_t dt) {
  PAUI();
  int8_t _dt = encoder_dt_internal;
  encoder_dt_internal -= dt;
  RESI();
  return _dt-dt;
}

void encoder_init() {
  encoder_dt_internal = 0;
  
  ENCODER_DDR &= ~(_BV(PHA_BIT) | _BV(PHB_BIT)); // Set encoder pins as input
  ENCODER_PORT |= _BV(PHA_BIT) | _BV(PHB_BIT); // Enable pull-up on encoder pins
  EICRB |= _BV(ISC40) | _BV(ISC50);    // Trigger on any edge
  EICRB &= ~(_BV(ISC41) | _BV(ISC51)); // Trigger on any edge
  // EIMSK |= _BV(INT4) | _BV(INT5); // Enable INT4 and INT5

  sei();
  
}