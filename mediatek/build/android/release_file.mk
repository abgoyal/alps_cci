# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.

# MediaTek Inc. (C) 2010. All rights reserved.
#
# BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
# THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
# RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
# AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
# NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
# SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
# SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
# THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
# THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
# CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
# SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
# STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
# CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
# AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
# OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
# MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
#
# The following software/firmware and/or related documentation ("MediaTek Software")
# have been modified by MediaTek Inc. All revisions are subject to any receiver's
# applicable license agreements with MediaTek Inc.
PROTECT_DIRS := mediatek/protect
_release_policy := $(shell find $(PROTECT_DIRS) -name "Android.mk" | sed "s/\/Android.mk$$//") 

PROTECT_DIRS := mediatek/protect
_release_policy := $(shell find $(PROTECT_DIRS) -name "Android.mk" | sed "s/\/Android.mk$$//")
_release_policy +:= \
  mediatek/external/aee: \
  mediatek/external/bluetooth/blueangel/btcore/btstack: \
  mediatek/external/mhal/src/lib/lib3a: \
  mediatek/external/mhal/src/lib/libfd: \
  mediatek/external/mhal/src/lib/libmp4venc_rc: \
  mediatek/external/pal: \
  mediatek/protect/mhal/src/lib/lib3a: \
  mediatek/protect/mhal/src/lib/lib3a/mt6573: \
  mediatek/protect/mhal/src/lib/libeis: \
  mediatek/protect/mhal/src/lib/libeis/mt6573: \
  mediatek/protect/mhal/src/lib/libfd: \
  mediatek/protect/mhal/src/lib/libmav: \
  mediatek/protect/mhal/src/lib/libmotion: \
  mediatek/protect/mhal/src/lib/libpano: \
  mediatek/protect/mhal/src/lib/libwarp: \
  mediatek/protect/mhal/src/lib: \
  mediatek/protect/mhal/src/lib/lce: \
  mediatek/protect/mhal/src/lib/libautorama: \
  mediatek/protect/sd_upgrad20: \
  system/core/debuggerd: \
  packages/apps/MediaTek/ExceptionTool: \
  frameworks/base/cmds/dumpstate: \
  frameworks/mtk/extensions/aee: \
  frameworks/base/opengl: \
  frameworks/base/opengl/libagl: \
  frameworks/base/opengl/libs: \
  frameworks/base/opengl/libagl_mtk: \
  system/core/libpixelflinger: \
  mediatek/protect/hardware/ril: \
  mediatek/protect/hardware/ril3: \
  mediatek/protect/frameworks/base/telephony: 

