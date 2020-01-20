/** @file

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2015 - 2016 Intel Corporation.

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

#include "MemOdt.h"
#include "BxtOdtValueTable.h"

#if ODT_TABLE

ODT_VALUE_STRUCT OdtValueTable[MAX_ODT_VALUE_CONFIG] = {
{
  //
  // Config index
  //
  (DDR3_800 << 0)       | // freq
  (SR_DIMM << 7)        | // slot0
  (EMPTY_DIMM << 10)    | // slot1
  (EMPTY_DIMM << 13),     // slot2
  MCODT_120,
  {
    {
      //
      // dimm0
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_INF << 4)   | RTT_NOM_120,         // rank0
      (RTT_WR_DIS << 4)   | RTT_NOM_DIS          // rank1
    },
    {
      //
      // dimm1
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_DIS << 4)   | RTT_NOM_DIS,         // rank0
      (RTT_WR_DIS << 4)   | RTT_NOM_DIS          // rank1
    }
  }
},
{
  //
  // Config index
  //
  (DDR3_1066 << 0)      | // freq
  (SR_DIMM << 7)        | // slot0
  (EMPTY_DIMM << 10)    | // slot1
  (EMPTY_DIMM << 13),     // slot2
  MCODT_60,
  {
    {
      //
      // dimm0
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_INF << 4)   | RTT_NOM_60,          // rank0
      (RTT_WR_DIS << 4)   | RTT_NOM_DIS          // rank1
    },
    {
      //
      // dimm1
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_DIS << 4)   | RTT_NOM_DIS,         // rank0
      (RTT_WR_DIS << 4)   | RTT_NOM_DIS          // rank1
    }
  }
},
{
  //
  // Config index
  //
  (DDR3_1333 << 0)      | // freq
  (SR_DIMM << 7)        | // slot0
  (EMPTY_DIMM << 10)    | // slot1
  (EMPTY_DIMM << 13),     // slot2
  MCODT_60,
  {
    {
      //
      // dimm0
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_INF << 4)   | RTT_NOM_60,          // rank0
      (RTT_WR_DIS << 4)   | RTT_NOM_DIS          // rank1
    },
    {
      //
      // dimm1
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_DIS << 4)   | RTT_NOM_DIS,         // rank0
      (RTT_WR_DIS << 4)   | RTT_NOM_DIS          // rank1
    }
  }
},
{
  //
  // Config index
  //
  (DDR3_1600 << 0)      | // freq
  (SR_DIMM << 7)        | // slot0
  (EMPTY_DIMM << 10)    | // slot1
  (EMPTY_DIMM << 13),     // slot2
  MCODT_60,
  {
    {
      //
      // dimm0
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_INF << 4)   | RTT_NOM_60,          // rank0
      (RTT_WR_DIS << 4)   | RTT_NOM_DIS          // rank1
    },
    {
      //
      // dimm1
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_DIS << 4)   | RTT_NOM_DIS,         // rank0
      (RTT_WR_DIS << 4)   | RTT_NOM_DIS          // rank1
    }
  }
},
{
  //
  // Config index
  //
  (DDR3_1866 << 0)      | // freq
  (SR_DIMM << 7)        | // slot0
  (EMPTY_DIMM << 10)    | // slot1
  (EMPTY_DIMM << 13),     // slot2
  MCODT_60,
  {
    {
      //
      // dimm0
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_INF << 4)   | RTT_NOM_40,          // rank0
      (RTT_WR_DIS << 4)   | RTT_NOM_DIS          // rank1
    },
    {
      //
      // dimm1
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_DIS << 4)   | RTT_NOM_DIS,         // rank0
      (RTT_WR_DIS << 4)   | RTT_NOM_DIS          // rank1
    }
  }
},
{
  //
  // Config index
  //
  (DDR3_800 << 0)       | // freq
  (DR_DIMM << 7)        | // slot0
  (EMPTY_DIMM << 10)    | // slot1
  (EMPTY_DIMM << 13),     // slot2
  MCODT_120,
  {
    {
      //
      // dimm0
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_INF << 4)   | RTT_NOM_120,         // rank0
      (RTT_WR_INF << 4)   | RTT_NOM_120          // rank1
    },
    {
      //
      // dimm1
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_DIS << 4)   | RTT_NOM_DIS,         // rank0
      (RTT_WR_DIS << 4)   | RTT_NOM_DIS          // rank1
    }
  }
},
{
  //
  // Config index
  //
  (DDR3_1066 << 0)      | // freq
  (DR_DIMM << 7)        | // slot0
  (EMPTY_DIMM << 10)    | // slot1
  (EMPTY_DIMM << 13),     // slot2
  MCODT_60,
  {
    {
      //
      // dimm0
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_INF << 4)   | RTT_NOM_60,          // rank0
      (RTT_WR_INF << 4)   | RTT_NOM_60           // rank1
    },
    {
      //
      // dimm1
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_DIS << 4)   | RTT_NOM_DIS,         // rank0
      (RTT_WR_DIS << 4)   | RTT_NOM_DIS          // rank1
    }
  }
},
{
  //
  // Config index
  //
  (DDR3_1333 << 0)      | // freq
  (DR_DIMM << 7)        | // slot0
  (EMPTY_DIMM << 10)    | // slot1
  (EMPTY_DIMM << 13),     // slot2
  MCODT_60,
  {
    {
      //
      // dimm0
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_INF << 4)   | RTT_NOM_60,          // rank0
      (RTT_WR_INF << 4)   | RTT_NOM_60           // rank1
    },
    {
      //
      // dimm1
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_DIS << 4)   | RTT_NOM_DIS,         // rank0
      (RTT_WR_DIS << 4)   | RTT_NOM_DIS          // rank1
    }
  }
},
{
  //
  // Config index
  //
  (DDR3_1600 << 0)      | // freq
  (DR_DIMM << 7)        | // slot0
  (EMPTY_DIMM << 10)    | // slot1
  (EMPTY_DIMM << 13),     // slot2
  MCODT_60,
  {
    {
      //
      // dimm0
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_INF << 4)   | RTT_NOM_60,          // rank0
      (RTT_WR_INF << 4)   | RTT_NOM_60           // rank1
    },
    {
      //
      // dimm1
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_DIS << 4)   | RTT_NOM_DIS,         // rank0
      (RTT_WR_DIS << 4)   | RTT_NOM_DIS          // rank1
    }
  }
},
{
  //
  // Config index
  //
  (DDR3_1866 << 0)      | // freq
  (DR_DIMM << 7)        | // slot0
  (EMPTY_DIMM << 10)    | // slot1
  (EMPTY_DIMM << 13),     // slot2
  MCODT_60,
  {
    {
      //
      // dimm0
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_INF << 4)   | RTT_NOM_40,          // rank0
      (RTT_WR_INF << 4)   | RTT_NOM_40           // rank1
    },
    {
      //
      // dimm1
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_DIS << 4)   | RTT_NOM_DIS,         // rank0
      (RTT_WR_DIS << 4)   | RTT_NOM_DIS          // rank1
    }
  }
},
{
  //
  // Config index
  //
  (DDR3_800 << 0)       | // freq
  (SR_DIMM << 7)        | // slot0
  (SR_DIMM << 10)       | // slot1
  (EMPTY_DIMM << 13),     // slot2
  MCODT_120,
  {
    {
      //
      // dimm0
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_120 << 4)   | RTT_NOM_40,          // rank0
      (RTT_WR_DIS << 4)   | RTT_NOM_DIS          // rank1
    },
    {
      //
      // dimm1
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_120 << 4)   | RTT_NOM_40,          // rank0
      (RTT_WR_DIS << 4)   | RTT_NOM_DIS          // rank1
    }
  }
},
{
  //
  // Config index
  //
  (DDR3_1066 << 0)      | // freq
  (SR_DIMM << 7)        | // slot0
  (SR_DIMM << 10)       | // slot1
  (EMPTY_DIMM << 13),     // slot2
  MCODT_60,
  {
    {
      //
      // dimm0
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_120 << 4)   | RTT_NOM_30,          // rank0
      (RTT_WR_DIS << 4)   | RTT_NOM_DIS          // rank1
    },
    {
      //
      // dimm1
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_120 << 4)   | RTT_NOM_30,          // rank0
      (RTT_WR_DIS << 4)   | RTT_NOM_DIS          // rank1
    }
  }
},
{
  //
  // Config index
  //
  (DDR3_1333 << 0)      | // freq
  (SR_DIMM << 7)        | // slot0
  (SR_DIMM << 10)       | // slot1
  (EMPTY_DIMM << 13),     // slot2
  MCODT_60,
  {
    {
      //
      // dimm0
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_60 << 4)   | RTT_NOM_30,           // rank0
      (RTT_WR_DIS << 4)   | RTT_NOM_DIS          // rank1
    },
    {
      //
      // dimm1
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_60 << 4)   | RTT_NOM_30,           // rank0
      (RTT_WR_DIS << 4)   | RTT_NOM_DIS          // rank1
    }
  }
},
{
  //
  // Config index
  //
  (DDR3_1600 << 0)      | // freq
  (SR_DIMM << 7)        | // slot0
  (SR_DIMM << 10)       | // slot1
  (EMPTY_DIMM << 13),     // slot2
  MCODT_60,
  {
    {
      //
      // dimm0
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_60 << 4)   | RTT_NOM_30,           // rank0
      (RTT_WR_DIS << 4)   | RTT_NOM_DIS          // rank1
    },
    {
      //
      // dimm1
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_60 << 4)   | RTT_NOM_30,           // rank0
      (RTT_WR_DIS << 4)   | RTT_NOM_DIS          // rank1
    }
  }
},
{
  //
  // Config index
  //
  (DDR3_1866 << 0)      | // freq
  (SR_DIMM << 7)        | // slot0
  (SR_DIMM << 10)       | // slot1
  (EMPTY_DIMM << 13),     // slot2
  MCODT_60,
  {
    {
      //
      // dimm0
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_60 << 4)   | RTT_NOM_30,           // rank0
      (RTT_WR_DIS << 4)   | RTT_NOM_DIS          // rank1
    },
    {
      //
      // dimm1
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_60 << 4)   | RTT_NOM_30,           // rank0
      (RTT_WR_DIS << 4)   | RTT_NOM_DIS          // rank1
    }
  }
},
{
  //
  // Config index
  //
  (DDR3_800 << 0)       | // freq
  (DR_DIMM << 7)        | // slot0
  (DR_DIMM << 10)       | // slot1
  (EMPTY_DIMM << 13),     // slot2
  MCODT_120,
  {
    {
      //
      // dimm0
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_120 << 4)   | RTT_NOM_40,          // rank0
      (RTT_WR_120 << 4)   | RTT_NOM_INF          // rank1
    },
    {
      //
      // dimm1
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_120 << 4)   | RTT_NOM_40,          // rank0
      (RTT_WR_120 << 4)   | RTT_NOM_INF          // rank1
    }
  }
},
{
  //
  // Config index
  //
  (DDR3_1066 << 0)      | // freq
  (DR_DIMM << 7)        | // slot0
  (DR_DIMM << 10)       | // slot1
  (EMPTY_DIMM << 13),     // slot2
  MCODT_60,
  {
    {
      //
      // dimm0
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_120 << 4)   | RTT_NOM_30,          // rank0
      (RTT_WR_120 << 4)   | RTT_NOM_INF          // rank1
    },
    {
      //
      // dimm1
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_120 << 4)   | RTT_NOM_30,          // rank0
      (RTT_WR_120 << 4)   | RTT_NOM_INF          // rank1
    }
  }
},
{
  //
  // Config index
  //
  (DDR3_1333 << 0)      | // freq
  (DR_DIMM << 7)        | // slot0
  (DR_DIMM << 10)       | // slot1
  (EMPTY_DIMM << 13),     // slot2
  MCODT_60,
  {
    {
      //
      // dimm0
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_60 << 4)   | RTT_NOM_30,           // rank0
      (RTT_WR_60 << 4)   | RTT_NOM_INF           // rank1
    },
    {
      //
      // dimm1
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_60 << 4)   | RTT_NOM_30,           // rank0
      (RTT_WR_60 << 4)   | RTT_NOM_INF           // rank1
    }
  }
},
{
  //
  // Config index
  //
  (DDR3_1600 << 0)      | // freq
  (DR_DIMM << 7)        | // slot0
  (DR_DIMM << 10)       | // slot1
  (EMPTY_DIMM << 13),     // slot2
  MCODT_60,
  {
    {
      //
      // dimm0
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_60 << 4)   | RTT_NOM_30,           // rank0
      (RTT_WR_60 << 4)   | RTT_NOM_INF           // rank1
    },
    {
      //
      // dimm1
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_60 << 4)   | RTT_NOM_30,           // rank0
      (RTT_WR_60 << 4)   | RTT_NOM_INF           // rank1
    }
  }
},
{
  //
  // Config index
  //
  (DDR3_1866 << 0)      | // freq
  (DR_DIMM << 7)        | // slot0
  (DR_DIMM << 10)       | // slot1
  (EMPTY_DIMM << 13),     // slot2
  MCODT_60,
  {
    {
      //
      // dimm0
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_60 << 4)   | RTT_NOM_30,           // rank0
      (RTT_WR_60 << 4)   | RTT_NOM_INF           // rank1
    },
    {
      //
      // dimm1
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_60 << 4)   | RTT_NOM_30,           // rank0
      (RTT_WR_60 << 4)   | RTT_NOM_INF           // rank1
    }
  }
},
{
  //
  // Config index
  //
  (DDR3_800 << 0)       | // freq
  (DR_DIMM << 7)        | // slot0
  (SR_DIMM << 10)       | // slot1
  (EMPTY_DIMM << 13),     // slot2
  MCODT_120,
  {
    {
      //
      // dimm0
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_120 << 4)   | RTT_NOM_40,          // rank0
      (RTT_WR_120 << 4)   | RTT_NOM_INF          // rank1
    },
    {
      //
      // dimm1
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_120 << 4)   | RTT_NOM_40,          // rank0
      (RTT_WR_DIS << 4)   | RTT_NOM_DIS          // rank1
    }
  }
},
{
  //
  // Config index
  //
  (DDR3_1066 << 0)      | // freq
  (DR_DIMM << 7)        | // slot0
  (SR_DIMM << 10)       | // slot1
  (EMPTY_DIMM << 13),     // slot2
  MCODT_60,
  {
    {
      //
      // dimm0
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_120 << 4)   | RTT_NOM_30,          // rank0
      (RTT_WR_120 << 4)   | RTT_NOM_INF          // rank1
    },
    {
      //
      // dimm1
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_120 << 4)   | RTT_NOM_30,          // rank0
      (RTT_WR_DIS << 4)   | RTT_NOM_DIS          // rank1
    }
  }
},
{
  //
  // Config index
  //
  (DDR3_1333 << 0)      | // freq
  (DR_DIMM << 7)        | // slot0
  (SR_DIMM << 10)       | // slot1
  (EMPTY_DIMM << 13),     // slot2
  MCODT_60,
  {
    {
      //
      // dimm0
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_60 << 4)   | RTT_NOM_30,           // rank0
      (RTT_WR_60 << 4)   | RTT_NOM_INF           // rank1
    },
    {
      //
      // dimm1
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_60 << 4)   | RTT_NOM_30,           // rank0
      (RTT_WR_DIS << 4)   | RTT_NOM_DIS          // rank1
    }
  }
},
{
  //
  // Config index
  //
  (DDR3_1600 << 0)      | // freq
  (DR_DIMM << 7)        | // slot0
  (SR_DIMM << 10)       | // slot1
  (EMPTY_DIMM << 13),     // slot2
  MCODT_60,
  {
    {
      //
      // dimm0
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_60 << 4)   | RTT_NOM_30,           // rank0
      (RTT_WR_60 << 4)   | RTT_NOM_INF           // rank1
    },
    {
      //
      // dimm1
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_60 << 4)   | RTT_NOM_30,           // rank0
      (RTT_WR_DIS << 4)   | RTT_NOM_DIS          // rank1
    }
  }
},
{
  //
  // Config index
  //
  (DDR3_1866 << 0)      | // freq
  (DR_DIMM << 7)        | // slot0
  (SR_DIMM << 10)       | // slot1
  (EMPTY_DIMM << 13),     // slot2
  MCODT_60,
  {
    {
      //
      // dimm0
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_60 << 4)   | RTT_NOM_30,           // rank0
      (RTT_WR_60 << 4)   | RTT_NOM_INF           // rank1
    },
    {
      //
      // dimm1
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_60 << 4)   | RTT_NOM_30,           // rank0
      (RTT_WR_DIS << 4)   | RTT_NOM_DIS          // rank1
    }
  }
},
{
  //
  // Config index
  //
  (DDR3_800 << 0)       | // freq
  (SR_DIMM << 7)        | // slot0
  (DR_DIMM << 10)       | // slot1
  (EMPTY_DIMM << 13),     // slot2
  MCODT_120,
  {
    {
      //
      // dimm0
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_120 << 4)   | RTT_NOM_40,          // rank0
      (RTT_WR_DIS << 4)   | RTT_NOM_DIS          // rank1
    },
    {
      //
      // dimm1
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_120 << 4)   | RTT_NOM_40,          // rank0
      (RTT_WR_120 << 4)   | RTT_NOM_INF          // rank1
    }
  }
},
{
  //
  // Config index
  //
  (DDR3_1066 << 0)      | // freq
  (SR_DIMM << 7)        | // slot0
  (DR_DIMM << 10)       | // slot1
  (EMPTY_DIMM << 13),     // slot2
  MCODT_60,
  {
    {
      //
      // dimm0
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_120 << 4)   | RTT_NOM_30,          // rank0
      (RTT_WR_DIS << 4)   | RTT_NOM_DIS          // rank1
    },
    {
      //
      // dimm1
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_120 << 4)   | RTT_NOM_30,          // rank0
      (RTT_WR_120 << 4)   | RTT_NOM_INF          // rank1
    }
  }
},
{
  //
  // Config index
  //
  (DDR3_1333 << 0)      | // freq
  (SR_DIMM << 7)        | // slot0
  (DR_DIMM << 10)       | // slot1
  (EMPTY_DIMM << 13),     // slot2
  MCODT_60,
  {
    {
      //
      // dimm0
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_60 << 4)   | RTT_NOM_30,           // rank0
      (RTT_WR_DIS << 4)   | RTT_NOM_DIS          // rank1
    },
    {
      //
      // dimm1
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_60 << 4)   | RTT_NOM_30,           // rank0
      (RTT_WR_60 << 4)   | RTT_NOM_INF           // rank1
    }
  }
},
{
  //
  // Config index
  //
  (DDR3_1600 << 0)      | // freq
  (SR_DIMM << 7)        | // slot0
  (DR_DIMM << 10)       | // slot1
  (EMPTY_DIMM << 13),     // slot2
  MCODT_60,
  {
    {
      //
      // dimm0
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_60 << 4)   | RTT_NOM_30,           // rank0
      (RTT_WR_DIS << 4)   | RTT_NOM_DIS          // rank1
    },
    {
      //
      // dimm1
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_60 << 4)   | RTT_NOM_30,           // rank0
      (RTT_WR_60 << 4)   | RTT_NOM_INF           // rank1
    }
  }
},
{
  //
  // Config index
  //
  (DDR3_1866 << 0)      | // freq
  (SR_DIMM << 7)        | // slot0
  (DR_DIMM << 10)       | // slot1
  (EMPTY_DIMM << 13),     // slot2
  MCODT_60,
  {
    {
      //
      // dimm0
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_60 << 4)   | RTT_NOM_30,           // rank0
      (RTT_WR_DIS << 4)   | RTT_NOM_DIS          // rank1
    },
    {
      //
      // dimm1
      // Rtt_wr           Rtt_nom
      //
      (RTT_WR_60 << 4)   | RTT_NOM_30,           // rank0
      (RTT_WR_60 << 4)   | RTT_NOM_INF           // rank1
    }
  }
},
};

#endif // ODT_TABLE
