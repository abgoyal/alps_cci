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


#
# MTK Build Info
#
MTK_BRANCH = ALPS.GB3.MP
	# Depend on branch

MTK_BUILD_VERNO = ALPS.GB3.MP.V1
	# Depend on release week

MTK_WEEK_NO =  
	# Depend on release week

# for build verno customization
CUSTOM_BUILD_VERNO =

# BICR (Build in CD ROM) -- Cui Zhang (83154)
MTK_BICR_SUPPORT=no

# Enlai Chu (83167)
MTK_CNN_CBN=no

MTK_2SDCARD_SWAP = no
MTK_S3D_SUPPORT = no
MTK_DATADIALOG_APP = no

AUTO_ADD_GLOBAL_DEFINE_BY_NAME = MTK_BQ24158_SUPPORT MTK_MMPROFILE_SUPPORT MTK_MEM_PRESERVED_MODE_ENABLE NAND_OTP_SUPPORT MTK_MATV_SERIAL_IF_SUPPORT MTK_MDM_SCOMO MTK_MDM_LAWMO MTK_MDM_FUMO MTK_UART_USB_SWITCH MTK_KERNEL_POWER_OFF_CHARGING MTK_CACHE_MERGE_SUPPORT MTK_IPV6_TETHER_NDP_MODE MTK_TETHERINGIPV6_SUPPORT MTK_FSCK_MSDOS_MTK MTK_MD_SHUT_DOWN_NT MTK_POWER_SAVING_SWITCH_UI_SUPPORT MTK_BENCHMARK_BOOST_TP MTK_AUD_LOCK_MD_SLEEP_SUPPORT HAVE_XLOG_FEATURE MTK_BT_PROFILE_MAPS MTK_BT_PROFILE_AVRCP MTK_WB_SPEECH_SUPPORT MTK_WIFI_HOTSPOT_SUPPORT MTK_RMVB_PLAYBACK_SUPPORT MTK_GPS_SUPPORT MTK_BT_PROFILE_OPP MTK_BT_PROFILE_TIMES MTK_BT_PROFILE_A2DP MTK_BRAZIL_CUSTOMIZATION_CLARO CUSTOM_KERNEL_GYROSCOPE MTK_BRAZIL_CUSTOMIZATION MTK_DSPIRDBG MTK_BT_PROFILE_MAPC MTK_FM_RECORDING_SUPPORT MTK_FM_SUPPORT MTK_AUTORAMA_SUPPORT MTK_QVGA_LANDSCAPE_SUPPORT MTK_FM_SHORT_ANTENNA_SUPPORT MTK_BT_PROFILE_TIMEC MTK_AUTO_DETECT_MAGNETOMETER MTK_MULTI_STORAGE_SUPPORT MTK_MATV_ANALOG_SUPPORT CUSTOM_KERNEL_ALSPS MTK_BT_PROFILE_SPP MTK_BT_30_SUPPORT MTK_BT_PROFILE_MANAGER MTK_BT_PROFILE_HFP MTK_BT_PROFILE_SIMAP MTK_ASF_PLAYBACK_SUPPORT MTK_DUAL_MIC_SUPPORT MTK_TVOUT_SUPPORT CUSTOM_KERNEL_ACCELEROMETER MTK_WAPI_SUPPORT MTK_BT_FM_OVER_BT_VIA_CONTROLLER HAVE_AACENCODE_FEATURE MTK_BT_21_SUPPORT MTK_BT_PROFILE_HIDH MTK_LCA_SUPPORT MODEM_UMTS_TDD128_MODE MTK_IPV6_SUPPORT MTK_DEDICATEDAPN_SUPPORT MTK_DHCPV6C_WIFI MTK_HDMI_SUPPORT MTK_BT_PROFILE_PRXM MTK_CTA_SUPPORT MTK_MAV_SUPPORT MTK_BT_SUPPORT MTK_BT_PROFILE_PBAP MTK_NFC_SUPPORT MTK_CAMERA_BSP_SUPPORT MTK_BT_PROFILE_PAN MTK_BT_PROFILE_PRXR MTK_BT_40_SUPPORT CUSTOM_KERNEL_MAGNETOMETER MTK_BT_PROFILE_FTP MTK_VT3G324M_SUPPORT HAVE_AWBENCODE_FEATURE MTK_BT_PROFILE_AVRCP14 MTK_BT_PROFILE_DUN MTK_CAMERA_APP_3DHW_SUPPORT MTK_BT_PROFILE_BIP MTK_BT_PROFILE_BPP MTK_WLAN_SUPPORT MTK_SENSOR_SUPPORT MTK_M4U_SUPPORT MTK_ENABLE_MD1 MTK_ENABLE_MD2  MTK_THEMEMANAGER_APP MTK_HDR_SUPPORT MTK_EMMC_SUPPORT MTK_FM_TX_SUPPORT MTK_BRAZIL_CUSTOMIZATION_VIVO MTK_AUTO_DETECT_ACCELEROMETER MTK_WLANBT_SINGLEANT MTK_USES_HD_VIDEO MTK_FD_SUPPORT MTK_FAN5405_SUPPORT MTK_FAT_ON_NAND MTK_2SDCARD_SWAP MTK_COMBO_QUICK_SLEEP_SUPPORT MTK_COMBO_CORE_DUMP_SUPPORT CUSTOM_USB_PID MTK_YAML_SCATTER_FILE_SUPPORT MTK_MTKLOGGER_SUPPORT MTK_COMBO_SUPPORT MTK_NAND_UBIFS_SUPPORT MTK_2IN1_SPK_SUPPORT

