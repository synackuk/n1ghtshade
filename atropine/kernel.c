#include <common.h>
#include <include/functions.h>

#define BX_LR 0x4770

static uint32_t bit_range(uint32_t x, int start, int end)
{
	x = (x << (31 - start)) >> (31 - start);
	x = (x >> end);
	return x;
}

static uint32_t ror(uint32_t x, int places)
{
	return (x >> places) | (x << (32 - places));
}

static int thumb_expand_imm_c(uint16_t imm12)
{
	if(bit_range(imm12, 11, 10) == 0)
	{
		switch(bit_range(imm12, 9, 8))
		{
			case 0:
				return bit_range(imm12, 7, 0);
			case 1:
				return (bit_range(imm12, 7, 0) << 16) | bit_range(imm12, 7, 0);
			case 2:
				return (bit_range(imm12, 7, 0) << 24) | (bit_range(imm12, 7, 0) << 8);
			case 3:
				return (bit_range(imm12, 7, 0) << 24) | (bit_range(imm12, 7, 0) << 16) | (bit_range(imm12, 7, 0) << 8) | bit_range(imm12, 7, 0);
			default:
				return 0;
		}
	} else
	{
		uint32_t unrotated_value = 0x80 | bit_range(imm12, 6, 0);
		return ror(unrotated_value, bit_range(imm12, 11, 7));
	}
}

static int insn_is_32bit(uint16_t* i)
{
	return (*i & 0xe000) == 0xe000 && (*i & 0x1800) != 0x0;
}

static int insn_is_bl(uint16_t* i)
{
	if((*i & 0xf800) == 0xf000 && (*(i + 1) & 0xd000) == 0xd000)
		return 1;
	else if((*i & 0xf800) == 0xf000 && (*(i + 1) & 0xd001) == 0xc000)
		return 1;
	else
		return 0;
}

static uint32_t insn_bl_imm32(uint16_t* i)
{
	uint16_t insn0 = *i;
	uint16_t insn1 = *(i + 1);
	uint32_t s = (insn0 >> 10) & 1;
	uint32_t j1 = (insn1 >> 13) & 1;
	uint32_t j2 = (insn1 >> 11) & 1;
	uint32_t i1 = ~(j1 ^ s) & 1;
	uint32_t i2 = ~(j2 ^ s) & 1;
	uint32_t imm10 = insn0 & 0x3ff;
	uint32_t imm11 = insn1 & 0x7ff;
	uint32_t imm32 = (imm11 << 1) | (imm10 << 12) | (i2 << 22) | (i1 << 23) | (s ? 0xff000000 : 0);
	return imm32;
}

static int insn_is_b_conditional(uint16_t* i)
{
	return (*i & 0xF000) == 0xD000 && (*i & 0x0F00) != 0x0F00 && (*i & 0x0F00) != 0xE;
}

static int insn_is_b_unconditional(uint16_t* i)
{
	if((*i & 0xF800) == 0xE000)
		return 1;
	else if((*i & 0xF800) == 0xF000 && (*(i + 1) & 0xD000) == 9)
		return 1;
	else
		return 0;
}

static int insn_is_ldr_literal(uint16_t* i)
{
	return (*i & 0xF800) == 0x4800 || (*i & 0xFF7F) == 0xF85F;
}

static int insn_ldr_literal_rt(uint16_t* i)
{
	if((*i & 0xF800) == 0x4800)
		return (*i >> 8) & 7;
	else if((*i & 0xFF7F) == 0xF85F)
		return (*(i + 1) >> 12) & 0xF;
	else
		return 0;
}

static int insn_ldr_literal_imm(uint16_t* i)
{
	if((*i & 0xF800) == 0x4800)
		return (*i & 0xF) << 2;
	else if((*i & 0xFF7F) == 0xF85F)
		return (*(i + 1) & 0xFFF) * (((*i & 0x0800) == 0x0800) ? 1 : -1);
	else
		return 0;
}

