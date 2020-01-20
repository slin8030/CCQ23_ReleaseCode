.386p
.MODEL FLAT,C
.code
;--------------------------------------------------------------
; VOID 
; LocalIrqSave(
;   OUT UINT32 * eflags
;   );
;
;
; PROC: the compiler will automatically insert prolog and epilog
LocalIrqSave PROC
;   mov ecx, eflagsPtr
;   pushfd
;   pop eax
;   mov dword ptr [ecx],eax
;   cli
  ret
LocalIrqSave ENDP
 
;--------------------------------------------------------------
; VOID 
; LocalIrqRestore(
;   IN UINT32  eflags
;   );
;
;
LocalIrqRestore PROC
;   mov eax, eflags
;   push eax
;   popfd
  ret
LocalIrqRestore  ENDP
END 
;--------------------------------------------------------------
;void LocalIrqRestore(
; UINT32  eflags)
;
;
