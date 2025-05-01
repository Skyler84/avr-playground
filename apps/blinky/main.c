#include <avr/io.h>
#include <util/delay.h>

#define LED_PORT PORTB
#define LED_DDR  DDRB
#define LED_PIN  PINB
#define LED_BIT  PB7

int main() {
    // set clock prescaler to 1
    CLKPR = _BV(CLKPCE); // Enable change
    CLKPR = 0; // Set prescaler to 1
    // Set LED pin as output
    LED_DDR |= _BV(LED_BIT);

    while (1) {
        // Toggle the LED
        LED_PIN = _BV(LED_BIT);

        // Delay
        _delay_ms(1000);
    }
}