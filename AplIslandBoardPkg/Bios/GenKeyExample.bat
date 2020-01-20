@REM ******************************************************************************
@REM * Copyright (c) 2012 - 2018, Insyde Software Corp. All Rights Reserved.
@REM *
@REM * You may not reproduce, distribute, publish, display, perform, modify, adapt,
@REM * transmit, broadcast, present, recite, release, license or otherwise exploit
@REM * any part of this publication in any form, by any means, without the prior
@REM * written permission of Insyde Software Corporation.
@REM *
@REM ******************************************************************************

@echo off
SetLocal EnableDelayedExpansion EnableExtensions


set OEM_KEY_NAME=privkey-OEM-ingr
set OEM_KM_NAME=privkey-OEM-KM

@FOR /f "tokens=1,4" %%a in ('find "INTEL_TXE_FW_KIT_VERSION" ..\Project.env') DO (
  IF %%a==DEFINE (
    set CSE_Version=%%b
  )
)

set OPENSSL_TOOL=..\Stitch\CSE\%CSE_Version%\APLK\MEU\openssl\openssl.exe
set MEU_PATH=..\Stitch\CSE\%CSE_Version%\APLK\MEU
set FIT_PATH=..\Stitch\CSE\%CSE_Version%\APLK\FIT

REM
REM Generate Private Keys...
REM
echo Generate Private Keys...
if exist OemKeys\%OEM_KEY_NAME%.pem (
  echo OEM Key already exist, skip...
  goto GenPriKeyDone
)
if not exist OemKeys mkdir OemKeys
echo================================================================================
%OPENSSL_TOOL% genrsa -out OemKeys\%OEM_KEY_NAME%.pem 2048 > NUL
%OPENSSL_TOOL% genrsa -out OemKeys\%OEM_KM_NAME%.pem 2048 > NUL
echo================================================================================

if not exist %MEU_PATH%\Temp mkdir %MEU_PATH%\Temp
copy /y OemKeys\*.* %MEU_PATH%\Temp > NUL

REM
REM Key Hash Extraction via MEU and OpenSSL...
REM
echo Key Hash Extraction via MEU and OpenSSL...
pushd %MEU_PATH%
meu.exe -keyhash Temp\%OEM_KEY_NAME% -key Temp\%OEM_KEY_NAME%.pem > NUL
meu.exe -keyhash Temp\%OEM_KM_NAME% -key Temp\%OEM_KM_NAME%.pem > NUL
if exist meu.log del meu.log > NUL
popd

copy /y %MEU_PATH%\Temp\*.* OemKeys > NUL
rmdir %MEU_PATH%\Temp /s /q > NUL

:GenPriKeyDone
REM
REM Backup Intel Keys
REM
echo Backup Intel Keys...
if not exist %FIT_PATH%\keys\Intel_bxt_dbg_priv_key.pem (
  copy /y %FIT_PATH%\keys\bxt_dbg_priv_key.pem %FIT_PATH%\keys\Intel_bxt_dbg_priv_key.pem > NUL
)
if not exist %MEU_PATH%\keys\Intel_bxt_dbg_priv_key.pem (
  copy /y %MEU_PATH%\keys\bxt_dbg_priv_key.pem %MEU_PATH%\keys\Intel_bxt_dbg_priv_key.pem > NUL
)
if not exist %MEU_PATH%\keys\Intel_bxt_dbg_priv_key_KM.pem (
  copy /y %MEU_PATH%\keys\bxt_dbg_priv_key_KM.pem %MEU_PATH%\keys\Intel_bxt_dbg_priv_key_KM.pem > NUL
)
if not exist %MEU_PATH%\keys\Intel_bxt_dbg_pub_key_hash.bin (
  copy /y %MEU_PATH%\keys\bxt_dbg_pub_key_hash.bin %MEU_PATH%\keys\Intel_bxt_dbg_pub_key_hash.bin > NUL
)

REM
REM Replace Intel Keys
REM
echo Replace Keys...
@echo Replace FIT\Keys\bxt_dbg_priv_key.pem to %OEM_KEY_NAME%.pem..
copy /y OemKeys\%OEM_KEY_NAME%.pem %FIT_PATH%\keys\bxt_dbg_priv_key.pem > NUL
@echo Replace MEU\Keys\bxt_dbg_priv_key.pem to %OEM_KEY_NAME%.pem..
copy /y OemKeys\%OEM_KEY_NAME%.pem %MEU_PATH%\keys\bxt_dbg_priv_key.pem > NUL
@echo Replace MEU\Keys\bxt_dbg_priv_key_KM.pem to %OEM_KM_NAME%.pem..
copy /y OemKeys\%OEM_KM_NAME%.pem %MEU_PATH%\keys\bxt_dbg_priv_key_KM.pem > NUL
@echo Replace MEU\Keys\bxt_dbg_pub_key_hash.bin to %OEM_KEY_NAME%.bin..
copy /y OemKeys\%OEM_KEY_NAME%.bin %MEU_PATH%\keys\bxt_dbg_pub_key_hash.bin > NUL

echo================================================================================
echo Please replace hash key to your XML file (OemPublicKeyHash)..
type OemKeys\%OEM_KM_NAME%.txt
echo.
echo================================================================================
explorer %FIT_PATH%\xml
pause

exit /B 
