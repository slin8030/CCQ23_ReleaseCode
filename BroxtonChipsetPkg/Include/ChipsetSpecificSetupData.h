/** @file
  Chipset Setup Configuration Data

;******************************************************************************
;* Copyright (c) 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifdef _IMPORT_CHIPSET_SPECIFIC_SETUP_

//==========================================================================================================
//==========================================================================================================
//----------------------------------------------------------------------------------------------------------
// The total size of variables in this part(Chipset_Start ~ Chipset_End) are fixed(470bytes). That means if
// you need to add or remove variables, please modify the ChipsetRSV buffer size as well.
//----------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------
// Please setup up the Chipset dependent variables here.
//----------------------------------------------------------------------------------------------------------
//Chipset_Start
//Offset(230),
  //
  // Processor (CPU)
  //
//[-start-160104-IB03090424-remove]//
//  UINT8         CpuidMaxValue;
//[-end-160104-IB03090424-remove]//
  UINT8         ProcessorVmxEnable;
  UINT8         ActiveProcessorCores;
//[-start-151211-IB03090424-add]//
  UINT8         Core0;
  UINT8         Core1;
  UINT8         Core2;
  UINT8         Core3;
//[-end-151211-IB03090424-add]//
  UINT8         TurboModeEnable;
  UINT8         ProcessorHtMode;
  UINT8         ProcessorXapic;                       // Enable Processor XAPIC
  UINT8         ProcessorXEEnable;                    // Enabling XE
  UINT8         ProcessorTDCLimitOverrideEnable;      // Turbo-XE Mode Processor TDC Limit Override Enable
  UINT16        ProcessorTDCLimit;                    // Turbo-XE Mode Processor TDC Limit
  UINT8         ProcessorTDPLimitOverrideEnable;      // Turbo-XE Mode Processor TDP Limit Override Enable
  UINT16        ProcessorTDPLimit;                    // Turbo-XE Mode Processor TDP Limit
  UINT8         ProcessorVirtualWireMode;             // Virtual wire A or B
  UINT8         ProcessorEistEnable;                  // EIST or GV3 setup option
  UINT8         ProcessorBistEnable;                  // Enabling BIST
  UINT8         BspSelection;                         // Select BSP
//[-start-160720-IB03090429-add]//
  UINT8         MonitorMwaitEnable;
//[-end-160720-IB03090429-add]//

  //
  // Ratio Limit options for Turbo-Mode
  //
  UINT8         RatioLimit4C;
  UINT8         RatioLimit3C;
  UINT8         RatioLimit2C;
  UINT8         RatioLimit1C;

  //
  // Hardware Prefetch
  //
  UINT8         MlcStreamerPrefetcherEnable;

  //
  // Adjacent Cache Line Prefetch
  //
  UINT8         MlcSpatialPrefetcherEnable;

  //
  // DCU Streamer Prefetcher
  //
  UINT8         DCUStreamerPrefetcherEnable;

  //
  // DCU IP Prefetcher
  //
  UINT8         DCUIPPrefetcherEnable;

  //
  // for PpmPlatformPlicy
  //
  UINT8         EnableProcHot;
  UINT8         EnableTm;
  UINT8         EnableGv;
  UINT8         BootPState;
  UINT8         TStatesEnable;
  UINT8         HTD;
  UINT8         FlexRatio;
  UINT8         FlexVid;
  UINT8         Cstates;
  UINT8         EnableC4;
  UINT8         EnableC6;
  UINT8         EnableCx;
  UINT8         EnableCxe;

  //
  // Wake on RTC variables
  //
//[-start-170314-IB07400847-modify]//
  UINT8         WakeOnRtcS5_Reserved;
//[-end-170314-IB07400847-modify]//
  UINT8         RTCWakeupDate;
  UINT8         RTCWakeupTimeHour;
  UINT8         RTCWakeupTimeMinute;
  UINT8         RTCWakeupTimeSecond;

  //
  // Wake On Lan
  //
  UINT8         WakeOnLanS5;
//[-start-160216-IB03090424-remove]//
//  //Spread spectrum
//  UINT8         SpreadSpectrum;
//[-end-160216-IB03090424-remove]//

  //
  // System ports
  //
//  UINT8         Serial;
//  UINT8         Serial2;
//  UINT8         Parallel;
//  UINT8         ParallelMode;
//  UINT8         AllUsb;
//  UINT8         Usb2;
  UINT8         UsbLegacy;
  //
  // Fixed Disk Boot Sector (Fdbs)
  //
  UINT8         Fdbs;

  //
  // Flash update sleep delay
  //
  //
  UINT8         FlashSleepDelay;
  //
  // Video
  //
  //
  UINT8         Igd;
  UINT8         GOPEnable;
  UINT8         GOPBrightnessLevel;                     //Gop Brightness level
  UINT8         IGDBootType;
  UINT8         IGDBootTypeSecondary;
  UINT8         PrimaryVideoAdaptor;
  UINT8         GraphicsPerfAnalyzers;
  UINT8         PavpEnable;
  UINT8         IgdApertureSize;
//[-start-160104-IB03090424-remove]//
//  UINT8         LidStatus;
//[-end-160104-IB03090424-remove]//
  UINT8         IgdLcdIBia;
  UINT8         AlsEnable;
  UINT8         IgdFlatPanel;
  UINT8         BootDisplayDevice;
  UINT8         IgdLcdIGmchBlc;
  UINT8         GraphicsDriverMemorySize;
  UINT8         IgdTurboEnabled;
  UINT8         VbtSelect;
  
  //
  // Gt Config
  //
  UINT8         PmSupport;
  UINT8         EnableRenderStandby;
  UINT8         MaxInverterPWM;
  UINT8         MinInverterPWM;
  UINT8         PrimaryDisplay;
  UINT8         PanelScaling;

  UINT8         CdClock;

//[-start-151211-IB03090424-modify]//
  //
  //Image Processing Unit PCI Device Configuration
  //
  UINT8         IpuEn;
  UINT8         IpuAcpiMode;
//[-end-151211-IB03090424-modify]//

  //
  // South Cluster Area - START
  //
//[-start-160216-IB03090424-add]//
  //
  // Security options
  //
  UINT8         ScBiosLock;
//[-end-160216-IB03090424-add]//
  //
  // Miscellaneous options
  //
  UINT8         Hpet;
//[-start-151222-IB03090424-add]//
  UINT8         Cg8254;
//[-end-151222-IB03090424-add]//
  UINT8         EnableClockSpreadSpec;
  UINT8         StateAfterG3;
  UINT8         UartInterface;
//[-start-160413-IB03090426-add]//
  UINT8         Wol;
//[-end-160413-IB03090426-add]//
  UINT8         AcpiWifiSel;
  //
  // SCC Configuration
  //
  UINT8         SccSdioEnabled;
  UINT8         SecureErase;
  UINT8         SccSdcardEnabled;
  UINT8         ScceMMCEnabled;
  UINT8         ScceMMCHostMaxSpeed;
  UINT8         SccUfsEnabled;
  UINT8         GPPLock;
  UINT8         GppLockOptionHide;
  //
  // LPSS Configuration
  //
  //
  UINT8         LpssI2C0Enabled;
  UINT8         LpssI2C1Enabled;
  UINT8         LpssI2C2Enabled;
  UINT8         LpssI2C3Enabled;
  UINT8         LpssI2C4Enabled;
  UINT8         LpssI2C5Enabled;
  UINT8         LpssI2C6Enabled;
  UINT8         LpssI2C7Enabled;

  UINT8         LpssHsuart0Enabled;
  UINT8         LpssHsuart1Enabled;
  UINT8         LpssHsuart2Enabled;
  UINT8         LpssHsuart3Enabled;

  UINT8         LpssSpi0Enabled;
  UINT8         LpssSpi1Enabled;
  UINT8         LpssSpi2Enabled;

//[-start-160311-IB03090425-remove]//
//  UINT8         LpssPwmEnabled;
//[-end-160311-IB03090425-remove]//

  UINT8         LpssNFCSelect;
//[-start-151211-IB03090424-remove]//
//  UINT8         I2CTouchAd;
//[-end-151211-IB03090424-remove]//

  UINT8         LpssI2cClkGateCfg[LPSS_I2C_DEVICE_NUM];
  UINT8         LpssHsuartClkGateCfg[LPSS_HSUART_DEVICE_NUM];
  UINT8         LpssSpiClkGateCfg[LPSS_SPI_DEVICE_NUM];
  UINT8         S0ixEnable;

  //
  // Usb Config
  //
  UINT8         UsbXhciSupport;
  UINT8         ScUsb30Mode;
  UINT8         ScUsb30Streams;
  UINT8         ScUsb20;
  UINT8         ScUsbPerPortCtl;
  UINT8         ScUsbPort[8];
  UINT8         ScUsbOtg;
  UINT8         ScUsbVbusOn;       //OTG VBUS control
//[-start-160119-IB03090424-add]//
  UINT8         DisableComplianceMode;
//[-end-160119-IB03090424-add]//

  //Hsic and SSic Config
  UINT8         Hsic1Support;
  UINT8         Ssic1Support;
  UINT8         Ssic2Support;
  UINT8         Ssic1Rate;
  UINT8         Ssic2Rate;
  UINT8         SsicDlanePg;
  UINT8         XdciSupport;

  //
  // Lan Config
  //
  UINT8         Lan;
  UINT8         SlpLanLowDc;

  //
  // HD-Audio Config
  //
  UINT8         ScHdAudio;
  UINT8         ScHdAudioDsp;
  UINT8         ScHdAudioCtlPwrGate;
  UINT8         ScHdAudioDspPwrGate;
  UINT8         ScHdAudioMmt;
  UINT8         ScHdAudioHmt;
  UINT8         ScHdAudioIoBufferOwnership;
  UINT8         ScHdAudioBiosCfgLockDown;
  UINT8         ScHdAudioPwrGate;
  UINT8         ScHdAudioClkGate;
  UINT8         ScHdAudioPme;
  UINT8         ScHdAudioFeature[HDAUDIO_FEATURES];
  UINT8         ScHdAudioPostProcessingMod[HDAUDIO_PP_MODULES];
//[-start-151211-IB03090424-add]//
//  CHAR16        ScHdAudioPostProcessingModCustomGuid1[GUID_CHARS_NUMBER];
//  CHAR16        ScHdAudioPostProcessingModCustomGuid2[GUID_CHARS_NUMBER];
//  CHAR16        ScHdAudioPostProcessingModCustomGuid3[GUID_CHARS_NUMBER];
//[-end-151211-IB03090424-add]//
  UINT8         ScHdAudioNhltEndpointDmic;
  UINT8         ScHdAudioNhltEndpointBt;
  UINT8         ScHdAudioNhltEndpointI2sSKP;
  UINT8         ScHdAudioNhltEndpointI2sHP;
//[-start-151228-IB03090424-add]//
  UINT8         TestHdaLowFreqLinkClockSource;
  UINT8         SvHdaVcType;
//[-end-151228-IB03090424-add]//
  UINT8         HdAudioLinkFrequency;
  UINT8         IDispLinkFrequency;
  UINT8         IDispLinkTmode;



  UINT8         AzaliaDs;
  UINT8         HdmiCodec;
  UINT8         AzaliaVCiEnable;
  UINT8         AzaliaPme;

  //
  // GMM Config
  //
  UINT8         Gmm;
  UINT8         GmmCgPGCBEnabled;
  UINT8         GmmCgSBDEnabled;
  UINT8         GmmCgSBTEnabled;
  UINT8         GmmCgSBPEnabled;
  UINT8         GmmCgCoreEnabled;
  UINT8         GmmCgDmaEnabled;
  UINT8         GmmCgRAEnabled;
  UINT8         GmmCgHostEnabled;
  UINT8         GmmCgPEnabled;
  UINT8         GmmCgTEnabled;
  UINT8         GmmPgHwAutoEnabled;
  UINT8         GmmPgD3HotEnabled;
  UINT8         GmmPgI3Enabled;
  UINT8         GmmPgPMCREnabled;

  //
  // ISH Config
  //
  UINT8         ScIshEnabled;
  UINT8         IshI2c0PullUp;
  UINT8         IshI2c1PullUp;

  //
  // SATA_CONFIG
  //
  UINT8         Sata;
  UINT8         SataTestMode;
  UINT8         SataInterfaceMode;
  UINT8         SataPort[SC_MAX_SATA_PORTS];
  UINT8         SataHotPlug[SC_MAX_SATA_PORTS];
  UINT8         SataMechanicalSw[SC_MAX_SATA_PORTS];
  UINT8         SataSpinUp[SC_MAX_SATA_PORTS];
  UINT8         SataExternal[SC_MAX_SATA_PORTS];
  UINT8         SataType[SC_MAX_SATA_PORTS];
  UINT8         SataSalp;
  UINT8         PxDevSlp[SC_MAX_SATA_PORTS];
  UINT8         EnableDitoConfig[SC_MAX_SATA_PORTS];
  UINT16        DitoVal[SC_MAX_SATA_PORTS];
  UINT8         DmVal[SC_MAX_SATA_PORTS];

  //
  // PCI_EXPRESS_CONFIG, 6 ROOT PORTS
  //
  UINT8         PcieClockGatingDisabled;
  UINT8         PcieRootPort8xhDecode;
  UINT8         Pcie8xhDecodePortIndex;
  UINT8         PcieRootPortPeerMemoryWriteEnable;
//[-start-160308-IB03090425-add]//
  UINT8         PcieComplianceMode;
//[-end-160308-IB03090425-add]//
  UINT8         PcieRootPortEn[SC_MAX_PCIE_ROOT_PORTS];
  UINT8         PcieRootPortAspm[SC_MAX_PCIE_ROOT_PORTS];
  UINT8         PcieRootPortURE[SC_MAX_PCIE_ROOT_PORTS];
  UINT8         PcieRootPortFEE[SC_MAX_PCIE_ROOT_PORTS];
  UINT8         PcieRootPortNFE[SC_MAX_PCIE_ROOT_PORTS];
  UINT8         PcieRootPortCEE[SC_MAX_PCIE_ROOT_PORTS];
  UINT8         PcieRootPortCTD[SC_MAX_PCIE_ROOT_PORTS];
  UINT8         PcieRootPortPIE[SC_MAX_PCIE_ROOT_PORTS];
  UINT8         PcieRootPortSFE[SC_MAX_PCIE_ROOT_PORTS];
  UINT8         PcieRootPortSNE[SC_MAX_PCIE_ROOT_PORTS];
  UINT8         PcieRootPortSCE[SC_MAX_PCIE_ROOT_PORTS];
  UINT8         PcieRootPortPMCE[SC_MAX_PCIE_ROOT_PORTS];
  UINT8         PcieRootPortHPE[SC_MAX_PCIE_ROOT_PORTS];
  UINT8         PcieRootPortSpeed[SC_MAX_PCIE_ROOT_PORTS];
  UINT8         PcieRootPortTHS[SC_MAX_PCIE_ROOT_PORTS];
  UINT8         PcieRootPortL1SubStates[SC_MAX_PCIE_ROOT_PORTS];
  UINT8         PcieRootPortACS[SC_MAX_PCIE_ROOT_PORTS];
  //
  // PCIe LTR Configuration
  //
  UINT8         PchPcieLtrEnable[SC_MAX_PCIE_ROOT_PORTS];
  UINT8         PchPcieLtrConfigLock[SC_MAX_PCIE_ROOT_PORTS];
//[-start-160119-IB03090424-add]//
  UINT16        PcieLtrMaxSnoopLatency[SC_MAX_PCIE_ROOT_PORTS];
  UINT16        PcieLtrMaxNoSnoopLatency[SC_MAX_PCIE_ROOT_PORTS];
//[-end-160119-IB03090424-add]//
  UINT8         PchPcieSnoopLatencyOverrideMode[SC_MAX_PCIE_ROOT_PORTS];
  UINT8         PchPcieSnoopLatencyOverrideMultiplier[SC_MAX_PCIE_ROOT_PORTS];
  UINT8         PchPcieNonSnoopLatencyOverrideMode[SC_MAX_PCIE_ROOT_PORTS];
  UINT8         PchPcieNonSnoopLatencyOverrideMultiplier[SC_MAX_PCIE_ROOT_PORTS];
  UINT16        PchPcieSnoopLatencyOverrideValue[SC_MAX_PCIE_ROOT_PORTS];
  UINT16        PchPcieNonSnoopLatencyOverrideValue[SC_MAX_PCIE_ROOT_PORTS];

  //
  // PCI Bridge Resources
  //
  UINT8         PcieExtraBusRsvd[SC_MAX_PCIE_ROOT_PORTS];
  UINT16        PcieMemRsvd[SC_MAX_PCIE_ROOT_PORTS];
  UINT8         PcieIoRsvd[SC_MAX_PCIE_ROOT_PORTS];

  //
  // South Cluster Area - END
  //

  UINT8         GTTSize;
  UINT8         PeiGraphicsPeimInit;

  //
  // DVMT5.0 Graphic memory setting
  //
  UINT8         IgdDvmt50PreAlloc;
  UINT8         IgdDvmt50TotalAlloc;
//
//  DPTF
//
  UINT8         EnableDptf;                              // Option to enable/disable DPTF
//[-start-160308-IB03090425-add]//
  UINT16        EnableDCFG;  
//[-end-160308-IB03090425-add]//
  UINT8         DptfProcessor;                           // Enable processor participant
  UINT16        DptfProcCriticalTemperature;             // Processor critical temperature
  UINT16        DptfProcPassiveTemperature;              // Processor passive temperature
  UINT16        DptfProcActiveTemperature;               // Processor active temperature
  UINT16        DptfProcCriticalTemperatureS3;           // Processor warm temperature  ]
  UINT16        DptfProcHotThermalTripPoint;             // Processor hot temperature  
  UINT16        GenericCriticalTemp0;                    // Critical temperature value for generic sensor0 participant
  UINT16        GenericPassiveTemp0;                     // Passive temperature value for generic sensor0 participant
  UINT16        GenericCriticalTemp1;                    // Critical temperature value for generic sensor1 participant
  UINT16        GenericPassiveTemp1;                     // Passive temperature value for generic sensor1 participant
  UINT16        GenericCriticalTemp2;                    // Critical temperature value for generic sensor2 participant
  UINT16        GenericPassiveTemp2;                     // Passive temperature value for generic sensor2 participant
  UINT16        GenericCriticalTemp3;                    // Critical temperature value for generic sensor3 participant
  UINT16        GenericPassiveTemp3;                     // Passive temperature value for generic sensor3 participant
  UINT16        GenericCriticalTemp4;                    // Critical temperature value for generic sensor3 participant
  UINT16        GenericPassiveTemp4;                     // Passive temperature value for generic sensor3 participant
  UINT8         ThermalSamplingPeriodTCPU;               //   
  UINT8         Clpm;                                    // Current low power mode
//[-start-160413-IB03090426-remove]//
//  UINT8         SuperDebug;                              // DPTF Super debug option
//[-end-160413-IB03090426-remove]//
  UINT32        LPOEnable;                               // DPTF: Instructs the policy to use Active Cores if they are available. If this option is set to 0, then policy does not use any active core controls ?even if they are available
  UINT32        LPOStartPState;                          // DPTF: Instructs the policy when to initiate Active Core control if enabled. Returns P state index.
  UINT32        LPOStepSize;                             // DPTF: Instructs the policy to take away logical processors in the specified percentage steps
  UINT32        LPOPowerControlSetting;                  // DPTF: Instructs the policy whether to use Core offliing or SMT offlining if Active core control is enabled to be used in P0 or when power control is applied. 1 ?SMT Off lining 2- Core Off lining
  UINT32        LPOPerformanceControlSetting;            // DPTF: Instructs the policy whether to use Core offliing or SMT offlining if Active core control is enabled to be used in P1 or when performance control is applied.1 ?SMT Off lining 2- Core Off lining
  UINT8         EnableDppm;                              // DPTF: Controls DPPM Policies (enabled/disabled)
 
//[-start-160317-IB03090425-modify]//
  UINT8         EnableSen0Participant;
  UINT8         PassiveThermalTripPointSen0;
  UINT8         CriticalThermalTripPointSen0;
  UINT8         CriticalThermalTripPointSen0S3;
  UINT8         HotThermalTripPointSen0;
 
  UINT8         EnableSen3Participant;
  UINT8         PassiveThermalTripPointSen3;
  UINT8         CriticalThermalTripPointSen3;
  UINT8         CriticalThermalTripPointSen3S3;
  UINT8         HotThermalTripPointSen3;

  UINT8         DptfChargerDevice;
  UINT8         DptfDisplayDevice;
//[-start-160413-IB03090426-add]//
  UINT8         DisplayHighLimit;
  UINT8         DisplayLowLimit;
//[-end-160413-IB03090426-add]//
  UINT8         EnablePowerParticipant;
  UINT16        PowerParticipantPollingRate;
  UINT8         DptfFanDevice;
  UINT8         DptfWwanDevice;
  UINT8         PassiveThermalTripPointWWAN;
  UINT8         CriticalThermalTripPointWWANS3;
  UINT8         HotThermalTripPointWWAN;
  UINT8         CriticalThermalTripPointWWAN;
//[-end-160317-IB03090425-modify]//

  UINT8         EnableActivePolicy;
  UINT8         EnablePassivePolicy;
  UINT8         TrtRevision;  
  UINT8         EnableCriticalPolicy;
  UINT8         EnableAPPolicy;
  UINT8         EnablePowerBossPolicy;
//[-start-151211-IB03090424-add]//
  UINT8         EnableVSPolicy;
//[-end-151211-IB03090424-add]//

  UINT8         EnableMemoryDevice;
  UINT8         ActiveThermalTripPointTMEM;
  UINT8         PassiveThermalTripPointTMEM;
  UINT8         CriticalThermalTripPointTMEM;
  UINT8         ThermalSamplingPeriodTMEM;

  UINT8         EnableSen1Participant;
  UINT8         ActiveThermalTripPointSen1;
  UINT8         PassiveThermalTripPointSen1;
  UINT8         CriticalThermalTripPointSen1;  
  UINT8         CriticalThermalTripPointSen1S3;
  UINT8         HotThermalTripPointSen1;  
  UINT8         SensorSamplingPeriodSen1;

  UINT8         EnableGen1Participant;
  UINT8         ActiveThermalTripPointGen1;
  UINT8         PassiveThermalTripPointGen1;
  UINT8         CriticalThermalTripPointGen1;
  UINT8         CriticalThermalTripPointGen1S3;
  UINT8         HotThermalTripPointGen1;
  UINT8         ThermistorSamplingPeriodGen1;

  UINT8         EnableGen2Participant;
  UINT8         ActiveThermalTripPointGen2;
  UINT8         PassiveThermalTripPointGen2;
  UINT8         CriticalThermalTripPointGen2;
  UINT8         CriticalThermalTripPointGen2S3;
  UINT8         HotThermalTripPointGen2;
  UINT8         ThermistorSamplingPeriodGen2;
  
  UINT8         EnableGen3Participant;
  UINT8         ActiveThermalTripPointGen3;
  UINT8         PassiveThermalTripPointGen3;
  UINT8         CriticalThermalTripPointGen3;
  UINT8         CriticalThermalTripPointGen3S3;
  UINT8         HotThermalTripPointGen3;
  UINT8         ThermistorSamplingPeriodGen3;  
  
  UINT8         EnableGen4Participant;
  UINT8         ActiveThermalTripPointGen4;
  UINT8         PassiveThermalTripPointGen4;
  UINT8         CriticalThermalTripPointGen4;
  UINT8         CriticalThermalTripPointGen4S3;
  UINT8         HotThermalTripPointGen4;
  UINT8         ThermistorSamplingPeriodGen4;

//[-start-151211-IB03090424-add]//
  UINT8         OemDesignVariable0;
  UINT8         OemDesignVariable1;
  UINT8         OemDesignVariable2;
  UINT8         OemDesignVariable3;
  UINT8         OemDesignVariable4;
  UINT8         OemDesignVariable5;

  UINT8         EnableVS1Participant;
  UINT8         ActiveThermalTripPointVS1;
  UINT8         PassiveThermalTripPointVS1;
  UINT8         CriticalThermalTripPointVS1;
  UINT8         CriticalThermalTripPointVS1S3;
  UINT8         HotThermalTripPointVS1;

  UINT8         EnableVS2Participant;
  UINT8         ActiveThermalTripPointVS2;
  UINT8         PassiveThermalTripPointVS2;
  UINT8         CriticalThermalTripPointVS2;
  UINT8         CriticalThermalTripPointVS2S3;
  UINT8         HotThermalTripPointVS2;

  UINT8         EnableVS3Participant;
  UINT8         ActiveThermalTripPointVS3;
  UINT8         PassiveThermalTripPointVS3;
  UINT8         CriticalThermalTripPointVS3;
  UINT8         CriticalThermalTripPointVS3S3;
  UINT8         HotThermalTripPointVS3;
//[-end-151211-IB03090424-add]//
    
  //
  // Security
  //
  UINT8         SeCOpEnable;
  UINT8         SeCModeEnable;
  UINT8         SeCEOPEnable;
  UINT8         SeCEOPDone;
//[-start-160308-IB03090425-add]//
  UINT8         SeCLockDir;
//[-end-160308-IB03090425-add]//
//  UINT8         Recovery;
//  UINT8         Suspend;
  UINT8         Tdt;                  // AT
  UINT8         TdtState;             // AT
  UINT8         MeasuredBootEnable;
  UINT8         EnableAESNI;          // AES Enable/Disable

  //
  // Temperature
  //
  //
//[-start-160308-IB03090425-add]//
  UINT8         AutoThermalReporting;
//[-end-160308-IB03090425-add]//
  UINT8         EnableDigitalThermalSensor;
  UINT8         PassiveThermalTripPoint;
  UINT8         PassiveTc1Value;
  UINT8         PassiveTc2Value;
  UINT8         CriticalThermalTripPoint;
//[-start-160509-IB03090427-add]//
  UINT8         ActiveTripPoint;
//[-end-160509-IB03090427-add]//
  UINT8         PassiveTspValue;


  UINT8         BidirectionalProchotEnable;
  UINT8         ThermalMonitoring;
  UINT8         ThermalMonitoringHot;
  UINT8         ThermalMonitoringSystherm0Hot;
  UINT8         ThermalMonitoringSystherm1Hot;
  UINT8         ThermalMonitoringSystherm2Hot;
  UINT8         DptfAllowHigherPerformance;    // DPTF: Allow higher performance on AC/USB - (Enable/Disable)
  UINT8         AmbientTripPointChange;
  UINT8         AmbientConstantSign[6];        // DPTF:Sign of Amb. Const
  UINT8         AmbientConstants[6];           // DPTF: #IMP# Takes 6 bytes



  UINT8         BomSelection;
  UINT8         FeatureSelection;
//[-start-160311-IB03090425-add]//
  UINT8         WorldCameraSel;
  UINT8         UserCameraSel;
//[-end-160311-IB03090425-add]//

  //
  // Memory Config Tools
  //
  UINT8         MemoryMode;
  UINT16        MemorySpeed;
  UINT8         MemoryTcl;
  UINT8         MemoryTrcd;
  UINT8         MemoryTrp;
  UINT8         MemoryTras;
  UINT8         MemoryTrc;
  UINT16        ChannelHashMask;
  UINT16        SliceHashMask;
  UINT8         InterleavedMode;
  UINT8         ChannelsSlicesEnabled;
  UINT8         MinRefRate2xEnabled;
  UINT8         DualRankSupportEnabled;
  UINT8         DualRankSupportEnableLockHide;
  
//[-start-160308-IB03090425-add]//
  UINT8         DynamicSrDisable;
//[-end-160308-IB03090425-add]//
  //
  // Third party Devices
  // Offset
  //

  UINT8         BTHStatus;
  UINT8         RvpCameraDevSel;
  UINT8         BatteryChargingSolution;                 //0-non ULPMC 1-ULPMC
//[-start-170314-IB07400847-modify]//
  UINT8         S5LongRunTest_Reserved;
//[-end-170314-IB07400847-modify]//
  UINT8         PmicEnable;
//[-start-151222-IB03090424-add]//
  UINT8         APEIBERT;
//[-end-151222-IB03090424-add]//

  UINT8         PssEnabled;
#if (TABLET_PF_ENABLE == 1)
  UINT8         EnterDnxFastBoot;
#else
  UINT8         Reserved7[1];
#endif

  //
  // Misc
  //
  //
  UINT8         AcpiSuspendState;
  UINT8         S0ix;
  //
  // Pci
  //
  //
  UINT8         PciLatency;
  UINT8         Asf;

  //
  // Android
  //
  UINT8         OsSelection;
//[-start-160216-IB03090424-add]//
  UINT8         PreOsSelection;
//[-end-160216-IB03090424-add]//
  UINT8         DroidBoot;
  UINT8         AndroidBoot;
  //
  //  SG
  //
  UINT8         EnSGFunction;
  //
  //
  UINT8         IspAcpiMode;

  UINT8         GpsModeSel;
  UINT8         FirmwareConfiguration;
  //
  // Enabling VTD
  //
  UINT8         VTdEnable;
  UINT8         ScramblerSupport;

  UINT8         ScceMMCHs400Enabled;   //

//
//  CpuPower.vfi
//
  UINT8         MaxPkgCState;
  UINT8         MaxCoreCState;
  UINT8         CStatePreWake;
  UINT8         CStateAutoDemotion;
  UINT8         CStateUnDemotion;
  UINT8         PkgCStateDemotion;
  UINT8         PkgCStateUnDemotion;
//[-start-160105-IB03090424-remove]//
//  UINT8         ExecuteDisableBit;
//[-end-160105-IB03090424-remove]//
  UINT8         Pram;

  // Kernel Debugger (WinDBG)
  UINT8         OsDbgEnable;
//[-start-160413-IB03090426-modify]//
  UINT8         DciEn;
  UINT8         DciAutoDetect;
//[-end-160413-IB03090426-modify]//

  //Dnx/Fastboot enable for PO
  //
  // Power State
  //
  UINT8         PowerState;
  UINT8         PowerBtn500msEn;

  UINT8         PciDelayOptimizationEcr;
  //
  // EC
  //
  UINT8         LowPowerS0Idle;
  UINT8         TenSecondPowerButtonEnable;   // NULL
  UINT8         EcLowPowerMode;
  UINT8         CSNotifyEC;
  UINT8         CSDebugLightEC;               // NULL

  //
  // PEP
  //
  UINT8         PepCpu;
  UINT8         PepGfx;
  UINT8         PepSataContraints;
  UINT8         PepUart;
  UINT8         PepSdio;
  UINT8         PepI2c0;
  UINT8         PepI2c1;
  UINT8         PepI2c2;
  UINT8         PepI2c3;
  UINT8         PepI2c4;
  UINT8         PepI2c5;
  UINT8         PepI2c6;
  UINT8         PepI2c7;
  UINT8         PepI2c8;
  UINT8         PepPwm1;
  UINT8         PepHsuart1;
  UINT8         PepHsuart2;
  UINT8         PepHsuart3;
  UINT8         PepHsuart4;
  UINT8         PepSpi1;
  UINT8         PepSpi2;
  UINT8         PepSpi3;
  UINT8         PepXhci;
  UINT8         PepAudio;
  UINT8         PepEmmc;
  UINT8         PepSdxc;
  //
  // RTD3
  //
  UINT8         Rtd3Support;
  UINT8         Rtd3P0dl;
  UINT8         Rtd3P3dl;
  UINT8         RTD3UsbPt1;
  UINT8         RTD3UsbPt2;
  UINT8         RTD3I2C0SensorHub;
  UINT16        RTD3AudioDelay;
  UINT16        RTD3TouchPadDelay;
  UINT16        RTD3TouchPanelDelay;
  UINT16        RTD3SensorHub;
  UINT16        VRStaggeringDelay;
  UINT16        VRRampUpDelay;
  UINT8         PstateCapping;
  UINT8         RTD3ZPODD;
  UINT8         RTD3Camera;
  UINT8         RTD3SataPort1;
  UINT8         RTD3SataPort2;
  UINT8         RTD3SataPort3;
  UINT8         Rtd3WaGpio;
  UINT16        RTD3I2C0ControllerPS0Delay;
  UINT16        RTD3I2C1ControllerPS0Delay;
  UINT8         ConsolidatedPR;

  //
  // Secure NFC
  //
//[-start-160308-IB03090425-add]//
  UINT8         SecureNfc;
  UINT8         SecureNfcAvailable;
//[-end-160308-IB03090425-add]//
//[-start-151211-IB03090424-modify]//
  UINT8         NfcSelect;
//[-end-151211-IB03090424-modify]//
  UINT8         SelectBtDevice;
  UINT8         ResetSelect;
//[-start-160216-IB03090424-add]//
  UINT8         FprrEnable;
//[-end-160216-IB03090424-add]//

  //
  // TPM
  //
  UINT8         ETpm;
  UINT8         SecEnable;
//[-start-151127-IB02950555-modify]//
//[-start-160421-IB08450341-modify]//
  UINT8         Reserved_Original_TPM;
//[-end-160421-IB08450341-modify]//
//[-end-151127-IB02950555-modify]//
//[-start-151211-IB03090424-add]//
  UINT8         TPMSupportedBanks;
  UINT8         TpmDetection;
  UINT8         PttSuppressCommandSend;  // For PTT Debug
//[-end-151211-IB03090424-add]//
  UINT8         SecFlashUpdate;
  UINT8         SecFirmwareUpdate;

  //
  // NPK debug Options
  //
  UINT8         NpkEn;
  UINT8         FwTraceEn;
  UINT8         FwTraceDestination;
  UINT8         RecoverDump;
//[-start-160308-IB03090425-modify]//
  UINT32        Msc0Size;
//[-end-160308-IB03090425-modify]//
  UINT8         Msc0Wrap;
//[-start-160308-IB03090425-modify]//
  UINT32        Msc1Size;
//[-end-160308-IB03090425-modify]//
  UINT8         Msc1Wrap;
  UINT8         PtiMode;
  UINT8         PtiTraining;
  UINT8         PtiSpeed;
  UINT8         PunitMlvl;
  UINT8         PmcMlvl;
  UINT8         SwTraceEn;

  UINT8         ProcTraceMemSize;
  UINT8         ProcTraceEnable;
  UINT8         ProcTraceOutputScheme;

  UINT8         PnpSettings;
  UINT8         EhciDebug;
  UINT8         IrmtConfiguration;
  UINT8         CRIDSettings;
  UINT8         OSDnX;
  UINT8         TDO;
  UINT8         ACPIMemDbg;          // [TODO]

  UINT8         IsctConfiguration;
  UINT8         IsctNotificationControl;
  UINT8         IsctWlanPowerControl;
  UINT8         IsctWwanPowerControl;
  UINT8         IsctSleepDurationFormat;
  UINT8         IsctRFKillSupport;
  UINT8         WlanNGFFCardPresence;
  UINT8         WlanUHPAMCardPresence;
  UINT8         DisableActiveTripPoints;
//[-start-161215-IB08450359-modify]//
  UINT8         Reserved_Original_H2OUVESupport;
//[-end-161215-IB08450359-modify]//
  UINT8         ENDBG2;
  UINT8         TargetTPM;
  UINT8         ModemSel;
  //
  // WIGIG enable switch
  //
  UINT8         WiGigEnable;
//[-start-151211-IB03090424-add]//
  UINT16        WiGigSPLCPwrLimit;
  UINT32        WiGigSPLCTimeWindow;
  UINT16        RfemSPLCPwrLimit;
  UINT32        RfemSPLCTimeWindow;

  //PSM
  UINT8         PsmEnable;
  UINT8         PsmSPLC0DomainType;
  UINT16        PsmSPLC0PwrLimit;
  UINT32        PsmSPLC0TimeWindow;
  UINT8         PsmSPLC1DomainType;
  UINT16        PsmSPLC1PwrLimit;
  UINT32        PsmSPLC1TimeWindow;

  UINT8         PsmDPLC0DomainType;
  UINT8         PsmDPLC0DomainPerference;
  UINT8         PsmDPLC0PowerLimitIndex;
  UINT16        PsmDPLC0PwrLimit;
  UINT32        PsmDPLC0TimeWindow;

  UINT8         PsmDPLC1DomainType;
  UINT8         PsmDPLC1DomainPerference;
  UINT8         PsmDPLC1PowerLimitIndex;
  UINT16        PsmDPLC1PwrLimit;
  UINT32        PsmDPLC1TimeWindow;  

  UINT8         EPIEnable;
  UINT8         TypeCEnable;

  //
  //DDR and High Speed Serial IO SSC
  //
  UINT8         DDRSSCEnable;
  UINT8         DDRSSCSelection;
  UINT8         DDRCLKBending;
  UINT8         HSSIOSSCEnable;
  UINT8         HSSIOSSCSelection;
//[-end-151211-IB03090424-add]//

  
  UINT8         PciExpNative;
  UINT8         NativeAspmEnable;
//[-start-160104-IB03090424-remove]//
//  //
//  // SGX Feature
//  //
//  UINT16        SgxMode; //_BXT_SGX_PORT_
//  UINT64        SgxEpoch0;    
//  UINT64        SgxEpoch1;    
//  UINT8         SgxMicrocodePrompt;
//[-end-160104-IB03090424-remove]//
  //
  // Hybrid Graphics
  //
  UINT8         HgCardSelect;
  UINT16        DelayAfterPwrEn;
  UINT16        DelayAfterHoldReset;
//[-start-151204-IB02950555-add]/ 
  //
  // LPSS devices
  //
  UINT8         I2s343A;
  UINT8         I2s34C1;
  UINT8         I2cNfc;
  UINT8         I2cPss;
  UINT8         UartBt;
  UINT8         UartGps;
//[-start-151211-IB03090424-add]//
  UINT8         Spi1SensorDevice;
  UINT32        Uart2KernelDebugBaseAddress;
  //
  // SMRR for SMM source level debug
  //
  UINT8         SMRREnable;
//[-end-151211-IB03090424-add]//
  UINT8         TrunkClockEnable;
//[-end-151204-IB02950555-add]/
//[-start-151222-IB03090424-add]//
  UINT8         CrashDump;
  //
  // S3 Setupvairable
  //
  UINT64        AcpiVariableSetCompatibility;
  UINT32        BootPerformanceTablePointer;
  UINT32        S3PerformanceTablePointer;
  UINT8         CseBootDevice;                  // 0 - eMMC, 1- UFS, 2 - SPI
//[-end-151222-IB03090424-add]//
//[-start-160824-IB07220130-add]//
  UINT32        NvStorageVariableBase;
//[-end-160824-IB07220130-add]//

//[-start-160413-IB03090426-add]//
  //
  // IPC1 (PMI) device
  //
  UINT8         Ipc1Enable;
  UINT8         RtcLock;
//[-end-160413-IB03090426-add]//
//[-start-160509-IB03090427-add]//
  UINT8         UseProductKey;
  UINT8         PowerButterDebounceMode;
  UINT8         PcieRootPortSelectableDeemphasis[SC_MAX_PCIE_ROOT_PORTS];
  UINT8         ConInBehavior;
  //
  // Number Of Processors
  //
  UINT8         NumOfProcessors;
  
  UINT8         GpioLock;
//[-end-160509-IB03090427-add]//
//[-start-160718-IB06720413-add]//
  UINT8         I2cTouchPanel;
  UINT8         I2cTouchPad;
  UINT8         CameraRotationAngle;

//[-start-160719-IB03090429-add]//
  //
  // I2C Speed
  //
  UINT8         I2C0Speed;
  UINT8         I2C1Speed;
  UINT8         I2C2Speed;
  UINT8         I2C3Speed;
  UINT8         I2C4Speed;
  UINT8         I2C5Speed;
  UINT8         I2C6Speed;
  UINT8         I2C7Speed;

  UINT8         TcoLock;
  UINT8         RpmbDataClear;
//[-end-160719-IB03090429-add]//
  
//[-start-160826-IB03090433-add]//
  //
  // PTT Status provided by CSE
  //
  UINT8         CsePttStatus;
//[-end-160826-IB03090433-add]//
  
//[-start-161018-IB06740518-add]//
  //
  // Intel Ultrabook feature
  //
  UINT8         ConvertibleEnable;
  UINT8         DockEnable;
  UINT8         SlateIndicator;
  UINT8         DockIndicator;
  UINT8         SlateIndicatorSx;
  UINT8         DockIndicatorSx;
  //
  // HdAudio Dsp Uaa Compliance
  //
  UINT8         HdAudioDspUaaCompliance;
  //
  // Data Clear Policy during Capsule Update
  //
  UINT8         CapsuleDataClear;
//[-end-161018-IB06740518-add]//
  UINT8         Reserved[26];
//[-end-160718-IB06720413-add]//

//Chipset_End
//----------------------------------------------------------------------------------------------------------
// End of area for Chipset team use.
//----------------------------------------------------------------------------------------------------------
//[-start-160217-IB07400702-add]//
//[-start-160429-IB07400722-modify]//
  UINT8         HideUnusedLpssDevices;
//[-start-160506-IB07400723-add]//
//[-start-160604-IB07400740-modify]//
  UINT8         Max2G;
//[-end-160604-IB07400740-modify]//
//[-start-160604-IB07400740-add]//
//[-start-160816-IB07400771-modify]//
  UINT8         AfeComp;
//[-end-160816-IB07400771-modify]//
  UINT8         PowerLimit1Enable;
  UINT8         PowerLimit1Clamp;
  UINT16        PowerLimit1Time;
  UINT16        PowerLimit1;
//[-end-160604-IB07400740-add]//
//[-start-160623-IB07400747-add]//
//[-start-160803-IB07400768-modify]//
  UINT8         Chipset2Reserved00;
//[-end-160803-IB07400768-modify]//
  UINT8         RTEn;                //Real Time
//[-end-160623-IB07400747-add]//
//[-end-160506-IB07400723-add]//
//[-start-160725-IB07400761-add]//
  UINT8         LogoScuResolution;
  UINT8         LpcClockRun;
  UINT8         VbtLfpType;
  UINT8         VbtLfpPort;
  UINT8         VbtLfpAux;
  UINT8         VbtLfpPanel;
  UINT8         VbtLfpEdid;
  UINT8         VbtEfpType[3];
  UINT8         VbtEfpPort[3];
  UINT8         VbtEfpDdc[3];
  UINT8         VbtEfpAux[3];
  UINT8         VbtEfpEdid[3];
//[-end-160725-IB07400761-add]//
//[-start-160809-IB07400769-add]//
  UINT8         SeCPrepareForUpdate;
//[-end-160809-IB07400769-add]//
//[-start-160811-IB07400770-add]//
  UINT8         ForceLegacyFree;
//[-end-160811-IB07400770-add]//
//[-start-160818-IB07400772-add]//
  UINT8         VbtLfpHpdInversion;
  UINT8         VbtLfpDdiLaneReversal;
  UINT8         VbtEfpHdmiLevelShifter[3];
  UINT8         VbtEfpOnboardLspcon[3];
  UINT8         VbtEfpHpdInversion[3];
  UINT8         VbtEfpDdiLaneReversal[3];
  UINT8         VbtEfpUsbTypeC[3];
  UINT8         VbtEfpDockablePort[3];
//[-end-160818-IB07400772-add]//
//[-start-160914-IB07400784-add]//
  UINT8         IsIOTGBoardIds;
  UINT8         PmicSetupDefault;
//[-end-160914-IB07400784-add]//
//[-start-160923-IB07400789-add]//
  UINT8         Win7KbMsSupport;
//[-end-160923-IB07400789-add]//
//[-start-160929-IB07400790-add]//
  UINT8         CpuTstate;
//[-end-160929-IB07400790-add]//
//[-start-161008-IB07400794-add]//
  UINT8         PlatformSettingEn;
//[-end-161008-IB07400794-add]//
//[-start-170124-IB07400837-add]//
  UINT8         RotateScreenConfig;
//[-end-170124-IB07400837-add]//
//[-start-170222-IB07400843-add]//
  UINT8         SirqEnabled;
  UINT8         SirqMode;
//[-end-170222-IB07400843-add]//
//[-start-170314-IB07400847-add]//
  UINT8         S5WakeOnRtcAfterSec;
//[-end-170314-IB07400847-add]//
//[-start-170626-IB07400880-add]//
  UINT8         S3WakeOnUsb;
  UINT8         S5WakeOnUsb;
  UINT8         S5WakeOnUsbWaitTime;
//[-end-170626-IB07400880-add]//
//[-start-170119-IB07401072-add]//
  UINT8         PciMem64DecodeSupport;
  UINT8         PciMem64SkipOnboard;
//[-end-170119-IB07401072-add]//
  UINT8         ChipsetRSV2[131];    // Reserve for Chipset2
//[-end-160429-IB07400722-modify]//
//[-end-160217-IB07400702-add]//
#endif
