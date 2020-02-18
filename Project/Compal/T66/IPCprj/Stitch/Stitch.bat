@echo off
SetLocal EnableDelayedExpansion EnableExtensions

@for /f "tokens=3" %%a in ('find "TARGET " %WORKSPACE%\Conf\target.txt') do @set TARGET=%%a
@for /f "tokens=3" %%a in ('find "TOOL_CHAIN_TAG" %WORKSPACE%\Conf\target.txt') do @set TOOL_CHAIN_TAG=%%a

REM [-start-160223-IB07400702-add]REM
@find /v "#" %WORKSPACE%\Build\%PROJECT_PKG%\IntelFwVersion.txt > string
@for /f "tokens=1,2,3" %%i in (string) do set %%i=%%k
@del string
REM [-end-160223-IB07400702-add]REM

REM[-start-151204-IB11270135-modify]REM
SET SOURCE_FV_FOLDER_PATH=%WORKSPACE%\Build\%PROJECT_PKG%\%TARGET%_%TOOL_CHAIN_TAG%\FV
REM[-end-151204-IB11270135-modify]REM

REM[-start-161123-IB07250310-add]REM
REM Check FSP_ENABLE feature enable
REM [-start-170517-IB07400866-modify]REM
@for /f "tokens=4" %%a in ('find "FSP_ENABLE" %WORKSPACE%\%PROJECT_PKG%\Project.env') do @set FSP_ENABLE=%%a

if "%FSP_ENABLE%" == "NO" goto FSP_Check1_Done

    echo.
    echo FSP_ENALBE...split FSP Binary 
    echo (Copy and split the binary under Build\%PROJECT_PKG%\FSP)
    echo.
    if not exist %WORKSPACE%\Build\%PROJECT_PKG%\FSP mkdir %WORKSPACE%\Build\%PROJECT_PKG%\FSP
    Copy /y %WORKSPACE%\BroxtonFspBinPkg\Fsp.fd  %WORKSPACE%\Build\%PROJECT_PKG%\FSP\Fsp.fd
    if "%FD_TARGET%"=="RELEASE" (
        Copy /y %WORKSPACE%\BroxtonFspBinPkg\Fsp.fd %WORKSPACE%\Build\%PROJECT_PKG%\FSP\Fsp.fd
    ) else if "%FD_TARGET%"=="PERF" (
        Copy /y %WORKSPACE%\BroxtonFspBinPkg\Fsp.fd %WORKSPACE%\Build\%PROJECT_PKG%\FSP\Fsp.fd
    ) else if "%FD_TARGET%"=="DDT" (
        Copy /y %WORKSPACE%\BroxtonFspBinPkg\Fsp_DDT.fd %WORKSPACE%\Build\%PROJECT_PKG%\FSP\Fsp.fd
    ) else if "%FD_TARGET%"=="EFIDEBUG" (
        Copy /y %WORKSPACE%\BroxtonFspBinPkg\Fsp_Debug.fd %WORKSPACE%\Build\%PROJECT_PKG%\FSP\Fsp.fd
    ) else (
        Copy /y %WORKSPACE%\BroxtonFspBinPkg\Fsp_DDT.fd %WORKSPACE%\Build\%PROJECT_PKG%\FSP\Fsp.fd
    )
    SET PYTHON="C:\Python27\python.exe"
    %PYTHON% %WORKSPACE%\BroxtonFspBinPkg\Tools\fsptool.py split -f %WORKSPACE%\Build\ApolloLakeBoardPkg\FSP\Fsp.fd -o %WORKSPACE%\Build\ApolloLakeBoardPkg\FSP\ -n FSP

:FSP_Check1_Done

