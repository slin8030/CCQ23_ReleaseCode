/** @file

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************

*/

#ifndef _PROPERTY_DEF_H_
#define _PROPERTY_DEF_H_

#define COMMON_PAGE_LAYOUT_ID                  0x0001
#define SCU_LAYOUT_ID                          0x0002
#define FRONT_PAGE_LAYOUT_ID                   0x0003
#define BOOT_MANAGER_LAYOUT_ID                 0x0004
#define BOOT_FROM_FILE_LAYOUT_ID               0x0005

#define SCREEN_PANEL_ID                        0x0001
#define OWNER_DRAW_PANEL_ID                    0x0002
#define TITLE_PANEL_ID                         0x0003
#define SETUP_MENU_PANEL_ID                    0x0004
#define SETUP_PAGE_PANEL_ID                    0x0005
#define HELP_TEXT_PANEL_ID                     0x0006
#define HOTKEY_PANEL_ID                        0x0007
#define QUESTION_PANEL_ID                      0x0008
#define SETUP_PAGE_2_PANEL_ID                  0x0009

#define SETUP_UTILITY_FORMSET_CLASS_GUID {0x9f85453e, 0x2f03, 0x4989, 0xad, 0x3b, 0x4a, 0x84, 0x07, 0x91, 0xaf, 0x3a}

#define ADVANCED_VFR_ID                        0x0002
#define EVENT_LOG_FORM_ID                      0x0003

#define H2O_PANEL_TYPE_SCREEN                  0x00000000 // default
#define H2O_PANEL_TYPE_HOTKEY                  0x00000001
#define H2O_PANEL_TYPE_HELP_TEXT               0x00000002
#define H2O_PANEL_TYPE_SETUP_MENU              0x00000004
#define H2O_PANEL_TYPE_SETUP_PAGE              0x00000008
#define H2O_PANEL_TYPE_FORM_TITLE              0x00000010
#define H2O_PANEL_TYPE_QUESTION                0x00000020
#define H2O_PANEL_TYPE_OWNER_DRAW              0x00000040
#define H2O_PANEL_TYPE_SETUP_PAGE2             0x00000080

#define OEM_TEXT_GRAY                          rgb0xFF808285
#define OEM_BK_GRAY                            rgb0xFFE6E7E8
#define OEM_ORANGE                             rgb0xFFE45620
#define OEM_BLUE                               rgb0xFF19BEEC

#define H2O_CONTROL_MAIN_PAGE_ONE_OF_GUID \
{ \
  0xd197cfcd, 0x825f, 0x4182, 0xa6, 0x8a, 0xda, 0x7f, 0x0a, 0x1a, 0x17, 0xa3 \
}

#define RESOLUTION_VERTICAL                    0x00000000
#define RESOLUTION_HORIZONTAL                  0x00000001

#define DISPLAY_NONE                           0x00000000
#define DISPLAY_LEFT                           0x00000001
#define DISPLAY_TOP                            0x00000002
#define DISPLAY_RIGHT                          0x00000004
#define DISPLAY_BOTTOM                         0x00000008

#define H2O_HOT_KEY_ACTION_SHOW_HELP              0x00000001
#define H2O_HOT_KEY_ACTION_SELECT_PREVIOUS_ITEM   0x00000002
#define H2O_HOT_KEY_ACTION_SELECT_NEXT_ITEM       0x00000003
#define H2O_HOT_KEY_ACTION_SELECT_PREVIOUS_MENU   0x00000004
#define H2O_HOT_KEY_ACTION_SELECT_NEXT_MENU       0x00000005
#define H2O_HOT_KEY_ACTION_MODIFY_PREVIOUS_VALUE  0x00000006
#define H2O_HOT_KEY_ACTION_MODIFY_NEXT_VALUE      0x00000007
#define H2O_HOT_KEY_ACTION_DISCARD                0x00000008
#define H2O_HOT_KEY_ACTION_DISCARD_AND_EXIT       0x00000009
#define H2O_HOT_KEY_ACTION_LOAD_DEFAULT           0x0000000A
#define H2O_HOT_KEY_ACTION_SAVE                   0x0000000B
#define H2O_HOT_KEY_ACTION_SAVE_AND_EXIT          0x0000000C
#define H2O_HOT_KEY_ACTION_ENTER                  0x0000000D
#define H2O_HOT_KEY_ACTION_CALLBACK               0x0000000E
#define H2O_HOT_KEY_ACTION_GOTO                   0x0000000F
#define H2O_HOT_KEY_ACTION_SET_QUESTION_VALUE     0x00000010

