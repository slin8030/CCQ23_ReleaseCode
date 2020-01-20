/** 
;******************************************************************************
;* Copyright (c) 2014 - 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _SEG_CHIPSET_FEATURE_SETUP_CONFIG_H_
#define _SEG_CHIPSET_FEATURE_SETUP_CONFIG_H_

//[-start-160821-IB07400772-modify]//
#ifdef VBT_HOOK_SUPPORT
#include <VbtHookSetupConfig.h>
#endif
//[-end-160821-IB07400772-modify]//

//
// Form IDs
//
#define VBT_HOOK_FORM_ID                    100
#define SEG_CHIPSET_FEATURE_FORM_ID         101

//
// KEY numbers
//
#define KEY_WIN7_KB_MS_SUPPORT              0x5001

//
// Setup Value
//
//[-start-170314-IB07400847-add]//
#define SCU_S5_WAKE_ON_RTC_DISABLED           0x00
#define SCU_S5_WAKE_ON_RTC_BY_EVERY_DAY       0x01
#define SCU_S5_WAKE_ON_RTC_BY_DAY_OF_MONTH    0x02
#define SCU_S5_WAKE_ON_RTC_BY_SLEEP_TIME      0x03
#define SCU_S5_WAKE_ON_RTC_BY_OS_UTILITY      0x04
//[-end-170314-IB07400847-add]//

//[-start-171115-IB07400928-add]//
#define SCU_WAKE_ON_PME_DISABLED              0x00
#define SCU_WAKE_ON_PME_ENABLED_BY_OS         0x01
#define SCU_WAKE_ON_PME_FORCE_ENALBED         0x02
//[-end-171115-IB07400928-add]//

#endif