AUTO_ADD_GLOBAL_DEFINE_BY_VALUE = BOOT_LOGO MTK_PLATFORM MTK_AUDIO_BLOUD_CUSTOMPARAMETER_REV CUSTOM_KERNEL_LENS CUSTOM_KERNEL_MAIN_BACKUP_LENS CUSTOM_KERNEL_LCM MTK_MODEM_SUPPORT MTK_ATV_CHIP CUSTOM_KERNEL_MAIN_IMGSENSOR MTK_BT_CHIP MTK_WLAN_CHIP CUSTOM_KERNEL_SUB_BACKUP_IMGSENSOR MTK_IME_INPUT_ENGINE CUSTOM_KERNEL_MAIN_BACKUP_IMGSENSOR CUSTOM_KERNEL_FLASHLIGHT CUSTOM_KERNEL_SUB_IMGSENSOR CUSTOM_KERNEL_SUB_LENS CUSTOM_KERNEL_IMGSENSOR MTK_GPS_CHIP CUSTOM_KERNEL_SUB_BACKUP_LENS CUSTOM_KERNEL_MAIN_LENS MTK_FM_AUDIO MTK_FM_CHIP MTK_IME_HANDWRITING_ENGINE MTK_COMBO_CHIP

AUTO_ADD_GLOBAL_DEFINE_BY_NAME_VALUE =  MTK_LCM_PHYSICAL_ROTATION LCM_WIDTH EMMC_CHIP MTK_SHARE_MODEM_SUPPORT MTK_SHARE_MODEM_CURRENT LCM_HEIGHT MTK_TOUCH_PHYSICAL_ROTATION_RELATIVE_TO_LCM 

