/** @file

Init SIO in PEI stage.

;******************************************************************************
;* Copyright (c) 2014-2015, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
**/

#include "SioInitPei.h"
#include <Library/CmosLib.h>

extern UINT16  mSioCfgPortList[];
extern UINT16  mSioCfgPortListSize;

static EFI_PEI_PPI_DESCRIPTOR mSioInitializedPpi = {
  (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gPeiSioInitializedPpiGuid,
  NULL
};

extern EFI_PEI_NOTIFY_DESCRIPTOR PeiSioInitNotifyList[];

/**
  SIO pei initial

  @param[in] FfsHeader            Pointer to EFI_FFS_FILE_HEADER
  @param[in] PeiServices          Pointer's pointer to EFI_PEI_SERVICES

  @retval     EFI_SUCCESS         The operation performed successfully.
**/
EFI_STATUS
EFIAPI
SioInitPeim (
  IN EFI_PEI_FILE_HANDLE         FileHandle,
  IN CONST EFI_PEI_SERVICES      **PeiServices
  )
{
  EFI_STATUS                     Status;
  UINT16                         IndexPort;
  UINT16                         DataPort;
  EFI_SIO_TABLE                  *OemTable;
  UINTN                          Index;
  UINT8                          SioInstance;

  OemTable    = (EFI_SIO_TABLE*)PCD_SIO_PEI_CUSTOMIZE_SETTING_TABLE;
  SioInstance = NULL_ID;

  PostCode (PEI_SIO_INIT); //PostCode = 0x70, Super I/O initial

  SioDecode ();

  // Get correct config port, if it's wrong, return fail.
  IndexPort = CheckDevice (&SioInstance);
  if (IndexPort == 0) {
    return EFI_UNSUPPORTED;
  }
  DataPort = IndexPort + 1;

  SioInitPs2KBMS (IndexPort, SioInstance);

  SioDebugPortInit (IndexPort);

  SioSpecificInit (IndexPort);

  for (Index = 0; Index < PCD_SIO_PEI_CUSTOMIZE_SETTING_TABLE_SIZE / sizeof (EFI_SIO_TABLE); Index++) {
    IoWrite8 (IndexPort, OemTable->Register);
    IoWrite8 (DataPort, OemTable->Value);
    OemTable++;
  }

  if (PeiSioInitNotifyList[0].Notify != NULL) {
    Status = (**PeiServices).NotifyPpi (PeiServices, PeiSioInitNotifyList);
  }

  //
  // Exit Config Mode
  //
  ExitConfigMode (IndexPort);
  Status = (**PeiServices).InstallPpi (
                             PeiServices,
                             &mSioInitializedPpi
                             );

  return Status;
}

/**
  Decode IO Port By Different Platform, Like AMD¡BINTEL¡K

  @retval     EFI_SUCCESS          The operation performed successfully.
**/
EFI_STATUS
SioDecode (
  VOID
  )
{
  //
  // Program and Enable SIO Base Addresses for Com1(3F8-3FF)/Com2(2F8-2FF)/EC(62/66)/KBC(60/64)
  // and enable decode range 2E,2F/4E,4F
  //
  SIO_DEVICE_LIST_TABLE              *TablePtr = NULL;

  TablePtr    = (SIO_DEVICE_LIST_TABLE*)PCD_SIO_CONFIG_TABLE;

  while (!((TablePtr->TypeH == NONE_ID) && (TablePtr->TypeL == NONE_ID))) {
    if (TablePtr->DeviceEnable){
      IoDecodeControl (TablePtr->DeviceBaseAdr,  TablePtr->DeviceSize);
    }
    TablePtr++;
  }

  return EFI_SUCCESS;
}

/**
  To get correct SIO data.

  @param[out]  *SioInstance         if success, return which SIO instance pointer.

  @retval     IndexPort             return value as zero, it means "not match"
                                    return value is not zero, it means "match".
**/
UINT16
CheckDevice (
  OUT UINT8           *SioInstance
  )
{
  SIO_DEVICE_LIST_TABLE              *mTablePtr;
  UINT16                             IndexPort;

  IndexPort = 0;
  mTablePtr  = (SIO_DEVICE_LIST_TABLE *)PCD_SIO_CONFIG_TABLE;

  //
  // Calculate the number of non-zero entries in the table
  //
  while (!((mTablePtr->TypeH == NONE_ID) && (mTablePtr->TypeL == NONE_ID))) {
    if (mTablePtr->Device == CFG_ID){
      CheckConfig (mTablePtr, &IndexPort);
      if (IndexPort != 0) {
        *SioInstance = mTablePtr->TypeInstance;
        return IndexPort;
      }
    }
    mTablePtr++;
  }
  return IndexPort;
}

/**
  Check ID of SIO whether it's correct.

  @param[in]  *DeviceTable          SIO_DEVICE_LIST_TABLE from PCD structure.
  @param[in]  *IndexPort            SIO config port.

  @retval     IndexPort             return value as zero, it means "not match".
**/
VOID
CheckConfig (
  IN SIO_DEVICE_LIST_TABLE      *DeviceTable,
  IN UINT16                     *Port
  )
{
  UINT16                IndexPort;
  UINT16                DataPort;
  UINT8                 Index;

  IndexPort = DeviceTable->DeviceBaseAdr;
  DataPort  = IndexPort + 1;

  if (IndexPort != 0) {
    //
    // Enter Config Mode
    //
    EnterConfigMode (IndexPort);

    IoWrite8 (IndexPort, SIO_CHIP_ID1);
    if (IoRead8 (DataPort) != DeviceTable->TypeH) {
      IndexPort = 0;
    }

    if ((DeviceTable->TypeL != NONE_ID) && (IndexPort != 0)) {
      IoWrite8 (IndexPort, SIO_CHIP_ID2);
      if (IoRead8 (DataPort) != DeviceTable->TypeL) {
        IndexPort = 0;
      }
    }
  }

  if (IndexPort == 0) {
    for (Index = 0; Index < mSioCfgPortListSize; Index++) {
      IndexPort = mSioCfgPortList[Index];
      DataPort  = IndexPort + 1;

      //
      // Enter Config Mode
      //
      EnterConfigMode (IndexPort);

      IoWrite8 (IndexPort, SIO_CHIP_ID1);
      if (IoRead8 (DataPort) == DeviceTable->TypeH) {
        if (DeviceTable->TypeL != NONE_ID) {
          IoWrite8 (IndexPort, SIO_CHIP_ID2);
          if (IoRead8 (DataPort) == DeviceTable->TypeL) {
            break;
          }
        } else {
          break;
        }
      }
    }

    if (Index == mSioCfgPortListSize) {  // if true, means not found SIO ID
      IndexPort = 0;
    }
  }

  WriteCmos16 (0x6C, IndexPort);
  IoWrite8 (IndexPort, SIO_CHIP_ID2);
  WriteCmos8 (0x6E, IoRead8 (DataPort));

  *Port = IndexPort;
}

