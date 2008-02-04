#
#   dupfinder - search for duplicate files
#   Copyright (C) 2007-2008 Matthias Boehm
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#

!include GetWindowsVersion.nsh
!include LogicLib.nsh

Name "DupFinder"

OutFile "DupFinderSetup.exe"

InstallDir $PROGRAMFILES\DupFinder

InstallDirRegKey HKLM "Software\DupFinder" "Install_Dir"

; Pages:

PageEx license
	LicenseData ..\license.txt
PageExEnd
Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

Section "Main"

	SectionIn RO

	SetOutPath $INSTDIR

	Call GetWindowsVersion
	Pop $R0

	File "/oname=readme.txt" "..\readme.txt"
	File "/oname=readme.txt" "..\license.txt"
	; supporting unicode ?
	${If} $R0 == "95" 
	${OrIf} $R0 == "98" 
	${OrIf} $R0 == "ME" 
		File "/oname=dupf.exe" ..\gcc\dupf.exe
		File "/oname=dupfgui.exe" ..\gcc\dupfgui.exe
	${Else}
		File "/oname=dupf.exe" ..\gccu\dupf.exe
		File "/oname=dupfgui.exe" ..\gccu\dupfgui.exe
	${EndIf}
	File "/oname=mingwm10.dll" C:\mingw\bin\mingwm10.dll

	; Store Installation dir
	WriteRegStr HKLM SOFTWARE\DupFinder "Install_dir" "$INSTDIR"

	; Uninstall keys
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DupFinder" "DisplayName" "DupFinder"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DupFinder" "UninstallString" '"$INSTDIR\uninstall.exe"'
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DupFinder" "NoModify" "1"
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DupFinder" "NoRepair" "1"
	WriteUninstaller "uninstall.exe"

SectionEnd

Section "Start Menu shortcuts"

	CreateDirectory "$SMPROGRAMS\DupFinder"
	CreateShortCut "$SMPROGRAMS\DupFinder\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
	CreateShortCut "$SMPROGRAMS\DupFinder\DupFinder.lnk" "$INSTDIR\dupfgui.exe" "" "$INSTDIR\dupfgui.exe" 0

	Call GetWindowsVersion
	Pop $R0
	${If} $R0 == '95'
	${OrIf} $R0 == '98'
	${OrIf} $R0 == 'ME'
		; command.com does not support what i want
	${Else}
		CreateShortCut "$SMPROGRAMS\DupFinder\Console mode DupFinder (type dupf).lnk" \
			"$SYSDIR\cmd.exe" '/K "cd $INSTDIR"'  "cmd.exe" 0 
	${Endif}


SectionEnd

Section "Uninstall"

	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DupFinder"
	DeleteRegKey HKLM SOFTWARE\DupFinder
	
	Delete $INSTDIR\readme.txt
	Delete $INSTDIR\license.txt
	Delete $INSTDIR\dupf.exe
	Delete $INSTDIR\dupfgui.exe
	Delete $INSTDIR\mingwm10.dll

	Delete $INSTDIR\uninstall.exe

	Delete $SMPROGRAMS\DupFinder\*.*

	RMDir "$SMPROGRAMS\DupFinder"
	RMDir "$INSTDIR"

SectionEnd