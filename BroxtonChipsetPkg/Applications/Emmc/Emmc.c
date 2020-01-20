/** @file

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
/** @file
  This sample application bases on HelloWorld PCD setting 
  to print "UEFI Hello World!" to the UEFI Console.

  Copyright (c) 2013, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials                          
  are licensed and made available under the terms and conditions of the BSD License         
  which accompanies this distribution.  The full text of the license may be found at        
  http://opensource.org/licenses/bsd-license.php                                            

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

**/

#include "Emmc.h"

// 
// Used to check-call local functions inside this eMMC module
//
// Func - The function name which needs to be checked
// Str	 - Optional, it is the description of the output infomation
// FailOrNot - When functional call is failed, it should stop or continue
//
#define CHK_FUNC_CALL(Func, Str, FailOrNot) \
    do { \
    EFI_STATUS ret = Func; \
    if (EFI_SUCCESS!= ret && TRUE == FailOrNot) { \
        Print(L"ERROR: %a,%d:", __FUNCTION__,__LINE__); \
        if ( NULL != (void *)Str  ) {\
            Print(L"%a:", Str); \
        } \
        Print(L" Status = 0x%x\n", ret); \
        return ret; \
    } \
    } while (0)

#define HOST_BLK_SIZE 512
#define MAX_GPP 4
#ifdef BYT_CR_PF_BUILD
#define B_PWR_WP_EN              0x01
#endif
typedef struct  {
    CARD_TYPE                  CardType;
    UINT32                    Address;
    UINT32                    BlockLen;
    UINT32                    MaxFrequency;
    UINT64                    BlockNumber;
    UINT8                     CurrentBusWidth; 
    CARD_STATUS               CardStatus;
    OCR                       OCRReg;
    CID                       CIDReg;
    CSD                       CSDReg; 
    EXT_CSD                   ExtCSDReg;
    EFI_BLOCK_IO_PROTOCOL     *BlockIo;
    EFI_SD_HOST_IO_PROTOCOL   *SdHostIo;
    UINT8                     *RawBufferPointer;  
    UINT8                     *AlignedBuffer; 
}EmmcCardData;

EmmcCardData gEmmcData;


VOID
EFIAPI
EmmcInfo (
  IN EmmcCardData *Data
  );

UINT64
EFIAPI
EmmcGetFlashSize (
    IN EmmcCardData *Data
  );

UINT32 
EmmcGetBootPartitionSize (
  IN EmmcCardData *Data
  );

UINT64 
EmmcGetMaxEnhSize (
  IN EmmcCardData *Data
  );

VOID
EmmcGetCardRevision (
 IN EmmcCardData *Data
 );

