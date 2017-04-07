/*
 * $FILE: paging.h
 *
 * i386 paging
 *
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XM_ARCH_PAGING_H_
#define _XM_ARCH_PAGING_H_

#ifndef _XM_KERNEL_
#error Kernel file, do not include.
#endif

#define PAGE_SHIFT 12
#define PGDIR_SHIFT 22

#define XMVIRT2PHYS(x) (((xm_u32_t)x)-XM_OFFSET)
#define XMPHYS2VIRT(x) (((xm_u32_t)x)+XM_OFFSET)

// Number of PGD entries used by XM
#define XM_PGD_START (XM_OFFSET >> PGDIR_SHIFT)
#define XM_PGD_END (PDT_ENTRIES-1) // Not included

// The current PGD is mapped at this PGD entry
//#define XM_PGT_DIR_ENTRY 1023
#define XM_PGT_DIR_VADDR 0xFFC00000

#define XM_VMAPSIZE (XM_PGT_DIR_VADDR-XM_OFFSET)

#define PAGE_SIZE 4096
#define PSE_PAGE_SIZE (4096*1024)

//#define LARGE_PAGE_SIZE PSE_PAGE_SIZE

#define PAGE_MASK (~(PAGE_SIZE-1))

#define PGT_MASK 0x3FF000
#define PGD_MASK 0xFFC00000

/* 
   Number of entries in a page directory/table 
*/

#define PDT_ENTRIES 1024

/*
  Page directory/table options
*/

#define _PG_PRESENT 0x001
#define _PG_RW 0x002
#define _PG_USER 0x004
#define _PG_PWT 0x008
#define _PG_PCD 0x010
#define _PG_ACCESSED 0x020
#define _PG_DIRTY 0x040
#define _PG_PSE 0x080
#define _PG_GLOBAL 0x100
#define _PG_UNUSED1 0x200
#define _PG_UNUSED2 0x400
#define _PG_UNUSED3 0x800

#define CMP_PAGE_MASK (PAGE_MASK|_PG_PRESENT)

#ifndef __ASSEMBLY__

/* virtual address to page directory entry */
#define VA2Pgd(vaddress) (vaddress>>PGDIR_SHIFT)

/* virtual address to page table entry */
#define VA2Pgt(vaddress) ((vaddress&PGT_MASK)>>PAGE_SHIFT)

/* page directory and table to virtual address */
#define PgdPgt2VA(pgd, pgt) ((pgd<<PGDIR_SHIFT)|(pgt<<PAGE_SHIFT))

/* XM's PGD */
extern xm_u32_t xmPgd[];
/* XM's PGT */
extern xm_u32_t xmPgt[];
#endif // !__ASSEMBLY__
#endif
