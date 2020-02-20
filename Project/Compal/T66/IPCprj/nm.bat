@echo off
SETLOCAL ENABLEDELAYEDEXPANSION

set INTEXTFILE=Project.uni
set OUTTEXTFILE=Project.tmp
set PAR1=%1
set tempfd=

if '%1'==''      goto Build64
if '%1'=='32'    goto Build32
if '%1'=='64'    goto Build64
if '%1'=='all'   goto BuildAll
if '%1'=='ALL' 	 goto BuildAll
if '%1'=='c'     goto BuildClean
if '%1'=='C'     goto BuildClean
if '%1'=='clean' goto BuildClean
if '%1'=='d'     goto BuildEfidebug
if '%1'=='D'     goto BuildEfidebug
if '%1'=='32d'   goto BuildUefi32ddt
if '%1'=='32D'   goto BuildUefi32ddt
if '%1'=='64d'   goto BuildUefi64ddt
if '%1'=='64D'   goto BuildUefi64ddt
if '%1'=='r'     goto BuildUefiRmt
if '%1'=='R'     goto BuildUefiRmt
if '%1'=='32c'   goto Build32Clean
if '%1'=='32C'   goto Build32Clean
if '%1'=='64c'   goto Build64Clean
if '%1'=='64C'   goto Build64Clean
if '%1'=='Win'   goto BuildWinFlash

goto Help

:Help
@cls
echo *
echo * (C) Copyright 2015-2020 Compal Electronics, Inc.
echo *
echo * This software is the property of Compal Electronics, Inc.
echo * You have to accept the terms in the license file before use.
echo *
echo * Copyright 2015-2020 Compal Electronics, Inc.. All rights reserved.
echo *
echo BIOS Compiler batch file
echo Ver       Date        Who            Change
echo --------  ----------  ----------     -----------------------------------------------------------------------------
echo 0.01      2015/09/03  Aven_Liu       Initial version.
echo 0.02      2015/12/24  Aven_Liu       Modify for simple paser BIOS version by Stan suggestion. Merge with small core files.
echo 0.03      2015/12/24  Aven_Liu       Correct combine EC+SYS BIN.
echo 0.04      2016/02/04  Stan_Chen      Add commands for DDT/RMT
echo.
echo nm [par1]
echo [par1] =           Default compiler
echo [par1] = 32        Compiler 32 bit BIOS
echo [par1] = 64        Compiler 64 bit BIOS
echo [par1] = all       Compiler both 32/64 bit BIOS
echo [par1] = d         Compiler efidebug BIOS
echo [par1] = 32d       Compiler uefi32ddt BIOS
echo [par1] = 64d       Compiler uefi64ddt BIOS
echo [par1] = r         Compiler uefi64rmt BIOS
echo [par1] = clean	    Clean build files
echo [par1] = h         Help
echo [par1] = 32c       Clean build files and Compiler 32 bit BIOS
echo [par1] = 64c       Clean build files and Compiler 64 bit BIOS

goto END

:ReplaceTarg
set SEARCHTEXT=i32
set REPLACETEXT=X64
if "%BIOSBuild%"=="IA32" (
    set SEARCHTEXT=X64
    set REPLACETEXT=i32
)
powershell -Command "(gc %INTEXTFILE%) -replace '%SEARCHTEXT%', '%REPLACETEXT%' | Out-File %OUTTEXTFILE%"
del %INTEXTFILE%
rename %OUTTEXTFILE% %INTEXTFILE%
powershell -Command "(gc .\WinFlash\platform_FORM.ini) -replace '%SEARCHTEXT%', '%REPLACETEXT%' | Out-File .\WinFlash\platform_FORM.ini -Encoding oem"
powershell -Command "(gc .\WinFlash\platform_SWDL.ini) -replace '%SEARCHTEXT%', '%REPLACETEXT%' | Out-File .\WinFlash\platform_SWDL.ini -Encoding oem"
Goto :eof

:BuildAll
nmake clean
goto Build32

:BuildClean
nmake clean
echo Clean Done.
goto END

:Build32Clean
nmake clean
goto Build32

:Build64Clean
nmake clean
goto Build64

:Build32
Set BIOSBuild=IA32
set ARCH=%BIOSBuild%
CALL :ReplaceTarg
del /q %WORKSPACE%%PROJECT_PKG%\Bios\%OemPath%_8M.%REPLACETEXT%.bin >NUL
nmake uefi32
IF NOT EXIST %WORKSPACE%%PROJECT_PKG%\Bios\%OemPath%_8M.%REPLACETEXT%.bin GOTO ERROR
CALL :BuildWinFlash
CALL :CopyReleasePackage 32
echo Build32 Done.
if '%PAR1%'=='32' goto END
if '%PAR1%'=='32c' goto END
if '%PAR1%'=='32C' goto END
nmake clean

