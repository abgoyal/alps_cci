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

 
#ifndef VENC_DRV_IF_DEP_H
#define VENC_DRV_IF_DEP_H

/*=============================================================================
 *                              Include Files
 *===========================================================================*/

#include "val_types.h" 
#include "vcodec_if.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=============================================================================
 *                              Type definition
 *===========================================================================*/

/**
 * @par Structure
 *   mhalVdoDrv_t
 * @par Description
 *   This is a structure which store common video enc driver information 
 */
typedef struct mhalVdoDrv_s 
{
    VAL_VOID_T                      *prCodecHandle;
    VAL_UINT32_T                    u4EncodedFrameCount;
    VCODEC_ENC_CALLBACK_T           rCodecCb;
    VIDEO_ENC_API_T                 *prCodecAPI;
    VENC_BS_T                       pBSBUF;

    VCODEC_ENC_BUFFER_INFO_T        EncoderInputParamNC;
    VENC_DRV_PARAM_BS_BUF_T         BSout;  
    VENC_HYBRID_ENCSETTING          rVencSetting;
    VAL_UINT8_T                     *ptr;	
}mhalVdoDrv_t;

typedef struct __VENC_HANDLE_T
{
    VAL_HANDLE_T            hHalHandle;    ///< HAL data.
    VAL_HANDLE_T            vdriver_Handle;       ///< for MMSYS power on/off
    VAL_MEMORY_T            rHandleMem;    ///< Save handle memory information to be used in release.
    VAL_BOOL_T              bFirstDecoded; /// < already pass first video data to codec
    VAL_BOOL_T              bHeaderPassed; /// < already pass video header to codec
    VAL_BOOL_T              bFlushAll;
    VAL_MEMORY_T            HeaderBuf;
    VAL_HANDLE_T            hCodec;
//    DRIVER_HANDLER_T        hDrv;
    VAL_UINT32_T            CustomSetting;
    VCODEC_MEMORY_TYPE_T    rVideoDecMemType;
    VAL_UINT32_T            nYUVBufferIndex;
    VCODEC_OPEN_SETTING_T   codecOpenSetting;

    mhalVdoDrv_t            rMhalVdoDrv;
    VAL_MEMORY_T            bs_driver_workingmem;

// Morris Yang 20110411 [
    VENC_DRV_VIDEO_FORMAT_T CodecFormat;
    VAL_VOID_T              *prExtraData;  ///< Driver private data pointer.
    VAL_MEMORY_T             rExtraDataMem; ///< Save extra data memory information to be used in release.
//  ]
    VAL_UINT32_T  nOmxTids;
#if 1   //defined(MT6572)     //VCODEC_MULTI_THREAD
// Jackal Chen [
    VAL_VOID_T              *pDrvModule;    ///< used for dlopen and dlclose
// ]
#endif
    VIDEO_ENC_WRAP_HANDLE_T hWrapper;
} VENC_HANDLE_T;


#ifdef __cplusplus
}
#endif

#endif /* VENC_DRV_IF_DEP_H */