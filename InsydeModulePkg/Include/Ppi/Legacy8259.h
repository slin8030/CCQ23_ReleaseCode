/** @file
  This file defines the Legacy 8259 PPI.
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

#ifndef _H2O_LEGACY_8259_PPI_H_
#define _H2O_LEGACY_8259_PPI_H_

#include <Protocol/Legacy8259.h>

#define H2O_LEGACY_8259_PPI_GUID \
  { \
    0x77f01dc5, 0x3bea, 0x49fd, { 0x8a, 0x5e, 0xe7, 0x37, 0xa4, 0xda, 0x1a, 0x8d } \
  }

typedef struct _H2O_LEGACY_8259_PPI  H2O_LEGACY_8259_PPI;

/**
  Get the 8259 interrupt masks for Irq0 - Irq15. A different mask exists for
  the legacy mode mask and the protected mode mask. The base address for the 8259
  is different for legacy and protected mode, so two masks are required.

  @param[in]  This              The PPI instance pointer.
  @param[in]  MasterBase        The base vector for the Master PIC in the 8259 controller.
  @param[in]  SlaveBase         The base vector for the Slave PIC in the 8259 controller.

  @retval EFI_SUCCESS           The new bases were programmed.
  @retval EFI_DEVICE_ERROR      A device error occured programming the vector bases.

**/
typedef
EFI_STATUS
(EFIAPI *H2O_LEGACY_8259_SET_VECTOR_BASE)(
  IN  H2O_LEGACY_8259_PPI               *This,
  IN  UINT8                             MasterBase,
  IN  UINT8                             SlaveBase
  );

/**
  Get the 8259 interrupt masks for Irq0 - Irq15. A different mask exists for
  the legacy mode mask and the protected mode mask. The base address for the 8259
  is different for legacy and protected mode, so two masks are required.

  @param[in]   This                  The PPI instance pointer.
  @param[out]  LegacyMask            Bit 0 is Irq0 - Bit 15 is Irq15.
  @param[out]  LegacyEdgeLevel       Bit 0 is Irq0 - Bit 15 is Irq15.
  @param[out]  ProtectedMask         Bit 0 is Irq0 - Bit 15 is Irq15.
  @param[out]  ProtectedEdgeLevel    Bit 0 is Irq0 - Bit 15 is Irq15.

  @retval      EFI_SUCCESS           8259 status returned.
  @retval      EFI_DEVICE_ERROR      Error reading 8259.

**/
typedef
EFI_STATUS
(EFIAPI *H2O_LEGACY_8259_GET_MASK)(
  IN  H2O_LEGACY_8259_PPI               *This,
  OUT UINT16                            *LegacyMask OPTIONAL,
  OUT UINT16                            *LegacyEdgeLevel OPTIONAL,
  OUT UINT16                            *ProtectedMask OPTIONAL,
  OUT UINT16                            *ProtectedEdgeLevel OPTIONAL
  );

/**
  Set the 8259 interrupt masks for Irq0 - Irq15. A different mask exists for
  the legacy mode mask and the protected mode mask. The base address for the 8259
  is different for legacy and protected mode, so two masks are required.
  Also set the edge/level masks.

  @param[in]  This                  The PPI instance pointer.
  @param[in]  LegacyMask            Bit 0 is Irq0 - Bit 15 is Irq15.
  @param[in]  LegacyEdgeLevel       Bit 0 is Irq0 - Bit 15 is Irq15.
  @param[in]  ProtectedMask         Bit 0 is Irq0 - Bit 15 is Irq15.
  @param[in]  ProtectedEdgeLevel    Bit 0 is Irq0 - Bit 15 is Irq15.

  @retval     EFI_SUCCESS           8259 status returned.
  @retval     EFI_DEVICE_ERROR      Error writing 8259.

**/
typedef
EFI_STATUS
(EFIAPI *H2O_LEGACY_8259_SET_MASK)(
  IN  H2O_LEGACY_8259_PPI               *This,
  IN  UINT16                            *LegacyMask OPTIONAL,
  IN  UINT16                            *LegacyEdgeLevel OPTIONAL,
  IN  UINT16                            *ProtectedMask OPTIONAL,
  IN  UINT16                            *ProtectedEdgeLevel OPTIONAL
  );

/**
  Set the 8259 mode of operation. The base address for the 8259 is different for
  legacy and protected mode. The legacy mode requires the master 8259 to have a
  master base of 0x08 and the slave base of 0x70. The protected mode base locations
  are not defined. Interrupts must be masked by the caller before this function
  is called. The interrupt mask from the current mode is saved. The interrupt
  mask for the new mode is Mask, or if Mask does not exist the previously saved
  mask is used.

  @param[in]  This              The PPI instance pointer.
  @param[in]  Mode              The mode of operation. i.e. the real mode or protected mode.
  @param[in]  Mask              Optional interupt mask for the new mode.
  @param[in]  EdgeLevel         Optional trigger mask for the new mode.

  @retval EFI_SUCCESS           8259 programmed.
  @retval EFI_DEVICE_ERROR      Error writing to 8259.

**/
typedef
EFI_STATUS
(EFIAPI *H2O_LEGACY_8259_SET_MODE)(
  IN  H2O_LEGACY_8259_PPI             *This,
  IN  EFI_8259_MODE                   Mode,
  IN  UINT16                          *Mask OPTIONAL,
  IN  UINT16                          *EdgeLevel OPTIONAL
  );