// TODO: More encodings
static int insn_is_ldr_imm(uint16_t* i)
{
	uint8_t opA = bit_range(*i, 15, 12);
	uint8_t opB = bit_range(*i, 11, 9);

	return opA == 6 && (opB & 4) == 4;
}

static int insn_ldr_imm_rt(uint16_t* i)
{
	return (*i & 7);
}

static int insn_ldr_imm_rn(uint16_t* i)
{
	return ((*i >> 3) & 7);
}

static int insn_ldr_imm_imm(uint16_t* i)
{
	return ((*i >> 6) & 0x1F);
}

// TODO: More encodings
static int insn_is_ldrb_imm(uint16_t* i)
{
	return (*i & 0xF800) == 0x7800;
}

static int insn_ldrb_imm_rt(uint16_t* i)
{
	return (*i & 7);
}

static int insn_ldrb_imm_rn(uint16_t* i)
{
	return ((*i >> 3) & 7);
}

static int insn_ldrb_imm_imm(uint16_t* i)
{
	return ((*i >> 6) & 0x1F);
}

static int insn_is_ldr_reg(uint16_t* i)
{
	if((*i & 0xFE00) == 0x5800)
		return 1;
	else if((*i & 0xFFF0) == 0xF850 && (*(i + 1) & 0x0FC0) == 0x0000)
		return 1;
	else
		return 0;
}

static int insn_ldr_reg_rn(uint16_t* i)
{
	if((*i & 0xFE00) == 0x5800)
		return (*i >> 3) & 0x7;
	else if((*i & 0xFFF0) == 0xF850 && (*(i + 1) & 0x0FC0) == 0x0000)
		return (*i & 0xF);
	else
		return 0;
}

int insn_ldr_reg_rt(uint16_t* i)
{
	if((*i & 0xFE00) == 0x5800)
		return *i & 0x7;
	else if((*i & 0xFFF0) == 0xF850 && (*(i + 1) & 0x0FC0) == 0x0000)
		return (*(i + 1) >> 12) & 0xF;
	else
		return 0;
}

int insn_ldr_reg_rm(uint16_t* i)
{
	if((*i & 0xFE00) == 0x5800)
		return (*i >> 6) & 0x7;
	else if((*i & 0xFFF0) == 0xF850 && (*(i + 1) & 0x0FC0) == 0x0000)
		return *(i + 1) & 0xF;
	else
		return 0;
}

static int insn_ldr_reg_lsl(uint16_t* i)
{
	if((*i & 0xFE00) == 0x5800)
		return 0;
	else if((*i & 0xFFF0) == 0xF850 && (*(i + 1) & 0x0FC0) == 0x0000)
		return (*(i + 1) >> 4) & 0x3;
	else
		return 0;
}

static int insn_is_add_reg(uint16_t* i)
{
	if((*i & 0xFE00) == 0x1800)
		return 1;
	else if((*i & 0xFF00) == 0x4400)
		return 1;
	else if((*i & 0xFFE0) == 0xEB00)
		return 1;
	else
		return 0;
}

static int insn_add_reg_rd(uint16_t* i)
{
	if((*i & 0xFE00) == 0x1800)
		return (*i & 7);
	else if((*i & 0xFF00) == 0x4400)
		return (*i & 7) | ((*i & 0x80) >> 4) ;
	else if((*i & 0xFFE0) == 0xEB00)
		return (*(i + 1) >> 8) & 0xF;
	else
		return 0;
}

static int insn_add_reg_rn(uint16_t* i)
{
	if((*i & 0xFE00) == 0x1800)
		return ((*i >> 3) & 7);
	else if((*i & 0xFF00) == 0x4400)
		return (*i & 7) | ((*i & 0x80) >> 4) ;
	else if((*i & 0xFFE0) == 0xEB00)
		return (*i & 0xF);
	else
		return 0;
}

static int insn_add_reg_rm(uint16_t* i)
{
	if((*i & 0xFE00) == 0x1800)
		return (*i >> 6) & 7;
	else if((*i & 0xFF00) == 0x4400)
		return (*i >> 3) & 0xF;
	else if((*i & 0xFFE0) == 0xEB00)
		return *(i + 1) & 0xF;
	else
		return 0;
}

