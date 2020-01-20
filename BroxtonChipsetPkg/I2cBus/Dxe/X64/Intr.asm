text  SEGMENT

ALIGN   8

;--------------------------------------------------------------
; VOID 
; LocalIrqSave(
;   OUT UINT32 * eflags // rcx
;   );
;
;
; PROC: the compiler will automatically insert prolog and epilog
;--------------------------------------------------------------
LocalIrqSave PROC
;  mov      rcx, eflagsPtr
  pushfq
  pop      rax
  mov qword ptr [rcx],rax
  cli
  ret
LocalIrqSave ENDP
 
;--------------------------------------------------------------
; VOID 
; LocalIrqRestore(
;   IN UINT32  eflags // rcx
;   );
;
;--------------------------------------------------------------

LocalIrqRestore PROC
  mov      rax, rcx
  push     rax
  popfq
  ret
LocalIrqRestore  ENDP


END