:Build64
Set BIOSBuild=IA32 X64
set ARCH=%BIOSBuild%

CALL :ReplaceTarg
del /q %WORKSPACE%%PROJECT_PKG%\Bios\BIOS_Full\bxt_spi_8mb_OxbowHill\BX\*.* >NUL
nmake uefi64debug
echo BiosBin=%WORKSPACE%%PROJECT_PKG%\Bios\BIOS_Full\bxt_spi_8mb_OxbowHill\BX\%SOC_NAME%_%CRB_BOARD_NAME%_X64_EFIDEBUG.bin 
IF NOT EXIST %WORKSPACE%%PROJECT_PKG%\Bios\BIOS_Full\bxt_spi_8mb_OxbowHill\BX\%SOC_NAME%_%CRB_BOARD_NAME%_X64_EFIDEBUG.bin GOTO ERROR 
Copy /v /y %WORKSPACE%%PROJECT_PKG%\Bios\BIOS_Full\bxt_spi_8mb_OxbowHill\BX\%SOC_NAME%_%CRB_BOARD_NAME%_X64_EFIDEBUG.bin %WORKSPACE%%PROJECT_PKG%\Bios\%OemPath%_B0_8M.%REPLACETEXT%.bin
CALL :BuildWinFlash
echo build winflash done
CALL :CopyReleasePackage 64
echo Build64 Done.
goto END

:BuildEfidebug
Set BIOSBuild=IA32 X64
set ARCH=%BIOSBuild%
CALL :ReplaceTarg
nmake clean
nmake efidebug
CALL :BuildWinFlash
CALL :CopyReleasePackage 64
echo efidebug Done.
goto END

:BuildUefi32ddt
Set BIOSBuild=IA32
set ARCH=%BIOSBuild%
CALL :ReplaceTarg
nmake clean
nmake uefi32ddt
CALL :BuildWinFlash
CALL :CopyReleasePackage 32
echo uefi32ddt Done.
goto END


:BuildUefi64ddt
Set BIOSBuild=X64
set ARCH=%BIOSBuild%
CALL :ReplaceTarg
nmake clean
nmake uefi64ddt
CALL :BuildWinFlash
CALL :CopyReleasePackage 64
echo uefi64ddt Done.
goto END


:BuildUefiRmt
Set BIOSBuild=IA32 X64
set ARCH=%BIOSBuild%
CALL :ReplaceTarg
nmake clean
nmake uefi64rmt
CALL :BuildWinFlash
CALL :CopyReleasePackage 64
echo uefirmt Done.
goto END
:DeQuote
for /f "delims=" %%A in ('echo %%%1%%') do set %1=%%~A
Goto :eof

:BuildWinFlash
echo BUILD_TARGET=%BUILD_TARGET%
if not %ERRORLEVEL%==0 (
    goto ERROR
)
echo BuildWinFlash.
SET BIOS_Ver=
SET BIOS_SUB_Ver=
type Project.uni |find "STR_MISC_BIOS_VERSION" > temp.txt
FOR /F "tokens=6" %%i IN (temp.txt) DO (
   set tempfd=%%i
   SET BIOS_Ver=!tempfd:~0,-5!
)
DEL /Q temp.txt

rem Setlocal
CALL :dequote BIOS_Ver
SET BIOS_Ver=%BIOS_Ver:~1%
SET BIOS_Ver=%BIOS_Ver:.=%
@echo.
pushd .\WinFlash\Packer
@echo Build Formal Release Winflash: %WORKSPACE%%PROJECT_PKG%\Bios\%OemPath%%BIOS_Ver%%REPLACETEXT%.EXE
Copy /v /y %WORKSPACE%%PROJECT_PKG%\WinFlash\platform_FORM.ini %WORKSPACE%%PROJECT_PKG%\WinFlash\platform.ini
iFdPacker.exe -winsrc .. -winini -b 64 -fv %WORKSPACE%%PROJECT_PKG%\Bios\%OemPath%_B0_8M.%REPLACETEXT%.bin -fv %WORKSPACE%%PROJECT_PKG%\Binary\Ec\ecb.bin -output %WORKSPACE%%PROJECT_PKG%\Bios\%OemPath%%BIOS_Ver%%REPLACETEXT%.EXE

