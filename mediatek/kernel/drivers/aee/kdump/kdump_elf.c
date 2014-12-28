/* 
 * (C) Copyright 2010
 * MediaTek <www.MediaTek.com>
 *
 * Android Exception Device
 *
 */
#include <linux/types.h>

#include <linux/elf.h>
#include <linux/elfcore.h>
#include <linux/mm.h>
#include <linux/ptrace.h>
#include <linux/fb.h>
#include <linux/stacktrace.h>
#include <linux/kallsyms.h>
#include <asm/stacktrace.h>
#include <asm/traps.h>
#include <linux/memory.h>
#include "kdump_elf.h"

/* An ELF note in memory */
struct memelfnote
{
	const char *name;
	int type;
	unsigned int datasz;
	void *data;
};

/*****************************************************************************/
/*
 * determine size of ELF note
 */
static int notesize(struct memelfnote *en)
{
	int sz;

	sz = sizeof(struct elf_note);
	sz += roundup((strlen(en->name) + 1), 4);
	sz += roundup(en->datasz, 4);

	return sz;
} /* end notesize() */

/*****************************************************************************/
/*
 * store a note in the header buffer
 */
static char *storenote(struct memelfnote *men, char *bufp)
{
	struct elf_note en;

#define DUMP_WRITE(addr,nr) do { memcpy(bufp,addr,nr); bufp += nr; } while(0)

	en.n_namesz = strlen(men->name) + 1;
	en.n_descsz = men->datasz;
	en.n_type = men->type;

	DUMP_WRITE(&en, sizeof(en));
	DUMP_WRITE(men->name, en.n_namesz);

	/* XXX - cast from long long to long to avoid need for libgcc.a */
	bufp = (char*) roundup((unsigned long)bufp,4);
	DUMP_WRITE(men->data, men->datasz);
	bufp = (char*) roundup((unsigned long)bufp,4);

#undef DUMP_WRITE

	return bufp;
} /* end storenote() */

static void fill_regs(elf_gregset_t *pr_reg) 
{
	struct pt_regs regs;
	asm volatile(
		"stmia %0, {r0 - r15}\n\t"
		:
		: "r" (&regs)
		: "memory"
		);
	elf_core_copy_regs(pr_reg, &regs);
}

int unwind_frame_fix(struct stackframe *frame)
{
	int ret = unwind_frame(frame);
	if (ret == 0)
	{
		frame->lr = frame->pc;
		if (frame->fp != 0)
			frame->pc = *((unsigned long *)(frame->fp));
	}
	return ret;
}

int find_exception_stack(struct stackframe *frame)
{
	/* fill current info */
	struct stackframe current_stk;
	register unsigned long current_sp asm("sp");
	char ksym[KSYM_SYMBOL_LEN], ksymlr[KSYM_SYMBOL_LEN];
	frame->fp = (unsigned long)__builtin_frame_address(0);
	frame->sp = current_sp;
	frame->lr = (unsigned long)__builtin_return_address(0);
	frame->pc = (unsigned long)find_exception_stack;
	
	/* start unwinding ... */
	printk("Unwinding stack ...\n");
	printk("Outer frame: fp=%08lx, pc=%08lx\n", frame->fp, frame->pc);
	memcpy(&current_stk, frame, sizeof(struct stackframe));
	while (1)
	{
		int ret = unwind_frame_fix(&current_stk);
		sprint_symbol(ksym, current_stk.pc);
		sprint_symbol(ksymlr, current_stk.lr);
		printk("Current frame: fp = %08lx, pc = %08lx [%s], lr = %08lx [%s]\n", current_stk.fp, current_stk.pc, ksym, current_stk.lr, ksymlr);
		/* oops, reached end without exception. return original info */
		if (ret < 0)
			break;
		if (in_exception_text(current_stk.pc))
		{
			printk("Found exception point!\n"); 
			/* passed exception point, return this if unwinding is sucessful */
			memcpy(frame, &current_stk, sizeof(struct stackframe));
			frame->lr = 0; // let it be recovered from stack.
			return 1;
		}
	}
	return 0;
}

static unsigned int linear_map_size = 0;
static unsigned int vmalloc_map_size = 0;
static unsigned int module_map_size = 0;

unsigned int kdump_init_mem_len(void)
{
     /*PA [0 PHYS_OFFSET) not mapped into linear mapping */
    linear_map_size = (unsigned long)high_memory - PAGE_OFFSET;
    vmalloc_map_size = VMALLOC_END - VMALLOC_START;
    module_map_size = MODULES_END - MODULES_VADDR; 

    return linear_map_size  + vmalloc_map_size + module_map_size;
}

