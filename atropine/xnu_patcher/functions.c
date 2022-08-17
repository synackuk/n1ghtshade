#include <libc/libc.h>
#include <include/functions.h>
#include <include/mach.h>

static uint32_t hex2int(char* ptr);
void* pattern_search(const void* addr, int len, int pattern, int mask, int step);
static void* ldr_search_up(const void* start_addr, int len);
static void* ldr32_search_up(const void* start_addr, int len);

static uint32_t bit_range(uint32_t x, int start, int end);
static uint32_t ror(uint32_t x, int places);
static int thumb_expand_imm_c(uint16_t imm12);
static int insn_is_ldr_literal(uint16_t* i);
static int insn_ldr_literal_rt(uint16_t* i);
static int insn_ldr_literal_imm(uint16_t* i);
static int insn_is_add_reg(uint16_t* i);
static int insn_add_reg_rd(uint16_t* i);
static int insn_add_reg_rn(uint16_t* i);
static int insn_add_reg_rm(uint16_t* i);
static int insn_is_movt(uint16_t* i);
static int insn_movt_rd(uint16_t* i);
static int insn_movt_imm(uint16_t* i);
static int insn_is_mov_imm(uint16_t* i);
static int insn_mov_imm_rd(uint16_t* i);
static int insn_mov_imm_imm(uint16_t* i);

struct segment_command *find_segment(struct mach_header *mh, const char *segname) {
	struct load_command *lc;
	struct segment_command *s, *fs = NULL;
	lc = (struct load_command *)((uint32_t)mh + sizeof(struct mach_header));
	while ((uint32_t)lc < (uint32_t)mh + (uint32_t)mh->sizeofcmds) {
		if (lc->cmd == LC_SEGMENT) {
			s = (struct segment_command *)lc;
			if (!strcmp(s->segname, segname)) {
				fs = s;
				break;
			}
		}
		lc = (struct load_command *)((uint32_t)lc + (uint32_t)lc->cmdsize);
	}
	return fs;
}

/* Find start of a load command in a macho */
struct load_command *find_load_command(struct mach_header *mh, uint32_t cmd)
{
	struct load_command *lc, *flc;
	
	lc = (struct load_command *)((uint32_t)mh + sizeof(struct mach_header));
	
	while (1) {
		if ((uint32_t)lc->cmd == cmd) {
			flc = (struct load_command *)(uint32_t)lc;
			break;
		}
		lc = (struct load_command *)((uint32_t)lc + (uint32_t)lc->cmdsize);
	}
	return flc;
}

struct section *find_section(struct segment_command *seg, const char *name) {
	struct section *sect, *fs = NULL;
	uint32_t i = 0;
	for (i = 0, sect = (struct section *)((uint32_t)seg + (uint32_t)sizeof(struct segment_command));
		 i < seg->nsects;
		 i++, sect = (struct section*)((uint32_t)sect + sizeof(struct section))) {
		if (!strcmp(sect->sectname, name)) {
			fs = sect;
			break;
		}
	}
	return fs;
}

/* Find offset of an exported symbol in a macho */
void* find_sym(struct mach_header *mh, const char *name, uint32_t phys_base, uint32_t virt_base) {
	struct segment_command* linkedit = find_segment(mh, SEGMENT_LINKEDIT);
	struct symtab_command* symtab = (struct symtab_command*) find_load_command(mh, LC_SYMTAB);

	uint32_t linkedit_phys = VIRT_TO_PHYS(linkedit->vmaddr);
	
	char* sym_str_table = (char*) (((char*)(linkedit_phys - linkedit->fileoff)) + symtab->stroff);
	struct nlist* sym_table = (struct nlist*)(((char*)(linkedit_phys - linkedit->fileoff)) + symtab->symoff);
	
	for (uint32_t i = 0; i < symtab->nsyms; i++) {
		if (sym_table[i].n_value && !strcmp(name,&sym_str_table[sym_table[i].n_un.n_strx])) {
			return (void*)VIRT_TO_PHYS(sym_table[i].n_value);
		}
	}
	return 0;
}

