/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
 */

/*
 * Copyright (C) 2009
 * Guennadi Liakhovetski, DENX Software Engineering, <lg@denx.de>
 *
 * Description:
 * Helper routines for i.MX3x SoCs from Freescale, needed by the fsl_usb2_udc.c
 * driver to function correctly on these systems.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/fsl_devices.h>
#include <linux/platform_device.h>

#include <mach/hardware.h>

static struct clk *mxc_ahb_clk;
static struct clk *mxc_usb_clk;

int fsl_udc_clk_init(struct platform_device *pdev)
{
	struct fsl_usb2_platform_data *pdata;
	unsigned long freq;
	int ret;

	pdata = pdev->dev.platform_data;

	if (!cpu_is_mx35()) {
		mxc_ahb_clk = clk_get(&pdev->dev, "usb_ahb");
		if (IS_ERR(mxc_ahb_clk))
			return PTR_ERR(mxc_ahb_clk);

		ret = clk_enable(mxc_ahb_clk);
		if (ret < 0) {
			dev_err(&pdev->dev, "clk_enable(\"usb_ahb\") failed\n");
			goto eenahb;
		}
	}

	/* make sure USB_CLK is running at 60 MHz +/- 1000 Hz */
	mxc_usb_clk = clk_get(&pdev->dev, "usb");
	if (IS_ERR(mxc_usb_clk)) {
		dev_err(&pdev->dev, "clk_get(\"usb\") failed\n");
		ret = PTR_ERR(mxc_usb_clk);
		goto egusb;
	}

	if (!cpu_is_mx51()) {
		freq = clk_get_rate(mxc_usb_clk);
		if (pdata->phy_mode != FSL_USB2_PHY_ULPI &&
		    (freq < 59999000 || freq > 60001000)) {
			dev_err(&pdev->dev, "USB_CLK=%lu, should be 60MHz\n", freq);
			ret = -EINVAL;
			goto eclkrate;
		}
	}

	ret = clk_enable(mxc_usb_clk);
	if (ret < 0) {
		dev_err(&pdev->dev, "clk_enable(\"usb_clk\") failed\n");
		goto eenusb;
	}

	return 0;

eenusb:
eclkrate:
	clk_put(mxc_usb_clk);
	mxc_usb_clk = NULL;
egusb:
	if (!cpu_is_mx35())
		clk_disable(mxc_ahb_clk);
eenahb:
	if (!cpu_is_mx35())
		clk_put(mxc_ahb_clk);
	return ret;
}

void fsl_udc_clk_finalize(struct platform_device *pdev)
{
	struct fsl_usb2_platform_data *pdata = pdev->dev.platform_data;

	/* ULPI transceivers don't need usbpll */
	if (pdata->phy_mode == FSL_USB2_PHY_ULPI) {
		clk_disable(mxc_usb_clk);
		clk_put(mxc_usb_clk);
		mxc_usb_clk = NULL;
	}
}

void fsl_udc_clk_release(void)
{
	if (mxc_usb_clk) {
		clk_disable(mxc_usb_clk);
		clk_put(mxc_usb_clk);
	}
	if (!cpu_is_mx35()) {
		clk_disable(mxc_ahb_clk);
		clk_put(mxc_ahb_clk);
	}
}