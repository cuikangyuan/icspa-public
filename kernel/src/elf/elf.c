#include "common.h"
#include "memory.h"
#include "string.h"

#include <elf.h>

#ifdef HAS_DEVICE_IDE
#define ELF_OFFSET_IN_DISK 0
#endif

#define STACK_SIZE (1 << 20)

void ide_read(uint8_t *, uint32_t, uint32_t);
void create_video_mapping();
uint32_t get_ucr3();

/*
装载是操作系统的功能，返回程序入口地址
程序头表：只有可执行文件有，可重定位文件没有
装载程序的实现:
	1.解析程序头表：对应文件中的哪一部分(节)，搬到内存中的哪个位置(段)

*/
uint32_t loader()
{
	Elf32_Ehdr *elf;
	Elf32_Phdr *ph, *eph;

#ifdef HAS_DEVICE_IDE
	uint8_t buf[4096];
	ide_read(buf, ELF_OFFSET_IN_DISK, 4096);
	elf = (void *)buf;
	Log("ELF loading from hard disk.");
#else
	//模拟内存0x0处是RAM Disk, 存放testcase ELF file, 最开始部分是ELF头
	elf = (void *)0x0;
	Log("ELF loading from ram disk.");
#endif

	/* Load each program segment */
	//找到ELF文件中的程序头表
	ph = (void *)elf + elf->e_phoff;
	eph = ph + elf->e_phnum;
	for (; ph < eph; ph++)
	{
		if (ph->p_type == PT_LOAD)
		{
			/*
			装载到内存
			若type为LOAD, ELF文件中从文件Offset开始位置，连续FileSize个字节的内容需要被装载
			装载到内存VirtAddr开始，连续MemSize个字节的区域中(bss节存储未初始化的全局变量-MemSize会比FileSize大[文件中不需要存储，但在内存中需要分配空间]
			)
			*/
			/*copy the segment from the ELF file to its proper memory area */

			//开启分页机制之后:
			uint32_t start_addr;
#ifdef IA32_PAGE
			start_addr = mm_malloc(ph->p_vaddr, ph->p_memsz);
#else
			start_addr = ph->p_vaddr;
#endif
			//printf("elf loader start_addr 0x%x\n", start_addr);

			memcpy((void *)start_addr, (void *)ph->p_offset, ph->p_filesz);
			if (ph->p_memsz > ph->p_filesz)
			{
				memset((void *)(start_addr + ph->p_filesz), 0x0, ph->p_memsz - ph->p_filesz);
			}

			//未开启分页机制以前：
			// memcpy((void *)ph->p_vaddr, (void *)ph->p_offset, ph->p_filesz);
			// /*zeror the memory area [vaddr + file_sz, vaddr + mem_sz) */
			// if (ph->p_memsz > ph->p_filesz)
			// {
			// 	memset((void *)(ph->p_vaddr + ph->p_filesz), 0x0, ph->p_memsz - ph->p_filesz);
			// }

#ifdef IA32_PAGE
			/* Record the program break for future use */
			extern uint32_t brk;
			uint32_t new_brk = ph->p_vaddr + ph->p_memsz - 1;
			if (brk < new_brk)
			{
				brk = new_brk;
			}
#endif
		}
	}

	//头文件中指出的testcase起始地址 应该是0x60000
	volatile uint32_t entry = elf->e_entry;

#ifdef IA32_PAGE
	mm_malloc(KOFFSET - STACK_SIZE, STACK_SIZE);
#ifdef HAS_DEVICE_VGA
	create_video_mapping();
#endif
	write_cr3(get_ucr3());
#endif
	//返回testcase起始地址，在init_cond()后面执行((void(*)(void))eip)();
	return entry;
}
