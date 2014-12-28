#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/wakelock.h>
#include <linux/module.h>
#include <asm/delay.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/gfp.h>
#include <asm/io.h>
#include <asm/memory.h>
#include <asm/outercache.h>
#include <linux/spinlock.h>

#include <linux/slab.h>
#include <linux/leds-mt65xx.h>
#include <linux/sched.h>
#include <linux/vmalloc.h>
#include "slt.h"

extern int fp2_scu_start(unsigned long);

static int g_iCPU0_PassFail, g_iCPU1_PassFail;
static int g_iScuLoopCount;
static int g_iCPU0_SCU_Sync = 0;
static int g_iCPU1_SCU_Sync = 0;
volatile unsigned int scu_error_flag;
static unsigned long *scu_mem_test_buf;

static struct device_driver slt_cpu0_scu_drv =
{
    .name = "slt_cpu0_scu",
    .bus = &platform_bus_type,
    .owner = THIS_MODULE,
};

static struct device_driver slt_cpu1_scu_drv =
{
    .name = "slt_cpu1_scu",
    .bus = &platform_bus_type,
    .owner = THIS_MODULE,
};

static struct device_driver slt_scu_loop_count_drv =
{
    .name = "slt_scu_loop_count",
    .bus = &platform_bus_type,
    .owner = THIS_MODULE,
};