REM [-end-170517-IB07400866-modify]REM
REM[-end-161123-IB07250310-add]REM
REM [-start-160223-IB07400702-modify]REM
echo CSE_Version=%CSE_Version%
REM [-end-160223-IB07400702-modify]REM
if not exist CSE\%CSE_Version%\APLK\FIT\input\ (
    mkdir CSE\%CSE_Version%\APLK\FIT\input
)
REM del /f/q CSE\%CSE_Version%\FIT\input\*.*
copy /y CSE\%CSE_Version%\APLK\Silicon\cse_image.bin CSE\%CSE_Version%\APLK\FIT\input\cse_image.bin
REM[-start-160527-IB07220084-add]
copy /y CSE\%CSE_Version%\APLK\Silicon\cse_image_b0.bin CSE\%CSE_Version%\APLK\FIT\input\cse_image_b0.bin
REM[-end-160527-IB07220084-add]

REM Gen NvStore
GenNvStoreFv.exe %SOURCE_FV_FOLDER_PATH%\CHV.fd -o %SOURCE_FV_FOLDER_PATH%\NvStore.Fv

REM [-start-160226-IB07400704-add]REM
if not exist CSE\%CSE_Version%\APLK\MEU\input\module_bin\ (
    mkdir CSE\%CSE_Version%\APLK\MEU\input\module_bin\
)
REM [-end-160226-IB07400704-add]REM
REM[-start-161123-IB07250310-add]REM
if "%FSP_ENABLE%" == "YES" (
    copy /y /b %SOURCE_FV_FOLDER_PATH%\IBBFV.Fv+%WORKSPACE%\Build\%PROJECT_PKG%\FSP\FSP-M.fv %SOURCE_FV_FOLDER_PATH%\IBBM.Fv
    copy /y /b %WORKSPACE%\Build\%PROJECT_PKG%\FSP\FSP-S.fv %SOURCE_FV_FOLDER_PATH%\FSP-S.Fv
    REM Gen PAD for MEU
    GenPad.exe CSE\%CSE_Version%\APLK\MEU\xml\APLKBiosFspW.xml -o CSE\%CSE_Version%\APLK\MEU\input\module_bin\PAD.bin
) else (
    REM [-start-160216-IB07220041-add]
    REM Gen PAD for MEU
    GenPad.exe CSE\%CSE_Version%\APLK\MEU\xml\APLKBios.xml -o CSE\%CSE_Version%\APLK\MEU\input\module_bin\PAD.bin
    REM [-end-160216-IB07220041-add]
)
REM[-end-161123-IB07250310-add]REM

REM Gen BPM for MEU
GenBpmLite.exe -i %SOURCE_FV_FOLDER_PATH%\FVIBBL.Fv %SOURCE_FV_FOLDER_PATH%\IBBFV.Fv %SOURCE_FV_FOLDER_PATH%\RECOVERYFV.Fv -o %SOURCE_FV_FOLDER_PATH%\BPM.bin
pushd CSE\%CSE_Version%\APLK\MEU
if not exist input\module_bin\ (
    mkdir input\module_bin\
)

REM Gen Alignment Ec
if exist input\module_bin\AlignmentEc.bin del input\module_bin\AlignmentEc.bin /F/Q
Copy /b %WORKSPACE%%PROJECT_PKG%\Binary\Ec\Alignment.bin + %WORKSPACE%%PROJECT_PKG%\Binary\Ec\ecb.bin input\module_bin\AlignmentEc.bin
REM[-start-161123-IB07250310-modify]REM
if "%FSP_ENABLE%" == "YES" (
  copy /y /b %SOURCE_FV_FOLDER_PATH%\IBBM.Fv input\module_bin
  copy /y /b %SOURCE_FV_FOLDER_PATH%\FSP-S.Fv input\module_bin
) else (
  copy %SOURCE_FV_FOLDER_PATH%\IBBFV.Fv input\module_bin
)
copy %SOURCE_FV_FOLDER_PATH%\FVIBBL.Fv input\module_bin\
copy %SOURCE_FV_FOLDER_PATH%\RECOVERYFV.Fv input\module_bin
copy %SOURCE_FV_FOLDER_PATH%\FVMAIN_COMPACT.Fv input\module_bin
copy %SOURCE_FV_FOLDER_PATH%\NvStore.Fv input\module_bin
copy %SOURCE_FV_FOLDER_PATH%\BPM.bin input\module_bin
copy %SOURCE_FV_FOLDER_PATH%\NVCOMMON.fd input\module_bin
REM[-end-161123-IB07250310-modify]REM