@echo Build SWDL Winflash: %WORKSPACE%%PROJECT_PKG%\Bios\%OemPath%%BIOS_Ver%%REPLACETEXT%_SWDL.EXE
Copy /v /y %WORKSPACE%%PROJECT_PKG%\WinFlash\platform_SWDL.ini %WORKSPACE%%PROJECT_PKG%\WinFlash\platform.ini
iFdPacker.exe -winsrc .. -winini -b 64 -fv %WORKSPACE%%PROJECT_PKG%\Bios\%OemPath%_B0_8M.%REPLACETEXT%.bin -fv %WORKSPACE%%PROJECT_PKG%\Binary\Ec\ecb.bin -output  %WORKSPACE%%PROJECT_PKG%\Bios\%OemPath%%BIOS_Ver%%REPLACETEXT%_SWDL.EXE
popd
echo BuildWinFlash Done.
Goto :eof

:CopyReleasePackage
if not %ERRORLEVEL%==0 (
   goto ERROR
)

SET BIOS_Ver=
SET BIOS_SUB_Ver=
type Project.uni |find "STR_MISC_BIOS_VERSION" > temp.txt
FOR /F "tokens=6" %%i IN (temp.txt) DO (
   set tempfd=%%i
   SET BIOS_Ver=!tempfd:~0,-5!
)
DEL /Q temp.txt

rem Setlocal
CALL :dequote BIOS_Ver
SET BIOS_Ver=%BIOS_Ver:~1%
SET BIOS_Ver=%BIOS_Ver:.=%
@echo.

IF "%PAR1%" == "ALL" SET BIOS_Ver=%BIOS_Ver%.%1
IF "%PAR1%" == "all" SET BIOS_Ver=%BIOS_Ver%.%1
echo %BIOS_Ver%
for /f "skip=7 tokens=5" %%a in ('dir .\Bios /a:d') do (
  IF EXIST %WORKSPACE%%PROJECT_PKG%\Bios\%%a RD /s/q %WORKSPACE%%PROJECT_PKG%\Bios\%%a
)
mkdir %WORKSPACE%%PROJECT_PKG%\Bios\%BIOS_Ver%
mkdir %WORKSPACE%%PROJECT_PKG%\Bios\%BIOS_Ver%\Shell
mkdir %WORKSPACE%%PROJECT_PKG%\Bios\%BIOS_Ver%\Win
mkdir %WORKSPACE%%PROJECT_PKG%\Bios\%BIOS_Ver%\SWDL
copy /v /y .\WinFlash\H2OFFT-Sx64.efi %WORKSPACE%%PROJECT_PKG%\Bios\%BIOS_Ver%\Shell
rem copy .\WinFlash\platform_FORM.ini %WORKSPACE%%PROJECT_PKG%\Bios\%BIOS_Ver%\Shell\platform.ini
copy /v /y %WORKSPACE%%PROJECT_PKG%\Bios\%OemPath%_B0_8M.%REPLACETEXT%.bin %WORKSPACE%%PROJECT_PKG%\Bios\%BIOS_Ver%\Shell\%OemPath%%BIOS_Ver%.bin >NUL
copy /b %WORKSPACE%%PROJECT_PKG%\Binary\Ec\Ecb.bin+%WORKSPACE%%PROJECT_PKG%\Bios\%OemPath%_B0_8M.%REPLACETEXT%.bin %WORKSPACE%%PROJECT_PKG%\Bios\%BIOS_Ver%\Shell\%OemPath%%BIOS_Ver%EC.bin >NUL
echo H2OFFT-Sx64.efi %OemPath%%BIOS_Ver%EC.bin -ecb -bios > %WORKSPACE%%PROJECT_PKG%\Bios\%BIOS_Ver%\Shell\BiosEC.nsh
copy /v /y %WORKSPACE%%PROJECT_PKG%\Bios\%OemPath%%BIOS_Ver%%REPLACETEXT%.EXE %WORKSPACE%%PROJECT_PKG%\Bios\%BIOS_Ver%\Win\
copy /v /y %WORKSPACE%%PROJECT_PKG%\Bios\%OemPath%%BIOS_Ver%%REPLACETEXT%_SWDL.EXE %WORKSPACE%%PROJECT_PKG%\Bios\%BIOS_Ver%\SWDL\
 
@echo Output success to: %WORKSPACE%%PROJECT_PKG%\Bios\%BIOS_Ver%

IF EXIST *.7z del /q *.7z >NUL
REM ++ >>>> Modify for build server process.
%TOOL_DRV%\%DEVTLS_DIR%\TOOLS\7z a %BIOS_Ver%.7z .\Bios\%BIOS_Ver% 
REM ++ >>>> Modify for build server process.
Goto :eof

:ERROR
@echo.
@echo [ERROR] Build fail...
Goto END

:END
