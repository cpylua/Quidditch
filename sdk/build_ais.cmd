@ECHO OFF
ECHO Deleting old files...
DEL .\ais\bin\*.* /Q /F >nul

ECHO Creating backup files...
REN .\src\Tc_Client.cpp Tc_Client.bak >nul

FOR %%i in (.\ais\src\*.cpp) DO ECHO+ && COPY /Y "%%i" .\src\Tc_Client.cpp >nul && ECHO Building %%~ni && ECHO+ && VCBUILD /nologo /rebuild .\src\TC9Client_VS2008.vcproj Release | FINDSTR /I /C:" : error" || COPY /B /Y ".\src\Release_VS2008\bin\*.dll" ".\ais\bin\%%~ni.dll" >nul

dir .\ais\bin\*.dll | findstr /R /C:"File(s)"

ECHO Restoring files...
DEL .\src\Tc_Client.cpp >nul
REN .\src\Tc_Client.bak Tc_Client.cpp >nul
