#include <avr/io.h>
#include <util/delay.h>
#include "module/module.h"

#define T 25

static void setup_led() {
  // Set up LED pin
  DDRB |= (1 << PB7); // Set PB7 as output
  PORTB &= ~(1 << PB7); // Turn off LED
}

static void led_on() {
  PORTB |= (1 << PB7); // Turn on LED
}

static void led_off() {
  PORTB &= ~(1 << PB7); // Turn off LED
}

static void wait(int n) {
  uint8_t ckpr = CLKPR;
  CLKPR = 0x80; // enable prescaler change
  CLKPR = 0x03; // Set prescaler to 8
  for (int i = 0; i < n; i++) {
    _delay_ms(T);
  }
  CLKPR = 0x80; // Set prescaler to 1
  CLKPR = ckpr; // restore clock prescaler
}

static void blink_1() {
  led_on();
  wait(3);
  led_off();
  wait(1);
}

static void blink_0() {
  led_on();
  wait(1);
  led_off();
  wait(3);
}

static void long_blink() {
  led_on();
  wait(9);
  led_off();
  wait(1);
}

static void blink_once() {
  led_on();
  wait(1);
  led_off();
  wait(1);
}

void blink_bits(uint32_t value, uint8_t nbits) {
  for (uint8_t i = 0; i < nbits; i++) {
    if (value & (1 << i)) {
      blink_1();
    } else {
      blink_0();
    }
  }
}

static void blink_n_times(uint8_t n) {
  for (uint8_t i = 0; i < n; i++) {
    blink_once();
  }
}

void module_runtime_init_error_handler(module_id_t id, moduleptr_t *modptr, uint16_t fn_id, fn_ptr_t *fn_ptr) 
{
  uint8_t ckpr = CLKPR;
  CLKPR = 0x80; // enable prescaler change
  CLKPR = 0x03; // Set prescaler to 8
  (void)  id;
  (void)  modptr;
  (void)  fn_id;
  (void)  fn_ptr;
  setup_led();

  long_blink();
  // uint32_t value = 0;
  // uint8_t nbits = 0;
  if (*modptr == 0) {
    // blink 0
    blink_n_times(1);
    wait(6);
    blink_bits(id, 16);
  } else if (*fn_ptr == 0) {
    // blink 1
    blink_n_times(2);
    wait(6);
    blink_bits(id, 16);
    wait(6);
    blink_bits(fn_id, 16);
  } else {
    // blink 2
    long_blink();
    blink_n_times(3);
  }
  
  wait(10);
  
  CLKPR = 0x80; // Set prescaler to 1
  CLKPR = ckpr; // restore clock prescaler
}