#include <linux/string.h>
#include "kdump.h"
#include "kdump_elf.h"
#include "kdump_sdhc.h"
#include "sd_misc.h"
#include <asm/setup.h>
#include <linux/vmalloc.h>

FileHandler g_FH;
#define DEVICE_SECTOR_BYTES 512
extern int msdc_init_panic(int dev);
extern int card_dump_func_read(unsigned char* buf, unsigned int len, unsigned long long offset, int dev);
extern int card_dump_func_write(unsigned char* buf, unsigned int len, unsigned long long offset, int dev);



static BOOL ReadSectorFromSD(UINT32 sector_addr,PBYTE pdBuf,INT32 blockLen)
{
    return card_dump_func_read(pdBuf, blockLen * DEVICE_SECTOR_BYTES , 
                               sector_addr * DEVICE_SECTOR_BYTES , 
                               DUMP_INTO_EXTERN_CARD);
}

static BOOL WriteSectorToSD(UINT32 sector_addr, PBYTE pdBuf, INT32 blockLen)
{
	DBGKDUMP_PRINTK(".");
    return card_dump_func_write(pdBuf, blockLen * DEVICE_SECTOR_BYTES, 
                                sector_addr * DEVICE_SECTOR_BYTES, 
                                DUMP_INTO_EXTERN_CARD);
}

FAT_Para m_bFATInfo;
DWORD m_bLastFATPage; 

LfnEntry g_LfnEntry = {
	0x41,								// sequence number
	{'C','\0','E','\0','D','\0','u','\0','m','\0'},		// name characters (five UTF-16 characters)
	0x0F,								// attributes (always 0x0F)
	0x00,								// reserved (alwyas 0x00)
	0xDF,								// checksum of DOS file name
	{'p','\0','.','\0','k','\0','d','\0','m','\0','p','\0'},	// name characters (six UTF-16 characters)
	0x0000,								// word of first cluster (always 0x0000)
	{'\0','\0',0xFF,0xFF}
};

DirEntry g_DirEntry = {
	{'C','E','D','U','M','P','~','1','K','D','M'},	// name
	0x20,							// attr
	0x00,							// NTflags
	0x00,							// createdTimeMsec
	0x63E8,							// createdTime
	0x2E21,							// createdDate
	0x2E21,							// lastAccessDate
	0x0000,							// clusFirstHigh
	0x6490,							// time
	0x2E21,							// date
	0x0000,							// clusFirst
	0x00000000						// size
};

unsigned int OALGetTickCount(void)
{
    return 0 ;
}
static unsigned char ToLower(unsigned char c)
{
    if (c>='A' && c<='Z')
        return c+ 'a' - 'A' ;
    else 
        return c ;
}

BOOL Compare_sd(const u8 *a, const u8 *b, int length)
{
	while (length--)
		if (ToLower(*a++) != ToLower(*b++)) return FALSE;
	return TRUE;
}

DWORD BytesToNum_sd(BYTE *b, int bytes)
{
	DWORD result = 0;
	int i;

	for (i = 0; i < bytes; i++) {
		result |= b[i] << (i << 3);
	}
	return result;
}

DWORD FindBootPartition_sd(PBYTE SectorBuffer )
{
	DWORD PartitionStart;
	DWORD PartitionTpye;
	PartitionTpye = BytesToNum_sd(SectorBuffer + 0x1c2, 1);
	PartitionStart = BytesToNum_sd(SectorBuffer + 0x1c6, 4);

	DBGKDUMP_PRINTK("SDCard: PartitionStart=0x%x ,PartitionTpye=0x%x\n", PartitionStart, PartitionTpye);
	if (PartitionTpye == 5) {
		if (!ReadSectorFromSD(PartitionStart,SectorBuffer,1)) {
			DBGKDUMP_PRINTK("SDCard: can not find BootPosition! \n"); 
			return FALSE;
		}
		PartitionStart += BytesToNum_sd(SectorBuffer + 0x1c6, 4);
		DBGKDUMP_PRINTK("SDCard: PartitionStart=0x%x\n", PartitionStart);
	}
	return PartitionStart;
}

BOOL ReadBootPartition_sd(PBYTE SectorBuffer)
{ 
	m_bFATInfo.BPB_BytsPerSec = BytesToNum_sd(SectorBuffer+11, 2);
	m_bFATInfo.BPB_SecPerClus = BytesToNum_sd(SectorBuffer+13, 1);
	m_bFATInfo.BPB_RsvdSecCnt = BytesToNum_sd(SectorBuffer+14, 2);
	m_bFATInfo.BPB_NumFATs = BytesToNum_sd(SectorBuffer+16, 1);
	m_bFATInfo.BPB_FATSz = BytesToNum_sd(SectorBuffer+22, 2);
	if (m_bFATInfo.BPB_FATSz) {
		m_bFATInfo.FileSysType = FAT_16;		
		m_bFATInfo.BPB_RootEntCnt = BytesToNum_sd(SectorBuffer+17, 2);
		m_bFATInfo.BPB_TotSec = BytesToNum_sd(SectorBuffer+19, 2);
		m_bFATInfo.BPB_RootClus = 0;
		DBGKDUMP_PRINTK("SDCard: FilSysType = FAT16\n");
	}
	else {
		m_bFATInfo.FileSysType = FAT_32;
		m_bFATInfo.BPB_TotSec = BytesToNum_sd(SectorBuffer+32, 4);
		m_bFATInfo.BPB_FATSz = BytesToNum_sd(SectorBuffer+36, 4);
		m_bFATInfo.BPB_RootEntCnt = 0;
		m_bFATInfo.BPB_RootClus = BytesToNum_sd(SectorBuffer+44, 4);
		DBGKDUMP_PRINTK("SDCard: FilSysType = FAT32\n");
	}
	
	DBGKDUMP_PRINTK("SDCard: BPB_BytsPerSec = 0x%04x\n", m_bFATInfo.BPB_BytsPerSec);
	DBGKDUMP_PRINTK("SDCard: BPB_SecPerClus = 0x%02x\n", m_bFATInfo.BPB_SecPerClus);
	DBGKDUMP_PRINTK("SDCard: BPB_RsvdSecCnt = 0x%04x\n", m_bFATInfo.BPB_RsvdSecCnt);
	DBGKDUMP_PRINTK("SDCard: BPB_NumFATs = 0x%02x\n", m_bFATInfo.BPB_NumFATs);
	DBGKDUMP_PRINTK("SDCard: BPB_FATSz = 0x%08x\n", m_bFATInfo.BPB_FATSz);
	DBGKDUMP_PRINTK("SDCard: BPB_RootClus = 0x%08x\n", m_bFATInfo.BPB_RootClus);	
	DBGKDUMP_PRINTK("SDCard: BPB_TotSec = 0x%08x\n", m_bFATInfo.BPB_TotSec);

    if((PAGE_SIZE < m_bFATInfo.BPB_SecPerClus * m_bFATInfo.BPB_BytsPerSec) ||
        ((PAGE_SIZE % (m_bFATInfo.BPB_SecPerClus * m_bFATInfo.BPB_BytsPerSec)) != 0)) {

        DBGKDUMP_PRINTK("Can't support SDCard cluster bytes\n");
        return FALSE;
    }
	return TRUE;
}

BOOL Block0_is_BootSector(PBYTE Ptr)
{
	DWORD BytesPerSec = 0;
	DWORD SecPerClus = 0;
	DWORD BPB_Media = 0;

	BytesPerSec = BytesToNum_sd(Ptr + 11, 2);
	if(!((BytesPerSec == 512)||(BytesPerSec == 1024)||(BytesPerSec == 2048)||(BytesPerSec == 4096))) {
		return FALSE;
	}
	SecPerClus = BytesToNum_sd(Ptr + 13, 1);
	if(!((SecPerClus == 1)||(SecPerClus == 2)||(SecPerClus == 4)||
	   (SecPerClus == 8)||(SecPerClus == 16)||(SecPerClus == 32)||
	   (SecPerClus == 64)||(SecPerClus == 128)) && 
	    (SecPerClus * BytesPerSec <= 0x10000)) {
		return FALSE;
	}
	BPB_Media = BytesToNum_sd(Ptr + 21, 1);
	if(!((BPB_Media == 0xF8)||(BPB_Media == 0xF0)||(BPB_Media == 0xF9)||
	   (BPB_Media == 0xFA)||(BPB_Media == 0xFB)||(BPB_Media == 0xFC)||
	   (BPB_Media == 0xFD)||(BPB_Media == 0xFE)||(BPB_Media == 0xFF))) {
		return FALSE;
	}
	
    return TRUE;
}