uint32_t find_version(struct mach_header *mh) {
	struct version_min_command* vers = (struct version_min_command*) find_load_command(mh, LC_VERSION_MIN_IPHONEOS);
	return vers->version;
}

void* find_kext_start(void* kernel_base, uint32_t phys_base, uint32_t virt_base, char* kext_name) {

	/* First we find the PRELINK_INFO section */
	struct segment_command* prelink_info_segment = find_segment(kernel_base, SEGMENT_PRELINK_INFO);
	if(!prelink_info_segment) {
		return NULL;
	}
	void* segment_addr = (void*)VIRT_TO_PHYS(prelink_info_segment->vmaddr);
	/* Next we search down for the start of the XML */
	char* xml_start = memmem(segment_addr, prelink_info_segment->vmsize, "<dict>", sizeof("<dict>") - 1);
	if(!xml_start) {
		return NULL;
	}
	/* Really, really botched parsing, we're looking for the decleration of the kext */
	char* kext_decl = memmem(xml_start, strlen(xml_start), kext_name, strlen(kext_name));
	if(!kext_decl) {
		return NULL;
	}

	/* This contains the _PrelinkExecutableLoadAddr */
	char* load_address_key = memmem(kext_decl, strlen(kext_decl), "_PrelinkExecutableLoadAddr", strlen("_PrelinkExecutableLoadAddr"));
	if(!load_address_key) {
		return NULL;
	}

	/* We search for '0x' as that's the start of the hex string */
	char* load_addr_ascii = memmem(load_address_key, strlen(load_address_key), "0x", strlen("0x"));
	if(!load_addr_ascii) {
		return NULL;
	}
	/* Skip the 0x */
	load_addr_ascii += 2;

	return (void*)hex2int(load_addr_ascii);

}

void* push_lr_search_up(const void* start_addr, int len) {
	/* F0 B5 <-- PUSH LR */
	/* F0 BD <-- POP PC */
	return pattern_search(start_addr, len, 0x0000B580, 0x0000FF80, -2);
}

int insn_is_bne(uint16_t* i) {
	return (*i & 0xFF00) == 0xD100;
}

int insn_is_beq(uint16_t* i) {
	return (*i & 0xFF00) == 0xD000;
}

int insn_is_beqw(uint16_t* i) {
	return ((i[0] & 0xFBC0) == 0xf000) && ((i[1] & 0xD000) == 0x8000);
}

int insn_is_32bit(uint16_t* i) {
	return (*i & 0xe000) == 0xe000 && (*i & 0x1800) != 0x0;
}


/* iBoot32Patcher */

void* ldr_to(const void* loc) {
	uintptr_t xref_target = (uintptr_t)loc;
	uintptr_t i = xref_target;
	uintptr_t min_addr = xref_target - 0x420;
	for(; i > min_addr; i -= 2) {
		i = (uintptr_t)ldr_search_up((void*)i, i - min_addr);
		if (i == 0) {
			break;
		}

		uint32_t dw = *(uint32_t*)i;
		uintptr_t ldr_target = ((i + 4) & ~3) + ((dw & 0xff) << 2);
		if (ldr_target == xref_target) {
			return (void*)i;
		}
	}

	min_addr = xref_target - 0x1000;
	for(i = xref_target; i > min_addr; i -= 4) {
		i = (uintptr_t)ldr32_search_up((void*)i, i - min_addr);
		if (i == 0) {
			break;
		}
		uint32_t dw = *(uint32_t*)i;
		uintptr_t ldr_target = ((i + 4) & ~3) + ((dw >> 16) & 0xfff);
		if (ldr_target == xref_target) {
			return (void*)i;
		}
	}
	return NULL;
}

/* evasi0n patchfinder */

