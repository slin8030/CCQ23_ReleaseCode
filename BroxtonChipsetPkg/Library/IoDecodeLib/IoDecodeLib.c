/** @file

;******************************************************************************
;* Copyright (c) 2016, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
#include <Uefi.h>
#include <ChipsetSvc.h>
#include <ChipsetAccess.h>
#include <IndustryStandard/Pci22.h>
#include <Library/MmPciLib.h>
#include <Library/PciExpressLib.h>
#include <Library/ScPlatformLib.h>

typedef struct {
  UINT16              IoType;
  UINT16              IoAddress;
  UINT16              IoDecMask;
  UINT16              IoDecValue;
  UINT16              IoDecEn;
} IO_DECODE_CONTROL_TABLE;

typedef struct {
  UINT16              StartAddress;
  UINT16              EndAddress;
  UINT16              IoDecodeType;
  UINT16              IoDecoedAddress;
} ADDRESS_TO_TYPE_TABLE;


ADDRESS_TO_TYPE_TABLE mAddressToTypeTable [] = {
// Start Address     End Address    IoDecodeType       IoDecoedAddress
  {0x370,            0x375,         IoDecodeFdd,       AddressFdd370},
  {0x377,            0x377,         IoDecodeFdd,       AddressFdd370},
  {0x3F0,            0x3F5,         IoDecodeFdd,       AddressFdd3F0},
  {0x3F7,            0x3F7,         IoDecodeFdd,       AddressFdd3F0},
  {0x378,            0x37F,         IoDecodeLpt,       AddressLpt378},
  {0x778,            0x77F,         IoDecodeLpt,       AddressLpt378},
  {0x278,            0x27F,         IoDecodeLpt,       AddressLpt278},
  {0x678,            0x67F,         IoDecodeLpt,       AddressLpt278},
  {0x3BC,            0x3BE,         IoDecodeLpt,       AddressLpt3BC},
  {0x7BC,            0x7BE,         IoDecodeLpt,       AddressLpt3BC},
  {0x3F8,            0x3FF,         IoDecodeComA,      AddressCom3F8},
  {0x2F8,            0x2FF,         IoDecodeComA,      AddressCom2F8},
  {0x220,            0x227,         IoDecodeComA,      AddressCom220},
  {0x228,            0x22F,         IoDecodeComA,      AddressCom228},
  {0x238,            0x23F,         IoDecodeComA,      AddressCom238},
  {0x2E8,            0x2EF,         IoDecodeComA,      AddressCom2E8},
  {0x338,            0x33F,         IoDecodeComA,      AddressCom338},
  {0x3E8,            0x3EF,         IoDecodeComA,      AddressCom3E8},
  {0x04E,            0x04E,         IoDecodeCnf2,      AddressCnf24E4F},
  {0x04F,            0x04F,         IoDecodeCnf2,      AddressCnf24E4F},
  {0x02E,            0x02E,         IoDecodeCnf1,      AddressCnf12E2F},
  {0x02F,            0x02F,         IoDecodeCnf1,      AddressCnf12E2F},
  {0x062,            0x062,         IoDecodeMc,        AddressMc6266},
  {0x066,            0x066,         IoDecodeMc,        AddressMc6266},
  {0x060,            0x060,         IoDecodeKbc,       AddressKbc6064},
  {0x064,            0x064,         IoDecodeKbc,       AddressKbc6064},
  {0x208,            0x20F,         IoDecodeGameh,     AddressGameh208},
  {0x200,            0x207,         IoDecodeGamel,     AddressGamel200},
};


IO_DECODE_CONTROL_TABLE mIoDecConTable[] = {
//{IoType,        IoAddress,       IoDecMask,          IoDecValue,             IoDecEn}
  {IoDecodeFdd,   AddressFdd3F0,   B_PCH_LPC_IOD_FDD,  0x0000,                 B_PCH_LPC_IOE_FDE},
  {IoDecodeFdd,   AddressFdd370,   B_PCH_LPC_IOD_FDD,  B_PCH_LPC_IOD_FDD,      B_PCH_LPC_IOE_FDE},
  {IoDecodeLpt,   AddressLpt378,   B_PCH_LPC_IOD_LPT,  0x0000,                 B_PCH_LPC_IOE_PPE},
  {IoDecodeLpt,   AddressLpt278,   B_PCH_LPC_IOD_LPT,  0x0100,                 B_PCH_LPC_IOE_PPE},
  {IoDecodeLpt,   AddressLpt3BC,   B_PCH_LPC_IOD_LPT,  0x0200,                 B_PCH_LPC_IOE_PPE},
  {IoDecodeComA,  AddressCom3F8,   B_PCH_LPC_IOD_COMA, V_PCH_LPC_IOD_COMA_3F8, B_PCH_LPC_IOE_CAE},
  {IoDecodeComA,  AddressCom2F8,   B_PCH_LPC_IOD_COMA, V_PCH_LPC_IOD_COMA_2F8, B_PCH_LPC_IOE_CAE},
  {IoDecodeComA,  AddressCom220,   B_PCH_LPC_IOD_COMA, V_PCH_LPC_IOD_COMA_220, B_PCH_LPC_IOE_CAE},
  {IoDecodeComA,  AddressCom228,   B_PCH_LPC_IOD_COMA, V_PCH_LPC_IOD_COMA_228, B_PCH_LPC_IOE_CAE},
  {IoDecodeComA,  AddressCom238,   B_PCH_LPC_IOD_COMA, V_PCH_LPC_IOD_COMA_238, B_PCH_LPC_IOE_CAE},
  {IoDecodeComA,  AddressCom2E8,   B_PCH_LPC_IOD_COMA, V_PCH_LPC_IOD_COMA_2E8, B_PCH_LPC_IOE_CAE},
  {IoDecodeComA,  AddressCom338,   B_PCH_LPC_IOD_COMA, V_PCH_LPC_IOD_COMA_338, B_PCH_LPC_IOE_CAE},
  {IoDecodeComA,  AddressCom3E8,   B_PCH_LPC_IOD_COMA, V_PCH_LPC_IOD_COMA_3E8, B_PCH_LPC_IOE_CAE},
  {IoDecodeComB,  AddressCom3F8,   B_PCH_LPC_IOD_COMB, V_PCH_LPC_IOD_COMB_3F8, B_PCH_LPC_IOE_CBE},
  {IoDecodeComB,  AddressCom2F8,   B_PCH_LPC_IOD_COMB, V_PCH_LPC_IOD_COMB_2F8, B_PCH_LPC_IOE_CBE},
  {IoDecodeComB,  AddressCom220,   B_PCH_LPC_IOD_COMB, V_PCH_LPC_IOD_COMB_220, B_PCH_LPC_IOE_CBE},
  {IoDecodeComB,  AddressCom228,   B_PCH_LPC_IOD_COMB, V_PCH_LPC_IOD_COMB_228, B_PCH_LPC_IOE_CBE},
  {IoDecodeComB,  AddressCom238,   B_PCH_LPC_IOD_COMB, V_PCH_LPC_IOD_COMB_238, B_PCH_LPC_IOE_CBE},
  {IoDecodeComB,  AddressCom2E8,   B_PCH_LPC_IOD_COMB, V_PCH_LPC_IOD_COMB_2E8, B_PCH_LPC_IOE_CBE},
  {IoDecodeComB,  AddressCom338,   B_PCH_LPC_IOD_COMB, V_PCH_LPC_IOD_COMB_338, B_PCH_LPC_IOE_CBE},
  {IoDecodeComB,  AddressCom3E8,   B_PCH_LPC_IOD_COMB, V_PCH_LPC_IOD_COMB_3E8, B_PCH_LPC_IOE_CBE},
  {IoDecodeCnf2,  AddressCnf24E4F, 0,                  0,                      B_PCH_LPC_IOE_ME2},
  {IoDecodeCnf1,  AddressCnf12E2F, 0,                  0,                      B_PCH_LPC_IOE_SE},
  {IoDecodeMc,    AddressMc6266,   0,                  0,                      B_PCH_LPC_IOE_ME1},
  {IoDecodeKbc,   AddressKbc6064,  0,                  0,                      B_PCH_LPC_IOE_KE},
  {IoDecodeGameh, AddressGameh208, 0,                  0,                      B_PCH_LPC_IOE_HGE},
  {IoDecodeGamel, AddressGamel200, 0,                  0,                      B_PCH_LPC_IOE_LGE},
};

/**
 Find a specified address belongs to specified address type or IoDecodeGenericIo.

 @param[in]         Address             Decode address
 @param[in]         Length              Decode length
 @param[out]        IoDecodeType        The type of Address
 @param[out]        IoDecodeAddress     The address of IoDecodeType
                                
 @retval            EFI_SUCCESS           Function returns successfully
*/
UINT16
ChipsetAddressToType (
  IN  UINT16             Address,
  IN  UINT16             Length,
  OUT IO_DECODE_TYPE     *IoDecodeType,
  OUT IO_DECODE_ADDRESS  *IoDecodeAddress
  )
{
  UINT8                        Index;
  UINTN                        LpcBaseAddr;
  UINT16                       LpcIoEnableDecoding;
  UINT16                       LpcIoDecodeRanges;
  UINT8                        ComPortAddress;

  Index                   = 0;
  LpcBaseAddr             = 0;
  LpcIoEnableDecoding     = 0;
  LpcIoDecodeRanges       = 0;
  ComPortAddress          = 0;
  *IoDecodeType           = IoDecodeGenericIo;


  for (Index = 0; Index < sizeof (mAddressToTypeTable) / sizeof (ADDRESS_TO_TYPE_TABLE); Index++) {
    if ((Address >= mAddressToTypeTable[Index].StartAddress) && 
        (Address <= mAddressToTypeTable[Index].EndAddress) &&
        (Address + Length  - 1 <= mAddressToTypeTable[Index].EndAddress)){
        *IoDecodeType    = mAddressToTypeTable[Index].IoDecodeType;
        *IoDecodeAddress = mAddressToTypeTable[Index].IoDecoedAddress;      
    }
  }

  //
  // COM PORT A/B
  //
  if (*IoDecodeType == IoDecodeComA) {
      LpcBaseAddr  = MmPciBase (
                       DEFAULT_PCI_BUS_NUMBER_SC,
                       PCI_DEVICE_NUMBER_PCH_LPC,
                       PCI_FUNCTION_NUMBER_PCH_LPC
                       );
      LpcIoEnableDecoding = MmioRead16 (LpcBaseAddr + R_PCH_LPC_IOE);
      LpcIoDecodeRanges = MmioRead16 (LpcBaseAddr + R_PCH_LPC_IOD);
      
      if ((LpcIoEnableDecoding & B_PCH_LPC_IOE_CAE) != 0) {
        switch ((LpcIoDecodeRanges & B_PCH_LPC_IOD_COMA) >> N_PCH_LPC_IOD_COMA){
        case V_PCH_LPC_IOD_COMA_3F8:
          ComPortAddress = AddressCom3F8;      
          break;
        case V_PCH_LPC_IOD_COMA_2F8:
          ComPortAddress = AddressCom2F8;
          break;
        case V_PCH_LPC_IOD_COMA_220:
          ComPortAddress = AddressCom220;
          break;
        case V_PCH_LPC_IOD_COMA_228:
          ComPortAddress = AddressCom228;
          break;
        case V_PCH_LPC_IOD_COMA_238:
          ComPortAddress = AddressCom238;
          break;
        case V_PCH_LPC_IOD_COMA_2E8:
          ComPortAddress = AddressCom2E8;
          break;
        case V_PCH_LPC_IOD_COMA_338:
          ComPortAddress = AddressCom338;
          break;
        case V_PCH_LPC_IOD_COMA_3E8:
          ComPortAddress = AddressCom3E8;
          break;      
        default:      
          break;
        }

        if (*IoDecodeAddress != ComPortAddress) {
          *IoDecodeType = IoDecodeComB;
        }
    }   
      
    if (((LpcIoEnableDecoding & B_PCH_LPC_IOE_CBE) != 0) && (*IoDecodeType == IoDecodeComB)) {
      switch ((LpcIoDecodeRanges & B_PCH_LPC_IOD_COMB) >> N_PCH_LPC_IOD_COMB){
      case V_PCH_LPC_IOD_COMB_3F8:
        ComPortAddress = AddressCom3F8;      
        break;
      case V_PCH_LPC_IOD_COMB_2F8:
        ComPortAddress = AddressCom2F8;
        break;
      case V_PCH_LPC_IOD_COMB_220:
        ComPortAddress = AddressCom220;
        break;
      case V_PCH_LPC_IOD_COMB_228:
        ComPortAddress = AddressCom228;
        break;
      case V_PCH_LPC_IOD_COMB_238:
        ComPortAddress = AddressCom238;
        break;
      case V_PCH_LPC_IOD_COMB_2E8:
        ComPortAddress = AddressCom2E8;
        break;
      case V_PCH_LPC_IOD_COMB_338:
        ComPortAddress = AddressCom338;
        break;
      case V_PCH_LPC_IOD_COMB_3E8:
        ComPortAddress = AddressCom3E8;
        break;      
      default:      
        break;
      }

      if (*IoDecodeAddress != ComPortAddress) {
        *IoDecodeType = IoDecodeGenericIo;
      }
    }
  }

  return EFI_SUCCESS;
}