BOOL GetBPBInfo_sd(PBYTE Ptr)
{   
	m_bFATInfo.BPB_BytsPerSec = 512;
	m_bFATInfo.BootStartSec = 0;

	//*pSectorPosition
	if (!ReadSectorFromSD(m_bFATInfo.BootStartSec, Ptr, 1)) {
		DBGKDUMP_PRINTK("SDCard: can not find MBR!\n"); 
		return FALSE;
	}

	// Add support block0 is bootPartition
	if (Block0_is_BootSector(Ptr)) {
		m_bFATInfo.BootStartSec = 0;
		DBGKDUMP_PRINTK("SDCard: Get SD Card BootSector : 0 .\n");
	}
	else {
		m_bFATInfo.BootStartSec  = FindBootPartition_sd(Ptr);
		if (!ReadSectorFromSD(m_bFATInfo.BootStartSec, Ptr, 1)) {
			DBGKDUMP_PRINTK("SDCard: can not find BootPosition! \n"); 
			return FALSE;
		}
		if (!Block0_is_BootSector(Ptr)) {
			DBGKDUMP_PRINTK("SDCard: BPB sector dismatch FAT Spec!\n");
			return FALSE;
		}
	}
	if (!ReadBootPartition_sd(Ptr)) {
		DBGKDUMP_PRINTK("SDCard: can not Read BootPartition!\n");
		return FALSE;
	}

	m_bFATInfo.FATStartSec = m_bFATInfo.BootStartSec + m_bFATInfo.BPB_RsvdSecCnt;

	if (m_bFATInfo.FileSysType == FAT_32) {
		m_bFATInfo.ClusStartSec = m_bFATInfo.FATStartSec+(m_bFATInfo.BPB_NumFATs)*(m_bFATInfo.BPB_FATSz);
		m_bFATInfo.RootDirStartSec = m_bFATInfo.ClusStartSec + (m_bFATInfo.BPB_RootClus-2)*(m_bFATInfo.BPB_SecPerClus);
	}
	else {
		m_bFATInfo.RootDirStartSec = m_bFATInfo.FATStartSec+(m_bFATInfo.BPB_NumFATs)*(m_bFATInfo.BPB_FATSz);
		m_bFATInfo.ClusStartSec = m_bFATInfo.RootDirStartSec+32*m_bFATInfo.BPB_RootEntCnt/m_bFATInfo.BPB_BytsPerSec;
	}	

	return TRUE;
}

BOOL FormatRevdSec(FileHandler *pFileHandler)
{
	u32 data_clus_num, rev_sec_start;
	int data_len, temp;

	rev_sec_start = m_bFATInfo.BootStartSec+1;
	
	data_len = m_bFATInfo.BPB_RsvdSecCnt* m_bFATInfo.BPB_BytsPerSec;
	temp = (data_len > buf_size ) ? buf_size  : data_len;
	memset(pFileHandler->FileBuffer, 0, temp * sizeof(BYTE));
	if (m_bFATInfo.FileSysType == FAT_32) {
		// FIS_LeadDig = 0x41615252
		*(pFileHandler->FileBuffer)= 0x52;
		*(pFileHandler->FileBuffer + 1)= 0x52;
		*(pFileHandler->FileBuffer + 2)= 0x61;
		*(pFileHandler->FileBuffer + 3)= 0x41;

		// FIS_StrucDig = 0x61417272
		*(pFileHandler->FileBuffer + 484)= 0x72;
		*(pFileHandler->FileBuffer + 485)= 0x72;
		*(pFileHandler->FileBuffer + 486)= 0x41;
		*(pFileHandler->FileBuffer + 487)= 0x61;

		data_clus_num = pFileHandler->TotalLen/(m_bFATInfo.BPB_BytsPerSec * m_bFATInfo.BPB_SecPerClus); 
		if (pFileHandler->TotalLen % (m_bFATInfo.BPB_BytsPerSec * m_bFATInfo.BPB_SecPerClus))
			data_clus_num ++;

		//FSI_Free_Count = 0xFFFFFFFF, means need count again
		*(unsigned int*)(pFileHandler->FileBuffer + 488) = 0xFFFFFFFF ;//m_bFATInfo.BPB_TotSec/m_bFATInfo.BPB_SecPerClus - data_clus_num;

		
		//FSI_Nxt_free = 0xFFFFFFFF, means need count again
		*(unsigned int*)(pFileHandler->FileBuffer + 492)= 0xFFFFFFFF ; //data_clus_num + m_bFATInfo.BPB_RootClus;

		//FSI_TrailSig = 0xAA550000
		*(pFileHandler->FileBuffer + 510)= 0x55;
		*(pFileHandler->FileBuffer + 511)= 0xAA;
	}

	if (!WriteSectorToSD((m_bFATInfo.BootStartSec+1), pFileHandler->FileBuffer, temp/m_bFATInfo.BPB_BytsPerSec)) {
		DBGKDUMP_PRINTK("SDCard: can not write Rsvd Sectors!\n"); 
		return FALSE;
	}
	data_len -= temp;
	rev_sec_start += temp/m_bFATInfo.BPB_BytsPerSec;
	
	while(data_len > 0) {
		temp = (data_len > buf_size ) ? buf_size  : data_len;
		memset(pFileHandler->FileBuffer, 0, temp * sizeof(BYTE));
		if (!WriteSectorToSD(rev_sec_start, pFileHandler->FileBuffer, temp/m_bFATInfo.BPB_BytsPerSec -1)) {
			DBGKDUMP_PRINTK("SDCard: can not write Rsvd Sectors!\n"); 
			return FALSE;
		}
		data_len -= temp;
		rev_sec_start += temp/m_bFATInfo.BPB_BytsPerSec;
	}	
	
	return TRUE;
}

/*
use 64k buffer write fat table in FAT eara
*/
BOOL FormatFATSec(FileHandler *pFileHandler)
{
	unsigned int i, FAT_len, current_clus=0, use_once, temp1; 
	int fat_left_len, data_clus_left;
	unsigned int data_clus_num;
	unsigned int total_FAT_len, FAT_start_sec;

	total_FAT_len = m_bFATInfo.BPB_FATSz * m_bFATInfo.BPB_BytsPerSec;
	data_clus_num = pFileHandler->TotalLen/(m_bFATInfo.BPB_BytsPerSec * m_bFATInfo.BPB_SecPerClus); 

	if(pFileHandler->TotalLen % (m_bFATInfo.BPB_BytsPerSec * m_bFATInfo.BPB_SecPerClus))
		data_clus_num ++;
	DBGKDUMP_PRINTK("SDCard: file use %d Clusters!\n", data_clus_num);

	if(((m_bFATInfo.BPB_TotSec - m_bFATInfo.BPB_FATSz*m_bFATInfo.BPB_NumFATs)/m_bFATInfo.BPB_SecPerClus- 2) < data_clus_num)
	{
		DBGKDUMP_PRINTK("SDCard: the card is too small to dump file\n"); 
		return FALSE;
	}

	if(m_bFATInfo.FileSysType == FAT_32)
	{
		FAT_len = 4;
		data_clus_left = data_clus_num + 3;	
	}
	else
	{
		FAT_len = 2;
		data_clus_left = data_clus_num + 2;	
	}

    FAT_start_sec = m_bFATInfo.FATStartSec;
	fat_left_len = total_FAT_len;	
	
	use_once = 1;	
	while(fat_left_len > 0) 
	{	
		memset(pFileHandler->FileBuffer, 0, buf_size * sizeof(BYTE));	
		if(data_clus_left > 0)
		{			
			if(use_once == 1) /*first FAT*/
			{
	            if(m_bFATInfo.FileSysType == FAT_32)
				{
					*(unsigned int*)(pFileHandler->FileBuffer) = 0x0FFFFFF8;
					*(unsigned int*)(pFileHandler->FileBuffer + FAT_len) = 0xFFFFFFFF;
					*(unsigned int*)(pFileHandler->FileBuffer + 2*FAT_len) = 0x0FFFFFFF;
	                temp1 =( (data_clus_num + 3)>( buf_size/FAT_len))? buf_size/FAT_len :  (data_clus_num + 3);						
					for(i=3; i< temp1; i++)
					{
						*(unsigned int*)(pFileHandler->FileBuffer + FAT_len*i) = i + 1;
					}
					if(i == (data_clus_num + 3))
					{
						*(unsigned int*)(pFileHandler->FileBuffer + FAT_len*i) = 0x0FFFFFFF;
						data_clus_left = 0;
					}
				}
				else
				{
					*(unsigned short*)(pFileHandler->FileBuffer) = 0xFFF8;
					*(unsigned short*)(pFileHandler->FileBuffer + FAT_len) = 0xFFFF;
					temp1 =( (data_clus_num + 2)>( buf_size/FAT_len))? buf_size/FAT_len : (data_clus_num + 2);	  
					for(i=2; i< temp1; i++)
					{
						*(unsigned short*)(pFileHandler->FileBuffer + FAT_len*i) = i + 1;
					}
					if(i == (data_clus_num + 2))
					{
						*(unsigned short*)(pFileHandler->FileBuffer + FAT_len*i) = 0xFFFF;
						data_clus_left = 0;
					}
				}
				current_clus = buf_size/FAT_len;
				use_once = 0;
				if(data_clus_left)
					data_clus_left -= buf_size/FAT_len;				
				
			}
			else if(data_clus_left > (buf_size/FAT_len))
	        {
		       temp1 = buf_size/FAT_len;
				if(m_bFATInfo.FileSysType == FAT_32)
				{
					for(i=0; i< temp1; i++)
					{
						*(unsigned int*)(pFileHandler->FileBuffer + FAT_len*i) = current_clus + i+ 1 ;
					}					
				}
				else
				{					
					for(i=0; i< temp1; i++)
					{
						*(unsigned short*)(pFileHandler->FileBuffer +FAT_len*i) = current_clus + i + 1 ;
					}					
				}
				current_clus += buf_size/FAT_len;
				data_clus_left -= buf_size/FAT_len;
	        }
			else // last make FAT.
			{		    
	            temp1 = data_clus_left;				
				if(m_bFATInfo.FileSysType == FAT_32)
				{						
					for(i=0; i< temp1; i++)
					{
						*(unsigned int*)(pFileHandler->FileBuffer + FAT_len*i) = current_clus + i +1;
					}
					*(unsigned int*)(pFileHandler->FileBuffer + FAT_len*i) = 0x0FFFFFFF;
				}
				else
				{
					for(i=0; i< temp1; i++)
					{
						*(unsigned short*)(pFileHandler->FileBuffer + FAT_len*i) = current_clus + i +1;
					}
					*(unsigned short*)(pFileHandler->FileBuffer + FAT_len*i) = 0xFFFF;
				}
				current_clus += i;
				data_clus_left = 0;
			}			
	
		}
		temp1 = ((fat_left_len > buf_size)? buf_size : fat_left_len)/m_bFATInfo.BPB_BytsPerSec;
		if(!WriteSectorToSD(FAT_start_sec, pFileHandler->FileBuffer, temp1)) 
		{
			DBGKDUMP_PRINTK("SDCard: can not write FAT1 Sectors!\n"); 
			return FALSE;
		}
		for( i=1; i< m_bFATInfo.BPB_NumFATs; i++)
		{
			if(!WriteSectorToSD((FAT_start_sec + m_bFATInfo.BPB_FATSz*i), pFileHandler->FileBuffer,temp1)) 
			{
				DBGKDUMP_PRINTK("SDCard: can not write FAT%d Sectors!\n", i); 
				return FALSE;
			}
		}
		fat_left_len -= (temp1*m_bFATInfo.BPB_BytsPerSec);
		FAT_start_sec += temp1;
		
	}
	DBGKDUMP_PRINTK("SDCard:write FAT Sectors done\n");
	return TRUE;
}