static int insn_is_movt(uint16_t* i)
{
	return (*i & 0xFBF0) == 0xF2C0 && (*(i + 1) & 0x8000) == 0;
}

static int insn_movt_rd(uint16_t* i)
{
	return (*(i + 1) >> 8) & 0xF;
}

static int insn_movt_imm(uint16_t* i)
{
	return ((*i & 0xF) << 12) | ((*i & 0x0400) << 1) | ((*(i + 1) & 0x7000) >> 4) | (*(i + 1) & 0xFF);
}

static int insn_is_mov_imm(uint16_t* i)
{
	if((*i & 0xF800) == 0x2000)
		return 1;
	else if((*i & 0xFBEF) == 0xF04F && (*(i + 1) & 0x8000) == 0)
		return 1;
	else if((*i & 0xFBF0) == 0xF240 && (*(i + 1) & 0x8000) == 0)
		return 1;
	else
		return 0;
}

static int insn_mov_imm_rd(uint16_t* i)
{
	if((*i & 0xF800) == 0x2000)
		return (*i >> 8) & 7;
	else if((*i & 0xFBEF) == 0xF04F && (*(i + 1) & 0x8000) == 0)
		return (*(i + 1) >> 8) & 0xF;
	else if((*i & 0xFBF0) == 0xF240 && (*(i + 1) & 0x8000) == 0)
		return (*(i + 1) >> 8) & 0xF;
	else
		return 0;
}

static int insn_mov_imm_imm(uint16_t* i)
{
	if((*i & 0xF800) == 0x2000)
		return *i & 0xF;
	else if((*i & 0xFBEF) == 0xF04F && (*(i + 1) & 0x8000) == 0)
		return thumb_expand_imm_c(((*i & 0x0400) << 1) | ((*(i + 1) & 0x7000) >> 4) | (*(i + 1) & 0xFF));
	else if((*i & 0xFBF0) == 0xF240 && (*(i + 1) & 0x8000) == 0)
		return ((*i & 0xF) << 12) | ((*i & 0x0400) << 1) | ((*(i + 1) & 0x7000) >> 4) | (*(i + 1) & 0xFF);
	else
		return 0;
}

static int insn_is_cmp_imm(uint16_t* i)
{
	if((*i & 0xF800) == 0x2800)
		return 1;
	else if((*i & 0xFBF0) == 0xF1B0 && (*(i + 1) & 0x8F00) == 0x0F00)
		return 1;
	else
		return 0;
}

static int insn_cmp_imm_rn(uint16_t* i)
{
	if((*i & 0xF800) == 0x2800)
		return (*i >> 8) & 7;
	else if((*i & 0xFBF0) == 0xF1B0 && (*(i + 1) & 0x8F00) == 0x0F00)
		return *i & 0xF;
	else
		return 0;
}

static int insn_cmp_imm_imm(uint16_t* i)
{
	if((*i & 0xF800) == 0x2800)
		return *i & 0xFF;
	else if((*i & 0xFBF0) == 0xF1B0 && (*(i + 1) & 0x8F00) == 0x0F00)
		return thumb_expand_imm_c(((*i & 0x0400) << 1) | ((*(i + 1) & 0x7000) >> 4) | (*(i + 1) & 0xFF));
	else
		return 0;
}

static int insn_is_and_imm(uint16_t* i)
{
	return (*i & 0xFBE0) == 0xF000 && (*(i + 1) & 0x8000) == 0;
}

static int insn_and_imm_rn(uint16_t* i)
{
	return *i & 0xF;
}

static int insn_and_imm_rd(uint16_t* i)
{
	return (*(i + 1) >> 8) & 0xF;
}

static int insn_and_imm_imm(uint16_t* i)
{
	return thumb_expand_imm_c(((*i & 0x0400) << 1) | ((*(i + 1) & 0x7000) >> 4) | (*(i + 1) & 0xFF));
}

