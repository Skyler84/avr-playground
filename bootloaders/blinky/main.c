#include <avr/io.h>
#include <util/delay.h>

int main() 
{
  DDRB = 0x80;
  while(1) {
    PINB = 0x80;
    _delay_ms(100);
  }
  return 0;
}