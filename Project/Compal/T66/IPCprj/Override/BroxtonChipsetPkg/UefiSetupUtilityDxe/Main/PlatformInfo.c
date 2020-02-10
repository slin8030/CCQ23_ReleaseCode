/** @file

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
//[PRJ] ++ >>>> Add EC version on SCU Main page
#include <Main/PlatformInfo.h>
//[PRJ] ++ <<<< Add EC version on SCU Main page
#include <ScPolicyCommon.h>
#include <Protocol/LegacyBios.h>
#include <Guid/PlatformInfo_Apl.h>
//[-start-160704-IB07220103-add]//
#include <Guid/MemoryConfigData.h>
//[-end-160704-IB07220103-add]//
#include <Library/PcdLib.h>
#include <Library/DxeOemSvcKernelLib.h>
#include <Library/I2clib.h>
//[-start-160704-IB07220103-add]//
#include <Library/PmcIpcLib.h>
#include <Library/HeciMsgLib.h>
//[-end-160704-IB07220103-add]//
#include <Protocol/SeCOperation.h>
//[-start-160704-IB07220103-add]//
#include <Protocol/CpuInfo.h>
#include <MmrcData.h>
//[-end-160704-IB07220103-add]//
#include <CpuRegs.h>
#include <ScAccess.h>
#if defined (ENBDT_PF_ENABLE) && (ENBDT_PF_ENABLE == 1)
#include <KscLib.h>
#endif
#include <PlatformBaseAddresses.h>

//[PRJ] ++ >>>> Add EC version on SCU Main page
#include <CompalEcLib.h> 
//[PRJ] ++ <<<< Add EC version on SCU Main page
#define ICH_REG_REV                 0x08
//#define MSR_IA32_PLATFORM_ID        0x17

typedef struct {
  UINT8 RevId;
  CHAR8 String[24];
} SB_REV;

//[-start-170116-IB07400834-modify]//
//[-start-160818-IB03090432-remove]//
//[-start-180803-IB07400993-modify]//
SB_REV  mSocSteppingTable[] = {
  {BxtPA0, "A0 Stepping"},
  {BxtPB0, "B0 Stepping"},
  {BxtPB1, "B1 Stepping"},
  {BxtPE0, "E0 Stepping"},
//[-start-190102-IB07401061-modify]//
#ifdef IOTG_SIC_RC_CODE_SUPPORTED
  {BxtPF1, "F1 Stepping"}
#else
  {BxtPF0, "F0 Stepping"}
#endif
//[-end-190102-IB07401061-modify]//
};
//[-end-180803-IB07400993-modify]//
//[-end-160818-IB03090432-remove]//
//[-end-170116-IB07400834-modify]//

STATIC EFI_HII_STRING_PROTOCOL   *mIfrLibHiiString;
//[-start-160901-IB07400777-add]//
CHIPSET_CONFIGURATION            *mSystemConfig;
//[-end-160901-IB07400777-add]//

EFI_STATUS
JudgeHandleIsPCIDevice(
  EFI_HANDLE    Handle,
  UINT8            Device,
  UINT8            Funs
)
{
  EFI_STATUS        Status;
  EFI_DEVICE_PATH   *DPath;
  EFI_DEVICE_PATH   *DevicePath;                              
  PCI_DEVICE_PATH   *PCIPath;     
  
  Status = gBS->HandleProtocol (
                        Handle,
                        &gEfiDevicePathProtocolGuid,
                        (VOID **) &DPath
                        );
  if (!EFI_ERROR(Status)) {
    DevicePath = DPath;     
    while (!IsDevicePathEnd(DPath)) { 
      if ((DPath->Type == HARDWARE_DEVICE_PATH) && (DPath->SubType == HW_PCI_DP)) {
               
        PCIPath = (PCI_DEVICE_PATH*) DPath;                            
        DPath = NextDevicePathNode(DPath);                             
        if(IsDevicePathEnd(DPath) && (PCIPath->Device == Device) && (PCIPath->Function == Funs)) {
          return EFI_SUCCESS;
        }
      } else {
        DPath = NextDevicePathNode(DPath);             
      }
    }                  
  }   
  return EFI_UNSUPPORTED;
}

EFI_STATUS
GetDriverName(
  EFI_HANDLE   Handle,
  CHAR16         *Name
)
{
  EFI_STATUS                     Status;
  EFI_DRIVER_BINDING_PROTOCOL    *BindHandle;  
  UINT32                         Version;                                                 
  UINT16                         *Ptr;
  UINT8                          *PtrUINT8;  
  
  BindHandle = NULL;
  Status = gBS->OpenProtocol(      
                     Handle,
                     &gEfiDriverBindingProtocolGuid,
                     (VOID**)&BindHandle,
                     NULL,
                     NULL,
                     EFI_OPEN_PROTOCOL_GET_PROTOCOL
                     );  
  
  if (EFI_ERROR(Status)) {
    return EFI_NOT_FOUND;
  }
    
  Version = BindHandle->Version;
  Ptr = (UINT16*)&Version;
  PtrUINT8 = (UINT8*) (UINTN)Ptr + 2;

  UnicodeSPrint(Name, 40, L"%d.%d.%04d", *(PtrUINT8 + 1), *(PtrUINT8), *(Ptr));

  return EFI_SUCCESS;
}


EFI_STATUS
SearchChildHandle(
  EFI_HANDLE Father,
  EFI_HANDLE *Child
  )
{
  EFI_STATUS                                          Status;
  UINTN                                               HandleIndex;
  EFI_GUID                                            **ProtocolGuidArray;
  UINTN                                               ArrayCount;
  UINTN                                               ProtocolIndex;
  UINTN                                               OpenInfoCount;
  UINTN                                               OpenInfoIndex;
  EFI_OPEN_PROTOCOL_INFORMATION_ENTRY                 *OpenInfo;      
  UINTN                                               HandleCount;
  EFI_HANDLE                                          *HandleBuffer;


  ProtocolGuidArray = NULL;
  OpenInfo = NULL;
  HandleBuffer = NULL;
  
  //
  // Retrieve the list of all handles from the handle database
  //
  Status = gBS->LocateHandleBuffer (
                      AllHandles,
                      NULL,
                      NULL,
                      &HandleCount,
                      &HandleBuffer
                      );

  for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
    //
    // Retrieve the list of all the protocols on each handle
    //
    Status = gBS->ProtocolsPerHandle (
                         HandleBuffer[HandleIndex],
                         &ProtocolGuidArray,
                         &ArrayCount
                         );
    if (!EFI_ERROR (Status)) {
      for (ProtocolIndex = 0; ProtocolIndex < ArrayCount; ProtocolIndex++) {        
        Status = gBS->OpenProtocolInformation (
                               HandleBuffer[HandleIndex],
                               ProtocolGuidArray[ProtocolIndex],
                               &OpenInfo,
                               &OpenInfoCount
                               );
        if (!EFI_ERROR (Status)) {
          for (OpenInfoIndex = 0; OpenInfoIndex < OpenInfoCount; OpenInfoIndex++) {
            if (OpenInfo[OpenInfoIndex].AgentHandle == Father) {
              if ((OpenInfo[OpenInfoIndex].Attributes & EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER) == EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER) {
                *Child = HandleBuffer[HandleIndex];                    
                Status = EFI_SUCCESS;
                goto TryReturn;
              }        
            }
          }
          Status = EFI_NOT_FOUND;
        }   
      }
      if(OpenInfo != NULL) {
        FreePool(OpenInfo);
        OpenInfo = NULL;
      }   
    }
    FreePool (ProtocolGuidArray);
    ProtocolGuidArray = NULL;
  }
  
TryReturn:
  if (OpenInfo != NULL) {
    FreePool (OpenInfo);
    OpenInfo = NULL;
  }
  if (ProtocolGuidArray != NULL) {
    FreePool (ProtocolGuidArray);
    ProtocolGuidArray = NULL;
  }
  if (HandleBuffer != NULL) {
    FreePool (HandleBuffer);
    HandleBuffer = NULL;
  }
  return Status;
}

EFI_STATUS
GetGOPDriverName(
  CHAR16 *Name
)
{
  UINTN                         HandleCount;
  EFI_HANDLE                *Handles= NULL;
  UINTN                         Index;
  EFI_STATUS                Status;
  EFI_HANDLE                Child = 0;

  Status = gBS->LocateHandleBuffer(
              ByProtocol, 
              &gEfiDriverBindingProtocolGuid,              
              NULL,
              &HandleCount,
              &Handles
                  );  
  for (Index = 0; Index < HandleCount ; Index++) {
    Status = SearchChildHandle(Handles[Index], &Child);
    if (!EFI_ERROR(Status)) {
      Status = JudgeHandleIsPCIDevice(Child, 0x02, 0x00);
      if(!EFI_ERROR(Status)) {
        return GetDriverName(Handles[Index], Name);
      }
    }
  }
  return EFI_UNSUPPORTED;
}


/**

 Update new string to Hii Handle.

 @param [in]   SrcHiiHandle   New Hii Handle.
 @param [in]   SrcStringId    New String Token.
 @param [in]   DstHiiHandle   Hii Handle of the package to be updated.
 @param [out]   DstStringId   String Token to be updated.

 @retval EFI_SUCCESS          String update successfully.

**/
EFI_STATUS
NewStringToHandle (
  IN  EFI_HII_HANDLE           SrcHiiHandle,
  IN  EFI_STRING_ID            SrcStringId,
  IN  EFI_HII_HANDLE           DstHiiHandle,
  OUT EFI_STRING_ID            *DstStringId
  )
{
  EFI_STATUS                   Status;
  CHAR16                       *String;
  UINTN                        Size;
  UINTN                        StringSize;
  CHAR8                        *Languages;
  CHAR8                        *LangStrings;
  CHAR8                        Lang[RFC_3066_ENTRY_SIZE];


  StringSize = 0x200;
  String = AllocateZeroPool (StringSize);
  if (String == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Use english string as the default string.
  //
  Size = StringSize;
  Status = mIfrLibHiiString->GetString (
                               mIfrLibHiiString,
                               "en-US",
                               SrcHiiHandle,
                               SrcStringId,
                               String,
                               &Size,
                               NULL
                               );
  if (Status == EFI_BUFFER_TOO_SMALL) {
    gBS->FreePool (String);
    StringSize = Size;
    String = AllocateZeroPool (StringSize);
    Status = mIfrLibHiiString->GetString (
                                 mIfrLibHiiString,
                                 "eng",
                                 SrcHiiHandle,
                                 SrcStringId,
                                 String,
                                 &Size,
                                 NULL
                                 );
    if (EFI_ERROR (Status)) {
      gBS->FreePool (String);
      return Status;
    }
  } else if (EFI_ERROR (Status)) {
    return Status;
  }

  *DstStringId = HiiSetString (DstHiiHandle, 0, String, NULL);

  //
  // Update string by each language.
  //
  Languages = HiiGetSupportedLanguages (DstHiiHandle);
  LangStrings = Languages;
  while (*LangStrings != 0) {
    SetupUtilityLibGetNextLanguage (&LangStrings, Lang);

    Size = StringSize;
    Status = mIfrLibHiiString->GetString (
                                 mIfrLibHiiString,
                                 Lang,
                                 SrcHiiHandle,
                                 SrcStringId,
                                 String,
                                 &Size,
                                 NULL
                                 );
    if (!EFI_ERROR (Status)) {
      mIfrLibHiiString->SetString (
                          mIfrLibHiiString,
                          DstHiiHandle,
                          *DstStringId,
                          Lang,
                          String,
                          NULL
                          );
    }
  }

  gBS->FreePool (String);
  gBS->FreePool (Languages);

  return EFI_SUCCESS;
}


/**
  Update platform dynamic informations. 

  @param [in]   MainHiiHandle       Main page Hii handle.
  @param [in]   AdvanceHiiHandle    Advance Hii handle.
  @param [in]   StartOpCodeHandle   Start op code handle.

  @retval EFI_SUCCESS   Opeartion successfully completed.

**/
EFI_STATUS
UpdatePlatformInformation (
  IN EFI_HII_HANDLE     MainHiiHandle,
  IN EFI_HII_HANDLE     AdvanceHiiHandle,
  IN VOID               *StartOpCodeHandle
  )
{
  UINT32                   MicroCodeVersion;
  CHAR16                   Buffer[40];
  UINT8                    IgdVBIOSRevH;
  UINT8                    IgdVBIOSRevL;
  EFI_STATUS               Status;
  EFI_LEGACY_BIOS_PROTOCOL *LegacyBios;
  UINT8                    CpuFlavor;
  EFI_PEI_HOB_POINTERS     GuidHob;
  EFI_PLATFORM_INFO_HOB    *PlatformInfo;
//  UINTN                    NumHandles;
//  EFI_HANDLE               *HandleBuffer;
//  UINTN                    Index;
//  DXE_PCH_PLATFORM_POLICY_PROTOCOL  
//                           *PchPlatformPolicy;
//  UINTN                    PciD31F0RegBase;
//  UINT8                    count;
//[-start-160704-IB07220103-remove]//
//   UINT8                    Data8;
//[-end-160704-IB07220103-remove]//
  CHAR16                   Name[40];
  UINT32                   MrcVersion;
  UINT8                    MrcMajorVer;
  UINT8                    MrcMinorVer; 
  UINT8                    MrcReleaseVer; 
  UINT8                    MrcInternalVer;
  SEC_OPERATION_PROTOCOL   *SeCOp;
  SEC_INFOMATION           SeCInfo;
//[-start-160704-IB07220103-remove]//
//   UINT8                    PIDData8;
//[-end-160704-IB07220103-remove]//
  EFI_STRING_ID            NewStringId;
  EFI_STRING_ID            TitleStringId;
  EFI_STRING_ID            HelpStringId;
  UINT16                   EDX;
  EFI_IA32_REGISTER_SET    RegSet;
//[-start-160704-IB07220103-add]//
//[-start-160711-IB07400754-remove]//
//  UINT32                   SocVersion;
//[-end-160711-IB07400754-remove]//
  MRC_PARAMS_SAVE_RESTORE  *MemInfoHob;
  UINT8                    PMCVersion[8];
//[-start-160711-IB07400754-modify]//
#if 0
  EFI_CPUID_REGISTER       CpuidRegs;
#endif
//[-end-160711-IB07400754-modify]//
  FW_VERSION_CMD_RESP_DATA MsgGetFwVersionRespData;
  CHAR8                    *IshManName;
  UINT8                    Index;
//[-end-160704-IB07220103-add]//
//[-start-160711-IB07400754-add]//
  UINT8                    Data8;
  UINT8                    count;
//[-end-160711-IB07400754-add]//
//[-start-160726-IB07400762-add]//
  UINT32                   SrcVersion;
//[-end-160726-IB07400762-add]//
//[-start-180626-IB07400990-add]//
  UINT32                   PlatformId;
//[-end-180626-IB07400990-add]//
//[-start-181001-IB07401020-add]//
  UINTN                    BiosRomSize;
//[-end-181001-IB07401020-add]//
//[-start-160418-IB07400718-modify]//
#if defined (ENBDT_PF_ENABLE) && (ENBDT_PF_ENABLE == 1)
#ifdef APOLLOLAKE_CRB
  UINT8          KscFwRevH;
  UINT8          KscFwRevL;

  KscFwRevH = 0;
  KscFwRevL = 0;
#endif
#endif
//[-end-160418-IB07400718-modify]//

  CpuFlavor=0;
  LegacyBios = NULL;
  PlatformInfo=NULL;
//[-start-160704-IB07220103-add]//
  MemInfoHob = NULL;
//[-end-160704-IB07220103-add]//
  //
  // Get the HOB list.  If it is not present, then ASSERT.
  //
  GuidHob.Raw = GetHobList ();
  if (GuidHob.Raw != NULL) {
    if ((GuidHob.Raw = GetNextGuidHob (&gEfiPlatformInfoGuid, GuidHob.Raw)) != NULL) {
      PlatformInfo = GET_GUID_HOB_DATA (GuidHob.Guid);
    }
  }

  //
  //Update Soc Version
  //

//[-start-160418-IB07400718-modify]//
  NewStringId = (EFI_STRING_ID)0; 
  Data8 = BxtStepping();
  count = sizeof (mSocSteppingTable) / sizeof (mSocSteppingTable[0]);
//[-start-180626-IB07400990-modify]//
  PlatformId = (UINT32) (RShiftU64 (AsmReadMsr64 (MSR_IA32_PLATFORM_ID), 50) & 0x7);
  for (Index = 0; Index < count; Index++) {
    if(Data8 == mSocSteppingTable[Index].RevId) {
      if ((Data8 == BxtPB1) && (PlatformId == 0x01)) {
        UnicodeSPrint (Buffer, sizeof (Buffer), L"D0 Stepping");
      } else {
        UnicodeSPrint (Buffer, sizeof (Buffer), L"%a", mSocSteppingTable[Index].String);
      }
      NewStringId = HiiSetString (MainHiiHandle, (EFI_STRING_ID)0, Buffer, NULL);
      break;
    }
  }
//[-end-180626-IB07400990-modify]//
  if (NewStringId != (EFI_STRING_ID)0) {
    NewStringToHandle (
      AdvanceHiiHandle,
      STRING_TOKEN (STR_SOC_STRING),
      MainHiiHandle,
      &TitleStringId
      );
    HiiCreateTextOpCode (
      StartOpCodeHandle,
      TitleStringId,
      0,
      NewStringId
      );
  }
//[-end-160418-IB07400718-modify]//
//[-start-160726-IB07400762-add]//
//[-start-180115-IB07400945-modify]//
#ifdef IOTG_SIC_RC_CODE_SUPPORTED
  SrcVersion = PcdGet32 (PcdIotgSiliconRefCodeVersion);
  UnicodeSPrint (Buffer, sizeof (Buffer), L"%d.%d.%d (IOTG)", (SrcVersion >> 24) & 0xFF, (SrcVersion >> 16) & 0xFF, (SrcVersion >> 8) & 0xFF);
#else
  SrcVersion = PcdGet32 (PcdSiliconRefCodeVersion);
  UnicodeSPrint (Buffer, sizeof (Buffer), L"%d.%d.%d (CCG)", (SrcVersion >> 24) & 0xFF, (SrcVersion >> 16) & 0xFF, (SrcVersion >> 8) & 0xFF);
#endif
//[-end-180115-IB07400945-modify]//
  NewStringId = HiiSetString (MainHiiHandle, (EFI_STRING_ID)0, Buffer, NULL);
  NewStringToHandle (
    AdvanceHiiHandle,
//[-start-160809-IB07400769-modify]//
    STRING_TOKEN (STR_SIC_VERSION_STRING),
//[-end-160809-IB07400769-modify]//
    MainHiiHandle,
    &TitleStringId
    );
  HiiCreateTextOpCode (
    StartOpCodeHandle,
    TitleStringId,
    0,
    NewStringId
    );
//[-end-160726-IB07400762-add]//
  //
  // Retrieve all instances of PCH Platform Policy protocol
  //
//  NewStringId = (EFI_STRING_ID)0; 
//  Status = gBS->LocateHandleBuffer (
//                  ByProtocol,
//                  &gDxePchPlatformPolicyProtocolGuid,
//                  NULL,
//                  &NumHandles,
//                  &HandleBuffer
//                  );
//  if (!EFI_ERROR (Status)) {
//    //
//    // Find the matching PCH Policy protocol
//    //
//    for (Index = 0; Index < NumHandles; Index++) {
//      Status = gBS->HandleProtocol (
//                      HandleBuffer[Index],
//                      &gDxePchPlatformPolicyProtocolGuid,
//                      (VOID **)&PchPlatformPolicy
//                      );
//      if (!EFI_ERROR (Status)) {
//        PciD31F0RegBase = MmPciAddress (
//                            0,
//                            PchPlatformPolicy->BusNumber,
//                            PCI_DEVICE_NUMBER_LPC,
//                            PCI_FUNCTION_NUMBER_LPC,
//                            0
//                            );
//
//         Data8 = MmioRead8 (PciD31F0RegBase + R_LPC_RID_CC)  & B_LPC_RID_STEPPING_MASK;
//         count = sizeof (mSBRevisionTable) / sizeof (mSBRevisionTable[0]);
//         for (Index = 0; Index < count; Index++) {
//           if(Data8 == mSBRevisionTable[Index].RevId) {
//              UnicodeSPrint (Buffer, sizeof (Buffer), L"%02x %a", Data8, mSBRevisionTable[Index].String);
//            NewStringId = HiiSetString (MainHiiHandle, (EFI_STRING_ID)0, Buffer, NULL);
//             break;
//           }
//         }
//        break;
//      }
//    }
//  }
//  if (NewStringId != (EFI_STRING_ID)0) {
//    NewStringToHandle (
//      AdvanceHiiHandle,
//      STRING_TOKEN (STR_SOC_STRING),
//      MainHiiHandle,
//      &TitleStringId
//      );
//    HiiCreateTextOpCode (
//      StartOpCodeHandle,
//      TitleStringId,
//      0,
//      NewStringId
//      );
//  }

  //
  //Update MRC Version
  //
//[-start-160704-IB07220103-add]//
  GuidHob.Raw = GetHobList ();
  if (GuidHob.Raw != NULL) {
    if ((GuidHob.Raw = GetNextGuidHob (&gEfiMemoryConfigDataGuid, GuidHob.Raw)) != NULL) {
      MemInfoHob = (MRC_PARAMS_SAVE_RESTORE *) GET_GUID_HOB_DATA (GuidHob.Guid);
    }
  }
//[-start-160711-IB07400754-modify]//
  if (MemInfoHob != NULL) {
    MrcVersion = MemInfoHob->MrcVer;
//[-end-160704-IB07220103-add]//
    MrcVersion &= 0xFFFFFFFF;
    MrcMajorVer    = (MrcVersion >> 24) & 0xFF;
    MrcMinorVer    = (MrcVersion >> 16) & 0xFF;
    MrcReleaseVer  = (MrcVersion >> 8) & 0xFF;
    MrcInternalVer=  MrcVersion & 0xFF;
//[-start-170622-IB07400878-modify]//
//   UnicodeSPrint (Buffer, sizeof (Buffer), L"%02d.%02d.%02d.%02d", MrcMajorVer,MrcMinorVer,MrcReleaseVer,MrcInternalVer);
    UnicodeSPrint (Buffer, sizeof (Buffer), L"%02d.%02d (%02d.%02d)", MrcMajorVer,MrcMinorVer, MrcReleaseVer, MrcInternalVer);
//[-end-170622-IB07400878-modify]//
    NewStringId = HiiSetString (MainHiiHandle, (EFI_STRING_ID)0, Buffer, NULL);
    NewStringToHandle (
      AdvanceHiiHandle,
      STRING_TOKEN (STR_MRC_VERSION_STRING),
      MainHiiHandle,
      &TitleStringId
      );
    HiiCreateTextOpCode (
      StartOpCodeHandle,
      TitleStringId,
      0,
      NewStringId
      );
  }
//[-end-160711-IB07400754-modify]//

  //
  // Punit Version (Microcode)
  //
//[-start-160704-IB07220103-modify]//
  AsmWriteMsr64 (MSR_IA32_BIOS_SIGN_ID, 0);
  AsmCpuid (CPUID_VERSION_INFO, NULL, NULL, NULL, NULL);
  MicroCodeVersion = (UINT32) RShiftU64 (AsmReadMsr64 (MSR_IA32_BIOS_SIGN_ID), 32);
  if (MicroCodeVersion == 0) {
    NewStringId = HiiSetString(MainHiiHandle, (EFI_STRING_ID)0, L"Not Loaded", NULL); 
  } else {
    UnicodeSPrint (Buffer, sizeof (Buffer), L"%x", MicroCodeVersion);
    NewStringId = HiiSetString(MainHiiHandle, (EFI_STRING_ID)0, Buffer, NULL);  
  }
//[-end-160704-IB07220103-modify]//
  NewStringToHandle (
    AdvanceHiiHandle,
    STRING_TOKEN (STR_PUNIT_FW_STRING),
    MainHiiHandle,
    &TitleStringId
    );
  HiiCreateTextOpCode (
    StartOpCodeHandle,
    TitleStringId,
    0,
    NewStringId
    );

  //
  //  PMC Version
  //
//[-start-160711-IB07400754-modify]//
//[-start-160704-IB07220103-modify]//
//   Data8 = (UINT8)((MmioRead32 (PMC_BASE_ADDRESS + R_PMC_PRSTS) >> 16) & 0x00FF);
//   PIDData8 = (UINT8)((MmioRead32 (PMC_BASE_ADDRESS + R_PMC_PRSTS) >> 24) & 0x00FF);
  Status = IpcSendCommandEx (IPC_CMD_ID_PMC_VER, IPC_SUBCMD_ID_PMC_VER_READ, PMCVersion, 8);
  if (!EFI_ERROR (Status)) {
    UnicodeSPrint (Buffer, sizeof (Buffer), L"%02x.%02x", PMCVersion[1], PMCVersion[0]);
//[-end-160704-IB07220103-modify]//
    NewStringId = HiiSetString (MainHiiHandle, (EFI_STRING_ID)0, Buffer, NULL);
    NewStringToHandle (
      AdvanceHiiHandle,
      STRING_TOKEN (STR_PMC_FW_STRING),
      MainHiiHandle,
      &TitleStringId
      );
    HiiCreateTextOpCode (
      StartOpCodeHandle,
      TitleStringId,
      0,
      NewStringId
      );
  }
//[-end-160711-IB07400754-modify]//
//[-start-160418-IB07400718-modify]//
#if defined (ENBDT_PF_ENABLE) && (ENBDT_PF_ENABLE == 1)  
//[-start-161205-IB07400822-modify]//
//[-start-170111-IB07400832-modify]//
//#if FeaturePcdGet(PcdKscSupport)
#if defined (KSC_SUPPORT)
//[-end-170111-IB07400832-modify]//
#ifdef APOLLOLAKE_CRB
  //
  // CRB EC code to get EC FW version
  //
  //
  // KSC Firmware Version
  //
  if (IoRead8(0x62) != 0xFF) { // EC exist
    InitializeKscLib();
    Status = SendKscCommand(0x90);    // SMC_READ_REVISION command
    if(!EFI_ERROR(Status)) {
      Status = ReceiveKscData(&KscFwRevH);
      Status = ReceiveKscData(&KscFwRevL);
      if(!EFI_ERROR(Status)){
        UnicodeSPrint (Buffer, sizeof (Buffer), L"%02X.%02X", KscFwRevH,KscFwRevL);
          NewStringId = HiiSetString (MainHiiHandle, (EFI_STRING_ID)0, Buffer, NULL);
          NewStringToHandle (
            AdvanceHiiHandle,
            STRING_TOKEN (STR_KSC_FW_STRING),
            MainHiiHandle,
            &TitleStringId
            );
          HiiCreateTextOpCode (
            StartOpCodeHandle,
            TitleStringId,
            0,
            NewStringId
            );
        }
    }
  }
#else
  //
  // OEM EC code to get EC FW version
  //
#endif  
#endif
//[-end-161205-IB07400822-modify]//
//[-end-160418-IB07400718-modify]//
#endif
  //
  // SEC FW Version
  //
  ZeroMem (&SeCInfo, sizeof(SeCInfo));
  Status = gBS->LocateProtocol (
                &gEfiSeCOperationProtocolGuid,
                NULL,
                (VOID **)&SeCOp
                );
  if (!EFI_ERROR(Status)) {
    Status = SeCOp->GetPlatformSeCInfo (&SeCInfo);
  }
  if (SeCInfo.SeCVerValid) {
    UnicodeSPrint (
      Buffer, 
      sizeof (Buffer),
      L"%d.%d.%d.%d",
      SeCInfo.SeCVer.CodeMajor,
      SeCInfo.SeCVer.CodeMinor,
      SeCInfo.SeCVer.CodeHotFix,
      SeCInfo.SeCVer.CodeBuildNo
      );
  } else {
    UnicodeSPrint (Buffer, sizeof (Buffer), L"0.0.0.0");
  }
  NewStringId = HiiSetString (MainHiiHandle, (EFI_STRING_ID)0, Buffer, NULL);
  NewStringToHandle (
    AdvanceHiiHandle,
//[-start-160419-IB07400718-modify]//
    STRING_TOKEN (STR_TXE_FW_STRING),
//[-end-160419-IB07400718-modify]//
    MainHiiHandle,
    &TitleStringId
    );
  HiiCreateTextOpCode (
    StartOpCodeHandle,
    TitleStringId,
    0,
    NewStringId
    );

//[-start-160704-IB07220103-add]//
  //
  // ISH FW Revision
  //
  DEBUG ((EFI_D_INFO, "Executing GetImageFwVersion().\n"));
  SetMem(&MsgGetFwVersionRespData, sizeof(FW_VERSION_CMD_RESP_DATA), 0x0);
  SetMem(Buffer, sizeof(Buffer), 0x0);
  IshManName = "ISHC.man";  //ISH manifest file name
  Status = HeciGetImageFwVerMsg(&MsgGetFwVersionRespData);
  if (!EFI_ERROR(Status)) {
    for (Index = 0; Index < MAX_IMAGE_MODULE_NUMBER; Index++) {
      if (!CompareMem(MsgGetFwVersionRespData.ModuleEntries[Index].EntryName, IshManName, AsciiStrLen(IshManName))) {
        UnicodeSPrint (Buffer, sizeof (Buffer), L"%d.%d.%d.%d",
          MsgGetFwVersionRespData.ModuleEntries[Index].Major,
          MsgGetFwVersionRespData.ModuleEntries[Index].Minor,
          MsgGetFwVersionRespData.ModuleEntries[Index].Hotfix,
          MsgGetFwVersionRespData.ModuleEntries[Index].Build
          );
        NewStringId = HiiSetString(MainHiiHandle, (EFI_STRING_ID)0, Buffer, NULL);
        NewStringToHandle (
          AdvanceHiiHandle,
          STRING_TOKEN (STR_ISH_FW_STRING),
          MainHiiHandle,
          &TitleStringId
          );
        HiiCreateTextOpCode (
          StartOpCodeHandle,
          TitleStringId,
          0,
          NewStringId
          );
      }
    }
  }
//[-end-160704-IB07220103-add]//
  

  //
  // GOP version
  //
//[-start-160901-IB07400777-modify]//
  //
  // Only Display GOP version when boot type = EFI mode.
  //
  if (mSystemConfig->BootType == EFI_BOOT_TYPE) {
    Status = GetGOPDriverName(Name);
    if(!EFI_ERROR(Status)) {       
      NewStringId = HiiSetString(MainHiiHandle, (EFI_STRING_ID)0, Name, NULL);  
      NewStringToHandle (
        AdvanceHiiHandle,
        STRING_TOKEN (STR_GOP_STRING),
        MainHiiHandle,
        &TitleStringId
        );
      HiiCreateTextOpCode (
        StartOpCodeHandle,
        TitleStringId,
        0,
        NewStringId
        );
    }    
  }
//[-end-160901-IB07400777-modify]//

  //
  // VBIOS version
  //
  IgdVBIOSRevH = 0;
  IgdVBIOSRevL = 0;
  Status = gBS->LocateProtocol(&gEfiLegacyBiosProtocolGuid, NULL, (VOID **)&LegacyBios);
  if (!EFI_ERROR (Status)) {
    RegSet.X.AX = 0x5f01;
    Status = LegacyBios->Int86 (LegacyBios, 0x10, &RegSet);
    ASSERT_EFI_ERROR(Status);

    EDX = (UINT16)((RegSet.E.EBX >> 16) & 0xffff);
    IgdVBIOSRevH = (UINT8)(((EDX & 0x0F00) >> 4) | (EDX & 0x000F));
    IgdVBIOSRevL = (UINT8)(((RegSet.X.BX & 0x0F00) >> 4) | (RegSet.X.BX & 0x000F));
  }
  if (IgdVBIOSRevH != 0 || IgdVBIOSRevL != 0) {
    UnicodeSPrint (Buffer, sizeof (Buffer), L"%02X%02X", IgdVBIOSRevH,IgdVBIOSRevL);
    NewStringId = HiiSetString(MainHiiHandle, (EFI_STRING_ID)0, Buffer, NULL);
    NewStringToHandle (
      AdvanceHiiHandle,
      STRING_TOKEN (STR_CHIP_IGD_VBIOS_REV_NAME),
      MainHiiHandle,
      &TitleStringId
      );
    NewStringToHandle (
      AdvanceHiiHandle,
      STRING_TOKEN (STR_CHIP_IGD_VBIOS_REV_HELP),
      MainHiiHandle,
      &HelpStringId
      );
    HiiCreateTextOpCode (
      StartOpCodeHandle,
      TitleStringId,
      HelpStringId,
      NewStringId
      );
  }
  
  //
  // Microcode Revision
  //
//[-start-160704-IB07220103-remove]//
//   AsmWriteMsr64 (MSR_IA32_BIOS_SIGN_ID, 0);
//   AsmCpuid (CPUID_VERSION_INFO, NULL, NULL, NULL, NULL);
//   MicroCodeVersion = (UINT32) RShiftU64 (AsmReadMsr64 (MSR_IA32_BIOS_SIGN_ID), 32);
//   if (MicroCodeVersion == 0) {
//     NewStringId = HiiSetString(MainHiiHandle, (EFI_STRING_ID)0, L"Not Loaded", NULL); 
//   } else {
//     UnicodeSPrint (Buffer, sizeof (Buffer), L"%x", MicroCodeVersion);
//     NewStringId = HiiSetString(MainHiiHandle, (EFI_STRING_ID)0, Buffer, NULL);  
//   }
//   NewStringToHandle (
//     AdvanceHiiHandle,
//     STRING_TOKEN (STR_PROCESSOR_MICROCODE_REV_NAME),
//     MainHiiHandle,
//     &TitleStringId
//     );
//   HiiCreateTextOpCode (
//     StartOpCodeHandle,
//     TitleStringId,
//     0,
//     NewStringId
//     );
//[-end-160704-IB07220103-remove]//

//[-start-160704-IB07220103-modify]//
//[-start-160711-IB07400754-add]//
#if 0
  //
  //CpuFlavor
  //
  //BXT
  //BXT Tablet            CPUID for EAX will be 0x506C0 , 0x506C1 , 0x506C2 and
  //                      Bit 3 of EAX will be 0
  //BXT Notebook/Desktop  CPUID for EAX will be 0x506C8 , 0x506C9 and 
  //                      Bit 3 of EAX will be 1

  //CPU flavor
  AsmCpuid (1, &(CpuidRegs.RegEax), &(CpuidRegs.RegEbx), &(CpuidRegs.RegEcx), &(CpuidRegs.RegEdx));
  CpuFlavor = (UINT8) ((CpuidRegs.RegEax) & 0x8) >> 3;
  switch(CpuFlavor){
    case 0x0:
//#if !BXTI_PF_ENABLE
//        UnicodeSPrint (Buffer, sizeof (Buffer), L"%s (%01x)", L"BXT Tablet", CpuFlavor);
//#else
        UnicodeSPrint (Buffer, sizeof (Buffer), L"%s (%01x)", L"BXT IVI", CpuFlavor);
//#endif
        break;
    case 0x01:
        UnicodeSPrint (Buffer, sizeof (Buffer), L"%s (%01x)", L"BXT Notebook/Desktop", CpuFlavor);
        break;
    case 0x02:
//         UnicodeSPrint (Buffer, sizeof (Buffer), L"%s (%01x)", L"BXT Desktop", CpuFlavor);
//         break;
    case 0x03:
//         UnicodeSPrint (Buffer, sizeof (Buffer), L"%s (%01x)", L"BXT Notebook", CpuFlavor);
//         break;
    default:
        UnicodeSPrint (Buffer, sizeof (Buffer), L"%s (%01x)", L"Unknown CPU", CpuFlavor);
        break;
  }
// #if (ENBDT_PF_ENABLE == 1)
//   UnicodeSPrint (Buffer, sizeof (Buffer), L"%s (%01x)", L"BXT-CPU", CpuFlavor);
// #endif
//[-end-160704-IB07220103-modify]//

  NewStringId = HiiSetString(MainHiiHandle, (EFI_STRING_ID)0, Buffer, NULL);  
  NewStringToHandle (
    AdvanceHiiHandle,
    STRING_TOKEN (STR_CPU_FLAVOR_NAME),
    MainHiiHandle,
    &TitleStringId
    );
  NewStringToHandle (
    AdvanceHiiHandle,
    STRING_TOKEN (STR_CPU_FLAVOR_HELP),
    MainHiiHandle,
    &HelpStringId
    );
  HiiCreateTextOpCode (
    StartOpCodeHandle,
    TitleStringId,
    HelpStringId,
    NewStringId
    );
#endif
//[-end-160711-IB07400754-add]//

  //
  //BoardId
  //
  if (PlatformInfo != NULL) {
//[-start-160704-IB07220103-modify]//
    switch(PlatformInfo->BoardId){
      case 0x0:
          UnicodeSPrint (Buffer, sizeof (Buffer), L"APL ERB (%02x)", PlatformInfo->BoardId);
          break;

      case 0x1:
          UnicodeSPrint (Buffer, sizeof (Buffer), L"APL RVP 1A (%02x)", PlatformInfo->BoardId);
          break;

      case 0x2:
          UnicodeSPrint (Buffer, sizeof (Buffer), L"APL RVP 2A (%02x)", PlatformInfo->BoardId);
          break;

      case 0x3:
          UnicodeSPrint (Buffer, sizeof (Buffer), L"APL RVP 1C Lite (%02x)", PlatformInfo->BoardId);
          break;

      case 0x5:
          UnicodeSPrint (Buffer, sizeof (Buffer), L"Mineral Hill RVP (%02x)", PlatformInfo->BoardId);
          break;
	  
      case 0x6:
          UnicodeSPrint (Buffer, sizeof (Buffer), L"Oxbow Hill CRB (%02x)", PlatformInfo->BoardId);
          break;
	  
      case 0x7:
          UnicodeSPrint (Buffer, sizeof (Buffer), L"Leaf Hill CRB (%02x)", PlatformInfo->BoardId);
          break;

      case 0x8:
          UnicodeSPrint (Buffer, sizeof (Buffer), L"Juniper Hill CRB (%02x)", PlatformInfo->BoardId);
          break;
	  
      case 0xF:
          UnicodeSPrint (Buffer, sizeof (Buffer), L"Gordon Ridge BMP MRB (%02x)", PlatformInfo->BoardId);
          break;

      default:
          UnicodeSPrint (Buffer, sizeof (Buffer), L"Unknown BOARD (%02x)", PlatformInfo->BoardId);
      break;
    }
//[-end-160704-IB07220103-modify]//
    NewStringId = HiiSetString(MainHiiHandle, (EFI_STRING_ID)0, Buffer, NULL);  
    NewStringToHandle (
      AdvanceHiiHandle,
      STRING_TOKEN (STR_BOARD_ID_NAME),
      MainHiiHandle,
      &TitleStringId
      );
    NewStringToHandle (
      AdvanceHiiHandle,
      STRING_TOKEN (STR_BOARD_ID_HELP),
      MainHiiHandle,
      &HelpStringId
      );
    HiiCreateTextOpCode (
      StartOpCodeHandle,
      TitleStringId,
      HelpStringId,
      NewStringId
      );

    //
    // Get Board FAB ID Info from protocol, update into the NVS area.
    // bit0~bit3 are for Fab ID, 0x0F means unknow FAB.
    //
//[-start-160427-IB07400720-modify]//
////[-start-160704-IB07220103-modify]//
//    switch (PlatformInfo->BoardRev) {
////[-end-160704-IB07220103-modify]//
//      case FAB1:
//        UnicodeSPrint (Buffer, sizeof (Buffer), L"FAB1 (%02x)", PlatformInfo->BoardRev);
//        break;
//      case FAB2:
//        UnicodeSPrint (Buffer, sizeof (Buffer), L"FAB2 (%02x)", PlatformInfo->BoardRev);
//        break;
//      case FAB3:
//        UnicodeSPrint (Buffer, sizeof (Buffer), L"FAB3 (%02x)", PlatformInfo->BoardRev);
//        break;
//      default:
//        UnicodeSPrint (Buffer, sizeof (Buffer), L"Unknown FAB (%02x)", PlatformInfo->BoardRev);
//        break;
//    }
//[-start-161008-IB07400794-modify]//
    if (!mSystemConfig->IsIOTGBoardIds) {
      UnicodeSPrint (Buffer, sizeof (Buffer), L"FAB%x (%02x)", PlatformInfo->BoardRev + 1, PlatformInfo->BoardRev);
    } else {
//[-start-170825-IB07400902-modify]//
      switch (PlatformInfo->BoardRev) {
        case 0x00:
          UnicodeSPrint (Buffer, sizeof (Buffer), L"FAB A (%02x)", PlatformInfo->BoardRev);
          break;
        case 0x01:
          UnicodeSPrint (Buffer, sizeof (Buffer), L"FAB B (%02x)", PlatformInfo->BoardRev);
          break;
//[-start-170920-IB07400911-add]//
        case 0x03:
          UnicodeSPrint (Buffer, sizeof (Buffer), L"FAB C (%02x)", PlatformInfo->BoardRev);
          break;
//[-end-170920-IB07400911-add]//
        case 0x07:
          UnicodeSPrint (Buffer, sizeof (Buffer), L"FAB D (%02x)", PlatformInfo->BoardRev);
          break;
        case 0x0B:
          UnicodeSPrint (Buffer, sizeof (Buffer), L"FAB E (%02x)", PlatformInfo->BoardRev);
          break;
//[-start-170920-IB07400911-add]//
        case 0x0F:
          UnicodeSPrint (Buffer, sizeof (Buffer), L"FAB F (%02x)", PlatformInfo->BoardRev);
          break;
//[-end-170920-IB07400911-add]//
        default:
          UnicodeSPrint (Buffer, sizeof (Buffer), L"Unknown FAB (%02x)", PlatformInfo->BoardRev);
          break;
      }
//[-end-170825-IB07400902-modify]//
    }
//[-end-161008-IB07400794-modify]//
//[-end-160427-IB07400720-modify]//
    NewStringId = HiiSetString(MainHiiHandle, (EFI_STRING_ID)0, Buffer, NULL);  
    NewStringToHandle (
      AdvanceHiiHandle,
      STRING_TOKEN (STR_FAB_ID_STRING),
      MainHiiHandle,
      &TitleStringId
      );
    NewStringToHandle (
      AdvanceHiiHandle,
      STRING_TOKEN (STR_FAB_ID_HELP),
      MainHiiHandle,
      &HelpStringId
      );
    HiiCreateTextOpCode (
      StartOpCodeHandle,
      TitleStringId,
      HelpStringId,
      NewStringId
      );
  }
//[PRJ] ++ >>>> Add EC version on SCU Main page  
 {
    UINT8                    DataBuffer8[32];

    Status = CompalECReadKBCVersion(DataBuffer8);
    if(EFI_ERROR (Status)) {
      //
      // Read EC Fw version fail , set version to 5.5.0 for debug;
      //
      DataBuffer8 [0] = 5;
      DataBuffer8 [1] = 5;
      DataBuffer8 [2] = 0;
    }

    if(DataBuffer8[2] == 0x00) {
      // Formal Release EC Version
      UnicodeSPrint (Buffer, sizeof (Buffer), L"%02x (V%x.%02x)", DataBuffer8[1], DataBuffer8[0], DataBuffer8[1]);
    } else {
      // Test EC Version
      UnicodeSPrint (Buffer, sizeof (Buffer), L"%02x (V%x.%02x.T%02x)", DataBuffer8[1], DataBuffer8[0], DataBuffer8[1], DataBuffer8[2]);
    }
    NewStringId = HiiSetString(MainHiiHandle, (EFI_STRING_ID)0, Buffer, NULL);
    NewStringToHandle (
      AdvanceHiiHandle,
      STRING_TOKEN (STR_EC_VERSION_STRING),
      MainHiiHandle,
      &TitleStringId
      );
    NewStringToHandle (
      AdvanceHiiHandle,
      STRING_TOKEN (STR_EC_VERSION_HELP),
      MainHiiHandle,
      &HelpStringId
      );
    HiiCreateTextOpCode (
      StartOpCodeHandle,
      TitleStringId,
      HelpStringId,
      NewStringId
      );
  }  
//[PRJ] ++ <<<< Add EC version on SCU Main page  
  
//[-start-170613-IB07400874-add]//
  //
  // Display BIOS Boot Source (eMMC/USF/SPI)
  //
  switch (mSystemConfig->CseBootDevice) {
  case 0:
    UnicodeSPrint (Buffer, sizeof (Buffer), L"EMMC (%02x)", mSystemConfig->CseBootDevice);
    break;
  case 1:
    UnicodeSPrint (Buffer, sizeof (Buffer), L"UFS (%02x)", mSystemConfig->CseBootDevice);
    break;
  case 2:
//[-start-181001-IB07401020-modify]//
//    UnicodeSPrint (Buffer, sizeof (Buffer), L"SPI (%02x)", mSystemConfig->CseBootDevice);
    BiosRomSize  = PcdGet32(PcdFlashSpiRomSize) / (1024 * 1024);
    UnicodeSPrint (Buffer, sizeof (Buffer), L"SPI (%02x) (%02d MB)", mSystemConfig->CseBootDevice, BiosRomSize);
//[-end-181001-IB07401020-modify]//
    break;
  default:
    UnicodeSPrint (Buffer, sizeof (Buffer), L"Unknown (%02x)", mSystemConfig->CseBootDevice);
    break;
  }
  NewStringId = HiiSetString(MainHiiHandle, (EFI_STRING_ID)0, Buffer, NULL);  
  NewStringToHandle (
    AdvanceHiiHandle,
    STRING_TOKEN (STR_BIOS_BOOT_SOURCE_PROMPT),
    MainHiiHandle,
    &TitleStringId
    );
  NewStringToHandle (
    AdvanceHiiHandle,
    STRING_TOKEN (STR_BIOS_BOOT_SOURCE_HELP),
    MainHiiHandle,
    &HelpStringId
    );
  HiiCreateTextOpCode (
    StartOpCodeHandle,
    TitleStringId,
    HelpStringId,
    NewStringId
    );
//[-end-170613-IB07400874-add]//

  return EFI_SUCCESS;
}



/**
  Update platform informations on main page 

  @param [in]   SUBrowser           Pointer browser data

  @retval EFI_SUCCESS   Opeartion successfully completed.

**/
EFI_STATUS
DisplayPlatformInfo (
  IN  SETUP_UTILITY_BROWSER_DATA            *SUBrowser
  )
{
  EFI_STATUS                Status;
  EFI_HII_HANDLE            LocalMainHiiHandle;
  EFI_HII_HANDLE            LocalAdvanceHiiHandle;
  EFI_STRING_ID             TitleStringId;
  EFI_STRING_ID             BlankStringId;
  VOID                      *StartOpCodeHandle;
  EFI_IFR_GUID_LABEL        *StartLabel;

  Status                  = EFI_SUCCESS;
  LocalMainHiiHandle      = SUBrowser->SUCInfo->MapTable[MainHiiHandle].HiiHandle;
  LocalAdvanceHiiHandle   = SUBrowser->SUCInfo->MapTable[AdvanceHiiHandle].HiiHandle;

  Status = gBS->LocateProtocol (&gEfiHiiStringProtocolGuid, NULL, (VOID **)&mIfrLibHiiString);
  ASSERT_EFI_ERROR (Status);
  
//[-start-160901-IB07400777-add]//
  mSystemConfig = (CHIPSET_CONFIGURATION *)SUBrowser->SCBuffer;
//[-end-160901-IB07400777-add]//

  //
  // Allocate space for creation of Buffer
  //
  StartOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT (StartOpCodeHandle != NULL);
  
  //
  // Update Main form from MAIN_PAGE_PLATFORM_INFO_LABEL label 
  //
  StartLabel               = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (StartOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
  StartLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  StartLabel->Number       = MAIN_PAGE_PLATFORM_INFO_LABEL; 
    
  NewStringToHandle (
              LocalAdvanceHiiHandle,
              STRING_TOKEN (STR_BLANK_STRING),
              LocalMainHiiHandle,
              &BlankStringId
              );

  //
  // Spliter
  //
  HiiCreateSubTitleOpCode (StartOpCodeHandle, BlankStringId, 0, 0, 0);

  //
  // Platform firmware subtitle
  //
  NewStringToHandle (
              LocalAdvanceHiiHandle,
              STRING_TOKEN (STR_PLATFORM_FIRMWARE_STRING),
              LocalMainHiiHandle,
              &TitleStringId
              );
  HiiCreateSubTitleOpCode (StartOpCodeHandle, TitleStringId, 0, 0, 0);

  //
  // Dynamic created/detect texts
  //
  UpdatePlatformInformation (
              LocalMainHiiHandle,
              LocalAdvanceHiiHandle,
              StartOpCodeHandle
              );    

  //
  // Spliter
  //
  HiiCreateSubTitleOpCode (StartOpCodeHandle, BlankStringId, 0, 0, 0);

  
  HiiUpdateForm (
    LocalMainHiiHandle,
    NULL,
    ROOT_FORM_ID,
    StartOpCodeHandle,  
    NULL
    );
  HiiFreeOpCodeHandle (StartOpCodeHandle);
  return Status;
}