#define FRONT_PAGE_FORMSET_GUID     {0x9e0c30bc, 0x3f06, 0x4ba6, 0x82, 0x88, 0x9 , 0x17, 0x9b, 0x85, 0x5d, 0xbe}
#define BOOT_MANAGER_FORMSET_GUID   {0x847bc3fe, 0xb974, 0x446d, 0x94, 0x49, 0x5a, 0xd5, 0x41, 0x2e, 0x99, 0x3b}
#define DEVICE_MANAGER_FORMSET_GUID {0x3ebfa8e6, 0x511d, 0x4b5b, 0xa9, 0x5f, 0xfb, 0x38, 0x26, 0xf , 0x1c, 0x27}
#define DRIVER_HEALTH_FORMSET_GUID  {0xf76e0a70, 0xb5ed, 0x4c38, 0xac, 0x9a, 0xe5, 0xf5, 0x4b, 0xf1, 0x6e, 0x34}
#define BOOT_MAINT_FORMSET_GUID     {0x642237c7, 0x35d4, 0x472d, 0x83, 0x65, 0x12, 0xe0, 0xcc, 0xf2, 0x7a, 0x22}
#define FILE_EXPLORE_FORMSET_GUID   {0x1f2d63e1, 0xfebd, 0x4dc7, 0x9c, 0xc5, 0xba, 0x2b, 0x1c, 0xef, 0x9c, 0x5b}
#define SECURE_BOOT_FORMSET_GUID    {0xaa1305b9, 0x01f3, 0x4afb, 0x92, 0x0e, 0xc9, 0xb9, 0x79, 0xa8, 0x52, 0xfd}

#define FORMSET_ID_GUID_MAIN      {0xc1e0b01a, 0x607e, 0x4b75, 0xb8, 0xbb, 0x06, 0x31, 0xec, 0xfa, 0xac, 0xf2}
#define FORMSET_ID_GUID_BOOT      {0x2d068309, 0x12ac, 0x45ab, 0x96, 0x00, 0x91, 0x87, 0x51, 0x3c, 0xcd, 0xd8}
#define FORMSET_ID_GUID_SECURITY  {0x5204f764, 0xdf25, 0x48a2, 0xb3, 0x37, 0x9e, 0xc1, 0x22, 0xb8, 0x5e, 0x0d}
#define FORMSET_ID_GUID_EXIT      {0xb6936426, 0xfb04, 0x4a7b, 0xaa, 0x51, 0xfd, 0x49, 0x39, 0x7c, 0xdc, 0x01}
#define FORMSET_ID_GUID_ADVANCE   {0xc6d4769e, 0x7f48, 0x4d2a, 0x98, 0xe9, 0x87, 0xad, 0xcc, 0xf3, 0x5c, 0xcc}
#define FORMSET_ID_GUID_POWER     {0xa6712873, 0x925f, 0x46c6, 0x90, 0xb4, 0xa4, 0x0f, 0x86, 0xa0, 0x91, 0x7b}
#define VFR_APP_FORMSET_GUID      {0xA04A27f4, 0xDF00, 0x1234, 0xB5, 0x52, 0x39, 0x51, 0x13, 0x02, 0x11, 0x3F}
#define ROOT_FORM_ID              1
#endif

