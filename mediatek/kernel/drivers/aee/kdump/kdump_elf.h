/* 
 * (C) Copyright 2010
 * MediaTek <www.MediaTek.com>
 *
 * Android Exception Device
 *
 */

#if !defined(__KDUMP_ELF_H__)
#define __KDUMP_ELF_H__

#define CORE_STR "CORE"

#ifndef ELF_CORE_EFLAGS
#define ELF_CORE_EFLAGS	0
#endif

#define KDUMP_CORE_SIZE PAGE_SIZE

void kdump_core_header_init(u8 *buf);

#endif /* __KDUMP_ELF_H__ */