CUSTOM_HAL_ANT=mt6620_ant_m3
CUSTOM_HAL_AUDIOFLINGER=audio
CUSTOM_HAL_BLUETOOTH=bluetooth
CUSTOM_HAL_CAMERA=camera
CUSTOM_HAL_CAM_CAL = dummy_eeprom
CUSTOM_KERNEL_CAM_CAL = dummy_eeprom
CUSTOM_HAL_EEPROM=dummy_eeprom
CUSTOM_KERNEL_BTWLANEM=
CUSTOM_KERNEL_CAMERA=camera
CUSTOM_KERNEL_EEPROM=dummy_eeprom
CUSTOM_KERNEL_OFN=
CUSTOM_KERNEL_WIFI=
HAVE_MATV_FEATURE=no
HAVE_XLOG_FEATURE=yes
MTK_KERNEL_POWER_OFF_CHARGING = no
MTK_ACWFDIALOG_APP=yes
MTK_ANDROIDFACTORYMODE_APP=yes
MTK_APKINSTALLER_APP=no
MTK_ASF_PLAYBACK_SUPPORT=yes
MTK_AUTO_DETECT_ACCELEROMETER=no
MTK_AUTO_DETECT_MAGNETOMETER=no
MTK_BACKUPANDRESTORE_APP=no
MTK_BRAZIL_CUSTOMIZATION=no
MTK_BRAZIL_CUSTOMIZATION_CLARO=no
MTK_BRAZIL_CUSTOMIZATION_TIM=no
MTK_BRAZIL_CUSTOMIZATION_VIVO=no
MTK_BT_POWER_EFFICIENCY_ENHANCEMENT=yes
MTK_CAMERA_APP=yes
MTK_CAMERA_APP_3DHW_SUPPORT=yes
MTK_CAMERA_BSP_SUPPORT=yes
MTK_CHIP_VER=S01
MTK_CTA_SUPPORT=no
MTK_DIALER_SEARCH_SUPPORT=yes
MTK_DITHERING_SUPPORT=yes
MTK_DM_CU_SERVER_SUPPORT=no
MTK_DSPIRDBG=no
MTK_DUAL_MIC_SUPPORT=no
MTK_EXLOG2MAIL_APP=no
MTK_FAN5405_SUPPORT=no
MTK_FAT_ON_NAND=no
MTK_FILEMANAGER_APP=yes
MTK_FLV_PLAYBACK_SUPPORT=no
MTK_FM_AUDIO = FM_DIGITAL_INPUT
MTK_FM_TX_SUPPORT=no
MTK_FOTA_ENTRY=no
MTK_GALLERY_APP=no
MTK_GEMINI_3G_SWITCH=no
MTK_HDMI_SUPPORT=no
MTK_HEADSET_ICON_SUPPORT=no
MTK_IME_HANDWRITING_ENGINE=none
MTK_IME_INPUT_ENGINE=none
MTK_IME_RUSSIAN_SUPPORT = no
MTK_IME_SUPPORT = no
MTK_INPUTMETHOD_PINYINIME_APP=yes
MTK_INTERNAL=no
MTK_INTERNAL_LANG_SET=no
MTK_IPV6_SUPPORT=yes
MTK_DEDICATEDAPN_SUPPORT=no
MTK_DHCPV6C_WIFI=yes
MTK_LAUNCHERPLUS_APP=no
MTK_LCM_PHYSICAL_ROTATION=0
MTK_TOUCH_PHYSICAL_ROTATION_RELATIVE_TO_LCM=0
MTK_LOCKSCREEN_TYPE=2
MTK_MATV_ANALOG_SUPPORT=no
MTK_MAV_SUPPORT=yes
MTK_MFV_MPEG4_EXTRA=no
MTK_MMS_MULTIFORWARD_SUPPORT = yes
MTK_MMS_UNKNOWN_ATTACH_TYPE_SUPPORT = no
MTK_MT519X_FM_SUPPORT=no
MTK_MUSIC_LRC_SUPPORT=no
MTK_NEW_IPTABLES_SUPPORT=yes
MTK_NETWORK_TYPE_ALWAYS_ON=no
MTK_NFC_SUPPORT=no
MTK_OMA_DOWNLOAD_SUPPORT=yes
MTK_PHONE_VOICE_RECORDING=yes
MTK_QVGA_LANDSCAPE_SUPPORT=no
MTK_RELEASE_PACKAGE=rel_customer_basic
MTK_RMVB_PLAYBACK_SUPPORT=no
MTK_SCREEN_OFF_WIFI_OFF=no
MTK_SCOMO_ENTRY=no
MTK_SEARCH_DB_SUPPORT=yes
# using customization key/cert. for app. signing
MTK_SIGNATURE_CUSTOMIZATION=no
MTK_SMARTSWITCH_SUPPORT=no
MTK_SMS_NATIONAL_LANGUAGE_SUPPORT=no
MTK_SMS_TURKISH_TABLE_ALWAYS_SUPPORT=yes
MTK_SPECIAL_FACTORY_RESET=no
MTK_TB_APP_LANDSCAPE_SUPPORT=no
MTK_THEMEMANAGER_APP=yes
MTK_TTY_SUPPORT=no
MTK_WAPPUSH_SUPPORT=yes
MTK_WB_SPEECH_SUPPORT=yes
MTK_WEATHER_PROVIDER_APP=yes
MTK_WEATHER_WIDGET_APP=yes
MTK_WIFI_HOTSPOT_SUPPORT=yes
MTK_WLANBT_SINGLEANT=no
MTK_WLAN_SUPPORT=yes
MTK_WPA2PSK_SUPPORT=no
MTK_YGPS_APP=yes
#================================================================================
MTK_PRODUCT_LOCALES=en_US es_ES zh_CN zh_TW ru_RU pt_BR fr_FR de_DE tr_TR it_IT in_ID ms_MY vi_VN ar_EG  th_TH  pt_PT   nl_NL ldpi hdpi mdpi
EVB=no
GEMINI=no
LCM_HEIGHT=800
LCM_WIDTH=480
MTK_GPS_SUPPORT=yes
MTK_NAND_PAGE_SIZE=4K
MTK_YAML_SCATTER_FILE_SUPPORT=no
MTK_NAND_UBIFS_SUPPORT=no
MTK_EMMC_SUPPORT=yes
MTK_WLAN_CHIP=MT6620
MTK_GPS_CHIP=MTK_GPS_MT6620
EMMC_CHIP=4

