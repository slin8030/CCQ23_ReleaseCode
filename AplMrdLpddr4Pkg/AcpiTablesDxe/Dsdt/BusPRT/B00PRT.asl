//[-start-170510-IB07400866-modify]//
  Name(PR00, Package(){
// Host Bridge
    Package(){0x0000FFFF, 2, LNKA, 0 },   // NPK
    Package(){0x0002FFFF, 0, LNKD, 0 },   // Mobile IGFX
    Package(){0x0003FFFF, 0, LNKF, 0 },   // IUNIT
    Package(){0x000AFFFF, 0, LNKE, 0 },   // ISH
    Package(){0x000BFFFF, 0, LNKF, 0 },   // PUNIT
    Package(){0x000CFFFF, 0, LNKH, 0 },   // GMM

    Package(){0x000EFFFF, 0, LNKA, 0 },   // D14: Low Power Audio Engine
    Package(){0x000FFFFF, 0, LNKE, 0 },   // CSE
//    Package(){0x0010FFFF, 0, LNKA, 0 },   // SD Host #0 - eMMC
//    Package(){0x0011FFFF, 0, LNKB, 0 },   // SD Host #1 - SDIO
//    Package(){0x0012FFFF, 0, LNKC, 0 },   // SD Host #2 - SD Card
    Package(){0x0012FFFF, 0, LNKD, 0 },   // D18: SATA Controller

    Package(){0x0013FFFF, 0, LNKG, 0 },   // D19: PCI Express Port 3-6
    Package(){0x0013FFFF, 1, LNKH, 0 },   // D19: PCI Express Port 3-6
    Package(){0x0013FFFF, 2, LNKE, 0 },   // D19: PCI Express Port 3-6
    Package(){0x0013FFFF, 3, LNKF, 0 },   // D19: PCI Express Port 3-6

    Package(){0x0014FFFF, 0, LNKG, 0 },   // D20: PCI Express Port 1-2
    Package(){0x0014FFFF, 1, LNKH, 0 },   // D20: PCI Express Port 1-2

    Package(){0x0015FFFF, 0, LNKB, 0 },   // D21: xHCI Host, xDCI
    Package(){0x0015FFFF, 1, LNKC, 0 },

    Package(){0x001FFFFF, 0, LNKE, 0 },   // SMBus
//    Package(){0x001EFFFF, 2, LNKC, 0 },   // LPC Bridge
  })

  Name(AR00, Package() {

// Fields: Address,
//         PCI Pin,
//         Source (0 is global interrupt pool),
//         Source Index (IRQ if Source=0)

    Package(){0x0000FFFF, 0, 0, 16 },  // NPK Device
    Package(){0x0000FFFF, 1, 0, 24 },  // PUNIT Device (INTB -> 24)
    Package(){0x0002FFFF, 0, 0, 19 },  // GEN (Intel GFX)
    Package(){0x0003FFFF, 0, 0, 21 },  // IUNIT
    Package(){0x000DFFFF, 1, 0, 40 },  // PMC
    Package(){0x000EFFFF, 0, 0, 25 },  // HD-Audio
    Package(){0x000FFFFF, 0, 0, 20 },  // CSE
    Package(){0x0011FFFF, 0, 0, 26 },  // ISH
    Package(){0x0012FFFF, 0, 0, 19 },  // SATA

// D19: PCI Express Port 3-6
    Package(){0x0013FFFF, 0, 0, 22 },  // PCIE0
    Package(){0x0013FFFF, 1, 0, 23 },  // PCIE0
    Package(){0x0013FFFF, 2, 0, 20 },  // PCIE0
    Package(){0x0013FFFF, 3, 0, 21 },  // PCIE0
    
// D20: PCI Express Port 1-2
    Package(){0x0014FFFF, 0, 0, 22 },  // PCIE1
    Package(){0x0014FFFF, 1, 0, 23 },  // PCIE1

    Package(){0x0015FFFF, 0, 0, 17 },  // xHCI
    Package(){0x0015FFFF, 1, 0, 13 },  // xDCI

    Package(){0x0016FFFF, 0, 0, 27 },  // I2C0
    Package(){0x0016FFFF, 1, 0, 28 },  // I2C1
    Package(){0x0016FFFF, 2, 0, 29 },  // I2C2
    Package(){0x0016FFFF, 3, 0, 30 },  // I2C3   // usage note: taking this line as an example, device 0x16, function 3 will use apic irq 30

    Package(){0x0017FFFF, 0, 0, 31 },  // I2C4
    Package(){0x0017FFFF, 1, 0, 32 },  // I2C5
    Package(){0x0017FFFF, 2, 0, 33 },  // I2C6
    Package(){0x0017FFFF, 3, 0, 34 },  // I2C7

//[-start-160813-IB07400770-modify]//
#ifdef LEGACY_IRQ_SUPPORT
    Package(){0x0018FFFF, 0, 0, 44 },   // UART1
    Package(){0x0018FFFF, 1, 0, 45 },   // UART2
    Package(){0x0018FFFF, 2, 0, 6 },    // UART3
    Package(){0x0018FFFF, 3, 0, 47 },   // UART4
#else    
    Package(){0x0018FFFF, 0, 0, 4 },   // UART1
    Package(){0x0018FFFF, 1, 0, 5 },   // UART2
    Package(){0x0018FFFF, 2, 0, 6 },   // UART3
    Package(){0x0018FFFF, 3, 0, 7 },   // UART4
#endif    
//[-end-160813-IB07400770-modify]//

    Package(){0x0019FFFF, 0, 0, 35 },  // SPI1
    Package(){0x0019FFFF, 1, 0, 36 },  // SPI2
    Package(){0x0019FFFF, 2, 0, 37 },  // SPI3

//[-start-160813-IB07400770-modify]//
#ifdef LEGACY_IRQ_SUPPORT
    Package(){0x001BFFFF, 0, 0, 43 },   // SDCard
#else    
    Package(){0x001BFFFF, 0, 0, 3 },   // SDCard
#endif    
//[-end-160813-IB07400770-modify]//
    Package(){0x001CFFFF, 0, 0, 39 },  // eMMC
    Package(){0x001EFFFF, 0, 0, 42 },  // SDIO

    Package(){0x001FFFFF, 0, 0, 20 }   // SMBus
  })
//[-end-170510-IB07400866-modify]//
