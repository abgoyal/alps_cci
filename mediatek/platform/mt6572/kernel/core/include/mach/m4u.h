/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#ifndef __M4U_H__
#define __M4U_H__
#include <linux/ioctl.h>
#include <linux/fs.h>


#define M4U_PAGE_SIZE    0x1000                                  //4KB

#define M4U_CLIENT_MODULE_NUM        M4U_CLNTMOD_MAX
#define TOTAL_MVA_RANGE   0x20000000   //0x40000000                              //total virtual address range: 1GB

#define PT_TOTAL_ENTRY_NUM    (TOTAL_MVA_RANGE/DEFAULT_PAGE_SIZE)              //total page table entries

#define M4U_GET_PTE_OFST_TO_PT_SA(MVA)    (((MVA) >> 12) << 2)

//==========================
//hw related
//=========================
//m4u global
#define TOTAL_M4U_NUM         1
#define M4U_REG_SIZE        0x5f4
#define M4U_PORT_NR         16

//SMI related
#define SMI_LARB_NR         1

//tlb related
#define M4U_MAIN_TLB_NR   32
#define M4U_PRE_TLB_NR    32

//wrap range related
#define M4U_WRAP_NR       2
#define TOTAL_WRAP_NUM    ((M4U_WRAP_NR)*(TOTAL_M4U_NUM))


//seq range related
#define M4U_SEQ_NR          8
#define SEQ_RANGE_NUM       M4U_SEQ_NR
#define TOTAL_RANGE_NUM       (M4U_SEQ_NR)*TOTAL_M4U_NUM
#define M4U_SEQ_ALIGN_MSK   (0x40000-1)
#define M4U_SEQ_ALIGN_SIZE  0x40000


//====================================
// about portid
//====================================
#define M4U_LARB0_PORTn(n)      ((n)+0)


typedef enum
{
    M4U_PORT_LCD_OVL               =  M4U_LARB0_PORTn(0)   ,
    M4U_PORT_LCD_R                 =  M4U_LARB0_PORTn(1)   ,
    M4U_PORT_LCD_W                 =  M4U_LARB0_PORTn(2)   ,
    M4U_PORT_LCD_DBI               =  M4U_LARB0_PORTn(3)   ,
    M4U_PORT_CAM_WDMA              =  M4U_LARB0_PORTn(4)   ,
    M4U_PORT_CMDQ                  =  M4U_LARB0_PORTn(5)   ,
    M4U_PORT_VENC_BSDMA_VDEC_POST0 =  M4U_LARB0_PORTn(6)   ,
    M4U_PORT_MDP_RDMA              =  M4U_LARB0_PORTn(7)   ,
    M4U_PORT_MDP_WDMA              =  M4U_LARB0_PORTn(8)   ,
    M4U_PORT_MDP_ROTO              =  M4U_LARB0_PORTn(9)   ,                                                           
    M4U_PORT_MDP_ROTCO             =  M4U_LARB0_PORTn(10)   ,
    M4U_PORT_MDP_ROTVO             =  M4U_LARB0_PORTn(11)   ,
    M4U_PORT_VENC_MVQP             =  M4U_LARB0_PORTn(12)   ,
    M4U_PORT_VENCMC                =  M4U_LARB0_PORTn(13)   ,
    M4U_PORT_VENC_CDMA_VDEC_CDMA   =  M4U_LARB0_PORTn(14)   ,
    M4U_PORT_VENC_REC_VDEC_WDMA    =  M4U_LARB0_PORTn(15)   ,
    M4U_PORT_NUM,
    M4U_PORT_UNKNOWN
} M4U_PORT_ID_ENUM;

static inline int m4u_port_2_smi_port(M4U_PORT_ID_ENUM port)
{
    int local_port = port;
    return local_port;

}

static inline M4U_PORT_ID_ENUM larb_port_2_m4u_port(unsigned int larb, unsigned int local_port)
{
    return local_port;
}

