/** @file
  Kernel Setup Configuration Data

;******************************************************************************
;* Copyright (c) 2012 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifdef _IMPORT_KERNEL_SETUP_

  //--------------------------------------------------------------------------------------------------------------
  // The following area from "Common_Part_Start" to "Common_Part_End" is the common part. Do NOT modify it.
  // The total size of variable in this part are fixed (146bytes).If you modify add or remove one of them, it will
  // cause system unstable between previous BIOS and current BIOS.
  //--------------------------------------------------------------------------------------------------------------
  //Common_Part_Start
  //Offset(0),
  UINT8         PciSlot1;              //Offset 0
  UINT8         PciSlot2;              //Offset 1
  UINT8         PciSlot3;              //Offset 2
  UINT8         PciSlot4;              //Offset 3
  UINT8         PciSlot5;              //Offset 4
  UINT8         PciSlot6;              //Offset 5
  UINT8         PciSlot7;              //Offset 6
  UINT8         PciSlot8;              //Offset 7

  UINT8         NumLock;               //Offset 8
  UINT8         UsbZipEmulation;       //Offset 9

  UINT8         ComPortA;              //Offset 10
  UINT8         ComPortABaseIo;        //Offset 11
  UINT8         ComPortAInterrupt;     //Offset 12
  UINT8         ComPortAMode;          //Offset 13
  UINT8         ComPortADma;           //Offset 14
  UINT8         ComPortB;              //Offset 15
  UINT8         ComPortBBaseIo;        //Offset 16
  UINT8         ComPortBInterrupt;     //Offset 17
  UINT8         ComPortBMode;          //Offset 18
  UINT8         ComPortBDma;           //Offset 19
  UINT8         FirHighSpeed;          //Offset 20
  UINT8         ComPortC;              //Offset 21
  UINT8         ComPortCBaseIo;        //Offset 22
  UINT8         ComPortCInterrupt;     //Offset 23
  UINT8         ComPortCMode;          //Offset 24
  UINT8         ComPortCDma;           //Offset 25
  UINT8         ComPortD;              //Offset 26
  UINT8         ComPortDBaseIo;        //Offset 27
  UINT8         ComPortDInterrupt;     //Offset 28
  UINT8         ComPortDMode;          //Offset 29
  UINT8         ComPortDDma;           //Offset 30

  UINT8         Lpt1;                  //Offset 31
  UINT8         Lpt1BaseIo;            //Offset 32
  UINT8         Lpt1Interrupt;         //Offset 33
  UINT8         Lpt1Mode;              //Offset 34
  UINT8         Lpt1Dma;               //Offset 35
  UINT8         Lpt2;                  //Offset 36
  UINT8         Lpt2BaseIo;            //Offset 37
  UINT8         Lpt2Interrupt;         //Offset 38
  UINT8         Lpt2Mode;              //Offset 39
  UINT8         Lpt2Dma;               //Offset 40
  UINT8         Lpt3;                  //Offset 41
  UINT8         Lpt3BaseIo;            //Offset 42
  UINT8         Lpt3Interrupt;         //Offset 43
  UINT8         Lpt3Mode;              //Offset 44
  UINT8         Lpt3Dma;               //Offset 45

  UINT8         FddController;         //Offset 46
  UINT8         FddWriteProtect;       //Offset 47

  UINT8         HighGamePort;          //Offset 48
  UINT8         LowGamePort;           //Offset 49

  UINT8         Midi;                  //Offset 50

  UINT8         Azalia;                //Offset 51
  UINT8         Audio;                 //Offset 52
  UINT8         Modem;                 //Offset 53

  UINT8         IdeController;         //Offset 54
  UINT8         SATASpeed;             //Offset 55

  UINT8         PataCableType;         //Offset 56
  UINT8         SataCnfigure;          //Offset 57
  UINT8         AtaIdeMode;            //Offset 58
  UINT8         IdeMode;               //Offset 59
  UINT8         LegacyConfig;          //Offset 60
  UINT8         IdeDelay;              //Offset 61
  UINT8         PSType;                //Offset 62
  UINT8         KernelRSV8[8];         //Offset 63 - 70

  UINT8         HighSpeedUsb;          //Offset 71
  UINT8         LegacyUsbSupport;      //Offset 72

  UINT8         PciLatencyTimer;       //Offset 73
  UINT8         ExtendedConfig;        //Offset 74
  UINT8         SdRamFrequency;        //Offset 75
  UINT8         SdRamTimeCtrl;         //Offset 76
  UINT8         SdRamRasActToPre;      //Offset 77
  UINT8         SdRamCasLatency;       //Offset 78
  UINT8         SdRamRasCasDelay;      //Offset 79
  UINT8         SdRamRasPrecharge;     //Offset 80

  UINT8         FacpC2LatencyValue;    //Offset 81
  UINT8         FacpC3LatencyValue;    //Offset 82
  UINT8         FacpRTCS4Wakeup;       //Offset 83
  UINT8         IoApicMode;            //Offset 84
  UINT8         HPETSupport;           //Offset 85
  UINT8         HPETAddress;           //Offset 86
  UINT8         EMASupport;            //Offset 87

  UINT8         VgaOrder;              //Offset 88
  UINT8         SoftOff;               //Offset 89
  UINT8         KBPowerOn;             //Offset 90
  UINT8         MSPowerOn;             //Offset 91
  UINT8         UsbBoot;               //Offset 92
  UINT8         EventLog;              //Offset 93
  UINT8         SystemBootEvent;       //Offset 94
  UINT8         EventLogClear;         //Offset 95
  UINT8         AcpiVer;               //Offset 96
  //
  // Tpm
  //
  UINT8         TpmDeviceOk;           //Offset 97
  UINT8         TpmOperation;          //Offset 98
  UINT8         TpmClear;              //Offset 99
  UINT8         NetworkProtocol;       //Offset 100
  UINT8         Win8FastBoot;          //Offset 101
  UINT8         Tpm2DeviceOk;          //Offset 102
  UINT8         Tpm2Enable;            //Offset 103
  UINT8         Tpm2Operation;         //Offset 104
  UINT16        Timeout;               //Offset 105 - 106
  UINT8         ActivePcrBanks;        //Offset 107
  UINT8         GrayoutTpmClear;       //Offset 108

  UINT8         QuickBoot;             //Offset 109
  UINT8         QuietBoot;             //Offset 110
  UINT8         PxeBootToLan;          //Offset 111
  UINT8         MemoryLess256MB;       //Offset 112
  UINT8         PEGFlag;               //Offset 113
  UINT16        OperatingSystem;       //Offset 114 - 115
  UINT8         BootMenuType;          //Offset 116
  UINT8         BootNormalPriority;    //Offset 117
  UINT8         LegacyNormalMenuType;  //Offset 118
  UINT8         KernelRSV4[2];         //Offset 119 - 120
  UINT8         BootType;              //Offset 121
  UINT8         KernelRSV5;            //Offset 122
  UINT16        DefaultBootType;       //Offset 123 - 124
  UINT16        NewPositionPolicy;     //Offset 125 - 126
  UINT8         Language;              //Offset 127
  UINT8         UserAccessLevel;       //Offset 128
  UINT8         SetUserPass;           //Offset 129
  UINT8         DeviceExist;           //Offset 130
  UINT8         S3ResumeSecurity;      //Offset 131
  UINT8         DisableAcpiS1;         //Offset 132
  UINT8         DisableAcpiS3;         //Offset 133
  UINT8         WakeOnPME;             //Offset 134
  UINT8         WakeOnModemRing;       //Offset 135
  UINT8         WakeOnS5;              //Offset 136
  EFI_HII_TIME  WakeOnS5Time;          //Offset 137 - 139
  UINT8         WakeOnS5DayOfMonth;    //Offset 140
  //
  // Common_Part_End
  //----------------------------------------------------------------------------------------------------------
  // The following area is used by Kernel team to modify.
  // The total size of variables in this part(Kernel_Start~Kernel_End) are fixed within 84bytes. That means if
  // you need to add or remove variables, please modify the KernelRSV buffer size as well.
  //----------------------------------------------------------------------------------------------------------
  //Kernel_Start
  //Offset(146),
  //
  UINT8         BootTypeOrder[MAX_BOOT_ORDER_NUMBER];   //Offset 141 - 156, MAX_BOOT_ORDER_NUMBER is 16
  UINT8         TpmHide;                                //Offset 157, SupervisorPassword[2] remove
  UINT8         TrEEVersion;                            //Offset 158, SupervisorPassword[2] remove
  UINT8         PowerOnPassword;                        //Offset 159
  UINT8         KernelRSV2[8];                          //Offset 160 - 167, UserPassword[2], HddPassword[2], SetAllHddPassword[2], SetAllHddPasswordFlag remove
  UINT8         SupervisorFlag;                         //Offset 168
  UINT8         UserFlag;                               //Offset 169
  UINT8         KernelRSV3[38];                          //Offset 170 - 175, UseMasterPassword, SetMasterHddPassword[2], SetAllMasterHddPassword[2], SetAllMasterHddPasswordFlag remove
                                                         //Offset 176 - 207, XXXXSetHddPasswordFlag, XXXXCBMSetHddPasswordFlag, XXXXCBMUnlockHddPasswordFlag, XXXXCBMUnlockHddPasswordFlag remove
  //
  // Console Redirection
  //
  UINT8         CREnable;                               //Offset 208
  UINT8         CRInfoWaitTime;                         //Offset 209
  UINT8         CRTerminalType;                         //Offset 210
  UINT8         CRSerialPort;                           //Offset 211
  UINT8         CRBaudRate;                             //Offset 212
  UINT8         CRParity;                               //Offset 213
  UINT8         CRDataBits;                             //Offset 214
  UINT8         CRStopBits;                             //Offset 215
  UINT8         CRFlowControl;                          //Offset 216
  UINT8         CRAfterPost;                            //Offset 217
  UINT8         ACPISpcr;                               //Offset 218
  UINT8         KernelRSV6;                             //Offset 219
  UINT8         QuickLaunchBootMenu;                    //Offset 220
  UINT8         QuickLaunchSpecialFunction;             //Offset 221
  UINT8         AutoFailover;                           //Offset 222
  UINT8         UefiDualVgaControllers;                 //Offset 223
  UINT8         CRTextModeResolution;                   //Offset 224
  UINT8         UsbHotKeySupport;                       //Offset 225
  UINT8         PUISEnable;                             //Offset 226
  UINT8         TpmDevice;                              //Offset 227
  UINT8         KernelRSV[2];                           //Offset 228 - 229, Reserve for Kernel use.
  //Kernel_End

#endif