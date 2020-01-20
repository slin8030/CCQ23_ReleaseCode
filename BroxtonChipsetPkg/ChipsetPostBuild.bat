@REM
@REM Chipset PostBuild batch file
@REM
@REM ******************************************************************************
@REM * Copyright (c) 2013 -2014, Insyde Software Corp. All Rights Reserved.
@REM *
@REM * You may not reproduce, distribute, publish, display, perform, modify, adapt,
@REM * transmit, broadcast, present, recite, release, license or otherwise exploit
@REM * any part of this publication in any form, by any means, without the prior
@REM * written permission of Insyde Software Corporation.
@REM *
@REM ******************************************************************************

@REM     Add Chispet specific post-build process here


find /v "#" %WORKSPACE%\Conf\target.txt > string
for /f "tokens=1,2,3" %%i in (string) do set %%i=%%k
del string

for /f "tokens=1,2,3,4" %%i in (%WORKSPACE%\Build\%PROJECT_PKG%\Project.env) do set %%j=%%l

if "%X64_CONFIG%" == "NO" (
  set TARGET_ARCH=Ia32
) else (
  set TARGET_ARCH=X64
)

copy /y %WORKSPACE%\Build\%PROJECT_PKG%\%TARGET%_%TOOL_CHAIN%\FV\Chv.fd %WORKSPACE%\Build\%PROJECT_PKG%\%TARGET%_%TOOL_CHAIN%\FV\%TARGET_BIN%.fd > NUL