BOOL WriteRootDir(FileHandler *pFileHandler, unsigned int dir_clus_len)
{
	unsigned int data_len;
	data_len = m_bFATInfo.BPB_SecPerClus* m_bFATInfo.BPB_BytsPerSec * dir_clus_len;
	memset(pFileHandler->FileBuffer, 0, data_len * sizeof(BYTE));
    
	//File name {'K','D','U','M','P',' ',' ',' ',' ',' ',' '}
	*(pFileHandler->FileBuffer) = 0x4C;
	*(pFileHandler->FileBuffer + 1) = 0x45;
	*(pFileHandler->FileBuffer + 2) = 0x56;
	*(pFileHandler->FileBuffer + 3) = 0x4e;
	*(pFileHandler->FileBuffer + 4) = 0x51;
	*(pFileHandler->FileBuffer + 5) = 0x20;
	*(pFileHandler->FileBuffer + 6) = 0x20;
	*(pFileHandler->FileBuffer + 7) = 0x20;
	*(pFileHandler->FileBuffer + 8) = 0x20;
	*(pFileHandler->FileBuffer + 9) = 0x20;
	*(pFileHandler->FileBuffer +10) = 0x20;
	
	//File Dir Attr
	*(pFileHandler->FileBuffer + 11) = 0x20;

	//File create time
	*(unsigned short*)(pFileHandler->FileBuffer + 14) = 0x63E8;
	*(unsigned short*)(pFileHandler->FileBuffer + 16) = 0x2E21;
	*(unsigned short*)(pFileHandler->FileBuffer + 18) = 0x2E21;
	*(unsigned short*)(pFileHandler->FileBuffer + 22) = 0x6490;
	*(unsigned short*)(pFileHandler->FileBuffer + 24) = 0x2E21;

	//File first clus and file size
	if (m_bFATInfo.FileSysType == FAT_32)
		*(unsigned short*)(pFileHandler->FileBuffer + 26) = m_bFATInfo.BPB_RootClus + dir_clus_len;
	else
		*(unsigned short*)(pFileHandler->FileBuffer + 26) = 0x2;

	*(unsigned int*)(pFileHandler->FileBuffer + 28) = pFileHandler->TotalLen;	

	if (!WriteSectorToSD((m_bFATInfo.RootDirStartSec), pFileHandler->FileBuffer, dir_clus_len * m_bFATInfo.BPB_SecPerClus)) {
		DBGKDUMP_PRINTK("SDCard: can not Write RootDir cluster!\n"); 
		return FALSE;
	}	
	
	return TRUE;
}

BOOL FormatFATToSDCard(FileHandler *pFileHandler, unsigned int dump_total_size)
{
	// init File Handler
	memset(pFileHandler, 0, sizeof(FileHandler));

	if (!GetBPBInfo_sd(pFileHandler->FileBuffer)) {
		DBGKDUMP_PRINTK("SDCard: Failed to GetBPBInfo_sd \n");  
		return FALSE;
	}	
	DBGKDUMP_PRINTK("SDCard: FATStartSec = %d \n", m_bFATInfo.FATStartSec);
	DBGKDUMP_PRINTK("SDCard: RootDirStartSec = %d \n", m_bFATInfo.RootDirStartSec);
    	
	pFileHandler->TotalLen = dump_total_size;
	DBGKDUMP_PRINTK("SDCard: file size = %dMB \n", pFileHandler->TotalLen/(1024*1024));

	if (!FormatRevdSec(pFileHandler)) {
		DBGKDUMP_PRINTK("SDCard: Failed to FormatRevdSec\n");  
		return FALSE;
	}
	
	if (!FormatFATSec(pFileHandler)) {
		DBGKDUMP_PRINTK("SDCard: Failed to FormatFATSec\n");  
		return FALSE;
	}
    
	if (!WriteRootDir(pFileHandler, 1)) {
		DBGKDUMP_PRINTK("SDCard: Failed to WriteRootDir\n");  
		return FALSE;
	}	
	DBGKDUMP_PRINTK("SDCard: format FAT done!\n"); 
	return TRUE;
}


BOOL WriteDataToSDCard(FileHandler *pFileHandler, PBYTE ptr, unsigned int data_len, 
                       unsigned int start_offset, int *pOffset)
{

	unsigned int data_start_sec, byte_per_clus ,temp; 
	int left_len = data_len;
	unsigned int every_write_secs = 256;
	unsigned int every_write_bytes = every_write_secs * m_bFATInfo.BPB_BytsPerSec;
  	*pOffset = 0;
    
	if (m_bFATInfo.FileSysType == FAT_16)
		data_start_sec = m_bFATInfo.ClusStartSec;
	else
		data_start_sec = m_bFATInfo.RootDirStartSec + m_bFATInfo.BPB_SecPerClus;
	data_start_sec += start_offset / m_bFATInfo.BPB_BytsPerSec;

	byte_per_clus = m_bFATInfo.BPB_SecPerClus * m_bFATInfo.BPB_BytsPerSec;

	while(left_len > 0)
	{
		if(left_len >= byte_per_clus)
		{
			if(left_len >= every_write_bytes)
				temp = every_write_bytes;
			else if(left_len >= buf_size)
				temp = buf_size;
			else 
				temp = byte_per_clus;
			if(!WriteSectorToSD(data_start_sec, ptr, temp/m_bFATInfo.BPB_BytsPerSec)) 
			{
				DBGKDUMP_PRINTK("SDCard: write dump failed, left %dBytes not dumped.\n", left_len) ; 
				return FALSE;
			}
		}
		else
		{
			temp = left_len;
			memset(pFileHandler->FileBuffer, 0, byte_per_clus * sizeof(BYTE));
			memcpy(pFileHandler->FileBuffer, ptr, temp);
			if(!WriteSectorToSD(data_start_sec, pFileHandler->FileBuffer, m_bFATInfo.BPB_SecPerClus)) 
			{
				DBGKDUMP_PRINTK("SDCard: write last cluster failed\n") ; 
				return FALSE;
			}
		}	
					
		left_len -= temp;
		data_start_sec += temp/m_bFATInfo.BPB_BytsPerSec;
		ptr += temp;
		*pOffset += temp;				
	}	

    /*empty write loop here too much*/
	//DBGKDUMP_PRINTK("SDCard: write data done!\n");
	return TRUE;
}

