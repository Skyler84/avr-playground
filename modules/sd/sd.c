#include "sd/sd.h"

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#include "module/pic.h"


// -------------------------------------------
// SD Card Definitions
// -------------------------------------------

#define CD_BIT 6
#define CD_PORT PORTB
#define CD_DDR DDRB
#define CD_PIN PINB

#define CS_BIT 0
#define CS_PORT PORTB
#define CS_DDR DDRB
#define CS_PIN PINB

#define SCK_BIT 1
#define SCK_PORT PORTB
#define SCK_DDR DDRB
#define SCK_PIN PINB
#define MOSI_BIT 2
#define MOSI_PORT PORTB
#define MOSI_DDR DDRB
#define MOSI_PIN PINB
#define MISO_BIT 3
#define MISO_PORT PORTB
#define MISO_DDR DDRB
#define MISO_PIN PINB
#define SPI_PORT PORTB
#define SPI_DDR DDRB
#define SPI_PIN PINB


static uint8_t spi_tx_rx_byte(uint8_t byte) {

  // load data into register
  SPDR = byte;

  // Wait for transmission complete
  while(!(SPSR & (1 << SPIF)));

  // return SPDR
  return SPDR;
}

static 
__attribute__((noinline))
uint8_t sdcard_calc_crc(uint8_t cmd, uint32_t args) {
  // calculate crc
  uint64_t data = indirect_call(__ashldi3)(((uint64_t)cmd|0x40), 40) | indirect_call(__ashldi3)(args, 8);
  for (int i = 40; i > 0; i--) {
    uint64_t val = indirect_call(__lshrdi3)(data, i)&0xff;
    if ((uint8_t)val & 0x80) {
      data ^= indirect_call(__ashldi3)(((uint64_t)0x89), i);
    }
  }
  return data;
}

static void sdcard_send_command(uint8_t cmd, uint32_t args, uint8_t crc) {
  // send command
  cmd |= 0x40; // CMD0
  spi_tx_rx_byte(cmd); // CMD0
  spi_tx_rx_byte((args >> 24) & 0xFF); // arg0
  spi_tx_rx_byte((args >> 16) & 0xFF); // arg1
  spi_tx_rx_byte((args >> 8) & 0xFF); // arg2
  spi_tx_rx_byte(args & 0xFF); // arg3
  // calculate crc
  // uint8_t crc = sdcard_calc_crc(cmd, args);
  spi_tx_rx_byte(crc | 0x01); // CRC
}

static uint8_t sdcard_read_status() {
  // read status
  uint8_t status = 0xFF;
  for (int i = 0; i < 20; i++) {
    status = spi_tx_rx_byte(0xFF);
    if (status < 0x80) {
      break;
    }
  }
  return status;
}

static void sdcard_command(uint8_t cmd, uint32_t args, uint8_t *buf, uint8_t nbytes) {
  // send command
  spi_tx_rx_byte(0xFF); // send 8 clock pulses
  CS_PORT &= ~_BV(CS_BIT); // CS low
  indirect_call(sdcard_send_command)(cmd, args, indirect_call(sdcard_calc_crc)(cmd, args));
  buf[0] = indirect_call(sdcard_read_status)(); // read status
  if (buf[0] & 0x80) {
    goto err;
  }
  // read data
  for (int i = 1; i < nbytes; i++) {
    buf[i] = spi_tx_rx_byte(0xFF);
  }
err:
  CS_PORT |= _BV(CS_BIT); // CS high
  spi_tx_rx_byte(0xff); // send 8 clock pulses
  return;
}

MODULE_FN_PROTOS(sd, SD_FUNCTION_EXPORTS)


void sd_preinit()
{
  // set up detect pins
  CD_DDR &= ~_BV(CD_BIT); // CD pin as input
  CD_PORT |= _BV(CD_BIT); // pull up
}

