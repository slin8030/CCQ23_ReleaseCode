/** @file
 H2O Oem feature library implement code.

 This c file contains H2O Oem feature library instance for SMM phase.

;******************************************************************************
;* Copyright (c) 2017, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************

*/
//[-start-180412-IB09330446-add]//
#include <Library/IpmiOemFeatureLib.h>
//[-end-180412-IB09330446-add]//
  
/**
 Set Dimm error to BMS to turn on Dimm error LED.

 @param[in]        socket               Dimm Error CPU number;
 @param[in]        ch                   Dimm Error Channel number.
 @param[in]        dimm                 Dimm Error Dimm number.

 @retval EFI_UNSUPPORTED                Default Null lib return EFI_UNSUPPORTED.

*/

EFI_STATUS
EFIAPI
IpmiBmcFeatureLibSendDimmError (
  UINT8    socket,
  UINT8    ch,
  UINT8    dimm
  )
{
  return EFI_UNSUPPORTED;
}

//[-start-180412-IB09330446-add]//
/**
 Get Nic Information from BMC.

 @param[in]        InfoIndex            Nic Index

 @retval EFI_SUCCESS                    Get NIC information from BMC success.
 @retval Others                         Get NIC information from BMC fail.
*/
EFI_STATUS
EFIAPI
IpmiBmcFeatureLibGetNicInfo (
  IN  UINT8     InfoIndex,
  OUT VOID      *NicInfo
  )
{
  return EFI_UNSUPPORTED;
}

/**
 Set Dimm presence bit map to BMC.
*/
VOID
IpmiBmcFeatureLibSetDimmBitMap (
  VOID
  )
{
  return;
}

/**
 Get Fw version from BMC.

 @param[in]         IdTag                ID for Fw version, define in IPMI_BMC_ID_TAG
 @param[out]        VersionArray         Fw information for IdTag, define at IPMI_BMC_FW_VERSION_INFO

 @retval EFI_SUCCESS                     Get Fw Version from BMC success.
 @retval Others                          Get Fw Version from BMC fail.
*/
EFI_STATUS
EFIAPI
IpmiBmcFeatureLibGetFwVersion (
  IN  UINT8           IdTag,
  OUT VOID            *VersionArray
  )
{
  return EFI_UNSUPPORTED;
}

/**
 Get Security Version command from BMC.

 @param[in]         IdTag                ID for Security version, define in IPMI_SECURITY_ID_TAG
 @param[out]        VersionArray         Fw information for IdTag, define at IPMI_BMC_SECURITY_VERSION_INFO

 @retval EFI_SUCCESS                     Get Security Version from BMC success.
 @retval Others                          Get Security Version from BMC fail.
*/
EFI_STATUS
EFIAPI
IpmiBmcFeatureLibGetSecurityVersion (
  IN  UINT8           IdTag,
  OUT VOID            *VersionArray
  )
{
  return EFI_UNSUPPORTED;
}

/**
 Set HDD Fault command to BMC.
 Get HDD Fault org status from BMC, and set current HDD fault to BMC.

 @param[in]        HddNum                 Fault HDD Number

 @retval EFI_SUCCESS                      Set HDD num to fault to BMC success.
 @retval Others                           Set HDD num to fault to BMC Fail.
*/
EFI_STATUS
EFIAPI
IpmiBmcFeatureLibSetHddFault (
  IN  UINT8           HddNum
  )
{
  return EFI_UNSUPPORTED;
}

/**
 Get CPLD Revision from BMC.

 @param[out]        CpldRevision            Return CPLD Revision, define in IPMI_BMC_CPLD_REVISION

 @retval EFI_SUCCESS                        Get CPLD Revision from BMC success.
 @retval Others                             Get CPLD Revision from BMC fail.
*/
EFI_STATUS
EFIAPI
IpmiBmcFeatureLibGetCpldRevision (
  OUT  UINT8           *CpldRevision
  )
{
  return EFI_UNSUPPORTED;
}

