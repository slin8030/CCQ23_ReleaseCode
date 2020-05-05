/** @file
  IoDecodeLib F71811 Code, need to be modified for different chipset support

;******************************************************************************
;* Copyright (c) 2016-2017, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
#include <Base.h>
#include <Uefi.h>
#include <ChipsetAccess.h>
#include <ChipsetSvc.h>
#include <Library/DebugLib.h>
#include <Library/MmPciLib.h>
#include <Library/PchCycleDecodingLib.h>
#include <Library/PchPcrLib.h>

#define MAX_DECODE_NUMBER    4
#define MAX_DECODE_LENGTH    0x100
#define DECODE_ALIGNMENT     0x4
#define ADDRESS_MASK         0xFF00

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
  {0x04E,            0x04F,         IoDecodeCnf2,      AddressCnf24E4F},
  {0x02E,            0x02F,         IoDecodeCnf1,      AddressCnf12E2F},
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
  {IoDecodeComB,  AddressCom3F8,   B_PCH_LPC_IOD_COMB, V_PCH_LPC_IOD_COMB_3F8 << 4, B_PCH_LPC_IOE_CBE},
  {IoDecodeComB,  AddressCom2F8,   B_PCH_LPC_IOD_COMB, V_PCH_LPC_IOD_COMB_2F8 << 4, B_PCH_LPC_IOE_CBE},
  {IoDecodeComB,  AddressCom220,   B_PCH_LPC_IOD_COMB, V_PCH_LPC_IOD_COMB_220 << 4, B_PCH_LPC_IOE_CBE},
  {IoDecodeComB,  AddressCom228,   B_PCH_LPC_IOD_COMB, V_PCH_LPC_IOD_COMB_228 << 4, B_PCH_LPC_IOE_CBE},
  {IoDecodeComB,  AddressCom238,   B_PCH_LPC_IOD_COMB, V_PCH_LPC_IOD_COMB_238 << 4, B_PCH_LPC_IOE_CBE},
  {IoDecodeComB,  AddressCom2E8,   B_PCH_LPC_IOD_COMB, V_PCH_LPC_IOD_COMB_2E8 << 4, B_PCH_LPC_IOE_CBE},
  {IoDecodeComB,  AddressCom338,   B_PCH_LPC_IOD_COMB, V_PCH_LPC_IOD_COMB_338 << 4, B_PCH_LPC_IOE_CBE},
  {IoDecodeComB,  AddressCom3E8,   B_PCH_LPC_IOD_COMB, V_PCH_LPC_IOD_COMB_3E8 << 4, B_PCH_LPC_IOE_CBE},
  {IoDecodeCnf2,  AddressCnf24E4F, 0,                  0,                      B_PCH_LPC_IOE_ME2},
  {IoDecodeCnf1,  AddressCnf12E2F, 0,                  0,                      B_PCH_LPC_IOE_SE},
  {IoDecodeMc,    AddressMc6266,   0,                  0,                      B_PCH_LPC_IOE_ME1},
  {IoDecodeKbc,   AddressKbc6064,  0,                  0,                      B_PCH_LPC_IOE_KE},
  {IoDecodeGameh, AddressGameh208, 0,                  0,                      B_PCH_LPC_IOE_HGE},
  {IoDecodeGamel, AddressGamel200, 0,                  0,                      B_PCH_LPC_IOE_LGE},
};

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
  UINT8                        FddPortAddress;
  UINT8                        LptPortAddress;

  Index                   = 0;
  LpcBaseAddr             = 0;
  LpcIoEnableDecoding     = 0;
  LpcIoDecodeRanges       = 0;
  ComPortAddress          = 0;
  FddPortAddress          = 0;
  LptPortAddress          = 0;
  *IoDecodeType           = IoDecodeGenericIo;


  for (Index = 0; Index < sizeof(mAddressToTypeTable) / sizeof(ADDRESS_TO_TYPE_TABLE); Index++) {
    if ((Address >= mAddressToTypeTable[Index].StartAddress) &&
        (Address <= mAddressToTypeTable[Index].EndAddress) &&
        (Address + Length  - 1 <= mAddressToTypeTable[Index].EndAddress)){
        *IoDecodeType    = mAddressToTypeTable[Index].IoDecodeType;
        *IoDecodeAddress = mAddressToTypeTable[Index].IoDecoedAddress;
    }
  }

  LpcBaseAddr  = MmPciBase (
             DEFAULT_PCI_BUS_NUMBER_PCH,
             PCI_DEVICE_NUMBER_PCH_LPC,
             PCI_FUNCTION_NUMBER_PCH_LPC
             );
  LpcIoEnableDecoding = MmioRead16 (LpcBaseAddr + R_PCH_LPC_IOE);
  LpcIoDecodeRanges = MmioRead16 (LpcBaseAddr + R_PCH_LPC_IOD);

  //
  // COM PORT A/B
  //
  if (*IoDecodeType == IoDecodeComA) {

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

  //
  // FDD
  //
  if (*IoDecodeType == IoDecodeFdd) {
    if ((LpcIoEnableDecoding & B_PCH_LPC_IOE_FDE) != 0) {
      switch ((LpcIoDecodeRanges & B_PCH_LPC_IOD_FDD) >> N_PCH_LPC_IOD_FDD) {
      case V_PCH_LPC_IOD_FDD_3F0:
        FddPortAddress = AddressFdd3F0;
        break;
      case V_PCH_LPC_IOD_FDD_370:
        FddPortAddress = AddressFdd370;
        break;
      default:
        break;
      }

      if (*IoDecodeAddress != FddPortAddress) {
        *IoDecodeType = IoDecodeGenericIo;
      }
    }
  }

  //
  // LPT
  //
  if (*IoDecodeType == IoDecodeLpt) {
    if ((LpcIoEnableDecoding & B_PCH_LPC_IOE_PPE) != 0) {
      switch ((LpcIoDecodeRanges & B_PCH_LPC_IOD_LPT) >> N_PCH_LPC_IOD_LPT){
      case V_PCH_LPC_IOD_LPT_378:
        LptPortAddress = AddressLpt378;
        break;
      case V_PCH_LPC_IOD_LPT_278:
        LptPortAddress = AddressLpt278;
        break;
      case V_PCH_LPC_IOD_LPT_3BC:
        LptPortAddress = AddressLpt3BC;
        break;
      default:
        break;
      }

      if (*IoDecodeAddress != LptPortAddress) {
        *IoDecodeType = IoDecodeGenericIo;
      }
    }
  }

  return EFI_SUCCESS;
}

STATIC
BOOLEAN
CompletelyCover (
  IN UINT16 AddressA,
  IN UINT16 LengthA,
  IN UINT16 AddressB,
  IN UINT16 LengthB
  )
{
  if (AddressA <= AddressB && AddressB + LengthB <= AddressA + LengthA) {
    return TRUE;
  }

  return FALSE;
}

STATIC
VOID
LpcIoDecode (
  PCH_LPC_GEN_IO_RANGE_LIST LpcList
  )
{
  UINT8  Index;
  UINT16 Address;
  UINT16 Length;
  UINT32 Data32;
  UINTN  LpcBase;

  PchPcrRead32 (PID_DMI, R_PCH_PCR_DMI_DMIC, &Data32);
  if ((Data32 & B_PCH_PCR_DMI_DMIC_SRL) != 0) {
    ASSERT (FALSE);
    return;
  }

  for (Index = 0; Index < PCH_LPC_GEN_IO_RANGE_MAX; ++Index) {
    if (!LpcList.Range[Index].Enable) {
      continue;
    }

    Address = (UINT16)LpcList.Range[Index].BaseAddr;
    Length = (UINT16)LpcList.Range[Index].Length;

    //
    // Program LPC/eSPI generic IO range register accordingly.
    //
    LpcBase = MmPciBase (
                DEFAULT_PCI_BUS_NUMBER_PCH,
                PCI_DEVICE_NUMBER_PCH_LPC,
                PCI_FUNCTION_NUMBER_PCH_LPC
                );
    Data32 =  (UINT32) (((Length - 1) << 16) & B_PCH_LPC_GENX_DEC_IODRA);
    Data32 |= (UINT32) Address;
    Data32 |= B_PCH_LPC_GENX_DEC_EN;
    //
    // Program LPC/eSPI PCI Offset 84h ~ 93h of Mask, Address, and Enable.
    //
    MmioWrite32 (
      LpcBase + R_PCH_LPC_GEN1_DEC + Index * 4,
      Data32
      );
    //
    // Program LPC Generic IO Range #, PCR[DMI] + 2730h ~ 273Fh to the same value programmed in LPC/eSPI PCI Offset 84h~93h.
    //
    PchPcrWrite32 (
      PID_DMI, (UINT16) (R_PCH_PCR_DMI_LPCLGIR1 + Index * 4),
      Data32
      );
  }
}

STATIC
EFI_STATUS
GenIoRangeSet (
  UINT16 Address,
  UINTN  Length
  )
{
  PCH_LPC_GEN_IO_RANGE_LIST LpcList;
  UINT8                     LpcIndex;
  UINT16                    LpcLength;
  UINT16                    LpcAddress;
  UINT16                    BaseAddress;
  UINT16                    NewLength;
  UINT16                    NewAddress;
  UINT16                    TempLength;

  PchLpcGenIoRangeGet (&LpcList);
  LpcAddress = 0;
  LpcLength = 0;
  BaseAddress = 0;
  NewLength = 0;

  //
  // find out which decoder has already had the same base
  //
  for (LpcIndex = 0; LpcIndex < PCH_LPC_GEN_IO_RANGE_MAX; ++LpcIndex) {
    if (!LpcList.Range[LpcIndex].Enable) {
      LpcAddress = Address;
      LpcLength = (UINT16)Length;
      BaseAddress = LpcAddress & ADDRESS_MASK;
      break;
    }
    LpcAddress = (UINT16)LpcList.Range[LpcIndex].BaseAddr;
    LpcLength = (UINT16)LpcList.Range[LpcIndex].Length;
    BaseAddress = LpcAddress & ADDRESS_MASK;

    if ((Address & ADDRESS_MASK) == BaseAddress) {
      break;
    }
  }

  //
  // all decoder has been used, and none of them has the same base as the
  // requested address
  //
  if (LpcIndex == PCH_LPC_GEN_IO_RANGE_MAX) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // find best Address
  // it is guaranteed to find one, since it can just be
  // BaseAddress with Length MAX_DECODE_LENGTH
  //
  NewAddress = LpcAddress > Address ? Address : LpcAddress;
  for (; NewAddress >= BaseAddress; NewAddress -= NewLength) {
    //
    // get the length that sastifies:
    // 1. it is power of 2
    // 2. Address align with it
    // 3. no bigger than MAX_DECODE_LENGTH
    // 4. no smaller than DECODE_ALIGNMENT
    // 5. as big as possible while maintaining above conditions
    //
    NewLength = NewAddress ^ (NewAddress - 1);
    NewLength >>= 1;
    NewLength &= MAX_DECODE_LENGTH - 1;
    ++NewLength;
    if (!CompletelyCover (NewAddress, NewLength, Address, (UINT16)Length)) {
      continue;
    }
    if (!CompletelyCover (NewAddress, NewLength, LpcAddress, LpcLength)) {
      continue;
    }
    break;
  }

  //
  // find the shortest length that still covers all ranges
  //
  for (TempLength = NewLength; TempLength >= DECODE_ALIGNMENT; TempLength >>= 1) {
    if (!CompletelyCover (NewAddress, TempLength, Address, (UINT16)Length)) {
      break;
    }
    if (!CompletelyCover (NewAddress, TempLength, LpcAddress, LpcLength)) {
      break;
    }
    NewLength = TempLength;
  }

  LpcList.Range[LpcIndex].Enable = TRUE;
  LpcList.Range[LpcIndex].BaseAddr = NewAddress;
  LpcList.Range[LpcIndex].Length = NewLength;
  LpcIoDecode (LpcList);
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
  UINT16                       IoDecodeRangesReg,IoDecodeRangesEnable;

  Status = EFI_INVALID_PARAMETER;

  if (Type == IoDecodeGenericIo) {
    Status = GenIoRangeSet ((UINT16)Address, (UINTN) Length);
  } else {
    if ((Type < IoDecodeTypeMaximum) && (Address < IoDecodeAddressMaximum)) {
      IoDecodeRangesReg = PchLpcPciCfg16 (R_PCH_LPC_IOD);
      IoDecodeRangesEnable = PchLpcPciCfg16 (R_PCH_LPC_IOE);
      for (Index = 0; Index < sizeof(mIoDecConTable) / sizeof(IO_DECODE_CONTROL_TABLE); Index++) {
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


EFI_STATUS
IoDecodeControl (
  IN UINT16                    Address,
  IN UINT16                    Length
  )
{
  IO_DECODE_TYPE               IoDecodeType;
  IO_DECODE_ADDRESS            IoDecodeAddress;

  ChipsetAddressToType (Address, Length, &IoDecodeType, &IoDecodeAddress);

  if (IoDecodeType == IoDecodeGenericIo) {
    ChipsetIoDecodeControl (IoDecodeType, Address, Length);
  } else {
    ChipsetIoDecodeControl (IoDecodeType, IoDecodeAddress, Length);
  }

  return EFI_SUCCESS;
}
