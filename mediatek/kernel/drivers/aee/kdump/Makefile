
ifeq ($(MTK_INTERNAL), yes)
ifeq ($(MTK_USE_RESERVED_EXT_MEM), yes)
KBUILD_CFLAGS += -DMTK_USE_RESERVED_EXT_MEM
endif
endif

obj-$(CONFIG_MTK_AEE_KDUMP) += kdump.o
kdump-y := kdump_main.o kdump_elf.o kdump_usb.o kdump_sd.o
