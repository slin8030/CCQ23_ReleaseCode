/** @file
  Definition for PEI OEM Services Chipset Lib.

;******************************************************************************
;* Copyright (c) 2014 - 2015, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _PEI_OEM_SVC_CHIPSET_LIB_H_
#define _PEI_OEM_SVC_CHIPSET_LIB_H_

#include <Guid/PlatformInfo.h>
#include <Ppi/AzaliaPolicy.h>
#include <Ppi/SaPolicy.h>
//[-start-151216-IB11270137-add]//
#include <Ppi/CpuPolicy.h>
#include <Ppi/SiPolicyPpi.h>
#include <Ppi/ScPolicy.h>
//[-end-151216-IB11270137-add]//
typedef struct {
  UINT32 GpioSelect        : 3;   ///< 000:GPIO 001:GPO 010:GPI 011:HI_Z 100:Native 101~111:Reserved
  UINT32 GpioMode          : 4;   ///< 0000:M0 0001:M1 0010:M2 0011:M3 0100:M4 0101:M5 0110:M6 0111:M7 1000:M8 1001:M9 1010:M10 1011:M11 1100:M12 1101:M13 1110,1111:Reserved
  UINT32 GpioConfig        : 3;   ///< 000:GPIO 001:GPO 010:GPI 011:HI_Z 100:Native 101:NA 110,111:Reserved
  UINT32 GpioTxState       : 3;   ///< 000:LOW 001:HIGH 010:NA 011:Reserved
  UINT32 GpioLight         : 3;   ///< 000:LOW 001:HIGH 010:NA 011:Reserved    
  UINT32 GpioIntType       : 3;   ///< 000:INT_DIS 001:Trig_Edge_Low 010:Trig_Edge_High 011:Trig_Edge_Both 100:Trig_Level 101:NA 110,111:Reserved
  UINT32 GpioIntSelect     : 5;   ///< 00000:Line0 00001:Line1 00010:Line2 00011:Line3 00100:Line4 00101:Line5 00110:Line6 00111:Line7 01000:Line8 01001:Line9 01010:Line10 01011:Line11 01100:Line12 01101:Line13 01110:Line14 01111:Line15 10000:NA 10001~11111:Reserved
  UINT32 GpioTermHL        : 3;   ///< 000:P_NONE 001:P_20K_L 010:P_5K_L 011:P_1K_L 100:P_20K_H 101:P_5K_H 110:P_1K_H 111:NA 
  UINT32 GpioOpenDrain     : 2;   ///< 00:_DISABLE 01:_ENABLE 10:NA 11:Reserved
  UINT32 GpioCurrentSource : 3;   ///< 000:NA 001:Reserved
  UINT32 GpioMask          : 2;   ///< 00:Maskable 01:NonMaskable 10:NA 11:Reserved
  UINT32 GpioGlitchCfg     : 3;   ///< 000:glitch_Disable 001:En_EdgeDetect 010:En_RX_Data 011:En_Edge_RX_Data 100:NA
  UINT32 GpioInvertRXTX    : 3;   ///< 000:No_Inversion 001:Inv_RX_Enable 010:Inv_TX_Enable 011:Inv_RX_TX_Enable 100:Inv_RX_Data 101:Inv_TX_Data 110:NA 111:Reserved 
  UINT32 GpioWakeCap       : 4;   ///< 0000:MASK_WAKE 0001:UNMASK_WAKE 0010:NA 0011:Reserved
  UINT32 GpioWakeMaskBit   : 7;   ///< 0000000:0,0000001:1,0000010:2,0000011:3,0000100:4,0000101:5,0000110:6,0000111:7,0001000:8,0001001:9,0001010:10,0001011:11,0001100:12,0001101:13,0001110:14,0001111:15,0010000:16,0010001:17,0010010:18,0010011:19,0010100:20,0010101:21,0010110:22,0010111:23,0011000:24,0011001:25,0011010:26,0011011:27,0011100:28,0011101:29,0011110:30,0011111:31,0100000:32,0100001:33,0100010:34,0100011:35,0100100:36,0100101:37,0100110:38,0100111:39,0101000:40,0101001:41,0101010:42,0101011:43,0101100:44,0101101:45,0101110:46,0101111:47,0110000:48,0110001:49,0110010:50,0110011:51,0110100:52,0110101:53,0110110:54,0110111:55,0111000:56,0111001:57,0111010:58,0111011:59,0111100:60,0111101:61,0111110:62,0111111:63,1000000:NA
  UINT32 GpioGpe           : 5;   ///< 00000:GPE 00001:SMI 00010:SCI 00011:NA
  UINT32 Reserved          : 8;
} GPIO_SETTINGS_TABLE;
//[-start-160114-IB11270145-add]//
#pragma pack(1)
typedef struct {
  UINT8   Bus;
  UINT8   Device;
  UINT8   Function;
  UINT8   SvidRegisterOffset;
  UINT32  SsidSvid;
} SSID_SVID_PEI_CONFIG;
#pragma pack()
//[-end-160114-IB11270145-add]//
/**
 This function offers an interface to Dynamically change GPIO setting procedure.
 Note:The default in memory space

 @param[in, out]    PlatformGpioDataNC   On entry, points to a structure that specifies the GPIO setting. 
                                         On exit, points to the updated structure. 
                                         The default value is decided in ProgramGpioPeim.c.

 @param[in, out]    PlatformGpioDataSC   On entry, points to a structure that specifies the GPIO setting. 
                                         On exit, points to the updated structure. 
                                         The default value is decided in ProgramGpioPeim.c.
                                        
 @param[in, out]    PlatformGpioDataSUS  On entry, points to a structure that specifies the GPIO setting. 
                                         On exit, points to the updated structure. 
                                         The default value is decided in ProgramGpioPeim.c.

 @retval            EFI_UNSUPPORTED     Returns unsupported by default.
 @retval            EFI_MEDIA_CHANGED   Alter the Configuration Parameter or hook code.
 @retval            EFI_SUCCESS         The function performs the same operation as caller.
                                        The caller will skip the specified behavior and assuming
                                        that it has been handled completely by this function.
*/
EFI_STATUS
OemSvcGpioSetting (
  IN CONST EFI_PEI_SERVICES            **PeiServices,
  IN EFI_PLATFORM_INFO_HOB             *PlatformInfoHob
  );