static int insn_is_push(uint16_t* i)
{
	if((*i & 0xFE00) == 0xB400)
		return 1;
	else if(*i == 0xE92D)
		return 1;
	else if(*i == 0xF84D && (*(i + 1) & 0x0FFF) == 0x0D04)
		return 1;
	else
		return 0;
}

static int insn_push_registers(uint16_t* i)
{
	if((*i & 0xFE00) == 0xB400)
		return (*i & 0x00FF) | ((*i & 0x0100) << 6);
	else if(*i == 0xE92D)
		return *(i + 1);
	else if(*i == 0xF84D && (*(i + 1) & 0x0FFF) == 0x0D04)
		return 1 << ((*(i + 1) >> 12) & 0xF);
	else
		return 0;
}

static int insn_is_preamble_push(uint16_t* i)
{
	return insn_is_push(i) && (insn_push_registers(i) & (1 << 14)) != 0;
}

static int insn_is_str_imm(uint16_t* i)
{
	if((*i & 0xF800) == 0x6000)
		return 1;
	else if((*i & 0xF800) == 0x9000)
		return 1;
	else if((*i & 0xFFF0) == 0xF8C0)
		return 1;
	else if((*i & 0xFFF0) == 0xF840 && (*(i + 1) & 0x0800) == 0x0800)
		return 1;
	else
		return 0;
}

static int insn_str_imm_postindexed(uint16_t* i)
{
	if((*i & 0xF800) == 0x6000)
		return 1;
	else if((*i & 0xF800) == 0x9000)
		return 1;
	else if((*i & 0xFFF0) == 0xF8C0)
		return 1;
	else if((*i & 0xFFF0) == 0xF840 && (*(i + 1) & 0x0800) == 0x0800)
		return (*(i + 1) >> 10) & 1;
	else
		return 0;
}

static int insn_str_imm_wback(uint16_t* i)
{
	if((*i & 0xF800) == 0x6000)
		return 0;
	else if((*i & 0xF800) == 0x9000)
		return 0;
	else if((*i & 0xFFF0) == 0xF8C0)
		return 0;
	else if((*i & 0xFFF0) == 0xF840 && (*(i + 1) & 0x0800) == 0x0800)
		return (*(i + 1) >> 8) & 1;
	else
		return 0;
}

static int insn_str_imm_imm(uint16_t* i)
{
	if((*i & 0xF800) == 0x6000)
		return (*i & 0x07C0) >> 4;
	else if((*i & 0xF800) == 0x9000)
		return (*i & 0xFF) << 2;
	else if((*i & 0xFFF0) == 0xF8C0)
		return (*(i + 1) & 0xFFF);
	else if((*i & 0xFFF0) == 0xF840 && (*(i + 1) & 0x0800) == 0x0800)
		return (*(i + 1) & 0xFF);
	else
		return 0;
}

static int insn_str_imm_rt(uint16_t* i)
{
	if((*i & 0xF800) == 0x6000)
		return (*i & 7);
	else if((*i & 0xF800) == 0x9000)
		return (*i >> 8) & 7;
	else if((*i & 0xFFF0) == 0xF8C0)
		return (*(i + 1) >> 12) & 0xF;
	else if((*i & 0xFFF0) == 0xF840 && (*(i + 1) & 0x0800) == 0x0800)
		return (*(i + 1) >> 12) & 0xF;
	else
		return 0;
}

static int insn_str_imm_rn(uint16_t* i)
{
	if((*i & 0xF800) == 0x6000)
		return (*i >> 3) & 7;
	else if((*i & 0xF800) == 0x9000)
		return 13;
	else if((*i & 0xFFF0) == 0xF8C0)
		return (*i & 0xF);
	else if((*i & 0xFFF0) == 0xF840 && (*(i + 1) & 0x0800) == 0x0800)
		return (*i & 0xF);
	else
		return 0;
}

