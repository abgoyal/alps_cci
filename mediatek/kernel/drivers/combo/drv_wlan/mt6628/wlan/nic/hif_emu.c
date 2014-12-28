/*
** $Id: //Department/DaVinci/BRANCHES/MT6628_WIFI_DRIVER_V2_1/nic/hif_emu.c#1 $
*/

/*! \file   nic.c
    \brief  Functions that provide operation in NIC's (Network Interface Card) point of view.

    This file includes functions which unite multiple hal(Hardware) operations
    and also take the responsibility of Software Resource Management in order
    to keep the synchronization with Hardware Manipulation.
*/

/*******************************************************************************
* Copyright (c) 2007 MediaTek Inc.
*
* All rights reserved. Copying, compilation, modification, distribution
* or any other use whatsoever of this material is strictly prohibited
* except in accordance with a Software License Agreement with
* MediaTek Inc.
********************************************************************************
*/

/*******************************************************************************
* LEGAL DISCLAIMER
*
* BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND
* AGREES THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK
* SOFTWARE") RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE
* PROVIDED TO BUYER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY
* DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT
* LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
* PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE
* ANY WARRANTY WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY
* WHICH MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK
* SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY
* WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE
* FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION OR TO
* CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
* BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
* LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL
* BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT
* ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY
* BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
* THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
* WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT
* OF LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING
* THEREOF AND RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN
* FRANCISCO, CA, UNDER THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE
* (ICC).
********************************************************************************
*/

/*
** $Log: hif_emu.c $
 *
 * 03 18 2011 cp.wu
 * [WCXRP00000577] [MT6620 Wi-Fi][Driver][FW] Create V2.0 branch for firmware and driver
 * create V2.0 driver release based on label "MT6620_WIFI_DRIVER_V2_0_110318_1600" from main trunk
 *
 * 03 07 2011 terry.wu
 * [WCXRP00000521] [MT6620 Wi-Fi][Driver] Remove non-standard debug message
 * Toggle non-standard debug messages to comments.
 *
 * 07 08 2010 cp.wu
 *
 * [WPD00003833] [MT6620 and MT5931] Driver migration - move to new repository.
 *
 * 06 06 2010 kevin.huang
 * [WPD00003832][MT6620 5931] Create driver base
 * [MT6620 5931] Create driver base
**  \main\maintrunk.MT6620WiFiDriver_Prj\10 2009-11-16 22:40:11 GMT mtk02752
**  change prTxCtrl->aucFreeBuf to prTxCtrl->rTc.aucFreeBuf for TX_CTRL_T change
**  \main\maintrunk.MT6620WiFiDriver_Prj\9 2009-11-13 13:54:21 GMT mtk01084
**  \main\maintrunk.MT6620WiFiDriver_Prj\8 2009-11-11 10:36:04 GMT mtk01084
**  \main\maintrunk.MT6620WiFiDriver_Prj\7 2009-11-09 22:56:36 GMT mtk01084
**  modify HW access routines
**  \main\maintrunk.MT6620WiFiDriver_Prj\6 2009-11-04 14:11:22 GMT mtk01084
**  add TX/ RX burst test caseS
**  \main\maintrunk.MT6620WiFiDriver_Prj\5 2009-10-30 18:17:45 GMT mtk01084
**  modify test cases
**  \main\maintrunk.MT6620WiFiDriver_Prj\4 2009-10-29 19:54:34 GMT mtk01084
**  add new emulation functions
**  \main\maintrunk.MT6620WiFiDriver_Prj\3 2009-10-23 16:08:26 GMT mtk01084
**  \main\maintrunk.MT6620WiFiDriver_Prj\2 2009-10-13 22:18:17 GMT mtk01084
**  fix data-type warning
**  \main\maintrunk.MT6620WiFiDriver_Prj\1 2009-10-13 22:12:03 GMT mtk01084
**
*/

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/
#include "precomp.h"
#include "hif_emu.h"

#if CFG_HIF_EMULATION_TEST

VOID
nicSDIOReadIntStatus (
    IN P_ADAPTER_T prAdapter,
    OUT PUINT_32   pu4IntStatus
    );

BOOLEAN
sdioCmd52ByteReadWrite (
    PDEVICE_EXTENSION     prDx,
    UINT_32               u4Address,
    PUCHAR                pucData,
    UCHAR                 ucFuncNo,
    SD_TRANSFER_DIRECTION rRwFlag
    );

NDIS_STATUS
sdioConfigProperty (
    IN PDEVICE_EXTENSION prDevExt,
    IN SD_REQUEST_FUNCTION eRequestFunction,
    IN SDBUS_PROPERTY eProperty,
    OUT PUINT_8      aucBuffer,
    IN UINT_32       u4BufLen
    );

VOID
emuWorkItemProc (
    IN struct _NDIS_WORK_ITEM *  pWorkItem,
    IN PVOID  Context
    );

BOOLEAN
emuCheckChipID (
    IN P_ADAPTER_T prAdapter
    );

BOOLEAN
emuCheckPowerOnResetFunc (
    IN P_ADAPTER_T prAdapter
    );

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/
UINT_8 aucFBR2[] =
{
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x30,0x0, 0x0, 0x0, 0x0, 0x0, 0x0
};

UINT_8 aucFBR1[] =
{
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x20,0x0, 0x0, 0x0, 0x0, 0x0, 0x0
};

UINT_8 aucCccr[] =
{
    0x32,0x2, 0x2, 0x2, 0x3, 0x0, 0x0, 0x2, 0x17,0x0, 0x10,0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
};

UINT_8 aucFn0Cis[] =
{
    0x21,0x02,0x0C,0x00,0x22,0x04,0x00,0x00,0x08,0x5A,0x20,0x04,0x7A,0x03,0x20,0x66,0x15,
    0x2F,0x08,0x00,0x4D,0x65,0x64,0x69,0x61,0x54,0x65,0x6B,0x20,0x4D,0x54,0x36,0x36,0x32,
    0x30,0x20,0x46,0x4D,0x2B,0x42,0x54,0x2B,0x47,0x50,0x53,0x2B,0x57,0x69,0x46,0x69,0x20,
    0x43,0x6F,0x6D,0x62,0x6F,0x20,0x43,0x68,0x69,0x70,0x00,0x00,0x00,0xFF,0xFF,
};

UINT_8 aucFn1Cis[] =
{
    0x21,0x02,0x0C,0x00,0x22,0x2A,0x01,0x01,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x03,0x00,0x08,0x00,0x00,0x7F,0x00,0x64,0x96,0xC8,0x0A,0x0F,0x14,0x32,0x00,0xDC,0x05,
    0x00,0x00,0x96,0x00,0xC8,0x00,0x96,0x00,0xC8,0x00,0x96,0x00,0xC8,0x00,0xFF,
};

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/
UINT_8 g_aucEmuBuf[4096*16 + 256/* for RX enhance mode data struct*/];

LARGE_INTEGER g_rSysTimeRecord;
LARGE_INTEGER g_rTxTotalByteCount;
UINT_32 g_u4TxTotalPacketCount;

BOOLEAN fgEmuTxBurst = FALSE, fgEmuRxBurst = FALSE;

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/


/*******************************************************************************
*                   F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/
VOID
emuInit (
    IN P_ADAPTER_T prAdapter
    )
{
    BOOLEAN fgResult;
    HAL_LP_OWN_CLR_OK(prAdapter, &fgResult);
    if (!fgResult) {
        DBGLOG(INIT, ERROR, ("Init own-back fail\n"));
    }

    NdisInitializeWorkItem(&prAdapter->prGlueInfo->rWorkItem, emuWorkItemProc, prAdapter);
}

BOOLEAN
emuStart (
    IN P_ADAPTER_T prAdapter
    )
{
    UINT_32 i, j, k;
    BOOLEAN status = TRUE;

    BOOLEAN fgResult;
    HAL_LP_OWN_CLR_OK(prAdapter, &fgResult);
    if (!fgResult) {
        DBGLOG(INIT, ERROR, ("Init own-back fail\n"));
    }

    nicPutMailbox(prAdapter, 0, 0);
    nicPutMailbox(prAdapter, 1, 0);

    // use polling mode (mask global interrupt)
    nicDisableInterrupt(prAdapter);

    do {
        RUN_TEST_CASE((emuInitChkCis(prAdapter)));

        //RUN_TEST_CASE((emuChkIntEn(prAdapter)));

        RUN_TEST_CASE((emuCheckChipID(prAdapter)));

        //RUN_TEST_CASE((emuCheckPowerOnResetFunc(prAdapter)));

        RUN_TEST_CASE((emuCheckTxCount(prAdapter, FALSE)));
        RUN_TEST_CASE((emuCheckTxCount(prAdapter, TRUE)));

#if 0
        RUN_TEST_CASE((emuResetTxCount(prAdapter)));

        RUN_TEST_CASE((emuChkTxState(prAdapter)));
        RUN_TEST_CASE((emuChkRxState(prAdapter)));

        RUN_TEST_CASE((emuRxPacketLenOverflow(prAdapter)));

        RUN_TEST_CASE((emuReadHalfRxPacket(prAdapter)));
#endif
#if 0 // wait for bug fixed (done @ 20091109, due to HAL access single packet with 2 commands)
        // RX AGG
        for (i = 4; i < 400; i++) {
            RUN_TEST_CASE((emuRxPacketAggN(prAdapter,
                           0,
                           FALSE,
                           i,
                           10,
                           6)));
       }
#endif

#if 0 // wait for bug fixed (done @ 20091109, due to HAL access single packet with 2 commands)
        // RX enhance mode
        for (i = 401; i < 450; i++) {
            RUN_TEST_CASE((emuRxPacketAggN(prAdapter,
                           0,
                           TRUE,
                           i,
                           3,
                           2)));
       }
#endif

#if 1
        for (i = 512; i < 550; i++) {
            RUN_TEST_CASE((emuRxPacket1(prAdapter,
                                        0,
                                        i,
                                        TRUE,
                                        TRUE,
                                        TRUE)));
        }
#endif
#if 0 // wait for bug fixed (done @ 20091109, due to HAL access single packet with 2 commands)
        // RX enhance mode
        for (i = 10; i < 50; i++) {
            RUN_TEST_CASE((emuRxPacketAggN(prAdapter,
                           0,
                           TRUE,
                           i,
                           3,
                           2)));
       }
#endif


#if 0
        for (i = 1005; i < 1110; i++) {
            RUN_TEST_CASE((emuRxPacket1(prAdapter,
                                        0,
                                        i,
                                        FALSE,
                                        FALSE,
                                        FALSE)));
            RUN_TEST_CASE((emuRxPacket1(prAdapter,
                                        0,
                                        i,
                                        TRUE,
                                        FALSE,
                                        FALSE)));
        }
        for (i = 1005; i < 1110; i++) {
            RUN_TEST_CASE((emuRxPacket1(prAdapter,
                                        0,
                                        i,
                                        TRUE,
                                        TRUE,
                                        TRUE)));
        }
#endif
#if 0
        /* mix RX0, RX1 */
        for (i = 1; i < 10; i++) {
            RUN_TEST_CASE((emuRxPacketAggN(prAdapter,
                                           0,
                                           (BOOLEAN)0,
                                           (BOOLEAN)400,
                                           (BOOLEAN)5,
                                           (BOOLEAN)i)));
        }
#endif

#if 0//test temp!
{
UINT_32 i, u4PortId, u4Len, u4AggNum, u4MaxReadAggNum, fgRxEn;

for (fgRxEn = 0; fgRxEn < 2; fgRxEn++) {
    for (u4Len = 1500; u4Len < 4096; u4Len++) {
        for (u4AggNum = 15; u4AggNum <= 16; u4AggNum++) {
            for (u4MaxReadAggNum = 14; u4MaxReadAggNum <= 16; u4MaxReadAggNum++) {
                /* mix RX0, RX1 */
                RUN_TEST_CASE((emuRxPacketAggN(prAdapter,
                                               0,
                                               (BOOLEAN)fgRxEn,
                                               u4Len,
                                               u4AggNum,
                                               u4MaxReadAggNum)));
                // RX1 only link 2 descriptors
                if (u4AggNum <= 2) {
                    RUN_TEST_CASE((emuRxPacketAggN(prAdapter,
                                                   1,
                                                   (BOOLEAN)fgRxEn,
                                                   u4Len,
                                                   u4AggNum,
                                                   u4MaxReadAggNum)));
               }
            }
        }
    }
}
}
#endif

#if 0
        {
        UINT_32 i, u4PortId, u4Len, u4AggNum, u4MaxReadAggNum, fgRxEn;

        for (fgRxEn = 0; fgRxEn < 2; fgRxEn++) {
            for (u4Len = 4; u4Len < 4096; u4Len++) {
                for (u4AggNum = 1; u4AggNum <= 16; u4AggNum++) {
                    for (u4MaxReadAggNum = 1; u4MaxReadAggNum <= 16; u4MaxReadAggNum++) {
                        /* mix RX0, RX1 */
                        RUN_TEST_CASE((emuRxPacketAggN(prAdapter,
                                                       0,
                                                       (BOOLEAN)fgRxEn,
                                                       u4Len,
                                                       u4AggNum,
                                                       u4MaxReadAggNum)));
                        // RX1 only link 2 descriptors
                        if (u4AggNum <= 2) {
                            RUN_TEST_CASE((emuRxPacketAggN(prAdapter,
                                                           1,
                                                           (BOOLEAN)fgRxEn,
                                                           u4Len,
                                                           u4AggNum,
                                                           u4MaxReadAggNum)));
                       }
                    }
                }
            }
        }
        }
#endif

#if 0
        {
        UINT_32 i, u4PortId, u4Len;
        UINT_32 fgIntrEn, fgRxEn, fgMboxRc;

        for (u4PortId = 0; u4PortId < 2; u4PortId++) {
            for (u4Len = 4; u4Len < 4096; u4Len++) {
                for (fgIntrEn = 0; fgIntrEn < 2; fgIntrEn++) {
                    for (fgRxEn = 0; fgRxEn < 2; fgRxEn++) {
                        for (fgMboxRc = 0; fgMboxRc < 2; fgMboxRc++) {
                            RUN_TEST_CASE((emuRxPacket1(prAdapter,
                                                        u4PortId,
                                                        u4Len,
                                                        (BOOLEAN)fgIntrEn,
                                                        (BOOLEAN)fgRxEn,
                                                        (BOOLEAN)fgMboxRc)));
                        }
                    }
                }
            }
        }
        }
#endif

#if 0 //20091028 pass overnight
#if 1
        // 20091019: still buggy in phase 1 FPGA now
        RUN_TEST_CASE((emuLPown_illegal_access(prAdapter)));
#endif

        //RUN_TEST_CASE((emuSendPacket1(prAdapter, 0, FALSE)));
#if 1
        for (i = 0; i < 2; i++) {
            for (j = 0; j < 2; j++) {
                RUN_TEST_CASE((emuSendPacket1(prAdapter, i, j)));
            }
        }
#endif
        //RUN_TEST_CASE((emuSendPacketAggN(prAdapter, 0, 2, 4020, 4095, FALSE)));
#if 1
        for (j = 0; j < 2; j++) {
            for (i = 2; i < 16; i++) {
                /* mix TX0 and TX1 */
                RUN_TEST_CASE((emuSendPacketAggN(prAdapter, 0, i, 4, 4095, j)));
                RUN_TEST_CASE((emuSendPacketAggN(prAdapter, 1, 2, 4, 4095, j)));
            }
        }
#endif

        RUN_TEST_CASE((emuRxPacketLenChk(prAdapter)));

        RUN_TEST_CASE((emuIntrEnhanceChk(prAdapter)));

#endif//20091028 pass overnight

