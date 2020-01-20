/** @file
 H2ODebugCfgToErrLevelLib is to make the translation between EFI DEBUG print error
 lever and Debug Config utility of Insyde Event Log feature.

;******************************************************************************
;* Copyright (c) 2015 - 2018, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************

*/

//[-start-160218-IB08400332-add]//
#include <Uefi.h>
#include <Library/DebugLib.h>
#include <Guid/DebugMessageConfigUtilHii.h>
//[-end-160218-IB08400332-add]//

//[-start-160219-IB08400332-modify]//
/**
  Translate the Debug Message configuration to Debug Print Error Level value.
  
  @param[in]   DebugMsgCfg         Debug config setting.
  
  @retval      DebugPrintErrLevel  The debug print error level value.

**/
UINT32
EFIAPI
DebugCfgToErrLevel (
  IN VOID   *DebugCfg
  )
//[-end-160219-IB08400332-modify]//
{
  UINT32                DebugPrintErrLevel = 0;
//[-start-160218-IB08400332-add]//
  DEBUG_MESSAGE_CONFIG_UTIL_CONFIGURATION  *DebugMsgCfg;
//[-end-160218-IB08400332-add]//

//[-start-160218-IB08400332-modify]//
  DebugMsgCfg = (DEBUG_MESSAGE_CONFIG_UTIL_CONFIGURATION*)DebugCfg;
  if (DebugMsgCfg->EfiDebugMsgAdvancedMode) {
    if (DebugMsgCfg->DebugInit) {
      DebugPrintErrLevel |= DEBUG_INIT;
    } 
    if (DebugMsgCfg->DebugWarn) {
      DebugPrintErrLevel |= DEBUG_WARN;
    } 
    if (DebugMsgCfg->DebugLoad) {
      DebugPrintErrLevel |= DEBUG_LOAD;
    } 
    if (DebugMsgCfg->DebugFs) {
      DebugPrintErrLevel |= DEBUG_FS;
    } 
    if (DebugMsgCfg->DebugInfo) {
      DebugPrintErrLevel |= DEBUG_INFO;
    } 
    if (DebugMsgCfg->DebugDispatch) {
      DebugPrintErrLevel |= DEBUG_DISPATCH;
    } 
    if (DebugMsgCfg->DebugVariable) {
      DebugPrintErrLevel |= DEBUG_VARIABLE;
    } 
    if (DebugMsgCfg->DebugBm) {
      DebugPrintErrLevel |= DEBUG_BM;
    } 
    if (DebugMsgCfg->DebugBlkio) {
      DebugPrintErrLevel |= DEBUG_BLKIO;
    } 
    if (DebugMsgCfg->DebugNet) {
      DebugPrintErrLevel |= DEBUG_NET;
    } 
    if (DebugMsgCfg->DebugUndi) {
      DebugPrintErrLevel |= DEBUG_UNDI;
    } 
    if (DebugMsgCfg->DebugLoadFile) {
      DebugPrintErrLevel |= DEBUG_LOADFILE;
    } 
    if (DebugMsgCfg->DebugEvent) {
      DebugPrintErrLevel |= DEBUG_EVENT;
    } 
    if (DebugMsgCfg->DebugGcd) {
      DebugPrintErrLevel |= DEBUG_GCD;
    } 
    if (DebugMsgCfg->DebugCache) {
      DebugPrintErrLevel |= DEBUG_CACHE;
    } 
    if (DebugMsgCfg->DebugVerbose) {
      DebugPrintErrLevel |= DEBUG_VERBOSE;
    } 
    if (DebugMsgCfg->DebugError) {
      DebugPrintErrLevel |= DEBUG_ERROR;
    } 
    
  } else {
    switch (DebugMsgCfg->EfiDebugMsgLevel) {
    case 0: // Default setting
//[-start-180518-IB08400596-modify]//
      DebugPrintErrLevel = PcdGet32 (PcdDebugPrintErrorLevel);
//[-end-180518-IB08400596-modify]//
      break;
    case 1: // Information
      DebugPrintErrLevel = (DEBUG_INFO);
      break;
    case 2: // Error
      DebugPrintErrLevel = (DEBUG_ERROR);
      break;
    case 3: // Warnings
      DebugPrintErrLevel = (DEBUG_WARN);
      break;
    case 4: // Verbose
      DebugPrintErrLevel = 0xFFFFFFCF; // Workaround: remove DEBUG_POOL and DEBUG_PAGE to avoid IntelFrameworkPkg DebugLib issue.
      break;
    case 0xFF: // Disable 
    default:
      DebugPrintErrLevel = 0;
      break;
    }
  }
//[-end-160218-IB08400332-modify]//

  return DebugPrintErrLevel;
}

/**
  Translate the Debug Print Error Level value to Debug Message configuration.
  
  @param[in]     DebugPrintErrLevel  The debug print error level value.
  @param[in,out] DebugCfg            Debug config setting.  
  @retval      

**/
//[-start-160219-IB08400332-modify]//
VOID
EFIAPI
ErrLevelToDebugCfg (
  IN     UINT32       DebugPrintErrLevel,
  IN OUT VOID         *DebugCfg
  )