//[-start-151216-IB11270137-modify]//
/**
This function offers an interface to Dynamically change GPIO setting procedure.
 Note:The default in memory space 

 @param[in, out]    PlatformGpioSetting_N     On entry, points to a structure that specifies the GPIO setting. 
                                              On exit, points to the updated structure. 
                                              The default value is decided in BoardGpios.c.

 @param[in, out]    PlatformGpioSetting_NW    On entry, points to a structure of that specifies the GPIO setting. 
                                              On exit, points to the updated structure. 
                                              The default value is decided in BoardGpios.c.

 @param[in, out]    PlatformGpioSetting_W     On entry, points to a structure that specifies the GPIO setting. 
                                              On exit, points to the updated structure. 
                                              The default value is decided in BoardGpios.c.
                                        
 @param[in, out]    PlatformGpioSetting_SW    On entry, points to a structure that specifies the GPIO setting. 
                                              On exit, points to the updated structure. 
                                              The default value is decided in BoardGpios.c.

 @retval            EFI_UNSUPPORTED       Returns unsupported by default.
 @retval            EFI_MEDIA_CHANGED     Alter the Configuration Parameter or hook code.
 @retval            EFI_SUCCESS           The function performs the same operation as caller.
                                          The caller will skip the specified behavior and assuming
                                          that it has been handled completely by this function.
*/
EFI_STATUS
OemSvcModifyGpioSettingTable (
  IN OUT BXT_GPIO_PAD_INIT     **PlatformGpioSetting_N,
  IN OUT BXT_GPIO_PAD_INIT     **PlatformGpioSetting_NW,
  IN OUT BXT_GPIO_PAD_INIT     **PlatformGpioSetting_W,
  IN OUT BXT_GPIO_PAD_INIT     **PlatformGpioSetting_SW
  );
