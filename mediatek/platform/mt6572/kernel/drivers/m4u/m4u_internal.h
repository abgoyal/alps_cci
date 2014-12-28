/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors:
 * any reproduction, modification, use or disclosure of MediaTek Software:
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES:
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
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE:
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE:
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#include <mach/m4u.h>

 static void m4u_mvaGraph_init(void);
 void m4u_mvaGraph_dump_raw(void);
 void m4u_mvaGraph_dump(void);
 static int m4u_dealloc_mva_dynamic(const M4U_MODULE_ID_ENUM eModuleID, 
									 const unsigned int BufAddr, 
									 const unsigned int BufSize,
									 const unsigned int mvaRegionAddr);
 static unsigned int m4u_do_mva_alloc(const M4U_MODULE_ID_ENUM eModuleID, 
								   const unsigned int BufAddr, 
								   const unsigned int BufSize);
 static int m4u_do_mva_free(const M4U_MODULE_ID_ENUM eModuleID, 
								 const unsigned int BufAddr,
								 const unsigned int BufSize,
								 const unsigned int mvaRegionStart) ;
 static M4U_MODULE_ID_ENUM mva2module(const unsigned int mva);
 static int m4u_invalid_seq_range_by_mva(unsigned int MVAStart, unsigned int MVAEnd);
 void m4u_dump_pagetable(const M4U_MODULE_ID_ENUM eModuleID);
 static int m4u_confirm_range_invalidated(const unsigned int MVAStart, const unsigned int MVAEnd);
 
 static bool m4u_struct_init(void);
 static int m4u_hw_init(void);
									 
 static int m4u_get_pages(const M4U_MODULE_ID_ENUM eModuleID,
					 const unsigned int BufAddr, 
					 const unsigned int BufSize, 
					 unsigned int* const pPhys);
								
 static void m4u_release_pages(const M4U_MODULE_ID_ENUM eModuleID,
					 const unsigned int BufAddr, 
					 const unsigned int BufSize,
					 const unsigned int MVA);
 
 static M4U_DMA_DIR_ENUM m4u_get_dir_by_module(M4U_MODULE_ID_ENUM eModuleID);
 static void m4u_clear_intr(const unsigned int m4u_base);
#define  m4u_port_2_m4u_id(portID) 0  // TODO: FIXME! cloud
 static void m4u_memory_usage();
 void m4u_print_active_port();
 static M4U_MODULE_ID_ENUM m4u_port_2_module(M4U_PORT_ID_ENUM portID);
 static char* m4u_get_port_name(const M4U_PORT_ID_ENUM portID);
 static char* m4u_get_module_name(const M4U_MODULE_ID_ENUM moduleID);
 void m4u_get_power_status(void);
 unsigned int m4u_get_pa_by_mva(unsigned int mva);
 int m4u_dump_user_addr_register(unsigned int m4u_index);
 static int m4u_add_to_garbage_list(const struct file * a_pstFile,
										 const unsigned int mvaStart, 
										 const unsigned int bufSize,
										 const M4U_MODULE_ID_ENUM eModuleID,
										 const unsigned int va,
										 const unsigned int flags,
										 const int security,
										 const int cache_coherent);
 static int m4u_delete_from_garbage_list(const M4U_MOUDLE_STRUCT* p_m4u_module, const struct file * a_pstFile);
 M4U_PORT_ID_ENUM m4u_get_error_port(unsigned int m4u_index, unsigned int mva);
 int m4u_dump_mva_info(void);
 int m4u_get_write_mode_by_module(M4U_MODULE_ID_ENUM moduleID);
 void m4u_dump_pagetable_range(unsigned int vaStart, const unsigned int nr);
 void m4u_print_mva_list(struct file *filep, const char *pMsg);
 int m4u_dma_cache_flush_all(void);
 extern void mlock_vma_page(struct page *page);
 extern void munlock_vma_page(struct page *page);
 static void m4u_dump_main_tlb_tags(void) ;
 int m4u_dump_main_tlb_des(); 
 static void m4u_dump_pfh_tlb_tags(void);
 int m4u_dump_pfh_tlb_des();
 static void m4u_enable_error_hang(const bool fgEnable);
 static void m4u_invalidate_and_check(unsigned int start, unsigned int end);
 int m4u_query_mva(const M4U_MODULE_ID_ENUM eModuleID, 
						const unsigned int BufAddr, 
						const unsigned int BufSize, 
						unsigned int *pRetMVABuf,
						const struct file * a_pstFile);

static int m4u_invalid_seq_all(const M4U_MODULE_ID_ENUM eModuleID);
static void m4u_dump_wrap_range_info(void);
static int m4u_perf_timer_on(void);
static void m4u_profile_init(void);
static int m4u_log_on(void);
static int m4u_log_off(void);
static void m4u_dump_seq_range_info(void);

 
