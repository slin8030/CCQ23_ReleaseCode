/** @file
 IPMI oem feature library header file.
 
 This file contains functions prototype of IPMI oem functions.

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

#ifndef _IPMI_OEM_FEATURE_PEI_LIB_H_
#define _IPMI_OEM_FEATURE_PEI_LIB_H_

#include <Library/PcdLib.h>
#include <IndustryStandard/SmBios.h>

#pragma pack(1)
typedef struct {
  UINT8       NicDescription;
  UINT8       IpmiChannelNumber;
  UINT16      NcsiPackageChannelID;
  UINT8       LinkStatus;
  UINT8       HostMAC[6];
  UINT8       BMCMAC[6];
} IPMI_BMC_FEATURE_NIC_INFO;

typedef struct {
  UINT8       FanControlProfileSupportBit;
  UINT8       FanControlProfileEnable;
  UINT8       Flags;
  UINT8       DimmMap[4];
} IPMI_BMC_FAN_CONTROL_CONFIG;

typedef struct {
  UINT8         IdTag;
  UINT8         MajorVersionNum;
  UINT8         MinorVersionNum;
  UINT8         BuildNumber[4];
  UINT8         BuildTimeStamp[4];
  UINT8         UodateTimeStamp[4];
} IPMI_BMC_FW_VERSION_INFO;

typedef struct {
  UINT8         IdTag;
  UINT8         SecurityVersion[2];
} IPMI_BMC_SECURITY_VERSION_INFO;

typedef struct {
  UINT8          SgpioCpldMajorRevision;
  UINT8          SgpioCpldMinorRevision;
  UINT8          MainCpldMajorRevision; 
  UINT8          MainCpldMinorRevision;
} IPMI_BMC_CPLD_REVISION;

typedef struct {
  UINT8          BmcMajorVersion;
  UINT8          BmcMinorVersion;
  UINT8          PrimaryHscMajorVersion;
  UINT8          PrimaryHscMinorVersion;
  UINT8          SecondaryHscMajorVersion;
  UINT8          SecondaryHscMinorVersion;
  UINT8          MeMajorVersion;
  UINT8          MeMinorVersion;
  UINT8          ThirdHscMajorVersion;
  UINT8          ThirdHscMinorVersion;
} IPMI_BMC_DEVICE_INFO;

typedef struct {
  UINT8          MajorVersion;
  UINT8          MinorVersion;
} IPMI_BMC_SDR_INFO;

typedef enum {
  BMC_ACTIVE_IMAGE                = 1,
  BBU_ACTIVE_IMAGE,
  BMC_BACKUP_IMAGE, 
  BBU_BACKUP_IMAGE, 
  BBR_IMAGE 
} IPMI_BMC_ID_TAG;

typedef enum {
  SECURITY_BMC_ACTIVE_IMAGE       = 1,
  SECURITY_BBU_STAGED_IMAGE, 
  SECURITY_BBR_IMAGE 
} IPMI_SECURITY_ID_TAG;

typedef enum {
  UNDEFINED_CONTEXT               = 0,
  LINUX_OS_FULL_TIME              = 0x10, 
  LINUX_OS_FORCED_FW_UPDATE_MODE  = 0x11,
  RESERVED_CONTEXT_1              = 0x20,
  RESERVED_CONTEXT_2              = 0x21
} IPMI_BMC_CURRENT_EXECUTION_CONTEXT;

typedef enum {
  UNDEFINED_POINTER               = 0,
  PRIMARY_IMAGE_POINTER           = 0x01,
  SECONDARY_IMAGE_POINTER         = 0x02,
  FACTORY_CONFIGURATION           = 0xFF
} IPMI_BMC_PARTITION_POINTER;

typedef enum {
  BMC_GET_RESTORE_STATUS          = 0x00,
  BMC_INITIATE_DEFAULT_RESTORE    = 0xAA,
  BMC_INITIATE_FULL_RESTORE       = 0xBB,
  BMC_INITIATE_REFORMATE_RESTORE  = 0xCC
} IPMI_BMC_RESTORE_OPERATION;

typedef enum {
  BMC_OEM_DEVICE_INFO_BIOS        = 0x00,
  BMC_OEM_DEVICE_INFO_BMC         = 0x01,
  BMC_OEM_DEVICE_INFO_SDR         = 0x02
} IPMI_BMC_OEM_DEVICE;

typedef struct {
  UINT8   SelVer;
  UINT16  SelEntries;
  UINT32  FreeSpace;
  UINT32  RecentAdditionTimeStamp;
  UINT32  RecentEraseTimeStamp;
  UINT8   OperationSupport;
} H2O_IPMI_OEM_SEL_INFO;

//
// Standard System Event Log Structure
//
typedef struct {
  UINT16  RecordId;
  UINT8   RecordType;
  UINT32  TimeStamp;
  UINT16  GeneratorId;
  UINT8   EvMRev;
  UINT8   SensorType;
  UINT8   SensorNum;
  UINT8   EventType    :7;
  UINT8   EventDir     :1;
  UINT8   EventData1;
  UINT8   EventData2;
  UINT8   EventData3;
//[-start-180619-IB09330471-modify]//
  UINT8   SeverityCode :2;
//[-end-180619-IB09330471-modify]//
  UINT8   Reserved     :2;
  UINT8   ValidBytes   :3;
  UINT8   Reserved1    :1;
  UINT8   ExtensionBytes[7];
} H2O_IPMI_OEM_SEL_DATA;
//
//  Structure to "OEM Event" (a.k.a. OEM Event Message)
//
typedef struct {
  UINT8 Reserved    :1;
  UINT8 SoftwareID  :7;
  UINT8 EvMRev;
  UINT8 SensorType;
  UINT8 SensorNumber;
  UINT8 EventDirType;
  UINT8 EventData1;
  UINT8 EventData2;
  UINT8 EventData3;
  UINT8 EventSeverity;
  UINT8 ExtensionBytes[7];
} H2O_IPMI_OEM_EVENT_MESSAGE;

//
// Structure to store System Event Log entry
//
typedef struct {
  UINT16                 NextRecordId;
  H2O_IPMI_OEM_SEL_DATA  Data;
} H2O_IPMI_OEM_SEL_ENTRY;

typedef struct {
  UINT16  UnitNum;
  UINT16  UnitSize;
  UINT16  FreeUnitNum;
  UINT16  LargestFreeBlock;
  UINT8   MaxRecordSize;
} H2O_IPMI_OEM_SEL_ALLOC_INFO;

typedef enum {
  IpmiFlashInitializing,
  IpmiFlashValidatingImage,
  IpmiFlashProgrammingImage,
  IpmiFlashSuccessfullyUpdated,
  IpmiFlashFailedInitialize = 0x80,
  IpmiFlashFailedValidateImage,
  IpmiFlashFailedProgramImage
} IPMI_FLASH_BIOS_STATUS;

#pragma pack()

/**
 Set Dimm error to BMC to turn on Dimm error LED.

 @param[in]        Socket               Dimm Error CPU number;
 @param[in]        Ch                   Dimm Error Channel number.
 @param[in]        Dimm                 Dimm Error Dimm number.

 @retval EFI_SUCCESS                    Set Fault Indication to BMC success.
 @retval Others                         Set Fault Indication to BMC fail.
*/
EFI_STATUS
IpmiBmcFeatureLibSendDimmError (
  UINT8    socket,
  UINT8    ch,
  UINT8    dimm
  );

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
  );

