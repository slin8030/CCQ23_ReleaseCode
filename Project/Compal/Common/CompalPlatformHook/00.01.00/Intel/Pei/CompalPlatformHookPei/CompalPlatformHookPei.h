
#ifndef _COMPAL_PLATFORM_HOOK_PEI_H_
#define _COMPAL_PLATFORM_HOOK_PEI_H_

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

#include <IndustryStandard/Pci22.h>
#include <Library/IoLib.h>
#include <Library/PciLib.h>
#include <PchRegs/PchRegsUsb.h>
#include <PchRegs/PchRegsLpc.h>
#include <Library/PeiServicesLib.h>
#include <Library/MemoryAllocationLib.h>
#include <CompalPlatformHookPeiPpi.h>
#include <CompalPlatformInitPeiPpi.h>

EFI_STATUS
ProjectPlatformHookPeiPlatformStage1Override (
    IN CONST EFI_PEI_SERVICES          **PeiServices
);

EFI_STATUS
ProjectPlatformHookPeiPlatformStage2Override (
    IN CONST EFI_PEI_SERVICES          **PeiServices
);

#endif
