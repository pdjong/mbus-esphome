@echo off
set targetDir=z:\esphome

REM YML file
set ymlFile=.\warmte-meter-mbus.yaml
copy %ymlFile% %targetDir%

REM Component files
set customComponentsSrcDir=.\custom-components\heatmeter_mbus
set customComponentsTargetDir=%targetDir%\custom_components\heatmeter_mbus
copy %customComponentsSrcDir%\__init__.py %customComponentsTargetDir%
copy %customComponentsSrcDir%\sensor.py %customComponentsTargetDir%
REM copy %customComponentsSrcDir%\Adc.cpp %customComponentsTargetDir%
REM copy %customComponentsSrcDir%\Adc.h %customComponentsTargetDir%
copy %customComponentsSrcDir%\HeatMeterMbus.cpp %customComponentsTargetDir%
copy %customComponentsSrcDir%\HeatMeterMbus.h %customComponentsTargetDir%
@REM copy %customComponentsSrcDir%\Kamstrup303WA02.cpp %customComponentsTargetDir%
@REM copy %customComponentsSrcDir%\Kamstrup303WA02.h %customComponentsTargetDir%
@REM copy %customComponentsSrcDir%\Pwm.cpp %customComponentsTargetDir%
@REM copy %customComponentsSrcDir%\Pwm.h %customComponentsTargetDir%
