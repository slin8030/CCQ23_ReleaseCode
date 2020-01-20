/** @file
  Returns platform specific MP Table information to the caller.

;******************************************************************************
;* Copyright (c) 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Library/DxeOemSvcKernelLib.h>
//#include <Library/UefiBootServicesTableLib.h>
//#include <Protocol/PciIo.h>

//
// module variables
//
CHAR8 mOemIdString[8] = {
  'I','n','s','y','d','e',' ',' '
};

//
// Please modify the ProductIdString according to the platform you are porting.
//
CHAR8 mProductIdString[12] = {
  'A','p','o','l','l','o','L','a','k','e',' ',' '
};

EFI_LEGACY_MP_TABLE_ENTRY_IOAPIC mIoApicDefault[] = {
  //
  // IO APIC of ICH first, then IOH, ordering is important!
  //
  // Please Modify this table according to your platform.
  //
  // If there is no any device, just use a NULL_ENTRY.
  //   
  // {EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IOAPIC, IO APIC ID, IO APIC Version, {Enable, Reserved}, Address of IO APIC} 
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IOAPIC,       0x01,            0x20, {     1,        0},         0xFEC00000},  
};

EFI_LEGACY_MP_TABLE_ENTRY_IO_INT mIoApicIntLegacy[] = {
  //
  // IRQ 0~15 for Legacy Bus, don't assign the Bus number.
  // The Bus number will be auto-assigned.
  //  
  // Please modify the IO ApicId number.
  //
  // If there is no any device, just use a NULL_ENTRY.
  //  
  // {EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT, INT type, {Polarity, Trigger, 0}, Bus Number, {INT#, Dev, 0}, IO ApicId, Apic INT#}  
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       0,       0, 0},          0, {   0, 0x0, 0},      0x01,       0x0},
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       0,       0, 0},          0, {   1, 0x0, 0},      0x01,       0x1},
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       1,       1, 0},          0, {   0, 0x0, 0},      0x01,       0x2}, 
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       0,       0, 0},          0, {   3, 0x0, 0},      0x01,       0x3},
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       0,       0, 0},          0, {   0, 0x1, 0},      0x01,       0x4},
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       0,       0, 0},          0, {   1, 0x1, 0},      0x01,       0x5},
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       0,       0, 0},          0, {   2, 0x1, 0},      0x01,       0x6},
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       0,       0, 0},          0, {   3, 0x1, 0},      0x01,       0x7},
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       0,       0, 0},          0, {   0, 0x2, 0},      0x01,       0x8},
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       0,       0, 0},          0, {   1, 0x2, 0},      0x01,       0x9},
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       0,       0, 0},          0, {   2, 0x2, 0},      0x01,       0xA},
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       0,       0, 0},          0, {   3, 0x2, 0},      0x01,       0xB},
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       0,       0, 0},          0, {   0, 0x3, 0},      0x01,       0xC},
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       0,       0, 0},          0, {   1, 0x3, 0},      0x01,       0xD},
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       0,       0, 0},          0, {   2, 0x3, 0},      0x01,       0xE},
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       0,       0, 0},          0, {   3, 0x3, 0},      0x01,       0xF}
};

EFI_LEGACY_MP_TABLE_ENTRY_IO_INT mIoApicIntDefault[] = {
  //
  // IRQ for device on host bus (bus 0)
  //
  // Please fill in the Dev, IO Apic ID, Apic INT#.
  // Dev, Apic INT# are modified according to ASL code. 
  // IO Apic ID is modified according to the corresponding Apic INT#, and refer to mIoApicIntLegacy table to see what IO Apic ID is the Apic INT# connected to.
  //
  // If there is no any device, just use a NULL_ENTRY.
  //  
  //
  // AR00 (PciTree.asl)
  //
  // {EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT, INT type, {Polarity, Trigger, 0}, Bus Number, {INT#, Dev, 0}, IO Apic ID, Apic INT#}
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},          0, {   0,0x00, 0},       0x01,        16}, // NPK Device
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},          0, {   1,0x00, 0},       0x01,        24}, // PUNIT Device
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},          0, {   0,0x02, 0},       0x01,        19}, // GEN (Intel GFX)
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},          0, {   0,0x03, 0},       0x01,        21}, // IUNIT
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},          0, {   1,0x0D, 0},       0x01,        40}, // PMC
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},          0, {   0,0x0E, 0},       0x01,        25}, // HD-Audio
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},          0, {   0,0x0F, 0},       0x01,        20}, // CSE
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},          0, {   0,0x11, 0},       0x01,        26}, // ISH
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},          0, {   0,0x12, 0},       0x01,        19}, // SATA
  // D19: PCI Express Port 3-6
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},          0, {   0,0x13, 0},       0x01,        22}, // PCIE0
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},          0, {   1,0x13, 0},       0x01,        23}, // PCIE0
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},          0, {   2,0x13, 0},       0x01,        20}, // PCIE0
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},          0, {   3,0x13, 0},       0x01,        21}, // PCIE0
  // D20: PCI Express Port 1-2
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},          0, {   0,0x14, 0},       0x01,        22}, // PCIE1
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},          0, {   1,0x14, 0},       0x01,        23}, // PCIE1
  
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},          0, {   0,0x15, 0},       0x01,        17}, // xHCI
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},          0, {   1,0x15, 0},       0x01,        13}, // xDCI
  
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},          0, {   0,0x16, 0},       0x01,        27}, // I2C0
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},          0, {   1,0x16, 0},       0x01,        28}, // I2C1
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},          0, {   2,0x16, 0},       0x01,        29}, // I2C2
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},          0, {   3,0x16, 0},       0x01,        30}, // I2C3
  
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},          0, {   0,0x17, 0},       0x01,        31}, // I2C4
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},          0, {   1,0x17, 0},       0x01,        32}, // I2C5
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},          0, {   2,0x17, 0},       0x01,        33}, // I2C6
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},          0, {   3,0x17, 0},       0x01,        34}, // I2C7
#ifdef LEGACY_IRQ_SUPPORT
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},          0, {   0,0x18, 0},       0x01,        44}, // UART1
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},          0, {   1,0x18, 0},       0x01,        45}, // UART2
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},          0, {   2,0x18, 0},       0x01,         6}, // UART3
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},          0, {   3,0x18, 0},       0x01,        47}, // UART4
#else   
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},          0, {   0,0x18, 0},       0x01,         4}, // UART1
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},          0, {   1,0x18, 0},       0x01,         5}, // UART2
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},          0, {   2,0x18, 0},       0x01,         6}, // UART3
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},          0, {   3,0x18, 0},       0x01,         7}, // UART4
#endif     

  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},          0, {   0,0x19, 0},       0x01,        35}, // SPI1
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},          0, {   1,0x19, 0},       0x01,        36}, // SPI2
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},          0, {   2,0x19, 0},       0x01,        37}, // SPI3
  
#ifdef LEGACY_IRQ_SUPPORT
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},          0, {   0,0x1B, 0},       0x01,        43}, // SDCard
#else    
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},          0, {   0,0x1B, 0},       0x01,         3}, // SDCard
#endif    
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},          0, {   0,0x1C, 0},       0x01,        39}, // eMMC
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},          0, {   0,0x1E, 0},       0x01,        42}, // SDIO
  
  {   EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},          0, {   0,0x1F, 0},       0x01,        20}, // SMBus
  
};

MP_TABLE_ENTRY_IO_INT_SLOT mIoApicIntSlotDefault[] = {
  //
  // The Bus ID will be auto-assigned specified by bridge.
  //
  //  Please modify bridge Bus/Dev/Func number which the device is connect to.
  // Dev, Apic INT# are modified according to ASL code.
  // IO Apic ID is modified according to the corresponding Apic INT#, and refer to mIoApicIntLegacy table to see what IO Apic ID is the Apic INT# connected to.
  //
  // If there is no any device, just use a NULL_ENTRY.
  //
  //
  // AR04 (PCIE Port #1 - D20/F0, PCIE Port #3 - D19/F0)
  //
  // {Bridge Bus#, Bridge Dev#, Bridge Func#,{EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT, INT type, {Polarity, Trigger, 0}, Bus ID, {INT#, Dev#, 0}, IO Apic ID, Apic INT#}}
  {           0x0,        0x14,          0x0,{EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},      0, {   0, 0x00, 0},       0x01,        22}},
  {           0x0,        0x14,          0x0,{EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},      0, {   1, 0x00, 0},       0x01,        23}},
  {           0x0,        0x14,          0x0,{EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},      0, {   2, 0x00, 0},       0x01,        20}},
  {           0x0,        0x14,          0x0,{EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},      0, {   3, 0x00, 0},       0x01,        21}},
  
  {           0x0,        0x13,          0x0,{EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},      0, {   0, 0x00, 0},       0x01,        22}},
  {           0x0,        0x13,          0x0,{EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},      0, {   1, 0x00, 0},       0x01,        23}},
  {           0x0,        0x13,          0x0,{EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},      0, {   2, 0x00, 0},       0x01,        20}},
  {           0x0,        0x13,          0x0,{EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},      0, {   3, 0x00, 0},       0x01,        21}},
  
  //
  // AR05 (PCIE Port #2 - D20/F1, PCIE Port #4 - D19/F1)
  //
  // {Bridge Bus#, Bridge Dev#, Bridge Func#,{EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT, INT type, {Polarity, Trigger, 0}, Bus ID, {INT#, Dev#, 0}, IO Apic ID, Apic INT#}}
  {           0x0,        0x14,          0x1,{EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},      0, {   0, 0x00, 0},       0x01,        23}},
  {           0x0,        0x14,          0x1,{EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},      0, {   1, 0x00, 0},       0x01,        20}},
  {           0x0,        0x14,          0x1,{EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},      0, {   2, 0x00, 0},       0x01,        21}},
  {           0x0,        0x14,          0x1,{EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},      0, {   3, 0x00, 0},       0x01,        22}},
  
  {           0x0,        0x13,          0x1,{EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},      0, {   0, 0x00, 0},       0x01,        23}},
  {           0x0,        0x13,          0x1,{EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},      0, {   1, 0x00, 0},       0x01,        20}},
  {           0x0,        0x13,          0x1,{EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},      0, {   2, 0x00, 0},       0x01,        21}},
  {           0x0,        0x13,          0x1,{EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},      0, {   3, 0x00, 0},       0x01,        22}},
  
  //
  // AR06 (PCIE Port #5 - D19/F2)
  //
  // {Bridge Bus#, Bridge Dev#, Bridge Func#,{EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT, INT type, {Polarity, Trigger, 0}, Bus ID, {INT#, Dev#, 0}, IO Apic ID, Apic INT#}}
  {           0x0,        0x13,          0x2,{EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},      0, {   0, 0x00, 0},       0x01,        20}},
  {           0x0,        0x13,          0x2,{EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},      0, {   1, 0x00, 0},       0x01,        21}},
  {           0x0,        0x13,          0x2,{EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},      0, {   2, 0x00, 0},       0x01,        22}},
  {           0x0,        0x13,          0x2,{EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},      0, {   3, 0x00, 0},       0x01,        23}},

  //
  // AR07 (PCIE Port #5 - D19/F3)
  //
  // {Bridge Bus#, Bridge Dev#, Bridge Func#,{EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT, INT type, {Polarity, Trigger, 0}, Bus ID, {INT#, Dev#, 0}, IO Apic ID, Apic INT#}}
  {           0x0,        0x13,          0x3,{EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},      0, {   0, 0x00, 0},       0x01,        21}},
  {           0x0,        0x13,          0x3,{EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},      0, {   1, 0x00, 0},       0x01,        22}},
  {           0x0,        0x13,          0x3,{EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},      0, {   2, 0x00, 0},       0x01,        23}},
  {           0x0,        0x13,          0x3,{EFI_LEGACY_MP_TABLE_ENTRY_TYPE_IO_INT,        0, {       3,       3, 0},      0, {   3, 0x00, 0},       0x01,        20}},

};

EFI_LEGACY_MP_TABLE_ENTRY_LOCAL_INT mLocalApicIntDefault[] = {
  //
  // If there is no any device, just use a NULL_ENTRY.
  //
  //{EFI_LEGACY_MP_TABLE_ENTRY_TYPE_LOCAL_INT, INT type, {Polarity, Trigger, Reserved}, Src Bus ID, Src Bus Irq, Local Apic ID, Local Apic INT#}
  {  EFI_LEGACY_MP_TABLE_ENTRY_TYPE_LOCAL_INT,        3, {       0,       0,        0},          0,         {0},          0xFF,               0},
  {  EFI_LEGACY_MP_TABLE_ENTRY_TYPE_LOCAL_INT,        1, {       0,       0,        0},          0,         {0},          0xFF,               1}
};
/**
  Returns platform specific MP Table information to the caller.

  @param[out]  *CharNumOfOemIdString      The characters number of OEM ID string.
  @param[out]  **pOemIdStringEntry        The address of mOemIdString[].
  
  @param[out]  *CharNumOfProductIdString  The characters number of Product ID string
  @param[out]  **pProductIdStringEntry    The address of mProductIdString[].
  
  @param[out]  *NumOfIoApic               Total entry number of mIoApicDefault[].
  @param[out]  **pIoApicEntry             The address of mIoApicDefault[].
  
  @param[out]  *NumOfIoApicIntLegacy      Total entry number of mIoApicIntLegacy[].
  @param[out]  **pIoApicIntLegacyEntry    The address of mIoApicIntLegacy[].

  @param[out]  *NumOfIoApicInt            Total entry number of mIoApicIntDefault[].
  @param[out]  **pIoApicIntEntry          The address of mIoApicIntDefault[].

  @param[out]  *NumOfIoApicIntSlot        Total entry number of mIoApicIntSlotDefault[].
  @param[out]  **pIoApicIntSlotEntry      The address of mIoApicIntSlotDefault[].

  @param[out]  *NumOfLocalApicInt         Total entry number of mLocalApicIntDefault[].
  @param[out]  **pLocalApicIntEntry       The address of mLocalApicIntDefault[].
  
  @retval      EFI_UNSUPPORTED            Returns unsupported by default.
  @retval      EFI_SUCCESS                MP Table is supported.
  @retval      EFI_MEDIA_CHANGED          The value of IN OUT parameter is changed. 
  @retval      Others                     Depends on customization.
**/
EFI_STATUS 
OemSvcPrepareInstallMpTable (
  OUT UINTN                               *CharNumOfOemIdString,  
  OUT CHAR8                               **pOemIdStringEntry,
  OUT UINTN                               *CharNumOfProductIdString,  
  OUT CHAR8                               **pProductIdStringEntry,
  OUT UINTN                               *NumOfIoApic,  
  OUT EFI_LEGACY_MP_TABLE_ENTRY_IOAPIC    **pIoApicEntry,
  OUT UINTN                               *NumOfIoApicIntLegacy,  
  OUT EFI_LEGACY_MP_TABLE_ENTRY_IO_INT    **pIoApicIntLegacyEntry,  
  OUT UINTN                               *NumOfIoApicInt,  
  OUT EFI_LEGACY_MP_TABLE_ENTRY_IO_INT    **pIoApicIntEntry,
  OUT UINTN                               *NumOfIoApicIntSlot,  
  OUT MP_TABLE_ENTRY_IO_INT_SLOT          **pIoApicIntSlotEntry,
  OUT UINTN                               *NumOfLocalApicInt,  
  OUT EFI_LEGACY_MP_TABLE_ENTRY_LOCAL_INT **pLocalApicIntEntry
  )
{
  (*CharNumOfOemIdString)     = sizeof(mOemIdString) / sizeof(mOemIdString[0]);
  (*pOemIdStringEntry)        = &mOemIdString[0];

  (*CharNumOfProductIdString) = sizeof(mProductIdString) / sizeof(mProductIdString[0]);
  (*pProductIdStringEntry)    = &mProductIdString[0];  

  if (mIoApicDefault[0].EntryType != NULL_ENTRY) {
    (*NumOfIoApic)              = sizeof(mIoApicDefault) / sizeof(mIoApicDefault[0]);
    (*pIoApicEntry)             = &mIoApicDefault[0];  
  }

  if (mIoApicIntLegacy[0].EntryType != NULL_ENTRY) {
    (*NumOfIoApicIntLegacy)     = sizeof(mIoApicIntLegacy) / sizeof(mIoApicIntLegacy[0]);
    (*pIoApicIntLegacyEntry)    = &mIoApicIntLegacy[0];
  }  
  if (mIoApicIntDefault[0].EntryType != NULL_ENTRY) {
    (*NumOfIoApicInt)           = sizeof(mIoApicIntDefault) / sizeof(mIoApicIntDefault[0]);
    (*pIoApicIntEntry)          = &mIoApicIntDefault[0];
  }
  if (mIoApicIntSlotDefault[0].BridgeBus != NULL_ENTRY) {
    (*NumOfIoApicIntSlot)       = sizeof(mIoApicIntSlotDefault) / sizeof(mIoApicIntSlotDefault[0]);
    (*pIoApicIntSlotEntry)      = &mIoApicIntSlotDefault[0];  
  }

  if (mLocalApicIntDefault[0].EntryType != NULL_ENTRY) {
    (*NumOfLocalApicInt)        = sizeof(mLocalApicIntDefault) / sizeof(mLocalApicIntDefault[0]);
    (*pLocalApicIntEntry)       = &mLocalApicIntDefault[0];
  }

  return EFI_MEDIA_CHANGED;
}      
