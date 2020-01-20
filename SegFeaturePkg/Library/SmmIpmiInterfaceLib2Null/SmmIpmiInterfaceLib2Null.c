/** @file
 H2O IPMI Interface library implement code.

 This c file contains H2O IPMI interface library instance for SMM phase.

;******************************************************************************
;* Copyright (c) 2013 - 2014, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************

*/

/**
 Send request, include Network Function, LUN, and command of IPMI, with/without
 additional data to BMC.

 @param[in]        NetFn                Network Function of the IPMI command want to send.
 @param[in]        Cmd                  Command code of the IPMI command want to send.
 @param[in]        SendData             Optional arguments, if an IPMI command is required to
                                        send with data, this argument is a pointer to the data buffer.
                                        If no data is required, set this argument as NULL.
 @param[in]        SendLength           When sending command with data, this argument is the length of the data,
                                        otherwise set this argument as 0.
 @param[out]       RecvData             Data buffer to put the data read from BMC.
 @param[in out]    RecvLength           in : Length of RecvData
                                        out : Length of Data readed from BMC.
 @param[in]        SendToHeci           This command send to HECI or not(BMC)

 @retval EFI_UNSUPPORTED                Function Unsupported.
*/
EFI_STATUS
EFIAPI
IpmiLibExecuteIpmiCmd2 (
  IN  UINT8                             NetFn,
  IN  UINT8                             Cmd,
  IN  VOID                              *SendData OPTIONAL,
  IN  UINT8                             SendLength,
  OUT VOID                              *RecvData,
//[-start-171228-IB09330412-modify]//
  IN OUT UINT8                          *RecvLength,
//[-end-171228-IB09330412-modify]//
  IN  BOOLEAN                           SendToHeci
  )
{
  return EFI_UNSUPPORTED;
}