#define DEFINE_SLT_CPU_SCU_SHOW(_N)    \
static ssize_t slt_cpu##_N##_scu_show(struct device_driver *driver, char *buf) \
{   \
    if(g_iCPU##_N##_PassFail == -1) \
        return snprintf(buf, PAGE_SIZE, "CPU%d SCU - CPU%d is powered off\n", _N, _N); \
    else    \
        return snprintf(buf, PAGE_SIZE, "CPU%d SCU - %s(loop_count = %d)\n", _N, g_iCPU##_N##_PassFail != g_iScuLoopCount ? "FAIL" : "PASS", g_iCPU##_N##_PassFail);  \
}

DEFINE_SLT_CPU_SCU_SHOW(0)
DEFINE_SLT_CPU_SCU_SHOW(1)

static ssize_t slt_scu_loop_count_show(struct device_driver *driver, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "SCU Loop Count = %d\n", g_iScuLoopCount);
}

#define DEFINE_SLT_CPU_SCU_STORE(_N)                               \
static ssize_t slt_cpu##_N##_scu_store(struct device_driver *driver, const char *buf, size_t count) \
{                                                                       \
    unsigned int i, ret;                                                \
    unsigned long mask;                                                 \
    int retry=0;                                                        \
    DEFINE_SPINLOCK(cpu##_N##_lock);                                    \
    unsigned long cpu##_N##_flags;                                      \
    unsigned char *mem_buf = (unsigned long *)kmalloc(0x10000, GFP_KERNEL); \
                                                                        \
    g_iCPU##_N##_PassFail = 0;                                          \
    if(!mem_buf)                                                        \
    {                                                                   \
        printk("allocate memory for cpu%d test fail\n", _N);      \
        return 0;                                                       \
    }                                                                   \
                                                                        \
    printk("allocate memory for cpu%d = 0x%08x\n", _N, mem_buf);        \
                                                                        \
    mask = (1 << _N); /* processor _N */                                \
    while(sched_setaffinity(0, (struct cpumask*) &mask) < 0)            \
    {                                                                   \
        printk("Could not set cpu%d affinity for current process(%d).\n", _N, retry); \
        g_iCPU##_N##_PassFail = -1;                                     \
        retry++;                                                        \
        if(retry > 100)                                                 \
        {                                                               \
            kfree(mem_buf);                                             \
            return count;                                               \
        }                                                               \
    }                                                                   \
                                                                        \
    printk("\n>> CPU%d Scu test start (cpu id = %d) <<\n\n", _N, raw_smp_processor_id()); \
                                                                        \
    for (i = 0, g_iCPU##_N##_PassFail = 0; i < g_iScuLoopCount; i++) { \
        spin_lock_irqsave(&cpu##_N##_lock, cpu##_N##_flags);            \
        ret = 1; \
        spin_unlock_irqrestore(&cpu##_N##_lock, cpu##_N##_flags);       \
        if(ret != -1)                                                   \
        {                                                               \
            g_iCPU##_N##_PassFail += ret;                               \
        }                                                               \
        else                                                            \
        {                                                               \
            g_iCPU##_N##_PassFail = -1;                                 \
            break;                                                      \
        }                                                               \
    }                                                                   \
                                                                        \
    if (g_iCPU##_N##_PassFail == g_iScuLoopCount) {                \
        printk("\n>> CPU%d scu test pass <<\n\n", _N);             \
    }else {                                                             \
        printk("\n>> CPU%d scu test fail (loop count = %d)<<\n\n", _N, g_iCPU##_N##_PassFail); \
    }                                                                   \
                                                                        \
    kfree(mem_buf);                                                     \
    return count;                                                       \
}

DEFINE_SLT_CPU_SCU_STORE(0)
DEFINE_SLT_CPU_SCU_STORE(1)

static ssize_t slt_scu_loop_count_store(struct device_driver *driver, const char *buf, size_t count)
{
    int result;

    if ((result = sscanf(buf, "%d", &g_iScuLoopCount)) == 1)
    {
        printk("set SLT Scu test loop count = %d successfully\n", g_iScuLoopCount);
    }
    else
    {
        printk("bad argument!!\n");
        return -EINVAL;
    }

    return count;
}

DRIVER_ATTR(slt_cpu0_scu, 0644, slt_cpu0_scu_show, slt_cpu0_scu_store);
DRIVER_ATTR(slt_cpu1_scu, 0644, slt_cpu1_scu_show, slt_cpu1_scu_store);
DRIVER_ATTR(slt_scu_loop_count, 0644, slt_scu_loop_count_show, slt_scu_loop_count_store);

#define DEFINE_SLT_CPU_SCU_INIT(_N)    \
int __init slt_cpu##_N##_scu_init(void) \
{   \
    int ret;    \
    \
    ret = driver_register(&slt_cpu##_N##_scu_drv);  \
    if (ret) {  \
        printk("fail to create SLT CPU%d Scu driver\n", _N);    \
    }   \
    else    \
    {   \
        printk("success to create SLT CPU%d Scu driver\n", _N); \
    }   \
    \
    ret = driver_create_file(&slt_cpu##_N##_scu_drv, &driver_attr_slt_cpu##_N##_scu);   \
    if (ret) {  \
        printk("fail to create SLT CPU%d Scu sysfs files\n", _N);   \
    }   \
    else    \
    {   \
        printk("success to create SLT CPU%d Scu sysfs files\n", _N);    \
    }   \
    \
    return 0;   \
}

DEFINE_SLT_CPU_SCU_INIT(0)
DEFINE_SLT_CPU_SCU_INIT(1)

int __init slt_scu_loop_count_init(void)
{
    int ret;

    ret = driver_register(&slt_scu_loop_count_drv);
    if (ret) {
        printk("fail to create Scu loop count driver\n");
    }
    else
    {
        printk("success to create Scu loop count driver\n");
    }


    ret = driver_create_file(&slt_scu_loop_count_drv, &driver_attr_slt_scu_loop_count);
    if (ret) {
        printk("fail to create Scu loop count sysfs files\n");
    }
    else
    {
        printk("success to create Scu loop count sysfs files\n");
    }

    g_iScuLoopCount = SLT_LOOP_CNT;

    return 0;
}

arch_initcall(slt_cpu0_scu_init);
arch_initcall(slt_cpu1_scu_init);
arch_initcall(slt_scu_loop_count_init);
