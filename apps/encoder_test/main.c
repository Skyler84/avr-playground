#include <avr/io.h>
#include <avr/interrupt.h>

int8_t encoder_dt;

#define PHA_BIT 4
#define PHB_BIT 5
#define ENCODER_PORT PORTE
#define ENCODER_DDR DDRE
#define ENCODER_PIN PINE
#define PHA_VECTOR INT4_vect
#define PHB_VECTOR INT5_vect

void __attribute__((noinline)) encoder_irq() {
  static uint8_t last_state = 0;
  uint8_t state = (ENCODER_PIN & (_BV(PHA_BIT) | _BV(PHB_BIT))) >> PHA_BIT;
  uint8_t diff = state ^ last_state;

  static const __flash int8_t lut[16] = {
     0,  1, -1,  0,
    -1,  0,  0,  1,
     1,  0,  0, -1,
     0, -1,  1,  0
  };
  encoder_dt += lut[state | (last_state << 2)];
  last_state = state;
  PINB = 0x80;
}

ISR(PHA_VECTOR) {
  encoder_irq();
}
ISR(PHB_VECTOR) {
  encoder_irq();
}

void encoder_test() {
  cli();
  // encoder_pos += encoder_dt;
  encoder_dt = 0;
  sei();
  // lcd_fns.fill_rectangle(0, 100, 0, 50, BLACK);
  // lcd_fns.display_stringP(0, lcd_size.x, 0, 2, PSTR("Encoder"), WHITE);
  // lcd_fns.display_char(0, 20, 2, hex[(encoder_pos>>4)&0x0f], WHITE);
  // lcd_fns.display_char(16, 20, 2, hex[(encoder_pos>>0)&0x0f], WHITE);

}

int main() {

  // disable watchdog
  MCUSR = ~_BV(WDRF); // clear watchdog reset flag
  WDTCSR = _BV(WDCE);
  WDTCSR = 0x00;

  // Set clock prescaler to 1
  CLKPR = 0x80; // Enable clock change
  CLKPR = 0x00; // Set clock prescaler to 1

  DDRB = 0xFF; // Set PORTB as output
  
  encoder_dt = 0;

  ENCODER_DDR &= ~(_BV(PHA_BIT) | _BV(PHB_BIT)); // Set encoder pins as input
  ENCODER_PORT |= _BV(PHA_BIT) | _BV(PHB_BIT); // Enable pull-up on encoder pins
  EICRB |= _BV(ISC40) | _BV(ISC50);    // Trigger on any edge
  EICRB &= ~(_BV(ISC41) | _BV(ISC51)); // Trigger on any edge
  EIMSK |= _BV(INT4) | _BV(INT5); // Enable INT4 and INT5
  sei();
  while(1) {
    // encoder_test();
  }
}