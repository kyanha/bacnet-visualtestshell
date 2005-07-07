@echo off
REM This batch file constructs the distribution files for VTS 3.x
REM It requires the command line version of pkzip 2.5 for Win32.
REM
REM The user must specify the root name of the release as the first
REM command line argument, i.e. "vts-3.4.5"
REM
REM This should be invoked from the VTS3 root.
REM
REM "-dir" is the same as combining "-path" and "-recurse"
REM  
REM --- First make the executable distribution ---
del %1-win32.zip
pkzip25 -add %1-win32.zip release\VTS.exe release\ptp.dll winpcap\*.exe
pkzip25 -add %1-win32.zip NBLink\release\nb_link_settings.dll 
cd docs
pkzip25 -add -path ..\%1-win32.zip *.html images\*.png images\*.jpg *.doc
cd ..
pkzip25 -add -dir %1-win32.zip *.vts
REM --- Then make the source distribution ---
del %1-source.zip
pkzip25 -add -dir %1-source.zip *.cpp *.hpp *.c *.h *.inl 
pkzip25 -add -dir %1-source.zip *.bmp *.cur *.ico *.rc *.rc2 *.dsp *.dsw *.lib *.vts *.def
REM pkzip25 -add -path %1-source.zip bacmacNT\*.*
pkzip25 -add -path %1-source.zip winpcap\*.exe
pkzip25 -add -dir %1-source.zip nblink\*.dll 

