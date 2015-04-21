@echo off

call udpsender.exe "127.0.0.1" 27015 "hogehoge"
echo %ERRORLEVEL%

pause