uint8_t sd_initialise()
{
  if (!sd_detected()) {
    return 1; // card not inserted
  }

  CS_DDR |= _BV(CS_BIT); // CS pin as output
  CS_PORT |= _BV(CS_BIT); // CS high
  SCK_DDR |= _BV(SCK_BIT); // SCK pin as output
  MOSI_DDR |= _BV(MOSI_BIT); // MOSI pin as output
  MISO_DDR &= ~_BV(MISO_BIT); // MISO pin as input

  // set up SPI
  SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0); // enable SPI, set as master, fck/16
  SPSR = (1 << SPI2X); // double speed
  
  uint8_t resp_buf[5];
  
  // put card into spi mode
  _delay_ms(1);
  CS_PORT |= _BV(CS_BIT); // CS high
  for (int i = 0; i < 10; i++) {
    spi_tx_rx_byte(0xFF); // send 8 clock pulses
  }
  // send cmd0 command
  indirect_call(sdcard_command)(0x00, 0x00000000, resp_buf, 1);
  if (resp_buf[0] != 0x01) {
    return resp_buf[1];
  }

  indirect_call(sdcard_command)(8, 0x000001AA, resp_buf, 5);

  if (
      resp_buf[1] != 0x00 
      || resp_buf[2] != 0x00 
      || resp_buf[3] != 0x01 
      || resp_buf[4] != 0xAA) {
    return 3;
  }
  
  indirect_call(sdcard_command)(58, 0x00000000, resp_buf, 5);
  // check if voltage supported?

  do {
    // cmd41
    indirect_call(sdcard_command)(1, 0x40000000, resp_buf, 1);
    _delay_ms(50);
  } while(resp_buf[0] != 0x00);

  // cmd58
  indirect_call(sdcard_command)(50, 0x00000000, resp_buf, 5);
  _delay_ms(50);

  return 0;
} 

uint8_t sd_detected() 
{
  // check if card is inserted
  if (CD_PIN & _BV(CD_BIT)) {
    return 0; // card not inserted
  }
  return 1; // card inserted
}



uint8_t sd_rdblock(void *self, uint32_t block, uint8_t *buf) {
  (void)self;
  // send cmd17 with args block
  spi_tx_rx_byte(0xFF); // send 8 clock pulses
  CS_PORT &= ~_BV(CS_BIT); // CS low
  sdcard_send_command(17, block, 0);
  // wait for response
  uint8_t response = sdcard_read_status();
  if (response != 0x00) {
    goto err;
  }
  while(spi_tx_rx_byte(0xFF) != 0xFE) {
    // wait for start block token
  }
  // read data
  for (int i = 0; i < 512; i++) {
    buf[i] = spi_tx_rx_byte(0xFF);
  }
  spi_tx_rx_byte(0xff);
  spi_tx_rx_byte(0xff);
err:
  CS_PORT |= _BV(CS_BIT); // CS high
  spi_tx_rx_byte(0xff); // send 8 clock pulses
  return response;  
}

uint8_t sd_wrblock(void *self, uint32_t block, const uint8_t *buf) {
  (void)self;
  // send cmd24 with args block
  spi_tx_rx_byte(0xFF); // send 8 clock pulses
  CS_PORT &= ~_BV(CS_BIT); // CS low
  sdcard_send_command(24, block, 0);
  // wait for response
  uint8_t response = sdcard_read_status();
  if (response != 0x00) {
    goto err;
  }
  spi_tx_rx_byte(0xFF); // send 8 clock pulses
  // send data token
  spi_tx_rx_byte(0xFE); // start block token
  // write data
  for (int i = 0; i < 512; i++) {
    spi_tx_rx_byte(buf[i]);
  }
  spi_tx_rx_byte(0xff);
  spi_tx_rx_byte(0xff);
  // wait for write to complete
  while(spi_tx_rx_byte(0xFF) == 0x00) {
    // wait for write to complete
  }
  // read data response
  response = spi_tx_rx_byte(0xFF);
  if ((response & 0x1F) != 0x05) {
    goto err;
  }
  response = 0;
  // wait for write to complete
  while(spi_tx_rx_byte(0xFF) == 0x00) {
    // wait for write to complete
  }
err:
  CS_PORT |= _BV(CS_BIT); // CS high
  spi_tx_rx_byte(0xff); // send 8 clock pulses
  return response;  
}

REGISTER_MODULE(sd, SD_MODULE_ID, SD_FUNCTION_EXPORTS, SD_API_VER);
