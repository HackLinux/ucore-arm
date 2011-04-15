#ifndef __ARCH_MMU_H__
#define __ARCH_MMU_H__

#define PDE_FINE    3
#define PTE_SMALL

#define PDE_P   (1<<4)
// Really, this is not pte_p, but the flag for small page
#define PTE_P   (1<<1)

#define PTE_RW 0xFF0

#endif /* !__ARCH_MMU_H__ */
