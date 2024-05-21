@echo off

pushd ..\build\
cl /Zi ..\code\gol.cpp user32.lib gdi32.lib Ole32.lib
popd