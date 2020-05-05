/*++
  This file contains an 'Intel Peripheral Driver' and is        
  licensed for Intel CPUs and chipsets under the terms of your  
  license agreement with Intel or your vendor.  This file may   
  be modified by the user, subject to additional terms of the   
  license agreement                                             
--*/
/** @file
  Null instance of Sec Platform Hook Lib.

  Copyright (c) 2007, Intel Corporation. All rights reserved.<BR>
  This software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
  with the terms of the license. Except as permitted by such
  license, no part of this software or documentation may be
  reproduced, stored in a retrieval system, or transmitted in any
  form or by any means without the express written consent of
  Intel Corporation.


**/

//
// The package level header files this module uses
//
#include "PlatformSecLib.h"
//[-start-160216-IB03090424-add]//
#include <Library/PreSiliconLib.h>
#include <Library/DebugLib.h>
//[-end-160216-IB03090424-add]//
#include <Library/IoLib.h>
#include <Library/MmPciLib.h>
#include <Library/GpioLib.h>
#include <Library/ScSerialIoUartLib.h>
//[-start-160915-IB07400785-add]//
#include <Library/ScPlatformLib.h> 
//[-end-160915-IB07400785-add]//
//[-start-180621-IB07400979-add]//
#include <Library/PlatformCmosLib.h>
//[-end-180621-IB07400979-add]//

//[-start-161107-IB07400810-add]//
#define GPIO_INIT_IN_SEC_PHASE
#ifdef APOLLOLAKE_CRB 
// CRB platform
#include <BoardGpiosRvp.h>
#include <BoardGpiosOxbowHill.h>

