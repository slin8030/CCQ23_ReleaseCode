/** @file
  GUIDs used as HII FormSet and HII Package list GUID in SecureBootConfigDxe driver. 
  
Copyright (c) 2011, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials are licensed and made available under 
the terms and conditions of the BSD License that accompanies this distribution.  
The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php.                                            

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __SIO_NCT5104D_HII_SETUP00_GUID_H__
#define __SIO_NCT5104D_HII_SETUP00_GUID_H__

#define SIO_NCT5104D_SETUP00_FORMSET_GUID \
  { \
    0x34a07178, 0x1a48, 0x4a62, { 0xa4, 0xf2, 0x91, 0xf5, 0xc9, 0x6f, 0x76, 0x7b } \
  }

extern EFI_GUID gSioNct5104dSetup00FormSetGuid;

#endif
