@REM IBB override add here
@echo IBB Restore.
    REM ------------Restore InsydeModulePkg\Package.fdf file -------------#
    @if exist %WORKSPACE%\InsydeModulePkg\Package_.fdf (
        copy %WORKSPACE%\InsydeModulePkg\Package_.fdf %WORKSPACE%\InsydeModulePkg\Package.fdf /y
        del  %WORKSPACE%\InsydeModulePkg\Package_.fdf
    )

    REM ------------Restore MdeModulePkg\Package.fdf file -------------#
    @if exist %WORKSPACE%\MdeModulePkg\Package_.fdf (
        copy %WORKSPACE%\MdeModulePkg\Package_.fdf %WORKSPACE%\MdeModulePkg\Package.fdf /y
        del  %WORKSPACE%\MdeModulePkg\Package_.fdf
    )

REM[-start-161123-IB07250310-add]REM
@for /f "tokens=4" %%a in ('find "FSP_ENABLE" %WORKSPACE%\%PROJECT_PKG%\Project.env') do @set FSP_ENABLE=%%a
if "%FSP_ENABLE%" == "YES" (
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