typedef enum
{
    M4U_CLNTMOD_MDP     = 0,	                             
    M4U_CLNTMOD_DISP       ,                             
    M4U_CLNTMOD_VIDEO      ,
    M4U_CLNTMOD_CAM        ,
    M4U_CLNTMOD_CMDQ       ,
    M4U_CLNTMOD_LCDC_UI    ,
    M4U_CLNTMOD_UNKNOWN    ,
    M4U_CLNTMOD_MAX

} M4U_MODULE_ID_ENUM;



// 200MB = 200 * 0X001000000
#define    M4U_CLNTMOD_SZ_MDP         (100*0x00100000)
#define    M4U_CLNTMOD_SZ_DISP        (100*0x00100000)
#define    M4U_CLNTMOD_SZ_VIDEO       (200*0x00100000)
#define    M4U_CLNTMOD_SZ_CAM         (100*0x00100000)
#define    M4U_CLNTMOD_SZ_CMDQ          (10*0x00100000)
#define    M4U_CLNTMOD_SZ_LCDC_UI     (100*0x00100000)
#define    M4U_CLNTMOD_SZ_RESERVED    (10*0x00100000)



typedef struct _M4U_RANGE_DES  //sequential entry range
{
    unsigned int Enabled;
    M4U_MODULE_ID_ENUM eModuleID;
    unsigned int MVAStart;
    unsigned int MVAEnd;
    unsigned int entryCount;
} M4U_RANGE_DES_T;

typedef struct _M4U_WRAP_DES
{
    unsigned int Enabled;
    M4U_MODULE_ID_ENUM eModuleID;
    M4U_PORT_ID_ENUM ePortID;    
    unsigned int MVAStart;
    unsigned int MVAEnd;
} M4U_WRAP_DES_T;

typedef enum
{
	M4U_DMA_READ_WRITE = 0,
	M4U_DMA_READ = 1,
	M4U_DMA_WRITE = 2,
	M4U_DMA_NONE_OP = 3,

} M4U_DMA_DIR_ENUM;

// port related: virtuality, security, distance
typedef struct _M4U_PORT
{  
    M4U_PORT_ID_ENUM ePortID;		   //hardware port ID, defined in M4U_PORT_ID_ENUM
    unsigned int Virtuality;						   
    unsigned int Security;
    unsigned int domain;            //domain : 0 1 2 3
    unsigned int Distance;
    unsigned int Direction;         //0:- 1:+
}M4U_PORT_STRUCT;

typedef enum
{
	ROTATE_0=0,
	ROTATE_90,
	ROTATE_180,
	ROTATE_270,
	ROTATE_HFLIP_0,
	ROTATE_HFLIP_90,
	ROTATE_HFLIP_180,
	ROTATE_HFLIP_270
} M4U_ROTATOR_ENUM;

typedef struct _M4U_PORT_ROTATOR
{  
	M4U_PORT_ID_ENUM ePortID;		   // hardware port ID, defined in M4U_PORT_ID_ENUM
	unsigned int Virtuality;						   
	unsigned int Security;
	// unsigned int Distance;      // will be caculated actomatically inside M4U driver
	// unsigned int Direction;
  unsigned int MVAStart; 
  unsigned int BufAddr;
  unsigned int BufSize;  
  M4U_ROTATOR_ENUM angle;	
}M4U_PORT_STRUCT_ROTATOR;

// module related:  alloc/dealloc MVA buffer
typedef struct _M4U_MOUDLE
{
	// MVA alloc / dealloc
	M4U_MODULE_ID_ENUM eModuleID;	// module ID used inside M4U driver, defined in M4U_PORT_MODULE_ID_ENUM
	unsigned int BufAddr;				// buffer virtual address
	unsigned int BufSize;				// buffer size in byte

	// TLB range invalidate or set uni-upadte range
	unsigned int MVAStart;						 // MVA start address
	unsigned int MVAEnd;							 // MVA end address
	unsigned int entryCount;

    // manually insert page entry
	unsigned int EntryMVA;						 // manual insert entry MVA
	unsigned int Lock;							 // manual insert lock or not
	int security;
    int cache_coherent;
}M4U_MOUDLE_STRUCT;