//[-end-151216-IB11270137-modify]//
/**
 This function offers an interface to modify AZALIA_POLICY data before the system 
 installs AZALIA_POLICY.

 @param[in, out]   *PchAzaliaPolicyPpi            On entry, points to AZALIA_POLICY structure.
                                                  On exit, points to updated AZALIA_POLICY structure.
 @param[in, out]   *PchAzaliaPolicyPpiDesc        On entry, points to EFI_PEI_PPI_DESCRIPTOR structure.
                                                  On exit, points to updated EFI_PEI_PPI_DESCRIPTOR structure.

 @retval            EFI_UNSUPPORTED               Returns unsupported by default.
 @retval            EFI_MEDIA_CHANGED             Alter the Configuration Parameter or hook code.
 @retval            EFI_SUCCESS                   The function performs the same operation as caller.
                                                  The caller will skip the specified behavior and assuming
                                                  that it has been handled completely by this function.
*/
EFI_STATUS
OemSvcUpdateAzaliaPolicy (
  IN OUT AZALIA_POLICY                           *PchAzaliaPolicyPpi,
  IN OUT EFI_PEI_PPI_DESCRIPTOR                  *PchAzaliaPolicyPpiDesc
  );
//[-start-151222-IB11270140-modify]//
/**
 This function offers an interface to Get Board/Fab Id.


 @param[out]        *BoardId            On entry, points to BoardId.
                                        On exit, points to updated BoardId.
 @param[out]        *FabId              On entry, points to FabId.
                                        On exit, points to updated FabId.
 @retval            EFI_UNSUPPORTED     Returns unsupported by default.
 @retval            EFI_MEDIA_CHANGED   Alter the Configuration Parameter or hook code.
 @retval            EFI_SUCCESS         The function performs the same operation as caller.
                                        The caller will skip the specified behavior and assuming
                                        that it has been handled completely by this function.
*/
EFI_STATUS
OemSvcGetBoardFabIds (
  OUT UINT8                    *BoardId,
  OUT UINT8                    *FabId
  );
//[-end-151222-IB11270140-modify]//
/**
 This function offers an interface for OEM code to change the MXM GPU power enable sequence
 and modify the Switchable Graphics Information data HOB.

 @param[in]         None

 @retval            EFI_UNSUPPORTED     Returns unsupported by default.
 @retval            EFI_MEDIA_CHANGED   Alter the Configuration Parameter or hook code.
 @retval            EFI_SUCCESS         The function performs the same operation as caller.
                                        The caller will skip the specified behavior and assuming
                                        that it has been handled completely by this function.
*/
EFI_STATUS
OemSvcMxmDgpuPowerSequence (
  VOID
  );


/**
  This function offers an interface for OEM code to change the SI_SA_POLICY_PPI.

 @param[in]        **PerServices                  General purpose services available to every PEIM.
 @param[in, out]   *ChvPolicyPpi                  On entry, points to SI_SA_POLICY_PPI structure.
                                                  On exit, points to updated SI_SA_POLICY_PPI structure.

 @retval            EFI_UNSUPPORTED               Returns unsupported by default.
 @retval            EFI_MEDIA_CHANGED             Alter the Configuration Parameter or hook code.
 @retval            EFI_SUCCESS                   The function performs the same operation as caller.

**/
EFI_STATUS
OemSvcUpdatePeiChvPlatformPolicy (
  IN CONST    EFI_PEI_SERVICES    **PeiServices,
  IN OUT      SI_SA_POLICY_PPI      *ChvPolicyPpi
  );
//[-start-151216-IB11270137-add]//
/**
  This function offers an interface for OEM code to change the SI_CPU_POLICY_PPI.

 @param[in]        **PerServices                  General purpose services available to every PEIM.
 @param[in, out]   *SiCpuPolicyPpi                On entry, points to SI_CPU_POLICY_PPI structure.
                                                  On exit, points to updated SI_CPU_POLICY_PPI structure.

 @retval            EFI_UNSUPPORTED               Returns unsupported by default.
 @retval            EFI_MEDIA_CHANGED             Alter the Configuration Parameter or hook code.
 @retval            EFI_SUCCESS                   The function performs the same operation as caller.

**/
EFI_STATUS
OemSvcUpdatePeiCpuPlatformPolicy (
  IN CONST    EFI_PEI_SERVICES     **PeiServices,
  IN OUT      SI_CPU_POLICY_PPI    *SiCpuPolicyPpi
  );
