/** @file

;******************************************************************************
;* Copyright (c) 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _LTDE_SUPPORT_H_
#define _LTDE_SUPPORT_H_

typedef struct _LTDE_PANEL_BORDER_LINE_INFO {
  UINT32                                      HorizonalLineWidth;
  UINT32                                      VerticalLineHeight;
  CHAR16                                      *TopHorizonalLine;
  CHAR16                                      *BottomHorizonalLine;
  CHAR16                                      *LeftVerticalLine;
  CHAR16                                      *RightVerticalLine;
} LTDE_PANEL_BORDER_LINE_INFO;

EFI_STATUS
GetBorderLineInfo (
  IN  RECT                                       *CurrentPanelField,
  IN  RECT                                       *BorderLineFieldList,
  IN  UINT32                                     BorderLineFieldCount,
  OUT LTDE_PANEL_BORDER_LINE_INFO                *BorderLineInfo
  );

VOID
FreeBorderLineInfo (
  IN LTDE_PANEL_BORDER_LINE_INFO              *BorderLineInfo
  );

EFI_STATUS
PrintBorderLineFunc (
  IN RECT                                     *CurrentPanelField,
  IN LTDE_PANEL_BORDER_LINE_INFO              *BorderLineInfo     OPTIONAL
  );

#endif
