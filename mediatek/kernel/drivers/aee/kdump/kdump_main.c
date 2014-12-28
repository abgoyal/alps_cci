#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/sched.h>
#include <linux/atomic.h>

#include <asm/ptrace.h>
#include <asm/processor.h>
#include "kdump.h"
#include "kdump_elf.h"
#include "kdump_sdhc.h"
#include <linux/mm.h>

static atomic_t kdump_on_panic = ATOMIC_INIT(0);
u8 kdump_core_header[KDUMP_CORE_SIZE];

extern void mtk_wdt_disable(void);
#ifdef CONFIG_LOCAL_WDT
extern void mpcore_wk_wdt_stop(void);
#endif

static int kdump_create_dump(struct notifier_block *this, unsigned long event, void *ptr)
{
	unsigned long irq;
	int current_kop;
	
	current_kop = atomic_xchg(&kdump_on_panic, 0);
	if (current_kop == 0) {
		printk("%s: No kdump activate\n", __func__);
		return NOTIFY_DONE;
	}
	
	if (current_kop) {
		mtk_wdt_disable();
#ifdef CONFIG_LOCAL_WDT
		mpcore_wk_wdt_stop();
#endif
		local_irq_save(irq);
		
		switch (current_kop & 0x7) {
		case KDUMP_CONFIG_USB:
			USBCreateDumpFile();
			break;

		case KDUMP_CONFIG_MSDC_FORMAT_WRITE:
			kdump_create_dump_msdc_format_file();
			break;

		case KDUMP_CONFIG_MSDC_WRITE:
			kdump_create_dump_msdc_file();
			break;
        
		default:
			break;
		}
		
		local_irq_restore(irq);
	}
	return NOTIFY_DONE;
}

static int kdump_flag_write(struct file *file, const char *buffer, unsigned long count,
			    void *data)
{
	char Buf[32]; 
	int copy_size = 0, new_kop;

	copy_size = (count < (sizeof(Buf) - 1)) ? count : (sizeof(Buf) - 1);

	if (copy_from_user(Buf, buffer, copy_size))
		return 0;
	Buf[copy_size] = '\0';

	sscanf(Buf, "%x", &new_kop);

	switch (new_kop & 0x7) {
	case KDUMP_CONFIG_NONE:
		printk(KERN_INFO "%s: kdump flag set no dump\n", __func__);
		break;

	case KDUMP_CONFIG_USB:
		printk(KERN_INFO "%s: kdump flag set dump to usb\n", __func__);
		break;

	case KDUMP_CONFIG_MSDC_FORMAT_WRITE:
		printk(KERN_INFO "%s: kdump flag set dump to sdcard(format)\n", __func__);
		break;

	case KDUMP_CONFIG_MSDC_WRITE:
		printk(KERN_INFO "%s: kdump flag set dump to sdcard\n", __func__);
		break;
    
	default:
		printk(KERN_WARNING "%s: Unknown kdump flag %x, clear to zero\n", __func__, new_kop);
		new_kop = 0;
	}

	atomic_set(&kdump_on_panic, new_kop & 0x7);
	smp_mb();
	
	if (new_kop & 0x80000000) {
		kdump_create_dump(NULL, 0, NULL);
	}
	
	return count;
}

static int kdump_flag_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	char *p = page;
	int len = 0, current_kop;

	current_kop = atomic_read(&kdump_on_panic);
	p += sprintf(p, "0x%x\n", current_kop);
	*start = page + off;

	len = p - page;
	if (len > off)
		len -= off;
	else
		len = 0;

	return (len < count) ? len : count;
}

static struct notifier_block panic_blk = {
	.notifier_call	= kdump_create_dump,
};

#ifdef MTK_USE_RESERVED_EXT_MEM
extern void init_kdump_sd_cache(void);
#endif
extern int USBDumpInit(void);

struct proc_dir_entry *kop_entry;

static int __init kdump_init(void)
{
	printk("%s: kdump module start\n", __func__);
    
	atomic_notifier_chain_register(&panic_notifier_list, &panic_blk);
    
	kop_entry = create_proc_entry(KDUMP_ON_PANIC_NAME, 0, NULL);
	if (kop_entry) {
		kop_entry->read_proc = kdump_flag_read;
		kop_entry->write_proc = kdump_flag_write;
	}
	
	USBDumpInit();
#ifdef MTK_USE_RESERVED_EXT_MEM
	init_kdump_sd_cache();
#endif    
	return 0;
}

static void __exit kdump_exit(void)
{
	printk(KERN_INFO "%s: kdump module exit\n", __func__);

	atomic_notifier_chain_unregister(&panic_notifier_list, &panic_blk);
	if (kop_entry != NULL) {
		remove_proc_entry(KDUMP_ON_PANIC_NAME, kop_entry);
		kop_entry = NULL;
	}
}

module_init(kdump_init);
module_exit(kdump_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MediaTek KDump Driver");
MODULE_AUTHOR("MediaTek Inc.");