/**
 Set Dimm presence bit map to BMC.
*/
VOID
IpmiBmcFeatureLibSetDimmBitMap (
  VOID
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

/**
 Get OEM SEL Allocation Info. 

 @param[out]        SelAllocInfo            A pointer to H2O_IPMI_OEM_SEL_ALLOC_INFO structure.

 @return Execute Lib IPMI Command Status.
*/
EFI_STATUS
IpmiBmcFeatureLibGetSelAllocInfo (
  OUT H2O_IPMI_OEM_SEL_ALLOC_INFO       *SelAllocInfo,
  IN  BOOLEAN                           IsHeci
  );

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
  );

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
  );

/**
 Set OEM Platform Event Message.

 @param[in]         EventMessage        Platform Event Message.

 @return Execute Lib IPMI Command Status.
*/
EFI_STATUS
IpmiBmcFeatureLibSetPlatformEventMessage (
  IN  H2O_IPMI_OEM_EVENT_MESSAGE        *EventMessage,
  IN  BOOLEAN                           IsHeci
  );

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
  );

/**
 Get Platform Configuration. input key sting and get back value string

 @param[out]        String          Voltage Name String.

 @return Execute Lib IPMI Command Status.
*/
EFI_STATUS
IpmiBmcFeatureLibGetPlatformConfiguration (
  OUT CHAR8                          *String
  );

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
  );

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
  );

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
  );

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
  );

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
  );