/**
 Get BMC Execution Context from BMC.

 @param[out]        CurrentExecutionContext   Current Context, define in IPMI_BMC_CURRENT_EXECUTION_CONTEXT
 @param[out]        PartitionPointer          Partition Pointer, definein IPMI_BMC_PARTITION_POINTER

 @retval EFI_SUCCESS                          Get BMC Execution Context from BMC success.
 @retval Others                               Get BMC Execution Context from BMC fail.
*/
EFI_STATUS
EFIAPI
IpmiBmcFeatureLibGetExecutionContext (
  OUT  UINT8           *CurrentExecutionContext,
  OUT  UINT8           *PartitionPointer
  )
{
  return EFI_UNSUPPORTED;
}

/**
 Set Restore Configuration command to BMC to reset Configuration.

 @param[in]         Operation           Restore Operation, define in IPMI_BMC_RESTORE_OPERATION
 @param[out]        RestoreStatus       Restore Status.
 
 @retval EFI_SUCCESS                    Set Restore Configuration to BMC success.
 @retval Others                         Set Restore Configuration to BMC fail.
*/
EFI_STATUS
EFIAPI
IpmiBmcFeatureLibRestoreConfiguration (
  IN  IPMI_BMC_RESTORE_OPERATION                Operation,
  OUT UINT8                                     *RestoreStatus
  )
{
  return EFI_UNSUPPORTED;
}

/**
 Get OEM Device Info from BMC.

 @param[in]         Device              Get Info of device, define in IPMI_BMC_OEM_DEVICE
 @param[out]        DeviceInfo          If device is BMC_OEM_DEVICE_INFO_BMC return IPMI_BMC_DEVICE_INFO
                                        If device is BMC_OEM_DEVICE_INFO_SDR return IPMI_BMC_SDR_INFO

 @retval EFI_SUCCESS                    Get Oem Device Info from BMC success.
 @retval Others                         Get Oem Device Info from BMC fail.
*/
EFI_STATUS
EFIAPI
IpmiBmcFeatureGetOemDeviceInfo (
  IN  UINT8                                     Device,
  IN  UINT8                                     *RecvLength,
  OUT UINT8                                     *DeviceInfo
  )
{
  return EFI_UNSUPPORTED;
}

/**
 Set Processor Tcontrol command to BMC.

 @param[in]         CpuNum              CPU Number
 @param[in]         TcontrolValue       In units of 64
 
 @retval EFI_SUCCESS                    Set Restore Configuration to BMC success.
 @retval Others                         Set Restore Configuration to BMC fail.
*/
EFI_STATUS
EFIAPI
IpmiBmcFeatureLibSetProcessorTcontrol (
  IN  UINT8                                     CpuNum,
  IN  UINT8                                     TcontrolValue
  )
{
  return EFI_UNSUPPORTED;
}

/**
 Set System GUID command to BMC.

 @param[in]         Guid                System GUID
 
 @retval EFI_SUCCESS                    Set System GUID to BMC success.
 @retval Others                         Set System GUID to BMC fail.
*/
EFI_STATUS
EFIAPI
IpmiBmcFeatureLibSetSystemGuid (
  IN  EFI_GUID                                     *Guid
  )
{
  return EFI_UNSUPPORTED;
}

/**
 Enable/Disable BMC System Reset Action command to BMC.

 @param[in]         ResetAction         Enable/Disable Reset action
 
 @retval EFI_SUCCESS                    Set BMC System Reset to BMC success.
 @retval Others                         Set BMC System Reset to BMC fail.
*/
EFI_STATUS
EFIAPI
IpmiBmcFeatureLibSetBmcSystemReset (
  IN  BOOLEAN                               *ResetAction
  )
{
  return EFI_UNSUPPORTED;
}

/**
 Get BMC System Reset Action command from BMC.

 @param[out]         ResetAction        Current Reset action
 
 @retval EFI_SUCCESS                    Get BMC System Reset to BMC success.
 @retval Others                         Get BMC System Reset to BMC fail.
*/
EFI_STATUS
EFIAPI
IpmiBmcFeatureLibGetBmcSystemReset (
  OUT  BOOLEAN                               *ResetAction
  )
{
  return EFI_UNSUPPORTED;
}

/**
 Get OEM SEL Info. Returns the number of entries in the SEL, SEL command version, and the timestamp for the most
 recent entry and delete/clear.

 @param[out]        SelInfo            A pointer to H2O_IPMI_OEM_SEL_INFO structure.

 @return Execute Lib IPMI Command Status.
*/
EFI_STATUS
IpmiBmcFeatureLibGetSelInfo (
  OUT H2O_IPMI_OEM_SEL_INFO             *SelInfo,
  IN  BOOLEAN                           IsHeci
  )
{
  return EFI_UNSUPPORTED;
}

