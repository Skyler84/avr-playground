#include "spm.h"
#include "instructions.h"

#include "module/imports.h"
#include "gfx/gfx.h"

extern gfx_t gfx;

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/boot.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include <stdlib.h>
#include <stdio.h>

void setup_timer_0b(enum spm_sequence_type spm_seq) {
  TCCR0A = 0; // normal mode
  TCCR0B = 0; // stop timer 0
  TCNT0 = 0; // reset timer 0
  TIFR0 = _BV(OCF0B) | _BV(OCF0A) | _BV(TOV0); // clear any pending interrupts on timer 0
  switch(spm_seq) {
    case SPM_SEQUENCE_TYPE_OUT_SPMCSR: OCR0B = 8+0; break;
    case SPM_SEQUENCE_TYPE_STS_SPMCSR: OCR0B = 8+1; break;
  };
  TIMSK0 = _BV(OCIE0B); // enable compare match B interrupt on timer 0
  sei();
}

void dump(){
  // uint8_t regs[32];

}

void timeout(uint8_t t) {
  cli();
  uint16_t z;
  asm("movw %0, r30\n\t" :"=r" (z):); // save timeout value in r30:r31
  char buf[64];
  snprintf(buf, sizeof(buf), "t: %02x", t);
  MODULE_CALL_THIS(gfx, text, &gfx, ((display_region_t){0, 220, 319, 239}), buf);
  snprintf(buf, sizeof(buf), "Z: %04x", z);
  MODULE_CALL_THIS(gfx, text, &gfx, ((display_region_t){0, 200, 319, 239}), buf);
  while(1);
}

ISR(TIMER0_COMPB_vect, ISR_NAKED) {
	asm volatile(
    "L_%=:"
    // "rjmp L_%=\n\t"
    "ldi r30, 0\n\t"
		"sts %0, r30\n\t"	// stop timer 0
		"sts %1, r30\n\t"	// reset timer 0.
		"ldi r30, 7\n\t"
		"sts %2, r30\n\t"	// clear interrupts on timer 0.
		"pop r31\n\t"	// Pop interrupt return address
		"pop r30\n\t" // 
		"ret\n\t" 
    :: 
    "i" (_SFR_MEM_ADDR(TCCR0B)), 
    "i" (_SFR_MEM_ADDR(TCNT0)), 
    "i" (_SFR_MEM_ADDR(TIMSK0))
  );
  __builtin_unreachable();
}

void do_spm(struct spm_sequence spm_seq, uint8_t cmd, uint32_t addr, uint16_t optvalue) {
  // This function is used to execute the SPM instruction.
  // It will call the bootloader stub with the command.
  setup_timer_0b(spm_seq.seq_type);
  if (spm_seq.seq == NULL) {
    return; // no sequence found
  }
  eeprom_busy_wait();
  boot_spm_busy_wait(); // Wait until the memory is erased.

  uint8_t seq_reg_no = 0;
  if (spm_seq.seq_type == SPM_SEQUENCE_TYPE_OUT_SPMCSR) {
    // find the sequence register number
    seq_reg_no = out_instruction_reg(pgm_read_word_far(((uint_farptr_t)(uintptr_t)spm_seq.seq)*2));
  } else if (spm_seq.seq_type == SPM_SEQUENCE_TYPE_STS_SPMCSR) {
    // find the sequence register number
    seq_reg_no = sts_instruction_reg(pgm_read_word_far(((uint_farptr_t)(uintptr_t)spm_seq.seq)*2));
  }
  asm volatile(
    "mov r30, %[savereg]\n\t"        // save the sequence register number      
    "ldi r31, 0\n\t"                 // clear r31                              
    "push r30\n\t"                   // save r30                               
    "ld r0, Z\n\t"                   // load dynamic register value            
    "push r0\n\t"                    // save dynamic register value   
    "out %[rampz], %[addrhi]\n\t"

    "ldi r30, pm_lo8(SPMRET_%=)\n\t" // load the address of the return address 
    "ldi r31, pm_hi8(SPMRET_%=)\n\t" // load the address of the return address 
    "push r30\n\t"                   // save the return address       
    "push r31\n\t"                   // save the return address        

    "movw r30, %[spmseq]\n\t"        // load the jump address        
    "push r30\n\t"                   // save the jump address          
    "push r31\n\t"                   // save the jump address          

    "movw r0, %[optvalue]\n\t"

    "movw r30, %[addr]\n\t"          // load address into Z register           
    "push r30\n\t"                   // save the address low byte              
    "push r31\n\t"                   // save the address high byte             
    
    "mov r30, %[savereg]\n\t"        // save the sequence register number      
    "ldi r31, 0\n\t"                 // clear r31                              
    "st Z, %[in]\n\t"            // store the command in the sequence register 
    
    "ldi r30, 1\n\t"                  // load the sequence register number
    "sts %[tccr], r30\n\t"            // start timer 0                         [0]
    
    "pop r31\n\t"                    // restore the address high byte          [2] +2
    "pop r30\n\t"                    //                                        [4] +2
    "ret\n\t"                        // jump to the SPM sequence               [9?] +4/5
    "SPMRET_%=:\n\t"                 // label for the SPM sequence

    "pop r0\n\t"            // load dynamic register value
    "pop r30\n\t"                    // restore dynamic register number
    "ldi r31, 0\n\t"                 // clear r31
    "st Z, r0\n\t"          // store the command in the sequence register
    "clr r1\n\t"
    :
    : [spmcsr] "I" (_SFR_IO_ADDR(SPMCSR))
    , [rampz] "I" (_SFR_IO_ADDR(RAMPZ))
    , [tccr] "i" (_SFR_MEM_ADDR(TCCR0B))
    , [in] "r" (cmd)
    , [savereg] "r" (seq_reg_no)
    , [addr] "r" ((uint16_t)addr)
    , [addrhi] "r" (addr>>16)
    , [spmseq] "r" (spm_seq.seq)
    , [optvalue] "r" (optvalue)
    : "r30", "r31"
  );
  while (SPMCSR & _BV(SPMEN)) {
    // wait for SPM to complete
  }
}