REM[-start-160330-IB07220055-remove]
REM meu -f xml/ROTKeyManifest.xml -o rotkeymn2.bin -u1 . -key keys/bxt_dbg_priv_key.pem -w out
REM if %ERRORLEVEL% NEQ 0  echo Error in meu & goto Fail
REM move /y rotkeymn2.bin  ..\FIT\input\rotkeymn2.bin
REM if %ERRORLEVEL% NEQ 0  echo Error in move rotkeymn2.bin & goto Fail
REM[-end-160330-IB07220055-remove]
REM [-start-180418-IB07400959-modify]REM
meu -f xml/OEMKeyManifest.xml -o oemkeymn2.bin -u1 . -key keys/bxt_dbg_priv_key_KM.pem -w out
REM [-end-180418-IB07400959-modify]REM
if %ERRORLEVEL% NEQ 0  echo Error in meu & goto Fail
REM [-start-160713-IB07400757-modify]REM
if exist oemkeymn2.bin move /y oemkeymn2.bin %WORKSPACE%\%PROJECT_PKG%\Bios\oemkeymn2.bin > NUL
REM [-end-160713-IB07400757-modify]REM
if %ERRORLEVEL% NEQ 0  echo Error in move oemkeymn2.bin & goto Fail
REM[-start-161123-IB07250310-modify]REM
if "%FSP_ENABLE%" == "YES" (
  meu -f xml/APLKBiosFspW.xml -o bios.bin -u1 . -key keys/bxt_dbg_priv_key.pem -w out -s input
) else (
  meu -f xml/APLKBios.xml -o bios.bin -u1 . -key keys/bxt_dbg_priv_key.pem -w out -s input
)
REM[-end-161123-IB07250310-modify]REM
if %ERRORLEVEL% NEQ 0  echo Error in meu & goto Fail
REM [-start-160713-IB07400757-modify]REM
if exist bios.bin move /y bios.bin %WORKSPACE%\%PROJECT_PKG%\Bios\bios.bin > NUL
if exist bios.map move /y bios.map %WORKSPACE%\%PROJECT_PKG%\Bios\bios.map > NUL
if defined BUILD_BIOS_NAME (
  move /y %WORKSPACE%\%PROJECT_PKG%\Bios\bios.bin %WORKSPACE%\%PROJECT_PKG%\Bios\bios_%BUILD_BIOS_NAME%.bin > NUL
  move /y %WORKSPACE%\%PROJECT_PKG%\Bios\bios.map %WORKSPACE%\%PROJECT_PKG%\Bios\bios_%BUILD_BIOS_NAME%.map > NUL
)
REM [-end-160713-IB07400757-modify]REM
if %ERRORLEVEL% NEQ 0  echo Error in move bios.bin & goto Fail
popd

REM [-start-160420-IB07400719-add]REM
REM
REM Gen Bios.bin and oemkeymn2.bin only 
REM
rmdir CSE\%CSE_Version%\APLK\MEU\input /s /q
rmdir CSE\%CSE_Version%\APLK\MEU\out /s /q
goto Exit
REM [-end-160420-IB07400719-add]REM

REM [-start-160223-IB07400702-modify]REM
echo ISH_Version=%ISH_Version%
REM [-end-160223-IB07400702-modify]REM
REM [-start-151122-IB07220012-modify]
REM Merge ISH
REM [-end-151122-IB07220012-modify]
echo.
echo ------------------------------------------
echo.
echo   Merging ISH [ %ISH_Version% ] ...
pushd ISH\%ISH_Version%
if not exist %ISH_Version%.bin (
  echo ISH %ISH_Version%.bin not found. 
  goto ScriptFail
)
copy /y %ISH_Version%.bin ..\..\CSE\%CSE_Version%\APLK\FIT\input\ISHC.bin

