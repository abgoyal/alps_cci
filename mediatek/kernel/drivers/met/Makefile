
ifeq ($(MET_TOP),)

include $(MTK_ROOT_BUILD)/Makefile

#export CONFIG_MTK_MET=y

obj-y := core/

platform-dir := platform/$(call lc,$(MTK_PLATFORM))/
check-platform-dir-exist := $(wildcard $(srctree)/mediatek/kernel/drivers/met/platform/$(call lc,$(MTK_PLATFORM))/)
ifneq ($(check-platform-dir-exist),)
	obj-y += $(platform-dir)
endif

ifeq ($(CONFIG_THERMAL),y)
 	EXTRA_CFLAGS +=	-DNO_MTK_THERMAL_GET_TEMP=0
else
 	EXTRA_CFLAGS +=	-DNO_MTK_THERMAL_GET_TEMP=1
endif

subdir-ccflags-y := -I$(srctree)/mediatek/kernel/drivers/met


else
ifneq ($(KERNELRELEASE),)

ifeq ($(CONFIG_MTK_MET),)
obj-y := core/
endif

ifneq ($(MET_PLATFORM),)
obj-y += platform/$(MET_PLATFORM)/
endif

subdir-ccflags-y := -I$(MET_TOP)/met

else

ifeq ($(KDIR),)
$(error Error! You should execute "make" in parent directory)
endif

export KBUILD_OUTPUT_SUPPORT=yes

default:
	$(MAKE) -C $(KDIR) M=$$PWD
clean:
	$(MAKE) -C $(KDIR) M=$$PWD clean

endif

endif
