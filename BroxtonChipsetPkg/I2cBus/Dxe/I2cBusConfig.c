/** @file
  For I2C Bus Configuration Manager Protocol

;******************************************************************************
;* Copyright (c) 2012 - 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/


#include "I2cMasterDxe.h"

/**
  Enable access to an I2C bus configuration.

  This routine must be called at or below TPL_NOTIFY.  For synchronous
  requests this routine must be called at or below TPL_CALLBACK.

  Reconfigure the switches and multiplexers in the I2C bus to enable
  access to a specific I2C bus configuration.  Also select the maximum
  clock frequency for this I2C bus configuration.

  This routine uses the I2C Master protocol to perform I2C transactions
  on the local bus.  This eliminates any recursion in the I2C stack for
  configuration transactions on the same I2C bus.  This works because the
  local I2C bus is idle while the I2C bus configuration is being enabled.

  If I2C transactions must be performed on other I2C busses, then the
  EFI_I2C_HOST_PROTOCOL, the EFI_I2C_IO_PROTCOL, or a third party I2C
  driver interface for a specific device must be used.  This requirement
  is because the I2C host protocol controls the flow of requests to the
  I2C controller.  Use the EFI_I2C_HOST_PROTOCOL when the I2C device is
  not enumerated by the EFI_I2C_ENUMERATE_PROTOCOL.  Use a protocol
  produced by a third party driver when it is available or the
  EFI_I2C_IO_PROTOCOL when the third party driver is not available but
  the device is enumerated with the EFI_I2C_ENUMERATE_PROTOCOL.

  When Event is NULL, EnableI2cBusConfiguration operates synchronously
  and returns the I2C completion status as its return value.

  @param[in]  This            Pointer to an EFI_I2C_BUS_CONFIGURATION_MANAGEMENT_PROTOCOL
                              structure.
  @param[in]  I2cBusConfiguration Index of an I2C bus configuration.  All
                                  values in the range of zero to N-1 are
                                  valid where N is the total number of I2C
                                  bus configurations for an I2C bus.
  @param[in]  Event           Event to signal when the transaction is complete
  @param[out] I2cStatus       Buffer to receive the transaction status.

  @return  When Event is NULL, EnableI2cBusConfiguration operates synchrouously
  and returns the I2C completion status as its return value.  In this case it is
  recommended to use NULL for I2cStatus.  The values returned from
  EnableI2cBusConfiguration are:

  @retval EFI_SUCCESS           The asynchronous bus configuration request
                                was successfully started when Event is not
                                NULL.
  @retval EFI_SUCCESS           The bus configuration request completed
                                successfully when Event is NULL.
  @retval EFI_DEVICE_ERROR      The bus configuration failed.
  @retval EFI_NO_MAPPING        Invalid I2cBusConfiguration value

**/

EFI_STATUS
EFIAPI
I2cBusConfiguration (
  IN CONST EFI_I2C_BUS_CONFIGURATION_MANAGEMENT_PROTOCOL *This,
  IN UINTN                                               I2cBusConfiguration,
  IN EFI_EVENT                                           Event      OPTIONAL,
  IN EFI_STATUS                                          *I2cStatus OPTIONAL
  )
{
  I2C_BUS_INSTANCE            *I2cContext;  
  EFI_STATUS                  Status;
  UINTN                       BusSpeedArray[] = { 100000, 400000, 1000000};
  UINTN                       BusSpeedFreq;
  
  Status = EFI_SUCCESS;
  //
  //  Locate the enumeration context
  //
  I2cContext = I2C_PORT_CONTEXT_FROM_COFNIG_PROTOCOL (This);
  Status = EFI_SUCCESS;

  if (I2cBusConfiguration >= (sizeof (BusSpeedArray)/sizeof(UINTN))) {
    return EFI_NO_MAPPING;
  }

  BusSpeedFreq = BusSpeedArray[I2cBusConfiguration];
  Status = I2cContext->MasterApi.SetBusFrequency (&I2cContext->MasterApi, &BusSpeedFreq);

  if (EFI_ERROR(Status)) {
    return EFI_DEVICE_ERROR;
  }
  
  //
  //  Return the operation status
  //
  if ( NULL != I2cStatus ) {
    *I2cStatus = Status;
  }
  if ( NULL != Event ) {
    gBS->SignalEvent (Event);
  }
  return EFI_SUCCESS;;
}