BOOL WriteZeroBlock0(FileHandler *pFileHandler)
{
	memset(pFileHandler->FileBuffer, 0, m_bFATInfo.BPB_BytsPerSec * sizeof(BYTE));
    
	if (!WriteSectorToSD(0, pFileHandler->FileBuffer, 1)) {
		DBGKDUMP_PRINTK("SDCard: can not Write zero to block0!\n"); 
		return FALSE;
	}	 
	return TRUE;
}

DWORD FindFirstClusInFAT_sd(DWORD StartClusNum, PBYTE Ptr)
{
	DWORD SectorNum;
	DWORD NextClusterPosition;
	DWORD BytsPerFAT;
	DWORD BytsPerAdd;
	if(m_bFATInfo.FileSysType == FAT_32)
		BytsPerAdd = 4;//FAT32	
	else
		BytsPerAdd = 2;//FAT16
	
	BytsPerFAT = m_bFATInfo.BPB_BytsPerSec/BytsPerAdd;
	
	m_bLastFATPage = StartClusNum/BytsPerFAT;
	SectorNum = m_bFATInfo.FATStartSec+m_bLastFATPage;
#ifdef SD_DATA_PRINT
	DBGKDUMP_PRINTK("SDCard: now FATSec = %d \n",SectorNum);
#endif
	if (!ReadSectorFromSD(SectorNum,Ptr,1)) {
		DBGKDUMP_PRINTK("SDCard: can not FindFirstClusInFAT_sd!\n"); 
		return FALSE;
	}

	NextClusterPosition = BytesToNum_sd(Ptr+BytsPerAdd * (StartClusNum%BytsPerFAT), BytsPerAdd);
#ifdef SD_DATA_PRINT
	DBGKDUMP_PRINTK("SDCard: NextClusterPosition = %08x\n", NextClusterPosition);
#endif
	return NextClusterPosition;
}

DWORD FindNextClusInFAT_sd(DWORD StartClusNum,PBYTE Ptr)
{
	//int	j;
	DWORD SectorNum;
	DWORD NextClusterPosition;
	DWORD BytsPerFAT;
	DWORD BytsPerAdd;
	DWORD TempPage;
	if(m_bFATInfo.FileSysType==FAT_32)
	    BytsPerAdd = 4;//FAT32	
	else
	    BytsPerAdd = 2;//FAT16
	
	BytsPerFAT = m_bFATInfo.BPB_BytsPerSec/BytsPerAdd;	
	TempPage = StartClusNum/BytsPerFAT;
	if(TempPage!=m_bLastFATPage)
	{	    
		SectorNum = m_bFATInfo.FATStartSec+TempPage;
#ifdef SD_DATA_PRINT
		DBGKDUMP_PRINTK("SDCard: now FATSec = %d \n",SectorNum);
#endif
		if(!ReadSectorFromSD(SectorNum,Ptr,1)) 
		{
			DBGKDUMP_PRINTK("SDCard: can not FindNextClusInFAT_sd! \n"); 
			return FALSE;
		}
		m_bLastFATPage=TempPage;
/*        for(j=0;j<(int)m_bFATInfo.BPB_BytsPerSec;j++)
            {
                if((j%16)==0) DBGKDUMP_PRINTK("i= 0x%04x    ",j);
            	DBGKDUMP_PRINTK(" %02x ", Ptr[j]);
                if(((j+1)%16)==0) DBGKDUMP_PRINTK("     i= %d\n",j);
                
            }*/
    
	}
	NextClusterPosition=BytesToNum_sd(Ptr+BytsPerAdd*(StartClusNum%BytsPerFAT),BytsPerAdd);
#ifdef SD_DATA_PRINT
	DBGKDUMP_PRINTK("SDCard: NextClusterPosition = %08x \n",NextClusterPosition);
#endif
	return NextClusterPosition;
}

DWORD FindFirstFreeClusInFAT_sd(FileHandler *pFileHandler)
{
	DWORD i;
	DWORD SectorNum;
	DWORD FreeClusterNum;
	DWORD value;
	DWORD FATSector, FATOffset;
	DWORD EntryPerSector;	
	DWORD BytsPerAdd;
	
	if(m_bFATInfo.FileSysType==FAT_32)
		BytsPerAdd = 4;//FAT32	
	else
		BytsPerAdd = 2;//FAT16

	FreeClusterNum = 0;
	EntryPerSector = m_bFATInfo.BPB_BytsPerSec/BytsPerAdd;
	FATSector = pFileHandler->CurrClusterNum/EntryPerSector;
	FATOffset = pFileHandler->CurrClusterNum%EntryPerSector + 1;	 // start search frome next cluster
	
	// for safty, we don't use all FAT entries, just reserve the last FAT sector
	while (FATSector < (m_bFATInfo.BPB_FATSz-1))
	{
		// Read new FAT sector to cache
		SectorNum = m_bFATInfo.FATStartSec+FATSector;
		if (!ReadSectorFromSD(SectorNum, pFileHandler->FATBuffer, 1))
		{
			DBGKDUMP_PRINTK("SDCard: DeleteFile_sd ReadSectorFromSD failed \n"); 
			return 0;
		}

		for (i=FATOffset; i<EntryPerSector; i++) 
		{
			value = BytesToNum_sd(pFileHandler->FATBuffer+i*BytsPerAdd, BytsPerAdd);
			if (value == 0) {
				FreeClusterNum = FATSector*EntryPerSector + i;	// found free entry in FAT
				pFileHandler->FATSector = FATSector;
				return FreeClusterNum;
			}
		}
		// try next FAT sector
		FATSector++;
		FATOffset = 0;
	}

	return 0;
}

DWORD ChainFreeClusInFAT_sd(FileHandler *pFileHandler)
{
	DWORD i;
	DWORD SectorNum;
	DWORD FreeClusterNum;
	DWORD value;
	DWORD CurrFATSector, CurrFATOffset;
	DWORD NextFATSector, NextFATOffset;
	DWORD EntryPerSector;	
	DWORD BytsPerAdd;
	BYTE     TempFAT[512];
	BYTE     *pBuf;
	
	if(m_bFATInfo.FileSysType==FAT_32)
		BytsPerAdd = 4;//FAT32	
	else
		BytsPerAdd = 2;//FAT16

	FreeClusterNum = 0;
	EntryPerSector = m_bFATInfo.BPB_BytsPerSec/BytsPerAdd;
	CurrFATSector = pFileHandler->CurrClusterNum/EntryPerSector;
	CurrFATOffset = pFileHandler->CurrClusterNum%EntryPerSector;
	NextFATSector = CurrFATSector;
	NextFATOffset = CurrFATOffset+1;	// start search frome next cluster
	memset(TempFAT,0,512);

	pBuf = pFileHandler->FATBuffer;
	// for safty, we don't use all FAT entries, just reserve the last FAT sector
	while (NextFATSector < (m_bFATInfo.BPB_FATSz-1))
	{
		if (NextFATSector != pFileHandler->FATSector)
		{
			// FAT sector changed, read new FAT sector to temp buffer
			pBuf = TempFAT;
			SectorNum = m_bFATInfo.FATStartSec+NextFATSector;
			if (!ReadSectorFromSD(SectorNum, pBuf, 1))
			{
				DBGKDUMP_PRINTK("SDCard: DeleteFile_sd ReadSectorFromSD failed \n"); 
				return 0;
			}
		}
		// find free cluster in FAT cache
		for (i=NextFATOffset; i<EntryPerSector; i++) 
		{
			value = BytesToNum_sd(pBuf+i*BytsPerAdd, BytsPerAdd);
			if (value == 0) {
				FreeClusterNum = NextFATSector*EntryPerSector + i;	// found free entry in FAT
				// commit FAT entry
				if(m_bFATInfo.FileSysType==FAT_32)
				{

					*(DWORD*)(pFileHandler->FATBuffer+CurrFATOffset*BytsPerAdd) = (DWORD)FreeClusterNum;
				}
				else
				{
					*(WORD*)(pFileHandler->FATBuffer+CurrFATOffset*BytsPerAdd) = (WORD)FreeClusterNum;
				}

				break;
			}
		}

		if (FreeClusterNum!=0) {
			break;
		}
		
		// try next FAT sector
		NextFATSector++;
		NextFATOffset = 0;
	}

	// check if need to refresh cache
	if ((FreeClusterNum != 0) && (NextFATSector != pFileHandler->FATSector))
	{
		// FAT sector changed, flush cache to SD
		SectorNum = m_bFATInfo.FATStartSec+CurrFATSector;
		WriteSectorToSD(SectorNum, pFileHandler->FATBuffer, 1);
		
		// copy temp buffer to cache
		memcpy(pFileHandler->FATBuffer, TempFAT, 512);
		pFileHandler->FATSector = NextFATSector;
	}

	return FreeClusterNum;
}

