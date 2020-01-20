@REM IBB override add here
@echo IBB Fixup.
    REM ------------Fixup InsydeModulePkg\Package.fdf file -------------#
    @if exist %WORKSPACE%\InsydeModulePkg\Package_.fdf (
        copy %WORKSPACE%\%CHIPSET_PKG%\Override\InsydeModulePkg\Package.fdf %WORKSPACE%\InsydeModulePkg\Package.fdf /y
    ) else (
        copy %WORKSPACE%\InsydeModulePkg\Package.fdf %WORKSPACE%\InsydeModulePkg\Package_.fdf /y
        copy %WORKSPACE%\%CHIPSET_PKG%\Override\InsydeModulePkg\Package.fdf %WORKSPACE%\InsydeModulePkg\Package.fdf /y
    )

    REM ------------Fixup MdeModulePkg\Package.fdf file -------------#
    @if exist %WORKSPACE%\MdeModulePkg\Package_.fdf (
        copy %WORKSPACE%\%CHIPSET_PKG%\Override\MdeModulePkg\Package.fdf %WORKSPACE%\MdeModulePkg\Package.fdf /y
    ) else (
        copy %WORKSPACE%\MdeModulePkg\Package.fdf %WORKSPACE%\MdeModulePkg\Package_.fdf /y
        copy %WORKSPACE%\%CHIPSET_PKG%\Override\MdeModulePkg\Package.fdf %WORKSPACE%\MdeModulePkg\Package.fdf /y
    )

REM[-start-161123-IB07250310-add]REM
if "%FSP_ENABLE_BUILD%" == "YES" (
    REM ------------Fixup ApolloLakeBoardPkg\Project.env file -------------#
    @if not exist %WORKSPACE%\%PROJECT_PKG%\Project_.env (
        copy %WORKSPACE%\%PROJECT_PKG%\Project.env %WORKSPACE%\%PROJECT_PKG%\Project_.env /y
    )
    call SetFspFlag.js
) else (
    REM ------------Restore ApolloLakeBoardPkg\Project.env file -------------#
    @if exist %WORKSPACE%\%PROJECT_PKG%\Project_.env (
        copy %WORKSPACE%\%PROJECT_PKG%\Project_.env %WORKSPACE%\%PROJECT_PKG%\Project.env /y
        del  %WORKSPACE%\%PROJECT_PKG%\Project_.env
    )
)
@for /f "tokens=4" %%a in ('find "FSP_ENABLE" %WORKSPACE%\%PROJECT_PKG%\Project.env') do @set FSP_ENABLE=%%a
if "%FSP_ENABLE%" == "YES" (
    REM ------------Fixup Conf\tools_def.txt file -------------#
    @if exist %WORKSPACE%\Conf\tools_def_.txt (
        copy %WORKSPACE%\%CHIPSET_PKG%\Override\Conf\tools_def.txt %WORKSPACE%\Conf\tools_def.txt /y
    ) else (
        copy %WORKSPACE%\Conf\tools_def.txt %WORKSPACE%\Conf\tools_def_.txt /y
        copy %WORKSPACE%\%CHIPSET_PKG%\Override\Conf\tools_def.txt %WORKSPACE%\Conf\tools_def.txt /y
    )
    REM ------------Fixup MdePkg\Include\Library\PcdLib.h file -------------#
    @if exist %WORKSPACE%\MdePkg\Include\Library\PcdLib_.h (
        copy %WORKSPACE%\%CHIPSET_PKG%\Override\MdePkg\Include\Library\PcdLib.h %WORKSPACE%\MdePkg\Include\Library\PcdLib.h /y
    ) else (
        copy %WORKSPACE%\MdePkg\Include\Library\PcdLib.h %WORKSPACE%\MdePkg\Include\Library\PcdLib_.h /y
        copy %WORKSPACE%\%CHIPSET_PKG%\Override\MdePkg\Include\Library\PcdLib.h %WORKSPACE%\MdePkg\Include\Library\PcdLib.h /y
    )
) else (
    REM ------------Restore Conf\tools_def.txt file -------------#
    @if exist %WORKSPACE%\Conf\tools_def_.txt (
        copy %WORKSPACE%\Conf\tools_def_.txt %WORKSPACE%\Conf\tools_def.txt /y
        del  %WORKSPACE%\Conf\tools_def_.txt
    )
    REM ------------Restore MdePkg\Include\Library\PcdLib.h file -------------#
    @if exist %WORKSPACE%\MdePkg\Include\Library\PcdLib_.h (
        copy %WORKSPACE%\MdePkg\Include\Library\PcdLib_.h %WORKSPACE%\MdePkg\Include\Library\PcdLib.h /y
        del  %WORKSPACE%\MdePkg\Include\Library\PcdLib_.h
    )
)
REM[-end-161123-IB07250310-add]REM