# for cmmb chip. Xingyu Zhou (60360) CUSTOM_KERNEL_CMMB=no
HAVE_CMMB_FEATURE=no
MTK_CMMB_CHIP=no
MTK_GPU_CHIP=SGX351_110
MTK_COMBO_CHIP=MT6620E3
MTK_FM_CHIP=MT6620_FM
MTK_ATV_CHIP=
BOOT_LOGO=wvga
BUILD_KERNEL=yes
BUILD_PRELOADER=yes
BUILD_UBOOT=no
BUILD_LK=yes

CUSTOM_HAL_COMBO=mt6620
CUSTOM_HAL_FLASHLIGHT=constant_flashlight
CUSTOM_HAL_IMGSENSOR=ov5647_raw mt9v114_yuv
CUSTOM_HAL_LENS=fm50af dummy_lens
CUSTOM_HAL_MAIN_BACKUP_IMGSENSOR=
CUSTOM_HAL_MAIN_BACKUP_LENS=
CUSTOM_HAL_MAIN_IMGSENSOR=ov5647_raw
CUSTOM_HAL_MAIN_LENS=fm50af
CUSTOM_HAL_MATV=
CUSTOM_HAL_MSENSORLIB=mmc328x akm8975  ami304 yamaha530 akmd8963
CUSTOM_HAL_SENSORS=sensor
CUSTOM_HAL_SUB_BACKUP_IMGSENSOR=
CUSTOM_HAL_SUB_BACKUP_LENS=
CUSTOM_HAL_SUB_IMGSENSOR=mt9v114_yuv
CUSTOM_HAL_SUB_LENS=dummy_lens
CUSTOM_KERNEL_ACCELEROMETER=bma222
CUSTOM_KERNEL_ALSPS=
CUSTOM_KERNEL_BATTERY=battery
CUSTOM_KERNEL_CORE=src
CUSTOM_KERNEL_DCT=dct
CUSTOM_KERNEL_FLASHLIGHT=constant_flashlight
CUSTOM_KERNEL_FM=mt6620
CUSTOM_KERNEL_GYROSCOPE=
CUSTOM_KERNEL_HEADSET=accdet
CUSTOM_KERNEL_IMGSENSOR=ov5647_raw mt9v114_yuv
CUSTOM_KERNEL_JOGBALL=
CUSTOM_KERNEL_KPD=kpd
CUSTOM_KERNEL_LCM=hx8363_6575_dsi
CUSTOM_KERNEL_LEDS=mt65xx
CUSTOM_KERNEL_LENS=fm50af dummy_lens
CUSTOM_KERNEL_MAGNETOMETER=yamaha530
CUSTOM_KERNEL_MAIN_BACKUP_IMGSENSOR=
CUSTOM_KERNEL_MAIN_BACKUP_LENS=
CUSTOM_KERNEL_MAIN_IMGSENSOR=ov5647_raw
CUSTOM_KERNEL_MAIN_LENS=fm50af
CUSTOM_KERNEL_MATV=
CUSTOM_KERNEL_RTC=rtc
CUSTOM_KERNEL_SOUND=amp_yad
CUSTOM_KERNEL_SUB_BACKUP_IMGSENSOR=
CUSTOM_KERNEL_SUB_BACKUP_LENS=
CUSTOM_KERNEL_SUB_IMGSENSOR=mt9v114_yuv
CUSTOM_KERNEL_SUB_LENS=dummy_lens
CUSTOM_KERNEL_TOUCHPANEL=GT818B
CUSTOM_KERNEL_USB=mt6575
CUSTOM_KERNEL_VIBRATOR=vibrator
CUSTOM_MODEM=athens15v2_hspa_tdd128
CUSTOM_PRELOADER_CUSTOM=custom
CUSTOM_UBOOT_LCM=hx8363_6575_dsi
CUSTOM_USB_PID=no