if not exist INTC_pdt_BXT_AOB_BOM_SENSORS (
  echo INTC_pdt_BXT_AOB_BOM_SENSORS not found. 
  goto ScriptFail
)
copy /y INTC_pdt_BXT_AOB_BOM_SENSORS ..\..\CSE\%CSE_Version%\APLK\FIT\input\INTC_pdt_BXT_AOB_BOM_SENSORS
popd
if !ERRORLEVEL! NEQ 0 (
  echo. & echo  -- Error Merging ISH ^^!
  goto ScriptFail
)

REM set IUnit_Version=.
REM Merge IUnit
REM echo.
REM echo ------------------------------------------
REM echo.
REM echo   Merging IUnit [ %IUnit_Version% ] ...
REM pushd IUnit
REM if not exist %IUnit_Version%\IUnit.bin (
REM     echo %IUnit_Version%\IUnit.bin not found. 
REM     goto ScriptFail
REM )
REM copy /y %IUnit_Version%\IUnit.bin ..\CSE\%CSE_Version%\APLK\FIT\input\IUnit.bin
REM popd
REM if !ERRORLEVEL! NEQ 0 (
REM     echo. & echo  -- Error Merging IUnit ^^!
REM     goto ScriptFail
REM )

::
:: Temporary SMIP integration
:: Can be removed once FIT has integrated the layout files
:: at that time, FIT will consume the user-config.xml and create SMIP during IFWI creation
::
echo.
echo ------------------------------------------
echo.
echo   Staging SMIP data...
:: copy CSE and PMC smip to FIT\input
copy /y CSE\%CSE_Version%\APLK\smip_cse.bin  CSE\%CSE_Version%\APLK\FIT\input
copy /y SMIP\smip_iafw.bin  CSE\%CSE_Version%\APLK\FIT\input

REM [-start-160223-IB07400702-modify]REM
echo PMC_Version=%PMC_Version%
REM [-end-160223-IB07400702-modify]REM
REM Staging PMC Patch
echo.
echo ------------------------------------------
echo.
echo   Merging PMC FW [ %PMC_Version% ] ...
if exist PMC\%PMC_Version%\Silicon\pmcp.bin (
    copy /y PMC\%PMC_Version%\Silicon\pmcp.bin CSE\%CSE_Version%\APLK\FIT\input
    if !ERRORLEVEL! NEQ 0 (
        echo. & echo  -- Error Merging PMC FW ^^!
        goto ScriptFail
    )
) else (
    echo PMC\%PMC_Version%\Silicon\pmcp.bin doesn't exist.
)
REM [-start-151122-IB07220012-modify]
REM [-start-160223-IB07400702-modify]REM
REM if "%RVP2%" == "NO" (
REM   set Smip_PMC_Version=smip_pmc_RVP1
REM ) else (
REM   set Smip_PMC_Version=smip_pmc_RVP2
REM )
echo Smip_PMC_Version=%Smip_PMC_Version%
REM [-end-160223-IB07400702-modify]REM
if exist PMC\%PMC_Version%\%Smip_PMC_Version%.bin (
    echo PMC\%PMC_Version%\%Smip_PMC_Version%.bin
    copy /y PMC\%PMC_Version%\%Smip_PMC_Version%.bin CSE\%CSE_Version%\APLK\FIT\input\smip_pmc.bin
    if !ERRORLEVEL! NEQ 0 (
        echo. & echo  -- Error Merging smip_pmc ^^!
        goto ScriptFail
    )
) else (
    echo PMC\%PMC_Version%\%Smip_PMC_Version%.bin doesn't exist.
)
REM [-end-151122-IB07220012-modify]