uint16_t* find_literal_ref(uint8_t* kdata, size_t ksize, uint16_t* insn, uintptr_t address) {
	uint16_t* current_instruction = insn;
	uint32_t value[16];
	memset(value, 0, sizeof(value));

	while((uintptr_t)current_instruction < (uintptr_t)(kdata + ksize)) {
		if(insn_is_mov_imm(current_instruction)) {
			value[insn_mov_imm_rd(current_instruction)] = insn_mov_imm_imm(current_instruction);
		} 
		else if(insn_is_ldr_literal(current_instruction)) {
			uintptr_t literal_address  = (uintptr_t)kdata + ((((uintptr_t)current_instruction - (uintptr_t)kdata) + 4) & 0xFFFFFFFC) + insn_ldr_literal_imm(current_instruction);
			if(literal_address >= (uintptr_t)kdata && (literal_address + 4) <= ((uintptr_t)kdata + ksize)) {
				value[insn_ldr_literal_rt(current_instruction)] = *(uint32_t*)(literal_address);
			}
		}
		else if(insn_is_movt(current_instruction)) {
			value[insn_movt_rd(current_instruction)] |= insn_movt_imm(current_instruction) << 16;
		} 
		else if(insn_is_add_reg(current_instruction)) {
			int reg = insn_add_reg_rd(current_instruction);
			if(insn_add_reg_rm(current_instruction) == 15 && insn_add_reg_rn(current_instruction) == reg) {
				value[reg] += ((uintptr_t)current_instruction - (uintptr_t)kdata) + 4;
				if(value[reg] == address) {
					return current_instruction;
				}
			}
		}

		current_instruction += insn_is_32bit(current_instruction) ? 2 : 1;
	}

	return NULL;
}


/* Convert a hex string to uint32_t */
static uint32_t hex2int(char* ptr) {
	uint32_t val = 0;
	int shift = 0;
	while (*ptr && shift < 32) {

		uint8_t byte = *ptr; 
		/* Get the 4 bit character, depending on if it's 0 - 10 or a - f*/
		if (byte >= '0' && byte <= '9'){
			byte = byte - '0';
		}
		else if (byte >= 'a' && byte <='f'){
			byte = byte - 'a' + 10;
		}
		else {
			/* We're done with the hex */
			break;
		}

		/* Shift to the next nibble */
		val *= 16;

		/* Get half of the byte (as we're reading in 4 bit increments) */
		val += byte & 0xF;
		shift += 4;
		ptr += 1;
	}
	return val;
}

void* pattern_search(const void* addr, int len, int pattern, int mask, int step) {
	char* caddr = (char*)addr;
	if (len <= 0)
		return NULL;
	if (step < 0) {
		len = -len;
		len &= ~-(step+1);
	} else {
		len &= ~(step-1);
	}
	for (int i = 0; i != len; i += step) {
		uint32_t x = *(uint32_t*)(caddr + i);
		if ((x & mask) == pattern)
			return (void*)(caddr + i);
	}
	return NULL;
}

static void* ldr_search_up(const void* start_addr, int len) {
	return pattern_search(start_addr, len, 0x00004800, 0x0000F800, -1);
}

static void* ldr32_search_up(const void* start_addr, int len) {
	return pattern_search(start_addr, len, 0x0000F8DF, 0x0000FFFF, -1);
}

static uint32_t bit_range(uint32_t x, int start, int end) {
	x = (x << (31 - start)) >> (31 - start);
	x = (x >> end);
	return x;
}

static uint32_t ror(uint32_t x, int places) {
	return (x >> places) | (x << (32 - places));
}