/**
 Get OEM SEL Allocation Info. 

 @param[out]        SelAllocInfo            A pointer to H2O_IPMI_OEM_SEL_ALLOC_INFO structure.

 @return Execute Lib IPMI Command Status.
*/
EFI_STATUS
IpmiBmcFeatureLibGetSelAllocInfo (
  OUT H2O_IPMI_OEM_SEL_ALLOC_INFO       *SelAllocInfo,
  IN  BOOLEAN                           IsHeci
  )
{
  return EFI_UNSUPPORTED;
}

/**
 Set OEM SEL Entry. Used to retrieve entries entire record from the SEL.

 @param[in]         SelRecordId         SEL Record ID.
 @param[out]        SelEntry            A pointer to H2O_IPMI_OEM_SEL_ENTRY structure.

 @return Execute Lib IPMI Command Status.
*/
EFI_STATUS
IpmiBmcFeatureLibSetSelEntry (
  IN  H2O_IPMI_OEM_SEL_ENTRY            *SelEntry,
  IN  BOOLEAN                           IsHeci
  )
{
  return EFI_UNSUPPORTED;
}

/**
 Get OEM SEL Entry. Used to retrieve entries entire record from the SEL.

 @param[in]         SelRecordId         SEL Record ID.
 @param[out]        SelEntry            A pointer to H2O_IPMI_OEM_SEL_ENTRY structure.

 @return Execute Lib IPMI Command Status.
*/
EFI_STATUS
IpmiBmcFeatureLibGetSelEntry (
  IN  UINT16                            SelRecordId,
  OUT H2O_IPMI_OEM_SEL_ENTRY            *SelEntry,
  IN  BOOLEAN                           IsHeci
  )
{
  return EFI_UNSUPPORTED;
}

/**
 Set OEM Platform Event Message.

 @param[in]         EventMessage        Platform Event Message.

 @return Execute Lib IPMI Command Status.
*/
EFI_STATUS
IpmiBmcFeatureLibSetPlatformEventMessage (
  IN  H2O_IPMI_OEM_EVENT_MESSAGE        *EventMessage,
  IN  BOOLEAN                           IsHeci
  )
{
  return EFI_UNSUPPORTED;
}

/**
 Get Voltage Name.

 @param[in]         VoltageSensorNum    Sensor Number of want to get nmae.
 @param[out]        NameString          Voltage Name String.

 @return Execute Lib IPMI Command Status.
*/
EFI_STATUS
IpmiBmcFeatureLibGetVoltageName (
  IN  UINT8                             VoltageSensorNum,
  OUT CHAR8                             **NameString,
  IN  BOOLEAN                           IsHeci
  )
{
  return EFI_UNSUPPORTED;
}

/**
 Get Platform Configuration. input key sting and get back value string

 @param[out]        String          Voltage Name String.

 @return Execute Lib IPMI Command Status.
*/
EFI_STATUS
IpmiBmcFeatureLibGetPlatformConfiguration (
  OUT CHAR8                          *String
  )
{
  return EFI_UNSUPPORTED;
}