REM [-start-160223-IB07400702-modify]REM
echo Patch_Version=%Patch_Version%
REM [-end-160223-IB07400702-modify]REM
REM Merge MCU Patch
echo.
echo ------------------------------------------
echo.
echo   Merging MCU Patch [ %Patch_Version% ] ...
pushd Patch
if not exist %Patch_Version%.inc (
    echo Patch %Patch_Version%.inc not found. 
    goto ScriptFail
)
GenFw.exe -m %Patch_Version%.inc -o %Patch_Version%.bin
copy /y %Patch_Version%.bin ..\CSE\%CSE_Version%\APLK\FIT\input\upatch1.bin

popd
if !ERRORLEVEL! NEQ 0 (
    echo. & echo  -- Error Merging ISH ^^!
    goto ScriptFail
)
REM [-start-160223-IB07400702-add]REM
if "%GEN_SPI_BIOS_BINARY%" == "NO" goto SkipGenSpiBinary
REM [-end-160223-IB07400702-add]REM

echo.
echo ------------------------------------------
echo.
echo   Generating SPI Image...
pushd CSE\%CSE_Version%\APLK\FIT
REM[-start-160302-IB07220044-modify]
REM [-start-160304-IB07400707-modify]REM
REM if "%RVP2%" == "NO" (
REM   fit -b -f xml/bxt_spi_8mb_RVP1.xml -o bxt_spi_8mb.bin
REM ) else (
REM   fit -b -f xml/bxt_spi_8mb_RVP2.xml -o bxt_spi_8mb.bin
REM )
  fit -b -f xml/%FIT_XML_SPI%.xml -o bxt_spi_8mb.bin
REM [-end-160304-IB07400707-modify]REM
REM copy /y bxt_spi_8mb.bin APLK_IFWI_X64_D_2015_03_27_0250_SPI_Simics.bin
REM[-end-160302-IB07220044-modify]
REM [-start-160304-IB07400707-add]REM
  if exist Int\gpio_Profile_0.bin (
    move /y Int\gpio_Profile_0.bin %WORKSPACE%\%PROJECT_PKG%\Bios\gpio_Profile_0_spi.bin
  )
REM [-end-160304-IB07400707-add]REM
popd
if !ERRORLEVEL! NEQ 0 (
  echo. & echo  -- Error during FIT Stitching ^^!
  goto ScriptFail
)
REM [-start-160223-IB07400702-remove]REM
REM copy /b %SOURCE_FV_FOLDER_PATH%\IBBFV.Fv+%SOURCE_FV_FOLDER_PATH%\FVIBBL.Fv CSE\%CSE_Version%\APLK\FIT\TopOfNonCse.bin
REM BinMerge.exe CSE\%CSE_Version%\APLK\FIT\bxt_spi_8mb.bin CSE\%CSE_Version%\APLK\FIT\TopOfNonCse.bin -o CSE\%CSE_Version%\APLK\FIT\APLK_IFWI_X64_D_2015_03_27_0250_SPI_Simics_CSE_SRAM.bin
REM [-end-160223-IB07400702-remove]REM

REM [-start-160223-IB07400702-add]REM
move /y %WORKSPACE%\%PROJECT_PKG%\Stitch\CSE\%CSE_Version%\APLK\FIT\bxt_spi_8mb.bin %WORKSPACE%\%PROJECT_PKG%\Bios\%BOARD_NAME%_SPI_%TARGET_ARCH%_%FD_TARGET%_8MB.bin > nul
move /y %WORKSPACE%\%PROJECT_PKG%\Stitch\CSE\%CSE_Version%\APLK\FIT\bxt_spi_8mb.map %WORKSPACE%\%PROJECT_PKG%\Bios\%BOARD_NAME%_SPI_%TARGET_ARCH%_%FD_TARGET%_8MB.map > nul
REM [-start-160311-IB07400709-modify]REM
REM move /y %WORKSPACE%\%PROJECT_PKG%\Stitch\CSE\%CSE_Version%\APLK\FIT\dnx_spi.bin %WORKSPACE%\%PROJECT_PKG%\Bios\DnX\DnX_SPI_%TARGET_ARCH%_%FD_TARGET%.bin > nul
del %WORKSPACE%\%PROJECT_PKG%\Stitch\CSE\%CSE_Version%\APLK\FIT\dnx_spi.bin /q
REM [-end-160311-IB07400709-modify]REM