static int thumb_expand_imm_c(uint16_t imm12) {
	if(bit_range(imm12, 11, 10) == 0) {
		switch(bit_range(imm12, 9, 8)) {
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
	} 
	else {
		uint32_t unrotated_value = 0x80 | bit_range(imm12, 6, 0);
		return ror(unrotated_value, bit_range(imm12, 11, 7));
	}
}

static int insn_is_ldr_literal(uint16_t* i) {
	return (*i & 0xF800) == 0x4800 || (*i & 0xFF7F) == 0xF85F;
}

static int insn_ldr_literal_rt(uint16_t* i) {
	if((*i & 0xF800) == 0x4800) {
		return (*i >> 8) & 7;
	}
	else if((*i & 0xFF7F) == 0xF85F) {
		return (*(i + 1) >> 12) & 0xF;
	}
	return 0;
}

static int insn_ldr_literal_imm(uint16_t* i) {
	if((*i & 0xF800) == 0x4800) {
		return (*i & 0xFF) << 2;
	}
	else if((*i & 0xFF7F) == 0xF85F) {
		return (*(i + 1) & 0xFFF) * (((*i & 0x0800) == 0x0800) ? 1 : -1);
	}
	return 0;
}

static int insn_is_add_reg(uint16_t* i) {
	if((*i & 0xFE00) == 0x1800) {
		return 1;
	}
	else if((*i & 0xFF00) == 0x4400) {
		return 1;
	}
	else if((*i & 0xFFE0) == 0xEB00) {
		return 1;
	}
	return 0;
}

static int insn_add_reg_rd(uint16_t* i) {
	if((*i & 0xFE00) == 0x1800) {
		return (*i & 7);
	}
	else if((*i & 0xFF00) == 0x4400) {
		return (*i & 7) | ((*i & 0x80) >> 4);
	}
	else if((*i & 0xFFE0) == 0xEB00) {
		return (*(i + 1) >> 8) & 0xF;
	}
	return 0;
}

static int insn_add_reg_rn(uint16_t* i) {
	if((*i & 0xFE00) == 0x1800) {
		return ((*i >> 3) & 7);
	}
	else if((*i & 0xFF00) == 0x4400) {
		return (*i & 7) | ((*i & 0x80) >> 4);
	}
	else if((*i & 0xFFE0) == 0xEB00) {
		return (*i & 0xF);
	}
	return 0;
}

static int insn_add_reg_rm(uint16_t* i) {
	if((*i & 0xFE00) == 0x1800) {
		return (*i >> 6) & 7;
	}
	else if((*i & 0xFF00) == 0x4400) {
		return (*i >> 3) & 0xF;
	}
	else if((*i & 0xFFE0) == 0xEB00) {
		return *(i + 1) & 0xF;
	}
	return 0;
}

static int insn_is_movt(uint16_t* i) {
	return (*i & 0xFBF0) == 0xF2C0 && (*(i + 1) & 0x8000) == 0;
}

static int insn_movt_rd(uint16_t* i) {
	return (*(i + 1) >> 8) & 0xF;
}

static int insn_movt_imm(uint16_t* i) {
	return ((*i & 0xF) << 12) | ((*i & 0x0400) << 1) | ((*(i + 1) & 0x7000) >> 4) | (*(i + 1) & 0xFF);
}

static int insn_is_mov_imm(uint16_t* i) {
	if((*i & 0xF800) == 0x2000) {
		return 1;
	}
	else if((*i & 0xFBEF) == 0xF04F && (*(i + 1) & 0x8000) == 0) {
		return 1;
	}
	else if((*i & 0xFBF0) == 0xF240 && (*(i + 1) & 0x8000) == 0) {
		return 1;
	}
	return 0;
}

static int insn_mov_imm_rd(uint16_t* i) {
	if((*i & 0xF800) == 0x2000) {
		return (*i >> 8) & 7;
	}
	else if((*i & 0xFBEF) == 0xF04F && (*(i + 1) & 0x8000) == 0) {
		return (*(i + 1) >> 8) & 0xF;
	}
	else if((*i & 0xFBF0) == 0xF240 && (*(i + 1) & 0x8000) == 0) {
		return (*(i + 1) >> 8) & 0xF;
	}
	return 0;
}

static int insn_mov_imm_imm(uint16_t* i) {
	if((*i & 0xF800) == 0x2000) {
		return *i & 0xF;
	}
	else if((*i & 0xFBEF) == 0xF04F && (*(i + 1) & 0x8000) == 0) {
		return thumb_expand_imm_c(((*i & 0x0400) << 1) | ((*(i + 1) & 0x7000) >> 4) | (*(i + 1) & 0xFF));
	}
	else if((*i & 0xFBF0) == 0xF240 && (*(i + 1) & 0x8000) == 0) {
		return ((*i & 0xF) << 12) | ((*i & 0x0400) << 1) | ((*(i + 1) & 0x7000) >> 4) | (*(i + 1) & 0xFF);
	}
	return 0;
}