/**
 This function offers an interface to modify SI_POLICY_PPI data before the system 
 installs SI_POLICY_PPI.

 @param[in, out]   *SiPlatformPolicyPpi           On entry, points to SI_POLICY_PPI structure.
                                                  On exit, points to updated SI_POLICY_PPI structure.

 @retval            EFI_UNSUPPORTED               Returns unsupported by default.
 @retval            EFI_MEDIA_CHANGED             Alter the Configuration Parameter.
 @retval            EFI_SUCCESS                   The function performs the same operation as caller.
                                                  The caller will skip the specified behavior and assuming
                                                  that it has been handled completely by this function.
*/

EFI_STATUS
OemSvcUpdatePeiSiPlatformPolicy (
  IN OUT SI_POLICY_PPI                   *SiPlatformPolicyPpi
  );

/**
 This function offers an interface to modify SC_POLICY_PPI data before the system 
 installs SC_POLICY_PPI.

 @param[in, out]    *ScPolicyPpi                  On entry, points to SC_POLICY_PPI structure.
                                                  On exit, points to updated SC_POLICY_PPI structure.

 @retval            EFI_UNSUPPORTED               Returns unsupported by default.
 @retval            EFI_MEDIA_CHANGED             Alter the Configuration Parameter.
 @retval            EFI_SUCCESS                   The function performs the same operation as caller.
                                                  The caller will skip the specified behavior and assuming
                                                  that it has been handled completely by this function.
*/
EFI_STATUS
OemSvcUpdateScPlatformPolicy (
  IN OUT SC_POLICY_PPI                         *ScPolicyPpi
  );


/**
 This function offers an interface to modify SI_SA_POLICY_PPI data before the system 
 installs SI_SA_POLICY_PPI.

 @param[in, out]    *SaPolicyPpi                  On entry, points to SI_SA_POLICY_PPI structure.
                                                  On exit, points to updated SI_SA_POLICY_PPI structure.

 @retval            EFI_UNSUPPORTED               Returns unsupported by default.
 @retval            EFI_MEDIA_CHANGED             Alter the Configuration Parameter.
 @retval            EFI_SUCCESS                   The function performs the same operation as caller.
                                                  The caller will skip the specified behavior and assuming
                                                  that it has been handled completely by this function.
*/
EFI_STATUS
OemSvcUpdateSaPlatformPolicy (
  IN OUT SI_SA_POLICY_PPI                         *SaPolicyPpi
  );
//[-end-151216-IB11270137-add]//

//[-start-160114-IB11270145-add]//
/**
 This function offers an interface to modify SSID_SVID_PEI_CONFIG data before the system 
 update SSID/SVID in PEI phase.

 @param[in, out]   *SsidSvidTbl          On entry, points to SSID_SVID_PEI_CONFIG structure.
                                         On exit, points to SSID_SVID_PEI_CONFIG structure.

 @param[in, out]   *SsidSvidTblSize      On entry, points to a value that the length of SSID_SVID_PEI_CONFIG table 
                                         On exit, points to the updated value.


 @retval            EFI_UNSUPPORTED      Returns unsupported by default.
 @retval            EFI_MEDIA_CHANGED    Alter the Configuration Parameter.
 @retval            EFI_SUCCESS          The function performs the same operation as caller.
                                         The caller will skip the specified behavior and assuming
                                         that it has been handled completely by this function.
*/
EFI_STATUS
OemSvcUpdateSsidSvidPei (
  IN OUT SSID_SVID_PEI_CONFIG         **SsidSvidTbl,
  IN OUT UINTN                        *SsidSvidTblSize
  );
//[-end-160114-IB11270145-add]//
#endif
