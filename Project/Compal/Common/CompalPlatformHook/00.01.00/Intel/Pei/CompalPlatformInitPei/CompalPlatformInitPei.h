
#ifndef _COMPAL_PLATFORM_INIT_PEI_H_
#define _COMPAL_PLATFORM_INIT_PEI_H_

//
// The LPC related register used to locate GPIO base
//
#define  LPC_BUS                        0x00
#define  LPC_DEV                        0x1f
#define  LPC_FUN                        0x00
#define    R_ACPI_GPIO_BASE             0x48
#define    ACPI_GPIO_BASE_MASK          0xFFF8
#define  LPC_PCI_GPIOBASE               PCI_LIB_ADDRESS (LPC_BUS, LPC_DEV, LPC_FUN, R_ACPI_GPIO_BASE)

//
// --------------------------- AZALIA Config ------------------------------
//
typedef struct {
  UINT32  VendorDeviceId;                         ///< This is the Vendor ID (byte 3 and byte 2) and Device ID (byte 1 and byte 0) of the Azalia codec.
  UINT16  SubSystemId;                            ///< This is the sub system ID of this codec.
  UINT8   RevisionId;                             ///< This is the revision ID of this codec. 0xFF applies to all steppings
  UINT8   FrontPanelSupport;                      ///< Whether or not support front panel. 1: Yes, 0: No.
  UINT16  NumberOfRearJacks;                      ///< Number of rear jacks.
  UINT16  NumberOfFrontJacks;                     ///< Number of front jacks.
} PCH_AZALIA_VERB_TABLE_HEADER;

typedef struct {
  PCH_AZALIA_VERB_TABLE_HEADER  VerbTableHeader;  ///< The header information in Azalia verb table.
  UINT32                        *VerbTableData;   ///< Pointer to the buffer containing verb tables data provided by platform.
} PCH_AZALIA_VERB_TABLE;

//
// Macro Definition
//
#define PCH_DEVICE_ENABLE               1
#define PCH_DEVICE_DISABLE              0
#define DEFAULT_PCI_BUS_NUMBER_PCH      0

///////////////////////////////////  Azalia Codec   ////////////////////////////////////////

//
// Azalia Configuration
//
					
#define AZALIA_SUPPORT                  PCH_DEVICE_ENABLE    // 0: Disable; 1: Enable; 2: Auto
#define AZALIA_PME                      0                    // 0: Disable; 1: Enable
#define AZALIA_DS                       0                    // 0: Docking is not supported; 1:Docking is supported
#define AZALIA_DA                       0                    // 0: Docking is not attached; 1:Docking is attached
#define AZALIA_HDMICODEC                PCH_DEVICE_ENABLE    // 0: Disable; 1: Enable

#define AZALIA_MAX_LOOP_TIME            10
#define AZALIA_MAX_SID_NUMBER           4
#define AZALIA_MAX_SID_MASK             ( ( 1 << AZALIA_MAX_SID_NUMBER ) - 1 )
#define AZALIA_WAIT_PERIOD              100
#define AZALIA_DEFAULT_RESET_WAIT_TIMER 300

#include <IndustryStandard/Pci22.h>
#include <Library/IoLib.h>
#include <Library/PciLib.h>
#include <PchRegs/PchRegsUsb.h>
#include <PchRegs/PchRegsLpc.h>
#include <Library/PeiServicesLib.h>
#include <Library/MemoryAllocationLib.h>
#include <CompalPlatformInitPeiPpi.h>

//
// Init Gpio Table
//
EFI_STATUS
CompalPlatformInitPeiIntelGpio (
  IN    CONST EFI_PEI_SERVICES   **PeiServices,
  IN    COMPAL_GPIO_SETTINGS     *CompalGpioTable,
  IN    UINT16                   CompalGpioTableCount,
  IN    UINT16                   CompalGpioTableVersion
);

EFI_STATUS
CompalPlatformInitPeiAMDSB700Gpio (
  IN    CONST EFI_PEI_SERVICES   **PeiServices,
  IN    COMPAL_GPIO_SETTINGS     *CompalGpioTable,
  IN    UINT16                   CompalGpioTableCount,
  IN    UINT16                   CompalGpioTableVersion
);

EFI_STATUS
CompalPlatformInitPeiAMDSB800Gpio (
  IN    CONST EFI_PEI_SERVICES   **PeiServices,
  IN    COMPAL_GPIO_SETTINGS     *CompalGpioTable,
  IN    UINT16                   CompalGpioTableCount,
  IN    UINT16                   CompalGpioTableVersion
);

//
// Init Audio Verb Table
//
EFI_STATUS
CompalPlatformInitPeiIntelAudioVerbTable (
  IN    CONST EFI_PEI_SERVICES   **PeiServices,
  IN    PCH_AZALIA_VERB_TABLE    *CompalVerbTable,
  IN    UINT8                    NumberOfVerbTable
);

#endif