// Given an instruction, search backwards until an instruction is found matching the specified criterion.
static uint16_t* find_last_insn_matching(uint32_t region, uint8_t* kdata, size_t ksize, uint16_t* current_instruction, int (*match_func)(uint16_t*))
{
	while((uintptr_t)current_instruction > (uintptr_t)kdata)
	{
		if(insn_is_32bit(current_instruction - 2) && !insn_is_32bit(current_instruction - 3))
		{
			current_instruction -= 2;
		} else
		{
			--current_instruction;
		}

		if(match_func(current_instruction))
		{
			return current_instruction;
		}
	}

	return NULL;
}

// Given an instruction and a register, find the PC-relative address that was stored inside the register by the time the instruction was reached.
static uint32_t find_pc_rel_value(uint32_t region, uint8_t* kdata, size_t ksize, uint16_t* insn, int reg)
{
	// Find the last instruction that completely wiped out this register
	int found = 0;
	uint16_t* current_instruction = insn;
	while((uintptr_t)current_instruction > (uintptr_t)kdata)
	{
		if(insn_is_32bit(current_instruction - 2))
		{
			current_instruction -= 2;
		} else
		{
			--current_instruction;
		}

		if(insn_is_mov_imm(current_instruction) && insn_mov_imm_rd(current_instruction) == reg)
		{
			found = 1;
			break;
		}

		if(insn_is_ldr_literal(current_instruction) && insn_ldr_literal_rt(current_instruction) == reg)
		{
			found = 1;
			break;
		}
	}

	if(!found)
		return 0;

	// Step through instructions, executing them as a virtual machine, only caring about instructions that affect the target register and are commonly used for PC-relative addressing.
	uint32_t value = 0;
	while((uintptr_t)current_instruction < (uintptr_t)insn)
	{
		if(insn_is_mov_imm(current_instruction) && insn_mov_imm_rd(current_instruction) == reg)
		{
			value = insn_mov_imm_imm(current_instruction);
		} else if(insn_is_ldr_literal(current_instruction) && insn_ldr_literal_rt(current_instruction) == reg)
		{
			value = *(uint32_t*)(kdata + (((((uintptr_t)current_instruction - (uintptr_t)kdata) + 4) & 0xFFFFFFFC) + insn_ldr_literal_imm(current_instruction)));
		} else if(insn_is_movt(current_instruction) && insn_movt_rd(current_instruction) == reg)
		{
			value |= insn_movt_imm(current_instruction) << 16;
		} else if(insn_is_add_reg(current_instruction) && insn_add_reg_rd(current_instruction) == reg)
		{
			if(insn_add_reg_rm(current_instruction) != 15 || insn_add_reg_rn(current_instruction) != reg)
			{
				// Can't handle this kind of operation!
				return 0;
			}

			value += ((uintptr_t)current_instruction - (uintptr_t)kdata) + 4;
		}

		current_instruction += insn_is_32bit(current_instruction) ? 2 : 1;
	}

	return value;
}

// Find PC-relative references to a certain address (relative to kdata). This is basically a virtual machine that only cares about instructions used in PC-relative addressing, so no branches, etc.
static uint16_t* find_literal_ref(uint32_t region, uint8_t* kdata, size_t ksize, uint16_t* insn, uint32_t address)
{
	uint16_t* current_instruction = insn;
	uint32_t value[16];
	memset(value, 0, sizeof(value));

	while((uintptr_t)current_instruction < (uintptr_t)(kdata + ksize))
	{
		if(insn_is_mov_imm(current_instruction))
		{
			value[insn_mov_imm_rd(current_instruction)] = insn_mov_imm_imm(current_instruction);
		} else if(insn_is_ldr_literal(current_instruction))
		{
			uintptr_t literal_address  = (uintptr_t)kdata + ((((uintptr_t)current_instruction - (uintptr_t)kdata) + 4) & 0xFFFFFFFC) + insn_ldr_literal_imm(current_instruction);
			if(literal_address >= (uintptr_t)kdata && (literal_address + 4) <= ((uintptr_t)kdata + ksize))
			{
				value[insn_ldr_literal_rt(current_instruction)] = *(uint32_t*)(literal_address);
			}
		} else if(insn_is_movt(current_instruction))
		{
			value[insn_movt_rd(current_instruction)] |= insn_movt_imm(current_instruction) << 16;
		} else if(insn_is_add_reg(current_instruction))
		{
			int reg = insn_add_reg_rd(current_instruction);
			if(insn_add_reg_rm(current_instruction) == 15 && insn_add_reg_rn(current_instruction) == reg)
			{
				value[reg] += ((uintptr_t)current_instruction - (uintptr_t)kdata) + 4;
				if(value[reg] == address)
				{
					return current_instruction;
				}
			}
		}

		current_instruction += insn_is_32bit(current_instruction) ? 2 : 1;
	}

	return NULL;
}