typedef enum
{
    M4U_CACHE_FLUSH_BEFORE_HW_READ_MEM = 0,  // optimized, recommand to use
    M4U_CACHE_FLUSH_BEFORE_HW_WRITE_MEM = 1, // optimized, recommand to use
    M4U_CACHE_CLEAN_BEFORE_HW_READ_MEM = 2,
    M4U_CACHE_INVALID_AFTER_HW_WRITE_MEM = 3,
    M4U_NONE_OP = 4,
} M4U_CACHE_SYNC_ENUM;

typedef struct _M4U_CACHE
{
    // MVA alloc / dealloc
    M4U_MODULE_ID_ENUM eModuleID;             // module ID used inside M4U driver, defined in M4U_MODULE_ID_ENUM
    M4U_CACHE_SYNC_ENUM eCacheSync;
    unsigned int BufAddr;                  // buffer virtual address
    unsigned int BufSize;                     // buffer size in byte
}M4U_CACHE_STRUCT;

typedef struct _M4U_PERF_COUNT
{
    unsigned int transaction_cnt;
    unsigned int main_tlb_miss_cnt;
    unsigned int pfh_tlb_miss_cnt;
    unsigned int pfh_cnt;
}M4U_PERF_COUNT;

//IOCTL commnad
#define MTK_M4U_MAGICNO 'g'
#define MTK_M4U_T_POWER_ON            _IOW(MTK_M4U_MAGICNO, 0, int)
#define MTK_M4U_T_POWER_OFF           _IOW(MTK_M4U_MAGICNO, 1, int)
#define MTK_M4U_T_DUMP_REG            _IOW(MTK_M4U_MAGICNO, 2, int)
#define MTK_M4U_T_DUMP_INFO           _IOW(MTK_M4U_MAGICNO, 3, int)
#define MTK_M4U_T_ALLOC_MVA           _IOWR(MTK_M4U_MAGICNO,4, int)
#define MTK_M4U_T_DEALLOC_MVA         _IOW(MTK_M4U_MAGICNO, 5, int)
#define MTK_M4U_T_INSERT_TLB_RANGE    _IOW(MTK_M4U_MAGICNO, 6, int)
#define MTK_M4U_T_INVALID_TLB_RANGE   _IOW(MTK_M4U_MAGICNO, 7, int)
#define MTK_M4U_T_INVALID_TLB_ALL     _IOW(MTK_M4U_MAGICNO, 8, int)
#define MTK_M4U_T_MANUAL_INSERT_ENTRY _IOW(MTK_M4U_MAGICNO, 9, int)
#define MTK_M4U_T_CACHE_SYNC          _IOW(MTK_M4U_MAGICNO, 10, int)
#define MTK_M4U_T_CONFIG_PORT         _IOW(MTK_M4U_MAGICNO, 11, int)
#define MTK_M4U_T_CONFIG_ASSERT       _IOW(MTK_M4U_MAGICNO, 12, int)
#define MTK_M4U_T_INSERT_WRAP_RANGE   _IOW(MTK_M4U_MAGICNO, 13, int)
#define MTK_M4U_T_MONITOR_START       _IOW(MTK_M4U_MAGICNO, 14, int)
#define MTK_M4U_T_MONITOR_STOP        _IOW(MTK_M4U_MAGICNO, 15, int)
#define MTK_M4U_T_RESET_MVA_RELEASE_TLB  _IOW(MTK_M4U_MAGICNO, 16, int)
#define MTK_M4U_T_CONFIG_PORT_ROTATOR _IOW(MTK_M4U_MAGICNO, 17, int)
#define MTK_M4U_T_QUERY_MVA           _IOW(MTK_M4U_MAGICNO, 18, int)
#define MTK_M4U_T_M4UDrv_CONSTRUCT    _IOW(MTK_M4U_MAGICNO, 19, int)
#define MTK_M4U_T_M4UDrv_DECONSTRUCT  _IOW(MTK_M4U_MAGICNO, 20, int)
#define MTK_M4U_T_DUMP_PAGETABLE      _IOW(MTK_M4U_MAGICNO, 21, int)
#define MTK_M4U_T_REGISTER_BUFFER     _IOW(MTK_M4U_MAGICNO, 22, int)
#define MTK_M4U_T_CACHE_FLUSH_ALL     _IOW(MTK_M4U_MAGICNO, 23, int)
#define MTK_M4U_T_REG_SET             _IOW(MTK_M4U_MAGICNO, 24, int)
#define MTK_M4U_T_REG_GET             _IOW(MTK_M4U_MAGICNO, 25, int)




