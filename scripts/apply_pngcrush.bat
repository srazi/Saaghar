@echo off

set SAAGHAR_SRC=".."
set PNG_CRUSH="pngcrush.exe"

pushd %SAAGHAR_SRC%
for /r %%x in (*.png) do call :convert %%x %%~nx

GOTO :finish

:convert
set inFile="%~1"
set inFileName="%~2.png"
set tempFile="%~1.2"
echo ~~~~~~~~~~~~~~~~~~~~
echo %inFile%
echo %tempFile%
echo ~~~~~~~~~~~~~~~~~~~~

%PNG_CRUSH% %inFile% %tempFile%
del %inFile%
rename %tempFile% %inFileName%

:finish

popd