:SkipGenSpiBinary
if "%GEN_EMMC_BIOS_BINARY%" == "NO" goto SkipGenEmmcBinary
REM [-end-160223-IB07400702-add]REM

echo.
echo ------------------------------------------
echo.
echo   Generating EMMC Image...
pushd CSE\%CSE_Version%\APLK\FIT
REM[-start-160302-IB07220044-modify]
REM [-start-160304-IB07400707-modify]REM
REM if "%RVP2%" == "NO" (
REM   fit -b -f xml/bxt_emmc_8mb_RVP1.xml -o bxt_emmc_8mb.bin
REM ) else (
REM   fit -b -f xml/bxt_emmc_8mb_RVP2.xml -o bxt_emmc_8mb.bin
REM )
  fit -b -f xml/%FIT_XML_EMMC%.xml -o bxt_emmc_8mb.bin
REM [-end-160304-IB07400707-modify]REM
REM copy /y bxt_emmc_8mb(1).bin APLK_IFWI_X64_D_2015_03_27_0250_BP1_Simics.bin
REM copy /y bxt_emmc_8mb(2).bin APLK_IFWI_X64_D_2015_03_27_0250_BP2_Simics.bin
REM[-end-160302-IB07220044-modify]
REM [-start-160304-IB07400707-add]REM
  if exist Int\gpio_Profile_0.bin (
    move /y Int\gpio_Profile_0.bin %WORKSPACE%\%PROJECT_PKG%\Bios\gpio_Profile_0_eMMC.bin
  )
REM [-end-160304-IB07400707-add]REM
popd
if !ERRORLEVEL! NEQ 0 (
  echo. & echo  -- Error during FIT Stitching ^^!
  goto ScriptFail
)

REM [-start-160223-IB07400702-remove]REM
REM copy /b /y %WORKSPACE%\%PROJECT_PKG%\Stitch\CSE\%CSE_Version%\APLK\FIT\bxt_emmc_8mb.bin %WORKSPACE%\%PROJECT_PKG%\Bios\%BOARD_NAME%_EMMC_%TARGET_ARCH%_%FD_TARGET%_8MB.bin > nul
REM copy /b /y %WORKSPACE%\%PROJECT_PKG%\Stitch\CSE\%CSE_Version%\APLK\FIT\bxt_spi_8mb.bin %WORKSPACE%\%PROJECT_PKG%\Bios\%BOARD_NAME%_SPI_%TARGET_ARCH%_%FD_TARGET%_8MB.bin > nul
REM copy /b /y %WORKSPACE%\%PROJECT_PKG%\Stitch\CSE\%CSE_Version%\APLK\FIT\dnx_emmc.bin %WORKSPACE%\%PROJECT_PKG%\Bios\DnX_EMMC_%TARGET_ARCH%_%FD_TARGET%.bin > nul
REM copy /b /y %WORKSPACE%\%PROJECT_PKG%\Stitch\CSE\%CSE_Version%\APLK\FIT\bxt_emmc_8mb(1).bin %WORKSPACE%\%PROJECT_PKG%\Bios\%BOARD_NAME%_EMMC_%TARGET_ARCH%_%FD_TARGET%_8MB_1.bin > nul
REM copy /b /y %WORKSPACE%\%PROJECT_PKG%\Stitch\CSE\%CSE_Version%\APLK\FIT\bxt_emmc_8mb(2).bin %WORKSPACE%\%PROJECT_PKG%\Bios\%BOARD_NAME%_EMMC_%TARGET_ARCH%_%FD_TARGET%_8MB_2.bin > nul
REM [-end-160223-IB07400702-remove]REM

