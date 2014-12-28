# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.
#
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


# *************************************************************************
# Set shell align with Android build system
# *************************************************************************
SHELL        := /bin/bash

.PHONY : run-preprocess remove-preprocess clean-preprocess clean-preprocessed gen-preprocessed remove-preprocessed

HAVE_PREPROCESS_FLOW := true
HAVE_PREPROCESS_FLOW := $(strip $(HAVE_PREPROCESS_FLOW))
MP_REMOVE_PREPROCESS := $(strip $(MP_REMOVE_PREPROCESS))

temp_all_preprocess_list := $(shell python mediatek/build/tools/preprocess/getPreprocessFiles.py; \
                                    if [ $$? != 0 ]; then echo GET_PREPROCESS_FILES_PY_FAIL; fi \
                              )
ifneq ($(filter GET_PREPROCESS_FILES_PY_FAIL, $(temp_all_preprocess_list)),)
  $(error mediatek/build/tools/getPreprocessFiles.py run fail, please check whiteList.xml)
else
  ALL_NEED_PREPROCESS_FILES := $(strip $(temp_all_preprocess_list))
endif

ALL_CTXT_FILES := $(filter %.ctxt, $(ALL_NEED_PREPROCESS_FILES))
ALL_GEN_CTXT_FILES := $(patsubst %.ctxt,%.txt,$(ALL_CTXT_FILES))

ALL_CXML_FILES := $(filter %.cxml, $(ALL_NEED_PREPROCESS_FILES))
ALL_GEN_CXML_FILES := $(patsubst %.cxml,%.xml,$(ALL_CXML_FILES))

ALL_CJAVA_FILES := $(filter %.cjava, $(ALL_NEED_PREPROCESS_FILES))
ALL_GEN_CJAVA_FILES := $(patsubst %.cjava,%.java,$(ALL_CJAVA_FILES))

ALL_UNPREPROCESS_FILES := $(ALL_CTXT_FILES) $(ALL_CXML_FILES) $(ALL_CJAVA_FILES)
ALL_PREPROCESS_GEN_FILES := $(ALL_GEN_CTXT_FILES) $(ALL_GEN_CXML_FILES) $(ALL_GEN_CJAVA_FILES)

include mediatek/build/tools/preprocess/preprocessFeatureList.mk

define mtk.custom.generate-macros.preprocess
$(strip $(foreach t,$(AUTO_ADD_PREPROCESS_DEFINE_BY_NAME),$(call .if-cfg-on,$(t),-D$(strip $(t))=$(strip $(t)))))
endef

ALL_PREPROCESS_DEFINITIONS := $(call mtk.custom.generate-macros.preprocess)

ifneq ($(ALL_GEN_CJAVA_FILES),)
$(ALL_GEN_CJAVA_FILES) : %.java : %.cjava
	@echo pre-process $< $(DEAL_STDOUT)
	@gcc -x c -P $(ALL_PREPROCESS_DEFINITIONS) -C -E $< -o $@ $(DEAL_STDOUT)
endif

ifneq ($(ALL_GEN_CXML_FILES),)
$(ALL_GEN_CXML_FILES) : %.xml : %.cxml
	@echo pre-process $< $(DEAL_STDOUT)
	@gcc -x c -P -C $(ALL_PREPROCESS_DEFINITIONS) -E $< -o $@ $(DEAL_STDOUT)
endif

ifneq ($(ALL_GEN_CTXT_FILES),)
$(ALL_GEN_CTXT_FILES) : %.txt : %.ctxt
	@echo pre-process $< $(DEAL_STDOUT)
	@gcc -x c -P -C $(ALL_PREPROCESS_DEFINITIONS) -E $< -o $@ $(DEAL_STDOUT)
endif

remove-preprocessed clean-preprocessed:
ifneq ($(ALL_PREPROCESS_GEN_FILES),)
	@rm -f $(ALL_PREPROCESS_GEN_FILES) $(DEAL_STDOUT)
endif
	@echo Done clean pre-processed files

gen-preprocessed-and-remove-preprocess: gen-preprocessed
ifneq ($(ALL_UNPREPROCESS_FILES),)
	@rm -f $(ALL_UNPREPROCESS_FILES) $(DEAL_STDOUT)
endif
	@echo Done clean pre-process files

gen-preprocessed: $(ALL_PREPROCESS_GEN_FILES)
	@echo Done Gen pre-process

ifeq ($(MP_REMOVE_PREPROCESS),true)
run-preprocess: gen-preprocessed-and-remove-preprocess
endif
run-preprocess: gen-preprocessed
	@echo Done pre-process