HAVE_AACENCODE_FEATURE=yes
HAVE_AEE_FEATURE=yes
HAVE_APPC_FEATURE=no
HAVE_AWBENCODE_FEATURE=yes

HAVE_GROUP_SCHEDULING=no

HAVE_SRSAUDIOEFFECT_FEATURE=no

HAVE_VORBISENC_FEATURE=yes

MODEM_UMTS_TDD128_MODE=no

MTK_AGPS_APP=yes

MTK_ASD_SUPPORT=yes

MTK_AUDIO_APE_SUPPORT=yes
MTK_AUD_LOCK_MD_SLEEP_SUPPORT=no
MTK_AUTORAMA_SUPPORT=yes
MTK_AUDIO_BLOUD_CUSTOMPARAMETER_REV = MTK_AUDIO_BLOUD_CUSTOMPARAMETER_V4

MTK_AVI_PLAYBACK_SUPPORT=yes

MTK_CALENDAR_IMPORTER_APP=yes

MTK_DEFAULT_DATA_OFF=no

MTK_DM_APP=no
MTK_MDM_APP=no
MTK_RSDM_APP = no

MTK_DM_CMCC_SERVER_SUPPORT=yes
MTK_DM_ENTRY_DISPLAY=no
MTK_DM_RELEASE=no

MTK_DRM_APP=yes

MTK_EAP_SIM_AKA=yes

MTK_EMULATOR_SUPPORT=no

MTK_ENGINEERMODE_APP=yes

MTK_FD_FORCE_REL_SUPPORT=yes
MTK_FD_SUPPORT=yes

MTK_FMRADIO_APP=yes
MTK_FM_RECORDING_SUPPORT=yes
MTK_FM_SHORT_ANTENNA_SUPPORT=yes
MTK_FM_SUPPORT=yes

MTK_FOTA_RELEASE=no
MTK_FOTA_SUPPORT=no

MTK_FSCK_MSDOS_MTK=no

MTK_GALLERY3D_APP=yes

MTK_GEMINI_ENHANCEMENT=no

