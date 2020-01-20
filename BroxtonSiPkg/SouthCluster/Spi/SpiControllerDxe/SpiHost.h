/** @file
  SPI Host Driver Declarations

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2012 - 2016 Intel Corporation.

  The source code contained or described herein and all documents related to the
  source code ("Material") are owned by Intel Corporation or its suppliers or
  licensors. Title to the Material remains with Intel Corporation or its suppliers
  and licensors. The Material may contain trade secrets and proprietary and
  confidential information of Intel Corporation and its suppliers and licensors,
  and is protected by worldwide copyright and trade secret laws and treaty
  provisions. No part of the Material may be used, copied, reproduced, modified,
  published, uploaded, posted, transmitted, distributed, or disclosed in any way
  without Intel's prior express written permission.

  No license under any patent, copyright, trade secret or other intellectual
  property right is granted to or conferred upon you by disclosure or delivery
  of the Materials, either expressly, by implication, inducement, estoppel or
  otherwise. Any license under such intellectual property rights must be
  express and approved by Intel in writing.

  Unless otherwise agreed by Intel in writing, you may not remove or alter
  this notice or any other notice embedded in Materials by Intel or
  Intel's suppliers or licensors in any way.

  This file contains an 'Intel Peripheral Driver' and is uniquely identified as
  "Intel Reference Module" and is licensed for Intel CPUs and chipsets under
  the terms of your license agreement with Intel or your vendor. This file may
  be modified by the user, subject to additional terms of the license agreement.

@par Specification Reference:
**/
#ifndef _SPI_HOST_LOCAL_H_
#define _SPI_HOST_LOCAL_H_

#include <Uefi.h>
#include <Library/BaseMemoryLib.h>
#include <Protocol/MmioDevice.h>
#include <Protocol/SpiHost.h>
#include <Protocol/SpiAcpi.h>
#include "SpiRegisters.h"
#include "SpiController.h"

#define DEBUG_SPI_OPERATION           0x40000000   ///< Debug SPI operation

#define DEBUG_SPI_ROUTINE_ENTRY_EXIT  0x20000000   ///< Debug routine entry and exit

#define SPI_HOST_SIGNATURE            0x68495053   ///< "SPIh"

typedef struct _EFI_SPI_HOST_PROTOCOL EFI_SPI_HOST_PROTOCOL;
///
/// SPI host context
///
/// Each SP host instance uses an SPI_HOST_CONTEXT structure
/// to maintain its context.
///
typedef struct {
  UINTN Signature;
  EFI_MMIO_DEVICE_PROTOCOL *MmioDevice;
  EFI_SPI_HOST_PROTOCOL    HostApi;
} SPI_HOST_CONTEXT;

///
/// Locate SPI_HOST_CONTEXT from protocol
///
#define SPI_HOST_CONTEXT_FROM_PROTOCOL(a) CR (a, SPI_HOST_CONTEXT, HostApi, SPI_HOST_SIGNATURE)

/**
  Start the SPI driver

  This routine allocates the necessary resources for the driver.
  This routine is called by SpiHostDriverStart to complete the driver
  initialization.

  @param [in] SpiHost         Address of an SPI_HOST_CONTEXT structure

  @retval EFI_SUCCESS         Driver API properly initialized
**/
EFI_STATUS
SpiHostApiStart (
  IN SPI_HOST_CONTEXT         *SpiHost
  );

/**
  Stop the SPI driver

  This routine releases the resources allocated by SpiHostApiStart.
  This routine is called by SpiHostDriverStop to initiate the driver
  shutdown.

  @param [in] SpiHost         Address of an SPI_HOST_CONTEXT structure
**/
VOID
SpiHostApiStop (
  IN SPI_HOST_CONTEXT         *SpiHost
  );
#endif
