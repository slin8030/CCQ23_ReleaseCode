
#ifndef _COMPAL_FLASH_LIB_H_
#define _COMPAL_FLASH_LIB_H_

#define bit0      (1 << 0)
#define bit1      (1 << 1)
#define bit2      (1 << 2)
#define bit3      (1 << 3)
#define bit4      (1 << 4)
#define bit5      (1 << 5)
#define bit6      (1 << 6)
#define bit7      (1 << 7)
#define bit8      (1 << 8)
#define bit9      (1 << 9)
#define bit10     (1 << 10)
#define bit11     (1 << 11)
#define bit12     (1 << 12)
#define bit13     (1 << 13)
#define bit14     (1 << 14)
#define bit15     (1 << 15)
#define bit16     (1 << 16)
#define bit17     (1 << 17)
#define bit18     (1 << 18)
#define bit19     (1 << 19)
#define bit20     (1 << 20)
#define bit21     (1 << 21)
#define bit22     (1 << 22)
#define bit23     (1 << 23)
#define bit24     (1 << 24)
#define bit25     (1 << 25)
#define bit26     (1 << 26)
#define bit27     (1 << 27)
#define bit28     (1 << 28)
#define bit29     (1 << 29)
#define bit30     (1 << 30)
#define bit31     (1 << 31)

#define PROJECT_VERSION_SIZE       20
#define PROJECT_NAME_SIZE          60
#define PROJECT_COUNT              10
#define OEM_PASS_INFO_SIZE         50
#define COMMON_FLASH_VERSION       1
#ifndef DISABLED
#define DISABLED                 0x00
#endif
#ifndef ENABLED
#define ENABLED                  0x01
#endif
#define FIELD_TAG                 0x00
#define END_TAG                   0xFF

#define PTEN                        (bit0 | bit1 | bit2 | bit3 | bit4)
#define PTDIS                      ~PTEN
#define ACEN                        bit1
#define ACDIS                      ~ACEN
#define DCEN                        bit2
#define DCDIS                      ~DCEN
#define RESSEN                      bit3
#define RESSDIS                    ~RESSEN
#define PJMDEN                      bit4
#define PJMDDIS                    ~PJMDEN
#define FHOS                        bit5
#define FHRST                       bit6
#define FHST                        bit7
#define CPVER                       bit8

//
// Protect error code
//
#define NO_ERROR                    0x00
#define AC_ERROR                    0x01
#define DC_ERROR                    0x02
#define GAS_GAUGE_ERROR             0x03
#define RESS_ERROR                  0x04
#define PJMD_ERROR                  0x05
#define CPVER_ERROR                 0x10
#define RESSSTR_ERROR               0x11
#define PJMDSTR_ERROR               0x12

//
// After Flash behavior
//
#define SYSYEM_BACK_TO_OS           0x01
#define SYSTEM_DIRECTLY_REBOOT      0x02
#define SYSTEM_DIRECTLY_SHUTDOWN    0x03

#pragma pack(1)

typedef struct {
    UINT8                                 ACPresent;
    UINT8                                 DCPresent;
    UINT8                                 BatteryGasGauge;
    CHAR8                                 OldProjectVersion[PROJECT_VERSION_SIZE] ;
    CHAR8                                 NewProjectVersion[PROJECT_VERSION_SIZE+1] ;
    CHAR8                                 OldProjectName[PROJECT_NAME_SIZE] ;
    CHAR8                                 NewProjectName[PROJECT_NAME_SIZE+1] ;
} BEFORE_FLASH_INPUT_BUFFER;

typedef struct {
    CHAR16                OEMPassInfo[OEM_PASS_INFO_SIZE];
} OEM_PASS_INFO_BEFORE_FLASH;

typedef struct {
    UINT16                 Flash;
    UINT16                 NumOfErrorMsg;
    UINT16                 SysBehavior;
} BEFORE_FLASH_OUTPUT_BUFFER;

#pragma pack()

EFI_STATUS
OemCheckBeforeFlash (
    BEFORE_FLASH_INPUT_BUFFER              *PlatformInfoPtr,
    UINT16                                  UserInputStatus,
    BEFORE_FLASH_OUTPUT_BUFFER             *PlatformOutputInfoPtr
);

UINT16
OemCheckParamterInfo (
    CHAR8                            *OemPassInfoPtr
);

VOID
GetOemSetting (
    UINTN       BatteryGasGauge,
    UINTN       BIOSRegLength
);

VOID
OemFlashAllowStart(
    VOID
);

VOID
OemFlashCompletely(
    VOID
);

EFI_STATUS
OemPassECFlash (
    BOOLEAN     OemIsEcFlash
);

UINT16
OemCheckFlashStates (
    CHAR16         *PassDateBuffer,
    UINT16          UserInputStatus
);

UINT8
Func_Detect_AC (
    BEFORE_FLASH_INPUT_BUFFER   *PlatformInfo
);

UINT8
Func_Detect_Battery (
    BEFORE_FLASH_INPUT_BUFFER   *PlatformInfo
);

UINT8
Func_Detect_BIOS_Regressive (
    BEFORE_FLASH_INPUT_BUFFER   *PlatformInfo
);

UINT8
Func_Detect_ModelString (
    BEFORE_FLASH_INPUT_BUFFER   *PlatformInfo
);

UINT16
OemGetBehaviorAfterFlash (
    UINT16   BehaviorAfterFlash
);

UINTN
Pow (
    UINTN Value,
    UINTN index
);

EFI_STATUS
HexStringToBuf (
  IN OUT UINT8                     *Buf,
  IN OUT UINTN                    *Len,
  IN     CHAR16                    *Str,
  OUT    UINTN                     *ConvertedStrLen  OPTIONAL
  );


#endif