#if 0
            RUN_TEST_CASE((emuSoftwareInterruptLoopback(prAdapter, TRUE)));
            RUN_TEST_CASE((emuSoftwareInterruptLoopback(prAdapter, FALSE)));

            RUN_TEST_CASE((emuLPown_ownback_stress(prAdapter, 1000)));

            RUN_TEST_CASE((emuMailboxLoopback(prAdapter, FALSE)));
            RUN_TEST_CASE((emuMailboxLoopback(prAdapter, TRUE)));
#endif

    //    RUN_TEST_CASE((emuMixTxRxStress(prAdapter)));

    } while (FALSE);

    /* reset event */
    nicPutMailbox(prAdapter, 0, 0x52455345); // RESE
    nicPutMailbox(prAdapter, 1, 0x545F5746); // T_WF
    nicSetSwIntr(prAdapter, BIT(16));

    // restore global interrupt
    nicEnableInterrupt(prAdapter);

    DBGLOG(EMU, TRACE, ("\n"));
    DBGLOG(EMU, TRACE, ("==============================================\n"));
    DBGLOG(EMU, TRACE, (" ------ Emulation test complete [%s] ------\n", (status) ? "pass" : "fail"));
    DBGLOG(EMU, TRACE, ("==============================================\n\n"));

    return status;
}

BOOLEAN
emuPollingIntrBit (
    IN P_ADAPTER_T prAdapter,
    IN UINT_32     u4IntrBit,
    IN UINT_32     u4Result
    )
{
    UINT_32 u4Value;
    UINT_32 i;
    BOOLEAN fgReturn = FALSE;

    for (i = 0; i < 2000; i++) {
        HAL_MCR_RD(prAdapter, MCR_WHISR, &u4Value);
        if ((u4Value & u4IntrBit) == u4Result) {
            fgReturn = TRUE;
            break;
        } else {
            kalMdelay(5);
        }
    }
    if (!fgReturn) {
        DBGLOG(INIT, ERROR, ("u4Value: 0x%08x, u4IntrBit: 0x%08x, u4Result: 0x%08x\n",
                                u4Value, u4IntrBit, u4Result));
    }

    return fgReturn;
}

BOOLEAN
emuMailboxLoopback (
    IN P_ADAPTER_T prAdapter,
    IN BOOLEAN fgReadClearChk
    )
{
    UINT_32 i, j, loop = 0;
    UINT_32 u4Mbox0, u4Mbox1;
    UINT_32 u4WHIERValue;
    UINT_32 u4WHCRValue, u4Value;
    BOOLEAN status = TRUE;
    LARGE_INTEGER rCurrentSysTime;

    DBGLOG(EMU, TRACE, (".....start....\n"));
    DBGLOG(EMU, TRACE, ("fgReadClearChk=%d\n", fgReadClearChk));

    // disable SW interrupt
    HAL_MCR_RD(prAdapter, MCR_WHIER, &u4WHIERValue);
    HAL_MCR_WR(prAdapter, MCR_WHIER, u4WHIERValue & ~BITS(8,31));

    HAL_MCR_RD(prAdapter, MCR_WHCR, &u4WHCRValue);
    HAL_SET_MAILBOX_READ_CLEAR(prAdapter, fgReadClearChk);

    nicPutMailbox(prAdapter, 0, HIF_TC_MBOX_LB | HIF_TEST_CASE_START);
    nicPutMailbox(prAdapter, 1, 0);
    nicSetSwIntr(prAdapter, BIT(31));

    // check SW interrupt from FW
    if (emuPollingIntrBit(prAdapter, BIT(31), BIT(31))) {
    } else {
        DBGLOG(EMU, ERROR, ("emuPollingIntrBit polling b31 failed\n"));
//        ASSERT(0);
        return FALSE;
    }

    kalMdelay(100);

    // put message into mailbox
#if 1
    // use random start for saving test time
    for (i = 10000; i <= 12000 ; i++) {
#else
//    for (i = 0; i <= BITS(0, 31) ; i++) {
#endif
        loop += 1;
        nicPutMailbox(prAdapter, 0, i);
        nicPutMailbox(prAdapter, 1, ~i);

        // set SW interrupt to FW
        nicSetSwIntr(prAdapter, BIT(30));

        // check SW interrupt from FW
        if (emuPollingIntrBit(prAdapter, BIT(30), BIT(30))) {
        } else {
            DBGLOG(EMU, ERROR, ("emuPollingIntrBit polling b30: 0x%x failed [loop=%d]\n", i, loop));
            status = FALSE;
            break;
        }

        nicGetMailbox(prAdapter, 0, &u4Mbox0);
        nicGetMailbox(prAdapter, 1, &u4Mbox1);
        if ((u4Mbox0 != i) || (u4Mbox1 != ~i)) {
            DBGLOG(EMU, ERROR, ("emuMailboxLoopback fail: (u4Mbox0 write 0x%x, read: 0x%x), (u4Mbox1 write 0x%x, read: 0x%x) [loop=%d]\n",
                                i, u4Mbox0, ~i, u4Mbox1, loop));
            status = FALSE;
            break;
        }

        nicGetMailbox(prAdapter, 0, &u4Mbox0);
        nicGetMailbox(prAdapter, 1, &u4Mbox1);
        if (fgReadClearChk) {
            if ((u4Mbox0 != 0) || (u4Mbox1 != 0)) {
                DBGLOG(EMU, ERROR, ("read is not cleared: (u4Mbox0 read: 0x%x), (u4Mbox1 read: 0x%x) [loop=%d]\n",
                                    u4Mbox0, u4Mbox1, loop));
                status = FALSE;
                break;
            }
        } else {
            if ((u4Mbox0 == 0) || (u4Mbox1 == 0)) {
                DBGLOG(EMU, ERROR, ("read is cleared: (u4Mbox0 read: 0x%x), (u4Mbox1 read: 0x%x) [loop=%d]\n",
                                    u4Mbox0, u4Mbox1, loop));
                status = FALSE;
                break;
            }
        }
        if ((loop % 1000) == 0) {
            DBGLOG(EMU, TRACE, ("[loop=%d] i=0x%x \n", loop, i));
        }
    }

    nicPutMailbox(prAdapter, 0, HIF_TC_MBOX_LB);
    nicPutMailbox(prAdapter, 1, 1);
    nicSetSwIntr(prAdapter, BIT(31));

    // recover SW interrupt
    HAL_MCR_WR(prAdapter, MCR_WHIER, u4WHIERValue);
    HAL_MCR_WR(prAdapter, MCR_WHCR,u4WHCRValue);

    DBGLOG(EMU, TRACE, (".....end....\n"));
    return status;
}

BOOLEAN
emuSoftwareInterruptLoopback (
    IN P_ADAPTER_T prAdapter,
    IN BOOLEAN fgIntrReadClear
    )
{
    UINT_32 u4WHIERValue, u4WHCRValue;
    UINT_32 i;
    BOOLEAN status = TRUE;

    DBGLOG(EMU, TRACE, (".....start....\n"));
    DBGLOG(EMU, TRACE, ("fgIntrReadClear= %d\n", fgIntrReadClear));

    // disable SW interrupt
    HAL_MCR_RD(prAdapter, MCR_WHIER, &u4WHIERValue);
    HAL_MCR_WR(prAdapter, MCR_WHIER, u4WHIERValue & ~BITS(8,31));

    HAL_MCR_RD(prAdapter, MCR_WHCR, &u4WHCRValue);
    HAL_MCR_WR(prAdapter, MCR_WHCR, (fgIntrReadClear) ?
                (u4WHCRValue & ~ WHCR_W_INT_CLR_CTRL) : (u4WHCRValue | WHCR_W_INT_CLR_CTRL)
                );


    nicPutMailbox(prAdapter, 0, HIF_TC_SW_INT | HIF_TEST_CASE_START);
    nicPutMailbox(prAdapter, 1, 0);
    nicSetSwIntr(prAdapter, BIT(31));
#if 0
    // check SW interrupt from FW
    if (emuPollingIntrBit(prAdapter, BIT(31), BIT(31))) {
    } else {
        DBGLOG(EMU, ERROR, ("emuPollingIntrBit polling b31 failed\n"));
//        ASSERT(0);
        return FALSE;
    }
#endif
    kalMdelay(500);

    // put message into mailbox
    for (i = 16; i < 31 ; i++) {
        // set SW interrupt to FW
        nicSetSwIntr(prAdapter, BIT(i));

        // check SW interrupt from FW
        if (emuPollingIntrBit(prAdapter, BIT(i), BIT(i))) {
        } else {
            DBGLOG(EMU, WARN, ("emuSoftwareInterruptLoopback: bit%d (0x%x) failed\n", i, BIT(i)));
//            ASSERT(0);
            status = FALSE;
            break;
        }
    }

    nicPutMailbox(prAdapter, 0, HIF_TC_SW_INT);
    nicPutMailbox(prAdapter, 1, 1);
    nicSetSwIntr(prAdapter, BIT(31));

    // recover SW interrupt
    HAL_MCR_WR(prAdapter, MCR_WHIER, u4WHIERValue);
    HAL_MCR_WR(prAdapter, MCR_WHCR, u4WHCRValue);

    DBGLOG(EMU, TRACE, (".....end....\n"));
    return status;
}

BOOLEAN
emuCheckTxCount (
    IN P_ADAPTER_T prAdapter,
    IN BOOLEAN fgReadByIntrEnhanMode
    )
{
    UINT_8 aucTxCount[8];
    UINT_32 u4Mbox;
    UINT_8 aucAccumTxCnt[6] = {0};
    UINT_32 i;
    ENHANCE_MODE_DATA_STRUCT_T rEnhanceDataStruct;
    UINT_32 u4WHIERValue, u4Value;
    BOOLEAN status = TRUE;

    DBGLOG(EMU, TRACE, (".....start....\n"));
    DBGLOG(EMU, TRACE, (".fgReadByIntrEnhanMode = %d\n", fgReadByIntrEnhanMode));

    // disable SW interrupt
    HAL_MCR_RD(prAdapter, MCR_WHIER, &u4WHIERValue);
    HAL_MCR_WR(prAdapter, MCR_WHIER, 0);

    // read clear
    HAL_MCR_RD(prAdapter, MCR_WHISR, &u4Value);

    // test case of CheckTxCountBySingleReadWTSR
    nicPutMailbox(prAdapter, 0, HIF_TC_COUNT_INCREASE | HIF_TEST_CASE_START);
    nicPutMailbox(prAdapter, 1, 0);
    nicSetSwIntr(prAdapter, BIT(31));
#if 0
    // check SW interrupt from FW
    if (emuPollingIntrBit(prAdapter, BIT(31), BIT(31))) {
    } else {
        DBGLOG(EMU, ERROR, ("emuPollingIntrBit polling b31 failed\n"));
//        ASSERT(0);
        return;
    }
#endif
    while (1) {
        // check SW interrupt from FW
        if (fgReadByIntrEnhanMode) {
            for (i = 0; i < 1000; i++) {
                HAL_READ_INTR_STATUS(prAdapter, sizeof(ENHANCE_MODE_DATA_STRUCT_T), (PUINT_8)&rEnhanceDataStruct);
                //if (rEnhanceDataStruct.u4WHISR & BIT(0)) {
                    aucAccumTxCnt[0] += rEnhanceDataStruct.rTxInfo.u.ucTQ0Cnt;
                    aucAccumTxCnt[1] += rEnhanceDataStruct.rTxInfo.u.ucTQ1Cnt;
                    aucAccumTxCnt[2] += rEnhanceDataStruct.rTxInfo.u.ucTQ2Cnt;
                    aucAccumTxCnt[3] += rEnhanceDataStruct.rTxInfo.u.ucTQ3Cnt;
                    aucAccumTxCnt[4] += rEnhanceDataStruct.rTxInfo.u.ucTQ4Cnt;
                    aucAccumTxCnt[5] += rEnhanceDataStruct.rTxInfo.u.ucTQ5Cnt;
                //}
                if ((aucAccumTxCnt[0] == 255) &&
                    (aucAccumTxCnt[1] == 255) &&
                    (aucAccumTxCnt[2] == 255) &&
                    (aucAccumTxCnt[3] == 255) &&
                    (aucAccumTxCnt[4] == 255) &&
                    (aucAccumTxCnt[5] == 255)) {
                    break;
                }
                kalMdelay(10);
            }
            if (i == 1000) {
                dumpMemory8((PUINT_8)&rEnhanceDataStruct, sizeof(rEnhanceDataStruct));
                DBGLOG(EMU, WARN, ("emuCheckTxCountBySingleReadWTSR: 0x%x failed\n", i));
                status = FALSE;
                HAL_READ_TX_RELEASED_COUNT(prAdapter, aucTxCount);
                for (i = 0; i < 6; i++) {
                    DBGLOG(EMU, WARN, ("aucTxCount[%d]: %d\n", i, aucTxCount[i]));
                }
                ASSERT(0);
                break;
            }
        } else {
            if (emuPollingIntrBit(prAdapter, BIT(0), BIT(0))) {
            } else {
                DBGLOG(EMU, WARN, ("emuCheckTxCount polling BIT(0) failed\n"));
//                ASSERT(0);
                status = FALSE;
                break;
            }
            HAL_READ_TX_RELEASED_COUNT(prAdapter, aucTxCount);
            for (i = 0; i < 6; i++) {
                aucAccumTxCnt[i] += aucTxCount[i];
                DBGLOG(EMU, TRACE, ("aucAccumTxCnt[%d]: %d\n", i, aucAccumTxCnt[i]));
            }
        }

        if ((aucAccumTxCnt[0] == 255) &&
            (aucAccumTxCnt[1] == 255) &&
            (aucAccumTxCnt[2] == 255) &&
            (aucAccumTxCnt[3] == 255) &&
            (aucAccumTxCnt[4] == 255) &&
            (aucAccumTxCnt[5] == 255)) {

            HAL_READ_TX_RELEASED_COUNT(prAdapter, aucTxCount);
            for (i = 0; i < 6; i++) {
                ASSERT(aucTxCount[i] == 0);
            }
            break;
        }

    }

    // test case of CheckTxCountBySingleReadWTSR
    nicPutMailbox(prAdapter, 0, HIF_TC_COUNT_INCREASE);
    nicPutMailbox(prAdapter, 1, 1);
    nicSetSwIntr(prAdapter, BIT(31));

    // check SW interrupt from FW
    if (emuPollingIntrBit(prAdapter, BIT(31), BIT(31))) {
    } else {
        DBGLOG(EMU, ERROR, ("emuPollingIntrBit polling b31 failed\n"));
//        ASSERT(0);
        status = FALSE;
    }

    HAL_READ_TX_RELEASED_COUNT(prAdapter, aucTxCount);
    for (i = 0; i < 6; i++) {
        if (aucTxCount[i]) {
            DBGLOG(EMU, WARN, ("Exit: aucTxCount[%d]: %d\n", i, aucTxCount[i]));
        }
    }
    HAL_MCR_WR(prAdapter, MCR_WHIER, u4WHIERValue);

    DBGLOG(EMU, TRACE, (".....end....\n"));
    return status;
}

BOOLEAN
emuResetTxCount (
    IN P_ADAPTER_T prAdapter
    )
{
    UINT_8 aucTxCount[8];
    UINT_32 i;
    BOOLEAN status = TRUE;

    DBGLOG(EMU, TRACE, (".....start....\n"));

    nicPutMailbox(prAdapter, 0, HIF_TC_COUNT_RESET | HIF_TEST_CASE_START);
    nicPutMailbox(prAdapter, 1, 0);
    nicSetSwIntr(prAdapter, BIT(31));

    kalMdelay(50);

    HAL_READ_TX_RELEASED_COUNT(prAdapter, aucTxCount);
    for (i = 0; i < 6; i++) {
        if (aucTxCount[i]) {
            break;
        } else {
            status = FALSE;
        }
    }

    if (status == FALSE) {
        DBGLOG(EMU, WARN, ("All-Zero Tx count!!\n"));
    }

    nicSetSwIntr(prAdapter, BIT(30));
    kalMdelay(50);

    HAL_READ_TX_RELEASED_COUNT(prAdapter, aucTxCount);
    for (i = 0; i < 6; i++) {
        if (aucTxCount[i] != 0) {
            DBGLOG(EMU, WARN, ("None-Zero Tx count!! aucTxCount[%d]: %d\n", i, aucTxCount[i]));
            status = FALSE;
            break;
        }
    }

    nicPutMailbox(prAdapter, 0, HIF_TC_COUNT_RESET);
    nicPutMailbox(prAdapter, 1, 1);
    nicSetSwIntr(prAdapter, BIT(31));

    DBGLOG(EMU, TRACE, (".....end....\n"));
    return status;
}