#else 
// OEM platform
#include <OemBoardGpios.h>
#endif
#if 0
//[-end-161107-IB07400810-add]//
//[-start-160216-IB03090424-modify]//
BXT_GPIO_PAD_INIT  UartGpio_Sec [] = 
//[-end-160216-IB03090424-modify]//
{
  //                   Group Pin#:  pad_name,    PMode,GPIO_Config,HostSw,GPO_STATE,INT_Trigger,Wake_Enabled, Term_H_L, Inverted,GPI_ROUT,IOSstae, IOSTerm,   MMIO_Offset, Community
  BXT_GPIO_PAD_CONF(L"GPIO_46 LPSS_UART2_RXD",   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,      NA  ,     NA, GPIO_PADBAR+0x0170,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_47 LPSS_UART2_TXD",   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,      NA  ,     NA, GPIO_PADBAR+0x0178,  NORTH),
};


//[-start-160720-IB03090429-modify]//
BXT_GPIO_PAD_INIT  LpcGpio_Sec [] =
{
  //                 Group Pin#:  pad_name,     PMode,GPIO_Config,HostSw,GPO_STATE,INT_Trigger, Wake_Enabled, Term_H_L,Inverted,GPI_ROUT,   IOSstae, IOSTerm,    MMIO_Offset, Community
  BXT_GPIO_PAD_CONF(L"LPC_ILB_SERIRQ",             M1,     NA    ,   NA ,  NA     ,   NA      ,Wake_Disabled, P_20K_H ,   NA   ,    NA  ,IOS_Masked, SAME   ,GPIO_PADBAR+0x0110 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"LPC_CLKOUT0",                M1,     NA    ,   NA ,  NA     ,   NA      ,Wake_Disabled, P_NONE  ,   NA   ,    NA  ,HizRx1I   , DisPuPd,GPIO_PADBAR+0x0118 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"LPC_CLKOUT1",                M1,     NA    ,   NA ,  NA     ,   NA      ,Wake_Disabled, P_NONE  ,   NA   ,    NA  ,HizRx1I   , DisPuPd,GPIO_PADBAR+0x0120 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"LPC_AD0",                    M1,     NA    ,   NA ,  NA     ,   NA      ,Wake_Disabled, P_20K_H ,   NA   ,    NA  ,HizRx1I   , DisPuPd,GPIO_PADBAR+0x0128 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"LPC_AD1",                    M1,     NA    ,   NA ,  NA     ,   NA      ,Wake_Disabled, P_20K_H ,   NA   ,    NA  ,HizRx1I   , DisPuPd,GPIO_PADBAR+0x0130 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"LPC_AD2",                    M1,     NA    ,   NA ,  NA     ,   NA      ,Wake_Disabled, P_20K_H ,   NA   ,    NA  ,HizRx1I   , DisPuPd,GPIO_PADBAR+0x0138 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"LPC_AD3",                    M1,     NA    ,   NA ,  NA     ,   NA      ,Wake_Disabled, P_20K_H ,   NA   ,    NA  ,HizRx1I   , DisPuPd,GPIO_PADBAR+0x0140 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"LPC_CLKRUNB",                M1,     NA    ,   NA ,  NA     ,   NA      ,Wake_Disabled, P_20K_H ,   NA   ,    NA  ,HizRx1I   , DisPuPd,GPIO_PADBAR+0x0148 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"LPC_FRAMEB",                 M1,     NA    ,   NA ,  NA     ,   NA      ,Wake_Disabled, P_20K_H ,   NA   ,    NA  ,HizRx1I   , DisPuPd,GPIO_PADBAR+0x0150 ,  SOUTHWEST),
};

BXT_GPIO_PAD_INIT  SmbusGpio_Sec [] =
{
  //                   Group Pin#:  pad_name,    PMode,GPIO_Config,HostSw,GPO_STATE,INT_Trigger,Wake_Enabled, Term_H_L, Inverted,GPI_ROUT,IOSstae, IOSTerm,   MMIO_Offset, Community
  BXT_GPIO_PAD_CONF(L"SMB_CLK",                    M1,     NA    ,   NA ,  NA     ,   NA      ,Wake_Disabled, P_20K_H ,   NA   ,    NA  ,IOS_Masked, SAME   ,GPIO_PADBAR+0x0100 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"SMB_DATA",                   M1,     NA    ,   NA ,  NA     ,   NA      ,Wake_Disabled, P_20K_H ,   NA   ,    NA  ,IOS_Masked, SAME   ,GPIO_PADBAR+0x0108 ,  SOUTHWEST),
};
//[-end-160720-IB03090429-modify]//

//[-start-161107-IB07400810-add]//
#endif
//[-end-161107-IB07400810-add]//

/**
  Perform those platform specific operations that are requried to be executed as early as possibile.
  
  @return TRUE always return true.
**/
EFI_STATUS
EFIAPI
PlatformSecLibConstructor (
  )
{
  BOOLEAN                       DefaultsRestored;

  //
  // Init Apic Timer for Performance collection.
  // Use EXCEPT_IA32_BOUND as interrupte type.
  //
  //PERF_CODE (
  //  InitializeApicTimer (0, (UINT32) -1, TRUE, 5);
  //);

  DefaultsRestored = FALSE;
  
  //
  // Perform a checksum computation and verify if the checksum is correct. If the checksum is incorrect
  // initialize all the CMOS location to their default values and recalculate the checksum.
  //
//[-start-161008-IB07400794-remove]//
//#if BXTI_PF_ENABLE
//  InitCmos (FALSE, &DefaultsRestored);
//#endif
//[-end-161008-IB07400794-remove]//

  return EFI_SUCCESS;
}
//[-start-160827-IB07400773-add]//
#define BASE_TICKS              3333333LL

STATIC
VOID
Stall (
  IN UINTN                      MilliSecond
  )
/*++

Routine Description:
 
  Stall for millisecond

Arguments:

  MilliSecond
  
Returns:

--*/
{
  UINT64                Tsc;

  Tsc = AsmReadTsc () + BASE_TICKS * MilliSecond;
  while (Tsc > AsmReadTsc());
}
//[-end-160827-IB07400773-add]//

//[-start-160819-IB07400772-add]//
VOID
UartWriteData (
  IN UINT16 UartBaseAddr,
  IN UINT8  UartData
  )
{
  while (!(IoRead8 (UartBaseAddr + 5) & BIT6));
  IoWrite8 (UartBaseAddr, UartData);
}

VOID
ComPortInit (
  IN  UINT16  ComPortBaseAddr
  )
{
  UINT8                               Data8;
  
  // Always Reset Com Port as 115200 8-n-1 for COM port DDT/EDBG
  Data8 = 8 - 5;    // 8 Bit
  Data8 &= (~BIT3); // No Parity
//[-start-161023-IB07400803-modify]//
  Data8 &= (~BIT2); // 1 Stop
//[-end-161023-IB07400803-modify]//
  IoWrite8 (ComPortBaseAddr + 3, Data8 | BIT7); // set baud rate
  
  IoWrite8 (ComPortBaseAddr, 0x01); // 115200
  IoWrite8 (ComPortBaseAddr + 1, 0x00); 

  Data8 &= ~(BIT7);
  IoWrite8 (ComPortBaseAddr + 3, Data8); 

  // Enable FIFO
  Data8 = IoRead8 (ComPortBaseAddr + 2);
  Data8 |= (BIT0 | BIT5); 
  IoWrite8 (ComPortBaseAddr + 2, Data8); 
  
  return;
}

VOID
SioIndexWrite (
  IN UINT8   Port,
  IN UINT8   Index,
  IN UINT8   Data
  )
{
  IoWrite8 (Port, Index);
  IoWrite8 (Port + 1, Data);
//[-start-160827-IB07400773-add]//
//[-start-161002-IB07400791-remove]//
//  Stall (1000); // Stall to wait SIO ready
//[-end-161002-IB07400791-remove]//
//[-end-160827-IB07400773-add]//
}

UINT8
SioIndexRead (
  IN UINT8  Port,
  IN UINT8  Index
  )
{
  IoWrite8 (Port, Index);
  return IoRead8 (Port + 1);
}

VOID
SioUartDebugInit (
  VOID
  )
{
  //
  // ToDo: Add OEM SIO early init code
  //
#ifdef W8374LF2_SIO_SUPPORT
  {
    UINT8   IndexPort = 0x2E;
    UINT16  ComPortBaseAddress;
//[-start-161023-IB07400803-add]//
    UINT16  Data16;
//[-end-161023-IB07400803-add]//

//[-start-161018-IB07400798-add]//
    ComPortBaseAddress = (UINT16)PcdGet64 (PcdSerialRegisterBase);
//[-end-161018-IB07400798-add]//
    //
    // Program and Enable SIO Base Addresses for Com1(3F8-3FF)/Com2(2F8-2FF)/EC(62/66)/KBC(60/64)
    // and enable decode range 2E,2F/4E,4F
    //
//[-start-161023-IB07400803-modify]//
//    IoWrite32 (0xcf8, 0x8000f880);
//    IoWrite32 (0xcfc, 0x3c030010);
    PchLpcIoDecodeRangesGet (&Data16);
    Data16 &= (~B_PCH_LPC_IOD_COMA);
    switch (ComPortBaseAddress) {
    case 0x3F8:
      Data16 |= (V_PCH_LPC_IOD_COMA_3F8 << N_PCH_LPC_IOD_COMA);
      break;
    case 0x2F8:
      Data16 |= (V_PCH_LPC_IOD_COMA_2F8 << N_PCH_LPC_IOD_COMA);
      break;
    case 0x220:
      Data16 |= (V_PCH_LPC_IOD_COMA_220 << N_PCH_LPC_IOD_COMA);
      break;
    case 0x228:
      Data16 |= (V_PCH_LPC_IOD_COMA_228 << N_PCH_LPC_IOD_COMA);
      break;
    case 0x238:
      Data16 |= (V_PCH_LPC_IOD_COMA_238 << N_PCH_LPC_IOD_COMA);
      break;
    case 0x2E8:
      Data16 |= (V_PCH_LPC_IOD_COMA_2E8 << N_PCH_LPC_IOD_COMA);
      break;
    case 0x338:
      Data16 |= (V_PCH_LPC_IOD_COMA_338 << N_PCH_LPC_IOD_COMA);
      break;
    case 0x3E8:
      Data16 |= (V_PCH_LPC_IOD_COMA_3E8 << N_PCH_LPC_IOD_COMA);
      break;
    default:
      // TODO: config generic decode
      break;
    }
    PchLpcIoDecodeRangesSet (Data16);

    //
    // Enable COMA decode
    //
    PchLpcIoEnableDecodingGet (&Data16);
    Data16 |= B_PCH_LPC_IOE_CAE; // Enable COMA
    PchLpcIoEnableDecodingSet (Data16);
//[-end-161023-IB07400803-modify]//

    //
    // Init UART for Debug
    //
    if (SioIndexRead (IndexPort, 0x20) == 0xF1) {
      SioIndexWrite (IndexPort, 0x07, 0x03); //COM1
      SioIndexWrite (IndexPort, 0x60, (ComPortBaseAddress >> 8) & 0xFF); //MSB
      SioIndexWrite (IndexPort, 0x61, (ComPortBaseAddress) & 0xFF); //LSB
      SioIndexWrite (IndexPort, 0x70, 0x04); //IRQ
      SioIndexWrite (IndexPort, 0x30, 0x01); //Active
      ComPortInit(ComPortBaseAddress);
      if (FeaturePcdGet(PcdUartPostCodeSupport)) { 
        //
        // Debug Uart Post Code Message
        //
        UartWriteData (ComPortBaseAddress, 'P'); 
        UartWriteData (ComPortBaseAddress, 'o'); 
        UartWriteData (ComPortBaseAddress, 's'); 
        UartWriteData (ComPortBaseAddress, 't'); 
        UartWriteData (ComPortBaseAddress, 'C'); 
        UartWriteData (ComPortBaseAddress, 'o'); 
        UartWriteData (ComPortBaseAddress, 'd'); 
        UartWriteData (ComPortBaseAddress, 'e'); 
        UartWriteData (ComPortBaseAddress, 0x0D); 
        UartWriteData (ComPortBaseAddress, 0x0A); 
      }
    }
  }
#endif
//[-start-161222-IB07400829-add]//
//
// Example Code for SIO IT8728F 
//
#ifdef SIO_IT8728F_SUPPORT
  {
    UINT8   IndexPort = 0x2E;
    UINT16  ComPortBaseAddress;
    UINT16  Data16;

    ComPortBaseAddress = (UINT16)PcdGet64 (PcdSerialRegisterBase);
    //
    // Program and Enable SIO Base Addresses for Com1(3F8-3FF)/Com2(2F8-2FF)/EC(62/66)/KBC(60/64)
    // and enable decode range 2E,2F/4E,4F
    //
    PchLpcIoDecodeRangesGet (&Data16);
    Data16 &= (~B_PCH_LPC_IOD_COMA);
    switch (ComPortBaseAddress) {
    case 0x3F8:
      Data16 |= (V_PCH_LPC_IOD_COMA_3F8 << N_PCH_LPC_IOD_COMA);
      break;
    case 0x2F8:
      Data16 |= (V_PCH_LPC_IOD_COMA_2F8 << N_PCH_LPC_IOD_COMA);
      break;
    case 0x220:
      Data16 |= (V_PCH_LPC_IOD_COMA_220 << N_PCH_LPC_IOD_COMA);
      break;
    case 0x228:
      Data16 |= (V_PCH_LPC_IOD_COMA_228 << N_PCH_LPC_IOD_COMA);
      break;
    case 0x238:
      Data16 |= (V_PCH_LPC_IOD_COMA_238 << N_PCH_LPC_IOD_COMA);
      break;
    case 0x2E8:
      Data16 |= (V_PCH_LPC_IOD_COMA_2E8 << N_PCH_LPC_IOD_COMA);
      break;
    case 0x338:
      Data16 |= (V_PCH_LPC_IOD_COMA_338 << N_PCH_LPC_IOD_COMA);
      break;
    case 0x3E8:
      Data16 |= (V_PCH_LPC_IOD_COMA_3E8 << N_PCH_LPC_IOD_COMA);
      break;
    default:
      // TODO: config generic decode
      break;
    }
    Data16 &= (~B_PCH_LPC_IOD_COMB);
    Data16 |= (V_PCH_LPC_IOD_COMB_2F8 << N_PCH_LPC_IOD_COMB);
    PchLpcIoDecodeRangesSet (Data16);

    //
    // Enable COMA decode
    //
    PchLpcIoEnableDecodingGet (&Data16);
    Data16 |= B_PCH_LPC_IOE_CAE; // Enable COMA
    Data16 |= B_PCH_LPC_IOE_CBE; // Enable COMB
    PchLpcIoEnableDecodingSet (Data16);


    //
    // Enter SIO
    //
    IoWrite8 (IndexPort, 0x87);
    IoWrite8 (IndexPort, 0x01);
    IoWrite8 (IndexPort, 0x55);
    IoWrite8 (IndexPort, 0x55);
    
    //
    // Init UART for Debug
    //
    if (SioIndexRead (IndexPort, 0x20) == 0x87) {
      SioIndexWrite (IndexPort, 0x07, 0x01); //COM1
      SioIndexWrite (IndexPort, 0x60, (ComPortBaseAddress >> 8) & 0xFF); //MSB
      SioIndexWrite (IndexPort, 0x61, (ComPortBaseAddress) & 0xFF); //LSB
      SioIndexWrite (IndexPort, 0x70, 0x04); //IRQ
      SioIndexWrite (IndexPort, 0x30, 0x01); //Active
      ComPortInit(ComPortBaseAddress);
      if (FeaturePcdGet(PcdUartPostCodeSupport)) { 
        //
        // Debug Uart Post Code Message
        //
        UartWriteData (ComPortBaseAddress, 'P'); 
        UartWriteData (ComPortBaseAddress, 'o'); 
        UartWriteData (ComPortBaseAddress, 's'); 
        UartWriteData (ComPortBaseAddress, 't'); 
        UartWriteData (ComPortBaseAddress, 'C'); 
        UartWriteData (ComPortBaseAddress, 'o'); 
        UartWriteData (ComPortBaseAddress, 'd'); 
        UartWriteData (ComPortBaseAddress, 'e'); 
        UartWriteData (ComPortBaseAddress, 0x0D); 
        UartWriteData (ComPortBaseAddress, 0x0A); 
      }
    }
    
    IoWrite8 (IndexPort, 0xAA);
  }
#endif
//[-end-161222-IB07400829-add]//
}
//[-end-160819-IB07400772-add]//

/**
  A developer supplied function to perform platform specific operations.

  It's a developer supplied function to perform any operations appropriate to a
  given platform. It's invoked just before passing control to PEI core by SEC
  core. Platform developer may modify the SecCoreData and PPI list that is
  passed to PEI Core. 

  @param  SecCoreData           The same parameter as passing to PEI core. It
                                could be overridden by this function.
  @param  PpiList               The default PPI list passed from generic SEC
                                part.

  @return The final PPI list that platform wishes to passed to PEI core.

**/
EFI_PEI_PPI_DESCRIPTOR *
EFIAPI
SecPlatformMain (
  IN OUT   EFI_SEC_PEI_HAND_OFF        *SecCoreData,
  IN       EFI_PEI_PPI_DESCRIPTOR      *PpiList
  )
{
  UINTN                   P2sbBase;
//[-start-160307-IB07400708-add]//
  UINT8                   PostCodeBuffer[]    = {"UART POST INIT!!\r\n"};
  UINT8                   PostCodeBufferSize  = sizeof (PostCodeBuffer);
//[-end-160307-IB07400708-add]//

//[-start-161107-IB07400810-add]//
#ifdef APOLLOLAKE_CRB
  //
  // CRB SEC phase GPIO init for debug
  //
  //
  // Get P2SB Base
  //
  P2sbBase = MmPciBase (0, 13, 0);
  
  //
  // Set SBREG base address.
  //
  MmioWrite32 (P2sbBase + 0x10, 0xD0000000);
  //
  // Enable the MSE bit for MMIO decode.
  //
  MmioOr8 (P2sbBase + 0x04, 0x0002);

  //
  // Init Gpio
  // Note. We don't know the Board ID in SEC phase,
  // This is common GPIO setting for all CRB
  //
#if defined(BUILD_IOTG_CRB)  
  GpioPadConfigTable(sizeof(mSecGpioInitData_LH)/sizeof(mSecGpioInitData_LH[0]), mSecGpioInitData_LH);
#elif defined(BUILD_CCG_CRB)  
  GpioPadConfigTable(sizeof(mSecGpioInitData_RVP)/sizeof(mSecGpioInitData_RVP[0]), mSecGpioInitData_RVP);
#endif

#else
  //
  // OEM SEC phase GPIO init for debug
  //
#ifdef mSecGpioInitData_OEM_Support
  //
  // Get P2SB Base
  //
  P2sbBase = MmPciBase (0, 13, 0);
  
  //
  // Set SBREG base address.
  //
  MmioWrite32 (P2sbBase + 0x10, 0xD0000000);
  //
  // Enable the MSE bit for MMIO decode.
  //
  MmioOr8 (P2sbBase + 0x04, 0x0002);

  //
  // Init Gpio
  //
  GpioPadConfigTable(sizeof(mSecGpioInitData_OEM)/sizeof(mSecGpioInitData_OEM[0]), mSecGpioInitData_OEM);

#endif
#endif
//[-end-161107-IB07400810-add]//

//[-start-160819-IB07400772-modify]//
//[-start-161108-IB07400810-modify]//
  //
  // Init HS-UART
  //
  if (FeaturePcdGet(PcdHsUartDebugSupport)) {
    //
    // Initial Serial IO UART2 for debug interface
    //
    PchSerialIoUartInit (PcdGet8 (PcdSerialIoUartNumber), TRUE, 115200, 3, FALSE);

    //
    // Dump UART POST CODE Init Message
    //
    if (FeaturePcdGet(PcdUartPostCodeSupport)) { 
      PchSerialIoUartOut (PcdGet8 (PcdSerialIoUartNumber), &PostCodeBuffer[0], PostCodeBufferSize);
    }
  }
//[-end-161108-IB07400810-modify]//
//[-end-160819-IB07400772-modify]//
//[-start-160216-IB03090424-modify]//
//[-start-161108-IB07400810-remove]//
//  GpioPadConfigTable(sizeof(LpcGpio_Sec)/sizeof(LpcGpio_Sec[0]), LpcGpio_Sec);
//[-end-161108-IB07400810-remove]//
//[-end-160216-IB03090424-modify]//
//[-start-160915-IB07400785-add]//
//#if (TABLET_PF_ENABLE == 0)
#if 0
  //
  // Enable Legacy IO decode
  //
  PchLpcIoDecodeRangesSet (
    (V_PCH_LPC_IOD_LPT_378  << N_PCH_LPC_IOD_LPT)  |
    (V_PCH_LPC_IOD_COMB_3E8 << N_PCH_LPC_IOD_COMB) |
    (V_PCH_LPC_IOD_COMA_3F8 << N_PCH_LPC_IOD_COMA)
    );

  PchLpcIoEnableDecodingSet (
    B_PCH_LPC_IOE_ME2  |  // 0x4E, 0x4F
    B_PCH_LPC_IOE_SE   |  // 0x2E, 0x2F
    B_PCH_LPC_IOE_ME1  |  // 0x62, 0x66
    B_PCH_LPC_IOE_KE   |  // 0x60, 0x64
    B_PCH_LPC_IOE_HGE  |  // 0x208 ~ 0x20F
    B_PCH_LPC_IOE_LGE  |  // 0x200 ~ 0x207
    B_PCH_LPC_IOE_FDE  |  // FDO
    B_PCH_LPC_IOE_PPE  |  // LPT
    B_PCH_LPC_IOE_CBE  |  // COMB
    B_PCH_LPC_IOE_CAE     // COMA
    );
#endif
//[-end-160915-IB07400785-add]//
//[-start-160819-IB07400772-add]//
  //
  // Init SIO UART
  //
  if (FeaturePcdGet(PcdUartDebugSupport)) { 
    SioUartDebugInit();
  }
//[-end-160819-IB07400772-add]//
//[-start-160720-IB03090429-add]//
//[-start-161108-IB07400810-remove]//
//  GpioPadConfigTable(sizeof(SmbusGpio_Sec)/sizeof(SmbusGpio_Sec[0]), SmbusGpio_Sec);
//[-end-161108-IB07400810-remove]//
//[-end-160720-IB03090429-add]//

//[-start-180621-IB07400979-add]//
  if (!CheckCmosBatteryStatus()) { // CMOS data missing, restore default value
    ChipsetLoadCmosDefault();
  }
//[-end-180621-IB07400979-add]//

  return NULL;
}


//[-start-160216-IB03090424-modify]//
/**
  This interface conveys state information out of the Security (SEC) phase into PEI.

  @param[in]      PeiServices               Pointer to the PEI Services Table.
  @param[in][out] StructureSize             Pointer to the variable describing size of the input buffer.
  @param[out]     PlatformInformationRecord Pointer to the EFI_SEC_PLATFORM_INFORMATION_RECORD.

  @retval         EFI_SUCCESS               The data was successfully returned.
  @retval         EFI_BUFFER_TOO_SMALL      The buffer was too small.
**/
EFI_STATUS
EFIAPI
SecPlatformInformation (
  IN CONST EFI_PEI_SERVICES                      **PeiServices,
  IN OUT   UINT64                                *StructureSize,
  OUT      EFI_SEC_PLATFORM_INFORMATION_RECORD   *PlatformInformationRecord
  )
{

  UINT32  *Bist;
  UINT32  TopOfCar;
  
  DEBUG ((EFI_D_INFO, "SecPlatformInformation entry \n"));
  if (PLATFORM_ID != VALUE_REAL_PLATFORM) {
    return EFI_UNSUPPORTED;
  }
  ///
  /// The entries of BIST information, together with the number of them,
  /// reside in the bottom of stack, left untouched by normal stack operation.
  /// This routine copies the BIST information to the buffer pointed by
  /// PlatformInformationRecord for output.
  ///

  TopOfCar = CAR_BASE_ADDR + CAR_SIZE;
  ///
  /// At this stage we only have information about the BSP.
  ///
  Bist = (UINT32 *) (UINTN) (TopOfCar - sizeof (UINT32) - sizeof (UINT32));

  CopyMem (PlatformInformationRecord, Bist, (UINTN)*StructureSize);
  DEBUG ((EFI_D_INFO, "Bist is = %x \n", Bist ));
  return EFI_SUCCESS;
}
//[-end-160216-IB03090424-modify]//
