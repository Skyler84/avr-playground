/*  Author: Steve Gunn
 * Licence: This work is licensed under the Creative Commons Attribution License.
 *           View this license at http://creativecommons.org/about/licenses/
 */
 
#include <avr/io.h>



#define CTRL A
#define DATA C

#define CAT(a, b) a ## b
#define XCAT(a, b) CAT(a, b)

#define PORT(x) XCAT(PORT, x)
#define DDR(x) XCAT(DDR, x)
#define PIN(x) XCAT(PIN, x)

#define CTRL_PORT	PORT(CTRL)
#define CTRL_DDR	DDR(CTRL)
#define CTRL_PIN	PIN(CTRL)
#define DATA_PORT	PORT(DATA)
#define DATA_DDR	DDR(DATA)
#define DATA_PIN	PIN(DATA)

#ifdef __AVR_AT90USB1286__
#define RESET_PORT PORTC
#define RESET_DDR  DDRC
#define RESET_PIN  PINC
#define RESET_BIT  7
#define BLC_PORT  PORTB
#define BLC_DDR   DDRB
#define BLC_PIN   PINB
#define BLC_BIT   4
#define VSYNC_PORT PORTC
#define VSYNC_DDR  DDRC
#define VSYNC_PIN  PINC
#define VSYNC_BIT  1

#else

#define CS_BIT			0	/* Active low chip select enable */
#define BLC_BIT			1	/* Active High back light control */
#define RESET_BIT		2
#define WR_BIT			3
#define RS_BIT			4
#define RD_BIT			5
#define VSYNC_BIT		6
#define FMARK_BIT		7

#define BLC_PORT PORTB
#define BLC_DDR  DDRB
#define BLC_PIN  PINB
#define RESET_PORT PORTB
#define RESET_DDR  DDRB
#define RESET_PIN  PINB
#define CS_PORT PORTB
#define CS_DDR  DDRB
#define CS_PIN  PINB
#define WR_PORT PORTB
#define WR_DDR  DDRB
#define WR_PIN  PINB
#define RS_PORT PORTB
#define RS_DDR  DDRB
#define RS_PIN  PINB
#define RD_PORT PORTB
#define RD_DDR  DDRB
#define RD_PIN  PINB
#define VSYNC_PORT PORTB
#define VSYNC_DDR  DDRB
#define VSYNC_PIN  PINB
#define VSYNC_BIT  6

#endif

#define BLC_lo()   do { BLC_PORT &= ~_BV(BLC_BIT);     } while(0)
#define BLC_hi()   do { BLC_PORT |= _BV(BLC_BIT);      } while(0)
#define RESET_lo() do { RESET_PORT &= ~_BV(RESET_BIT); } while(0)
#define RESET_hi() do { RESET_PORT |= _BV(RESET_BIT);  } while(0)
#ifndef __AVR_AT90USB1286__
#define CS_lo()    do { CS_PORT &= ~_BV(CS_BIT);       } while(0)
#define CS_hi()    do { CS_PORT |= _BV(CS_BIT);        } while(0)
#define WR_lo()    do { WR_PORT &= ~_BV(WR_BIT);       } while(0)
#define WR_hi()    do { WR_PORT |= _BV(WR_BIT);        } while(0)
#define RS_lo()    do { RS_PORT &= ~_BV(RS_BIT);       } while(0)
#define RS_hi()    do { RS_PORT |= _BV(RS_BIT);        } while(0)
#define RD_lo()    do { RD_PORT &= ~_BV(RD_BIT);       } while(0)
#define RD_hi()    do { RD_PORT |= _BV(RD_BIT);        } while(0)
#define WRITE(x)   do { DATA_PORT = (x);               } while(0)
#endif
#define VSYNC_lo() do { VSYNC_PORT &= ~_BV(VSYNC_BIT); } while(0)
#define VSYNC_hi() do { VSYNC_PORT |= _BV(VSYNC_BIT);  } while(0)