// for kernel direct call --------------------------------------------
int m4u_dump_reg();
int m4u_dump_info();
void m4u_dump_pagetable(const M4U_MODULE_ID_ENUM eModuleID);
void m4u_dump_pagetable_nearby(const M4U_MODULE_ID_ENUM eModuleID, const unsigned int mva_addr);
int m4u_power_on();
int m4u_power_off();

int m4u_alloc_mva(const M4U_MODULE_ID_ENUM eModuleID, 
                      const unsigned int BufAddr, 
                      const unsigned int BufSize, 
                      const int security,
                      const int cache_coherent,
                      unsigned int *pRetMVABuf);

int m4u_dealloc_mva(M4U_MODULE_ID_ENUM eModuleID, 
                    const unsigned int BufAddr, 
                    const unsigned int BufSize,
                    const unsigned int MVA);	

int m4u_insert_wrapped_range(const M4U_MODULE_ID_ENUM eModuleID, 
                             const M4U_PORT_ID_ENUM portID, 
                             const unsigned int MVAStart, 
                             const unsigned int MVAEnd);

int m4u_invalid_wrapped_range(const M4U_MODULE_ID_ENUM eModuleID, 
                              const M4U_PORT_ID_ENUM portID,
                              const unsigned int MVAStart, 
                              const unsigned int MVAEnd);
                                                                                             
int m4u_insert_seq_range(const M4U_MODULE_ID_ENUM eModuleID, 
                              const unsigned int MVAStart, 
                              const unsigned int MVAEnd, 
                              const unsigned int entryCount); //0:disable multi-entry, 1,2,4,8,16: enable multi-entry
                      
int m4u_invalid_seq_range(const M4U_MODULE_ID_ENUM eModuleID, const unsigned int MVAStart, const unsigned int MVAEnd);
                    
void m4u_invalid_tlb_all();
int m4u_manual_insert_entry(M4U_PORT_ID_ENUM eModuleID,
                            unsigned int EntryMVA, 
                            int secure_pagetable,
                            int Lock) ;
int m4u_config_port_rotator(M4U_PORT_STRUCT_ROTATOR *pM4uPort);
int m4u_config_port(M4U_PORT_STRUCT* pM4uPort); //native
int m4u_monitor_start(void);
int m4u_monitor_stop(void);


int m4u_dma_cache_maint(const M4U_MODULE_ID_ENUM eModuleID,
    const void *va,
    const size_t size, 
    const int direction);

int m4u_reset_mva_release_tlb(M4U_MODULE_ID_ENUM eModuleID); 

//int m4u_mau_check_pagetable(unsigned int start_addr, unsigned int end_addr);
int m4u_mau_get_physical_port(unsigned int* engineMask);								  

typedef void (*PFN_TF_T) ();
void m4u_set_tf_callback(const M4U_MODULE_ID_ENUM eModuleID, PFN_TF_T ptf);

#endif

