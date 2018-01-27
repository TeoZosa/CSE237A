#include <linux/module.h>
#include <linux/kernel.h>
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your name");
MODULE_DESCRIPTION("PMUON");

#define REG1 0x00
#define REG2 0x01
#define REG3 0x02
#define REG4 0x03
#define REG5 0x04
#define REG6 0x05

#define INST_EXEC 0X08
#define L1D_CACHE_ACSS 0X04
#define L1D_CACHE_MISS 0X03
#define L2D_CACHE_ACSS 0X16
#define L2D_CACHE_MISS 0X17
#define L1I_TLB_MISS 0X02

#define PMSELR_WRITE "mcr p15, 0, %0, c9, c12, 5\n\t" :: "r"
#define PMXEVTYPER_WRITE "mcr p15, 0, %0, c9, c13, 1\n\t" :: "r"

void set_pmu(void* dummy) {
 unsigned int v;
 printk("Turn PMU on Core %d\n", smp_processor_id());
 // 1. Enable "User Enable Register"
 asm volatile("mcr p15, 0, %0, c9, c14, 0\n\t" :: "r" (0x00000001));
 // 2. Reset
    // i) Performance Monitor Control Register(PMCR),
     // ii) Count Enable Set Register, and
     // iii) Overflow Flag Status Register
 asm volatile ("mcr p15, 0, %0, c9, c12, 0\n\t" :: "r"(0x00000017));
 asm volatile ("mcr p15, 0, %0, c9, c12, 1\n\t" :: "r"(0x8000003f));
 asm volatile ("mcr p15, 0, %0, c9, c12, 3\n\t" :: "r"(0x8000003f));
 // 3. Disable Interrupt Enable Clear Register
 asm volatile("mcr p15, 0, %0, c9, c14, 2\n\t" :: "r" (~0));
 // 4. Read how many event counters exist
 asm volatile("mrc p15, 0, %0, c9, c12, 0\n\t" : "=r" (v)); // Read PMCR


  printk("PMCR value is %d \n",
         (v ));
  printk("PMCR value bitshifted 11 times is %d\nMasked with 0x1f gives us # event counters\n",
         (v >> 11));

  printk("We have %d configurable event counters on Core %d\n",
         (v >> 11) & 0x1f, smp_processor_id());


 // 5. Set six event counter registers (Project Assignment you need to IMPLEMENT)

                    //[PMSELR register]: N. (0XN-1)
                    //[PMXEVTYPER register]: EVENT_MNEMONIC (0XEM) - description
//  1.(0X0)
// INST_RETIRED (0X08) - # of instructions architecturally executed
  asm volatile(PMSELR_WRITE (REG1));
  asm volatile(PMXEVTYPER_WRITE (INST_EXEC));

//  2.(0X1)
// L1D_CACHE (0X04) - # L1 Data cache accesses
  asm volatile(PMSELR_WRITE (REG2));
  asm volatile(PMXEVTYPER_WRITE (L1D_CACHE_ACSS));

//  3.(0X2)
// L1D_CACHE_REFILL (0X03) - # L1 Data cache misses (refill)
  asm volatile(PMSELR_WRITE (REG3));
  asm volatile(PMXEVTYPER_WRITE (L1D_CACHE_MISS));

//  4.(0X3)
// L2D_CACHE (0X16) - # L2 Data cache accesses
  asm volatile(PMSELR_WRITE (REG4));
  asm volatile(PMXEVTYPER_WRITE (L2D_CACHE_ACSS));

//  5.(0X4)
// L2D_CACHE_REFILL (0X17) - # L2 Data cache misses (refill)
  asm volatile(PMSELR_WRITE (REG5));
  asm volatile(PMXEVTYPER_WRITE (L2D_CACHE_MISS));

//  6.(0X5)
// L1I_TLB_REFILL (0X02) - # L1 Instruction TLB misses (refill)
  asm volatile(PMSELR_WRITE (REG6));
  asm volatile(PMXEVTYPER_WRITE (L1I_TLB_MISS));



printk("Event counters set\n");


}
int init_module(void) {
 // Set Performance Monitoring Unit (aka Performance Counter) across all cores
 on_each_cpu(set_pmu, NULL, 1);
 printk("Ready to use PMU\n");
 return 0;
}
void cleanup_module(void) {
 printk("PMU Kernel Module Off\n");
}