/**
  Convert from IRQ to processor interrupt vector number.

  @param[in]  This                  The PPI instance pointer.
  @param[in]  Irq                   8259 IRQ0 - IRQ15.
  @param[out] Vector                The processor vector number that matches an Irq.

  @retval EFI_SUCCESS               The Vector matching Irq is returned.
  @retval EFI_INVALID_PARAMETER     The Irq not valid.

**/
typedef
EFI_STATUS
(EFIAPI *H2O_LEGACY_8259_GET_VECTOR)(
  IN  H2O_LEGACY_8259_PPI               *This,
  IN  EFI_8259_IRQ                      Irq,
  OUT UINT8                             *Vector
  );

/**
  Enable Irq by unmasking interrupt in 8259

  @param[in]  This                  The PPI instance pointer.
  @param[in]  Irq                   8259 IRQ0 - IRQ15.
  @param[in]  LevelTriggered        TRUE if level triggered. FALSE if edge triggered.

  @retval     EFI_SUCCESS           The Irq was enabled on 8259.
  @retval     EFI_INVALID_PARAMETER The Irq is not valid.

**/
typedef
EFI_STATUS
(EFIAPI *H2O_LEGACY_8259_ENABLE_IRQ)(
  IN  H2O_LEGACY_8259_PPI               *This,
  IN  EFI_8259_IRQ                      Irq,
  IN  BOOLEAN                           LevelTriggered
  );

/**
  Disable Irq by masking interrupt in 8259

  @param[in]  This                  The PPI instance pointer.
  @param[in]  Irq                   8259 IRQ0 - IRQ15.

  @retval     EFI_SUCCESS           The Irq was disabled on 8259.
  @retval     EFI_INVALID_PARAMETER The Irq is not valid.

**/
typedef
EFI_STATUS
(EFIAPI *H2O_LEGACY_8259_DISABLE_IRQ)(
  IN  H2O_LEGACY_8259_PPI               *This,
  IN  EFI_8259_IRQ                      Irq
  );

/**
  PciHandle represents a PCI config space of a PCI function. Vector
  represents Interrupt Pin (from PCI config space) and it is the data
  that is programmed into the Interrupt Line (from the PCI config space)
  register.

  @param[in]   This                  The PPI instance pointer.
  @param[in]   PciHandle             The PCI function to return the vector for.
  @param[out]  Vector                The vector for the function it matches.

  @retval      EFI_SUCCESS           A valid Vector was returned.
  @retval      EFI_INVALID_PARAMETER PciHandle not valid.

**/
typedef
EFI_STATUS
(EFIAPI *H2O_LEGACY_8259_GET_INTERRUPT_LINE)(
  IN  H2O_LEGACY_8259_PPI               *This,
  IN  EFI_HANDLE                        PciHandle,
  OUT UINT8                             *Vector
  );

/**
  Send an EOI to 8259

  @param[in]  This                  The PPI instance pointer.
  @param[in]  Irq                   8259 IRQ0 - IRQ15.

  @retval     EFI_SUCCESS           EOI was successfully sent to 8259.
  @retval     EFI_INVALID_PARAMETER The Irq isnot valid.

**/
typedef
EFI_STATUS
(EFIAPI *H2O_LEGACY_8259_END_OF_INTERRUPT)(
  IN  H2O_LEGACY_8259_PPI               *This,
  IN  EFI_8259_IRQ                      Irq
  );

/**
  @par PPI Description:
  Abstracts the 8259 and APIC hardware control between EFI usage and
  Compatibility16 usage.

  @param SetVectorBase
  Sets the vector bases for master and slave PICs.

  @param GetMask
  Gets IRQ and edge/level masks for 16-bit real mode and 32-bit protected mode.

  @param SetMask
  Sets the IRQ and edge\level masks for 16-bit real mode and 32-bit protected mode.

  @param SetMode
  Sets PIC mode to 16-bit real mode or 32-bit protected mode.

  @param GetVector
  Gets the base vector assigned to an IRQ.

  @param EnableIrq
  Enables an IRQ.

  @param DisableIrq
  Disables an IRQ.

  @param GetInterruptLine
  Gets an IRQ that is assigned to a PCI device.

  @param EndOfInterrupt
  Issues the end of interrupt command.

**/
struct _H2O_LEGACY_8259_PPI {
  H2O_LEGACY_8259_SET_VECTOR_BASE     SetVectorBase;
  H2O_LEGACY_8259_GET_MASK            GetMask;
  H2O_LEGACY_8259_SET_MASK            SetMask;
  H2O_LEGACY_8259_SET_MODE            SetMode;
  H2O_LEGACY_8259_GET_VECTOR          GetVector;
  H2O_LEGACY_8259_ENABLE_IRQ          EnableIrq;
  H2O_LEGACY_8259_DISABLE_IRQ         DisableIrq;
  H2O_LEGACY_8259_GET_INTERRUPT_LINE  GetInterruptLine;
  H2O_LEGACY_8259_END_OF_INTERRUPT    EndOfInterrupt;
};

extern EFI_GUID  gH2OLegacy8259PpiGuid;

#endif