/**
 Send BMCDataRegionStatus command to the BMC.
  
 @param[in out]        ResponseData          Response of Send Data Region Status.
 @param[in]            ResponseSize          Response size.

 @return Execute Lib IPMI Command Status.

*/
EFI_STATUS 
IpmiBmcFeatureLibSendDataRegionStatusCommandToBMC (
  IN OUT UINT8      *ResponseData,
  IN UINTN          ResponseSize
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Send BMCDataRegionUpdateComplete command to the BMC.

  @param[out] SessionLockHandle
  
  @return Execute Lib IPMI Command Status.
*/

EFI_STATUS
IpmiBmcFeatureLibSendDataRegionUpdateCompleteCommandToBMC (
  IN  UINT8         SessionLockHandle,
  IN OUT UINT8      *ResponseData,
  IN UINTN          ResponseSize
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Send BMCDataRegionRrite command to get Region Data from BMC.

  @param[in]            RequestOffset         Request Offset
  @param[in out]        ResponseData          Response of Send Data Region Status.
  @param[in]            ResponseSize          Response size.
  
  @return Execute Lib IPMI Command Status.
*/
EFI_STATUS
IpmiBmcFeatureLibSendDataRegionReadCommandToBMC (
  IN        UINT16             ReadOffset,
  IN OUT    UINT8              *ReadLength,
  OUT       UINT8              *ReadBuf
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Send BMCDataRegionWrite command to the BMC.

  @param[in]            SessionLockHandle     Session Lock Handle
  @param[in]            RequestData           Request data of Data Region write
  @param[in]            RequestLength         Request data length
  @param[in]            RequestOffset         Request Offset
  @param[in out]        ResponseData          Response of Send Data Region Status.
  @param[in]            ResponseSize          Response size.
  
  @return Execute Lib IPMI Command Status.
*/
EFI_STATUS
IpmiBmcFeatureLibSendDataRegionWriteCommandToBMC (
  IN UINT8          SessionLockHandle,
  IN UINT8          *RequestData,
  IN UINT8          RequestLength,
  IN UINT16         RequestOffset,
  IN OUT UINT8      *ResponseData,
  IN UINTN          ResponseSize
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Send BMCDataRegionLock command to the BMC.

  @param[in]            LockType              Lock Type
  @param[in out]        ResponseData          Response of Send Data Region Status.
  @param[in]            ResponseSize          Response size.

  @return Execute Lib IPMI Command Status.
*/
EFI_STATUS
IpmiBmcFeatureLibSendDataRegionLockCommandToBMC (
  IN UINT8          LockType,
  IN OUT UINT8      *ResponseData,
  IN UINTN          ResponseSize
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Send BIOS id to BMC(seperateOEM command).

  @param[in]    Type0    Poniter to SMBIOS type 0 structure.

  @return Execute Lib IPMI Command Status.
*/

EFI_STATUS
IpmiBmcFeatureLibSendBiosIdtoBmc (
  SMBIOS_STRUCTURE        *Type0
  )
{
  return EFI_UNSUPPORTED;
}

/**
 Send Post end command to BMC when ready to boot.

 @retval EFI_SUCCESS                    Send IPMI command success.
 @retval Others                         Send IPMI command fail.
*/
EFI_STATUS
IpmiBmcFeatureLibSendBmcPostEnd (
  VOID
  )
{
  return EFI_UNSUPPORTED;
}

/**
 Send System Firmware Version command to BMC.

 @param[in]         FirmwareVersionStr  Firmware Version string.
 @param[in]         TotalLength         Firmware Version string length

 @retval EFI_SUCCESS                    Send IPMI command success.
 @retval Others                         Send IPMI command fail.
*/
EFI_STATUS
IpmiBmcFeatureLibSetSystemFirmwareVersion (
  IN  CHAR8                           *FirmwareVersionStr,
  IN  UINTN                           TotalLength
  )
{
  return EFI_UNSUPPORTED;
}

/**
 Send Ipmi exit flash command BMC.

 @retval TRUE                           Need to do BIOS update.
 @retval FALSE                          Do nothing.
*/
EFI_STATUS
IpmiBmcFeatureLibSendExitFlashBios (
  VOID
  )
{
  return EFI_UNSUPPORTED;
}

/**
 Send BIOS Flash status and percent command to BMC.

 @param[in]         FlashStatus         BIOS Flash status.
 @param[in]         Percent             BIOS Flash percent

 @retval EFI_SUCCESS                    Send IPMI command success.
 @retval Others                         Send IPMI command fail.
*/
EFI_STATUS
IpmiBmcFeatureLibSendUpdatePercent(
  IN  IPMI_FLASH_BIOS_STATUS  FlashStatus,
  IN  UINT8                   Percent
  )
{
  return EFI_UNSUPPORTED;
}

/**
 Send Ipmi command to bmc get need BIOS flash or not.

 @retval TRUE                           Need to do BIOS update.
 @retval FALSE                          Do nothing.
*/
BOOLEAN
IpmiBmcFeatureLibCheckBmcBiosUpgradeRequest (
  CHAR16                                  *FileName
  )
{
  return FALSE;
}
//[-end-180412-IB09330446-add]//

