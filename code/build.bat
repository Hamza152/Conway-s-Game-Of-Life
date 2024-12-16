@echo off

pushd ..\build\
cl /Zi /Ifreetype\include /Id3d11\include ..\code\gol.cpp /link /LIBPATH:freetype /LIBPATH:d3d11\Lib user32.lib gdi32.lib Ole32.lib freetype.lib d3d11.lib
popd