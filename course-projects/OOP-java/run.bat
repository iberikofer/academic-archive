@echo off
title TORNADO GYM Simulator
set "JAVA_HOME=E:\.jdks\openjdk-25.0.2"
set "PATH=%JAVA_HOME%\bin;%PATH%"
set "MAVEN_OPTS=--enable-native-access=ALL-UNNAMED --sun-misc-unsafe-memory-access=allow"
cd /d "%~dp0"
echo Starting TORNADO GYM Simulator...
call mvnw.cmd javafx:run
pause
