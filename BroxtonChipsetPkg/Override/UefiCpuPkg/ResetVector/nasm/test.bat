PATH=%PATH%;D:\APL\Code\Trunk_1070_RC113\BroxtonChipsetPkg\Override\UefiCpuPkg\ResetVector\nasm

set "Nasm_Flags=-D ARCH_IA32 -D DEBUG_PORT80 -D A0_TEMP_CHANGE"

pushd D:\APL\Code\Trunk_1070_RC113\BroxtonChipsetPkg\Override\UefiCpuPkg\ResetVector\Vtf0
nasm.exe %Nasm_Flags% -o Bin\ResetVector.ia32.port80.raw ResetVectorCode.asm
FixupForRawSection.exe Bin\ResetVector.ia32.port80.raw
popd