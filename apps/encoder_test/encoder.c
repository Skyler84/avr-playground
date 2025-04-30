#include "encoder.h"
#include <avr/io.h>
#include <avr/interrupt.h>


volatile int8_t encoder_dt;

void encoder_init()
{
    encoder_dt = 0;

    ENCODER_DDR &= ~(_BV(PHA_BIT) | _BV(PHB_BIT)); // Set encoder pins as input
    ENCODER_PORT |= _BV(PHA_BIT) | _BV(PHB_BIT);   // Enable pull-up on encoder pins
    EICRB |= _BV(ISC40) | _BV(ISC50);              // Trigger on any edge
    EICRB &= ~(_BV(ISC41) | _BV(ISC51));           // Trigger on any edge
    EIMSK |= _BV(INT4) | _BV(INT5);                // Enable INT4 and INT5
}

void __attribute__((noinline)) encoder_irq()
{
    static uint8_t last_state = 0;
    uint8_t state = (ENCODER_PIN & (_BV(PHA_BIT) | _BV(PHB_BIT))) >> PHA_BIT;
    uint8_t diff = state ^ last_state;

    static const __flash int8_t lut[16] = {
        0, 1, -1, 0,
        -1, 0, 0, 1,
        1, 0, 0, -1,
        0, -1, 1, 0};
    encoder_dt += lut[state | (last_state << 2)];
    last_state = state;
    PINB = 0x80;
}

ISR(PHA_VECTOR)
{
    encoder_irq();
}

ISR(PHB_VECTOR)
{
    encoder_irq();
}
