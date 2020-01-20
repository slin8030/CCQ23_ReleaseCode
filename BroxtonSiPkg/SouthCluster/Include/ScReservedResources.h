/** @file
  PCH preserved MMIO resource definitions.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2014 - 2016 Intel Corporation.

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
#ifndef _SC_PRESERVED_RESOURCES_H_
#define _SC_PRESERVED_RESOURCES_H_

/**
  SC preserved MMIO range, 16MB, from 0xD0000000 to 0xD0FFFFFF

  Detailed recommended static allocation
  +-------------------------------------------------------------------------+
  | Size    | Start       | End         | Usage                             |
  |  16 MB  | 0xD0000000  | 0xD0FFFFFF  | SBREG                             |
  |   8 kB  | 0xD1000000  | 0xD1001FFF  | PMC BAR0                          |
  |   4 kB  | 0xD1002000  | 0xD1002FFF  | PMC BAR1                          |
//  |   4 kB  | 0xD1003000  | 0xD1003FFF  | SPI BAR0                          |
  |   8 kB  | 0xD1004000  | 0xD1005FFF  | PMC SSRAM BAR0                    |
  |   4 kB  | 0xD1006000  | 0xD1006FFF  | PMC SSRAM BAR1                    |
  |  88 kB  | 0xFE020000  | 0xFE035FFF  | SerialIo BAR in ACPI mode         |
  |   8 kB  | 0xFE036000  | 0xFE037FFF  | SCS SDIO BAR in ACPI mode         |
  | 256 kB  | 0xFE1C0000  | 0xFE1FFFFF  | TraceHub FW BAR                   |
  |   2 MB  | 0xFE200000  | 0xFE3FFFFF  | TraceHub SW BAR                   |
  |   2 MB  | 0xFE600000  | 0xFE7FFFFF  | Temp address                      |
  +-------------------------------------------------------------------------+
**/
#define SC_PRESERVED_BASE_ADDRESS      0xD0000000     ///< SC preserved MMIO base address
#define SC_PRESERVED_MMIO_SIZE         0x01000000     ///< 16MB
#define SC_PCR_BASE_ADDRESS            0xD0000000     ///< SBREG MMIO base address
#define SC_PCR_MMIO_SIZE               0x01000000     ///< 16MB
#define SC_SPI_BASE_ADDRESS            0xFED01000     ///< SPI BAR0 MMIO base address
#define SC_SPI_MMIO_SIZE               0x00001000     ///< 4KB
#define SC_SERIAL_IO_BASE_ADDRESS      0xFE020000     ///< SerialIo MMIO base address
#define SC_SERIAL_IO_MMIO_SIZE         0x00016000     ///< 88KB
#define SC_TRACE_HUB_SW_BASE_ADDRESS   0xFE200000     ///< TraceHub SW MMIO base address
#define SC_TRACE_HUB_SW_MMIO_SIZE      0x00040000     ///< 2MB

#endif // _SC_PRESERVED_RESOURCES_H_

