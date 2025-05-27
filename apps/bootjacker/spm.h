#pragma once

#include <stdint.h>
#include <stdbool.h>

#define NRWW_START 0xf000
#define RWW_END (NRWW_START - 1)
#define NRWW_END 0xffff

enum spm_sequence_type {
  SPM_SEQUENCE_TYPE_STS_SPMCSR,
  SPM_SEQUENCE_TYPE_OUT_SPMCSR,
};

struct spm_sequence{
  void(*seq)(void);
  enum spm_sequence_type seq_type;
};

void page_erase(struct spm_sequence spm_seq, uint32_t page);
void page_program(struct spm_sequence spm_seq, uint32_t page, const uint8_t *buf);
bool page_verify(uint32_t page, const uint8_t *buf);

void do_spm(struct spm_sequence spm_seq, uint8_t cmd, uint32_t addr, uint16_t optvalue);
uint16_t find_next_spm_instruction(uint16_t pgm_addr);
bool decode_spm_sequence(struct spm_sequence *spm_seq, uint16_t pgm_addr);