MTK_GOOGLEOTA_SUPPORT=yes

MTK_HDR_SUPPORT=yes

MTK_IME_ARABIC_SUPPORT = no
MTK_IME_ENGLISH_SUPPORT = no
MTK_IME_FRENCH_SUPPORT = no
MTK_IME_GERMAN_SUPPORT = no
MTK_IME_HANDWRITING_SUPPORT = no
MTK_IME_HINDI_SUPPORT = no
MTK_IME_INDONESIAN_SUPPORT = no
MTK_IME_ITALIAN_SUPPORT = no
MTK_IME_MALAY_SUPPORT = no
MTK_IME_PINYIN_SUPPORT = no
MTK_IME_PORTUGUESE_SUPPORT = no
MTK_IME_SPANISH_SUPPORT = no
MTK_IME_STROKE_SUPPORT = no
MTK_IME_THAI_SUPPORT = no
MTK_IME_TURKISH_SUPPORT = no
MTK_IME_VIETNAM_SUPPORT = no
MTK_IME_ZHUYIN_SUPPORT = no

MTK_IPO_SUPPORT=yes

MTK_LAUNCHER_ALLAPPSGRID=yes

MTK_LIVEWALLPAPER_APP=gles20

MTK_LOG2SERVER_APP=no

MTK_M4U_SUPPORT=yes

MTK_MDLOGGER_SUPPORT=yes

MTK_MODEM_SUPPORT=modem_3g

MTK_MULTI_STORAGE_SUPPORT=yes

MTK_NETWORK_TYPE_DISPLAY=yes
MTK_NOTEBOOK_SUPPORT=no

MTK_OMACP_SUPPORT=yes

MTK_PHONE_VT_MM_RINGTONE=yes
MTK_PHONE_VT_VOICE_ANSWER=yes

MTK_RTP_OVER_RTSP_SUPPORT=yes

MTK_SEC_BOOT=ATTR_SBOOT_ONLY_ENABLE_ON_SCHIP
MTK_SEC_CHIP_SUPPORT=no
MTK_SEC_MODEM_AUTH=no
MTK_SEC_MODEM_ENCODE=no
MTK_SEC_USBDL=ATTR_SUSBDL_ONLY_ENABLE_ON_SCHIP
MTK_SENSOR_SUPPORT=yes

MTK_SHARE_MODEM_CURRENT=1
MTK_SHARE_MODEM_SUPPORT=1

MTK_SMSREG_APP=yes
MTK_SMS_FILTER_SUPPORT=yes

MTK_SNS_FACEBOOK_APP=yes
MTK_SNS_FLICKR_APP=yes
MTK_SNS_KAIXIN_APP=yes
MTK_SNS_RENREN_APP=yes
MTK_SNS_SINAWEIBO_APP=yes
MTK_SNS_SINAWEIBO_TEST=no
MTK_SNS_SUPPORT=yes
MTK_SNS_TWITTER_APP=yes

MTK_SOUNDRECORDER_APP=yes

MTK_TVOUT_SUPPORT=no

MTK_USES_HD_VIDEO=yes
MTK_USES_STAGEFRIGHT_DEFAULT_CODE=no

MTK_VIDEOPLAYER2_APP=yes
MTK_VIDEOPLAYER_APP=no

MTK_VLW_APP=no

MTK_VT3G324M_SUPPORT=yes

MTK_WAPI_SUPPORT=yes

MTK_WCDMA_SUPPORT=no

MTK_WML_SUPPORT=yes

MTK_WORLD_CLOCK_WIDGET_APP=yes

OPTR_SPEC_SEG_DEF=NONE

RESOURCE_OVERLAY_SUPPORT=


# for 6515 project only, Haizhen Wang (60359)
MTK_RTSP_BITRATE_ADAPTATION_SUPPORT=yes

# for athens15 project only, WS Chen (26760)
MTK_SEC_CHIP_SUPPORT=no

MTK_TODOS_APP=no

