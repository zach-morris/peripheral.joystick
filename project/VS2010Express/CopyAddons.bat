@ECHO OFF

SETLOCAL

IF [%KODI_HOME%] == [] GOTO KodiHomeError

SET CUR_PATH="%CD%"
SET SCRIPT_PATH="%~dp0"

cd "%SCRIPT_PATH%..\..\addons"

FOR /F "tokens=*" %%S IN ('dir /B "peripheral.*"') DO (
  echo Copying %%S
  xcopy "%%S\addon" "%KODI_HOME%\addons\%%S" /I /Q /R /Y
)

cd "%CUR_PATH%"
GOTO END

:KodiHomeError
echo "Set KODI_HOME environment variable to copy add-ons to Kodi branch"

:END
