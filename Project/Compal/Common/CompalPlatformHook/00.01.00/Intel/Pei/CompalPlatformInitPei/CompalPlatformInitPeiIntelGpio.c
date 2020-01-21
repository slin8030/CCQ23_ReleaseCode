
#include "CompalPlatformInitPei.h"

#define R_PCH_GPIO_OWN1		0x00
#define R_PCH_GPIO_OWN2		0x04
#define R_PCH_GPIO_OWN3		0x08

#define R_PCH_GPIO_ROUT1	0x30
#define R_PCH_GPIO_ROUT2	0x34
#define R_PCH_GPIO_ROUT3	0x38

typedef struct
{
    UINT8   GpioOffset;
    UINT32  GpioValue;
} COMPAL_GPIO_DEV;

EFI_STATUS
CompalPlatformInitPeiIntelGpio (
    IN    CONST EFI_PEI_SERVICES   **PeiServices,
    IN    COMPAL_GPIO_SETTINGS     *CompalGpioTable,
    IN    UINT16                   CompalGpioTableCount,
    IN    UINT16                   CompalGpioTableVersion
)
{
    UINT16                Index;
    UINT16                TableCount;
    UINT16                BitOffset;
    UINT16                GpioOffset;
    UINT32                GpioValue;

    UINT8                 GpioIndex;
    COMPAL_GPIO_TYPE      GpioType;
    COMPAL_GPIO_LEVEL     GpioLevel;
    UINT32                GpioExtData;

    UINT32		  RegConfig0            = 0;
    UINT32		  RegConfig4            = 0;
    UINT32                BlinkSetting          = 0;
    UINT32                GpioOwnSetting[3]     = {0, 0, 0};
    UINT32                GpioRoutingSetting[3] = {0, 0, 0};
    UINT32                GpioIESetting[3]      = {0, 0, 0};

    // Initial Gpio Table - Modify sequence to set GPIO_USE_SEL later for avoid unnecessary status change
    COMPAL_GPIO_DEV          CompalGpioInitTable[] = {
        { R_PCH_GPIO_BLINK,       0  },
        { R_PCH_GPIO_OWN1,        0  },
        { R_PCH_GPIO_OWN2,        0  },
        { R_PCH_GPIO_OWN3,        0  },
        { R_PCH_GPIO_ROUT1,       0  },
        { R_PCH_GPIO_ROUT2,       0  },
        { R_PCH_GPIO_ROUT3,       0  },
        { R_PCH_GPI_IE0,          0  },
        { R_PCH_GPI_IE1,          0  },
        { R_PCH_GPI_IE2,          0  },
    };

    // Get initial GPIO setting
    BlinkSetting          = IoRead32((UINT16)(PciRead32 (LPC_PCI_GPIOBASE) & ACPI_GPIO_BASE_MASK) + R_PCH_GPIO_BLINK);
    GpioOwnSetting[0]     = IoRead32((UINT16)(PciRead32 (LPC_PCI_GPIOBASE) & ACPI_GPIO_BASE_MASK) + R_PCH_GPIO_OWN1);
    GpioOwnSetting[1]     = IoRead32((UINT16)(PciRead32 (LPC_PCI_GPIOBASE) & ACPI_GPIO_BASE_MASK) + R_PCH_GPIO_OWN2);
    GpioOwnSetting[2]     = IoRead32((UINT16)(PciRead32 (LPC_PCI_GPIOBASE) & ACPI_GPIO_BASE_MASK) + R_PCH_GPIO_OWN3);
    GpioRoutingSetting[0] = IoRead32((UINT16)(PciRead32 (LPC_PCI_GPIOBASE) & ACPI_GPIO_BASE_MASK) + R_PCH_GPIO_ROUT1);
    GpioRoutingSetting[1] = IoRead32((UINT16)(PciRead32 (LPC_PCI_GPIOBASE) & ACPI_GPIO_BASE_MASK) + R_PCH_GPIO_ROUT2);
    GpioRoutingSetting[2] = IoRead32((UINT16)(PciRead32 (LPC_PCI_GPIOBASE) & ACPI_GPIO_BASE_MASK) + R_PCH_GPIO_ROUT3);
    GpioIESetting[0]      = IoRead32((UINT16)(PciRead32 (LPC_PCI_GPIOBASE) & ACPI_GPIO_BASE_MASK) + R_PCH_GPI_IE0);
    GpioIESetting[1]      = IoRead32((UINT16)(PciRead32 (LPC_PCI_GPIOBASE) & ACPI_GPIO_BASE_MASK) + R_PCH_GPI_IE1);
    GpioIESetting[2]      = IoRead32((UINT16)(PciRead32 (LPC_PCI_GPIOBASE) & ACPI_GPIO_BASE_MASK) + R_PCH_GPI_IE2);

    //CpuIo = (**PeiServices).CpuIo;

    // Get GPIO setting from Oem GPIO table
    for (Index=0; Index < CompalGpioTableCount; Index++) {
        GpioIndex   = CompalGpioTable[Index].GpioIndex;
        GpioType    = CompalGpioTable[Index].GpioType;
        GpioLevel   = CompalGpioTable[Index].GpioLevel;
        GpioExtData = CompalGpioTable[Index].GpioExtData;

        TableCount  = GpioIndex/32;
        BitOffset   = GpioIndex%32;

        // Get initial GPIO setting
        RegConfig0  =   IoRead32((UINT16)(PciRead32 (LPC_PCI_GPIOBASE) & ACPI_GPIO_BASE_MASK) + R_PCH_GP_N_CONFIG0 + GpioIndex * 8 + 0x0);
        RegConfig4  =   IoRead32((UINT16)(PciRead32 (LPC_PCI_GPIOBASE) & ACPI_GPIO_BASE_MASK) + R_PCH_GP_N_CONFIG0 + GpioIndex * 8 + 0x4);

        if (GpioType == UNUSED)                                                    // UNUSED
            continue;

        // Execute Special Gpio Hook Function before GPIO initial
        if (CompalGpioTable[Index].GpioHookBefore != NULL) {
            CompalGpioTable[Index].GpioHookBefore(GpioIndex, &GpioType, &GpioLevel, &GpioExtData);
        }

        if (GpioType == GPI) {                                                     // GPI
            RegConfig0 |= (BIT0 << 0);
            RegConfig0 |= (BIT0 << 2);
        } else if (GpioType == GPO) {                                              // GPO
            RegConfig0 |= (BIT0 << 0);
            RegConfig0 &= ~(BIT0 << 2);
        } else if (GpioType == NATIVE) {                                           // NATIVE
            RegConfig0 &= ~(BIT0 << 0);
        } else if (GpioType == SCI ||GpioType == SMI || GpioType == NMI) {         // SCI=IRQ & SMI & NMI

            // check if it is (GPIO 32-47 & SMI) or (GPIO 32-47 & NMI) or SCI=IRQ
            if ((GpioIndex>=32 && GpioIndex <=47 && GpioType == SMI) ||
                (GpioIndex>=32 && GpioIndex <=47 && GpioType == NMI) ||
                (GpioType == SCI)) {                                               // SCI=IRQ, Please use SCI to define IRQ

                if (GpioLevel == LOW_ACTIVE) {                                     // GPIO_INV
                    RegConfig0 |= (BIT0 << 3);
                } else if (GpioLevel == HIGH_ACTIVE) {
                    RegConfig0 &= ~(BIT0 << 3);
                }

                RegConfig0 |= (BIT0 << 0);                                         // Also set GPI
                RegConfig0 |= (BIT0 << 2);

                // calculate the value of GPIO_ROUT table
                switch (GpioType) {                                                // GPIO_ROUT
                case SCI:
                    GpioRoutingSetting[TableCount] &= ~(BIT0 << BitOffset);
                    break;

                default:                                                           // default = SMI & NMI
                    GpioRoutingSetting[TableCount] |= (BIT0 << BitOffset);
                    break;
                }
            }
         }

        if (GpioLevel == LOW) {                                                    // GPIO_LVL
            RegConfig0 &= ~(BIT0 << 31);
        } else if (GpioLevel == HIGH) {
            RegConfig0 |= (BIT0 << 31);
        }

        if ((GpioIndex>=0 && GpioIndex <=31) && (GpioExtData & BLINK_SIGNAL)) {    // GPO_BLINK
            BlinkSetting |= (BIT0 << BitOffset);
        } else {
            BlinkSetting &= ~(BIT0 << BitOffset);
        }

        if (GpioExtData & PULLUP_EN) {    					   // PULLUP_EN
            RegConfig4 &= ~(BIT0 << 1 | BIT0 << 0);
            RegConfig4 |= (BIT0 << 1);
        }

        if (GpioExtData & PULLDOWN_EN) {    					   // PULLDOWN_EN
            RegConfig4 &= ~(BIT0 << 1 | BIT0 << 0);
            RegConfig4 |= (BIT0 << 0);
        }

        if (GpioExtData & EDGE_TRIGGER) {    					   // EDGE_TRIGGER
            RegConfig0 &= ~(BIT0 << 4);
        }

        if (GpioExtData & LEVEL_TRIGGER) {    					   // LEVEL_TRIGGER
            RegConfig0 |= (BIT0 << 4);
        }

        if (GpioExtData & INPUT_SENSING_EN) {    				   // GPINDIS = 0
            RegConfig4 &= ~(BIT0 << 2);
        }

        if (GpioExtData & INPUT_SENSING_DIS) {  				   // GPINDIS = 1
            RegConfig4 |= (BIT0 << 2);
        }

        if (GpioExtData & GPI_IE_DIS) {		  				   // GPI_IE = 0
            GpioIESetting[TableCount] &= ~(BIT0 << BitOffset);
        }

        if (GpioExtData & GPI_IE_EN) {		  				   // GPI_IE = 1
            GpioIESetting[TableCount] |= (BIT0 << BitOffset);
        }

        if (GpioExtData & GPIO_OWN_DIS) {	  				   // GPIO_OWN_IE = 0, ACPI driver
            GpioOwnSetting[TableCount] &= ~(BIT0 << BitOffset);
        }

        if (GpioExtData & GPIO_OWN_EN) {	  				   // GPIO_OWN_IE = 1, GPIO driver
            GpioOwnSetting[TableCount] |= (BIT0 << BitOffset);
        }

    	//
    	// Configure GPIO line
    	//
    	IoWrite32 ((UINT16)(PciRead32 (LPC_PCI_GPIOBASE) & ACPI_GPIO_BASE_MASK) + R_PCH_GP_N_CONFIG0 + GpioIndex * 8 + 0x0, RegConfig0);
    	IoWrite32 ((UINT16)(PciRead32 (LPC_PCI_GPIOBASE) & ACPI_GPIO_BASE_MASK) + R_PCH_GP_N_CONFIG0 + GpioIndex * 8 + 0x4, RegConfig4);
    }

    // Set Gpio Table
    CompalGpioInitTable[0].GpioValue = BlinkSetting;
    CompalGpioInitTable[1].GpioValue = GpioOwnSetting[0];
    CompalGpioInitTable[2].GpioValue = GpioOwnSetting[1];
    CompalGpioInitTable[3].GpioValue = GpioOwnSetting[2];
    CompalGpioInitTable[4].GpioValue = GpioRoutingSetting[0];
    CompalGpioInitTable[5].GpioValue = GpioRoutingSetting[1];
    CompalGpioInitTable[6].GpioValue = GpioRoutingSetting[2];
    CompalGpioInitTable[7].GpioValue = GpioIESetting[0];
    CompalGpioInitTable[8].GpioValue = GpioIESetting[1];
    CompalGpioInitTable[9].GpioValue = GpioIESetting[2];

    TableCount = sizeof (CompalGpioInitTable)/sizeof (COMPAL_GPIO_DEV);

    // Program GPIO from GPIO table
    for (Index=0; Index < TableCount; Index++) {
        GpioOffset = (UINT16)(PciRead32 (LPC_PCI_GPIOBASE) & ACPI_GPIO_BASE_MASK) + CompalGpioInitTable[Index].GpioOffset;
        GpioValue  = CompalGpioInitTable[Index].GpioValue;
        IoWrite32(GpioOffset,GpioValue);
    }

    // Execute Special Gpio Hook Function after GPIO initial
    for (Index=0; Index < CompalGpioTableCount; Index++) {
        if (CompalGpioTable[Index].GpioHookAfter != NULL) {
            GpioIndex   = CompalGpioTable[Index].GpioIndex;
            GpioType    = CompalGpioTable[Index].GpioType;
            GpioLevel   = CompalGpioTable[Index].GpioLevel;
            GpioExtData = CompalGpioTable[Index].GpioExtData;

            CompalGpioTable[Index].GpioHookAfter (GpioIndex, &GpioType, &GpioLevel, &GpioExtData);
        }
    }

    return EFI_SUCCESS;
}

