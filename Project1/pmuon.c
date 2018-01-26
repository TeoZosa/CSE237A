#include <linux/module.h>
#include <linux/kernel.h>
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your name");
MODULE_DESCRIPTION("PMUON");
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
 printk("We have %d configurable event counters on Core %d\n",
 (v >> 11) & 0x1f, smp_processor_id());
 // 5. Set six event counter registers (Project Assignment you need to IMPLEMENT)

    //PMSELR register
 asm volatile("mcr p15, 0, %0, c9, c12, 5\n\t" :: "r" (0x0));
asm volatile("mcr p15, 0, %0, c9, c13, 1\n\t" :: "r" (0x08));




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