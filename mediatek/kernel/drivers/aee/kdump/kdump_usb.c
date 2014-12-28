#include <linux/compiler.h>
#include <linux/linkage.h>
#include <linux/string.h>
#include <linux/printk.h>

#include "kdump.h"
#include "kdump_elf.h"
#include "kdump_sdhc.h"

extern int usb_init(void);
extern unsigned int Serial_Recv(unsigned char *pData, unsigned int size, unsigned int timeout);
extern unsigned int Serial_Send(unsigned char *pData, unsigned int size) ;
extern int usb_get_max_send_size(void);

#define PMDMAGICSTRING "PMDSTAR:"
#define PMDMAGICLENGTH (0x8)
#define READMEMLENGTHONEIME (0x100)
#define PMDACK (0xa5a55a5a)
#define BUFFSIZE 0x100
static unsigned char buf[BUFFSIZE];

int RecvAck(void)
{
	memset(buf, 0, BUFFSIZE);
	if (Serial_Recv(buf, 0x10, 10000) <= 0)	{
		DBGKDUMP_PRINTK("error : cant receive ACK !\n");
		return -1 ;
	}
	if (*(DWORD*)buf != PMDACK) {
		DBGKDUMP_PRINTK(" error receive ACK, 0x%x!\n", *(DWORD*)buf);
		return -1;
	}
	return 0;
}

int USBCreateDumpFile(void)
{
	PBYTE ptr = (PBYTE) 0xc0000000;
	unsigned long offset = 0;
	int ret = 0;
	unsigned int total_dump_size = get_linear_mapping_size();
	unsigned int timeout = 2000000;
	unsigned int readlen = 0;
    
	if (usb_init() != 0)
		return -1;
    
	readlen = usb_get_max_send_size();
	if (readlen == 0)
		readlen = READMEMLENGTHONEIME;
    
	DBGKDUMP_PRINTK("usb init returns.\n");
    
	// hand shake.
	while (timeout--) {
		memset(buf, 0, BUFFSIZE);
		if (Serial_Recv(buf, 0x10, 10000) <= 0) {
			DBGKDUMP_PRINTK("error can't receive! \n");
			continue;
		}
    
		if (*(DWORD*)buf != 0x5a5aa5a5)	{
			DBGKDUMP_PRINTK("error tag 0x%x\n", *(DWORD*)buf);
			continue;
		}
		else {
			// hand shake OK.
			DBGKDUMP_PRINTK("tag OK 0x%x\n", *(DWORD*)buf);
			break;
		}
	}
	if (timeout == 0)
		return -1;
    
	// size
	memset(buf, 0, BUFFSIZE);
	memcpy(buf, (PMDMAGICSTRING), PMDMAGICLENGTH);
	*(DWORD*)(buf + PMDMAGICLENGTH) = total_dump_size + KDUMP_CORE_SIZE;
	*(DWORD*)(buf + PMDMAGICLENGTH + sizeof(DWORD)) = readlen ;
    
	if (Serial_Send(buf, BUFFSIZE) <= 0) {
		DBGKDUMP_PRINTK("error can't send! \n");
		return -1;
	}
	else {
		DBGKDUMP_PRINTK("send OK! \n");
	}
    
	// elf header
	ptr = (PBYTE) kdump_core_header;
	offset = 0;
	kdump_core_header_init(kdump_core_header);
	while (RecvAck () == 0) {
		int bytetosend = ((KDUMP_CORE_SIZE - offset) > readlen)?readlen:(KDUMP_CORE_SIZE - offset);
		if (unlikely(Serial_Send(ptr+offset, bytetosend) != bytetosend)) {
			DBGKDUMP_PRINTK("Serial_Send failed at 0x%x! \n", (unsigned int)offset);
			ret = -1;
			break;
		}
       
		offset += bytetosend;
        
		if (unlikely(offset >= KDUMP_CORE_SIZE)) {
			DBGKDUMP_PRINTK("elf header done: 0x%x transfered!\n", (unsigned int)offset);
			ret = 0;
			break;
		}
	}

	DBGKDUMP_PRINTK("starting transfer data!\n");
	// memory
	ptr = (PBYTE) 0xc0000000;
	offset = 0;
	do {
		int bytetosend = ((total_dump_size - offset) > readlen)?readlen:(total_dump_size - offset);
		int realsend = 0;
		//DBGKDUMP_PRINTK ("S");
		if (unlikely((realsend=Serial_Send(ptr+offset, bytetosend)) != bytetosend)) {
			DBGKDUMP_PRINTK("Serial_Send failed at 0x%x, 0x%x!=0x%x! \n", (unsigned int)offset, realsend, bytetosend) ;
			ret = -1;
			break;
		}
		//DBGKDUMP_PRINTK ("R");      
		offset += bytetosend;
        
		if (unlikely(offset >= total_dump_size)) {
			DBGKDUMP_PRINTK("done: 0x%x transfered!\n", (unsigned int)offset);
			ret = 0;
			break;
		}
		//        DBGKDUMP_PRINTK ("sending...\n", offset) ;
	} 
	while (RecvAck() == 0);
    
	RecvAck();
	DBGKDUMP_PRINTK("\ntransfer done!\n");
	return ret; // success.
}
