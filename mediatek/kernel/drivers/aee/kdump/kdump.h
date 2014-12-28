#if !defined(__KDUMP_H__)
#define __KDUMP_H__

#include <linux/mm.h>

#define KDUMP_ON_PANIC_NAME "kdump_on_panic"

#define KDUMP_CONFIG_NONE              0
#define KDUMP_CONFIG_USB               1
#define KDUMP_CONFIG_MSDC_FORMAT_WRITE 2
#define KDUMP_CONFIG_MSDC_WRITE        6

#define DBGKDUMP_PRINTK printk
static inline unsigned int get_linear_mapping_size(void)
{
    /*PA [0 PHYS_OFFSET) not mapped into linear mapping */
   return (unsigned long)high_memory - PAGE_OFFSET; 
}


extern u8 kdump_core_header[];

int USBCreateDumpFile(void);

int kdump_create_dump_msdc_format_file(void);

int kdump_create_dump_msdc_file(void);

#endif