BOOL MarkEndClusInFAT_sd(FileHandler *pFileHandler)
{
	DWORD SectorNum;
	DWORD BytsPerAdd;
	DWORD EntryPerSector;
	DWORD FATSector;
	DWORD FATOffset;

	if(m_bFATInfo.FileSysType==FAT_32)
		BytsPerAdd = 4;//FAT32	
	else
		BytsPerAdd = 2;//FAT16	

	EntryPerSector = m_bFATInfo.BPB_BytsPerSec/BytsPerAdd;
	FATSector = pFileHandler->CurrClusterNum/EntryPerSector;
	FATOffset = pFileHandler->CurrClusterNum%EntryPerSector;
	SectorNum = m_bFATInfo.FATStartSec+FATSector;
	
	if (FATSector != pFileHandler->FATSector)
	{
		if (!ReadSectorFromSD(SectorNum, pFileHandler->FATBuffer, 1))
		{
			DBGKDUMP_PRINTK("SDCard: MarkEndClusInFAT_sd ReadSectorFromSD failed \n"); 
			return FALSE;
		}
	}

	// mark last flie cluster in FAT
	if(m_bFATInfo.FileSysType==FAT_32)
	{

		*(DWORD*)(pFileHandler->FATBuffer+FATOffset*BytsPerAdd) = 0x0FFFFFFF;// 0xFFFFFF8
	}
	else
	{
		*(WORD*)(pFileHandler->FATBuffer+FATOffset*BytsPerAdd) = 0xFFFF;//0xFFF8
	}
	

	// flush FAT cache to SD
	if (!WriteSectorToSD(SectorNum, pFileHandler->FATBuffer, 1))
	{
		DBGKDUMP_PRINTK("SDCard: MarkEndClusInFAT_sd WriteSectorToSD failed \n"); 
		return FALSE;
	}
	return TRUE;
}

BOOL DeleteFileInFAT_sd(FileHandler *pFileHandler)
{
	DWORD StartClusNum;
	DWORD SectorNum;
	DWORD NextClusNum;
	DWORD FATSector, FATOffset;
	DWORD EntryPerSector;
	DWORD BytsPerAdd;
	BOOL    NewSector = TRUE;
	BOOL    LastEntry = FALSE;

	if(m_bFATInfo.FileSysType==FAT_32)
	{
		BytsPerAdd = 4;//FAT32	
	}
	else
	{
		BytsPerAdd = 2;//FAT16
	}

	EntryPerSector = m_bFATInfo.BPB_BytsPerSec/BytsPerAdd;
	StartClusNum = pFileHandler->CurrClusterNum;
	SectorNum = m_bFATInfo.FATStartSec+StartClusNum/EntryPerSector;

	while (!LastEntry)
	{
		FATSector = StartClusNum/EntryPerSector;
		FATOffset = StartClusNum%EntryPerSector;

		if (NewSector) 
		{
			SectorNum = m_bFATInfo.FATStartSec+FATSector;
#ifdef SD_DATA_PRINT
	    DBGKDUMP_PRINTK("SDCard: now FATSec = %d \n",SectorNum);
#endif
			if(!ReadSectorFromSD(SectorNum, pFileHandler->FATBuffer, 1)) 
			{
				DBGKDUMP_PRINTK("SDCard: DeleteFile_sd ReadSectorFromSD failed \n"); 
				return FALSE;
			}
			NewSector = FALSE;
		}
		
		NextClusNum = BytesToNum_sd(pFileHandler->FATBuffer+FATOffset*BytsPerAdd, BytsPerAdd);
	

		// Release FAT entry
		if(m_bFATInfo.FileSysType==FAT_32)
		{

			*(DWORD*)(pFileHandler->FATBuffer+FATOffset*BytsPerAdd) = 0x0;
			if ((NextClusNum >= 0xFFFFFF8) || (NextClusNum == 0)) {
				LastEntry = TRUE;
			}
		}
		else
		{
			*(WORD*)(pFileHandler->FATBuffer+FATOffset*BytsPerAdd) = 0x0;
			if ((NextClusNum >= 0xFFF8) || (NextClusNum == 0)) {
				LastEntry = TRUE;
			}
		}

		if (NextClusNum/EntryPerSector != FATSector)
		{
			NewSector = TRUE;	// next cluster is not at current FAT Sector, write current FAT sector back
		}

		if (NewSector || LastEntry)
		{
			if (!WriteSectorToSD(SectorNum, pFileHandler->FATBuffer, 1))
			{
				DBGKDUMP_PRINTK("SDCard: DeleteFile_sd WriteSectorToSD failed \n"); 
				return FALSE;
			}
		}

		StartClusNum = NextClusNum;
	}
	
#ifdef SD_DATA_PRINT
	DBGKDUMP_PRINTK("SDCard: NextClusterPosition = %08x \n",NextClusterPosition);
#endif

	return TRUE;
}

static BOOL OpenDumpFile_sd(FileHandler *pFileHandler)
{
	int i, j, times;
	DWORD SectorNum;
	DWORD Temp;
	DWORD dwStartTick;
	DWORD FindFile_TIMEOUT = 60000;
	int SecLen;
	DWORD NextRootFAT = 0;
	BYTE RootDirFAT[512];
	BOOL foundLfn = FALSE;

	// init File Handler
	memset(pFileHandler, 0, sizeof(FileHandler));
    pFileHandler->ISFILE = 1;
	if(!GetBPBInfo_sd(pFileHandler->FileBuffer))
	{
		DBGKDUMP_PRINTK("SDCard: Failed to GetBPBInfo_sd \n");  
		return FALSE;
	}
	
	dwStartTick = OALGetTickCount(); 

	DBGKDUMP_PRINTK("SDCard: FATStartSec = %d \n",m_bFATInfo.FATStartSec);
	DBGKDUMP_PRINTK("SDCard: RootDirStartSec = %d \n", m_bFATInfo.RootDirStartSec);
	SectorNum = m_bFATInfo.RootDirStartSec;

	if(m_bFATInfo.FileSysType==FAT_16)
	{
		SecLen=32;
		times=1;
	}
	else
	{
		SecLen = m_bFATInfo.BPB_SecPerClus;
		times=MaxFindFileClusNum;
	}

	while(times)
	{
		for (i=0; i <SecLen; i++)
		{
			if(!ReadSectorFromSD(SectorNum, pFileHandler->FileBuffer, 1)) 
			{
				DBGKDUMP_PRINTK("SDCard: can not read RootDir! \n"); 
				return FALSE;
			}

			for(j=0; j<(int)m_bFATInfo.BPB_BytsPerSec; j+=32)
			{
				if (foundLfn)
				{
					Temp = BytesToNum_sd(pFileHandler->FileBuffer+j+20, 2);
					pFileHandler->CurrClusterNum |= (Temp<<16);
					Temp = BytesToNum_sd(pFileHandler->FileBuffer+j+26, 2);
					pFileHandler->CurrClusterNum |= (Temp&0xFFFF);
					
					// delete file DIR entries
					memset(pFileHandler->FileBuffer+j, 0, 32);
					if(!WriteSectorToSD(SectorNum, pFileHandler->FileBuffer, 1)) 
					{
						DBGKDUMP_PRINTK("SDCard: can not write directory entry!\n"); 
						return FALSE;
					}
					// delete file in FAT entries
					if(!DeleteFileInFAT_sd(pFileHandler))
					{
						DBGKDUMP_PRINTK("SDCard: can not delete file in FAT entries!\n"); 
						return FALSE;
					}
					goto Done;
				}
				else if (pFileHandler->FileBuffer[j] == 0x41 &&
					Compare_sd(pFileHandler->FileBuffer+j+1, g_LfnEntry.name1, 10) &&
					Compare_sd(pFileHandler->FileBuffer+j+14, g_LfnEntry.name2, 12))
				{
					// delete LFN entries
					memset(pFileHandler->FileBuffer+j, 0, 32);
					if(!WriteSectorToSD(SectorNum, pFileHandler->FileBuffer, 1)) 
					{
						DBGKDUMP_PRINTK("SDCard: can not write LFN entry!\n"); 
						return FALSE;
					}
					foundLfn = TRUE;
				}
			}
			SectorNum+=1;
		}
		if((OALGetTickCount() - dwStartTick) >= FindFile_TIMEOUT)
		{
			DBGKDUMP_PRINTK("SDCard: Find File Error timeout!!!  \n");
			return FALSE;
		}
		if(m_bFATInfo.FileSysType==FAT_32)
		{    
			if(times==MaxFindFileClusNum)
			{ 
				NextRootFAT=FindFirstClusInFAT_sd(m_bFATInfo.BPB_RootClus, RootDirFAT);
				if(NextRootFAT>=0xFFFFFF8)
				{
					DBGKDUMP_PRINTK("SDCard: Dump file not exsited.\n");
					break;
				}
			}
			else
			{    
				NextRootFAT=FindNextClusInFAT_sd(NextRootFAT, RootDirFAT);
				if(NextRootFAT>=0xFFFFFF8)
				{
					DBGKDUMP_PRINTK("SDCard: Dump file not exsited.\n");
					break;
				}   
			}       
			DBGKDUMP_PRINTK("SDCard: NextRootFAT=0x%08x  \n", NextRootFAT);
			SectorNum=m_bFATInfo.ClusStartSec + (NextRootFAT-2)*(m_bFATInfo.BPB_SecPerClus);  
		}	
		else {
			// FAT16
			break;
		}
		times--;
	}
	
Done:
	// return a free cluster to create new file
	pFileHandler->CurrClusterNum = 2;
	pFileHandler->CurrClusterNum = FindFirstFreeClusInFAT_sd(pFileHandler);
	pFileHandler->PrevClusterNum = pFileHandler->CurrClusterNum;
	g_DirEntry.clusFirstHigh = (WORD)((pFileHandler->CurrClusterNum >> 16) & 0xFFFF);
	g_DirEntry.clusFirst = (WORD)(pFileHandler->CurrClusterNum & 0xFFFF);
	if(!pFileHandler->CurrClusterNum)
	{
		pFileHandler->DiskFull = TRUE;
		DBGKDUMP_PRINTK("SDCard: free cluster not found!\n");
		return FALSE;
	}
	
	return TRUE;
}



