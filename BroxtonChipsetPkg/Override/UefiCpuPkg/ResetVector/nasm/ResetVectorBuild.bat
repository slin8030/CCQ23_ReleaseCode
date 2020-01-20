  
REM [-start-160811-IB0725-FSP-modify] REM
if "%FSP_ENABLE%" == "YES" (
  set "Nasm_Flags=-D ARCH_IA32 -D DEBUG_PORT80 -D A0_TEMP_CHANGE -D FSP_WRAPPER_FLAG"
  set "Reset_Vector_File_Name=ResetVector.ia32.port80.fsp.raw"
) else (
  set "Nasm_Flags=-D ARCH_IA32 -D DEBUG_PORT80 -D A0_TEMP_CHANGE"
  set "Reset_Vector_File_Name=ResetVector.ia32.port80.raw"
)
 
PATH=%PATH%;%WORKSPACE%\BroxtonChipsetPkg\Override\UefiCpuPkg\ResetVector\nasm
  
pushd %WORKSPACE%\BroxtonChipsetPkg\Override\UefiCpuPkg\ResetVector\Vtf0
nasm.exe %Nasm_Flags% -o "Bin\%Reset_Vector_File_Name%" "ResetVectorCode.asm"
FixupForRawSection.exe "Bin\%Reset_Vector_File_Name%"
popd
REM [-end-160811-IB0725-FSP-modify] REM
