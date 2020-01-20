@echo off

echo.
echo  Cleaning Stitch/CSE Folders...
echo.
pushd CSE\
for /D /r %%G in ("APLK*" "FIT*" "MEU*" "input*" "keys*" "module_bin*" "openssl*" "out*" "xml*" "Silicon*" "Int*") do (
  echo    checking %%G...
  erase /q /f /a:-h %%G\*.* > nul 2>&1
)
popd

echo.
echo  Cleaning Stitch/ISH Folders...
echo.
pushd ISH\
  erase /q /s *.bin > nul 2>&1
  erase /q /s INTC_pdt_BXT_AOB_BOM_SENSORS > nul 2>&1
popd

echo.
echo  Cleaning Stitch/Patch Folders...
echo.
pushd Patch\
  erase /q *.bin > nul 2>&1
  erase /q *.inc > nul 2>&1
popd

echo.
echo  Cleaning Stitch/PMC Folders...
echo.
pushd PMC\
  erase /q /s *.bin > nul 2>&1
popd

echo.
echo  Cleaning Stitch/SMIP Folders...
echo.
pushd SMIP\
  erase /q smip_iafw.bin > nul 2>&1
popd

echo.
echo All done...
echo.
