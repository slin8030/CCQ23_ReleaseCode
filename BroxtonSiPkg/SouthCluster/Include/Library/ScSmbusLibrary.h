/** @file
  Header file for SC Smbus Lib.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2011 - 2016 Intel Corporation.

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

@par Specification
**/
#ifndef _SMBUS_LIBRARY_H_
#define _SMBUS_LIBRARY_H_

/**
  This function provides a standard way to execute Smbus sequential
  I2C Read. This function allows the SC to perform block reads to
  certain I2C devices, such as serial E2PROMs. Typically these data
  bytes correspond to an offset (address) within the serial memory
  chips.

  @param[in] SmBusAddress         Address that encodes the SMBUS Slave Address,
                                  SMBUS Command, SMBUS Data Length, and PEC.
  @param[out] Buffer              Pointer to the buffer to store the bytes read
                                  from the SMBUS
  @param[out] Status              eturn status for the executed command.

  @retval UINTN                   The number of bytes read
**/
UINTN
EFIAPI
SmBusSeqI2CRead (
  IN  UINTN          SmBusAddress,
  OUT VOID           *Buffer,
  OUT RETURN_STATUS  * Status OPTIONAL
  );
#endif