BOOL UpdateDirectoryEntry_sd(FileHandler *pFileHandler)
{
	int i, j, times;
	DWORD SectorNum;
	DWORD dwStartTick;
	DWORD FindFile_TIMEOUT = 60000;
	int SecLen;
	DWORD NextRootFAT = 0;
	BYTE RootDirFAT[512];

	dwStartTick = OALGetTickCount(); 
	DBGKDUMP_PRINTK("SDCard: FATStartSec = %d \n",m_bFATInfo.FATStartSec);
	DBGKDUMP_PRINTK("SDCard: RootDirStartSec = %d \n", m_bFATInfo.RootDirStartSec);
	SectorNum = m_bFATInfo.RootDirStartSec;

	if(m_bFATInfo.FileSysType==FAT_16)
	{
		SecLen=32;
		times=1;
	}
	else
	{
		SecLen = m_bFATInfo.BPB_SecPerClus;
		times=MaxFindFileClusNum;
	}

	while(times)
	{
		for (i=0; i <SecLen; i++)
		{
			if(!ReadSectorFromSD(SectorNum, pFileHandler->FileBuffer, 1)) 
			{
				DBGKDUMP_PRINTK("SDCard: can not read RootDir! \n"); 
				return FALSE;
			}

			for(j=0; j<(int)(m_bFATInfo.BPB_BytsPerSec-32); j+=32)
			{
				if ((pFileHandler->FileBuffer[j] == 0x0 || pFileHandler->FileBuffer[j] == 0xE5) &&
					(pFileHandler->FileBuffer[j+32] == 0x0 || pFileHandler->FileBuffer[j+32] == 0xE5))
				{
					memcpy(pFileHandler->FileBuffer+j, &g_LfnEntry, sizeof(LfnEntry));
					memcpy(pFileHandler->FileBuffer+j+32, &g_DirEntry, sizeof(DirEntry));

					if(!WriteSectorToSD(SectorNum, pFileHandler->FileBuffer, 1)) 
					{
						DBGKDUMP_PRINTK("SDCard: can not read RootDir! \n"); 
						return FALSE;
					}
					return TRUE;
				}
			}
			SectorNum+=1;
		}
		if((OALGetTickCount() - dwStartTick) >= FindFile_TIMEOUT)
		{
			DBGKDUMP_PRINTK("SDCard: Find File Error timeout!!!  \n");
			return FALSE;
		}
		if(m_bFATInfo.FileSysType==FAT_32)
		{    
			if(times==MaxFindFileClusNum)
			{ 
				NextRootFAT=FindFirstClusInFAT_sd(m_bFATInfo.BPB_RootClus, RootDirFAT);
				if(NextRootFAT>=0xFFFFFF8)
				{
					DBGKDUMP_PRINTK("SDCard: Dump file not exsited.\n");
					break;
				}
			}
			else
			{    
				NextRootFAT=FindNextClusInFAT_sd(NextRootFAT, RootDirFAT);
				if(NextRootFAT>=0xFFFFFF8)
				{
					DBGKDUMP_PRINTK("SDCard: Dump file not exsited.\n");
					break;
				}   
			}       
			DBGKDUMP_PRINTK("SDCard: NextRootFAT=0x%08x  \n", NextRootFAT);
			SectorNum=m_bFATInfo.ClusStartSec + (NextRootFAT-2)*(m_bFATInfo.BPB_SecPerClus);  
		}	
		else {
			// FAT16
			break;
		}
		times--;
	}
	
	return FALSE;
}

static BOOL WriteDumpFile_sd(FileHandler *pFileHandler, BYTE *Ptr, DWORD Length, DWORD Total)
{
	DWORD i;
	DWORD ClusterSize;
	DWORD SectorNum;
	DWORD FreeClusterNum;
	BYTE val;

//    DBGKDUMP_PRINTK("WriteDumpFile_sd (0x%x, 0x%x, %d, %d)\n", pFileHandler, Ptr, Length, Total);
	
	ClusterSize = m_bFATInfo.BPB_SecPerClus * m_bFATInfo.BPB_BytsPerSec;
	
	while (Length > 0)
	{
		// for every cluster boundary, check disk free space
		if (pFileHandler->DiskFull)
		{
			return FALSE;
		}	

		for(i=pFileHandler->BufferLen; (i<ClusterSize) && (Length>0); i++) {
			val = *Ptr++;
			pFileHandler->FileBuffer[pFileHandler->BufferLen++] = val;
			pFileHandler->CheckSum += val;
			pFileHandler->TotalLen++;
			Length--;
		}

		// a cluster collected, flush to SD
		if (pFileHandler->BufferLen == ClusterSize)
		{
			pFileHandler->BufferLen = 0;
			SectorNum=m_bFATInfo.ClusStartSec + (pFileHandler->CurrClusterNum-2)*(m_bFATInfo.BPB_SecPerClus); 
			//DBGKDUMP_PRINTK("SDCard: WriteDumpFile_sd() write sd card from %d blocks!\n",SectorNum);
//			WriteDebugByte_sd('.', pFileHandler->TotalLen, Total);
			if(!WriteSectorToSD(SectorNum, pFileHandler->FileBuffer, m_bFATInfo.BPB_SecPerClus))
			{
				DBGKDUMP_PRINTK("SDCard: WriteDumpFile_sd() write file content from %d blocks failed!!!!\n",SectorNum); 
				return FALSE;
			}
			pFileHandler->PrevClusterNum = pFileHandler->CurrClusterNum;
			FreeClusterNum =  ChainFreeClusInFAT_sd(pFileHandler);
			if (FreeClusterNum == 0) {
				pFileHandler->DiskFull = TRUE;
				DBGKDUMP_PRINTK("SDCard: WriteDumpFile_sd() disk full!\n"); 
				return FALSE;
			}
			pFileHandler->CurrClusterNum =FreeClusterNum;
		}
		else if (pFileHandler->BufferLen > ClusterSize)
		{
			DBGKDUMP_PRINTK("SDCard: WriteDumpFile_sd() BufferLen error!\n"); 
		}
	}
	
	return TRUE;
}

static BOOL CloseDumpFile_sd(FileHandler *pFileHandler)
{
	DWORD SectorNum;
	WORD year;

	g_DirEntry.size = pFileHandler->TotalLen;

/*
	g_DirEntry.createdTime = g_DirEntry.time = (((INREG16(&m_pRTCRegs->RTC_TC_HOU) << 11) & 0xF800) |
											((INREG16(&m_pRTCRegs->RTC_TC_MIN) << 5) & 0x7E0) |
											((INREG16(&m_pRTCRegs->RTC_TC_SEC)>>1) & 0x1F));
	year = INREG16(&m_pRTCRegs->RTC_TC_YEA);
	year = (year > 20) ? (year-20) : 0;
	g_DirEntry.createdDate = g_DirEntry.date = (((year<<9) & 0xFE00) |
											((INREG16(&m_pRTCRegs->RTC_TC_MTH)<<5) & 0x1E0) |
											(INREG16(&m_pRTCRegs->RTC_TC_DOM) & 0x1F));
*/											
	g_DirEntry.createdTime = g_DirEntry.time = 0 ;
	year = 0 ;
//	year = (year > 20) ? (year-20) : 0;
	g_DirEntry.createdDate = g_DirEntry.date = 0 ;

	if (pFileHandler->BufferLen == 0)
	{
		// the free cluster is not used, so that the last cluster should be the previous one
		pFileHandler->CurrClusterNum = pFileHandler->PrevClusterNum;
	}
	else if (!pFileHandler->DiskFull)
	{
		// flush the reset data
		SectorNum=m_bFATInfo.ClusStartSec + (pFileHandler->CurrClusterNum-2)*(m_bFATInfo.BPB_SecPerClus); 
		if(!WriteSectorToSD(SectorNum, pFileHandler->FileBuffer, m_bFATInfo.BPB_SecPerClus))
		{
			DBGKDUMP_PRINTK("SDCard: CloseFile_sd() write file content failed!\n"); 
			return FALSE;
		}
	}

	if(!MarkEndClusInFAT_sd(pFileHandler))
	{
		DBGKDUMP_PRINTK("SDCard: CloseFile_sd() MarkEndClusInFAT_sd failed!\n"); 
		return FALSE;
	}	

	if(!UpdateDirectoryEntry_sd(pFileHandler))
	{
		DBGKDUMP_PRINTK("SDCard: CloseFile_sd() UpdateDirectoryEntry_sd failed!\n"); 
		return FALSE;
	}	
	
	return TRUE;
}