/**
 IO address decode

 @param[in]         Type                Decode type
 @param[in]         Address             Decode address
 @param[in]         Length              Decode length
                                
 @retval            EFI_SUCCESS           Function returns successfully
 @retval            EFI_INVALID_PARAMETER One of the parameters has an invalid value
                                          1. Type has invalid value
                                          2. Address big then 0xFFFF
                                          3. Length need to be 4-byte aligned
 @retval            EFI_OUT_OF_RESOURCES  There are not enough resources available to set IO decode
*/
EFI_STATUS
ChipsetIoDecodeControl (
  IN IO_DECODE_TYPE     Type,
  IN IO_DECODE_ADDRESS  Address,
  IN UINT16             Length
  )
{
  EFI_STATUS                   Status;
  UINTN                        Index;
  UINT16                       IoDecodeRangesReg;
  UINT16                       IoDecodeRangesEnable;
  UINTN                        LpcBaseAddr;

  Status = EFI_INVALID_PARAMETER;


  if (Type == IoDecodeGenericIo) {
    Status = PchLpcGenIoRangeSet ((UINT16)Address, (UINTN) Length);
  } else {
    if ((Type < IoDecodeTypeMaximum) && (Address < IoDecodeAddressMaximum)) {
      LpcBaseAddr  = MmPciBase (
                       DEFAULT_PCI_BUS_NUMBER_SC,
                       PCI_DEVICE_NUMBER_PCH_LPC,
                       PCI_FUNCTION_NUMBER_PCH_LPC
                       );
      IoDecodeRangesReg = MmioRead16 (LpcBaseAddr + R_PCH_LPC_IOD);
      IoDecodeRangesEnable = MmioRead16 (LpcBaseAddr + R_PCH_LPC_IOE);
      for (Index = 0; Index < sizeof (mIoDecConTable) / sizeof (IO_DECODE_CONTROL_TABLE); Index++) {
        if ((Type == mIoDecConTable[Index].IoType) && (Address == mIoDecConTable[Index].IoAddress)) {
          IoDecodeRangesReg &= (~(mIoDecConTable[Index].IoDecMask));
          IoDecodeRangesReg |= (mIoDecConTable[Index].IoDecValue);
          if (Length == 0) {
            IoDecodeRangesEnable &= (~mIoDecConTable[Index].IoDecEn);
          } else {
            IoDecodeRangesEnable |= mIoDecConTable[Index].IoDecEn;
          }
          Status = EFI_SUCCESS;
          break;
        }
      }
      PchLpcIoDecodeRangesSet (IoDecodeRangesReg);
      PchLpcIoEnableDecodingSet (IoDecodeRangesEnable);
    }
  }

  return Status;
}

/**
 Set an IO address for SoC to decide it.

 @param[in]         Address             Decode address
 @param[in]         Length              Decode length
                                
 @retval            EFI_SUCCESS           Function returns successfully
 @retval            EFI_INVALID_PARAMETER One of the parameters has an invalid value
                                          1. Address big then 0xFFFF
                                          2. Length need to be 4-byte aligned
 @retval            EFI_OUT_OF_RESOURCES  There are not enough resources available to set IO decode
 @retval            EFI_UNSUPPORTED       Null library
*/
EFI_STATUS
IoDecodeControl (
  IN UINT16                    Address,
  IN UINT16                    Length
  ) 
{
  IO_DECODE_TYPE               IoDecodeType;
  IO_DECODE_ADDRESS            IoDecodeAddress;
  EFI_STATUS                   Status;

  ChipsetAddressToType (Address, Length, &IoDecodeType, &IoDecodeAddress);
  
  if (IoDecodeType == IoDecodeGenericIo) {
    Status = ChipsetIoDecodeControl (IoDecodeType, Address, Length);
  } else {
    Status = ChipsetIoDecodeControl (IoDecodeType, IoDecodeAddress, Length);
  }
  
  return Status;
}
