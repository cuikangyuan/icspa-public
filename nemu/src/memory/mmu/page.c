#include "cpu/cpu.h"
#include "memory/memory.h"
#include "cpu/cpu.h"

// translate from linear address to physical address
paddr_t page_translate(laddr_t laddr)
{
#ifndef TLB_ENABLED
#ifdef IA32_PAGE
    //printf("page translate laddr 0x%x\n", laddr);
	PDE *pde1 = (PDE *)(hw_mem + (cpu.cr3.page_directory_base << 12) + 4 * ((laddr >> 22) & 0x000003FF));
	//printf("page translate pde 0x%x\n", pde1->val);
	assert(pde1->present == 1);
	PTE *pte1 = (PTE *)(hw_mem + (pde1->page_frame << 12) + 4 * ((laddr >> 12) & 0x000003FF));
	//printf("page translate pte 0x%x\n", pte1->val);
	assert(pte1->present == 1);
	uint32_t paddr1 = (pte1->page_frame << 12) + (laddr & 0x00000FFF);
	//printf("page translate paddr1 0x%x\n", paddr1);

	return paddr1;

	// paddr_t pdir_base = cpu.cr3.val & ~PAGE_MASK;
	// uint32_t pdir_idx = (laddr >> 22) & 0x000003FF;
	// PDE pde;
	// pde.val = paddr_read(pdir_base + pdir_idx * 4, 4);
	// if(!pde.present) {
	// 	printf("page_translate, eip = %x, lnaddr = %x, pdir_base = %x, pde = %x", cpu.eip, laddr, pdir_base, pde.val);
	// 	assert(0);
	// }
	// assert(pde.present);

	// paddr_t pt_base = pde.val & ~PAGE_MASK;
	// PTE pte;
	// uint32_t pt_idx = (laddr >> 12) & 0x000003FF;
	// pte.val = paddr_read(pt_base + pt_idx * 4, 4);
	// if(!pte.present) {
	// 	printf("page_translate, eip = %x, lnaddr = %x, pt_base = %x, pte = %x", cpu.eip, laddr, pt_base, pte.val);
	// 	assert(0);
	// }
	// assert(pte.present);
	// uint32_t paddr2 = (pte.page_frame << 12) + (laddr & 0x00000FFF);
	// printf("page translate paddr2 0x%x\n", paddr2);
	// return paddr2;
#else
	printf("\nPlease implement page_translate()\n");
	fflush(stdout);
	assert(0);

#endif
#else
	return tlb_read(laddr) | (laddr & PAGE_MASK);
#endif
}
