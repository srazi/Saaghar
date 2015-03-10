@echo off

set SAAGHAR_SRC=".."
set PNG_CRUSH="./pngcrush"

pushd %SAAGHAR_SRC%
for /r %%x in (*.png) do call :convert %%x

GOTO :finish

:convert
set inFile="%~1"
set tempFile="%~1.2"
echo ~~~~~~~~~~~~~~~~~~~~
echo %inFile%
echo %tempFile%
echo ~~~~~~~~~~~~~~~~~~~~

%PNG_CRUSH% %inFile% %tempFile%
del %inFile%

:finish

popd