
/*++

Copyright (c)  1999 - 2016, Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  PlatformConfigDataGuid.h
    
Abstract:

  GUID used for Platform ConfigDataGuid.

--*/

#ifndef __PLATFORM_CONFIG_DATA_GUID_H__
#define __PLATFORM_CONFIG_DATA_GUID_H__

#define PLATFORM_CONFIG_DATA_GUID \
  {\
    0x3c029123, 0x2638, 0x43fc, 0x85, 0x7d, 0xc3, 0xab, 0xbc, 0x1f, 0x71, 0x6c \
  }

#define PLATFORM_SYSTEM_CONFIG_DATA_GUID \
  {\
    0xef927760, 0xa05d, 0x419c, 0x92, 0xe1, 0x12, 0x6b, 0xa9, 0x2d, 0x8c, 0xcd \
  }

#define PLATFORM_MEMORY_CONFIG_DATA_GUID \
  {\
    0xfce67798, 0xb347, 0x4c4c, 0xa1, 0xcf, 0xd, 0x85, 0x90, 0xcd, 0x25, 0xc6 \
  }

#define PLATFORM_SMIP_CONFIG_DATA_GUID \
  {\
    0x2bedcf0e, 0x633f, 0x4415, 0xb7, 0x57, 0xad, 0x11, 0x31, 0x79, 0x92, 0x61 \
  }

//[-start-160727-IB07250259-add]//
#define BOOT_VARIABLE_NV_DATA_GUID \
  {\
    0x8af4fafa, 0xfa9b, 0x43ea, 0x96, 0x13, 0xae, 0xb0, 0x44, 0x1f, 0x32, 0x32 \
  }
//[-end-160727-IB07250259-add]//

extern EFI_GUID gPlatformConfigDataGuid;
extern EFI_GUID gPlatformSystemConfigDataGuid;
extern EFI_GUID gPlatformMemoryConfigDataGuid;
extern EFI_GUID gPlatformSmipConfigDataGuid;
//[-start-160216-IB03090424-add]//
extern EFI_GUID gPlatformSystemConfigDataPreMemGuid;
//[-end-160216-IB03090424-add]//
//[-start-160727-IB07250259-add]//
extern EFI_GUID gBootVariableNvDataGuid;
//[-end-160727-IB07250259-add]//

#endif