/**
  Send BIOS id to BMC(seperateOEM command).

  @param[in]    Type0    Poniter to SMBIOS type 0 structure.

  @return Execute Lib IPMI Command Status.
*/

EFI_STATUS
IpmiBmcFeatureLibSendBiosIdtoBmc (
  SMBIOS_STRUCTURE        *Type0
  );

/**
 Send Post end command to BMC when ready to boot.

 @retval EFI_SUCCESS                    Send IPMI command success.
 @retval Others                         Send IPMI command fail.
*/
EFI_STATUS
IpmiBmcFeatureLibSendBmcPostEnd (
  VOID
  );

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
  );

/**
 Send Ipmi exit flash command BMC.

 @retval TRUE                           Need to do BIOS update.
 @retval FALSE                          Do nothing.
*/
EFI_STATUS
IpmiBmcFeatureLibSendExitFlashBios (
  VOID
  );

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
  );

/**
 Send Ipmi command to bmc get need BIOS flash or not.

 @retval TRUE                           Need to do BIOS update.
 @retval FALSE                          Do nothing.
*/
BOOLEAN
IpmiBmcFeatureLibCheckBmcBiosUpgradeRequest (
  CHAR16                                  *FileName
  );
  
//[-start-180426-IB09330453-add]//
/**
 Send CertificateData to BMC.
 
 @retval EFI_SUCCESS                    Send IPMI command success.
 @retval Others                         Send IPMI command fail.
*/
EFI_STATUS
IpmiBmcFeatureLibSendCertificate (
  VOID
  );
//[-start-180426-IB09330453-add]//
  
//[-start-180426-IB09330451-add]//
EFI_STATUS
IpmiBmcFeatureLibSetVersionStringToBmc (
  IN  UINTN                         StringLength,
  IN  CHAR8                         *Language,
  IN  CHAR16                        *String,
  IN OUT UINT8                      *StringIndex,
  IN BOOLEAN                        OverrideOldData
  );

VOID
IpmiBmcFeatureLibFeatureGetCurrentLanguage (
  OUT  CHAR8                                 **LanguageString
  );

VOID
EFIAPI
IpmiBmcFeatureLibGetNextLanguage (
  IN OUT CHAR8      **LangCode,
  OUT CHAR8         *Lang
  );
//[-end-180426-IB09330451-add]//

//[-start-180615-IB09330466-add]//
/**
 Get VM USB Status Type and Port Num

 @param[out]        SelInfo            A pointer to H2O_IPMI_OEM_SEL_INFO structure.

 @return Execute Lib IPMI Command Status.
*/
EFI_STATUS
IpmiBmcFeatureLibGetVmUsbStatus (
  OUT VOID                              *UsbList,
  IN  BOOLEAN                           IsHeci
  );
//[-end-180615-IB09330466-add]//
//[-start-180619-IB09330470-add]//
/**
  Send Send Embedded Firmware Update Status  to BMC.

  @param[in]    TargetDevice     Target of update.
  @param[in]    UpdateStatus     FW Update status.
  @param[in]    MajorRevision    FW Major rev.
  @param[in]    MinorRevision    FW Minor rev .
  @param[in]    ReleaseWeek      FW Release week.
  @param[in]    ReleaseNumber    FW Release Number.

  @return Execute Lib IPMI Command Status.
*/
EFI_STATUS
IpmiBmcFeatureLibSendFwUpdateStatustoBmc (
  UINT8         TargetDevice,
  UINT8         UpdateStatus,
  UINT8         MajorRevision,
  UINT8         MinorRevision,
  UINT8         ReleaseWeek,
  UINT16        ReleaseNumber
  );
//[-end-180619-IB09330470-add]//

#endif