KBUILD_OUTPUT_SUPPORT=yes

MTK_MTKLOGGER_SUPPORT=yes

MTK_BENCHMARK_BOOST_TP=no

MTK_POWER_SAVING_SWITCH_UI_SUPPORT=no

MTK_QQBROWSER_SUPPORT=no

MTK_TENCENT_MOBILE_MANAGER_SLIM_SUPPORT=no

MTK_TENCENT_MOBILE_MANAGER_NORMAL_SUPPORT=no

NAND_OTP_SUPPORT = no 


MTK_MD_SHUT_DOWN_NT = no 

MTK_BACKUPANDRESTORE_APP_DATA = no

MTK_TETHERINGIPV6_SUPPORT = yes 


MTK_IPV6_TETHER_NDP_MODE = no 


MTK_CACHE_MERGE_SUPPORT = no 

MTK_MEMORY_COMPRESSION_SUPPORT=no

MTK_SEC_SECRO_AC_SUPPORT = no 


MTK_SEC_MODEM_NVRAM_ANTI_CLONE = no 


MTK_GB_HW_KEY_REMAPPING = no


MTK_UART_USB_SWITCH = no 

MTK_SIP_SUPPORT = no


MTK_WIFI_P2P_SUPPORT =no
MTK_BAIDU_MAP_SUPPORT = no
MTK_BAIDU_SEARCH_BAR_SUPPORT = no

MTK_MVNO_SUPPORT = yes

# When this option set to yes, the MTK Bluetooth solution will be included.
MTK_BT_SUPPORT=yes

# When this option set to yes, the Bluetooth stack will comply to Bluetooth Sepc 2.1 (BR/EDR).
MTK_BT_21_SUPPORT=yes

# When this option set to yes, the Bluetooth stack will comply to Bluetooth Sepc 3.0 (no High Speed).
MTK_BT_30_SUPPORT=yes

# When this option set to yes, the Bluetooth stack will comply to Bluetooth Sepc 3.0 (High Speed).
MTK_BT_30_HS_SUPPORT=no

# When this option set to yes, the Bluetooth stack will comply to Bluetooth Sepc 4.0 (Low Energy).
MTK_BT_40_SUPPORT=no

# When this option set to yes, the feature "FM over BT controller" will be enabled. This is dependent on the chip capability.
MTK_BT_FM_OVER_BT_VIA_CONTROLLER=no

# When this option set to yes, the Bluetooth "Advanced Audio Distribution Profile" (A2DP) will be enabled.
MTK_BT_PROFILE_A2DP=yes

# When this option set to yes, the Bluetooth "Audio/Video Remote Control Profile" (AVRCP) version 1.0 will be enabled.
MTK_BT_PROFILE_AVRCP=yes

# When this option set to yes, the Bluetooth "Audio/Video Remote Control Profile" (AVRCP) version 1.3 will be enabled.
MTK_BT_PROFILE_AVRCP13=no

# When this option set to yes, the Bluetooth "Audio/Video Remote Control Profile" (AVRCP) version 1.4 will be enabled.
MTK_BT_PROFILE_AVRCP14=no

# When this option set to yes, the Bluetooth "Basic Imaging Profile" (BIP) will be enabled.
MTK_BT_PROFILE_BIP=no

# When this option set to yes, the Bluetooth "Basic Printing Profile" (BPP) will be enabled.
MTK_BT_PROFILE_BPP=no

# When this option set to yes, the Bluetooth "Dial-Up Networking Profile" (DUN) will be enabled.
MTK_BT_PROFILE_DUN=no

# When this option set to yes, the Bluetooth "File Transfer Profile" (FTP) will be enabled.
MTK_BT_PROFILE_FTP=no

# When this option set to yes, the Bluetooth "Hands-Free Profile" (HFP) will be enabled.
MTK_BT_PROFILE_HFP=yes

# When this option set to yes, the Bluetooth "Human Interface Devices Profile - Host Role" (HIDH) will be enabled.
MTK_BT_PROFILE_HIDH=yes

