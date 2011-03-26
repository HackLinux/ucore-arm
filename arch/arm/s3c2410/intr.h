#ifndef __ARCH_ARM_INTR_H__
#define __ARCH_ARM_INTR_H__

#include <types.h>

#define INT_BASE	(0x4a000000)
#define INTMSK		(INT_BASE+0x8)
#define INTOFFSET	(INT_BASE+0x14)
#define INTPND		(INT_BASE+0x10)
#define SRCPND		(INT_BASE+0x0)

/*
 * Initialize interrupts. 
 * Currently it'll set up the interrupt vector table.
 */
void intr_init(void);

/*
 * Enable interrupts.
 */
void intr_enable(void);
/*
 * Disable interrupts.
 */
void intr_disable(void);
/*
 * Mask an interrupt (disable the interrupt).
 */
void intr_mask(uint32_t offset);
/*
 * Unmask an interrupt (enable the interrupt).
 */
void intr_umask(uint32_t offset);

/*
 * Main interrupt dispatcher.
 */
void intr_dispatch(void);

#endif /* !__ARCH_ARM_INTR_H__ */