//[-end-160219-IB08400332-modify]//
{
//[-start-160219-IB08400332-modify]//
  UINT32               DefaultDebugPrintErrLevel = 0;
  DEBUG_MESSAGE_CONFIG_UTIL_CONFIGURATION  *DebugMsgCfg;
//[-end-160219-IB08400332-modify]//

//[-start-160219-IB08400332-modify]//
//[-start-180518-IB08400596-modify]//
  DefaultDebugPrintErrLevel = PcdGet32 (PcdDebugPrintErrorLevel);  
//[-end-180518-IB08400596-modify]//
  DebugMsgCfg = (DEBUG_MESSAGE_CONFIG_UTIL_CONFIGURATION*)DebugCfg;
  DebugMsgCfg->EfiDebugMsgAdvancedMode = FALSE;
  
  if (DebugPrintErrLevel == DefaultDebugPrintErrLevel) {
    DebugMsgCfg->EfiDebugMsgLevel = 0; // Default setting
  } else if (DebugPrintErrLevel == DEBUG_INFO) {
    DebugMsgCfg->EfiDebugMsgLevel = 1; // Information
  } else if (DebugPrintErrLevel == DEBUG_ERROR) {
    DebugMsgCfg->EfiDebugMsgLevel = 2; // Error
  } else if (DebugPrintErrLevel == DEBUG_WARN) {
    DebugMsgCfg->EfiDebugMsgLevel = 3; // Warnings
  } else if (DebugPrintErrLevel == 0xFFFFFFCF) {
    DebugMsgCfg->EfiDebugMsgLevel = 4; // Verbose
  } else if (DebugPrintErrLevel == 0) {
    DebugMsgCfg->EfiDebugMsgLevel = 0xFF; // Disabled
  } else {
    DebugMsgCfg->EfiDebugMsgAdvancedMode = TRUE;

    if ((DebugPrintErrLevel & DEBUG_INIT) == DEBUG_INIT) {
      DebugMsgCfg->DebugInit = TRUE;
    }
    if ((DebugPrintErrLevel & DEBUG_WARN) == DEBUG_WARN) {
      DebugMsgCfg->DebugWarn = TRUE;
    }
    if ((DebugPrintErrLevel & DEBUG_LOAD) == DEBUG_LOAD) {
      DebugMsgCfg->DebugLoad = TRUE;
    }
    if ((DebugPrintErrLevel & DEBUG_FS) == DEBUG_FS) {
      DebugMsgCfg->DebugFs = TRUE;
    }
    if ((DebugPrintErrLevel & DEBUG_INFO) == DEBUG_INFO) {
      DebugMsgCfg->DebugInfo = TRUE;
    }
    if ((DebugPrintErrLevel & DEBUG_DISPATCH) == DEBUG_DISPATCH) {
      DebugMsgCfg->DebugDispatch = TRUE;
    }
    if ((DebugPrintErrLevel & DEBUG_VARIABLE) == DEBUG_VARIABLE) {
      DebugMsgCfg->DebugVariable = TRUE;
    }
    if ((DebugPrintErrLevel & DEBUG_BM) == DEBUG_BM) {
      DebugMsgCfg->DebugBm = TRUE;
    }
    if ((DebugPrintErrLevel & DEBUG_BLKIO) == DEBUG_BLKIO) {
      DebugMsgCfg->DebugBlkio = TRUE;
    }
    if ((DebugPrintErrLevel & DEBUG_NET) == DEBUG_NET) {
      DebugMsgCfg->DebugNet = TRUE;
    }
    if ((DebugPrintErrLevel & DEBUG_UNDI) == DEBUG_UNDI) {
      DebugMsgCfg->DebugUndi = TRUE;
    }
    if ((DebugPrintErrLevel & DEBUG_LOADFILE) == DEBUG_LOADFILE) {
      DebugMsgCfg->DebugLoadFile = TRUE;
    }
    if ((DebugPrintErrLevel & DEBUG_EVENT) == DEBUG_EVENT) {
      DebugMsgCfg->DebugEvent = TRUE;
    }
    if ((DebugPrintErrLevel & DEBUG_GCD) == DEBUG_GCD) {
      DebugMsgCfg->DebugGcd = TRUE;
    }
    if ((DebugPrintErrLevel & DEBUG_CACHE) == DEBUG_CACHE) {
      DebugMsgCfg->DebugCache= TRUE;
    }
    if ((DebugPrintErrLevel & DEBUG_VERBOSE) == DEBUG_VERBOSE) {
      DebugMsgCfg->DebugVerbose = TRUE;
    }
    if ((DebugPrintErrLevel & DEBUG_ERROR) == DEBUG_ERROR) {
      DebugMsgCfg->DebugError = TRUE;
    }
  }
  
  return;
//[-end-160219-IB08400332-modify]//
}

