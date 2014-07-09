@echo off
rem As of 9 July 2014, original version of this script has some problems:
rem 1) pkzip25 is long since obsolete
rem 2) The commands used (-dir) include some junk (.jpg files under doc, for example)
rem    and miss some good stuff (templates.txt under scripts, for example)
rem 3) Doesn't include the Visual Studio redistributable
rem 4) The source code bundling is even more likely to miss things.
rem
rem So we just open the file and you can read the instructions to make the ZIP by hand.
rem If that offends you, go ahead and fix it.
notepad release.bat
exit /b

To make a release:
1) Create a ZIP file with a name like "vts-3.6.2.zip"
2) Copy the exe and dll files into the root of the zip file:
   - release\VTS.exe
   - release\ptp.dll 
   - NBLink\release\nb_link_settings.dll 
   - the WinPCAP installer, WinPcap_4_1_3.exe or newer.
     Most people will already have WinPCAP installed by WireShark.
     We should include the current installer from the WinPCAP site.
     Obviously you should verify that that this version WORKS with VTS
     before including ti in the installer.
   - the Visual Studio redistributable installer vcredist_x86.exe
     This may usually be found somewhere like
     C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\Bootstrapper\Packages\vcredist_x86\vcredist_x86.exe)
3) Copy the documentation folder "Docs" as a folder into the zip file
4) Copy the script folder "scripts" as a folder into the zip file

I don't see the point of a source zip: is svn really so hard?
If you want to:
1) Use svn to get a clean copy of the source.  DO NOT build it
2) ZIP up the whole damn thing.



rem the original contents (with flaws mentioned above) follow

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

rem cd docs
rem pkzip25 -add -path ..\%1-win32.zip *.html images\*.png images\*.jpg *.doc *.tpi
rem cd ..
rem Copy the documents stuff
pkzip25 -add -dir %1-win32.zip *.html *.png *.jpg *.gif *.doc *.tpi

pkzip25 -add -dir %1-win32.zip *.vts
REM --- Then make the source distribution ---
del %1-source.zip
pkzip25 -add -dir %1-source.zip *.cpp *.hpp *.c *.h *.inl 
pkzip25 -add -dir %1-source.zip *.bmp *.cur *.ico *.rc *.rc2 *.dsp *.dsw *.lib *.vts *.def *.doc *.html *.png *.jpg *.tpi
REM pkzip25 -add -path %1-source.zip bacmacNT\*.*
pkzip25 -add -path %1-source.zip winpcap\*.exe
pkzip25 -add -dir %1-source.zip nblink\*.dll 