# When this option is set to "yes", "Profile Manager" feature will be enabled and advanced UI will be showed.
MTK_BT_PROFILE_MANAGER=yes

# When this option set to yes, the Bluetooth "Message Access Profile - Client Role" (MAPC) will be enabled.
MTK_BT_PROFILE_MAPC=no

# When this option set to yes, the Bluetooth "Message Access Profile - Server Role" (MAPS) will be enabled.
MTK_BT_PROFILE_MAPS=no

# When this option set to yes, the Bluetooth "Object Push Profile" (OPP) will be enabled.
MTK_BT_PROFILE_OPP=yes

# When this option set to yes, the Bluetooth "Personal Area Networking Profile" (PAN) will be enabled.
MTK_BT_PROFILE_PAN=yes

# When this option set to yes, the Bluetooth "Phone Book Access Profile" (PBAP) will be enabled.
MTK_BT_PROFILE_PBAP=yes

# When this option set to yes, the Bluetooth Low Energy "Proximity Profile - Monitor Role" (PRXM) will be enabled.
MTK_BT_PROFILE_PRXM=no

# When this option set to yes, the Bluetooth Low Energy "Proximity Profile - Reporter Role" (PRXR) will be enabled.
MTK_BT_PROFILE_PRXR=no

# When this option set to yes, the Bluetooth "SIM Access Profile" (SIMAP) will be enabled.
MTK_BT_PROFILE_SIMAP=no

# When this option set to yes, the Bluetooth "Serial Port Profile" (SPP) will be enabled.
MTK_BT_PROFILE_SPP=yes

# When this option set to yes, the Bluetooth Low Energy "Time Profile - Client Role" will be enabled.
MTK_BT_PROFILE_TIMEC=no

# When this option set to yes, the Bluetooth Low Energy "Time Profile - Server Role" will be enabled.
MTK_BT_PROFILE_TIMES=no

# This feature option is used for LE debug purpose. Should not be enabled by customer.
MTK_BT_40_LE_STANDALONE = no


MTK_MDM_FUMO = no 


MTK_MDM_SCOMO = no 


MTK_MDM_LAWMO = no 


MTK_SUPPORT_MJPEG = no 


MTK_MATV_SERIAL_IF_SUPPORT = no 


MTK_GAMELOFT_LBC_ULC_WW_APP = no


MTK_GAMELOFT_AVENGERS_ULC_WW_APP = no


MTK_GAMELOFT_WONDERZOO_ULC_WW_APP = no


MTK_GAMELOFT_WONDERZOO_ULC_CN_APP = no


MTK_GAMELOFT_LBC_ULC_CN_APP = no


MTK_GAMELOFT_AVENGERS_ULC_CN_APP = no


MTK_SIM_RECOVERY=yes

MTK_SIM_HOT_SWAP=yes

MTK_RAT_BALANCING=no

MTK_ENS_SUPPORT=no

MTK_IMEI_LOCK=no

MTK_RAT_WCDMA_PREFERRED=yes

MTK_SINA_WEIBO_SUPPORT=no

MTK_AUDIO_HD_REC_SUPPORT = yes

MTK_GAMELOFT_GLL_ULC_CN_APP=no
MTK_GAMELOFT_GLL_ULC_WW_APP=no

MTK_VIDEOWIDGET_APP = no
MTK_CONTACT_WIDGET_APP = no

MTK_MEM_PRESERVED_MODE_ENABLE = no 

MTK_VIDEO_1080P=no

MTK_USES_VR_DYNAMIC_QUALITY_MECHANISM=no

OP01_CTS_COMPATIBLE = no 


MTK_MMPROFILE_SUPPORT = no 


MTK_PERFSERVICE_SUPPORT = yes 


MTK_AUTO_SANITY = yes 


MTK_NO_TRAN_ANIM = no 

MTK_BQ24158_SUPPORT = no