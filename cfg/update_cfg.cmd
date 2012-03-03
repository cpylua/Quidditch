@echo Updating game configuration database...
@call bld_cfgbld.cmd | findstr /I /C:" : error" || cfgbld.exe config.cfg
@copy /Y config.cfg ..\TuringCup9 >nul
@copy /Y config.cfg ..\Release >nul