/* Find start of a load command in a macho */
static struct load_command *find_load_command(struct mach_header *mh, uint32_t cmd)
{
	struct load_command *lc, *flc;
	
	lc = (struct load_command *)((uintptr_t)mh + sizeof(struct mach_header));
	
	while (1) {
		if ((uintptr_t)lc->cmd == cmd) {
			flc = (struct load_command *)(uintptr_t)lc;
			break;
		}
		lc = (struct load_command *)((uintptr_t)lc + (uintptr_t)lc->cmdsize);
	}
	return flc;
}

/* Find offset of an exported symbol in a macho */
static void* find_sym(struct mach_header *mh, const char *name) {
	struct segment_command* first = (struct segment_command*) find_load_command(mh, LC_SEGMENT);
	struct symtab_command* symtab = (struct symtab_command*) find_load_command(mh, LC_SYMTAB);
	vm_address_t vmaddr_slide = (vm_address_t)mh - (vm_address_t)first->vmaddr;
	
	char* sym_str_table = (char*) (((char*)mh) + symtab->stroff);
	struct nlist* sym_table = (struct nlist*)(((char*)mh) + symtab->symoff);
	
	for (int i = 0; i < symtab->nsyms; i++) {
		if (sym_table[i].n_value && !strcmp(name,&sym_str_table[sym_table[i].n_un.n_strx])) {
			return (void*)(uintptr_t)(sym_table[i].n_value + vmaddr_slide);
		}
	}
	return 0;
}

int patch_proc_enforce(char* address) {
	// Find the description.
	uint8_t* proc_enforce_description = memmem(address, KERNEL_LEN, "Enforce MAC policy on process operations", sizeof("Enforce MAC policy on process operations"));
	if(!proc_enforce_description)
		return -1;
	// Find what references the description.
	uint32_t proc_enforce_description_address = KERNEL_BASE_ADDRESS + ((uintptr_t)proc_enforce_description - (uintptr_t)address);
	uint8_t* proc_enforce_description_ptr = memmem(address, KERNEL_LEN, &proc_enforce_description_address, sizeof(proc_enforce_description_address));
	if(!proc_enforce_description_ptr)
		return -1;
	// Go up the struct to find the pointer to the actual data element.
	uint32_t* proc_enforce_address = (uint32_t*)(proc_enforce_description_ptr - (5 * sizeof(uint32_t)));

	uint32_t proc_enforce_offset = *proc_enforce_address - (uint32_t)KERNEL_BASE_ADDRESS;

	uint32_t* proc_enforce = (uint32_t*)(proc_enforce_offset + (uint32_t)address);

	*(uint32_t*)proc_enforce = 0;
	return 0;

}

int patch_mount(char* address) {
	char* patch = memmem(address, KERNEL_LEN, "\x10\xf0\x01\x0f\x14\xd0\xd6\xf8", 8); // iOS 7 find bytes
	if(!patch) {
		return 0;
	}
	patch += 5;
	*(uint8_t*)patch = 0xe0;
	return 0;
}

