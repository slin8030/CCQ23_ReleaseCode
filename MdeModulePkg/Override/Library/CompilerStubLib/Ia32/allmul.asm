;;******************************************************************************
;;* Copyright (c) 1983-2016, Insyde Software Corp. All Rights Reserved.
;;*
;;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;;* transmit, broadcast, present, recite, release, license or otherwise exploit
;;* any part of this publication in any form, by any means, without the prior
;;* written permission of Insyde Software Corporation.
;;*
;;******************************************************************************
;;
;; Module Name:
;;
;;   allmul.asm
;;
;; Abstract:
;;
;;   64 bits and 32 bits multiple method.
;;

TITLE   allmul.asm

  .686
  .MODEL FLAT,C
  .CODE

_allmul PROC public
      mov         eax,dword ptr [esp+8]
      mov         ecx,dword ptr [esp+10h]
      or          ecx,eax
      mov         ecx,dword ptr [esp+0Ch]
      jne         @F
      mov         eax,dword ptr [esp+4]
      mul         ecx
      ret         10h
@@:
      push        ebx
      mul         ecx
      mov         ebx,eax
      mov         eax,dword ptr [esp+8]
      mul         dword ptr [esp+14h]
      add         ebx,eax
      mov         eax,dword ptr [esp+8]
      mul         ecx
      add         edx,ebx
      pop         ebx
      ret         10h
      
_allmul ENDP
END