/*
 * Create a standalone file under SDCARD
 */

extern unsigned int get_memory_size(void);
static int is_valid_ram(unsigned addr)
{
    if ((PHYS_OFFSET <= addr) && (addr < (PHYS_OFFSET + get_memory_size()))) {
        return 1;
    }else {
        return 0;
    }

}
extern struct mm_struct init_mm;
static bool is_addr_valid_map(unsigned addr)
{
    struct mm_struct *mm = &init_mm;
    pgd_t *pgd;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;
    unsigned int pa = 0;
    /*check is this page maps to valid RAM?*/
    pgd = pgd_offset(mm, addr); 
    if (pgd_none(*pgd) || pgd_bad(*pgd)) {
        return 0;
    }

    pud = pud_offset(pgd, addr);
    if (pud_none(*pud) || pud_bad(*pud)) {
        return 0;
    }
    
    pmd = pmd_offset(pud, addr);
    if (pmd_none(*pmd) || pmd_bad(*pmd)) {
        return 0;
    }

    pte = pte_offset_map(pmd, addr); 

    if(pte_present(*pte)) {
        /*get the page frame number*/
        pa = pte_val(*pte) & PAGE_MASK;
        /*in case the pa lie in io region*/
        return is_valid_ram(pa);
    }else {
        return 0;
    }
}

/*
* @search from start for a continous range of VA that has mapping on RAM* 
* @or have no mapping on RAM
*
* the result stores in [start, end), empty
*
* return FALSE if start exceeds vmalloc_end
*/


static int find_continous_vmalloc_range(unsigned int start, 
                                        unsigned int *end, 
                                        bool *empty)
{
    unsigned int addr = start;
    bool valid = 0;   

    /*check addr should be page aligned*/
    if((start & (PAGE_SIZE - 1)) || start >= VMALLOC_END) {
        printk("%s addr not align or too big %x\n", __func__, start);
        return FALSE;
    }

    valid = is_addr_valid_map(addr);
    addr += PAGE_SIZE;

    *empty = !valid;
    
    /*find hole or continous map*/
    while(1) {

        if(valid != is_addr_valid_map(addr)) {
            break;
        }

        addr += PAGE_SIZE;
        
        if(addr >= VMALLOC_END) {
            break;
        }
    }

    *end = addr;
    return TRUE;
}

/*
 * dump memory address at start_addr, with len into sd card or file in sdcard
 * 
*/

BOOL raw_write_to_sd(PBYTE ptr, 
                     unsigned int data_len, 
                     int *pOffset)
{
    static unsigned int sd_addr = 0; 
    
    if(g_FH.ISFILE == 0) {
        if(FALSE == WriteDataToSDCard(&g_FH, ptr, 
                    data_len, sd_addr, pOffset)) {
            return FALSE;
        }

        sd_addr += data_len;
    }else {
        if(FALSE == WriteDumpFile_sd(&g_FH, ptr, data_len, 0)) {
            return FALSE;
        }
    }

    return TRUE;
}
////////////// cache API start
#define SIZEOF_KDUMP_SD_CACHE (sizeof(unsigned char) * PAGE_SIZE * 64)

#ifdef MTK_USE_RESERVED_EXT_MEM
extern void* extmem_malloc_page_align(size_t bytes);
static unsigned char * kdump_sd_cache = NULL;
void init_kdump_sd_cache(void)
{
    kdump_sd_cache = extmem_malloc_page_align(SIZEOF_KDUMP_SD_CACHE);
    if(kdump_sd_cache == NULL)
        panic("%s[%s] memory alloc failed!!!\n", __FILE__, __FUNCTION__);
}
EXPORT_SYMBOL(init_kdump_sd_cache);
#else
static unsigned char kdump_sd_cache[PAGE_SIZE * 64];
#endif
static unsigned int kdump_sd_cache_cursor = 0;
static unsigned int kdump_sd_cache_left = SIZEOF_KDUMP_SD_CACHE;

/*caller should take care of bounary check*/
static void fill_data_to_cache(void *start_addr, unsigned int len)
{
    memcpy((void *)(kdump_sd_cache + kdump_sd_cache_cursor), start_addr, len);
    kdump_sd_cache_cursor += len;
    kdump_sd_cache_left -= len;
}

/*caller should take care of bounary check*/
static void fill_zero_to_cache(unsigned int len)
{
    int i;
    for(i = 0; i < len; i++) {
        kdump_sd_cache[kdump_sd_cache_cursor + i] = 0;
    }

    kdump_sd_cache_cursor += len;
    kdump_sd_cache_left -= len;
}

static void reset_cache(void)
{
    kdump_sd_cache_cursor  = 0;
    kdump_sd_cache_left = SIZEOF_KDUMP_SD_CACHE;
}

static int flush_cache(void)
{
    int offset; 

    if(FALSE == raw_write_to_sd((PBYTE)kdump_sd_cache, 
                                kdump_sd_cache_cursor, 
                                &offset)) {
        return FALSE; 
    }
    
    reset_cache();
    return TRUE;
}
/*write len <= cache size data to cache*/
static int drop_data_to_cache(unsigned int start_addr, unsigned int len, bool zero)
{
    unsigned int copy_len;
    //fill the cache
    if(kdump_sd_cache_left) {
        copy_len = (kdump_sd_cache_left < len) ? kdump_sd_cache_left : len;
        
        if(!zero) {
            fill_data_to_cache((void *)start_addr, copy_len);                    
        }else {
            fill_zero_to_cache(copy_len);
        }   

        len -= copy_len;
        if (!zero)
            start_addr += copy_len;
    }

    //flush the cache
    if (kdump_sd_cache_left == 0) {
        if (FALSE == flush_cache()) {
            return FALSE;
        }
    }

    //fill the remaining data
    if (len) {
        if(!zero) {
            fill_data_to_cache((void *)start_addr, len) ;
        }else {
            fill_zero_to_cache(len);
        }
    }

    return TRUE;
}

////////////// cache API end
static int dump_memory_range_storage(unsigned int start_addr, 
                                     unsigned int len,
                                     bool empty)
{
    int offset;
    if(!empty) {
        //debug
        //printk("%s start %x len %x empty %d\n", __func__, start_addr, len, empty);
        
        if(FALSE == raw_write_to_sd((PBYTE)start_addr, len, &offset)) {
            return FALSE;
        }
    }else {
        //debug
        //printk("%s start %x len %x empty %d\n", __func__, start_addr, len, empty);
        
        while(len){
            unsigned int tmp_len;

            /*handle at most one cache size at one time*/
            tmp_len = ((len <= SIZEOF_KDUMP_SD_CACHE) ? len :  SIZEOF_KDUMP_SD_CACHE);
            len -= tmp_len;
        
            if(FALSE == drop_data_to_cache(0, tmp_len, 1)) {
                return FALSE;
            }
        }
        
        /*prevent data in cache*/
        flush_cache();
    }

    return TRUE;
}

/*all write from fragment memory should pass through cache*/
static int dump_memory_range_storage_cache(unsigned int start_addr, 
                                     unsigned int len,
                                     bool empty)
{
    printk("%s start %x len %x empty %d\n", __func__, start_addr, len, empty);
    
    while(len) {
        unsigned int tmp_len;
        
        /*handle at most one cache size at one time*/
        tmp_len = ((len <= SIZEOF_KDUMP_SD_CACHE) ? len :  SIZEOF_KDUMP_SD_CACHE);
        len -= tmp_len;

        if(empty) {
            if(FALSE == drop_data_to_cache(0, tmp_len, 1)) {
                return FALSE;
            }
        }else {

            if(FALSE == drop_data_to_cache(start_addr, tmp_len, 0)) {
                return FALSE;
            }
            start_addr += tmp_len;
        }
    }

    return TRUE;
}

static int kdump_elf_header(void)
{
    return dump_memory_range_storage((unsigned int)kdump_core_header, KDUMP_CORE_SIZE, 0);
}


static int kdump_vmalloc_varange(unsigned int va_start, unsigned int va_end)
{
   /*
    * for va from vmalloc_start to vmalloc_end, 
    * check whether it's valid from page table
    * write a range into sd card.
   */ 

    unsigned int start = va_start;
    unsigned int end = 0;
    bool empty = 0;
    while(1) {

        if(FALSE == find_continous_vmalloc_range(start, &end, &empty)) {
            printk("%s find range failed\n", __func__);
            return FALSE;
        }

        //debug
        //printk("%s %x %x %d\n", __func__, start, end, empty);
        if(FALSE == dump_memory_range_storage_cache(start, end - start, empty)) {
            printk("%s dump mem failed %x %x\n", __func__, start, end);
            return FALSE;
        }

        start = end;

        if(start >= va_end) {
            break;
        }
    }

    return TRUE;
}