uint16_t find_next_spm_instruction(uint16_t pgm_addr) {
  // This function will find the next SPM instruction in the program memory.
  // It will return the address of the next SPM instruction, or 0 if no SPM instruction was found.
  if (pgm_addr < NRWW_START) {
    return 0; // not in NRWW section
  }
  do{
    if (is_spm_instruction(pgm_read_word_far(pgm_addr*2))) {
      return pgm_addr; // found SPM instruction
    }
  }while(pgm_addr++ < NRWW_END);
  return 0; // no SPM instruction found
}

bool decode_spm_sequence(struct spm_sequence *spm_seq, uint16_t pgm_addr) {
  // This function will decode the SPM sequence from the program memory.
  // It will return true if the sequence was found, false otherwise.
  if (spm_seq == NULL) {
    return false;
  }
  if (pgm_addr < NRWW_START) {
    return false;
  }
  if (!is_spm_instruction(pgm_read_word_far(pgm_addr*2))) {
    return false; // not an SPM instruction
  }
  if (is_out_instruction(pgm_read_word_far(pgm_addr - 2)) &&
      out_instruction_addr(pgm_read_word_far(pgm_addr - 2)) == _SFR_IO_ADDR(SPMCSR)) {
    spm_seq->seq_type = SPM_SEQUENCE_TYPE_OUT_SPMCSR;
    spm_seq->seq = (void(*)(void))((uintptr_t)(pgm_addr/2-1));
    return true;
  } else if (is_sts_instruction(pgm_read_word_far(pgm_addr - 4)) &&
              sts_instruction_reg(pgm_read_word_far(pgm_addr - 4)) == _SFR_MEM_ADDR(SPMCSR)) {
    spm_seq->seq_type = SPM_SEQUENCE_TYPE_STS_SPMCSR;
    spm_seq->seq = (void(*)(void))((uintptr_t)(pgm_addr/2-2));
    return true;
  }
  return false;
}

void page_erase(struct spm_sequence seq, uint32_t page) {
  // This function will erase a flash page.
  // It will call the SPM instruction with the command 0x03 (page erase).
  if (seq.seq == NULL) {
    return; // no sequence found
  }
  do_spm(seq, 0x03, page, 0);
}

void page_program(struct spm_sequence seq, uint32_t page, const uint8_t *buf) {
  // This function will program a flash page.
  if (seq.seq == NULL) {
    return; // no sequence found
  }
  for (uint16_t i = 0; i < 256; i+=2) {
    uint16_t w = buf[i] | (buf[i+1] << 8);
    do_spm(seq, 0x01, (uint32_t)page + (uint32_t)i, w); // fill temporary buffer
  }
  // do_spm(seq, 0x03, page, 0);
  do_spm(seq, 0x05, page, 0); // write page
}

bool page_verify(uint32_t page, const uint8_t *buf) {
  // This function will verify a flash page.
  // It will return true if the page was verified, false otherwise.
  for (uint16_t i = 0; i < 256; i++) {
    if (pgm_read_byte_far(page + i) != buf[i]) {
      return false; // page verification failed
    }
  }
  return true; // page verified
}