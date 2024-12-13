@echo off

pushd ..\build\
cl /Zi /Ifreetype\include ..\code\gol.cpp /link /LIBPATH:freetype user32.lib gdi32.lib Ole32.lib freetype.lib
popd