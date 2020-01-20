@REM ******************************************************************************
@REM * Copyright (c) 2012 - 2017, Insyde Software Corp. All Rights Reserved.
@REM *
@REM * You may not reproduce, distribute, publish, display, perform, modify, adapt,
@REM * transmit, broadcast, present, recite, release, license or otherwise exploit
@REM * any part of this publication in any form, by any means, without the prior
@REM * written permission of Insyde Software Corporation.
@REM *
@REM ******************************************************************************

@echo off
SetLocal EnableDelayedExpansion EnableExtensions


@FOR /f "tokens=1,4" %%a in ('find "INTEL_TXE_FW_KIT_VERSION" ..\Project.env') DO (
  IF %%a==DEFINE (
    set CSE_Version=%%b
  )
)

REM 
REM Search BIOS name
REM
if exist BiosNameList.txt del BiosNameList.txt
for /f %%i in ('dir bios_*.bin /b') do echo %%i >> BiosNameList.txt


@FOR /f "tokens=1,2 delims=. " %%c in ('find "bios" BiosNameList.txt') DO (
  IF %%d==bin (
    set BIOS_FILE=%%c
    pushd ..\stitch
    call :DoReSignBios
    popd
    if NOT "!errorlevel!" == "0" goto ReSignBiosError
  )
)

REM
REM Clear MEU temp data
REM
rmdir ..\stitch\CSE\%CSE_Version%\APLK\MEU\input /s /q
rmdir ..\stitch\CSE\%CSE_Version%\APLK\MEU\out /s /q
if exist ..\stitch\CSE\%CSE_Version%\APLK\MEU\meu.log del ..\stitch\CSE\%CSE_Version%\APLK\MEU\meu.log > NUL

goto ExitReSignBios    

:ReSignBiosError
pause
exit /b 1

:ExitReSignBios

if exist BiosNameList.txt del BiosNameList.txt > NUL
EndLocal
exit /b

REM
REM ReSign BIOS
REM
:DoReSignBios

if not exist CSE\%CSE_Version%\APLK\MEU\input\module_bin\ (
  mkdir CSE\%CSE_Version%\APLK\MEU\input\module_bin\
)

copy /y /b ..\Bios\%BIOS_FILE%.bin CSE\%CSE_Version%\APLK\MEU\input > NUL

pushd CSE\%CSE_Version%\APLK\MEU

REM
REM Decomp BIOS.bin
REM
meu.exe -decomp BIOS -f input\%BIOS_FILE%.bin -save input\bios.xml -d input > NUL
REM copy /y /b input\%BIOS_FILE%\Decomp\*.* input\module_bin > NUL

if exist input\%BIOS_FILE%\Decomp\IBBL.bin      copy /y /b input\%BIOS_FILE%\Decomp\IBBL.bin      input\module_bin\FVIBBL.Fv          > NUL
if exist input\%BIOS_FILE%\Decomp\IBB.bin       copy /y /b input\%BIOS_FILE%\Decomp\IBB.bin       input\module_bin\IBBFV.Fv           > NUL
if exist input\%BIOS_FILE%\Decomp\IBBR.bin      copy /y /b input\%BIOS_FILE%\Decomp\IBBR.bin      input\module_bin\RECOVERYFV.Fv      > NUL
if exist input\%BIOS_FILE%\Decomp\PAD.bin       copy /y /b input\%BIOS_FILE%\Decomp\PAD.bin       input\module_bin\PAD.bin            > NUL
if exist input\%BIOS_FILE%\Decomp\NvCommon.bin  copy /y /b input\%BIOS_FILE%\Decomp\NvCommon.bin  input\module_bin\NVCOMMON.fd        > NUL
if exist input\%BIOS_FILE%\Decomp\OBB.bin       copy /y /b input\%BIOS_FILE%\Decomp\OBB.bin       input\module_bin\FVMAIN_COMPACT.Fv  > NUL
if exist input\%BIOS_FILE%\Decomp\NvStorage.bin copy /y /b input\%BIOS_FILE%\Decomp\NvStorage.bin input\module_bin\NvStore.Fv         > NUL

REM REM Gen BPM for MEU
REM REM GenBpmLite.exe -i %SOURCE_FV_FOLDER_PATH%\FVIBBL.Fv %SOURCE_FV_FOLDER_PATH%\IBBFV.Fv %SOURCE_FV_FOLDER_PATH%\RECOVERYFV.Fv -o %SOURCE_FV_FOLDER_PATH%\BPM.bin
REM copy %SOURCE_FV_FOLDER_PATH%\BPM.bin input\module_bin

REM [-start-180418-IB07400959-modify]REM
meu -f xml/OEMKeyManifest.xml -o oemkeymn2.bin -u1 . -key keys/bxt_dbg_priv_key_KM.pem -w out
REM [-end-180418-IB07400959-modify]REM
if %ERRORLEVEL% NEQ 0 echo Error in meu & goto ReSignBiosError

if exist oemkeymn2.bin move /y oemkeymn2.bin ..\..\..\..\..\Bios\oemkeymn2.bin > NUL
if %ERRORLEVEL% NEQ 0 echo Error in move oemkeymn2.bin & goto ReSignBiosError

meu -f xml/APLKBios.xml -o bios.bin -u1 . -key keys/bxt_dbg_priv_key.pem -w out -s input
if %ERRORLEVEL% NEQ 0 echo Error in meu & goto ReSignBiosError

if exist bios.bin move /y bios.bin ..\..\..\..\..\Bios\%BIOS_FILE%.bin > NUL
if exist bios.map move /y bios.map ..\..\..\..\..\Bios\%BIOS_FILE%.map > NUL
if %ERRORLEVEL% NEQ 0 echo Error in move bios.bin & goto ReSignBiosError

popd

exit /B 
