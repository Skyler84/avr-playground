#include <avr/io.h>
#include <avr/interrupt.h>

uint8_t lch, rch;

// generates a simple sawtooth wave
// on both left and right channels

ISR(TIMER1_OVF_vect) {
    OCR1A = lch;
    lch+=2;
    if (lch >= 250) lch -= 250;
}

ISR(TIMER3_OVF_vect) {
    OCR3A = rch;
    rch+=3;
    if (rch >= 250) rch -= 250;
}

void audio_init_32khz() {

    // we want TIM1/3 WGM 0xE FastPWM TOP=ICRn
    // COMnA = 0x3 
    PRR0 &= ~_BV(PRTIM1); // power up TIM1
    PRR1 &= ~_BV(PRTIM3); // power up TIM3
    ICR1 = 249; // 32KHz
    ICR3 = 249; // 32KHz
    
    TCCR1A = _BV(COM1A1) | _BV(COM1A0) | _BV(WGM11); // 
    TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS10); // 
    TCCR3A = _BV(COM3A1) | _BV(COM3A0) | _BV(WGM31); // 
    TCCR3B = _BV(WGM33) | _BV(WGM32) | _BV(CS30); // 
    
    // set OCRnA to achieve voltage out
    // modulate OCRnA to acheive frequency out
}

void sysclk_init() {
    CLKPR = 0x80;
    CLKPR = 0x00;
}

void main() {
    sysclk_init();
    audio_init_32khz();
    while(1);

}