BOOLEAN
emuChkTxState (
    IN P_ADAPTER_T prAdapter
    )
{
    UINT_8 aucTxCount[8];
    UINT_32 u4Mbox;
    UINT_8 aucAccumTxCnt[6] = {0};
    UINT_32 i;
    ENHANCE_MODE_DATA_STRUCT_T rEnhanceDataStruct;
    UINT_32 u4WHIERValue, u4Value;
    BOOLEAN status = TRUE;

    DBGLOG(EMU, TRACE, (".....start....\n"));

    nicPutMailbox(prAdapter, 0, HIF_TC_TX_CHK_STATE | HIF_TEST_CASE_START);
    nicPutMailbox(prAdapter, 1, 0);
    nicSetSwIntr(prAdapter, BIT(31));

    kalMdelay(50);//

    do {
        // stop
        nicPutMailbox(prAdapter, 1, 0);
        nicSetSwIntr(prAdapter, BIT(30));
        kalMdelay(50);

        nicGetMailbox(prAdapter, 1, &u4Mbox);
        if (u4Mbox != 0x101) {
            DBGLOG(EMU, WARN, ("break on stop \n"));
            status = FALSE;
            break;
        }

        // start
        nicPutMailbox(prAdapter, 1, 1);
        nicSetSwIntr(prAdapter, BIT(30));
        kalMdelay(50);

        nicGetMailbox(prAdapter, 1, &u4Mbox);
        if (u4Mbox != 0x101) {
            DBGLOG(EMU, WARN, ("break on start\n"));
            status = FALSE;
            break;
        }

        // resume
        nicPutMailbox(prAdapter, 1, 2);
        nicSetSwIntr(prAdapter, BIT(30));
        kalMdelay(50);

        nicGetMailbox(prAdapter, 1, &u4Mbox);
        if (u4Mbox != 0x101) {
            DBGLOG(EMU, WARN, ("break on resume\n"));
            status = FALSE;
            break;
        }

    } while (FALSE);

    nicPutMailbox(prAdapter, 0, HIF_TC_TX_CHK_STATE);
    nicPutMailbox(prAdapter, 1, 1);
    nicSetSwIntr(prAdapter, BIT(31));


    DBGLOG(EMU, TRACE, (".....end....\n"));
    return status;
}

BOOLEAN
emuChkRxState (
    IN P_ADAPTER_T prAdapter
    )
{
    UINT_8 aucTxCount[8];
    UINT_32 u4Mbox;
    UINT_8 aucAccumTxCnt[6] = {0};
    UINT_32 i;
    ENHANCE_MODE_DATA_STRUCT_T rEnhanceDataStruct;
    UINT_32 u4WHIERValue, u4Value;
    BOOLEAN status = TRUE;

    DBGLOG(EMU, TRACE, (".....start....\n"));

    nicPutMailbox(prAdapter, 0, HIF_TC_RX_CHK_STATE | HIF_TEST_CASE_START);
    nicPutMailbox(prAdapter, 1, 0);
    nicSetSwIntr(prAdapter, BIT(31));

    kalMdelay(50);//

    do {
        // stop
        nicPutMailbox(prAdapter, 1, 0);
        nicSetSwIntr(prAdapter, BIT(30));
        kalMdelay(50);

        nicGetMailbox(prAdapter, 1, &u4Mbox);
        if (u4Mbox != 0x101) {
            DBGLOG(EMU, WARN, ("break on stop \n"));
            status = FALSE;
            break;
        }

        // start
        nicPutMailbox(prAdapter, 1, 1);
        nicSetSwIntr(prAdapter, BIT(30));
        kalMdelay(50);

        nicGetMailbox(prAdapter, 1, &u4Mbox);
        if (u4Mbox != 0x101) {
            DBGLOG(EMU, WARN, ("break on start\n"));
            status = FALSE;
            break;
        }

        // resume
        nicPutMailbox(prAdapter, 1, 2);
        nicSetSwIntr(prAdapter, BIT(30));
        kalMdelay(50);

        nicGetMailbox(prAdapter, 1, &u4Mbox);
        if (u4Mbox != 0x101) {
            DBGLOG(EMU, WARN, ("break on resume\n"));
            status = FALSE;
            break;
        }

    } while (FALSE);


    nicPutMailbox(prAdapter, 0, HIF_TC_RX_CHK_STATE);
    nicPutMailbox(prAdapter, 1, 1);
    nicSetSwIntr(prAdapter, BIT(31));


    DBGLOG(EMU, TRACE, (".....end....\n"));
    return status;
}


