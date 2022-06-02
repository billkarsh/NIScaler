
:: You can call NIScaler three ways:
::
:: 1) > NIScaler cmd-line-parameters
:: 2) > runit.bat cmd-line-parameters
:: 3a) Edit parameters in runit.bat, then call it ...
:: 3b) > runit.bat
::
:: This script effectively says:
:: "If there are no parameters sent to runit.bat, call NIScaler
:: with the parameters hard coded here, else, pass all of the
:: parameters through to NIScaler."
::

@echo off
@setlocal enableextensions
@cd /d "%~dp0"

set LOCALARGS=-apply -cal_dir=D:/NIFIX ^
-src_dir="D:/catgt_test_data/SC024_092319_NP1.0_Midbrain_g0" ^
-dst_dir=D:/NIFIX

if [%1]==[] (set ARGS=%LOCALARGS%) else (set ARGS=%*)

%~dp0NIScaler %ARGS%

