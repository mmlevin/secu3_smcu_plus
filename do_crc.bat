@echo off
rem Batch file for generating check sum for firmware of SECU-3 SMCU PLUS project
rem Created by Maksim M. Levin, Russia, Voronezh. 

set HEXTOBIN=hextobin.exe
set CODECRC=codecrc.exe
set FW_SIZE=8190
set CRC_ADDR=1FFE

echo EXECUTING BATCH...
echo ---------------------------------------------

cd %~dp0\Release\

rem Convert HEX-file created by compiler into a binary file
for %%X in (%HEXTOBIN%) do (set FOUND_H2B=%~dp0)
if not defined FOUND_H2B (
 echo ERROR: Can not find file "%HEXTOBIN%"
 goto error
)
%~dp0\%HEXTOBIN% secu3_smcu_plus_app.a90 secu3_smcu_plus_app.bin
IF ERRORLEVEL 1 GOTO error

rem Calculate and put check sum into a binary file
for %%X in (%CODECRC%) do (set FOUND_CRC=%~dp0)
if not defined FOUND_CRC (
 echo ERROR: Can not find file "%CODECRC%"
 goto error
)
%~dp0\%CODECRC% secu3_smcu_plus_app.bin secu3_smcu_plus_app.a90  0  %FW_SIZE%  %CRC_ADDR% -h
IF ERRORLEVEL 1 GOTO error
%~dp0\%CODECRC% secu3_smcu_plus_app.bin secu3_smcu_plus_app.bin  0  %FW_SIZE%  %CRC_ADDR% -b
IF ERRORLEVEL 1 GOTO error

echo ---------------------------------------------
echo ALL OPERATIONS WERE COMPLETED SUCCESSFULLY!
exit 0

:error
echo ---------------------------------------------
echo WARNING! THERE ARE SOME ERRORS IN EXECUTING BATCH.
exit 1
