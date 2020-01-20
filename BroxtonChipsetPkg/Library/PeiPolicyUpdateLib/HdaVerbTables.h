/** @file

@copyright
 Copyright (c) 2015 - 2016 Intel Corporation. All rights reserved
 This software and associated documentation (if any) is furnished
 under a license and may only be used or copied in accordance
 with the terms of the license. Except as permitted by the
 license, no part of this software or documentation may be
 reproduced, stored in a retrieval system, or transmitted in any
 form or by any means without the express written consent of
 Intel Corporation.
 This file contains a 'Sample Driver' and is licensed as such
 under the terms of your license agreement with Intel or your
 vendor. This file may be modified by the user, subject to
 the additional terms of the license agreement.

@par Specification Reference:
**/

#ifndef _HDA_VERBTABLES_H_
#define _HDA_VERBTABLES_H_

//[-start-160317-IB03090425-modify]//
#include <Ppi/ScPolicy.h>
//[-end-160317-IB03090425-modify]//

enum HDAUDIO_CODEC_SELECT {
  HdaCodecPlatformOnboard = 0,
  HdaCodecExternalKit     = 1
};

//[-start-151228-IB03090424-modify]//
extern HDAUDIO_VERB_TABLE  HdaVerbTableAlc298;
//[-start-160406-IB07400715-modify]//
//#if (BXTI_PF_ENABLE == 1)
extern HDAUDIO_VERB_TABLE  HdaVerbTableAlc662;
//#endif
//[-end-160406-IB07400715-modify]//
//[-end-151228-IB03090424-modify]//

#endif // _HDA_VERBTABLES_H_
