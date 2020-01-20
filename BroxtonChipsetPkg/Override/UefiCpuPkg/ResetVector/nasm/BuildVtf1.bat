
set BUILD_PATH=B:\BuildCode\Package\APL\APLM\BroxtonChipsetPkg\Override\UefiCpuPkg\ResetVector

set "Nasm_Flags=-D ARCH_IA32 -D DEBUG_PORT80 -D A0_TEMP_CHANGE"

PATH=%PATH%;%BUILD_PATH%\nasm
pushd %BUILD_PATH%\Vtf1
nasm.exe %Nasm_Flags% -o Bin\ResetVector.ia32.port80.raw ResetVectorCode.asm
FixupForRawSection.exe Bin\ResetVector.ia32.port80.raw
popd