BOOLEAN
emuSendPacket1 (
    IN P_ADAPTER_T prAdapter,
    IN UINT_32     u4PortId,
    IN BOOLEAN     fgUseIntrEnhanceModeRead
    )
{
    UINT_32 len, i;
    UINT_8 aucTxCount[8] = {0};
    LARGE_INTEGER rCurrentSysTime, rDiffSysTime;
    BOOLEAN status = TRUE;
    ENHANCE_MODE_DATA_STRUCT_T rEnhanceDataStruct;

    DBGLOG(EMU, TRACE, (".....start....\n"));
    DBGLOG(EMU, TRACE, ("u4PortId=%d, fgUseEnhanceModeRead=%d\n", u4PortId, fgUseIntrEnhanceModeRead));

    ASSERT(u4PortId < 2);

    // test case of CheckTxCountBySingleReadWTSR
    nicPutMailbox(prAdapter, 0, HIF_TC_TX_SINGLE_PACKET | HIF_TEST_CASE_START);
    nicPutMailbox(prAdapter, 1, 0);
    nicSetSwIntr(prAdapter, BIT(31));


    NdisGetCurrentSystemTime(&g_rSysTimeRecord);
    g_rTxTotalByteCount.QuadPart = 0;
    g_u4TxTotalPacketCount = 0;


    // compose single TX packet format, and transmit by TQ0/1, leng from 4~4096
#if 1
    for (len = 4; len < 4096; len ++) {
#else
    for (len = 12; len < 100; len ++) {
#endif
        DBGLOG(EMU, TRACE, ("Send packet len: %d\n", len));

        // communicate TX packet length by mailbox
        nicPutMailbox(prAdapter, 1, len);

        *((PUINT_16)g_aucEmuBuf) = (UINT_16)len;
        for (i = 0; i < len - 4; i++) {
            g_aucEmuBuf[i + 4] = (UINT_8)i;
        }

        //dumpMemory8((PUINT_8)g_aucEmuBuf, len);

        HAL_WRITE_TX_PORT(prAdapter,
                       u4PortId,
                       len,
                       g_aucEmuBuf,
                       sizeof(g_aucEmuBuf));

        g_rTxTotalByteCount.QuadPart += (INT_64)len;
        g_u4TxTotalPacketCount += 1;

        if (fgUseIntrEnhanceModeRead) {
            kalMdelay(50);
            HAL_READ_INTR_STATUS(prAdapter, sizeof(ENHANCE_MODE_DATA_STRUCT_T), (PUINT_8)&rEnhanceDataStruct);
//                dumpMemory8((PUINT_8)(PUINT_8)&rEnhanceDataStruct, sizeof(ENHANCE_MODE_DATA_STRUCT_T));

            // TX done
            aucTxCount[0] = rEnhanceDataStruct.rTxInfo.u.ucTQ0Cnt;
            aucTxCount[1] = rEnhanceDataStruct.rTxInfo.u.ucTQ1Cnt;
            aucTxCount[2] = rEnhanceDataStruct.rTxInfo.u.ucTQ2Cnt;
            aucTxCount[3] = rEnhanceDataStruct.rTxInfo.u.ucTQ3Cnt;
            aucTxCount[4] = rEnhanceDataStruct.rTxInfo.u.ucTQ4Cnt;
            aucTxCount[5] = rEnhanceDataStruct.rTxInfo.u.ucTQ5Cnt;
#if 1
            DBGLOG(EMU, TRACE, ("aucTxCount[0]: %d\n", aucTxCount[0]));
            DBGLOG(EMU, TRACE, ("aucTxCount[1]: %d\n", aucTxCount[1]));
            DBGLOG(EMU, TRACE, ("aucTxCount[2]: %d\n", aucTxCount[2]));
            DBGLOG(EMU, TRACE, ("aucTxCount[3]: %d\n", aucTxCount[3]));
            DBGLOG(EMU, TRACE, ("aucTxCount[4]: %d\n", aucTxCount[4]));
            DBGLOG(EMU, TRACE, ("aucTxCount[5]: %d\n", aucTxCount[5]));
#endif
            ASSERT(rEnhanceDataStruct.u4RcvMailbox0 == 0x12345678);
            ASSERT(rEnhanceDataStruct.u4RcvMailbox1 == 0x87654321);
            ASSERT(aucTxCount[0] == 1);
            ASSERT(rEnhanceDataStruct.u4WHISR & BIT(0));
#if 0
            DBGLOG(EMU, WARN, ("!! emuSendPacket1: %d failed (u4RcvMailbox0: 0x%x, u4RcvMailbox1: 0x%x, )\n",
                                len, rEnhanceDataStruct.u4RcvMailbox0, rEnhanceDataStruct.u4RcvMailbox1));
            for (i = 0; i < 6; i++) {
                DBGLOG(EMU, WARN, ("aucTxCount[%d]: %d\n", i, aucTxCount[i]));
            }
#endif
        } else {
            if (emuPollingIntrBit(prAdapter, BIT(0), BIT(0))) {
                // TX done
            } else {
                DBGLOG(EMU, WARN, ("emuSendPacket1: %d failed\n", len));
                status = FALSE;
                break;
            }
            HAL_READ_TX_RELEASED_COUNT(prAdapter, aucTxCount);
        }

        // use TC0 only
        if ((aucTxCount[0] != 1) ||
            (aucTxCount[1] != 0) ||
            (aucTxCount[2] != 0) ||
            (aucTxCount[3] != 0) ||
            (aucTxCount[4] != 0) ||
            (aucTxCount[5] != 0)) {

            DBGLOG(EMU, WARN, ("----  release TX count failed  ----\n"));
            DBGLOG(EMU, WARN, ("aucTxCount[0]: %d\n", aucTxCount[0]));
            DBGLOG(EMU, WARN, ("aucTxCount[1]: %d\n", aucTxCount[1]));
            DBGLOG(EMU, WARN, ("aucTxCount[2]: %d\n", aucTxCount[2]));
            DBGLOG(EMU, WARN, ("aucTxCount[3]: %d\n", aucTxCount[3]));
            DBGLOG(EMU, WARN, ("aucTxCount[4]: %d\n", aucTxCount[4]));
            DBGLOG(EMU, WARN, ("aucTxCount[5]: %d\n", aucTxCount[5]));
            if (fgUseIntrEnhanceModeRead) {
                dumpMemory8((PUINT_8)(PUINT_8)&rEnhanceDataStruct, sizeof(rEnhanceDataStruct));
            }
    //            ASSERT(0);
            status = FALSE;
            break;
        }
    }



    // test case of CheckTxCountBySingleReadWTSR
    nicPutMailbox(prAdapter, 0, HIF_TC_TX_SINGLE_PACKET);
    nicPutMailbox(prAdapter, 1, 1);
    nicSetSwIntr(prAdapter, BIT(31));

    NdisGetCurrentSystemTime(&rCurrentSysTime);
    rDiffSysTime.QuadPart = rCurrentSysTime.QuadPart - g_rSysTimeRecord.QuadPart;

    DBGLOG(EMU, INFO, ("---------Statistics------\n"));
    DBGLOG(EMU, INFO, ("[time diff]: 0x%x us\n", rDiffSysTime.QuadPart / 10));
    DBGLOG(EMU, INFO, ("[total byte count]: %d\n", g_rTxTotalByteCount.QuadPart));
    DBGLOG(EMU, INFO, ("[total packet count]: %d\n", g_u4TxTotalPacketCount));
    DBGLOG(EMU, INFO, ("[throughput]: %d (bps)\n", (g_rTxTotalByteCount.QuadPart * 8 * 10000000) / rDiffSysTime.QuadPart));
    DBGLOG(EMU, INFO, ("-----------------------\n"));


    DBGLOG(EMU, TRACE, (".....end....\n"));
    return status;
}

BOOLEAN
emuSendPacketAggN (
    IN P_ADAPTER_T prAdapter,
    IN UINT_32     u4PortId,
    IN UINT_32     u4AggNum,
    IN UINT_32     u4LenStart,
    IN UINT_32     u4LenEnd,
    IN BOOLEAN     fgUseIntrEnhanceModeRead
    )
{
    UINT_32 len, i, accumNum = 0, offset = 0, j;
    UINT_8 aucTxCount[8], aucAccumTxCnt[8];
    UINT_32 u4TxOkCount = 0;
    BOOLEAN status = TRUE;
    UINT_32 code;
    UINT_32 u4AggLen = 0;
    ENHANCE_MODE_DATA_STRUCT_T rEnhanceDataStruct;
    LARGE_INTEGER rCurrentSysTime, rDiffSysTime;

    DBGLOG(EMU, TRACE, (".....start....\n"));
    DBGLOG(EMU, TRACE, ("u4PortId=%d, fgUseEnhanceModeRead=%d\n", u4PortId, fgUseIntrEnhanceModeRead));

    ASSERT(u4AggNum);
    ASSERT((u4LenEnd - u4LenStart + 1) >= u4AggNum);

    // test case of CheckTxCountBySingleReadWTSR
    nicPutMailbox(prAdapter, 0, HIF_TC_TX_AGG_PACKET | HIF_TEST_CASE_START);
    code = (u4AggNum & BITS(0, 4)) | ((u4LenStart & BITS(0, 11)) << 5) | ((u4LenEnd & BITS(0, 11)) << 17);
    nicPutMailbox(prAdapter, 1, code);
    nicSetSwIntr(prAdapter, BIT(31));

    HAL_READ_INTR_STATUS(prAdapter, sizeof(ENHANCE_MODE_DATA_STRUCT_T), (PUINT_8)&rEnhanceDataStruct);

    NdisGetCurrentSystemTime(&g_rSysTimeRecord);
    g_rTxTotalByteCount.QuadPart = 0;
    g_u4TxTotalPacketCount = 0;


    for (len = u4LenStart; len <= u4LenEnd; len++) {
        *(PUINT_16)(g_aucEmuBuf + offset) = (UINT_16)len;
        *(PUINT_16)(g_aucEmuBuf + offset + 2) = (UINT_16)0;
        for (i = 0; i < len - 4; i++) {
            g_aucEmuBuf[offset + i + 4] = (UINT_8)i;
        }
        offset += len;
        offset += (len % 4) ? 4 - (len % 4) : 0; // DWORD align for each packet
        accumNum += 1;


        g_rTxTotalByteCount.QuadPart += (INT_64)len;
        g_u4TxTotalPacketCount += 1;

        DBGLOG(EMU, TRACE, ("emuSendPacketAggN len: %d (offset: %d, accumNum: %d)\n", len, offset, accumNum));

        if ((accumNum >= u4AggNum) || (len == u4LenEnd)) {

            nicPutMailbox(prAdapter, 1, len);

//            dumpMemory8((PUINT_8)g_aucEmuBuf, offset);
            HAL_WRITE_TX_PORT(prAdapter,
                           u4PortId,
                           offset,
                           g_aucEmuBuf,
                           sizeof(g_aucEmuBuf));

#if 0
            if (emuPollingIntrBit(prAdapter, BIT(0), BIT(0))) {
            } else {
                DBGLOG(EMU, WARN, ("emuSendPacket1: 0x%x failed\n", len));
        //            ASSERT(0);
                status = FALSE;
                break;
            }
#endif

//            kalMdelay(100);

            if (fgUseIntrEnhanceModeRead) {
                kalMemZero(aucTxCount, sizeof(aucTxCount));

                for (i = 0; i < 1000; i++) {
                    HAL_READ_INTR_STATUS(prAdapter, sizeof(ENHANCE_MODE_DATA_STRUCT_T), (PUINT_8)&rEnhanceDataStruct);

                    // TX done
                    aucTxCount[0] += rEnhanceDataStruct.rTxInfo.u.ucTQ0Cnt;
                    aucTxCount[1] += rEnhanceDataStruct.rTxInfo.u.ucTQ1Cnt;
                    aucTxCount[2] += rEnhanceDataStruct.rTxInfo.u.ucTQ2Cnt;
                    aucTxCount[3] += rEnhanceDataStruct.rTxInfo.u.ucTQ3Cnt;
                    aucTxCount[4] += rEnhanceDataStruct.rTxInfo.u.ucTQ4Cnt;
                    aucTxCount[5] += rEnhanceDataStruct.rTxInfo.u.ucTQ5Cnt;

                    if ((rEnhanceDataStruct.u4WHISR & BIT(30))) {
                        DBGLOG(EMU, TRACE, (" --- break --- \n"));
//                        dumpMemory8((PUINT_8)(PUINT_8)&rEnhanceDataStruct, sizeof(ENHANCE_MODE_DATA_STRUCT_T));

                        ASSERT(rEnhanceDataStruct.u4RcvMailbox0 = 0x5a5a5a5a);
                        ASSERT(rEnhanceDataStruct.u4RcvMailbox1 = 0x33667788);
                        break;
                    }

                    kalMdelay(10);

                }
                DBGLOG(EMU, TRACE, (" ### \n"));
//                dumpMemory8((PUINT_8)(PUINT_8)&rEnhanceDataStruct, sizeof(ENHANCE_MODE_DATA_STRUCT_T));
            } else {
                // check SW interrupt from FW
                if (emuPollingIntrBit(prAdapter, BIT(30), BIT(30))) {
                } else {
                    DBGLOG(EMU, ERROR, ("emuPollingIntrBit polling b30 failed\n"));
            //        ASSERT(0);
                    return FALSE;
                }

                HAL_READ_TX_RELEASED_COUNT(prAdapter, aucTxCount);
            }
//            DBGLOG(EMU, TRACE, ("aucTxCount[0]: %d\n", aucTxCount[0]));
//            DBGLOG(EMU, TRACE, ("aucTxCount[1]: %d\n", aucTxCount[1]));
//            DBGLOG(EMU, TRACE, ("aucTxCount[2]: %d\n", aucTxCount[2]));
//            DBGLOG(EMU, TRACE, ("aucTxCount[3]: %d\n", aucTxCount[3]));
//            DBGLOG(EMU, TRACE, ("aucTxCount[4]: %d\n", aucTxCount[4]));
//            DBGLOG(EMU, TRACE, ("aucTxCount[5]: %d\n", aucTxCount[5]));

            if (aucTxCount[0] != accumNum) {
                DBGLOG(EMU, TRACE, ("aucTxCount[0]: %d\n", aucTxCount[0]));
                ASSERT(0);
            }
#if 0
            ASSERT(aucTxCount[0] == accumNum);
            ASSERT(aucTxCount[1] == 0);
            ASSERT(aucTxCount[2] == 0);
            ASSERT(aucTxCount[3] == 0);
            ASSERT(aucTxCount[4] == 0);
            ASSERT(aucTxCount[5] == 0);
#endif

            accumNum = 0;
            offset = 0;
            u4AggLen = 0;
        }
    }

    NdisGetCurrentSystemTime(&rCurrentSysTime);
    rDiffSysTime.QuadPart = rCurrentSysTime.QuadPart - g_rSysTimeRecord.QuadPart;

    DBGLOG(EMU, INFO, ("---------Statistics------\n"));
    DBGLOG(EMU, INFO, ("[time diff]: 0x%x us\n", rDiffSysTime.QuadPart / 10));
    DBGLOG(EMU, INFO, ("[total byte count]: %d\n", g_rTxTotalByteCount.QuadPart));
    DBGLOG(EMU, INFO, ("[total packet count]: %d\n", g_u4TxTotalPacketCount));
    DBGLOG(EMU, INFO, ("[throughput]: %d (bps)\n", (g_rTxTotalByteCount.QuadPart * 8 * 10000000) / rDiffSysTime.QuadPart));
    DBGLOG(EMU, INFO, ("-----------------------\n"));

    // test case of CheckTxCountBySingleReadWTSR
    nicPutMailbox(prAdapter, 0, HIF_TC_TX_AGG_PACKET);
    nicPutMailbox(prAdapter, 1, 1);
    nicSetSwIntr(prAdapter, BIT(31));

    DBGLOG(EMU, TRACE, (".....end....\n"));
    return status;
}

BOOLEAN
emuSendPacketAggNSwHdrFormat (
    IN P_ADAPTER_T prAdapter,
    IN UINT_32     u4PortId,
    IN UINT_32     u4LenStart
    )
{
    UINT_32 i, qId, accumNum = 0, offset = 0, j;
    BOOLEAN status = TRUE;
    UINT_32 u4AggLen = 0;
    ENHANCE_MODE_DATA_STRUCT_T rEnhanceDataStruct;
    P_NIC_HIF_TX_HEADER_T prHifTxHeader;
    P_TX_CTRL_T prTxCtrl;
    UINT_32 u4NumPerUsrprio;

    prTxCtrl = &prAdapter->rTxCtrl;

    ASSERT(u4LenStart > sizeof(NIC_HIF_TX_HEADER_T));

    for (qId = 0; qId < 6; qId++) {
        u4NumPerUsrprio= prTxCtrl->rTc.aucFreeBufferCount[qId];
        DBGLOG(EMU, TRACE, ("qId: %d (u4NumPerUsrprio: %d)\n", qId , u4NumPerUsrprio));

        for (i = 0; i < u4NumPerUsrprio; i++) {
            prHifTxHeader = (P_NIC_HIF_TX_HEADER_T)(g_aucEmuBuf + offset);
            kalMemZero(prHifTxHeader, sizeof(prHifTxHeader));
            prHifTxHeader->u2TxByteCount_UserPriority = (UINT_16)((u4LenStart & HIF_TX_HDR_TX_BYTE_COUNT_MASK) | (qId << HIF_TX_HDR_USER_PRIORITY_OFFSET));
            prHifTxHeader->ucResource_PktType_CSflags = qId;

            prHifTxHeader->ucEtherTypeOffset = 0;
            prHifTxHeader->ucWlanHeaderLength = 0;
            prHifTxHeader->ucPktFormtId_Flags = 0;

#if 1
            for (j = 0; j < u4LenStart - sizeof(NIC_HIF_TX_HEADER_T); j++) {
                g_aucEmuBuf[offset + j + sizeof(NIC_HIF_TX_HEADER_T)] = (UINT_8)j;
            }
#else
            for (j = 0; j < u4LenStart - sizeof(NIC_HIF_TX_HEADER_T); j++) {
                g_aucEmuBuf[offset + j + sizeof(NIC_HIF_TX_HEADER_T)] = 0;
            }
#endif
            offset += ALIGN_4(u4LenStart);// DWORD align for each packet
            accumNum += 1;
            prTxCtrl->rTc.aucFreeBufferCount[qId]--;

            DBGLOG(EMU, TRACE, ("emuSendPacketAggN (offset: %d, accumNum: %d) (u2TxByteCount_UserPriority: 0x%x)\n",
                offset, accumNum, prHifTxHeader->u2TxByteCount_UserPriority));
        }
    }

    *((PUINT_32)(g_aucEmuBuf + offset)) = 0;

    if (accumNum) {
//        dumpMemory8(g_aucEmuBuf, offset);
        HAL_WRITE_TX_PORT(prAdapter,
                       u4PortId,
                       offset,
                       g_aucEmuBuf,
                       sizeof(g_aucEmuBuf));
    }

    return status;
}

#if 0
BOOLEAN
emuStopTxQueue (
    IN P_ADAPTER_T prAdapter
    )
{
    UINT_32 u4WHIERValue;
    UINT_32 i;
    UINT_8 aucTxCount[8];
    BOOLEAN status = TRUE;

    DBGLOG(EMU, TRACE, (".....start....\n"));

    // disable SW interrupt
    HAL_MCR_RD(prAdapter, MCR_WHIER, &u4WHIERValue);
    HAL_MCR_WR(prAdapter, MCR_WHIER, u4WHIERValue & ~BITS(8,31));


    nicPutMailbox(prAdapter, 0, HIF_TC_TX_CLEAR_TC_COUNT | HIF_TEST_CASE_START);
    nicPutMailbox(prAdapter, 1, 0);
    nicSetSwIntr(prAdapter, BIT(31));

    // check SW interrupt from FW
    if (emuPollingIntrBit(prAdapter, BIT(31), BIT(31))) {
    } else {
        DBGLOG(EMU, ERROR, ("emuPollingIntrBit polling b31 failed\n"));
//        ASSERT(0);
        return;
    }

    nicPutMailbox(prAdapter, 0, HIF_TC_TX_CLEAR_TC_COUNT);
    nicPutMailbox(prAdapter, 1, 1);
    nicSetSwIntr(prAdapter, BIT(31));

    // recover SW interrupt
    HAL_MCR_WR(prAdapter, MCR_WHIER, u4WHIERValue);

    DBGLOG(EMU, TRACE, (".....end....\n"));
    return status;
}
#endif

BOOLEAN
emuRxPacketBurstInSwHdrFormat (
    IN P_ADAPTER_T prAdapter,
    IN BOOLEAN      fgEnable
    )
{
    BOOLEAN status = TRUE;


    if (fgEnable) {
        DBGLOG(EMU, TRACE, (".....RX burst start....\n"));

        nicPutMailbox(prAdapter, 0, HIF_TC_RX_BURST | HIF_TEST_CASE_START);
        nicPutMailbox(prAdapter, 1, 0);
        nicSetSwIntr(prAdapter, BIT(31));

        fgEmuRxBurst = TRUE;

    } else {

        fgEmuRxBurst = FALSE;

        nicPutMailbox(prAdapter, 0, HIF_TC_RX_BURST);
        nicPutMailbox(prAdapter, 1, 1);
        nicSetSwIntr(prAdapter, BIT(31));

        DBGLOG(EMU, TRACE, (".....RX burst end....\n"));
    }

    return status;
}

BOOLEAN
emuTxPacketBurstInSwHdrFormat (
    IN P_ADAPTER_T prAdapter,
    IN BOOLEAN      fgEnable
    )
{
    BOOLEAN status = TRUE;


    if (fgEnable) {
        DBGLOG(EMU, TRACE, (".....TX burst start....\n"));

        nicPutMailbox(prAdapter, 0, HIF_TC_TX_BURST | HIF_TEST_CASE_START);
        nicPutMailbox(prAdapter, 1, 0);
        nicSetSwIntr(prAdapter, BIT(31));

        fgEmuTxBurst = TRUE;

        emuSendPacketAggNSwHdrFormat(prAdapter, 0, 128);

    } else {

        fgEmuTxBurst = FALSE;

        nicPutMailbox(prAdapter, 0, HIF_TC_TX_BURST);
        nicPutMailbox(prAdapter, 1, 1);
        nicSetSwIntr(prAdapter, BIT(31));

        DBGLOG(EMU, TRACE, (".....TX burst end....\n"));
    }

    return status;
}

BOOLEAN
emuRxPacketSwHdrFormat (
    IN P_ADAPTER_T prAdapter,
    IN UINT_32      u4PortId,
    IN UINT_32      u4RxLen,
    IN UINT_32      u4Num
    )
{
    UINT_32 i, code;
    BOOLEAN status = TRUE;

    DBGLOG(EMU, TRACE, (".....start....\n"));


    nicPutMailbox(prAdapter, 0, HIF_TC_RX_SW_PKT_FORMAT | HIF_TEST_CASE_START);
    nicPutMailbox(prAdapter, 1, 0);
    nicSetSwIntr(prAdapter, BIT(31));

    kalMdelay(50);

    // only 1 RX packet to be received
    for (i = u4RxLen; i < u4RxLen + u4Num; i++) {

        code = u4RxLen | (u4Num << 16);
        nicPutMailbox(prAdapter, 0, u4PortId);
        nicPutMailbox(prAdapter, 1, code);
        nicSetSwIntr(prAdapter, BIT(30));

        kalMdelay(50);
    }

    nicPutMailbox(prAdapter, 0, HIF_TC_RX_SW_PKT_FORMAT);
    nicPutMailbox(prAdapter, 1, 1);
    nicSetSwIntr(prAdapter, BIT(31));

    DBGLOG(EMU, TRACE, (".....end....\n"));
    return status;
}

BOOLEAN
emuRxPacket1 (
    IN P_ADAPTER_T prAdapter,
    IN UINT_32      u4PortId,
    IN UINT_32      u4RxLen,
    IN BOOLEAN fgEnIntrEnhanceMode,
    IN BOOLEAN fgEnRxEnhanceMode,
    IN BOOLEAN fgMBoxReadClearByRxEnhance
    )
{
    UINT_32 len, tq, i;
    UINT_8 aucTxCount[8];
    ENHANCE_MODE_DATA_STRUCT_T rEnhanceDataStruct;
    UINT_16 au2RxLen[2];
    BOOLEAN status = TRUE;
    ENHANCE_MODE_DATA_STRUCT_T rEmptyEnhanceStruct = {0};
    UINT_32 u4Mbox0, u4Mbox1;
    P_ENHANCE_MODE_DATA_STRUCT_T prEnhanceDataStruct;
    UINT_32 code;

    ASSERT(u4RxLen < 4096);

    DBGLOG(EMU, TRACE, (".....start....\n"));
    DBGLOG(EMU, TRACE, ("u4PortId=%d, u4RxLen=%d, intrEnh=%d, rxEnh=%d, mboxEnh=%d\n",
            u4PortId, u4RxLen, fgEnIntrEnhanceMode, fgEnRxEnhanceMode, fgMBoxReadClearByRxEnhance));

    // clear enhancemode data structure before start
    HAL_READ_INTR_STATUS(prAdapter, sizeof(ENHANCE_MODE_DATA_STRUCT_T), (PUINT_8)&rEnhanceDataStruct);
    HAL_READ_TX_RELEASED_COUNT(prAdapter, aucTxCount);


    // Enable mailbox read clear
    HAL_SET_MAILBOX_READ_CLEAR(prAdapter, FALSE);


    // test case of CheckTxCountBySingleReadWTSR
    nicPutMailbox(prAdapter, 0, HIF_TC_RX_SINGLE_PACKET | HIF_TEST_CASE_START);
    code = u4RxLen | (u4PortId << 16);
    nicPutMailbox(prAdapter, 1, code);
    nicSetSwIntr(prAdapter, BIT(31));

    // only 1 RX packet to be received
    HAL_CFG_MAX_HIF_RX_LEN_NUM(prAdapter, 1);

    if (fgEnRxEnhanceMode) {
        HAL_ENABLE_RX_ENHANCE_MODE(prAdapter);
    } else {
        HAL_DISABLE_RX_ENHANCE_MODE(prAdapter);
    }

    // delay and ensure FW processed to insert "2 (to verify if RX limit only 1 packet)" RX packet to HIF
//    kalMdelay(100);
#if 1
    // check SW interrupt from FW
    if (emuPollingIntrBit(prAdapter, BIT(30), BIT(30))) {
    } else {
        DBGLOG(EMU, ERROR, ("emuPollingIntrBit polling b30 failed\n"));
//        ASSERT(0);
        return FALSE;
    }
#endif

    if (fgEnIntrEnhanceMode) {
        HAL_READ_INTR_STATUS(prAdapter, sizeof(ENHANCE_MODE_DATA_STRUCT_T), (PUINT_8)&rEnhanceDataStruct);
#if 0
        DBGLOG(EMU, TRACE, ("INTR rEnhanceDataStruct content:\n"));
        dumpMemory8((PUINT_8)(PUINT_8)&rEnhanceDataStruct, sizeof(rEnhanceDataStruct));
        DBGLOG(EMU, TRACE, ("\n"));
        DBGLOG(EMU, TRACE, ("u4WHISR: 0x%x\n", rEnhanceDataStruct.u4WHISR));
        DBGLOG(EMU, TRACE, ("u2NumValidRx0Len: 0x%x\n", rEnhanceDataStruct.rRxInfo.u.u2NumValidRx0Len));
        DBGLOG(EMU, TRACE, ("u2NumValidRx1Len: 0x%x\n", rEnhanceDataStruct.rRxInfo.u.u2NumValidRx1Len));
        DBGLOG(EMU, TRACE, ("u4RcvMailbox0: 0x%x\n", rEnhanceDataStruct.u4RcvMailbox0));
        DBGLOG(EMU, TRACE, ("u4RcvMailbox1: 0x%x\n", rEnhanceDataStruct.u4RcvMailbox1));
        DBGLOG(EMU, TRACE, ("au4WTSR[0]: 0x%x\n", rEnhanceDataStruct.rTxInfo.au4WTSR[0]));
        DBGLOG(EMU, TRACE, ("au4WTSR[1]: 0x%x\n", rEnhanceDataStruct.rTxInfo.au4WTSR[1]));
#endif
        ASSERT((rEnhanceDataStruct.u4WHISR & BIT(u4PortId + 1)));
        ASSERT((rEnhanceDataStruct.rRxInfo.u.u2NumValidRx0Len == ((u4PortId == 0) ? 1 : 0)));
        ASSERT((rEnhanceDataStruct.rRxInfo.u.u2NumValidRx1Len == ((u4PortId == 1) ? 1 : 0)));
        if ((rEnhanceDataStruct.rTxInfo.au4WTSR[0] != 1) ||
            (rEnhanceDataStruct.rTxInfo.au4WTSR[1] != 1)) {
            DBGLOG(EMU, TRACE, ("au4WTSR[0]=0x%x, au4WTSR[1]=0x%x\n",
                rEnhanceDataStruct.rTxInfo.au4WTSR[0], rEnhanceDataStruct.rTxInfo.au4WTSR[1]));
            HAL_READ_TX_RELEASED_COUNT(prAdapter, aucTxCount);
            for (i = 0; i < 6; i++) {
                DBGLOG(EMU, TRACE, ("aucTxCount[%d]=%d\n", i, aucTxCount[i]));
            }
            return FALSE;
        }
        ASSERT((rEnhanceDataStruct.u4RcvMailbox0 == 0x1234));
        ASSERT((rEnhanceDataStruct.u4RcvMailbox1 == 0x5678));

        au2RxLen[0] = rEnhanceDataStruct.rRxInfo.u.au2Rx0Len[0];
        for (i = 1; i < 16; i++) {
            ASSERT(rEnhanceDataStruct.rRxInfo.u.au2Rx0Len[i] == 0);
        }
        au2RxLen[1] = rEnhanceDataStruct.rRxInfo.u.au2Rx1Len[0];
        for (i = 1; i < 16; i++) {
            ASSERT(rEnhanceDataStruct.rRxInfo.u.au2Rx1Len[i] == 0);
        }

        ASSERT(rEnhanceDataStruct.rRxInfo.u.u2NumValidRx0Len == ((u4PortId == 0) ? 1 : 0));
        ASSERT(rEnhanceDataStruct.rRxInfo.u.u2NumValidRx1Len == ((u4PortId == 1) ? 1 : 0));

    } else {
        if (emuPollingIntrBit(prAdapter, BIT(u4PortId + 1), BIT(u4PortId + 1))) {
            HAL_READ_RX_LENGTH(prAdapter, &au2RxLen[0], &au2RxLen[1]);

            if (u4PortId == 0) {
                ASSERT(au2RxLen[0]);
                ASSERT(au2RxLen[1] == 0);
            } else {
                ASSERT(au2RxLen[1] == 0);
                ASSERT(au2RxLen[0]);
            }

            HAL_READ_TX_RELEASED_COUNT(prAdapter, aucTxCount);
            if ((aucTxCount[0] != 1) || (aucTxCount[4] != 1)) {
                DBGLOG(EMU, WARN, ("aucTxCount[0]: 0x%x, aucTxCount[1]: 0x%x\n",
                                    aucTxCount[0], aucTxCount[1]));
                return FALSE;
            }

        } else {
            DBGLOG(EMU, WARN, ("emuRxPacket1: failed\n"));
//            ASSERT(0);
            return FALSE;
        }
    }

    if (au2RxLen[u4PortId] != u4RxLen) {
        DBGLOG(EMU, TRACE, ("RX length not match [expect: %d], [read: %d]\n",
                            u4RxLen, au2RxLen[u4PortId]));
        ASSERT(0);
        return FALSE;
    }

    if (fgEnRxEnhanceMode) {
        nicSetSwIntr(prAdapter, BIT(30));
        kalMdelay(80);

        if (fgMBoxReadClearByRxEnhance) {
            HAL_SET_MAILBOX_READ_CLEAR(prAdapter, TRUE);
        }
    }

    // read RX data content
    HAL_READ_RX_PORT(prAdapter,
                     u4PortId,
                     ALIGN_4(au2RxLen[u4PortId] + 4) + sizeof(ENHANCE_MODE_DATA_STRUCT_T) + 4/* extra HW padding*/, // always get max size!
                     g_aucEmuBuf,
                     sizeof(g_aucEmuBuf));


    prEnhanceDataStruct = (P_ENHANCE_MODE_DATA_STRUCT_T) ALIGN_4((UINT_32)(g_aucEmuBuf + au2RxLen[u4PortId] + 4 + 4/*HW padding for TCP/IP CS*/));

//    DBGLOG(EMU, TRACE, ("RX content... (len: %d)\n", au2RxLen[u4PortId]));
//    dumpMemory8((PUINT_8)g_aucEmuBuf, ALIGN_4(au2RxLen[u4PortId]) + 4 + sizeof(ENHANCE_MODE_DATA_STRUCT_T) + 4);

    for (i = 4; i < au2RxLen[u4PortId]; i++) {
        if (g_aucEmuBuf[i] != (UINT_8)((i - 4) & BITS(0, 7))) {
            DBGLOG(EMU, WARN, ("RX data not match [offset: %d] [expect: 0x%x], [read: 0x%x]\n",
                                i, g_aucEmuBuf[i], (UINT_8)((i - 4) & BITS(0, 7))));
            //            ASSERT(0);
            return FALSE;
        }
    }

//    HAL_READ_TX_RELEASED_COUNT(prAdapter, aucTxCount);
//    ASSERT(aucTxCount[0] == 0);
//    ASSERT(aucTxCount[1] == 0);

    if (fgEnRxEnhanceMode) {
        // check VALID enhance mode structure
        DBGLOG(EMU, TRACE, ("--- RX rEnhanceDataStruct content:\n"));
        dumpMemory8((PUINT_8)prEnhanceDataStruct, sizeof(rEnhanceDataStruct));
        DBGLOG(EMU, TRACE, ("\n"));

        ASSERT((prEnhanceDataStruct->u4WHISR & BIT(0)));
        ASSERT((prEnhanceDataStruct->rRxInfo.u.u2NumValidRx0Len == ((u4PortId == 0) ? 1 : 0)));
        ASSERT((prEnhanceDataStruct->rRxInfo.u.u2NumValidRx1Len == ((u4PortId == 1) ? 1 : 0)));
        if ((prEnhanceDataStruct->rTxInfo.au4WTSR[0] != 0x0100) ||
            (prEnhanceDataStruct->rTxInfo.au4WTSR[1] != 0x0100)) {
            DBGLOG(EMU, TRACE, ("au4WTSR[0]=0x%x, au4WTSR[1]=0x%x\n",
                prEnhanceDataStruct->rTxInfo.au4WTSR[0], prEnhanceDataStruct->rTxInfo.au4WTSR[1]));
            HAL_READ_TX_RELEASED_COUNT(prAdapter, aucTxCount);
            for (i = 0; i < 6; i++) {
                DBGLOG(EMU, TRACE, ("[1]aucTxCount[%d]=%d\n", i, aucTxCount[i]));
            }
            return FALSE;
        }
        ASSERT((prEnhanceDataStruct->u4RcvMailbox0 == 0x3333));
        ASSERT((prEnhanceDataStruct->u4RcvMailbox1 == 0x5555));

        // read again for mailbox
        nicGetMailbox(prAdapter, 0, &u4Mbox0);
        nicGetMailbox(prAdapter, 1, &u4Mbox1);

        if (fgMBoxReadClearByRxEnhance) {
            ASSERT((u4Mbox0 == 0));
            ASSERT((u4Mbox1 == 0));
        } else {
            ASSERT((u4Mbox0 == prEnhanceDataStruct->u4RcvMailbox0));
            ASSERT((u4Mbox1 == prEnhanceDataStruct->u4RcvMailbox1));
        }

    } else {
        // check EMPTY enhance mode structure
        if (kalMemCmp(prEnhanceDataStruct,
                      &rEmptyEnhanceStruct,
                      sizeof(rEmptyEnhanceStruct))) {
            //            ASSERT(0);
            return FALSE;
        }
    }

    // check RX data content
#if 0
    for (i = 0; i < au2RxLen[u4PortId]; i+= 16) {
        DBGLOG(EMU, TRACE, ("[%d] %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                        i,
                        g_aucEmuBuf[i], g_aucEmuBuf[i + 1], g_aucEmuBuf[i + 2], g_aucEmuBuf[i + 3],
                        g_aucEmuBuf[i + 4], g_aucEmuBuf[i + 5], g_aucEmuBuf[i + 6], g_aucEmuBuf[i + 7],
                        g_aucEmuBuf[i + 8], g_aucEmuBuf[i + 9], g_aucEmuBuf[i + 10], g_aucEmuBuf[i + 11],
                        g_aucEmuBuf[i + 12], g_aucEmuBuf[i + 13], g_aucEmuBuf[i + 14], g_aucEmuBuf[i + 15]
                        ));
    }
#endif

    // test case of CheckTxCountBySingleReadWTSR
    nicPutMailbox(prAdapter, 0, HIF_TC_RX_SINGLE_PACKET);
    nicPutMailbox(prAdapter, 1, 1);
    nicSetSwIntr(prAdapter, BIT(31));

    // clear enhancemode data structure before leave
    HAL_READ_INTR_STATUS(prAdapter, sizeof(ENHANCE_MODE_DATA_STRUCT_T), (PUINT_8)&rEnhanceDataStruct);
    HAL_READ_TX_RELEASED_COUNT(prAdapter, aucTxCount);
    for (i = 0; i < 6; i++) {
//        DBGLOG(EMU, TRACE, ("aucTxCount[%d]=%d\n", i, aucTxCount[i]));
    }

    DBGLOG(EMU, TRACE, (".....end....\n"));
    return status;
}

BOOLEAN
emuRxPacketAggN (
    IN P_ADAPTER_T prAdapter,
    IN UINT_32      u4PortId,
    IN BOOLEAN fgEnRxEnhanceMode,
    IN UINT_32 u4RxLen,
    IN UINT_32 u4AggNum,
    IN UINT_32 u4MaxReadAggNum//0: unlimited
    )
{
    UINT_32 len, tq, i, j;
    UINT_8 aucTxCount[8];
    ENHANCE_MODE_DATA_STRUCT_T rEnhanceDataStruct;
    UINT_16 au2RxLen[2][16];
    BOOLEAN status = TRUE;
    ENHANCE_MODE_DATA_STRUCT_T rEmptyEnhanceStruct = {0};
    UINT_32 u4ReadPktNum;
    UINT_32 u4TotalByte = 0;
    UINT_32 u4AccumPktRead = 0, u4Value;
    PUINT_8 pucBuf;
    UINT_32 u4Offset = 0, code;
    UINT_32 u4ExistRxLen = u4RxLen;

    DBGLOG(EMU, TRACE, (".....start....\n"));
    DBGLOG(EMU, TRACE, ("u4PortId=%d, u4RxLen=%d, u4AggNum=%d, rxEnh=%d, u4MaxReadAggNum=%d\n",
            u4PortId, u4RxLen, u4AggNum, fgEnRxEnhanceMode, u4MaxReadAggNum));

    ASSERT(u4RxLen < 4096);
    // prevent buffer overflow
    if ((u4RxLen + u4AggNum) > 4096) {
        u4AggNum = 4096 - u4RxLen;
    }

    HAL_SET_MAILBOX_READ_CLEAR(prAdapter, FALSE);

    // test case of CheckTxCountBySingleReadWTSR
    nicPutMailbox(prAdapter, 0, HIF_TC_RX_AGG_PACKET | HIF_TEST_CASE_START);
    nicPutMailbox(prAdapter, 1, 0);
    nicSetSwIntr(prAdapter, BIT(31));

    ASSERT(u4MaxReadAggNum <= 16);
    kalMdelay(50);

    code = u4AggNum;
    nicPutMailbox(prAdapter, 0, code);
    code = u4RxLen | (u4PortId << 16);
    nicPutMailbox(prAdapter, 1, code);
    nicSetSwIntr(prAdapter, BIT(30));

    // only 1 RX packet to be received
    HAL_CFG_MAX_HIF_RX_LEN_NUM(prAdapter, u4MaxReadAggNum);

#if 0
    HAL_MCR_RD(prAdapter, MCR_WHCR, &u4Value);
    DBGLOG(EMU, TRACE, ("MCR_WHCR:0x%x\n", u4Value));
    HAL_MCR_RD(prAdapter, MCR_WRPLR, &u4Value);
    DBGLOG(EMU, TRACE, ("MCR_WRPLR:0x%x\n", u4Value));
#endif

    if (u4MaxReadAggNum == 0) {
        u4MaxReadAggNum = 16;
    }

    if (fgEnRxEnhanceMode) {
        HAL_ENABLE_RX_ENHANCE_MODE(prAdapter);
    } else {
        HAL_DISABLE_RX_ENHANCE_MODE(prAdapter);
    }

    // delay and ensure FW processed to insert "multiple" RX packet to HIF
//    kalMdelay(100);
    // check SW interrupt from FW
    if (emuPollingIntrBit(prAdapter, BIT(30), BIT(30))) {
    } else {
        DBGLOG(EMU, ERROR, ("emuPollingIntrBit polling b30 failed\n"));
//        ASSERT(0);
        return FALSE;
    }

    while (u4AccumPktRead < u4AggNum) {
        u4TotalByte = 0;

        if (1/*fgEnIntrEnhanceMode*/) {
            HAL_READ_INTR_STATUS(prAdapter, sizeof(ENHANCE_MODE_DATA_STRUCT_T), (PUINT_8)&rEnhanceDataStruct);

#if 0
            DBGLOG(EMU, TRACE, ("rEnhanceDataStruct content:\n"));
            dumpMemory8((PUINT_8)(PUINT_8)&rEnhanceDataStruct, sizeof(rEnhanceDataStruct));
            DBGLOG(EMU, TRACE, ("\n"));
            DBGLOG(EMU, TRACE, ("u4WHISR: 0x%x\n", rEnhanceDataStruct.u4WHISR));
            DBGLOG(EMU, TRACE, ("u2NumValidRx0Len: 0x%x\n", rEnhanceDataStruct.rRxInfo.u.u2NumValidRx0Len));
            DBGLOG(EMU, TRACE, ("u2NumValidRx1Len: 0x%x\n", rEnhanceDataStruct.rRxInfo.u.u2NumValidRx1Len));
            DBGLOG(EMU, TRACE, ("u4RcvMailbox0: 0x%x\n", rEnhanceDataStruct.u4RcvMailbox0));
            DBGLOG(EMU, TRACE, ("u4RcvMailbox1: 0x%x\n", rEnhanceDataStruct.u4RcvMailbox1));
            DBGLOG(EMU, TRACE, ("au4WTSR[0]: 0x%x\n", rEnhanceDataStruct.rTxInfo.au4WTSR[0]));
            DBGLOG(EMU, TRACE, ("au4WTSR[0]: 0x%x\n", rEnhanceDataStruct.rTxInfo.au4WTSR[1]));
#endif

            if (!(rEnhanceDataStruct.u4WHISR & BIT(u4PortId + 1))) {
                ASSERT(0);
                DBGLOG(EMU, TRACE, ("rEnhanceDataStruct content:\n"));
                dumpMemory8((PUINT_8)(PUINT_8)&rEnhanceDataStruct, sizeof(rEnhanceDataStruct));
                return FALSE;
            }
            if (u4PortId == 0) {
                if (u4MaxReadAggNum) {
                    if (rEnhanceDataStruct.rRxInfo.u.u2NumValidRx0Len > u4MaxReadAggNum) {
                        ASSERT(0);
                        DBGLOG(EMU, TRACE, ("rEnhanceDataStruct content:\n"));
                        dumpMemory8((PUINT_8)(PUINT_8)&rEnhanceDataStruct, sizeof(rEnhanceDataStruct));
                        return FALSE;
                    }
                }
                ASSERT((rEnhanceDataStruct.rRxInfo.u.u2NumValidRx1Len == 0));
            } else {
                ASSERT((rEnhanceDataStruct.rRxInfo.u.u2NumValidRx0Len == 0));
                if (u4MaxReadAggNum) {
                    if (rEnhanceDataStruct.rRxInfo.u.u2NumValidRx1Len > u4MaxReadAggNum) {
                        ASSERT(0);
                        DBGLOG(EMU, TRACE, ("rEnhanceDataStruct content:\n"));
                        dumpMemory8((PUINT_8)(PUINT_8)&rEnhanceDataStruct, sizeof(rEnhanceDataStruct));
                        return FALSE;
                    }
                }
            }
            ASSERT((rEnhanceDataStruct.rTxInfo.au4WTSR[0] == 0));
            ASSERT((rEnhanceDataStruct.rTxInfo.au4WTSR[1] == 0));
            ASSERT((rEnhanceDataStruct.u4RcvMailbox0 == 0x51515151));
            ASSERT((rEnhanceDataStruct.u4RcvMailbox1 == 0x12345678));

            for (i = 0; i < 16; i++) {
                au2RxLen[0][i] = rEnhanceDataStruct.rRxInfo.u.au2Rx0Len[i];
                au2RxLen[1][i] = rEnhanceDataStruct.rRxInfo.u.au2Rx1Len[i];
            }

            if (u4PortId == 0) {
                u4ReadPktNum = rEnhanceDataStruct.rRxInfo.u.u2NumValidRx0Len;
            } else {
                u4ReadPktNum = rEnhanceDataStruct.rRxInfo.u.u2NumValidRx1Len;
            }

            for(i = 0; i < u4ReadPktNum; i++) {
                ASSERT(au2RxLen[u4PortId][i] == u4ExistRxLen + i); // rx len is increased by 1 for each of the RX packet
            }
            for(i = u4ReadPktNum; i < 16; i++) {
                ASSERT(au2RxLen[u4PortId][i] == 0);
            }
            // non-tesed RX length should be all 0
            for(i = 0; i < 16; i++) {
                if (au2RxLen[(u4PortId == 0) ? 1 : 0][i] != 0) {
                    status = FALSE;
                    DBGLOG(EMU, ERROR, ("au2RxLen[%d][%d]: 0x%x\n",
                        (u4PortId == 0) ? 1 : 0,
                        i,
                        au2RxLen[(u4PortId == 0) ? 1 : 0][i]));
                    return FALSE;
                }
            }


            /* --------------- */
            if (u4PortId == 0) {
                u4ExistRxLen += rEnhanceDataStruct.rRxInfo.u.u2NumValidRx0Len;
                u4AccumPktRead += rEnhanceDataStruct.rRxInfo.u.u2NumValidRx0Len;
            } else {
                u4ExistRxLen += rEnhanceDataStruct.rRxInfo.u.u2NumValidRx1Len;
                u4AccumPktRead += rEnhanceDataStruct.rRxInfo.u.u2NumValidRx1Len;
            }
        }

        for(i = 0; i < u4ReadPktNum; i++) {
            u4TotalByte += ALIGN_4((au2RxLen[u4PortId][i] + 4/*HW padding for TCP/IP CS*/));
        }

//        if (fgEnRxEnhanceMode) {
            u4TotalByte += sizeof(ENHANCE_MODE_DATA_STRUCT_T) + 4 /* dummy padding DWORD */;
//        }
        // read RX data content
        HAL_READ_RX_PORT(prAdapter,
                         u4PortId,
                         u4TotalByte,
                         g_aucEmuBuf,
                         sizeof(g_aucEmuBuf));
#if 0
        DBGLOG(EMU, TRACE, ("RX data (AGG) content:\n"));
        dumpMemory8((PUINT_8)g_aucEmuBuf, u4TotalByte);
#endif
        u4Offset= 0;

        /* chk number of packet */
        for(j = 0; j < u4ReadPktNum; j++) {
            pucBuf = (PUINT_8)((UINT_32)g_aucEmuBuf + u4Offset);

            // chk each content
            for (i = 4; i < au2RxLen[u4PortId][j]; i++) {
                if (pucBuf[i] != (UINT_8)((i - 4) & BITS(0, 7))) {
                    DBGLOG(EMU, TRACE, ("RX data (AGG) content:\n"));
                    dumpMemory8((PUINT_8)g_aucEmuBuf, u4TotalByte);

                    DBGLOG(EMU, WARN, ("[pkt:%d, len: 0x%x] RX data not match [offset: %d] [read: 0x%x], [expect: 0x%x]\n",
                                        j, *(PUINT_16)pucBuf, i, pucBuf[i], (UINT_8)((i - 4) & BITS(0, 7))));
                    //            ASSERT(0);
                    return FALSE;
                }
            }
            // don't care for the HW padding TCP/IP chksum
            // ...

            u4Offset += (UINT_32)(ALIGN_4(au2RxLen[u4PortId][j])) + 4;
        }
#if 1
        if (fgEnRxEnhanceMode) {
            P_ENHANCE_MODE_DATA_STRUCT_T prEnhanceModeStruct;

            // check VALID enhance mode structure
            ASSERT(*(PUINT_32)((UINT_32)g_aucEmuBuf + u4Offset) == 0);/* dummy padding DWORD */
            prEnhanceModeStruct = (P_ENHANCE_MODE_DATA_STRUCT_T) ((UINT_32)g_aucEmuBuf + u4Offset + 4);
            ASSERT(prEnhanceModeStruct->u4RcvMailbox0 == 0x51515151);
            ASSERT(prEnhanceModeStruct->u4RcvMailbox1 == 0x12345678);
        } else {
            // check EMPTY enhance mode structure
            for (i = 0; i < sizeof(rEmptyEnhanceStruct) + 4; i++) {
                if (g_aucEmuBuf[u4Offset + i] != 0) {
                    //            ASSERT(0);
                    return FALSE;
                }
            }
        }
#endif
        // check RX data content
#if 0
        for (j = 0; j < 16; j++) {
            for (i = 0; i < au2RxLen[u4PortId][j]; i+= 16) {
                DBGLOG(EMU, TRACE, ("[i] %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                                g_aucEmuBuf[i], g_aucEmuBuf[i + 1], g_aucEmuBuf[i + 2], g_aucEmuBuf[i + 3],
                                g_aucEmuBuf[i + 4], g_aucEmuBuf[i + 5], g_aucEmuBuf[i + 6], g_aucEmuBuf[i + 7],
                                g_aucEmuBuf[i + 8], g_aucEmuBuf[i + 9], g_aucEmuBuf[i + 10], g_aucEmuBuf[i + 11],
                                g_aucEmuBuf[i + 12], g_aucEmuBuf[i + 13], g_aucEmuBuf[i + 14], g_aucEmuBuf[i + 15]
                                ));
            }
        }
#endif
    }

    // test case of CheckTxCountBySingleReadWTSR
    nicPutMailbox(prAdapter, 0, HIF_TC_RX_AGG_PACKET);
    nicPutMailbox(prAdapter, 1, 1);
    nicSetSwIntr(prAdapter, BIT(31));

    DBGLOG(EMU, TRACE, (".....end....\n"));
    return status;
}

BOOLEAN
emuInitChkCis (
    IN P_ADAPTER_T prAdapter
    )
{
    P_GLUE_INFO_T           prGlueInfo = prAdapter->prGlueInfo;
    PDEVICE_EXTENSION   prDevExt = &prGlueInfo->rHifInfo.dx;
    UINT_32 i, addr;
    UINT_8 ucData;
    BOOLEAN status = TRUE;

    // check CCCR
    DBGLOG(EMU, TRACE, ("-- Check CCCR value ---\n"));
    for (i = 0; i < sizeof(aucCccr); i++) {
        sdioCmd52ByteReadWrite(
            prDevExt,
            i,
            &ucData,
            0,
            SDTD_READ);
        if (ucData != aucCccr[i]) {
            DBGLOG(INIT, ERROR, ("Error addr[0x%x]: 0x%x (expected: 0x%x)\n", i, ucData, aucCccr[i]));
            status = FALSE;
        }
    }
    DBGLOG(EMU, TRACE, ("-- Check Fn0 CIS value ---\n"));
    for (i = 0; i < sizeof(aucFn0Cis); i++) {
        addr = i + 0x1000;//cis0 start from 0x1000
        sdioCmd52ByteReadWrite(
            prDevExt,
            addr,
            &ucData,
            0,
            SDTD_READ);
        if (ucData != aucFn0Cis[i]) {
            DBGLOG(INIT, ERROR, ("Error addr[0x%x]: 0x%x (expected: 0x%x)\n", addr, ucData, aucFn0Cis[i]));
            status = FALSE;
        }
    }
    DBGLOG(EMU, TRACE, ("-- Check FBR1 value ---\n"));
    for (i = 0; i < sizeof(aucFBR1); i++) {
        addr = i + 0x100;//fbr1 start from 0x100
        sdioCmd52ByteReadWrite(
            prDevExt,
            addr,
            &ucData,
            0,
            SDTD_READ);
        if (ucData != aucFBR1[i]) {
            DBGLOG(INIT, ERROR, ("Error addr[0x%x]: 0x%x (expected: 0x%x)\n", addr, ucData, aucFBR1[i]));
            status = FALSE;
        }
    }
    DBGLOG(EMU, TRACE, ("-- Check Fn1 CIS value ---\n"));
    for (i = 0; i < sizeof(aucFn1Cis); i++) {
        addr = i + 0x2000;
        sdioCmd52ByteReadWrite(
            prDevExt,
            addr,
            &ucData,
            0,
            SDTD_READ);
        if (ucData != aucFn1Cis[i]) {
            DBGLOG(INIT, ERROR, ("Error addr[0x%x]: 0x%x (expected: 0x%x)\n", addr, ucData, aucFn1Cis[i]));
            status = FALSE;
        }
    }
#if 0
    DBGLOG(EMU, TRACE, ("-- Check FBR2 value ---\n"));
    for (i = 0x200; i < (0x200 + sizeof(aucFBR2)); i++) {
        sdioCmd52ByteReadWrite(
            prDevExt,
            i,
            &ucData,
            0,
            SDTD_READ);
        if (ucData != aucFBR2[i]) {
            DBGLOG(INIT, ERROR, ("Error addr[0x%x]: 0x%x (expected: 0x%x)\n", i, ucData, aucFBR2[i]));
        }
    }
#endif
    return status;
}

BOOLEAN
emuMixTxRxStress (
    IN P_ADAPTER_T prAdapter
    )
{
    UINT_32 len, i, offset = 0, j;
    UINT_8 aucTxCount[8];
    UINT_32 u4TxOkCount = 0;
    BOOLEAN status = TRUE;
    UINT_32 code;
    UINT_32 u4LenStart = 1500, u4LenEnd = u4LenStart + 15;

    DBGLOG(EMU, TRACE, (".....start....\n"));

    // test case of CheckTxCountBySingleReadWTSR
    nicPutMailbox(prAdapter, 0, HIF_TC_MIX_TX_RX_STRESS | HIF_TEST_CASE_START);
    nicPutMailbox(prAdapter, 1, 0);
    nicSetSwIntr(prAdapter, BIT(31));

    for (len = u4LenStart; len <= u4LenEnd; len++) {
        DBGLOG(EMU, TRACE, ("emuMixTxRxStress len: %d\n", len));

        *((PUINT_16)g_aucEmuBuf + offset) = (UINT_16)len;
        for (i = 0; i < len - 4; i++) {
            g_aucEmuBuf[offset + i + 4] = (UINT_8)i;
        }
        offset += len;
    }

    HAL_WRITE_TX_PORT(prAdapter,
                   0,
                   offset + len,
                   g_aucEmuBuf,
                   sizeof(g_aucEmuBuf));

    // wait until all the packets are done, and then start next TX AGG
    for (j = 0; j < 1000; j++) {
        HAL_READ_TX_RELEASED_COUNT(prAdapter, aucTxCount);
        u4TxOkCount += (UINT_32)aucTxCount[0];
        ASSERT(aucTxCount[1] == 0);
        ASSERT(aucTxCount[2] == 0);
        ASSERT(aucTxCount[3] == 0);
        ASSERT(aucTxCount[4] == 0);
        ASSERT(aucTxCount[5] == 0);
        if (u4TxOkCount == 16) {
            DBGLOG(EMU, TRACE, ("u4TxOkCount == accumNum\n"));
            break;
        }
    }

    // test case of CheckTxCountBySingleReadWTSR
    nicPutMailbox(prAdapter, 0, HIF_TC_MIX_TX_RX_STRESS);
    nicPutMailbox(prAdapter, 1, 1);
    nicSetSwIntr(prAdapter, BIT(31));

    DBGLOG(EMU, TRACE, (".....end....\n"));
    return status;
}

BOOLEAN
emuReadHalfRxPacket (
    IN P_ADAPTER_T prAdapter
    )
{
    UINT_32 len, i, offset = 0, j;
    UINT_8 aucTxCount[8];
    UINT_32 u4TxOkCount = 0;
    BOOLEAN status = TRUE;
    UINT_32 code;
    UINT_32 u4LenStart = 1500, u4LenEnd = u4LenStart + 15;

    DBGLOG(EMU, TRACE, (".....start....\n"));

    nicPutMailbox(prAdapter, 0, HIF_TC_RX_READ_HALF | HIF_TEST_CASE_START);
    nicPutMailbox(prAdapter, 1, 0);
    nicSetSwIntr(prAdapter, BIT(31));


    nicPutMailbox(prAdapter, 0, HIF_TC_RX_READ_HALF);
    nicPutMailbox(prAdapter, 1, 1);
    nicSetSwIntr(prAdapter, BIT(31));

    DBGLOG(EMU, TRACE, (".....end....\n"));
    return status;
}

BOOLEAN
emuRxPacketLenChk (
    IN P_ADAPTER_T prAdapter
    )
{
    UINT_32 i, j;
    BOOLEAN status = TRUE;
    UINT_16 aucRx0[] = {4, 80, 192, 55, 4095, 1477, 4092, 551, 2999, 10, 1999, 431, 1536, 66, 2211, 23};
    UINT_16 aucRx1[] = {0, 431, 0, 1536, 2211, 0, 1477, 4092, 0, 2999, 831, 4, 0, 192, 990, 0};
    UINT_16 aucRx0Valid[sizeof(aucRx0)] = {0};
    UINT_16 aucRx1Valid[sizeof(aucRx1)] = {0};
    ENHANCE_MODE_DATA_STRUCT_T rEnhanceDataStruct;
    UINT_32 u4Rx0Num = 0, u4Rx1Num = 0;
    UINT_16 u2RxReg0,u2RxReg1;
    UINT_32 u4Rx0NumValid = 0, u4Rx1NumValid = 0;
    UINT_32 u4Value;
    UINT_32 code0, code1;

    DBGLOG(EMU, TRACE, (".....start....\n"));


    ASSERT(u4Rx0Num <= 16);
    ASSERT(u4Rx1Num <= 16);
    ASSERT(sizeof(aucRx0) == sizeof(aucRx1));

    // test case of CheckTxCountBySingleReadWTSR
    nicPutMailbox(prAdapter, 0, HIF_TC_RX_PACKET_LEN | HIF_TEST_CASE_START);
    nicPutMailbox(prAdapter, 1, 0);
    nicSetSwIntr(prAdapter, BIT(31));

    kalMdelay(50);
    HAL_MCR_RD(prAdapter, MCR_WHISR, &u4Value); // clear FW ACK

    for (i = 0; i < sizeof(aucRx0)/ sizeof(UINT_16); i++) {

        code0 = aucRx0[i] | (aucRx1[i] << 16);
        code1 = 0;
        if (u4Rx0Num > 16) {
            code1 |= BIT(0);
        }
        if (u4Rx1Num > 16) {
            code1 |= BIT(1);
        }

        nicPutMailbox(prAdapter, 0, code0);
        nicPutMailbox(prAdapter, 1, code1);
        nicSetSwIntr(prAdapter, BIT(30));
        kalMdelay(50);// wait FW process

        if (aucRx0[i]) {
            aucRx0Valid[u4Rx0Num++] = aucRx0[i];
        }
        if (aucRx1[i]) {
            aucRx1Valid[u4Rx1Num++] = aucRx1[i];
        }

        //kalMdelay(50); // wait FW process

        HAL_READ_INTR_STATUS(prAdapter, sizeof(ENHANCE_MODE_DATA_STRUCT_T), (PUINT_8)&rEnhanceDataStruct);
        //dumpMemory8((PUINT_8)&rEnhanceDataStruct, sizeof(rEnhanceDataStruct));


        ////----------- check VALUE of rx len
        for (j = 0; j < u4Rx0Num; j++) {
#if 0
            DBGLOG(EMU, TRACE, ("[RX len idx: %d] read au2Rx0Len[%d]: %d, write RX0 len[%d]: %d\n",
                                    i,
                                    j,
                                    rEnhanceDataStruct.rRxInfo.u.au2Rx0Len[j],
                                    j,
                                    aucRx0Valid[j]));
#endif
            ASSERT(rEnhanceDataStruct.rRxInfo.u.au2Rx0Len[j] == aucRx0Valid[j]);
        }
        for (j = u4Rx0Num; j < 16; j++) {
            ASSERT(rEnhanceDataStruct.rRxInfo.u.au2Rx0Len[j] == 0);
        }
        for (j = 0; j < u4Rx1Num; j++) {
#if 0
            DBGLOG(EMU, TRACE, ("[RX len idx: %d] read au2Rx1Len[%d]: %d, write RX1 len[%d]: %d\n",
                                    i,
                                    j,
                                    rEnhanceDataStruct.rRxInfo.u.au2Rx1Len[j],
                                    j,
                                    aucRx1Valid[j]));
#endif
            ASSERT(rEnhanceDataStruct.rRxInfo.u.au2Rx1Len[j] == aucRx1Valid[j]);
        }
        for (j = u4Rx1Num; j < 16; j++) {
            ASSERT(rEnhanceDataStruct.rRxInfo.u.au2Rx1Len[j] == 0);
        }

        ////----------- check NUMBER of rx len
#if 0
        DBGLOG(EMU, TRACE, ("(expected) u4Rx0Num: %d, (actual) u4Rx0Num: %d\n",
                                u4Rx0Num, rEnhanceDataStruct.rRxInfo.u.u2NumValidRx0Len));
        DBGLOG(EMU, TRACE, ("(expected) u4Rx1Num: %d, (actual) u4Rx1Num: %d\n",
                                u4Rx1Num, rEnhanceDataStruct.rRxInfo.u.u2NumValidRx1Len));
#endif

        ASSERT(u4Rx0Num == rEnhanceDataStruct.rRxInfo.u.u2NumValidRx0Len);
        ASSERT(u4Rx1Num == rEnhanceDataStruct.rRxInfo.u.u2NumValidRx1Len);


        ////----------- check by register read, always returns 1st entry
        HAL_READ_RX_LENGTH(prAdapter, &u2RxReg0, &u2RxReg1);
        if (u4Rx0Num) {
#if 0
            DBGLOG(EMU, TRACE, ("leng: (expected) u2RxReg0: %d, (actual) aucRx0Valid[0]: %d\n",
                                    u2RxReg0, aucRx0Valid[0]));
#endif
            ASSERT(u2RxReg0 == aucRx0Valid[0]);
        }
        if (u4Rx1Num) {
#if 0
            DBGLOG(EMU, TRACE, ("leng: (expected) u2RxReg1: %d, (actual) aucRx1Valid[0]: %d\n",
                                    u2RxReg1, aucRx1Valid[0]));
#endif
            ASSERT(u2RxReg1 == aucRx1Valid[0]);
        }

    }

    // test case of CheckTxCountBySingleReadWTSR
    nicPutMailbox(prAdapter, 0, HIF_TC_RX_PACKET_LEN);
    nicPutMailbox(prAdapter, 1, 1);
    nicSetSwIntr(prAdapter, BIT(31));

    kalMdelay(50);

    ////----------- ensure packet FIFO is always 0 after stop test (which FW will "reset" RX)
    HAL_READ_RX_LENGTH(prAdapter, &u2RxReg0, &u2RxReg1);
    ASSERT(u2RxReg0 == 0);
    ASSERT(u2RxReg1 == 0);


    DBGLOG(EMU, TRACE, (".....end....\n"));
    return status;
}


BOOLEAN
emuRxPacketLenOverflow (
    IN P_ADAPTER_T prAdapter
    )
{
    UINT_32 j;
    BOOLEAN status = TRUE;
    ENHANCE_MODE_DATA_STRUCT_T rEnhanceDataStruct;

    DBGLOG(EMU, TRACE, (".....start....\n"));


    // test case of CheckTxCountBySingleReadWTSR
    nicPutMailbox(prAdapter, 0, HIF_TC_RX_PACKET_LEN_OVERFLOW | HIF_TEST_CASE_START);
    nicPutMailbox(prAdapter, 1, 0);
    nicSetSwIntr(prAdapter, BIT(31));

    kalMdelay(50);

    /* entry #1~16 */
    for (j = 0; j < 16; j++) {
        nicPutMailbox(prAdapter, 0, 100 + j);
        nicPutMailbox(prAdapter, 1, 200 + j);
        nicSetSwIntr(prAdapter, BIT(30));

        kalMdelay(50);
    }

    HAL_READ_INTR_STATUS(prAdapter, sizeof(ENHANCE_MODE_DATA_STRUCT_T), (PUINT_8)&rEnhanceDataStruct);

    if ((rEnhanceDataStruct.u4WHISR & BIT(30))) {
        DBGLOG(EMU, TRACE, ("- u4WHISR = 0x%x\n", rEnhanceDataStruct.u4WHISR));
        status = FALSE;
        ASSERT(0);
    }

    if (rEnhanceDataStruct.u4RcvMailbox0 != 0x0) {
        DBGLOG(EMU, TRACE, ("- u4RcvMailbox0 = 0x%x\n", rEnhanceDataStruct.u4RcvMailbox0));
        status = FALSE;
        ASSERT(0);
    }

    /* entry #17 */
    nicPutMailbox(prAdapter, 0, 100);
    nicPutMailbox(prAdapter, 1, 200);
    nicSetSwIntr(prAdapter, BIT(30));

    kalMdelay(50);

    HAL_READ_INTR_STATUS(prAdapter, sizeof(ENHANCE_MODE_DATA_STRUCT_T), (PUINT_8)&rEnhanceDataStruct);

    if (!(rEnhanceDataStruct.u4WHISR & BIT(30))) {
        DBGLOG(EMU, TRACE, ("u4WHISR = 0x%x\n", rEnhanceDataStruct.u4WHISR));
        status = FALSE;
        ASSERT(0);
    }

    if (rEnhanceDataStruct.u4RcvMailbox0 != 0x3) {
        DBGLOG(EMU, TRACE, ("u4RcvMailbox0 = 0x%x\n", rEnhanceDataStruct.u4RcvMailbox0));
        status = FALSE;
        ASSERT(0);
    }

    // test case of CheckTxCountBySingleReadWTSR
    nicPutMailbox(prAdapter, 0, HIF_TC_RX_PACKET_LEN_OVERFLOW);
    nicPutMailbox(prAdapter, 1, 1);
    nicSetSwIntr(prAdapter, BIT(31));


    DBGLOG(EMU, TRACE, (".....end....\n"));
    return status;
}


BOOLEAN
emuIntrEnhanceChk (
    IN P_ADAPTER_T prAdapter
    )
{
    BOOLEAN status = FALSE;
    ENHANCE_MODE_DATA_STRUCT_T rEnhanceDataStruct;
    ENHANCE_MODE_DATA_STRUCT_T rEmptyEnhanceDataStruct = {0};
    UINT_32 u4Mbox0, u4Mbox1;

    DBGLOG(EMU, TRACE, (".....start....\n"));


    nicPutMailbox(prAdapter, 0, HIF_TC_INTR_ENHANCE | HIF_TEST_CASE_START);
    nicPutMailbox(prAdapter, 1, 0); // FW set SW intr, TX done, RX length, mailbox
    nicSetSwIntr(prAdapter, BIT(31));

    do {
        // Enable mailbox read clear
        HAL_SET_MAILBOX_READ_CLEAR(prAdapter, FALSE);

        /* ----- test 1 -------- */
        // FW set SW intr, TX done, RX length, mailbox
        nicPutMailbox(prAdapter, 1, 101); // FW set SW intr, TX done, RX length, mailbox
        nicSetSwIntr(prAdapter, BIT(30));

        kalMdelay(50); //expect FW ready before it

        kalMemZero(&rEnhanceDataStruct, sizeof(rEnhanceDataStruct));
        HAL_READ_INTR_STATUS(prAdapter, 4, (PUINT_8)&rEnhanceDataStruct);
//        dumpMemory8((PUINT_8)&rEnhanceDataStruct, sizeof(rEnhanceDataStruct));
        // verify the content (only non-empty INTR status)
        DBGLOG(EMU, TRACE, (" - verify the content (only non-empty INTR status)\n"));
        if (!(rEnhanceDataStruct.u4WHISR & BIT(30))) {
            ASSERT(0);
            break;
        }
        if (kalMemCmp( ((PUINT_8)&rEnhanceDataStruct) + 4,
                       &rEmptyEnhanceDataStruct,
                       sizeof(rEmptyEnhanceDataStruct) - 4)) {
            ASSERT(0);
            break;
        }


        /* ----- test 2 -------- */
        // FW set SW intr, TX done, RX length, mailbox
        kalMemZero(&rEnhanceDataStruct, sizeof(rEnhanceDataStruct));
        HAL_READ_INTR_STATUS(prAdapter, (4 + 8), (PUINT_8)&rEnhanceDataStruct);
        // verify the content (only INTR status + TX status)
//        dumpMemory8((PUINT_8)&rEnhanceDataStruct, sizeof(rEnhanceDataStruct));
        DBGLOG(EMU, TRACE, (" - verify the content (only INTR status + TX status)\n"));
        // verify the content (only non-empty TX status)
        if ((rEnhanceDataStruct.u4WHISR & BIT(30))) {
            ASSERT(0);
            break;
        }
        if ((rEnhanceDataStruct.rTxInfo.au4WTSR[0] != 0x01010101) ||
            (rEnhanceDataStruct.rTxInfo.au4WTSR[1] != 0x0101)) {
            ASSERT(0);
            break;
        }
        if (kalMemCmp( ((PUINT_8)&rEnhanceDataStruct) + (4 + 8),
                       &rEmptyEnhanceDataStruct,
                       sizeof(rEmptyEnhanceDataStruct) - (4 + 8))) {
            ASSERT(0);
            break;
        }


        /* ----- test 3 -------- */
        // FW set SW intr, TX done, RX length, mailbox
        kalMemZero(&rEnhanceDataStruct, sizeof(rEnhanceDataStruct));
        HAL_READ_INTR_STATUS(prAdapter, 12 + 68, (PUINT_8)&rEnhanceDataStruct);
        // verify the content (only INTR status + TX status + RX status)
        DBGLOG(EMU, TRACE, (" - verify the content (only INTR status + TX status + RX status)\n"));
//        dumpMemory8((PUINT_8)&rEnhanceDataStruct, sizeof(rEnhanceDataStruct));
        // verify the content (only INTR status)
        if ((rEnhanceDataStruct.u4WHISR & BIT(30))) {
            ASSERT(0);
            break;
        }
        if ((rEnhanceDataStruct.rTxInfo.au4WTSR[0] != 0) ||
            (rEnhanceDataStruct.rTxInfo.au4WTSR[1] != 0)) {
            ASSERT(0);
            break;
        }
        if ((rEnhanceDataStruct.rRxInfo.u.u2NumValidRx0Len != 3) ||
            (rEnhanceDataStruct.rRxInfo.u.u2NumValidRx1Len != 2) ||
            (rEnhanceDataStruct.rRxInfo.u.au2Rx0Len[0] != 100) ||
            (rEnhanceDataStruct.rRxInfo.u.au2Rx0Len[1] != 200) ||
            (rEnhanceDataStruct.rRxInfo.u.au2Rx0Len[2] != 300) ||
            (rEnhanceDataStruct.rRxInfo.u.au2Rx1Len[0] != 55) ||
            (rEnhanceDataStruct.rRxInfo.u.au2Rx1Len[1] != 66)
            ) {
#if 1
            DBGLOG(EMU, TRACE, ("u2NumValidRx0Len: %d\n", rEnhanceDataStruct.rRxInfo.u.u2NumValidRx0Len));
            DBGLOG(EMU, TRACE, ("u2NumValidRx1Len: %d\n", rEnhanceDataStruct.rRxInfo.u.u2NumValidRx1Len));
            DBGLOG(EMU, TRACE, ("au2Rx0Len[0]: %d\n", rEnhanceDataStruct.rRxInfo.u.au2Rx0Len[0]));
            DBGLOG(EMU, TRACE, ("au2Rx0Len[1]: %d\n", rEnhanceDataStruct.rRxInfo.u.au2Rx0Len[1]));
            DBGLOG(EMU, TRACE, ("au2Rx0Len[2]: %d\n", rEnhanceDataStruct.rRxInfo.u.au2Rx0Len[2]));
            DBGLOG(EMU, TRACE, ("au2Rx1Len[0]: %d\n", rEnhanceDataStruct.rRxInfo.u.au2Rx1Len[0]));
            DBGLOG(EMU, TRACE, ("au2Rx1Len[1]: %d\n", rEnhanceDataStruct.rRxInfo.u.au2Rx1Len[1]));
#endif
            ASSERT(0);
            break;
        }
        if (kalMemCmp( ((PUINT_8)&rEnhanceDataStruct) + (12 + 68),
                       &rEmptyEnhanceDataStruct,
                       sizeof(rEmptyEnhanceDataStruct) - (12 + 68))) {
            ASSERT(0);
            break;
        }


        /* ----- test 4 -------- */
        // FW set SW intr, TX done, RX length, mailbox

        // disable mailbox read clear
        HAL_SET_MAILBOX_READ_CLEAR(prAdapter, FALSE);

        kalMemZero(&rEnhanceDataStruct, sizeof(rEnhanceDataStruct));
        HAL_READ_INTR_STATUS(prAdapter, sizeof(ENHANCE_MODE_DATA_STRUCT_T), (PUINT_8)&rEnhanceDataStruct);
        DBGLOG(EMU, TRACE, (" - verify the content (full INTR status + TX status + RX status + mailbox)\n"));
//        dumpMemory8((PUINT_8)&rEnhanceDataStruct, sizeof(rEnhanceDataStruct));
        // verify the content (full INTR status + TX status + RX status + mailbox)
        if ((rEnhanceDataStruct.u4WHISR & BIT(30))) {
            ASSERT(0);
            break;
        }
        if ((rEnhanceDataStruct.rTxInfo.au4WTSR[0] != 0) ||
            (rEnhanceDataStruct.rTxInfo.au4WTSR[1] != 0)) {
            ASSERT(0);
            break;
        }
        if ((rEnhanceDataStruct.rRxInfo.u.u2NumValidRx0Len != 3) ||
            (rEnhanceDataStruct.rRxInfo.u.u2NumValidRx1Len != 2) ||
            (rEnhanceDataStruct.rRxInfo.u.au2Rx0Len[0] != 100) ||
            (rEnhanceDataStruct.rRxInfo.u.au2Rx0Len[1] != 200) ||
            (rEnhanceDataStruct.rRxInfo.u.au2Rx0Len[2] != 300) ||
            (rEnhanceDataStruct.rRxInfo.u.au2Rx1Len[0] != 55) ||
            (rEnhanceDataStruct.rRxInfo.u.au2Rx1Len[1] != 66)
            ) {
            ASSERT(0);
            break;
        }
        if ((rEnhanceDataStruct.u4RcvMailbox0 != 0x1234) ||
            (rEnhanceDataStruct.u4RcvMailbox1 != 0x5678)) {
            ASSERT(0);
            break;
        }

        HAL_GET_MAILBOX(prAdapter, 0, &u4Mbox0);
        HAL_GET_MAILBOX(prAdapter, 1, &u4Mbox1);
        if ((u4Mbox0 != 0x1234) ||
            (u4Mbox1 != 0x5678)) {
            ASSERT(0);
            break;
        }

        /* ----- test 5 -------- */
        // FW set SW intr, TX done, RX length, mailbox

        // Enable mailbox read clear
        HAL_SET_MAILBOX_READ_CLEAR(prAdapter, TRUE);

        HAL_READ_INTR_STATUS(prAdapter, sizeof(rEnhanceDataStruct), (PUINT_8)&rEnhanceDataStruct);
        // verify the content (full INTR status + TX status + RX status + RC mailbox)
        DBGLOG(EMU, TRACE, (" - verify the content (full INTR status + TX status + RX status + RC mailbox)\n"));
//        dumpMemory8((PUINT_8)&rEnhanceDataStruct, sizeof(rEnhanceDataStruct));
        if ((rEnhanceDataStruct.u4WHISR & BIT(30))) {
            ASSERT(0);
            break;
        }
        if ((rEnhanceDataStruct.rTxInfo.au4WTSR[0] != 0) ||
            (rEnhanceDataStruct.rTxInfo.au4WTSR[1] != 0)) {
            ASSERT(0);
            break;
        }
        if ((rEnhanceDataStruct.rRxInfo.u.u2NumValidRx0Len != 3) ||
            (rEnhanceDataStruct.rRxInfo.u.u2NumValidRx1Len != 2) ||
            (rEnhanceDataStruct.rRxInfo.u.au2Rx0Len[0] != 100) ||
            (rEnhanceDataStruct.rRxInfo.u.au2Rx0Len[1] != 200) ||
            (rEnhanceDataStruct.rRxInfo.u.au2Rx0Len[2] != 300) ||
            (rEnhanceDataStruct.rRxInfo.u.au2Rx1Len[0] != 55) ||
            (rEnhanceDataStruct.rRxInfo.u.au2Rx1Len[1] != 66)
            ) {
            ASSERT(0);
            break;
        }
        if ((rEnhanceDataStruct.u4RcvMailbox0 != 0x1234) ||
            (rEnhanceDataStruct.u4RcvMailbox1 != 0x5678)) {
            ASSERT(0);
            break;
        }

        HAL_GET_MAILBOX(prAdapter, 0, &u4Mbox0);
        HAL_GET_MAILBOX(prAdapter, 1, &u4Mbox1);
        if ((u4Mbox0 != 0) ||
            (u4Mbox1 != 0)) {
            ASSERT(0);
            break;
        }




        status = TRUE;
    } while (FALSE);

    nicPutMailbox(prAdapter, 0, HIF_TC_INTR_ENHANCE);
    nicPutMailbox(prAdapter, 1, 1);
    nicSetSwIntr(prAdapter, BIT(31));


    DBGLOG(EMU, TRACE, (".....end....\n"));
    return status;
}

BOOLEAN
emuChkIntEn (
    IN P_ADAPTER_T prAdapter
    )
{
    P_GLUE_INFO_T           prGlueInfo = prAdapter->prGlueInfo;
    PDEVICE_EXTENSION   prDevExt = &prGlueInfo->rHifInfo.dx;
    UINT_16 u2IntEn;
    NDIS_STATUS                rStatus;
    BOOLEAN status = TRUE;

    rStatus = sdioConfigProperty(prDevExt,
        SDRF_GET_PROPERTY,
        SDP_FUNCTION_INT_ENABLE,
        (PUINT_8)&u2IntEn,
        sizeof(u2IntEn));

    if (!NT_SUCCESS(rStatus)) {
        //ERRORLOG(("SdBusSubmitRequest fail, status:%x\n", rStatus));
        return FALSE;
    }
    else {
        //INITLOG(("[SDIO] GET INT En= %d \n", u2IntEn));
    }

    u2IntEn = (u2IntEn) ? 0 : 1;
    rStatus = sdioConfigProperty(prDevExt,
        SDRF_SET_PROPERTY,
        SDP_FUNCTION_INT_ENABLE,
        (PUINT_8)&u2IntEn,
        sizeof(u2IntEn));

    if (!NT_SUCCESS(rStatus)) {
        //ERRORLOG(("SdBusSubmitRequest fail, status:%x\n", rStatus));
        return FALSE;
    }
    else {
        //INITLOG(("[SDIO] SET INT En= %d \n", u2IntEn));
    }

    rStatus = sdioConfigProperty(prDevExt,
        SDRF_GET_PROPERTY,
        SDP_FUNCTION_INT_ENABLE,
        (PUINT_8)&u2IntEn,
        sizeof(u2IntEn));

    if (!NT_SUCCESS(rStatus)) {
        //ERRORLOG(("SdBusSubmitRequest fail, status:%x\n", rStatus));
        return FALSE;
    }
    else {
        //INITLOG(("[SDIO] GET INT En= %d \n", u2IntEn));
    }

    return status;
}

BOOLEAN
emuCheckChipID (
    IN P_ADAPTER_T prAdapter
    )
{
    BOOLEAN status = TRUE;
    UINT_32 u4Value;

    HAL_MCR_RD(prAdapter, MCR_WCIR, &u4Value);
    if ((u4Value & BITS(0, 19)) != 0x6620) {
        ASSERT(0);
        status = FALSE;
    }

    return status;
}

BOOLEAN
emuCheckPowerOnResetFunc (
    IN P_ADAPTER_T prAdapter
    )
{
    BOOLEAN status = TRUE;
    UINT_32 u4Value;

    HAL_MCR_RD(prAdapter, MCR_WCIR, &u4Value);
    if (!(u4Value & BIT(20))) {
//        ASSERT(0);
        DBGLOG(INIT, ERROR, ("POR is not 1 (0x%x)\n", u4Value));
        status = FALSE;
    } else {
        HAL_MCR_WR(prAdapter, MCR_WCIR, u4Value);
        HAL_MCR_RD(prAdapter, MCR_WCIR, &u4Value);
        if ((u4Value & BIT(20))) {
    //        ASSERT(0);
            DBGLOG(INIT, ERROR, ("POR is not 0 (0x%x)\n", u4Value));
            status = FALSE;
        }
    }
    return status;
}


BOOLEAN
emuLPown_ownback_stress (
    IN P_ADAPTER_T prAdapter,
    IN UINT_32     u4LoopCount
    )
{
    UINT_32 i;
    BOOLEAN fgResult;
    UINT_8 aucDelayTimeMs[] = {5, 10, 100};
    BOOLEAN status = TRUE;

    DBGLOG(EMU, TRACE, (".....start....\n"));


    for (i = 0; i < u4LoopCount; i++) {
        HAL_LP_OWN_SET(prAdapter);
        kalMdelay(aucDelayTimeMs[i%sizeof(aucDelayTimeMs)]);
        HAL_LP_OWN_CLR_OK(prAdapter, &fgResult);

        if (!fgResult) {
            DBGLOG(EMU, ERROR, ("Fail to ownback! [loop count: %d] \n", i));
        }
        if ((i % 100) == 0) {
            DBGLOG(EMU, TRACE, (".\n"));
        }
    }


    DBGLOG(EMU, TRACE, (".....end....\n"));

    return status;
}

BOOLEAN
emuLPown_illegal_access (
    IN P_ADAPTER_T prAdapter
    )
{
    UINT_32 i;
    BOOLEAN fgResult;
    UINT_32 u4Wasr, u4Whisr, u4Value, u4WHIERValue;
    BOOLEAN status = TRUE;

    DBGLOG(EMU, TRACE, (".....start....\n"));

    HAL_LP_OWN_CLR_OK(prAdapter, &fgResult);

    // disable SW interrupt
    HAL_MCR_RD(prAdapter, MCR_WHIER, &u4WHIERValue);
    HAL_MCR_WR(prAdapter, MCR_WHIER, 0);

    // read clear for the WASR
    HAL_GET_ABNORMAL_INTERRUPT_REASON_CODE(prAdapter, &u4Value);

    // it should be read clear by 1st time
    HAL_GET_ABNORMAL_INTERRUPT_REASON_CODE(prAdapter, &u4Value);
    if (u4Value) {
        ASSERT(0);
        status = FALSE;
    }

    for (i = MCR_WCIR; i <= MCR_WRPLR; i+=4) {
        // set to sleep
        HAL_LP_OWN_SET(prAdapter);

        // read
        HAL_MCR_RD(prAdapter, i, &u4Value);

        // write
        HAL_MCR_WR(prAdapter, i, 0);

        // check the result
        HAL_LP_OWN_CLR_OK(prAdapter, &fgResult);

        HAL_MCR_RD(prAdapter, MCR_WHISR, &u4Whisr);
        HAL_GET_ABNORMAL_INTERRUPT_REASON_CODE(prAdapter, &u4Wasr);
        if ((i == MCR_WCIR) ||
            (i == MCR_WHLPCR) ||
            (i == MCR_WSDIOCSR)) {
            if (u4Wasr & WASR_FW_OWN_INVALID_ACCESS) {
                DBGLOG(INIT, ERROR, ("u4Wasr && WASR_FW_OWN_INVALID_ACCESS for R/W addr: 0x%x (0x%x)\n",
                            i, u4Wasr));
                status = FALSE;
            }
            if (u4Whisr & WHISR_ABNORMAL_INT) {
                DBGLOG(INIT, ERROR, ("u4Whisr && WHISR_ABNORMAL_INT for R/W addr: 0x%x (0x%x)\n",
                            i, u4Whisr));
                status = FALSE;
            }
        } else {
            if (!(u4Wasr & WASR_FW_OWN_INVALID_ACCESS)) {
                DBGLOG(INIT, ERROR, ("!(u4Wasr && WASR_FW_OWN_INVALID_ACCESS) for R/W addr: 0x%x (0x%x)\n",
                            i, u4Wasr));
                status = FALSE;
            }
            if (!(u4Whisr & WHISR_ABNORMAL_INT)) {
                DBGLOG(INIT, ERROR, ("!(u4Whisr && WHISR_ABNORMAL_INT) for R/W addr: 0x%x (0x%x)\n",
                            i, u4Whisr));
                status = FALSE;
            }
        }
    }

    HAL_MCR_WR(prAdapter, MCR_WHIER, u4WHIERValue);

    DBGLOG(EMU, TRACE, (".....end....\n"));

    return status;
}


#if defined(_HIF_SDIO)
VOID
emuCheckWSDIOCSR (
    IN P_ADAPTER_T prAdapter
    )
{
    UINT_32 u4WSDIOCSR;

    HAL_MCR_RD(prAdapter, MCR_WSDIOCSR, &u4WSDIOCSR);
    //INITLOG(("u4WSDIOCSR=0x%x\n", u4WSDIOCSR));

    if (u4WSDIOCSR != 1) {
        //ERRORLOG(("u4WSDIOCSR=0x%x\n", u4WSDIOCSR));
    }
}
#endif

VOID
emuWorkItemProc (
    IN struct _NDIS_WORK_ITEM *  pWorkItem,
    IN PVOID  Context
    )
{
    UINT_32 i, j;
    P_ADAPTER_T prAdapter = (P_ADAPTER_T) Context;
    //INITLOG(("emuWorkItemProc\n"));

    switch (prAdapter->prGlueInfo->ucEmuWorkItemId) {
    case 0xf001:
        emuStart(prAdapter);
        break;
    case 0xf003:
        {
#if 1
        UINT_32 i;
        for (i = 10; i < 50; i++) {
#if 0
                emuRxPacket1(prAdapter,
                             0,
                             i,
                             FALSE,
                             FALSE,
                             TRUE);
#endif
                emuRxPacketSwHdrFormat(prAdapter,
                                       0,
                                       i,
                                       4);
        }
#endif
        }
        break;
    case 0xf004:
#if 0
        for (i = 0; i < 100; i++) {
            PACKET_INFO_INIT(&rPacketInfo,
                             (KAL_GET_PKT_IS_802_11(prNativePacket) ? TRUE : FALSE),
                             fgIs1x,
                             (KAL_GET_PKT_IS_PAL(prNativePacket) ? TRUE : FALSE),
                             prNativePacket,
                             ucTID,
                             KAL_GET_PKT_HEADER_LEN(prNativePacket),
                             KAL_GET_PKT_FRAME_LEN(prNativePacket));
            nicTxData(prAdapter, prPacketInfo, 0);
        }
#endif
        break;
    case 0xf005:
        for (i = 100; i < 3000; i++) {
                emuSendPacketAggN(prAdapter, 0, 6, 100 + i, 200 + i, FALSE);
                for (j = 100; j < 200; j++) {
                    emuRxPacketSwHdrFormat(prAdapter,
                                           0,
                                           j,
                                           4);
                }
                emuSendPacketAggN(prAdapter, 1, 2, 400 + i, 500 + i, FALSE);
        }
        break;
    case 0xf006:
        /* start TX burst */
        emuTxPacketBurstInSwHdrFormat(prAdapter, TRUE);
        break;
    case 0xf007:
        /* stop TX burst */
        emuTxPacketBurstInSwHdrFormat(prAdapter, FALSE);
        break;
    case 0xf008:
        /* start RX burst */
        emuRxPacketBurstInSwHdrFormat(prAdapter, TRUE);
        break;
    case 0xf009:
        /* stop RX burst */
        emuRxPacketBurstInSwHdrFormat(prAdapter, FALSE);
        break;
    case 0xf00a:
        emuRxPacket1(prAdapter,
                                    0,
                                    512,
                                    FALSE,
                                    FALSE,
                                    FALSE);
        break;
    case 0xf00b:
        emuRxPacket1(prAdapter,
                                    0,
                                    513,
                                    FALSE,
                                    FALSE,
                                    FALSE);
        break;
    }
}


#endif // #if CFG_HIF_EMULATION_TEST