void kdump_core_header_init(u8 *bufp)
{
	struct stackframe frame;
	int i;
	struct elf_prstatus prstatus;	/* NT_PRSTATUS */
	struct elf_prpsinfo prpsinfo;	/* NT_PRPSINFO */
	struct elf_phdr *nhdr, *phdr;
	struct elfhdr *elf;
	struct memelfnote notes[3];
	off_t offset = 0;
	//debug
	u8 *oldbufp = bufp;

	kdump_init_mem_len();

	/* setup ELF header */
	elf = (struct elfhdr *) bufp;
	bufp += sizeof(struct elfhdr);
	offset += sizeof(struct elfhdr);
	memcpy(elf->e_ident, ELFMAG, SELFMAG);
	elf->e_ident[EI_CLASS]	= ELF_CLASS;
	elf->e_ident[EI_DATA]	= ELF_DATA;
	elf->e_ident[EI_VERSION]= EV_CURRENT;
	elf->e_ident[EI_OSABI] = ELF_OSABI;
	memset(elf->e_ident+EI_PAD, 0, EI_NIDENT-EI_PAD);
	elf->e_type	= ET_CORE;
	elf->e_machine	= ELF_ARCH;
	elf->e_version	= EV_CURRENT;
	elf->e_entry	= 0;
	elf->e_phoff	= sizeof(struct elfhdr);
	elf->e_shoff	= 0;
	elf->e_flags	= ELF_CORE_EFLAGS;
	elf->e_ehsize	= sizeof(struct elfhdr);
	elf->e_phentsize= sizeof(struct elf_phdr);
	/*Note, linear mapping, vmalloc , module*/
	elf->e_phnum	= 4;
	elf->e_shentsize= 0;
	elf->e_shnum	= 0;
	elf->e_shstrndx	= 0;

	/* setup ELF PT_NOTE program header */
	nhdr = (struct elf_phdr *) bufp;
	bufp += sizeof(struct elf_phdr);
	offset += sizeof(struct elf_phdr);
	nhdr->p_type	= PT_NOTE;
	nhdr->p_offset	= 0;
	nhdr->p_vaddr	= 0;
	nhdr->p_paddr	= 0;
	nhdr->p_filesz	= 0;
	nhdr->p_memsz	= 0;
	nhdr->p_flags	= 0;
	nhdr->p_align	= 0;

	/*header for linear mapping*/
	phdr = (struct elf_phdr *) bufp;
	bufp += sizeof(struct elf_phdr);
	offset += sizeof(struct elf_phdr);

	phdr->p_type	= PT_LOAD;
	phdr->p_flags	= PF_R|PF_W|PF_X;
	phdr->p_offset	= KDUMP_CORE_SIZE;
	phdr->p_vaddr	= (size_t) PAGE_OFFSET;
	phdr->p_paddr	= 0;
	phdr->p_filesz	= phdr->p_memsz	= linear_map_size;
	phdr->p_align	= KDUMP_CORE_SIZE;

	/*header for vmalloc*/
	phdr = (struct elf_phdr *) bufp;
	bufp += sizeof(struct elf_phdr);
	offset += sizeof(struct elf_phdr);
	phdr->p_type	= PT_LOAD;
	phdr->p_flags	= PF_R|PF_W|PF_X;
	phdr->p_offset	= KDUMP_CORE_SIZE + linear_map_size;
	phdr->p_vaddr	= (size_t) VMALLOC_START;
	phdr->p_paddr	= 0;
	phdr->p_filesz	= phdr->p_memsz	= vmalloc_map_size;
	phdr->p_align	= PAGE_SIZE;

	/*header for module*/
	phdr = (struct elf_phdr *) bufp;
	bufp += sizeof(struct elf_phdr);
	offset += sizeof(struct elf_phdr);
	phdr->p_type	= PT_LOAD;
	phdr->p_flags	= PF_R|PF_W|PF_X;
	phdr->p_offset	= KDUMP_CORE_SIZE + linear_map_size + vmalloc_map_size;
	phdr->p_vaddr	= (size_t) MODULES_VADDR;
	phdr->p_paddr	= 0;
	phdr->p_filesz	= phdr->p_memsz	= module_map_size;
	phdr->p_align	= PAGE_SIZE;

	/*
	 * Set up the notes in similar form to SVR4 core dumps made
	 * with info from their /proc.
	 */
	nhdr->p_offset	= offset;

	/* set up the process status */
	notes[0].name = CORE_STR;
	notes[0].type = NT_PRSTATUS;
	notes[0].datasz = sizeof(struct elf_prstatus);
	notes[0].data = &prstatus;

	memset(&prstatus, 0, sizeof(struct elf_prstatus));
	fill_regs(&prstatus.pr_reg);

	/* skip exception stack */
	if (find_exception_stack(&frame) == 1)
	{
		unsigned long fp_addr = frame.fp + 4;
		memcpy(&prstatus.pr_reg, (unsigned long*)fp_addr, sizeof(elf_gregset_t));
		printk("Exception stack at %08lx:\n", fp_addr);
		for(i=0;i<16;i++)
			printk("reg[%d] = %08lx\n", i, prstatus.pr_reg[i]);
	}

	nhdr->p_filesz = notesize(&notes[0]);
	bufp = storenote(&notes[0], bufp);

	/* set up the process info */
	notes[1].name = CORE_STR;
	notes[1].type = NT_PRPSINFO;
	notes[1].datasz	= sizeof(struct elf_prpsinfo);
	notes[1].data = &prpsinfo;

	memset(&prpsinfo, 0, sizeof(struct elf_prpsinfo));
	prpsinfo.pr_state = 0;
	prpsinfo.pr_sname = 'R';
	prpsinfo.pr_zomb = 0;

	strcpy(prpsinfo.pr_fname, "vmlinux");
	strlcpy(prpsinfo.pr_psargs, saved_command_line, ELF_PRARGSZ);

	nhdr->p_filesz	+= notesize(&notes[1]);
	bufp = storenote(&notes[1], bufp);

	/* set up the task structure */
	notes[2].name = CORE_STR;
	notes[2].type = NT_TASKSTRUCT;
	notes[2].datasz	= sizeof(struct task_struct);
	notes[2].data = current;

	nhdr->p_filesz	+= notesize(&notes[2]);
	bufp = storenote(&notes[2], bufp);
    
	printk("%s elf header len %d \n", __func__, bufp - oldbufp);
}