static int kdump_discontinuous_lowmem(void)
{
    extern struct meminfo meminfo;
    struct meminfo *mi = &meminfo;
    int i;
    unsigned int pre_end = 0;

    for_each_bank(i, mi) {
        unsigned int start, end;
        struct membank *bank = &mi->bank[i];

        start = (unsigned int)__va(bank_phys_start(bank));
        end = (unsigned int)__va(bank_phys_end(bank));

        if (end > (unsigned int)high_memory)
            end = (unsigned int)high_memory;
        if (start >= end)
            break;
      
        
        /*this is hole, fill zero to sd card  [pre_end, start-pre_end]*/
        if(pre_end != 0 && (start - pre_end) != 0) {
            //debug
            //printk(KERN_ALERT"zero virtual memory area:%x - %x\n",
             //       pre_end, start);

            if(FALSE == dump_memory_range_storage(pre_end, start-pre_end, 1)) {
                return FALSE;
            }
        
        }

        //debug
        printk(KERN_ALERT"valid virtual memory area: 0x%x - %x\n",
                start, end);

        /*write start, end to sd card*/
        if(FALSE == dump_memory_range_storage(start, end - start, 0)) {
            return FALSE; 
        }

        pre_end = end;
    }
    return TRUE;
}

/*
 * Format and create a standalone file under SDCARD
 *
 */
extern unsigned int kdump_init_mem_len(void);
int kdump_create_dump_msdc_format_file(void)
{
	int offset = 0;    
    int ret = 0;
	unsigned int total_dump_size = 0;
    
    total_dump_size = kdump_init_mem_len();
    DBGKDUMP_PRINTK ("%s: Start dumping memory (size:%d)...\n", __func__, total_dump_size);
    
    if (!(ret = msdc_init_panic(DUMP_INTO_BOOT_CARD_KDUMP))) {
        DBGKDUMP_PRINTK ("%s: msdc_init failed %d\n", __func__, ret);
        return -1;
    }

    /*
       create header for linear mapping part, module part , and vmalloc part
    
       for linear mapping part, check it's validity.
       for vmalloc part, check it's page table
       for module part, check it's page table.
    */

	kdump_core_header_init(kdump_core_header);
    
	
    DBGKDUMP_PRINTK("%s: Format before dump to sdcard\n", __func__);
	if(!FormatFATToSDCard(&g_FH, KDUMP_CORE_SIZE + total_dump_size))
		return -1;	

	DBGKDUMP_PRINTK("%s: Write kdump core elf header\n", __func__);
	if(!kdump_elf_header()) {
        printk("%s: Write kdump core elf header failed\n", __func__);
    }
   
    if (FALSE == flush_cache()) {
        printk("%s: elf_header flush cache failed\n", __func__);
    }

	DBGKDUMP_PRINTK("%s: Write kdump linear memory\n", __func__);
    
    if(!kdump_discontinuous_lowmem()) {
        printk("%s: Write kdump linear mapping failed\n", __func__);
    }
    
    if (FALSE == flush_cache()) {
        printk("%s: low memory flush cache failed\n", __func__);
    }

	DBGKDUMP_PRINTK("%s: Write kdump vmalloc memory\n", __func__);
    if(!kdump_vmalloc_varange(VMALLOC_START, VMALLOC_END)) {
        printk("%s: Write kdump vmalloc mapping failed\n", __func__);
    }

   if (FALSE == flush_cache()) {
        printk("%s: vmalloc memory flush cache failed\n", __func__);
    }

	DBGKDUMP_PRINTK("%s: Write kdump module memory\n", __func__);
    if(!kdump_vmalloc_varange(MODULES_VADDR, MODULES_END)) {
        printk("%s write kdump module failed\n", __func__);
    }

    if (FALSE == flush_cache()) {
        printk("%s: module memory flush cache failed\n", __func__);
    }

	DBGKDUMP_PRINTK("%s: dump finished, 0x%x/0x%x dumped.\n", __func__, offset, total_dump_size);
	return 0;
}

int kdump_create_dump_msdc_file(void)
{
	int offset = 0;    
    int ret = 0;

	unsigned int total_dump_size = get_linear_mapping_size();
	DBGKDUMP_PRINTK("%s: Start dumping memory (size:%d)...\n", __func__, total_dump_size);
    
	if (!(ret = msdc_init_panic(DUMP_INTO_BOOT_CARD_KDUMP))) {
        DBGKDUMP_PRINTK ("%s: msdc_init failed %d\n", __func__, ret);
        return -1;
    }

	kdump_core_header_init(kdump_core_header);
    
	OpenDumpFile_sd(&g_FH);
		
	DBGKDUMP_PRINTK("%s: Write kdump core elf header\n", __func__);

    if(!kdump_elf_header()) {
        printk("%s: Write kdump core elf header failed\n", __func__);
    }

    if (FALSE == flush_cache()) {
        printk("%s: elf_header flush cache failed\n", __func__);
    }

    DBGKDUMP_PRINTK("%s: Write kdump linear memory\n", __func__);

    if(!kdump_discontinuous_lowmem()) {
        printk("%s: Write kdump linear mapping failed\n", __func__);
    }

    if (FALSE == flush_cache()) {
        printk("%s: low memory flush cache failed\n", __func__);
    }

    DBGKDUMP_PRINTK("%s: Write kdump vmalloc memory\n", __func__);
    if(!kdump_vmalloc_varange(VMALLOC_START, VMALLOC_END)) {
        printk("%s: Write kdump vmalloc mapping failed\n", __func__);
    }

    if (FALSE == flush_cache()) {
        printk("%s: vmalloc memory flush cache failed\n", __func__);
    }

    DBGKDUMP_PRINTK("%s: Write kdump module memory\n", __func__);
    if(!kdump_vmalloc_varange(MODULES_VADDR, MODULES_END)) {
        printk("%s write kdump module failed\n", __func__);
    }

    if (FALSE == flush_cache()) {
        printk("%s: module memory flush cache failed\n", __func__);
    }

    /*Flush file buffer, update metadata, this is not flush_cache*/
	CloseDumpFile_sd(&g_FH);

	DBGKDUMP_PRINTK("%s: dump finished, 0x%x/0x%x dumped.\n", __func__, offset, total_dump_size);
	return 0;
}

#if 0
/*using vmalloc, check it from memory dump*/
static struct test_bed{
    unsigned char *buf;
    unsigned int size;
}test_buffers[] = {{NULL, 20}, {NULL, 4096}, {NULL, 4096 + 20},
                   {NULL, 1024 * 1024}, {NULL, 1024 * 1024 + 20}, 
                   {NULL, 1024 * 1024 * 4}, {NULL, 1024 * 1024 * 4 + 20},
                  
                   {NULL, 20}, {NULL, 4096}, {NULL, 4096 + 20},
                   {NULL, 1024 * 1024}, {NULL, 1024 * 1024 + 20}, 
                   {NULL, 1024 * 1024 * 4}, {NULL, 1024 * 1024 * 4 + 20},
                  
                   {NULL, 20}, {NULL, 4096}, {NULL, 4096 + 20},
                   {NULL, 1024 * 1024}, {NULL, 1024 * 1024 + 20}, 
                   {NULL, 1024 * 1024 * 3}, {NULL, 1024 * 1024 + 20},
                  };


void kdump_test_case(void)
{
   int i,j;

   //write to low memory boundary, remember to check it.
   unsigned int b1[] = {0xcea00000, 0xdaa00000, 0xdbc00000};
   unsigned int s1[] = {0xc1000000, 0xcea10000, 0xdb400000};

   for(i = 0; i < (sizeof(b1)/sizeof(unsigned int)); i++) {
        for(j = 20; j > 0; j--) {
            *((unsigned char *)(b1[i]-j)) = 'Y';
        }
   }
   
   for(i = 0; i < (sizeof(s1)/sizeof(unsigned int)); i++) {
        for(j = 20; j > 0; j--) {
            *((unsigned char *)(s1[i] + j)) = 'X';
        }
   }

    //vmalloc range
   for (i = 0; i < sizeof(test_buffers)/sizeof(struct test_bed); i++) {
        
        if(NULL == (test_buffers[i].buf = vmalloc(test_buffers[i].size))) {
            printk("allocate buffer failed %d\n", i);
            return;
        }
        
        /*write the pattern*/
        for(j = 0; j < test_buffers[i].size; j++){
         test_buffers[i].buf[j] = 'A' + i % 26;
        }

//        printk("vmalloc pointer %x, pattern %c\n", 
  //                                  test_buffers[i].buf, test_buffers[i].buf[0]);

   }

   //module area
    
}
#endif
