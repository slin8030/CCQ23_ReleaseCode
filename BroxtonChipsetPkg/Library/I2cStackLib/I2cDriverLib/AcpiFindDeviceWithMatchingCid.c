/** @file
  Locate an ACPI device path node at the end of a device path

  Various drivers and applications use this API to locate the
  device that they are interested in manipulating.

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

  This file contains a 'Sample Driver' and is licensed as such under the terms
  of your license agreement with Intel or your vendor. This file may be modified
  by the user, subject to the additional terms of the license agreement.

@par Specification
**/

#include <Library/DriverLib.h>
#include <Library/DevicePathLib.h>

/**
  Locate a matching ACPI device path node

  This routine walks the device path attached to the ControllerHandle
  and determines if the last (non-end) device path node is an
  ACPI_HID_DEVICE_PATH node and if the CID or _CIDSTR values
  match the specified values.

  @param [in] CompatibleIdentification  The value to match against the CID
                                        value in the ACPI_HID_DEVICE_PATH
                                        node.  This value must be zero when
                                        the CompatibleIdentification
                                        value is not NULL.
  @param [in] CompatibleIdentificationString  This value is specified as NULL
                                              when the CompatibleIdentification
                                              value is non-zero.  When the
                                              CompatibleIdentification value is
                                              zero (0), this value should point
                                              to a zero terminated charater
                                              string value.

  @return           When the ACPI device path node is found, this routine
                    returns the pointer to the ACPI_HID_DEVICE_PATH node.
                    Otherwise when the device path is not found this routine
                    returns NULL.

**/
CONST ACPI_HID_DEVICE_PATH *
EFIAPI
DlAcpiFindDeviceWithMatchingCid (
  EFI_HANDLE ControllerHandle,
  UINTN CompatibleIdentification,
  CONST CHAR8 * CompatibleIdentificationString OPTIONAL
  )
{
  ACPI_EXTENDED_HID_DEVICE_PATH *AcpiPath;
  CHAR8 *CidStr;
  EFI_DEVICE_PATH_PROTOCOL *DevicePath;
  EFI_DEVICE_PATH_PROTOCOL *EndPath;

  //
  //  Locate the last node in the device path
  //
  EndPath = NULL;
  DevicePath = DevicePathFromHandle ( ControllerHandle );
  if ( NULL == DevicePath ) {
    DEBUG (( DEBUG_INFO, "INFO - No device path for this controller!\r\n" ));
  }
  else {
    EndPath = DevicePath;
    while ( !IsDevicePathEnd ( EndPath )) {
      //
      //  Skip this portion of the device path
      //
      DevicePath = EndPath;
      if ( sizeof ( *DevicePath ) > DevicePathNodeLength ( EndPath )) {
        //
        //  Invalid device path node
        //
        DEBUG (( DEBUG_ERROR,
                  "ERROR - Invalid device path found at 0x%016Lx\r\n",
                  (UINT64)(UINTN) EndPath ));
        return NULL;
      }
      EndPath = NextDevicePathNode ( EndPath );
    }

    //
    //  Determine if this device is supported
    //
    EndPath = NULL;
    AcpiPath = (ACPI_EXTENDED_HID_DEVICE_PATH *)DevicePath;
    if ( ACPI_DEVICE_PATH != DevicePathType ( DevicePath )) {
        DEBUG (( DEBUG_INFO, "INFO - Not an ACPI device path node!\r\n" ));
    }
    else {
      if ( ACPI_EXTENDED_DP != DevicePathSubType ( DevicePath )) {
        DEBUG (( DEBUG_INFO, "INFO - ACPI device path node does not include the CID field!\r\n" ));
      }
      else {
        if ( AcpiPath->CID != CompatibleIdentification ) {
          DEBUG (( DEBUG_INFO,
                    "INFO - The CID value (%d) does not match %d!\r\n",
                    AcpiPath->CID,
                    CompatibleIdentification ));
        }
        else {
          if ( 0 != AcpiPath->CID ) {
            DEBUG (( DEBUG_INFO, "INFO - Found device using CID value\r\n" ));
            EndPath = DevicePath;
          }
          else {
            if ( NULL != CompatibleIdentificationString ) {
              //
              //  Skip over the HID
              //
              CidStr = (CHAR8*)( AcpiPath + 1 );
              CidStr += AsciiStrLen ( CidStr ) + 1;

              //
              //  Skip over the UID
              //
              CidStr += AsciiStrLen ( CidStr ) + 1;

              //
              //  Validate the CID
              //
              if ( 0 == AsciiStrCmp ( CidStr, CompatibleIdentificationString )) {
                DEBUG (( DEBUG_INFO, "INFO - Found device using _CIDSTR value\r\n" ));
                EndPath = DevicePath;
              }
              else {
                DEBUG (( DEBUG_INFO,
                          "INFO - _CIDSTR value (%a) does not match %a!\r\n",
                          CidStr,
                          CompatibleIdentificationString ));
              }
            }
            else {
              DEBUG (( DEBUG_ERROR, "ERROR - CompatibleIdentificationString must be non-NULL for comparison!\r\n" ));
            }
          }
        }
      }
    }
  }

  //
  //  The ACPI device path node that was found
  //
  return (ACPI_HID_DEVICE_PATH *)EndPath;
}
