//;******************************************************************************
//;* Copyright (c) 1983-2006, Insyde Software Corporation. All Rights Reserved.
//;*
//;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
//;* transmit, broadcast, present, recite, release, license or otherwise exploit
//;* any part of this publication in any form, by any means, without the prior
//;* written permission of Insyde Software Corporation.
//;*
//;
//; Abstract:  ver 2
//;

#ifndef _UTILITY_SAMPLE_H_
#define _UTILITY_SAMPLE_H_

#include <BiosSet.h>

#define BIOS_SETTING_CMOS             SIGNATURE_32 ('S', 'M', 'P', 'G')

typedef struct {
  BIOS_SETTING_HEADER     Header;
  UINT8                   FIR1;
  UINT8                   FIR2;
  UINT8                   FIR3;
} BIOS_SETTING_SAMPLE;

#define Samp_PARM            L"-SAMP"
#define Samp_HELP_STRING     L"    -SAMP :sample command.\n"
#define Samp_HELP_EXAMPLE    L"           Example: BiosSet.efi -SAMP 1\n"

EFI_STATUS
Sample (
  LIST_ENTRY                *List
  );

#endif
