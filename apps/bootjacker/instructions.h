#pragma once


#define is_spm_instruction(inst) ((inst) == 0x95e8)
#define is_out_instruction(inst) (((inst) & 0xf800) == 0xb800)
#define is_sts_instruction(inst) (((inst) & 0xfe0f) == 0x9200)
#define out_instruction_addr(inst) ({\
  uint16_t __inst = (inst); \
  ((uint16_t)((__inst & 0x0600) >> 5) | (__inst & 0x000f));\
})

#define out_instruction_reg(inst) ({\
  uint16_t __inst = (inst); \
  ((uint16_t)((__inst & 0x01f0) >> 4));\
})

#define sts_instruction_reg(inst) ({\
  uint16_t __inst = (inst); \
  ((uint16_t)((__inst & 0x01f0) >> 4));\
})