EFI_STATUS
CheckCardStatus (
  IN  UINT32    Status
  ) 
{
  CARD_STATUS    *CardStatus;
  CardStatus = (CARD_STATUS*)(&Status);

  if (CardStatus->ADDRESS_OUT_OF_RANGE) {
    DEBUG ((EFI_D_ERROR, "CardStatus: ADDRESS_OUT_OF_RANGE\n"));
  }
  
  if (CardStatus->ADDRESS_MISALIGN) {
    DEBUG ((EFI_D_ERROR, "CardStatus: ADDRESS_MISALIGN\n"));
  }

  if (CardStatus->BLOCK_LEN_ERROR) {
    DEBUG ((EFI_D_ERROR, "CardStatus: BLOCK_LEN_ERROR\n"));
  }

  if (CardStatus->ERASE_SEQ_ERROR) {
    DEBUG ((EFI_D_ERROR, "CardStatus: ERASE_SEQ_ERROR\n"));
  }

  if (CardStatus->ERASE_PARAM) {
    DEBUG ((EFI_D_ERROR, "CardStatus: ERASE_PARAM\n"));
  }

  if (CardStatus->WP_VIOLATION) {
    DEBUG ((EFI_D_ERROR, "CardStatus: WP_VIOLATION\n"));
  }
  
  if (CardStatus->CARD_IS_LOCKED) {
    DEBUG ((EFI_D_ERROR, "CardStatus: CARD_IS_LOCKED\n"));
  }

  if (CardStatus->LOCK_UNLOCK_FAILED) {
    DEBUG ((EFI_D_ERROR, "CardStatus: LOCK_UNLOCK_FAILED\n"));
  }

  if (CardStatus->COM_CRC_ERROR) {
    DEBUG ((EFI_D_ERROR, "CardStatus: COM_CRC_ERROR\n"));
  }

  if (CardStatus->ILLEGAL_COMMAND) {
    DEBUG ((EFI_D_ERROR, "CardStatus: ILLEGAL_COMMAND\n"));
  }

  if (CardStatus->CARD_ECC_FAILED) {
    DEBUG ((EFI_D_ERROR, "CardStatus: CARD_ECC_FAILED\n"));
  }

  if (CardStatus->CC_ERROR) {
    DEBUG ((EFI_D_ERROR, "CardStatus: CC_ERROR\n"));
  }

  if (CardStatus->ERROR) {
    DEBUG ((EFI_D_ERROR, "CardStatus: ERROR\n"));
  }

  if (CardStatus->UNDERRUN) {
    DEBUG ((EFI_D_ERROR, "CardStatus: UNDERRUN\n"));
  }

  if (CardStatus->OVERRUN) {
    DEBUG ((EFI_D_ERROR, "CardStatus: OVERRUN\n"));
  }

  if (CardStatus->CID_CSD_OVERWRITE) {
    DEBUG ((EFI_D_ERROR, "CardStatus: CID_CSD_OVERWRITE\n"));
  }

  if (CardStatus->WP_ERASE_SKIP) {
    DEBUG ((EFI_D_ERROR, "CardStatus: WP_ERASE_SKIP\n"));
  }

  if (CardStatus->ERASE_RESET) {
    DEBUG ((EFI_D_ERROR, "CardStatus: ERASE_RESET\n"));
  }

  if (CardStatus->SWITCH_ERROR) {
    DEBUG ((EFI_D_ERROR, "CardStatus: SWITCH_ERROR\n"));
  }

  if ((Status & 0xFCFFA080) != 0) {
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}




EFI_STATUS
SendCommand (
  IN   EFI_SD_HOST_IO_PROTOCOL    *This,
  IN   UINT16                     CommandIndex,
  IN   UINT32                     Argument,
  IN   TRANSFER_TYPE              DataType,
  IN   UINT8                      *Buffer, OPTIONAL
  IN   UINT32                     BufferSize,    
  IN   RESPONSE_TYPE              ResponseType,
  IN   UINT32                     TimeOut,  
  OUT  UINT32                     *ResponseData
  )
/*++

  Routine Description:
    Send command by using Host IO protocol
  
  Arguments:
    This           - Pointer to EFI_SD_HOST_IO_PROTOCOL
    CommandIndex   - The command index to set the command index field of command register
    Argument       - Command argument to set the argument field of command register
    DataType       - TRANSFER_TYPE, indicates no data, data in or data out
    Buffer         - Contains the data read from / write to the device
    BufferSize     - The size of the buffer
    ResponseType   - RESPONSE_TYPE
    TimeOut        - Time out value in 1 ms unit
    ResponseData   - Depending on the ResponseType, such as CSD or card status

  Returns:  
    EFI_INVALID_PARAMETER
    EFI_UNSUPPORTED
    EFI_DEVICE_ERROR
    EFI_SUCCESS
 --*/   
{

  EFI_STATUS    Status;

  Status = This->SendCommand (
                   This,
                   CommandIndex,
                   Argument,
                   DataType,
                   Buffer,
                   BufferSize,
                   ResponseType,
                   TimeOut,
                   ResponseData
                   );
  if (!EFI_ERROR (Status)) {
    if (ResponseType == ResponseR1 || ResponseType == ResponseR1b) {
      ASSERT(ResponseData != NULL);
      Status = CheckCardStatus (*ResponseData);
    }
  } else {
    This->ResetSdHost (This, Reset_DAT_CMD);
  }

  return Status;
}


EFI_STATUS SendCmd0( IN EmmcCardData *Data) 
{
    return SendCommand (
                        Data->SdHostIo,
                        GO_IDLE_STATE,
                        0,
                        NoData,
                        NULL,
                        0,  
                        ResponseNo,
                        TIMEOUT_COMMAND,
                        NULL
                        );

}

 EFI_STATUS SendCmd1(IN EmmcCardData *Data, IN UINT32 OcrValue) 
{   
    return SendCommand (
                          Data->SdHostIo,
                          SEND_OP_COND,
                          OcrValue,
                          NoData,
                          NULL,
                          0,  
                          ResponseR3,
                          TIMEOUT_COMMAND,
                          (UINT32*)&(Data->OCRReg)
                          );
}

 EFI_STATUS SendCmd2(IN EmmcCardData *Data) 
{
    return SendCommand (
                            Data->SdHostIo,
                            ALL_SEND_CID,
                            0,
                            NoData,
                            NULL,
                            0,  
                            ResponseR2,
                            TIMEOUT_COMMAND,
                            (UINT32*)&(Data->CIDReg)
                            );

}

 EFI_STATUS SendCmd3(IN EmmcCardData *Data) 
{
    return SendCommand (
                            Data->SdHostIo,
                            SET_RELATIVE_ADDR,
                            (Data->Address << 16),
                            NoData,
                            NULL,
                            0,  
                            ResponseR1,
                            TIMEOUT_COMMAND,
                            (UINT32*)&(Data->CardStatus)
                            );

}

 EFI_STATUS SendCmd6(IN EmmcCardData *Data, IN SWITCH_ARGUMENT Argument) 
{
    return  SendCommand (
                            Data->SdHostIo,
                            SWITCH,
                            *(UINT32*)&Argument,
                            NoData,
                            NULL,
                            0,   
                            ResponseR1b,
                            TIMEOUT_COMMAND,
                            (UINT32*)&(Data->CardStatus)
                            );


}


 EFI_STATUS SendCmd7(IN EmmcCardData *Data, UINT32 Address) 
{
return  SendCommand (
                            Data->SdHostIo,
                            SELECT_DESELECT_CARD,
                            (Address << 16),
                            NoData,
                            NULL,
                            0,  
                            ResponseR1,
                            TIMEOUT_COMMAND,
                            (UINT32*)&(Data->CardStatus)
                            );

}

 EFI_STATUS SendCmd8(IN EmmcCardData *Data) 
{
    return SendCommand (
                            Data->SdHostIo,
                            SEND_EXT_CSD,
                            (Data->Address << 16),
                            InData,
                            Data->AlignedBuffer,
                            sizeof (EXT_CSD),
                            ResponseR1,
                            TIMEOUT_DATA,
                            (UINT32*)&(Data->CardStatus)
                            );

}

 EFI_STATUS SendCmd9(IN EmmcCardData *Data) 
{
    return SendCommand (
                            Data->SdHostIo,
                            SEND_CSD,
                            (Data->Address << 16),
                            NoData,
                            NULL,
                            0,  
                            ResponseR2,
                            TIMEOUT_COMMAND,
                            (UINT32*)&(Data->CSDReg)
                            );
}


 EFI_STATUS SendCmd13(IN EmmcCardData *Data) 
{
    return  SendCommand (
                            Data->SdHostIo,
                            SEND_STATUS,
                            (Data->Address << 16),
                            NoData,
                            NULL,
                            0,
                            ResponseR1,
                            TIMEOUT_COMMAND,
                            (UINT32*)&(Data->CardStatus)
                            );


}


EFI_STATUS
EFIAPI
MmcDecodeCID (
  IN CID *CIDReg
  )
{
    INT32 i = 0;
    Print (L"\n");
    Print (L"\n==========DECODE MMC CID REGISTER==================\n");
    switch (CIDReg->MID) {
      case 0x15:
       Print (L" MANUFACTURER_ID     = 0x%X(SamSung part)\n",CIDReg->MID);
      break;
      case 0x45:
       Print (L" MANUFACTURER_ID     = 0x%X(SanDisk part)\n",CIDReg->MID);
       break;
      default:
       Print (L" MANUFACTURER_ID     = 0x%X\n",CIDReg->MID);
    }
    Print (L" CARD_OR_BGA         = 0x%X\n",(CIDReg->OID & 0xFF00)>>6);
    Print (L" OEM_APPLICATION_ID  = 0x%X\n",(CIDReg->OID>>8)&0xFF);
    Print (L" PRODUCT_NAME        = ");
    for (i = 5; i >=0; i--) {
       Print (L"%c",CIDReg->PNM[i]);
    }
    Print (L"\n");

    Print (L" PRODUCT_REVISION    = 0x%X\n",CIDReg->PRV);
    Print (L" PRODUCT_SERIAL_NUM  = 0x%X\n",CIDReg->PSN);
    Print (L" MANUFACTURE_DATE    = Month(%d)/Year(%d)[0x%X]\n",((CIDReg->MDT >> 4) & 0x0f), (UINT32)(CIDReg->MDT & 0x0f) + 1997UL, CIDReg->MDT);
    return 0;
}


EFI_STATUS
EFIAPI
MmcDecodeCSD (
  IN CSD * CSDReg
  )
{
  Print (L"\n");
  Print (L"\n==========DECODE MMC CSD REGISTER==================\n");
  Print (L"csd_struct        : [0x%0x] \n", CSDReg->CSD_STRUCTURE);
  Print (L"specs_ver         : [0x%0x] \n", CSDReg->SPEC_VERS);
  Print (L"reserve2          : [0x%0x] \n", CSDReg->Reserved2);
  Print (L"taac              : [0x%0x] \n", CSDReg->TAAC);
  Print (L"nsac              : [0x%0x] \n", CSDReg->NSAC);
  Print (L"tran_speed        : [0x%0x] \n", CSDReg->TRAN_SPEED);
  Print (L"ccc               : [0x%0x] \n", CSDReg->CCC);
  Print (L"read_bl_len       : [0x%0x] \n", CSDReg->READ_BL_LEN);
  Print (L"read_partial      : [0x%0x] \n", CSDReg->READ_BL_PARTIAL);
  Print (L"write_misalign    : [0x%0x] \n", CSDReg->WRITE_BLK_MISALIGN);
  Print (L"read_misalign     : [0x%0x] \n", CSDReg->READ_BLK_MISALIGN);
  Print (L"dsr_imp           : [0x%0x] \n", CSDReg->DSR_IMP);
  Print (L"reserve1          : [0x%0x] \n", CSDReg->Reserved1);
  Print (L"c_size            : [0x%0x] \n", CSDReg->C_SIZELow2 | CSDReg->C_SIZEHigh10<<2);
  Print (L"vdd_r_curr_min    : [0x%0x] \n", CSDReg->VDD_R_CURR_MIN);
  Print (L"vdd_r_curr_max    : [0x%0x] \n", CSDReg->VDD_R_CURR_MAX);
  Print (L"vdd_w_curr_min    : [0x%0x] \n", CSDReg->VDD_W_CURR_MIN);
  Print (L"vdd_w_curr_max    : [0x%0x] \n", CSDReg->VDD_W_CURR_MAX);
  Print (L"c_size_mult       : [0x%0x] \n", CSDReg->C_SIZE_MULT);
  Print (L"erase_grp_size    : [0x%0x] \n", CSDReg->ERASE_GRP_SIZE);
  Print (L"erase_grp_mult    : [0x%0x] \n", CSDReg->ERASE_GRP_MULT);
  Print (L"wp_grp_size       : [0x%0x] \n", CSDReg->WP_GRP_SIZE);
  Print (L"wp_grp_enable     : [0x%0x] \n", CSDReg->WP_GRP_ENABLE);
  Print (L"default_ecc       : [0x%0x] \n", CSDReg->DEFAULT_ECC);
  Print (L"r2w_factor        : [0x%0x] \n", CSDReg->R2W_FACTOR);
  Print (L"write_bl_len      : [0x%0x] \n", CSDReg->WRITE_BL_LEN);
  Print (L"write_partial     : [0x%0x] \n", CSDReg->WRITE_BL_PARTIAL);
  Print (L"reserve0          : [0x%0x] \n", CSDReg->Reserved0);
  Print (L"content_prot_app  : [0x%0x] \n", CSDReg->CONTENT_PROT_APP);
  Print (L"file_format_grp   : [0x%0x] \n", CSDReg->FILE_FORMAT_GRP);
  Print (L"copy              : [0x%0x] \n", CSDReg->COPY);
  Print (L"perm_write_protect: [0x%0x] \n", CSDReg->PERM_WRITE_PROTECT);
  Print (L"tmp_write_prot    : [0x%0x] \n", CSDReg->TMP_WRITE_PROTECT);
  Print (L"file_format       : [0x%0x] \n", CSDReg->FILE_FORMAT);
  Print (L"ecc               : [0x%0x] \n", CSDReg->ECC);
  Print (L"==================================================\n");

  return 0;
}


EFI_STATUS
EFIAPI
MmcDecodeExtCSD (
  EXT_CSD * ExtCSDReg
  )
{
  Print (L"\n");
  Print (L"\n==========DECODE MMC EXT CSD REGISTER==================\n");

  Print (L" SUPPORTED_CMD_SETS        = 0x%X\n", ExtCSDReg->CMD_SET);
  Print (L" HPI_FEATURES              = 0x%X\n", ExtCSDReg->HPI_FEATURES);
  Print (L" BKOPS_SUPPORT             = 0x%X\n", ExtCSDReg->BKOPS_SUPPORT);
  Print (L" BKOPS_STATUS              = 0x%X\n", ExtCSDReg->BKOPS_STATUS);
  Print (L" CORRECTLY_PRG_SECTORS_NUM = 0x%X%X%X%X\n", ExtCSDReg->CORRECTLY_PRG_SECTORS_NUM[3], \
          ExtCSDReg->CORRECTLY_PRG_SECTORS_NUM[2], ExtCSDReg->CORRECTLY_PRG_SECTORS_NUM[1], ExtCSDReg->CORRECTLY_PRG_SECTORS_NUM[0]);
  Print (L" INI_TIMEOUT_AP            = 0x%X\n", ExtCSDReg->INI_TIMEOUT_AP);
  Print (L" PWR_CL_DDR_52_195         = 0x%X\n", ExtCSDReg->PWR_CL_DDR_52_195);
  Print (L" PWR_CL_DDR_52_360         = 0x%X\n", ExtCSDReg->PWR_CL_DDR_52_360);
  Print (L" MIN_PRF_DDR_W_8_52        = 0x%X\n", ExtCSDReg->MIN_PERF_DDR_W_8_52);
  Print (L" MIN_PRF_DDR_R_8_52        = 0x%X\n", ExtCSDReg->MIN_PERF_DDR_R_8_52);
  Print (L" TRIM_MULT                 = 0x%X\n", ExtCSDReg->TRIM_MULT);
  Print (L" SEC_FEATURE_SUPP          = 0x%X\n", ExtCSDReg->SEC_FEATURE_SUPPORT);
  Print (L" SEC_ERASE_MULT            = 0x%X\n", ExtCSDReg->SEC_ERASE_MULT);
  Print (L" SEC_TRIM_MULT             = 0x%X\n", ExtCSDReg->SEC_TRIM_MULT);
  Print (L" BOOT_INFO                 = 0x%X\n", ExtCSDReg->BOOT_INFO);
  Print (L" BOOT_PART_SIZE            = 0x%X\n", ExtCSDReg->BOOT_SIZE_MULTI);
  Print (L" ACCESS_SIZE               = 0x%X\n", ExtCSDReg->ACC_SIZE);
  Print (L" HI_CAP_ER_GRP_SIZE        = 0x%X\n", ExtCSDReg->HC_ERASE_GRP_SIZE);
  Print (L" HI_CAP_ER_TIMEOUT         = 0x%X\n", ExtCSDReg->ERASE_TIMEOUT_MULT);
  Print (L" REL_WR_SECTOR_CNT         = 0x%X\n", ExtCSDReg->REL_WR_SEC_C);
  Print (L" HI_CAP_WP_GRP_SIZE        = 0x%X\n", ExtCSDReg->HC_WP_GRP_SIZE);
  Print (L" SLEEP_CURRENT_VCC         = 0x%X\n", ExtCSDReg->S_C_VCC);
  Print (L" SLEEP_CURRENT_VCCQ        = 0x%X\n", ExtCSDReg->S_C_VCCQ);
  Print (L" SLP_AWK_TIMEOUT           = 0x%X\n", ExtCSDReg->S_A_TIMEOUT);
  Print (L" SECTOR_COUNT              = 0x%X\n", *(UINT32*)((UINT8 *)&ExtCSDReg->SEC_COUNT));
  Print (L" MIN_PERF_W_8_52           = 0x%X\n", ExtCSDReg->MIN_PERF_W_8_52);
  Print (L" MIN_PERF_R_8_52           = 0x%X\n", ExtCSDReg->MIN_PERF_R_8_52);
  Print (L" MIN_PERF_W_8_26_4_52      = 0x%X\n", ExtCSDReg->MIN_PERF_W_8_26_4_52);
  Print (L" MIN_PERF_W_8_26_4_52      = 0x%X\n", ExtCSDReg->MIN_PERF_W_8_26_4_52);
  Print (L" MIN_PERF_W_4_26           = 0x%X\n", ExtCSDReg->MIN_PERF_W_4_26);
  Print (L" MIN_PERF_R_4_26           = 0x%X\n", ExtCSDReg->MIN_PERF_R_4_26);
  Print (L" PWR_CLASS_26_360          = 0x%X\n", ExtCSDReg->PWR_CL_26_360);
  Print (L" PWR_CLASS_52_360          = 0x%X\n", ExtCSDReg->PWR_CL_52_360);
  Print (L" PWR_CLASS_26_195          = 0x%X\n", ExtCSDReg->PWR_CL_26_195);
  Print (L" PWR_CLASS_52_195          = 0x%X\n", ExtCSDReg->PWR_CL_52_195);
  Print (L" PARTITION_SWITCH_TIME     = 0x%X\n", ExtCSDReg->PARTITION_SWITCH_TIME);
  Print (L" OUT_OF_INTERRUPT_TIME     = 0x%X\n", ExtCSDReg->OUT_OF_INTERRUPT_TIME);
  Print (L" CARD_TYPE                 = 0x%X\n", ExtCSDReg->CARD_TYPE);
  Print (L" CSD_STRUCTURE             = 0x%X\n", ExtCSDReg->CSD_STRUCTURE);
  Print (L" EXT_CSD_REV               = 0x%X\n", ExtCSDReg->EXT_CSD_REV);
  Print (L" CMD_SET                   = 0x%X\n", ExtCSDReg->CMD_SET);
  Print (L" CMD_SET_REV               = 0x%X\n", ExtCSDReg->CMD_SET_REV);
  Print (L" PWR_CLASS                 = 0x%X\n", ExtCSDReg->POWER_CLASS);
  Print (L" HI_SPEED_TIMING           = 0x%X\n", ExtCSDReg->HS_TIMING);
  Print (L" BUS_WIDTH_MODE            = 0x%X\n", ExtCSDReg->BUS_WIDTH);
  Print (L" ERASED_MEM_CONTENT        = 0x%X\n", ExtCSDReg->ERASED_MEM_CONT);
  Print (L" PARTITION_CONFIG          = 0x%X\n", ExtCSDReg->PARTITION_CONFIG);
  Print (L" BOOT_CONFIG_PROT          = 0x%X\n", ExtCSDReg->BOOT_CONFIG_PROT);
  Print (L" BOOT_BUS_WIDTH            = 0x%X\n", ExtCSDReg->BOOT_BUS_WIDTH);
  Print (L" HI_DEN_ER_GRP_DEF         = 0x%X\n", ExtCSDReg->ERASE_GROUP_DEF);
  Print (L" BOOT_WP                   = 0x%X\n", ExtCSDReg->BOOT_WP);
  Print (L" USER_WP                   = 0x%X\n", ExtCSDReg->USER_WP);
  Print (L" FW_CONFIG                 = 0x%X\n", ExtCSDReg->FW_CONFIG);
  Print (L" RPMB_SIZE_MULT            = 0x%X\n", ExtCSDReg->RPMB_SIZE_MULT);
  Print (L" RST_N_FUNCTION            = 0x%X\n", ExtCSDReg->RST_n_FUNCTION);
  Print (L" PARTITIONING_SUPP         = 0x%X\n", ExtCSDReg->PARTITIONING_SUPPORT);
  Print (L" MAX_ENH_SIZE_MULT         = 0x%02X%02X%02X\n", ExtCSDReg->MAX_ENH_SIZE_MULT[2], ExtCSDReg->MAX_ENH_SIZE_MULT[1], ExtCSDReg->MAX_ENH_SIZE_MULT[0]);
  Print (L" PART_ATTRIBUTE            = 0x%X\n", ExtCSDReg->PARTITIONS_ATTRIBUTES);
  Print (L" PART_SETTING_COMP         = 0x%X\n", ExtCSDReg->PARTITION_SETTING_COMPLETED);
  Print (L" GP_SIZE_MULT              = 0x%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\n", ExtCSDReg->GP_SIZE_MULT_4[2], ExtCSDReg->GP_SIZE_MULT_4[1], ExtCSDReg->GP_SIZE_MULT_4[0],
                    ExtCSDReg->GP_SIZE_MULT_3[2], ExtCSDReg->GP_SIZE_MULT_3[1], ExtCSDReg->GP_SIZE_MULT_3[0],
                    ExtCSDReg->GP_SIZE_MULT_2[2], ExtCSDReg->GP_SIZE_MULT_2[1], ExtCSDReg->GP_SIZE_MULT_2[0],
                    ExtCSDReg->GP_SIZE_MULT_1[2], ExtCSDReg->GP_SIZE_MULT_1[1], ExtCSDReg->GP_SIZE_MULT_1[0]);
  Print (L" ENH_SIZE_MULT             = 0x%02X%02X%02X\n", ExtCSDReg->ENH_SIZE_MULT[2], ExtCSDReg->ENH_SIZE_MULT[1], ExtCSDReg->ENH_SIZE_MULT[0]);
  Print (L" ENH_START_ADDR            = 0x%02X%02X%02X%02X\n", ExtCSDReg->ENH_START_ADDR[3], ExtCSDReg->ENH_START_ADDR[2], ExtCSDReg->ENH_START_ADDR[1], ExtCSDReg->ENH_START_ADDR[0]);
  Print (L" SEC_BAD_BLK_MGMNT         = 0x%X\n", ExtCSDReg->SEC_BAD_BLOCK_MGMNT);
  Print (L"==================================================\n");

	return 0;
}


EFI_STATUS
EFIAPI
MmcDecodeOCR (
  IN OCR *OCRReg
  )
{
    Print (L"\n");
    Print (L"\n==========DECODE MMC OCR REGISTER==================\n");
    Print (L" OCR = 0x%08X\n", *((UINT32 *)OCRReg));
    Print (L" CARD_NOT_BUSY      = 0x%X\n", OCRReg->Busy);
    Print (L" ACCESS_MODE        = 0x%X\n", OCRReg->AccessMode);
    Print (L" VDD_270_360        = 0x%X\n", OCRReg->V270_V360);
    Print (L" VDD_200_260        = 0x%X\n", OCRReg->V200_V260);
    Print (L" VDD_170_195        = 0x%X\n", OCRReg->V170_V195);

    return 0;
}

EFI_STATUS 

MmcGetOCR(
             IN EmmcCardData *Data
             
            )
{

    EFI_STATUS                 Status = 0;
    UINT32                     TimeOut=5000;
    //
    // Go to Idle
    //
    CHK_FUNC_CALL(SendCmd0(Data), "CMD0", TRUE);

    // Delay to make sure card is in idle
    gBS->Stall (500 * 1000);

    //
    // Get OCR register to check voltage support, first time the OCR is 0x40FF8080
    //
    CHK_FUNC_CALL(SendCmd1(Data, 0x40FF8080), "CMD1", TRUE);

    while (Data->OCRReg.Busy != 1) {
        Data->OCRReg.AccessMode = 0x02; // sector mode;
        CHK_FUNC_CALL(SendCmd1(Data, *(UINT32*)&(Data->OCRReg)), "CMD1", TRUE);
          
        gBS->Stall(1 * 100);

        TimeOut--;
        if (TimeOut == 0) {
            DEBUG((EFI_D_ERROR, "Card is always in busy state\n"));
            Status = EFI_TIMEOUT;
            goto Exit;
        } 

    } 
    
   //MmcDecodeOCR(&Data->OCRReg);     
   //Print(L"OCR=0x%x\n", *(UINT32*)&(Data->OCRReg));

Exit:
    return Status;
}


EFI_STATUS 
MmcGetCID(
             IN EmmcCardData *Data
            )
{

    CHK_FUNC_CALL(SendCmd2(Data), NULL, TRUE);
    //MmcDecodeCID(&Data->CIDReg);
    return 0;
}

EFI_STATUS 
MmcGetCSD(
             IN EmmcCardData *Data
            )
{
    //
    // Get CSD Register
    //
    CHK_FUNC_CALL(SendCmd9(Data), NULL, TRUE);
    //MmcDecodeCSD(&Data->CSDReg);


    return EFI_SUCCESS;
}

EFI_STATUS 
MmcGetExtCSD(
             IN EmmcCardData *Data
            )
{
    //
    // Get Ext CSD Register
    //
    CHK_FUNC_CALL(SendCmd8(Data), NULL, TRUE);

    CopyMem (&(Data->ExtCSDReg), Data->AlignedBuffer, sizeof (EXT_CSD));
    //MmcDecodeExtCSD(&Data->ExtCSDReg);
    return EFI_SUCCESS;
}
EFI_STATUS
MmcSelect (
  IN EmmcCardData *Data,
  IN BOOLEAN       Select)
{
  UINT32  Address = Data->Address;
  //
  //Put the card into tran state
  //
  if (!Select)
    Address = 0;
  
  CHK_FUNC_CALL(SendCmd7(Data, Address), "CMD7", TRUE);
  return EFI_SUCCESS;
}


EFI_STATUS
MmcUpdateCardStatus (
  IN EmmcCardData *Data
  )
{
  CHK_FUNC_CALL(SendCmd13(Data), "CMD13", TRUE);
  return EFI_SUCCESS;

}


EFI_STATUS
MmcMoveToTranState (
  IN EmmcCardData *Data
  )
{
  EFI_STATUS Status;

  Status = EFI_SUCCESS;

  if (Data->CardStatus.CURRENT_STATE != Tran_STATE) {
    //
    // Put the card into tran state
    //
    Status = MmcSelect (Data, TRUE);
    DEBUG((EFI_D_INFO, "MmcMoveToTranState: CMD7 -> %r\n", Status));
    MmcUpdateCardStatus (Data);
  }

  if (Data->CardStatus.CURRENT_STATE != Tran_STATE) {
    DEBUG((EFI_D_ERROR, "MmcMoveToTranState: Unable to put card into tran state\n"));
    return EFI_DEVICE_ERROR;
  }

  return Status;
}


EFI_STATUS
MmcReadExtCsd (
  IN EmmcCardData *Data
  )
{
  EFI_STATUS Status;

  Status = MmcMoveToTranState (Data);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  CHK_FUNC_CALL( MmcGetExtCSD(Data), "MmcReadExtCsd", TRUE);

  return Status;
}


EFI_STATUS
MmcSetExtCsd8 (
  IN EmmcCardData            *Data,
  IN  UINT8                  Index,
  IN  UINT8                  Value
  )
{
  EFI_STATUS                 Status;
  SWITCH_ARGUMENT            SwitchArgument;

  Status = MmcMoveToTranState (Data);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  ZeroMem(&SwitchArgument, sizeof (SWITCH_ARGUMENT));
  SwitchArgument.CmdSet = 0;
  SwitchArgument.Value  = (UINT8) Value;
  SwitchArgument.Index  = (UINT8) Index;
  SwitchArgument.Access = WriteByte_Mode; // SetBits_Mode;
  return SendCmd6 (Data, SwitchArgument);
}


EFI_STATUS
MmcSetExtCsd24 (
  IN EmmcCardData            *Data,
  IN  UINT8                  Index,
  IN  UINT32                 Value
  )
{
  EFI_STATUS                 Status;
  UINTN                      Loop;
  
  Status = EFI_SUCCESS;
  ASSERT ((Value & 0xff000000ULL) == 0);

  for (Loop = 0; Loop < 3; Loop++) {
    Status = MmcSetExtCsd8 (Data, Index + (UINT8)Loop, (UINT8)Value & 0xff);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    Value = Value >> 8;
  }

  return Status;
}


UINT8
MmcGetExtCsd8 (
  IN EmmcCardData                     *Data,
  IN UINTN                            Offset
  )
{
  ASSERT (Offset < sizeof (Data->ExtCSDReg));
  return ((UINT8*)&Data->ExtCSDReg)[Offset];
}


UINT32
MmcGetExtCsd32 (
  IN EmmcCardData                     *Data,
  IN UINTN                            Offset
  )
{
  return *(UINT32*) (((UINT8*)&Data->ExtCSDReg) + Offset);
}


UINT32
MmcGetExtCsd24 (
  IN EmmcCardData                    *Data,
  IN UINTN                            Offset
  )
{
  return MmcGetExtCsd32 (Data, Offset) & 0xffffff;
}


UINT32 EmmcGetGeneralPartitionSize(  IN EmmcCardData *Data, UINT32 Num)
{
    UINT32 size = 0;
    EXT_CSD *ExtCSDReg = &Data->ExtCSDReg;
    if(0 == Num) {
        Print(L"The number of General Purpose Partition must be from 1 to 4.\n");
        return 0;
    }


    if(Num <= MAX_GPP) {
        switch (Num) {
            case 1:
            size = ((ExtCSDReg->GP_SIZE_MULT_1[2] *( 0x1<<16)) + ((ExtCSDReg->GP_SIZE_MULT_1[1] *( 0x1<< 8)))  + ExtCSDReg->GP_SIZE_MULT_1[0]) * ExtCSDReg->HC_WP_GRP_SIZE* ExtCSDReg->HC_ERASE_GRP_SIZE* 512 *1024;
            break;
            case 2:
            size = ((ExtCSDReg->GP_SIZE_MULT_2[2] *( 0x1<<16)) + ((ExtCSDReg->GP_SIZE_MULT_2[1] *( 0x1<< 8)))  + ExtCSDReg->GP_SIZE_MULT_2[0]) * ExtCSDReg->HC_WP_GRP_SIZE* ExtCSDReg->HC_ERASE_GRP_SIZE* 512 *1024;
            break;
            case 3:
            size = ((ExtCSDReg->GP_SIZE_MULT_3[2] *( 0x1<<16)) + ((ExtCSDReg->GP_SIZE_MULT_3[1] *( 0x1<< 8)))  + ExtCSDReg->GP_SIZE_MULT_3[0]) * ExtCSDReg->HC_WP_GRP_SIZE* ExtCSDReg->HC_ERASE_GRP_SIZE* 512 *1024;
            break;
            case 4:
            size = ((ExtCSDReg->GP_SIZE_MULT_4[2] *( 0x1<<16)) + ((ExtCSDReg->GP_SIZE_MULT_4[1] *( 0x1<< 8)))  + ExtCSDReg->GP_SIZE_MULT_4[0]) * ExtCSDReg->HC_WP_GRP_SIZE* ExtCSDReg->HC_ERASE_GRP_SIZE* 512 *1024;
            break;

            break;
            default:
                return 0;
            }
        }
    return size;
}


UINT64
EFIAPI
EmmcGetFlashSize (
    IN EmmcCardData *Data

  )
{
  UINT64 FlashSize;
  UINT32 CSize;
  UINT32 CSizeMult;

  CSize = Data->CSDReg.C_SIZELow2 | Data->CSDReg.C_SIZEHigh10 << 2 ;
  CSizeMult = Data->CSDReg.C_SIZE_MULT;
  if ((CSize == 0xFFF) && (CSizeMult == 0x7))
  {
    //
    // Per eMMC spec, this is for eMMC larger than 2GB
    //
    // Device density = SEC_COUNT x 512B
    //
    FlashSize = (UINT64)(*(UINT32*)((UINT8 *)&(Data->ExtCSDReg.SEC_COUNT))) * HOST_BLK_SIZE;
  }
  else
  {
    //
    // For eMMC less than 2GB
    //
    // Memory capacity = BLOCKNR * BLOCK_LEN where BLOCKNR = (C_SIZE+1) * MULT
    // 
    // MULT = 2 ^ (C_SIZE_MULT+2)
    //
    // BLOCK_LEN = 2 ^ READ_BL_LEN
    //
    FlashSize = (CSize + 1) * (1 << (CSizeMult + 2)) * (1 << Data->CSDReg.READ_BL_LEN);
  }
  
  return FlashSize;
}


UINT32 
EmmcGetBootPartitionSize (
  IN EmmcCardData *Data
  )
{
    return Data->ExtCSDReg.BOOT_SIZE_MULTI * 128;
}


UINT32 
EmmcGetRPMBPartitionSize (
  IN EmmcCardData *Data
  )
{
    return Data->ExtCSDReg.RPMB_SIZE_MULT* 128;
}


UINT32 
EmmcGetAccessingPartition (
  IN EmmcCardData *Data
  )
{
    UINT32 Part = 0;
    Part = (UINT32)(Data->ExtCSDReg.PARTITION_CONFIG & 0x3);
    switch (Part) {
      case 0:
        Print(L"User parttition is Accessable now\n");
        break;
      case 1:
        Print(L"Boot parttition 1 is Accessable now\n");
        break;
      case 2:
        Print(L"Boot parttition 2 is Accessable now\n");
        break;
       case 3:
        Print(L"RPMB parttition  is Accessable now\n");
        break;
        case 4:
        Print(L"GP parttition 1 is Accessable now\n");
        break;
        case 5:
        Print(L"GP parttition 2 is Accessable now\n");
        break;
        case 6:
        Print(L"GP parttition 3 is Accessable now\n");
        break;
        case 7:
        Print(L"GP parttition 4 is Accessable now\n");
        break;
      default:
        break;
    }
    return 0;
}


UINT64 EmmcGetUsrEnhStatAddr(  IN EmmcCardData *Data)
{
    UINT64 addr = 0;
    addr = (Data->ExtCSDReg.ENH_START_ADDR[3] <<24) + (Data->ExtCSDReg.ENH_START_ADDR[2] <<16) + (Data->ExtCSDReg.ENH_START_ADDR[1] <<8) + Data->ExtCSDReg.ENH_START_ADDR[0];
    if(!(0xfff == (Data->CSDReg.C_SIZELow2 | Data->CSDReg.C_SIZEHigh10<<2)&& 0x7 == Data->CSDReg.C_SIZE_MULT))
        return addr;
    else
        return (addr << 9);
}


UINT64 
EmmcGetUserMaxEnhSize (
  IN EmmcCardData *Data
  )
{
  UINT64 MaxEnhSize;
  
  MaxEnhSize = ((Data->ExtCSDReg.MAX_ENH_SIZE_MULT [2] << 16) + (Data->ExtCSDReg.MAX_ENH_SIZE_MULT [1] << 8) + (Data->ExtCSDReg.MAX_ENH_SIZE_MULT[0])) * \
               Data->ExtCSDReg.HC_WP_GRP_SIZE * \
               Data->ExtCSDReg.HC_ERASE_GRP_SIZE * \
               512;
  
  return MaxEnhSize;
}


VOID
EmmcGetCardRevision (
 IN EmmcCardData *Data
 )
{

  switch (Data->ExtCSDReg.EXT_CSD_REV) {
    case 0:
      Print (L"Revision 1.0 (for MMC v4.0)\n");
      break;

    case 1:
      Print (L"Revision 1.1 (for MMC v4.1)\n");
      break;

    case 2:
      Print (L"Revision 1.2 (for MMC v4.2)\n");
      break;

    case 3:
      Print (L"Revision 1.3 (for MMC v4.3)\n");
      break;

    case 4:
      Print (L"Revision 1.4 (Obsolete)\n");
      break;

    case 5:
      Print (L"Revision 1.5 (for MMC v4.41)\n");
      break;

    case 6:
      Print (L"Revision 1.6 (for MMC v4.5)\n");
      break;

    default:
      Print (L"Revision error\n");
      break;
  }
}

EFI_STATUS
MMCCardSetBusWidth (
    IN EmmcCardData *Data,
    IN  UINT8       BusWidth
  )
/*++

  Routine Description:
    This function set the bus and device width for MMC card
  
  Arguments:
    CardData    - Pointer to CARD_DATA   
    BusWidth    - 1, 4, 8 bits
  Returns:  
    EFI_INVALID_PARAMETER
    EFI_UNSUPPORTED
    EFI_SUCCESS
 --*/   
{
  EFI_STATUS                 Status;
  EFI_SD_HOST_IO_PROTOCOL    *SdHostIo;
  SWITCH_ARGUMENT            SwitchArgument;
  UINT8                      Value;

  SdHostIo = Data->SdHostIo;
  Value = 0;
  switch (BusWidth) {
    case 28:	//20 in 28 indicates DDR in 8 bit bus
      Value = 6;
      break;
          
    case 24:    //20 in 24 indicates DDR in 4 bit bus
      Value = 5;
      break;

    case 8:
      Value = 2;
      break; 

    case 4:
      Value = 1;
      break; 

    case 1:
      Value = 0;
      break; 

    default:
     ASSERT(0);
  }
  
  //HS_TIMING must be set to 0x1 before setting BUS_WIDTH for dual data rate operation (values 5 or 6)
  if(Value == 5 || Value == 6 ){
    ZeroMem(&SwitchArgument, sizeof (SWITCH_ARGUMENT));
    SwitchArgument.CmdSet = 0;
    SwitchArgument.Value  = 0x1;
    SwitchArgument.Index  = (UINT32)((UINTN)
    (&(Data->ExtCSDReg.HS_TIMING)) - (UINTN)(&(Data->ExtCSDReg)));
    SwitchArgument.Access = WriteByte_Mode;
    Status  = SendCmd6(Data, SwitchArgument);
    if (!EFI_ERROR (Status)) {
        Status  = SendCmd13(Data);
    }else{
      DEBUG((EFI_D_ERROR, "SWITCH Fail in HS Timing setting\n"));
    }
  }

  ZeroMem(&SwitchArgument, sizeof (SWITCH_ARGUMENT));
  SwitchArgument.CmdSet = 0;
  SwitchArgument.Value  = Value;
  SwitchArgument.Index  = (UINT32)((UINTN)
  (&(Data->ExtCSDReg.BUS_WIDTH)) - (UINTN)(&(Data->ExtCSDReg)));
  SwitchArgument.Access = WriteByte_Mode;
  Status  = SendCmd6(Data, SwitchArgument);

  if (!EFI_ERROR (Status)) {
      Status  = SendCmd13(Data);
    if (EFI_ERROR (Status)) {
      DEBUG((EFI_D_ERROR, "SWITCH %d bits Fail\n", BusWidth));
      goto Exit;
    } else {
      if ((BusWidth == 24) || (BusWidth == 28)){
          Status = SdHostIo->SetBusWidth (SdHostIo, BusWidth - 20);
      }
      else {
          Status = SdHostIo->SetBusWidth (SdHostIo, BusWidth);
      }
      if (EFI_ERROR (Status)) {
         goto Exit;
      }
      if ((BusWidth == 28) ||
          (BusWidth == 24)) { //20 in 28/24->DDR Mode is supported
        Status = SdHostIo->SetHostDdrMode(SdHostIo, TRUE);
      }
      gBS->Stall (5 * 1000);
    }
  }
  if ((BusWidth == 24) || (BusWidth == 28)){
      //Status = MMCCardBusWidthTest (CardData, BusWidth - 20); //Illegal commands
      //goto Exit;
  } 
  else {
     // Status = MMCCardBusWidthTest (CardData, BusWidth);
  }
  if (EFI_ERROR (Status)) {
    DEBUG((EFI_D_ERROR, "MMCCardBusWidthTest %d bit Fail\n", BusWidth));
    goto Exit;
  }

  Data->CurrentBusWidth = BusWidth;
  DEBUG((EFI_D_ERROR, "Data->CurrentBusWidth -> %d\n", (UINTN)Data->CurrentBusWidth));

Exit:
  return Status;
}


EFI_STATUS
EFIAPI
EmmcReadBlocks (
  IN EmmcCardData             *CardData,
  IN  UINT32                  MediaId,
  IN  EFI_LBA                 LBA,
  IN  UINTN                   BufferSize,
  OUT VOID                    *Buffer
  )
/*++

  Routine Description:
    Implements EFI_BLOCK_IO_PROTOCOL.ReadBlocks() function.

  Arguments:
    This     The EFI_BLOCK_IO_PROTOCOL instance.
    MediaId  The media id that the read request is for.
    LBA      The starting logical block address to read from on the device.
    BufferSize
              The size of the Buffer in bytes. This must be a multiple of
              the intrinsic block size of the device.
    Buffer    A pointer to the destination buffer for the data. The caller
              is responsible for either having implicit or explicit ownership
              of the buffer.

  Returns:
    EFI_INVALID_PARAMETER - Parameter is error
    EFI_SUCCESS           - Success
    EFI_DEVICE_ERROR      - Hardware Error
    EFI_NO_MEDIA          - No media
    EFI_MEDIA_CHANGED     - Media Change
    EFI_BAD_BUFFER_SIZE   - Buffer size is bad
 --*/
{
  EFI_STATUS                  Status;
  UINT32                      Address;
  EFI_SD_HOST_IO_PROTOCOL     *SdHostIo;
  UINT32                      RemainingLength;
  UINT32                      TransferLength;
  UINT8                       *BufferPointer;
  BOOLEAN                     SectorAddressing;
  UINT64                      CardSize;


  DEBUG((EFI_D_INFO, " ReadBlocks ...\n"));

  Status   = EFI_SUCCESS;
  SdHostIo = CardData->SdHostIo;

  DEBUG((EFI_D_INFO,
    "EmmcReadBlocks: Read(PART=%d, LBA=0x%08lx, Buffer=0x%08x, Size=0x%08x)\n",
    0, LBA, Buffer, BufferSize
    ));

//  Status = MmcSelectPartition (Partition);
//  if (EFI_ERROR (Status)) {
//    return Status;
//  }

  CardSize = MultU64x32 (CardData->BlockNumber, CardData->BlockLen);

  if ((CardData->CardType == SDMemoryCard2High) || (CardSize >= SIZE_2GB)) {
    SectorAddressing = TRUE;
  } else {
    SectorAddressing = FALSE;
  }

  if (SectorAddressing) {
    //
    // Sector Address
    //
    Address = (UINT32)DivU64x32 (MultU64x32 (LBA, CardData->BlockLen), 512);
  } else {
    //
    //Byte Address
    //
    Address  = (UINT32)MultU64x32 (LBA, CardData->BlockLen);
  }


  if (!Buffer) {
    Status = EFI_INVALID_PARAMETER;
    DEBUG ((EFI_D_ERROR, "EmmcReadBlocks: Invalid parameter \n"));
    goto Done;
  }

  if ((BufferSize % CardData->BlockLen) != 0) {
    Status = EFI_BAD_BUFFER_SIZE;
    DEBUG ((EFI_D_ERROR, "EmmcReadBlocks: Bad buffer size \n"));
    goto Done;
  }

  if (BufferSize == 0) {
    Status = EFI_SUCCESS;
    goto Done;
  }

  BufferPointer   = Buffer;
  RemainingLength = (UINT32) BufferSize;

  while (RemainingLength > 0) {

    if ((BufferSize > CardData->BlockLen)) {
      if (RemainingLength > SdHostIo->HostCapability.BoundarySize) {
        TransferLength = SdHostIo->HostCapability.BoundarySize;
      } else {
        TransferLength = RemainingLength;
      }

      if (CardData->CardType == MMCCard) {
        if (!(CardData->ExtCSDReg.CARD_TYPE & (BIT2 | BIT3))) {
           Status = SendCommand (
                       SdHostIo,
                       SET_BLOCKLEN,
                       CardData->BlockLen,
                       NoData,
                       NULL,
                       0,
                       ResponseR1,
                       TIMEOUT_COMMAND,
                       (UINT32*)&(CardData->CardStatus)
                       );
        
    
            if (EFI_ERROR (Status)) {
              break;
            }
        }
            Status = SendCommand (
                       SdHostIo,
                       SET_BLOCK_COUNT,
                       TransferLength / CardData->BlockLen,
                       NoData,
                       NULL,
                       0,
                       ResponseR1,
                       TIMEOUT_COMMAND,
                       (UINT32*)&(CardData->CardStatus)
                       );
            if (EFI_ERROR (Status)) {
              break;

        }
      }

      Status = SendCommand (
                 SdHostIo,
                 READ_MULTIPLE_BLOCK,
                 Address,
                 InData,
                 CardData->AlignedBuffer,
                 TransferLength,
                 ResponseR1,
                 TIMEOUT_DATA,
                 (UINT32*)&(CardData->CardStatus)
                 );

      if (EFI_ERROR (Status)) {
        DEBUG ((EFI_D_ERROR, "EmmcReadBlocks: READ_MULTIPLE_BLOCK -> Fail\n"));
        break;
      }
    } else {
      if (RemainingLength > CardData->BlockLen) {
        TransferLength = CardData->BlockLen;
      } else {
        TransferLength = RemainingLength;
      }

      Status = SendCommand (
                 SdHostIo,
                 READ_SINGLE_BLOCK,
                 Address,
                 InData,
                 CardData->AlignedBuffer,
                 (UINT32)TransferLength,
                 ResponseR1,
                 TIMEOUT_DATA,
                 (UINT32*)&(CardData->CardStatus)
                 );
      if (EFI_ERROR (Status)) {
        DEBUG ((EFI_D_ERROR, "EmmcReadBlocks: READ_SINGLE_BLOCK -> Fail\n"));
        break;
      }
    }

    CopyMem (BufferPointer, CardData->AlignedBuffer, TransferLength);

    if (SectorAddressing) {
      //
      // Sector Address
      //
      Address += TransferLength / 512;
    } else {
      //
      //Byte Address
      //
      Address += TransferLength;
    }
    BufferPointer   += TransferLength;
    RemainingLength -= TransferLength;
  }

  if (EFI_ERROR (Status)) {

       SendCommand (
         SdHostIo,
         STOP_TRANSMISSION,
         0,
         NoData,
         NULL,
         0,
         ResponseR1,
         TIMEOUT_COMMAND,
         (UINT32*)&(CardData->CardStatus)
         );
    

  }


Done:
  DEBUG((EFI_D_INFO, "EmmcReadBlocks: Status = 0x%x\n", Status));
  return Status;
}


EFI_STATUS
EFIAPI
EmmcWriteBlocks (
  IN EmmcCardData             *CardData,
  IN  UINT32                  MediaId,
  IN  EFI_LBA                 LBA,
  IN  UINTN                   BufferSize,
  IN  VOID                    *Buffer
  )
/*++

  Routine Description:
    Implements EFI_BLOCK_IO_PROTOCOL.WriteBlocks() function.

  Arguments:
    This     The EFI_BLOCK_IO_PROTOCOL instance.
    MediaId  The media id that the write request is for.
    LBA      The starting logical block address to be written.
             The caller is responsible for writing to only
             legitimate locations.
    BufferSize
              The size of the Buffer in bytes. This must be a multiple of
              the intrinsic block size of the device.
    Buffer    A pointer to the source buffer for the data. The caller
              is responsible for either having implicit or explicit ownership
              of the buffer.

  Returns:
    EFI_INVALID_PARAMETER - Parameter is error
    EFI_SUCCESS           - Success
    EFI_DEVICE_ERROR      - Hardware Error
    EFI_NO_MEDIA          - No media
    EFI_MEDIA_CHANGED     - Media Change
    EFI_BAD_BUFFER_SIZE   - Buffer size is bad

--*/
{
  EFI_STATUS                  Status;
  UINT32                      Address;
  EFI_SD_HOST_IO_PROTOCOL     *SdHostIo;
  UINT32                      RemainingLength;
  UINT32                      TransferLength;
  UINT8                       *BufferPointer;
  BOOLEAN                     SectorAddressing;
  UINT64                      CardSize;

  DEBUG((EFI_D_INFO, "EmmcWriteBlocks: WriteBlocks ...\n"));

  Status   = EFI_SUCCESS;
  SdHostIo = CardData->SdHostIo;

  DEBUG((EFI_D_INFO,
    "EmmcWriteBlocks: Write (PART=%d, LBA=0x%08lx, Buffer=0x%08x, Size=0x%08x)\n",
    0, LBA, Buffer, BufferSize
    ));

  //Status = MmcSelectPartition (Partition);
  //if (EFI_ERROR (Status)) {
   // return Status;
  //}

  CardSize = MultU64x32 (CardData->BlockNumber, CardData->BlockLen);

  if ((CardData->CardType == SDMemoryCard2High) || (CardSize >= SIZE_2GB)) {
    SectorAddressing = TRUE;
  } else {
    SectorAddressing = FALSE;
  }

  if (SectorAddressing) {
    //
    // Sector Address
    //
    Address = (UINT32)DivU64x32 (MultU64x32 (LBA, CardData->BlockLen), 512);
  } else {
    //
    //Byte Address
    //
    Address = (UINT32)MultU64x32 (LBA, CardData->BlockLen);
  }

  if (!Buffer) {
    Status = EFI_INVALID_PARAMETER;
    DEBUG ((EFI_D_ERROR, "EmmcWriteBlocks: Invalid parameter \n"));
    goto Done;
  }

  if ((BufferSize % CardData->BlockLen) != 0) {
    Status = EFI_BAD_BUFFER_SIZE;
    DEBUG ((EFI_D_ERROR, "EmmcWriteBlocks: Bad buffer size \n"));
    goto Done;
  }

  if (BufferSize == 0) {
    Status = EFI_SUCCESS;
    goto Done;
  }


  BufferPointer   = Buffer;
  RemainingLength = (UINT32) BufferSize;

  while (RemainingLength > 0) {

    if ((BufferSize > CardData->BlockLen) ) {
      if (RemainingLength > SdHostIo->HostCapability.BoundarySize) {
        TransferLength = SdHostIo->HostCapability.BoundarySize;
      } else {
        TransferLength = RemainingLength;
      }

       if (CardData->CardType == MMCCard) {

        if (!(CardData->ExtCSDReg.CARD_TYPE & (BIT2 | BIT3)))  {
            Status = SendCommand (
                       SdHostIo,
                       SET_BLOCKLEN,
                       CardData->BlockLen,
                       NoData,
                       NULL,
                       0,
                       ResponseR1,
                       TIMEOUT_COMMAND,
                       (UINT32*)&(CardData->CardStatus)
                       );
    
            if (EFI_ERROR (Status)) {
              break;
            }
        }
        Status = SendCommand (
                   SdHostIo,
                   SET_BLOCK_COUNT,
                   TransferLength / CardData->BlockLen,
                   NoData,
                   NULL,
                   0,
                   ResponseR1,
                   TIMEOUT_COMMAND,
                   (UINT32*)&(CardData->CardStatus)
                   );
        if (EFI_ERROR (Status)) {
          break;
        }
      }

      CopyMem (CardData->AlignedBuffer, BufferPointer, TransferLength);

      Status = SendCommand (
                 SdHostIo,
                 WRITE_MULTIPLE_BLOCK,
                 Address,
                 OutData,
                 CardData->AlignedBuffer,
                 (UINT32)TransferLength,
                 ResponseR1,
                 TIMEOUT_DATA,
                 (UINT32*)&(CardData->CardStatus)
                 );
      if (EFI_ERROR (Status)) {
        DEBUG ((EFI_D_ERROR, "EmmcWriteBlocks: WRITE_MULTIPLE_BLOCK -> Fail\n"));
        break;
      }
    } else {
      if (RemainingLength > CardData->BlockLen) {
        TransferLength = CardData->BlockLen;
      } else {
        TransferLength = RemainingLength;
      }

      CopyMem (CardData->AlignedBuffer, BufferPointer, TransferLength);

      Status = SendCommand (
                 SdHostIo,
                 WRITE_BLOCK,
                 Address,
                 OutData,
                 CardData->AlignedBuffer,
                 (UINT32)TransferLength,
                 ResponseR1,
                 TIMEOUT_DATA,
                 (UINT32*)&(CardData->CardStatus)
                 );
    }

    if (SectorAddressing) {
      //
      // Sector Address
      //
      Address += TransferLength / 512;
    } else {
      //
      //Byte Address
      //
      Address += TransferLength;
    }
    BufferPointer   += TransferLength;
    RemainingLength -= TransferLength;
  }

  if (EFI_ERROR (Status)) {
    SendCommand (
      SdHostIo,
      STOP_TRANSMISSION,
      0,
      NoData,
      NULL,
      0,
      ResponseR1b,
      TIMEOUT_COMMAND,
      (UINT32*)&(CardData->CardStatus)
      );

  }


Done:
//DEBUG((EFI_D_ERROR, "  Status = %r\n", Status));
  return Status;
}

UINTN
MmcGetCurrentPartitionNum (
  IN  EmmcCardData              *CardData
  )
{
  return MmcGetExtCsd8 (
           CardData,
           OFFSET_OF (EXT_CSD, PARTITION_CONFIG)
           ) & 0x7;
}



EFI_STATUS
MmcSelectPartitionNum (
  IN  EmmcCardData              *CardData,
  IN  UINT8                  Partition
  )
{
  EFI_STATUS  Status;
  UINTN       Offset;
  UINT8       *ExtByte;
  UINTN       CurrentPartition;

  if (Partition > 7) {
    return EFI_INVALID_PARAMETER;
  }
  
  CurrentPartition = MmcGetCurrentPartitionNum (CardData);
  if (Partition == CurrentPartition) {
    return EFI_SUCCESS;
  }

  Status = MmcReadExtCsd (CardData);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  DEBUG ((EFI_D_INFO,
    "MmcSelectPartitionNum: Switch partition: %d => %d\n",
    CurrentPartition,
    Partition
    ));

  Offset = OFFSET_OF (EXT_CSD, PARTITION_CONFIG);
  Status = MmcSetExtCsd8 (CardData, (UINT8)Offset, Partition);

#if 1
  Status = MmcReadExtCsd (CardData);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  CurrentPartition = MmcGetCurrentPartitionNum (CardData);
  if (Partition != CurrentPartition) {
    DEBUG ((EFI_D_INFO, "MmcSelectPartitionNum: Switch partition failed!\n"));
    return EFI_DEVICE_ERROR;
  }

  ExtByte = NULL;
#else
  if (!EFI_ERROR (Status)) {
    ExtByte = ((UINT8*)&CardData->ExtCSDRegister) + Offset;
    *ExtByte = (UINT8) ((*ExtByte & 0xF7) | Partition);
  }
#endif

  return Status;
}




EFI_STATUS
EFIAPI
SecureErase(
  IN EmmcCardData             *CardData
)
{
  EFI_STATUS             Status;  
  UINT64                 SecureEraseTimeout; // Secure erase-Timeout issue

  DEBUG ((EFI_D_INFO, "eMMC Secure Erase Checkpoint\n"));
  
        
                  MmcSelectPartitionNum(CardData, 0);   // 0 --> User partition
        
                  Status  = SendCommand (
                      CardData->SdHostIo,
                      ERASE_GROUP_START,
                      0,
                      NoData,
                      NULL,
                      0,
                      ResponseR1,
                      TIMEOUT_COMMAND,
                      (UINT32*)&(CardData->CardStatus)
                      );
                  if (Status) {
                      Print (L"Set ERASE_GROUP_START failed\n");
                      return Status;
                     }

                 //   Print (L"StartSector: %x\n",StartSector );
                                         
              Status  = SendCommand (
                  CardData->SdHostIo,
                  ERASE_GROUP_END,
                  (*(UINT32*)((UINT8 *)&CardData->ExtCSDReg.SEC_COUNT)-1),
                  NoData,
                  NULL,
                  0,
                  ResponseR1,
                  TIMEOUT_COMMAND,
                  (UINT32*)&(CardData->CardStatus)
                  );
              if (Status) {
                  Print (L"Totalsector: %x\n",*(UINT32*)((UINT8 *)&CardData->ExtCSDReg.SEC_COUNT) );
                  Print (L"Set ERASE_GROUP_END failed\n");
                  return Status;
                 }  

             
          
          if (CardData->ExtCSDReg.ERASE_GROUP_DEF) {
              SecureEraseTimeout = CardData->ExtCSDReg.SEC_ERASE_MULT 
                                    * CardData->ExtCSDReg.ERASE_TIMEOUT_MULT 
                                     * (*(UINT32*)((UINT8 *)&CardData->ExtCSDReg.SEC_COUNT) + 1) 
                                    / (1024 * (CardData->ExtCSDReg.HC_WP_GRP_SIZE) * CardData->ExtCSDReg.HC_ERASE_GRP_SIZE);
          }
          else {
              SecureEraseTimeout = CardData->ExtCSDReg.SEC_ERASE_MULT 
                                    * CardData->ExtCSDReg.ERASE_TIMEOUT_MULT 
                                    * (*(UINT32*)((UINT8 *)&CardData->ExtCSDReg.SEC_COUNT) + 1) 
                                    / ((CardData->CSDReg.ERASE_GRP_SIZE + 1) * (CardData->ExtCSDReg.HC_WP_GRP_SIZE) * (CardData->CSDReg.ERASE_GRP_MULT));
          }
        // DEBUG ((EFI_D_INFO, "CardData->ExtCSDRegister.SEC_ERASE_MULT : %x\n", CardData->ExtCSDRegister.SEC_ERASE_MULT));
       //   DEBUG ((EFI_D_INFO, "CardData->ExtCSDRegister.ERASE_TIMEOUT_MULT : %x\n", CardData->ExtCSDRegister.ERASE_TIMEOUT_MULT));
        // DEBUG ((EFI_D_INFO, "CardData->ExtCSDRegister.HC_ERASE_GRP_SIZE : %x\n", CardData->ExtCSDRegister.HC_ERASE_GRP_SIZE));
        //  DEBUG ((EFI_D_INFO, "CardData->ExtCSDRegister.SEC_ERASE_MULT : %x\n", CardData->ExtCSDRegister.SEC_ERASE_MULT));
       //  DEBUG ((EFI_D_INFO, "CardData->ExtCSDRegister.ERASE_TIMEOUT_MULT : %x\n", CardData->ExtCSDRegister.ERASE_TIMEOUT_MULT));
       //   DEBUG ((EFI_D_INFO, "SecureEraseTimeout : %lx\n", SecureEraseTimeout)); 
       //   DEBUG ((EFI_D_INFO, "lastblock : %lx\n", ((UINTN)CardData->Partitions[0].BlockIoMedia.LastBlock + 1)));
          
          Status  = SendCommand (
              CardData->SdHostIo,
              ERASE,
              0x80000000,                                               // Secure Erase bit 31
              NoData,
              NULL,
              0,
              ResponseR1b,
              (UINT32)SecureEraseTimeout,
              (UINT32*)&(CardData->CardStatus)
              );
          
		  do{
          //Print (L"Secure Erase and wait\n");
          gBS->Stall(10);                               // Timeout needed to avoid further errors during the flow.
		  }while(SecureEraseTimeout--);
          MmcUpdateCardStatus (CardData);
          Print (L"after MmcUpdateCardStatus \n");
          if (CardData->CardStatus.WP_ERASE_SKIP) {
               Print (L"Secure Erase failed due to Write protect\n");
          }
          else {
              Print (L"Secure Erase  SecureEraseDone = TRUE\n");         
          }
       return Status;
         
      }
  






EFI_STATUS
EFIAPI
TrimErase(
  IN EmmcCardData             *CardData,
  IN  UINTN                   StartSector,
  IN  UINTN                   Endsector
)
{
  EFI_STATUS             Status;  
  UINT64                 SecureEraseTimeout; // Secure erase-Timeout issue

                  MmcSelectPartitionNum(CardData, 0);   // 0 --> User partition
                  Status  = SendCommand (
                      CardData->SdHostIo,
                      ERASE_GROUP_START,
                      (UINT32)StartSector,
                      NoData,
                      NULL,
                      0,
                      ResponseR1,
                      TIMEOUT_COMMAND,
                      (UINT32*)&(CardData->CardStatus)
                      );
                  if (Status) {
                      Print (L"Set ERASE_GROUP_START failed\n");
                      return Status;
                     }

                    Print (L"StartSector: %x\n",StartSector );
              Status  = SendCommand (
                  CardData->SdHostIo,
                  (UINT32)ERASE_GROUP_END,
                  (UINT32)Endsector,
                  NoData,
                  NULL,
                  0,
                  ResponseR1,
                  TIMEOUT_COMMAND,
                  (UINT32*)&(CardData->CardStatus)
                  );
              if (Status) {
                  Print (L"Set ERASE_GROUP_END failed\n");
                  return Status;
                 }  

               Print (L"Endsector: %x\n",Endsector );
          
          if (CardData->ExtCSDReg.ERASE_GROUP_DEF) {
              SecureEraseTimeout = CardData->ExtCSDReg.SEC_ERASE_MULT 
                                    * CardData->ExtCSDReg.ERASE_TIMEOUT_MULT 
                                    * ((UINT32)(Endsector - StartSector) + 1) 
                                    / (1024 * (CardData->ExtCSDReg.HC_WP_GRP_SIZE) * CardData->ExtCSDReg.HC_ERASE_GRP_SIZE);
          }
          else {
              SecureEraseTimeout = CardData->ExtCSDReg.SEC_ERASE_MULT 
                                    * CardData->ExtCSDReg.ERASE_TIMEOUT_MULT 
                                    * ( (UINT32)(Endsector - StartSector)+ 1) 
                                    / ((CardData->CSDReg.ERASE_GRP_SIZE + 1) * (CardData->ExtCSDReg.HC_WP_GRP_SIZE) * (CardData->CSDReg.ERASE_GRP_MULT));
          }
       //  DEBUG ((EFI_D_INFO, "CardData->ExtCSDRegister.SEC_ERASE_MULT : %x\n", CardData->ExtCSDRegister.SEC_ERASE_MULT));
       //   DEBUG ((EFI_D_INFO, "CardData->ExtCSDRegister.ERASE_TIMEOUT_MULT : %x\n", CardData->ExtCSDRegister.ERASE_TIMEOUT_MULT));
       //  DEBUG ((EFI_D_INFO, "CardData->ExtCSDRegister.HC_ERASE_GRP_SIZE : %x\n", CardData->ExtCSDRegister.HC_ERASE_GRP_SIZE));
      //    DEBUG ((EFI_D_INFO, "CardData->ExtCSDRegister.SEC_ERASE_MULT : %x\n", CardData->ExtCSDRegister.SEC_ERASE_MULT));
      //   DEBUG ((EFI_D_INFO, "CardData->ExtCSDRegister.ERASE_TIMEOUT_MULT : %x\n", CardData->ExtCSDRegister.ERASE_TIMEOUT_MULT));
      //    DEBUG ((EFI_D_INFO, "SecureEraseTimeout : %lx\n", SecureEraseTimeout)); 
     //     DEBUG ((EFI_D_INFO, "lastblock : %lx\n", ((UINTN)CardData->Partitions[0].BlockIoMedia.LastBlock + 1)));
          
          Status  = SendCommand (
              CardData->SdHostIo,
              ERASE,
              0x00000001,                                               // Secure Erase bit 31
              NoData,
              NULL,
              0,
              ResponseR1b,
              (UINT32)SecureEraseTimeout,
              (UINT32*)&(CardData->CardStatus)
              );
		  do{
          //Print (L"Secure Erase and wait\n");
          gBS->Stall(10);                               // Timeout needed to avoid further errors during the flow.
		  }while(SecureEraseTimeout--);
          MmcUpdateCardStatus (CardData);
          Print (L"after MmcUpdateCardStatus \n");
          if (CardData->CardStatus.WP_ERASE_SKIP) {
               Print (L"Secure Erase failed due to Write protect\n");
          }
          else {
              Print (L"Secure Erase  SecureEraseDone = TRUE\n");         
          }
       return Status;
      }

EFI_STATUS
CalculateCardParameter (
    IN EmmcCardData *Data
  )
/*++

  Routine Description:
    Get card interested information for CSD rergister
  
  Arguments:
    CardData  - Pointer to CARD_DATA   

  Returns:  
    EFI_INVALID_PARAMETER
    EFI_UNSUPPORTED
    EFI_SUCCESS
 --*/   
{
    EFI_STATUS     Status;
    UINT32         Frequency;
    UINT32         Multiple;
    UINT32         CSize;


    Status = EFI_SUCCESS;


    switch (Data->CSDReg.TRAN_SPEED & 0x7) {
    case 0:
        Frequency = 100 * 1000;
        break;

    case 1:
        Frequency = 1 * 1000 * 1000;
        break;

    case 2:
        Frequency = 10 * 1000 * 1000;
        break;

    case 3:
        Frequency = 100 * 1000 * 1000;
        break;

    default:
        Print(L"CalculateCardParameter: Invalid CSD TRAN_SPEED Frequency: 0x%x\n", Data->CSDReg.TRAN_SPEED & 0x7);
        Status = EFI_INVALID_PARAMETER;
        goto Exit;
    }

    switch ((Data->CSDReg.TRAN_SPEED >> 3) & 0xF) {
    case 1:
        Multiple = 10;
        break;

    case 2:
        Multiple = 12;
        break;

    case 3:
        Multiple = 13;
        break;

    case 4:
        Multiple = 15;
        break;

    case 5:
        Multiple = 20;
        break;

    case 6:
        if (Data->CardType == MMCCard) {
            Multiple = 26;
        } else {
            Multiple = 25;
        }
        break;

    case 7:
        Multiple = 30;
        break;

    case 8:
        Multiple = 35;
        break;

    case 9:
        Multiple = 40;
        break;

    case 10:
        Multiple = 45;
        break;

    case 11:
        if (Data->CardType == MMCCard) {
            Multiple = 52;
        } else {
            Multiple = 50;
        }
        break;

    case 12:
        Multiple = 55;
        break;

    case 13:
        Multiple = 60;
        break;

    case 14:
        Multiple = 70;
        break;

    case 15:
        Multiple = 80;
        break;

    default:
        DEBUG((EFI_D_ERROR, "CalculateCardParameter: Invalid CSD TRAN_SPEED Multiple: 0x%x\n", Data->CSDReg.TRAN_SPEED >> 3));
        Status = EFI_INVALID_PARAMETER;
        goto Exit;
    }

    Frequency = Frequency * Multiple / 10;
    Data->MaxFrequency = Frequency;

  if ((Data->ExtCSDReg.CARD_TYPE & BIT2) ||
      (Data->ExtCSDReg.CARD_TYPE & BIT3)) {
      Data->BlockLen = 512;
  }
  else {
      Data->BlockLen = 1 << Data->CSDReg.READ_BL_LEN;
  }

    //
    // For MMC card > 2G, the block number will be recaculate later
    //
    CSize = Data->CSDReg.C_SIZELow2 | (Data->CSDReg.C_SIZEHigh10 << 2);
    Data->BlockNumber = MultU64x32 (LShiftU64 (1, Data->CSDReg.C_SIZE_MULT + 2), CSize + 1);


    //
    //For >= 2G card, BlockLen may be 1024, but the transfer size is still 512 bytes
    //
    if (Data->BlockLen > 512) {
        Data->BlockNumber = DivU64x32 (MultU64x32 (Data->BlockNumber, Data->BlockLen), 512);
        Data->BlockLen    = 512;
    }

    DEBUG((
          EFI_D_ERROR,
          "CalculateCardParameter: Card Size: 0x%lx\n", MultU64x32 (Data->BlockNumber, Data->BlockLen)
          ));

    Exit:
    return Status;
}


EFI_STATUS
EmmcInit (
  IN  EmmcCardData *Data,
  IN  UINT32      ClockFreq,
  OUT BOOLEAN     *InitFlag
  )
/*++

  Routine Description:
    eMMC card init function
  
  Arguments:
    CardData    - Pointer to CARD_DATA   
  
  Returns:  
    EFI_INVALID_PARAMETER
    EFI_SUCCESS
    EFI_UNSUPPORTED
    EFI_BAD_BUFFER_SIZE
 --*/   
{
    EFI_STATUS                 Status = 0;
    EFI_SD_HOST_IO_PROTOCOL   *SdHostIo;
    SWITCH_ARGUMENT            SwitchArgument;
    UINT32                     Blocks;
    UINT8                      PowerValue;
    UINT8                      DoubleSpeed;

    ASSERT(Data != NULL);
    SdHostIo                  = Data->SdHostIo;
    Data->CurrentBusWidth = 1;

    ASSERT (SdHostIo->HostCapability.BoundarySize >= 4 * 1024);
    Data->RawBufferPointer = (UINT8*)AllocateZeroPool (2 * SdHostIo->HostCapability.BoundarySize);
    if (Data->RawBufferPointer == NULL) {
       Print (L"EmmcInit: Fail to AllocateZeroPool(2*0x%x) \n",SdHostIo->HostCapability.BoundarySize );
       Status =  EFI_OUT_OF_RESOURCES;
       return Status;
    }
    Data->AlignedBuffer = Data->RawBufferPointer - ((UINTN)(Data->RawBufferPointer) & (SdHostIo->HostCapability.BoundarySize - 1)) + SdHostIo->HostCapability.BoundarySize;


    Data->CardType = MMCCard;

    //
    //MMC, SD card need host auto stop command support
    //
    SdHostIo->EnableAutoStopCmd (SdHostIo, TRUE);

    SdHostIo->SetupDevice (SdHostIo);

    CHK_FUNC_CALL(MmcGetOCR(Data), "MmcGetOCR", TRUE);
    Print(L"OCR=0x%x\n", *(UINT32*)&(Data->OCRReg));

    CHK_FUNC_CALL(MmcGetCID(Data), "MmcGetCID", TRUE);
    

    //
    //SET_RELATIVE_ADDR
    //
    if (Data->CardType == MMCCard) {
         Data->Address = 1;

        //
        // Set RCA Register
        //
        Status  = SendCmd3(Data);

        if (EFI_ERROR (Status)) {
            DEBUG((EFI_D_ERROR, "EmmcInit: SET_RELATIVE_ADDR Fail -> Status = 0x%x\n", Status));
            goto Exit;
        }
    } 

    //
    // Get CSD Register
    //
    CHK_FUNC_CALL(MmcGetCSD(Data), "MmcGetCSD", TRUE);

    CHK_FUNC_CALL(CalculateCardParameter (Data), "CalculateCardParameter", TRUE);


    //
    //Put the card into tran state
    //
    CHK_FUNC_CALL(SendCmd7(Data, Data->Address), "CMD7", TRUE);

    CHK_FUNC_CALL(SendCmd13(Data), "CMD13", TRUE);

    if (Data->CardType == MMCCard) {
        //
        //Only V4.0 and above supports more than 1 bits and high speed
        //
        if (Data->CSDReg.SPEC_VERS >= 4) {
           //
           //Get ExtCSDReg
           //
           CHK_FUNC_CALL(MmcGetExtCSD(Data), "MmcGetExtCSD", TRUE);
           
            //
            // Recaculate the block number for >2G MMC card
            //
            Blocks  = (Data->ExtCSDReg.SEC_COUNT[0]) |
                    (Data->ExtCSDReg.SEC_COUNT[1] << 8) | 
                    (Data->ExtCSDReg.SEC_COUNT[2] << 16) |
                    (Data->ExtCSDReg.SEC_COUNT[3] << 24);

            if (Blocks != 0) {
                Data->BlockNumber = Blocks;
            }
             DEBUG((DEBUG_INFO, "eMMC Card BlockNumbe  %d\n", Data));
            // Check current chipset capability and the plugged-in card 
            // whether supports HighSpeed
            //     
            if (SdHostIo->HostCapability.HighSpeedSupport) {
                //
                //Change card timing to high speed interface timing
                //
                ZeroMem(&SwitchArgument, sizeof (SWITCH_ARGUMENT));
                SwitchArgument.CmdSet = 0;
                SwitchArgument.Value  = 1;
                SwitchArgument.Index  = (UINT32)((UINTN)(&(Data->ExtCSDReg.HS_TIMING)) 
                                                 - (UINTN)(&(Data->ExtCSDReg)));
                SwitchArgument.Access = WriteByte_Mode;
                Status  = SendCmd6(Data, SwitchArgument);

                if (EFI_ERROR (Status)) {
                    DEBUG((EFI_D_ERROR, "SWITCH frequency -> %r\n", Status));
                }

                gBS->Stall (5);
                if (!EFI_ERROR (Status)) {
                    Status  = SendCmd13(Data);
                        if (!EFI_ERROR (Status)) {
                             //
                             // Enable the high speed mode
                             // 
                             SdHostIo->SetHostSpeedMode (SdHostIo, 1);
                            //
                            // Change host clock to support high speed and enable chispet to 
                            // support speed
                            // 
                            if ((Data->ExtCSDReg.CARD_TYPE & BIT1) != 0) {
                                if ((ClockFreq * 1000 * 1000) > FREQUENCY_MMC_PP_HIGH) {
                                  ClockFreq = FREQUENCY_MMC_PP_HIGH/(1000*1000);
                                 }
                                Status = SdHostIo->SetClockFrequency (SdHostIo, ClockFreq*1000*1000);
                            } else if ((Data->ExtCSDReg.CARD_TYPE & BIT0) != 0) {                   
                                 if ((ClockFreq * 1000 * 1000) > FREQUENCY_MMC_PP) {
                                  ClockFreq = FREQUENCY_MMC_PP/(1000*1000);
                                 }
                                Status = SdHostIo->SetClockFrequency (SdHostIo, ClockFreq*1000*1000);
                            } else {
                                Status = EFI_UNSUPPORTED;
                            }
                            if (EFI_ERROR (Status)) {
                                DEBUG((EFI_D_ERROR, "EmmcInit: SetClockFrequency Fail -> Status = 0x%x\n", Status));
                                goto Exit;
                            }
                            //
                            // It seems no need to stall after changing bus freqeuncy.
                            // It is said that the freqeuncy can be changed at any time. Just appends 8 clocks after command.  
                            // But SetClock alreay has delay.     
                            // 
                        }
                 }
            // Card bus width test only works for single data rate mode !!!
      }

       DoubleSpeed = 0 ;
       DEBUG((EFI_D_ERROR, "Data->ExtCSDReg.CARD_TYPE -> %d\n", (UINTN)Data->ExtCSDReg.CARD_TYPE));
       if ((Data->ExtCSDReg.CARD_TYPE & BIT2)||
           (Data->ExtCSDReg.CARD_TYPE & BIT3)) {      
          DEBUG((EFI_D_ERROR, "DDR is enabled\n"));
          DoubleSpeed = 20;   //Add 20 for double speed, decoded in MMCCardSetBusWidth()
       }

            //
            // Prefer wide bus width for performance
            //
            //
            // Set to BusWidth bits mode, only version 4.0 or above support more than 1 bits
            //
            if (SdHostIo->HostCapability.BusWidth8 == TRUE) {
              
                 DEBUG((EFI_D_ERROR, "Support 8 bit, set value 0x%x\n", DoubleSpeed + 8));
                Status = MMCCardSetBusWidth (Data, DoubleSpeed + 8);
                if (EFI_ERROR (Status)) {
                    //
                    // CE-ATA may support 8 bits and 4 bits, but has no software method for detection
                    // 
                    Status = MMCCardSetBusWidth (Data, DoubleSpeed + 4);
                    if (EFI_ERROR (Status)) {
                        goto Exit;
                    }
                }
            } else if (SdHostIo->HostCapability.BusWidth4 == TRUE) {
                DEBUG((EFI_D_ERROR, "Support 4 bit, set value 0x%x\n", DoubleSpeed + 4));
                Status = MMCCardSetBusWidth (Data, DoubleSpeed + 4);
                if (EFI_ERROR (Status)) {
                    goto Exit;
                }
            }
            DEBUG((EFI_D_ERROR, "Data->CurrentBusWidth -> %d\n", (UINTN)Data->CurrentBusWidth));

            PowerValue = 0;

            if (Data->CurrentBusWidth == 8) {
                if ((Data->ExtCSDReg.CARD_TYPE & BIT1) != 0) {
                    PowerValue = Data->ExtCSDReg.PWR_CL_52_360;
                    PowerValue = PowerValue >> 4;
                } else if ((Data->ExtCSDReg.CARD_TYPE & BIT0) != 0) {
                    PowerValue = Data->ExtCSDReg.PWR_CL_26_360;
                    PowerValue = PowerValue >> 4;
                }
            } else if (Data->CurrentBusWidth == 4) {
                if ((Data->ExtCSDReg.CARD_TYPE & BIT1) != 0) {
                    PowerValue = Data->ExtCSDReg.PWR_CL_52_360;
                    PowerValue = PowerValue & 0xF;
                } else if ((Data->ExtCSDReg.CARD_TYPE & BIT0) != 0) {
                    PowerValue = Data->ExtCSDReg.PWR_CL_26_360;
                    PowerValue = PowerValue & 0xF;
                }
            }

            if (PowerValue != 0) {
                //
                //Update Power Class
                //
                ZeroMem(&SwitchArgument, sizeof (SWITCH_ARGUMENT));
                SwitchArgument.CmdSet = 0;
                SwitchArgument.Value  = PowerValue;
                SwitchArgument.Index  = (UINT32)((UINTN)(&(Data->ExtCSDReg.POWER_CLASS)) 
                                                 - (UINTN)(&(Data->ExtCSDReg)));
                SwitchArgument.Access = WriteByte_Mode;
                Status  = SendCmd6(Data, SwitchArgument);
                 if (!EFI_ERROR (Status)) {
                   Status  = SendCmd13(Data);
                        if (EFI_ERROR (Status)) {
                            DEBUG((EFI_D_ERROR, "EmmcInit: SWITCH Power Class Fail -> Status = 0x%x\n", Status));
                        }
                        gBS->Stall (10);
                    }
              }
         } else {
            DEBUG((EFI_D_ERROR, "EmmcInit: MMC Card version %d only supportes 1 bits at lower transfer speed\n",Data->CSDReg.SPEC_VERS));
        }

    }  

//
// Set Block Length, to improve compatibility in case of some cards
//
    if (!((Data->ExtCSDReg.CARD_TYPE & BIT2) ||
         (Data->ExtCSDReg.CARD_TYPE & BIT3))) {  
         //
         // Set Block Length, to improve compatibility in case of some cards
         //
         Status  = SendCommand (
                     SdHostIo,
                     SET_BLOCKLEN,
                     512,
                     NoData,
                     NULL,
                     0,  
                     ResponseR1,
                     TIMEOUT_COMMAND,
                     (UINT32*)&(Data->CardStatus)
                     );
         if (EFI_ERROR (Status)) {
           DEBUG((EFI_D_ERROR, "SET_BLOCKLEN -> %r\n", Status));
           goto Exit;
         }
     }
     SdHostIo->SetBlockLength (SdHostIo, 512);
    
    Print(L"eMMC completed the initialization successfully!!\n");
    *InitFlag = TRUE;
    
    Exit:
    return Status;
}


VOID
EFIAPI
EmmcInfo (
    IN EmmcCardData *Data

  )
{
    CSD *CSDReg = &Data->CSDReg;
    EXT_CSD * ExtCSDReg = &Data->ExtCSDReg;

    Print (L"eMMC Size = %d GB (Decimal)\n", EmmcGetFlashSize (Data) >> 30);
    EmmcGetCardRevision (Data);
    Print (L"Boot PartitionSize = %d KB (Decimal)\n", EmmcGetBootPartitionSize (Data));
    Print (L"RPMB PartitionSize = %d KB (Decimal)\n", EmmcGetRPMBPartitionSize (Data));
    Print(L"General Purpose Partition 1 Size:   %dMB(%a)\n", EmmcGetGeneralPartitionSize(Data, 1)/(1024*1024), ((ExtCSDReg->PARTITIONS_ATTRIBUTES >> 1) & 0x1) ? " Enhanced" : " Not enhanced");
    Print(L"General Purpose Partition 2 Size:   %dMB(%a)\n", EmmcGetGeneralPartitionSize(Data, 2)/(1024*1024), ((ExtCSDReg->PARTITIONS_ATTRIBUTES >> 2) & 0x1) ? " Enhanced" : " Not enhanced");
    Print(L"General Purpose Partition 3 Size:   %dMB(%a)\n", EmmcGetGeneralPartitionSize(Data, 3)/(1024*1024), ((ExtCSDReg->PARTITIONS_ATTRIBUTES >> 3) & 0x1) ? " Enhanced" : " Not enhanced");
    Print(L"General Purpose Partition 4 Size:   %dMB(%a)\n", EmmcGetGeneralPartitionSize(Data, 4)/(1024*1024), ((ExtCSDReg->PARTITIONS_ATTRIBUTES >> 4) & 0x1) ? " Enhanced" : " Not enhanced");
    Print(L"Enhanced User Data Area. \n");
    Print(L" Start Address:          %dKB\n", EmmcGetUsrEnhStatAddr(Data)>>10);
    Print(L" Size:                   %dKB\n", EmmcGetUserMaxEnhSize(Data));
    
    EmmcGetAccessingPartition(Data);

    Print(L"Protection features: \n");
    Print(L"Permanent write protect: %x\n", CSDReg->PERM_WRITE_PROTECT);
}


CHAR16 *Strtok(CHAR16 *Str, CONST CHAR16 *Delim) {
        STATIC CHAR16 *Last;
        CHAR16 *Tok;
        CHAR16 *ucDelim;
        INT32 Found = 0;
        INT32 DelimFound = 0;


        if (Str == NULL)
                Str = Last;
        if(Str == NULL)
                return NULL;

        Tok = Str;

        
        while (!Found && *Str != L'\0') {
                ucDelim = (CHAR16 *) Delim;
                while (*ucDelim) {
                        while (*Str == *ucDelim) {
                              *Str = L'\0';
                              Str++; // bypass the delim;
                              DelimFound = 1;
                            }
                        if (DelimFound)
                        {
                                Str--; // back to last delim
                                Found = 1;
                                *Str = L'\0';
                                Last = Str + 1;
                                break;
                        }
                        ucDelim++;
                }
                if (!Found) {
                        Str++;
                        Last++;
                        if (*Str == L'\0') {
                           Last = NULL;
                        }
                }
        }
        if(Last) {//check Last is not a NULL pointer
          if (*Last == L'\0') {
             Last = NULL;
         }
        }
        return Tok;
}

EFI_STATUS EmmcGetArgument(IN CHAR16 *Str,   IN UINTN * Argc,  IN CHAR16 **Argv)
{
   CHAR16 *Token;
   CONST CHAR16 *Delim = L" ";
   
   * Argc = 0;
/* Establish string and get the first token: */
  Token = Strtok( Str, Delim );
  //Print(L" %s\n", Token );

  while( Token != NULL)
  { 
     StrCpy(Argv[*Argc], Token); 
     //Print(L" %s, len is %d\n", Argv[*Argc], StrLen(Argv[*Argc]) );
     (*Argc) ++;
     
     /* While there are tokens in "string" */
     
     /* Get next token: */
     Token = Strtok( NULL, Delim );
  }
  //Print(L" %d\n", *Argc );
  return 0;
}

VOID
EFIAPI
DefaultHelpMessage (
  )
{
  Print (L"Help message\n");
  Print (L"          Ex:Emmc.efi -help\n\n");
  Print (L"Interactive Mode\n");
  Print (L"          Ex:Emmc.efi -i\n");
  Print (L"             In this mode, emmc App can init the emmc card instead did in driver.\n");
  Print (L"             OCR, CID, CSD, EXTCSD wil be updated and read/write interfaces are provided in App.\n");
  Print (L"             Default mode is using the data and protocols from driver.\n");
  Print (L"Get Emmc Infomation\n");
  Print (L"          Ex:Info\n\n");
  Print (L"Show CID infomation\n");
  Print (L"          Ex:CID\n\n");
  Print (L"Show CSD infomation\n");
  Print (L"          Ex:CSD\n\n");
  Print (L"Show EXTCSD infomation\n");
  Print (L"             Ex:ExtCSD\n\n");
  Print (L"Update EXTCSD infomation\n");
  Print (L"             Ex:UPEXTCSD\n\n");
  Print (L"Show OCR infomation\n");
  Print (L"          Ex:OCR\n\n");
  Print (L"Get Card Status infomation\n");
  Print (L"          Ex:status\n\n");
  Print (L"Reset\n");
  Print (L"   Ex:Emmc.efi Reset\n\n");
  Print (L"ReadBlocks (RB) to the Buffer\n");
  Print (L"             Ex:Emmc.efi RB LBA BufferSize *Buffer\n");
  Print (L"                                 LBA        Hex format\n");
  Print (L"                                 BufferSize Hex format\n");
  Print (L"                                 *Buffer    Hex format\n");
  Print (L"             Ex:Emmc.efi rb 0 200 1000000\n\n");
  Print (L"WriteBlocks (WB)from the Buffer to the specified block\n");
  Print (L"              Ex:Emmc.efi WB LBA BufferSize *Buffer\n");
  Print (L"                                  LBA        Hex format\n");
  Print (L"                                  BufferSize Hex format\n");
  Print (L"                                  *Buffer    Hex format\n");
  Print (L"             Ex:Emmc.efi wb 0 200 1000000\n\n");
}

VOID
EFIAPI
InteractiveHelpMessage (
  )
{
  Print (L"Help message\n");
  Print (L"          Ex:help\n\n");
  Print (L"Init eMMC in assigned clock frequency with decimal format\n");
  Print (L"It will init to max frequency if no clkfreq parameter\n");
  Print (L"          Ex:Init [clkfreq (MHZ)]\n\n");
  Print (L"Get Emmc Infomation\n");
  Print (L"          Ex:Info\n\n");
  Print (L"Show CID infomation\n");
  Print (L"          Ex:CID\n\n");
  Print (L"Show CSD infomation\n");
  Print (L"          Ex:CSD\n\n");
  Print (L"Show EXTCSD infomation\n");
  Print (L"             Ex:ExtCSD\n\n");
  Print (L"Show OCR infomation\n");
  Print (L"          Ex:OCR\n\n");
  Print (L"Get Card Status infomation\n");
  Print (L"          Ex:status\n\n");
  Print (L"Reset\n");
  Print (L"   Ex:Emmc.efi Reset\n\n");
  Print (L"rd value from specific ext csd register\n");
  Print (L"             Ex:rd extcsd [Index in decimal format]\n");
  Print (L"wr value to specific ext csd register\n");
  Print (L"             Ex:wr extcsd [Index in decimal format] [value in hex format]\n");
  Print (L"ReadBlocks (RB) to the Buffer\n");
  Print (L"             Ex:RB LBA BufferSize *Buffer\n");
  Print (L"                                 LBA        Hex format\n");
  Print (L"                                 BufferSize Hex format\n");
  Print (L"                                 *Buffer    Hex format\n");
  Print (L"             Ex:rb 0 200 1000000\n\n");
  Print (L"WriteBlocks (WB)from the Buffer to the specified block\n");
  Print (L"              Ex:WB LBA BufferSize *Buffer\n");
  Print (L"                                  LBA        Hex format\n");
  Print (L"                                  BufferSize Hex format\n");
  Print (L"                                  *Buffer    Hex format\n");
  Print (L"             Ex:wb 0 200 1000000\n\n");
  Print (L"Secure Erase the emmc data\n");
  Print (L"          Ex:secureerase\n\n");
}


EFI_STATUS InteractiveMode(EmmcCardData *Data)
{

  CHAR16 *InputStr = NULL;
  UINT32 Index = 0;
  BOOLEAN Complete = FALSE; 
  BOOLEAN InitFlag = FALSE;
  EFI_STATUS Status = 0;
  UINT32 ArgCount = 0;
  CHAR16 *ArgStr[50];

  for (Index=0; Index<50; Index++)
      ArgStr[Index] = (CHAR16*)AllocateZeroPool (sizeof(CHAR16) * 100);
  //
  // interactive mode
  //
  ShellSetPageBreakMode (TRUE);
  Complete = FALSE;
  do {

    ShellPrintEx(-1, -1, L" eMMC -> ");
    //
    // wait user input to modify
    //
    if (InputStr != NULL) {
      FreePool(InputStr);
      InputStr = NULL;
    }
    ShellPromptForResponse(ShellPromptResponseTypeFreeform, NULL, (VOID**)&InputStr);
  
    //
    // skip space characters
    //
    for (Index = 0; InputStr != NULL && InputStr[Index] == ' '; Index++);
  
    //
    // parse input string
    //
    if (InputStr != NULL && (InputStr[Index] == '.' || InputStr[Index] == 'q' || InputStr[Index] == 'Q')) {
      Complete = TRUE;
    } else if (InputStr == NULL || InputStr[Index] == CHAR_NULL) {
      //
      // Continue to next address
      //
    } else {
            if (EmmcGetArgument((CHAR16 *)&InputStr[Index], (UINTN*)&ArgCount, (CHAR16 **)ArgStr)) {
                Print(L"Get Argument failed\n");
                return 0;      
            }
            if (!StrCmp (L"help", ArgStr[0])) {
              InteractiveHelpMessage ();
            } else if ((!StrCmp (L"INFO", ArgStr[0]) || (!StrCmp (L"info", ArgStr[0])))) {
              if (!InitFlag) {Print(L"Please run init command firstly\n"); continue;}
              EmmcInfo ( Data );
              if (!EFI_ERROR (Status)) {
                Print (L"Info success\n");
              } else {
                Print (L"Info fail\n");
              }
            }else if (!StrCmp (L"INIT", ArgStr[0]) || !StrCmp (L"init", ArgStr[0])) {
             if (ArgCount > 1) {
                EmmcInit(Data, (UINT32)StrDecimalToUintn(ArgStr[1]), &InitFlag);
              } else {
                EmmcInit(Data, FREQUENCY_MMC_PP_HIGH/(1000*1000), &InitFlag);
              }
            } else if (!StrCmp (L"CID", ArgStr[0]) || !StrCmp (L"cid", ArgStr[0])) {
               if (!InitFlag) {Print(L"Please run init command firstly\n"); continue;}
              MmcDecodeCID (&Data->CIDReg);
            } else if (!StrCmp (L"CSD", ArgStr[0]) || !StrCmp (L"csd", ArgStr[0])) {
               if (!InitFlag) {Print(L"Please run init command firstly\n"); continue;}
              MmcDecodeCSD (&Data->CSDReg);
            } else if (!StrCmp (L"ExtCSD", ArgStr[0])  || !StrCmp (L"extcsd", ArgStr[0])) {
              //if (!InitFlag) {Print(L"Please run init command firstly\n"); continue;}
              Print(L"Start read ExtCSD:\n");
              MmcReadExtCsd(Data);
              Print(L"Start Decode ExtCSD\n");
              MmcDecodeExtCSD (&Data->ExtCSDReg);
            } else if (!StrCmp (L"OCR", ArgStr[0]) || !StrCmp (L"ocr", ArgStr[0])) {
               if (!InitFlag) {Print(L"Please run init command firstly\n"); continue;}
              MmcDecodeOCR (&Data->OCRReg);
            } else if (!StrCmp (L"STATUS", ArgStr[0]) || !StrCmp (L"status", ArgStr[0])) {
               
              MmcUpdateCardStatus (Data);
                    Print(L"Carddc Status is 0x%x\n", *(UINT32*)&Data->CardStatus);
            }   else if (!StrCmp (L"WR", ArgStr[0]) || !StrCmp (L"wr", ArgStr[0])) {
            
               if (!InitFlag) {Print(L"Please run init command firstly\n"); continue;}
                 if (ArgCount > 3) {
                  if (!StrCmp (L"ExtCSD", ArgStr[1]) || !StrCmp (L"extcsd", ArgStr[1])) {
                       UINT8 i = 0;
                       UINT8 Temp = 0;
                       i = (UINT8)StrDecimalToUintn ((CHAR16 *)ArgStr[2]);
                       Temp = (UINT8)StrHexToUintn ((CHAR16 *)ArgStr[3]);
                       MmcSetExtCsd8(Data, i, Temp);
                    }
                 }
            }  else if (!StrCmp (L"RD", ArgStr[0]) || !StrCmp (L"rd", ArgStr[0])) {
            
               if (!InitFlag) {Print(L"Please run init command firstly\n"); continue;}
                 if (ArgCount > 2) {
                  if (!StrCmp (L"ExtCSD", ArgStr[1]) || !StrCmp (L"extcsd", ArgStr[1])) {
                       UINT8 i = 0;
                       UINT8 Temp = 0;
                       i = (UINT8)StrDecimalToUintn ((CHAR16 *)ArgStr[2]);
                       Print(L"Start read ExtCSD:\n");
                       MmcReadExtCsd(Data);
                       Temp = MmcGetExtCsd8(Data, i);
                       Print(L"ExtCSD Offset[%d] = 0x%x\n", i, Temp);
                    }
                 }
            }  else if (!StrCmp (L"Reset", ArgStr[0]) || !StrCmp (L"reset", ArgStr[0])) {
                                         
              Status = Data->SdHostIo->ResetSdHost (Data->SdHostIo, Reset_All);
              if (!EFI_ERROR (Status)) {
                Print (L"Reset ALL success\n");
              } else {
                Print (L"Reset fail\n");
              }
            } else if (!StrCmp (L"RB", (CHAR16 *)ArgStr[0]) || !StrCmp (L"rb", (CHAR16 *)ArgStr[0])) {
               if (!InitFlag) {Print(L"Please run init command firstly\n"); continue;}
              if (ArgCount > 3) {
                Status = EmmcReadBlocks (
                                    Data,
                                    0,
                                    StrHexToUint64 ((CHAR16 *)ArgStr[1]),
                                    StrHexToUintn ((CHAR16 *)ArgStr[2]),
                                    (VOID *)StrHexToUintn ((CHAR16 *)ArgStr[3])
                                    );

                if (!EFI_ERROR (Status)) {
                  Print (L"ReadBlocks success\n");
                } else {
                  Print (L"ReadBlocks fail\n");
                }
              } else {
                Print (L"Invalid Parameters, type help to check correct parameters\n");
              }
            } else if (!StrCmp (L"WB", (CHAR16 *)ArgStr[0]) || !StrCmp (L"wb",(CHAR16 *)ArgStr[0])) {
              if (!InitFlag) {Print(L"Please run init command firstly\n"); continue;}
                if (ArgCount > 4) {
                  Status = EmmcWriteBlocks (
                                      Data,
                                      0,
                                      StrHexToUint64 ((CHAR16 *)ArgStr[1]),
                                      StrHexToUintn ((CHAR16 *)ArgStr[2]),
                                      (VOID *)StrHexToUintn ((CHAR16 *)ArgStr[3])
                
                                      );
                  if (!EFI_ERROR (Status)) {
                    Print (L"WriteBlocks success\n");
                  } else {
                    Print (L"WriteBlocks fail\n");
                  }
                }
              } else if (!StrCmp (L"SECUREERASE", (CHAR16 *)ArgStr[0]) || !StrCmp (L"secureerase",(CHAR16 *)ArgStr[0])) {
                if (!InitFlag) {
                  Print(L"Please run init command firstly\n"); 
                  continue;
                }
                Status = SecureErase ( Data );
           
              } else if (!StrCmp (L"TRIMERASE", (CHAR16 *)ArgStr[0]) || !StrCmp (L"trimerase",(CHAR16 *)ArgStr[0])) {
                if (!InitFlag) {
                  Print (L"Please run init command firstly\n"); 
                  continue;
                }
                Status = TrimErase ( Data,
                           StrHexToUintn ((CHAR16 *)ArgStr[1]),
                           StrHexToUintn ((CHAR16 *)ArgStr[2])
                           );
                
                if (!EFI_ERROR (Status)) {
                  Print (L"EraseBlocks success\n");
                } else {
                  Print (L"EraseBlocks fail\n");
                }
              } else {
                Print (L"Invalid Parameters, type help to check correct parameters\n");
              }
            } 
        
    Print( L"\r\n");
  
  } while (!Complete);
  ShellSetPageBreakMode (FALSE);
  return 0;
}


EFI_STATUS DefaultMode( EmmcCardData *Data,   
                            IN UINTN    Argc,
                            IN CHAR16 **Argv)
{
  EFI_EMMC_CARD_INFO_PROTOCOL *EfiEmmcApplicationRegister;
  EFI_BLOCK_IO_PROTOCOL  *BlockIo;
  EFI_STATUS Status;



  Status = gBS->LocateProtocol (
                  &gEfiEmmcCardInfoProtocolGuid,
                  NULL,
                  (VOID **)&EfiEmmcApplicationRegister
                  );
  if (EFI_ERROR (Status)) {
    Print(L"eMMC card driver is not there, please use emmc -i to use interactive mode!\n");
    return Status;
  }

  Status = gBS->HandleProtocol (
                  EfiEmmcApplicationRegister->CardData->Partitions->Handle,
                  &gEfiBlockIoProtocolGuid,
                  (VOID **)&BlockIo
                  );
  if (EFI_ERROR (Status)) {
    Print(L"No Block Io for eMMC card, please use emmc -i to use interactive mode!\n");
    return Status;
  }
  
  ShellSetPageBreakMode (TRUE);
  if (Argc == 1) {
    Print (L"Please see help message\n");
    Print (L"emmc.efi -help\n");
  } else {

    if (!StrCmp (L"-help", *(Argv + 1))) {
      DefaultHelpMessage ();
    } else if ((!StrCmp (L"INFO", *(Argv + 1))) || (!StrCmp (L"info", *(Argv + 1)))) {
      CopyMem(&Data->ExtCSDReg,&(EfiEmmcApplicationRegister->CardData->ExtCSDRegister), sizeof(Data->ExtCSDReg));
      CopyMem(&Data->CSDReg,&(EfiEmmcApplicationRegister->CardData->CSDRegister), sizeof(Data->CSDReg));
      CopyMem(&Data->CIDReg,&(EfiEmmcApplicationRegister->CardData->CIDRegister), sizeof(Data->CIDReg));
      CopyMem(&Data->OCRReg,&(EfiEmmcApplicationRegister->CardData->OCRRegister), sizeof(Data->OCRReg));
      EmmcInfo (
           Data
        );
      if (!EFI_ERROR (Status)) {
        Print (L"Info success\n");
      } else {
        Print (L"Info fail\n");
      }
    }  else if ((!StrCmp (L"CID", *(Argv + 1))) || (!StrCmp (L"cid", *(Argv + 1)))) {
      MmcDecodeCID (&(EfiEmmcApplicationRegister->CardData->CIDRegister));
    } else if ((!StrCmp (L"CSD", *(Argv + 1))) || (!StrCmp (L"csd", *(Argv + 1)))) {
      MmcDecodeCSD (&(EfiEmmcApplicationRegister->CardData->CSDRegister));
    } else if ((!StrCmp (L"ExtCSD", *(Argv + 1)))  || (!StrCmp (L"extcsd", *(Argv + 1)))) {
      MmcDecodeExtCSD (&(EfiEmmcApplicationRegister->CardData->ExtCSDRegister));
    } else if ((!StrCmp (L"UpExtCsd", *(Argv + 1)))  || (!StrCmp (L"upextcsd", *(Argv + 1)))) {
      Print(L"Start read ExtCSD:\n");
      MmcReadExtCsd(Data);
      Print(L"Start Decode ExtCSD\n");
      MmcDecodeExtCSD (&(EfiEmmcApplicationRegister->CardData->ExtCSDRegister));
    } else if (!StrCmp (L"STATUS", *(Argv + 1)) || !StrCmp (L"status", *(Argv + 1))) {       
      MmcUpdateCardStatus (Data);
      Print(L"Carddc Status is 0x%x\n", *(UINT32*)&Data->CardStatus);
    }   
    else if ((!StrCmp (L"OCR", *(Argv + 1))) || (!StrCmp (L"ocr", *(Argv + 1)))) {
      MmcDecodeOCR (&(EfiEmmcApplicationRegister->CardData->OCRRegister));

    } else if ((!StrCmp (L"Reset", *(Argv + 1))) || (!StrCmp (L"reset", *(Argv + 1)))) {
  
      Status = BlockIo->Reset (BlockIo, 0);
  
      if (!EFI_ERROR (Status)) {
        Print (L"Reset success\n");
      } else {
        Print (L"Reset fail\n");
      }
    } else if ((!StrCmp (L"RB", *(Argv + 1))) || (!StrCmp (L"rb", *(Argv + 1)))) {
      Status = BlockIo->ReadBlocks (
                          BlockIo,
                          BlockIo->Media->MediaId,
                          StrHexToUint64 (*(Argv + 2)),
                          StrHexToUintn (*(Argv + 3)),
                          (VOID *)StrHexToUintn (*(Argv + 4))
                          );
  
      if (!EFI_ERROR (Status)) {
        Print (L"ReadBlocks success\n");
      } else {
        Print (L"ReadBlocks fail\n");
      }
    } else if ((!StrCmp (L"WB", *(Argv + 1))) || (!StrCmp (L"wb", *(Argv + 1)))) {
      Status = BlockIo->WriteBlocks (
                          BlockIo,
                          BlockIo->Media->MediaId,
                          StrHexToUint64 (*(Argv + 2)),
                          StrHexToUintn (*(Argv + 3)),
                          (VOID *)StrHexToUintn (*(Argv + 4))
                          );
      if (!EFI_ERROR (Status)) {
        Print (L"WriteBlocks success\n");
      } else {
        Print (L"WriteBlocks fail\n");
      }
    } else if ((!StrCmp (L"FB", *(Argv + 1))) || (!StrCmp (L"fb", *(Argv + 1)))) {
      Status = BlockIo->FlushBlocks (BlockIo);
      Print (L"In this driver, this function just returns EFI_SUCCESS.\n");
      
      if (!EFI_ERROR (Status)) {
        Print (L"FlushBlocks success\n");
      } else {
        Print (L"FlushBlocks fail\n");
      }
    }
  }

  ShellSetPageBreakMode (FALSE);
  return 0;

}




/**
  The user Entry Point for Application. The user code starts with this function
  as the real entry point for the application.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.  
  @param[in] SystemTable    A pointer to the EFI System Table.
  
  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
INTN
EFIAPI
ShellAppMain (
  IN UINTN Argc,
  IN CHAR16 **Argv
  )
{

  EFI_STATUS Status;
  EmmcCardData *Data = &gEmmcData ;

  if (Argc == 1) {
    Print (L"Please see help message\n");
    Print (L"emmc.efi -help\n");
    return EFI_SUCCESS;
  } 


  Data->Address = 1;
  Status = gBS->LocateProtocol (       
                  &gEfiSdHostIoProtocolGuid,
                  NULL,
                  (VOID **)&Data->SdHostIo
                  );
  if (EFI_ERROR (Status)) {
    Print(L"No SdHost driver, Application is exiting!\n");
    return Status;
  }
  
  if (!StrCmp (L"-i", *(Argv + 1))) {
       InteractiveMode(Data);
       return EFI_SUCCESS;
     } 

  DefaultMode(Data, Argc, Argv);

  return EFI_SUCCESS;
}