int patch_amfi(char* address) {
	uint32_t memcmp_func = (uint32_t)find_sym((void*)address, "_memcmp");
	if(!memcmp_func) {
		return -1;
	}
	memcmp_func -= (uint32_t)address;

	uint32_t memcmp_address = (memcmp_func) + KERNEL_BASE_ADDRESS + 1;
	uint32_t mach_msg_rpc_from_kernel_proper_func = (uint32_t)find_sym((void*)address, "_mach_msg_rpc_from_kernel_proper");
	if(!mach_msg_rpc_from_kernel_proper_func) {
		return -1;
	}
	mach_msg_rpc_from_kernel_proper_func -= (uint32_t)address;
	uint32_t mach_msg_rpc_from_kernel_proper_address = (mach_msg_rpc_from_kernel_proper_func) + KERNEL_BASE_ADDRESS + 1;	
	uint32_t bx_lr_gadget = 0;
	for(uint32_t i = 0; i < KERNEL_LEN; i++) {
		if(*(uint32_t*)&address[i] == 0x47702000) {
			bx_lr_gadget = i + KERNEL_BASE_ADDRESS + 1;
			break;
		}
	}
	if(!bx_lr_gadget) {
		return -1;
	}

	uint32_t* overwrite = 0;
	for(uint32_t i = 0; i < KERNEL_LEN; i++) {
		if(*(uint32_t*)&address[i] == mach_msg_rpc_from_kernel_proper_address) {
			if(*(uint32_t*)&address[i + sizeof(uint32_t)] == memcmp_address) {
				overwrite = (uint32_t*)(address + i + sizeof(uint32_t));
				break;
			}
		}
	}
	if(!overwrite) {
		return -1;
	}

	*(uint32_t*)overwrite = bx_lr_gadget;
	return 0;
}

int patch_sandbox(char* address) {
	uint32_t* mac_label_get_ptr = find_sym((void*)address, "_mac_label_get");
	if(!mac_label_get_ptr) {
		return -1;
	}
	*(uint32_t*)mac_label_get_ptr = 0x47702000;
	return 0;
}

int patch_i_can_has_debugger(char* address) {
	uint32_t* pe_i_can_has_debugger_func = (uint32_t*)find_sym((void*)address, "_PE_i_can_has_debugger");
	if(!pe_i_can_has_debugger_func) {
		return -1;
	}

	int i = 0;
	uint32_t i_can_has_debugger_offset = 0;

	while(i < 0x100) {
		uint16_t* insn = (uint16_t*)&pe_i_can_has_debugger_func[i];
		if(insn_is_ldr_imm(insn)) {
			i_can_has_debugger_offset = (uint32_t)find_pc_rel_value(KERNEL_BASE_ADDRESS, address, KERNEL_LEN, insn, insn_ldr_reg_rn(insn));
			if(i_can_has_debugger_offset) {
				break;
			}
		}
		i += insn_is_32bit(insn) ? 2 : 1;
	}

	if(!i_can_has_debugger_offset) {
		return -1;
	}

	uint32_t* i_can_has_debugger = (uint32_t*)((uint32_t)i_can_has_debugger_offset + (uint32_t)address);

	*(uint32_t*)i_can_has_debugger = 1;
	return 0;
}

int patch_kernel(char* address) {
	int ret;

	log("Patching proc_enforce\n");
	ret = patch_proc_enforce(address);
	if(ret != 0) {
		return -1;
	}

	log("Patching mount\n");
	ret = patch_mount(address);
	if(ret != 0) {
		return -1;
	}

	log("Patching amfi\n");
	ret = patch_amfi(address);
	if(ret != 0) {
		return -1;
	}

	log("Patching sandbox\n");
	ret = patch_sandbox(address);
	if(ret != 0) {
		return -1;
	}

	log("Patching i_can_has_debugger\n");
	ret = patch_i_can_has_debugger(address);
	if(ret != 0) {
		return -1;
	}
	
	return 0;
}

void hooker(void* func_address, void* address) {
	int ret;
	log("Patching kernel\n");
	ret = patch_kernel(address);
	if(ret != 0) {
		log("hanging here.");
		while(1) {}
	}
	log("Booting...\n");
	return;
}

void hook_kernel() {
	void* hook = &hooker;
	memcpy(kern_load_target, &hook, 4);
	void* target = ldr_to(kern_load_target);
	if(!target) {
		return;
	}
	void* bl = bl_search_down(target, 8);
	if(!bl) {
		return;
	}
	memcpy(bl, "\x80\x47\x00\xbf", 4);
}