REM [-start-160223-IB07400702-add]REM
REM [-start-160311-IB07400709-modify]REM
REM move /y %WORKSPACE%\%PROJECT_PKG%\Stitch\CSE\%CSE_Version%\APLK\FIT\bxt_emmc_8mb.bin %WORKSPACE%\%PROJECT_PKG%\Bios\%BOARD_NAME%_EMMC_%TARGET_ARCH%_%FD_TARGET%_8MB.bin > nul
del %WORKSPACE%\%PROJECT_PKG%\Stitch\CSE\%CSE_Version%\APLK\FIT\bxt_emmc_8mb.bin /q
REM [-end-160311-IB07400709-modify]REM
move /y %WORKSPACE%\%PROJECT_PKG%\Stitch\CSE\%CSE_Version%\APLK\FIT\bxt_emmc_8mb.map %WORKSPACE%\%PROJECT_PKG%\Bios\%BOARD_NAME%_EMMC_%TARGET_ARCH%_%FD_TARGET%_8MB.map > nul
REM [-start-160311-IB07400709-modify]REM
move /y %WORKSPACE%\%PROJECT_PKG%\Stitch\CSE\%CSE_Version%\APLK\FIT\dnx_emmc.bin %WORKSPACE%\%PROJECT_PKG%\Bios\DnX\DnX_EMMC_%TARGET_ARCH%_%FD_TARGET%.bin > nul
REM [-end-160311-IB07400709-modify]REM
move /y %WORKSPACE%\%PROJECT_PKG%\Stitch\CSE\%CSE_Version%\APLK\FIT\bxt_emmc_8mb(1).bin %WORKSPACE%\%PROJECT_PKG%\Bios\%BOARD_NAME%_EMMC_%TARGET_ARCH%_%FD_TARGET%_8MB_1.bin > nul
move /y %WORKSPACE%\%PROJECT_PKG%\Stitch\CSE\%CSE_Version%\APLK\FIT\bxt_emmc_8mb(2).bin %WORKSPACE%\%PROJECT_PKG%\Bios\%BOARD_NAME%_EMMC_%TARGET_ARCH%_%FD_TARGET%_8MB_2.bin > nul

:SkipGenEmmcBinary

echo.
echo  ------------  Clear Temp files  ------------
rmdir CSE\%CSE_Version%\APLK\FIT\input /s /q
REM [-start-160304-IB07400707-add]REM
rmdir CSE\%CSE_Version%\APLK\FIT\Int /s /q
REM [-end-160304-IB07400707-add]REM
rmdir CSE\%CSE_Version%\APLK\MEU\input /s /q
rmdir CSE\%CSE_Version%\APLK\MEU\out /s /q
del CSE\%CSE_Version%\APLK\FIT\*.xml /q 
del CSE\%CSE_Version%\APLK\FIT\*.log /q
del CSE\%CSE_Version%\APLK\FIT\Smip* /q
REM [-end-160223-IB07400702-add]REM

echo.
echo  ----------  IFWI_Stitch Complete  ----------
REM [-start-160223-IB07400702-add]REM
@echo TXE FW      Version: %CSE_Version%
@echo ISH FW      Version: %ISH_Version%
@echo PMC FW      Version: %PMC_Version%
REM [-start-160629-IB07400750-remove]REM
REM @echo PMC SMIP    Version: %Smip_PMC_Version%
REM [-end-160629-IB07400750-remove]REM
@echo Microcode1  Version: %Patch_Version%
REM [-start-160304-IB07400707-add]REM
if "%GEN_SPI_BIOS_BINARY%" == "YES" (
@echo FIT SPI Config  XML: %FIT_XML_SPI%.xml
)
if "%GEN_EMMC_BIOS_BINARY%" == "YES" (
@echo FIT EMMC Config XML: %FIT_XML_EMMC%.xml
)
REM [-end-160304-IB07400707-add]REM
echo  --------------------------------------------
echo.
REM [-end-160223-IB07400702-add]REM
goto Exit

:ScriptFail
set exitCode=1 

:Exit
exit /b %exitCode%
EndLocal
