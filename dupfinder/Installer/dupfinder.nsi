#
#   Duplicate Files Finder - search for duplicate files
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
!include MUI2.nsh

Name "Duplicate Files Finder"

OutFile "DupFinderSetup.exe"

InstallDir "$PROGRAMFILES\Duplicate Files Finder"

InstallDirRegKey HKLM "Software\Duplicate Files Finder" "Install_Dir"

; Pages:

!define MUI_ABORTWARNING

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE ..\license.txt
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
Var StartMenuFolder
!insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

Section "Main" MainSection

	SectionIn RO

	SetOutPath $INSTDIR

	Call GetWindowsVersion
	Pop $R0

	File "/oname=readme.txt" "..\readme.txt"
	File "/oname=license.txt" "..\license.txt"
	; supporting unicode ?
	${If} $R0 == "95" 
	${OrIf} $R0 == "98" 
	${OrIf} $R0 == "ME" 
		File "/oname=dupf.exe" ..\gcc\dupf.exe
		File "/oname=dupfgui.exe" ..\gcc\dupfgui.exe

		FileOpen $1 $INSTDIR\dupfcon.bat w
		FileWrite $1 `cd "$INSTDIR"`
		FileClose $1
	${Else}
		File "/oname=dupf.exe" ..\gccu\dupf.exe
		File "/oname=dupfgui.exe" ..\gccu\dupfgui.exe
	${EndIf}
	File "/oname=mingwm10.dll" C:\mingw\bin\mingwm10.dll

	; Store Installation dir
	WriteRegStr HKLM "SOFTWARE\Duplicate Files Finder" "Install_dir" "$INSTDIR"

	; Uninstall keys
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Duplicate Files Finder" "DisplayName" "Duplicate Files Finder"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Duplicate Files Finder" "UninstallString" '"$INSTDIR\uninstall.exe"'
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Duplicate Files Finder" "NoModify" "1"
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Duplicate Files Finder" "NoRepair" "1"
	WriteUninstaller "uninstall.exe"

!insertmacro MUI_STARTMENU_WRITE_BEGIN Application

	CreateDirectory "$SMPROGRAMS\Duplicate Files Finder"
	CreateShortCut "$SMPROGRAMS\Duplicate Files Finder\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
	CreateShortCut "$SMPROGRAMS\Duplicate Files Finder\Duplicate Files Finder.lnk" "$INSTDIR\dupfgui.exe" "" "$INSTDIR\dupfgui.exe" 0
	CreateShortCut "$SMPROGRAMS\Duplicate Files Finder\Readme.lnk" "$INSTDIR\readme.txt"

	Call GetWindowsVersion
	Pop $R0
	${If} $R0 == '95'
	${OrIf} $R0 == '98'
	${OrIf} $R0 == 'ME'
		CreateShortCut "$SMPROGRAMS\Duplicate Files Finder\Text mode only (type dupf).lnk" \
			"$WINDIR\command.com" '/K "$INSTDIR\dupfcon.bat"'  "cmd.exe" 0 
	${Else}
		CreateShortCut "$SMPROGRAMS\Duplicate Files Finder\Text mode only (type dupf).lnk" \
			"$SYSDIR\cmd.exe" '/K "cd $INSTDIR"'  "cmd.exe" 0 
	${Endif}

!insertmacro MUI_STARTMENU_WRITE_END

SectionEnd

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
	!insertmacro MUI_DESCRIPTION_TEXT ${MainSection} "All"
!insertmacro MUI_FUNCTION_DESCRIPTION_END

Section "Uninstall"

	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Duplicate Files Finder"
	DeleteRegKey HKLM "SOFTWARE\Duplicate Files Finder"
	
	Delete $INSTDIR\readme.txt
	Delete $INSTDIR\license.txt
	Delete $INSTDIR\dupf.exe
	Delete $INSTDIR\dupfgui.exe
	Delete $INSTDIR\mingwm10.dll

	Call un.GetWindowsVersion
	Pop $R0
	${If} $R0 == '95'
	${OrIf} $R0 == '98'
	${OrIf} $R0 == 'ME'
		Delete $INSTDIR\dupfcon.bat
	${Endif}


	Delete $INSTDIR\uninstall.exe

	Delete "$SMPROGRAMS\Duplicate Files Finder\*.*"

	RMDir "$SMPROGRAMS\Duplicate Files Finder"
	RMDir "$INSTDIR"

SectionEnd



