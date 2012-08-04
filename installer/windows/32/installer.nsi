
Name "HydraDesktop"

OutFile "HydraDesktopSetup.exe"

InstallDir $PROGRAMFILES\WexusSoftware\HydraDesktop

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\WexusSoftware_HydraDesktop" "Install_Dir"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

LicenseText "License - MIT"
LicenseData ..\..\..\LICENSE.MIT.txt

Page license
Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

Section "HydraDesktop (required)"

  SectionIn RO

  SetOutPath $INSTDIR

  File HydraDesktop.exe.manifest
  File HydraDesktop.exe
  File hydratag.exe.manifest
  File hydratag.exe
  File hydraweb.exe.manifest
  File hydraweb.exe

  File Microsoft.VC90.CRT.manifest
  File msvcm90.dll
  File msvcp90.dll
  File msvcr90.dll

  File imageformats\Microsoft.VC90.CRT.manifest
  File imageformats\msvcm90.dll
  File imageformats\msvcp90.dll
  File imageformats\msvcr90.dll

  File QtCore4.dll
  File QtGui4.dll
  File QtXml4.dll

  File imageformats\qgif4.dll
  File imageformats\qjpeg4.dll
  File imageformats\qtiff4.dll

  WriteRegStr HKLM "SOFTWARE\WexusSoftware_HydraDesktop" "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\WexusSoftware_HydraDesktop" "DisplayName" "HydraDesktop"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\WexusSoftware_HydraDesktop" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\WexusSoftware_HydraDesktop" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\WexusSoftware_HydraDesktop" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
  ; register the file extension
  ;WriteRegStr HKCR ".psbk" "" "HydraDesktop.Document"
  ;WriteRegStr HKCR "HydraDesktop.Document" "" "HydraDesktop Book"
  ;WriteRegStr HKCR "HydraDesktop.Document\DefaultIcon" "" "$INSTDIR\HydraDesktop.exe,0"
  ;WriteRegStr HKCR "HydraDesktop.Document\shell\open\command" "" '"$INSTDIR\HydraDesktop.exe" "%1"'

SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"

  ; dont create a startmenu sub dir, yet...

  ;CreateDirectory "$SMPROGRAMS\Example2"
  ;CreateShortCut "$SMPROGRAMS\Example2\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  ;CreateShortCut "$SMPROGRAMS\Example2\Example2 (MakeNSISW).lnk" "$INSTDIR\example2.nsi" "" "$INSTDIR\example2.nsi" 0
  CreateShortCut "$SMPROGRAMS\HydraDesktop.lnk" "$INSTDIR\HydraDesktop.exe" "" "$INSTDIR\HydraDesktop.exe" 0
  
SectionEnd

; Uninstaller

Section "Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\WexusSoftware_HydraDesktop"
  DeleteRegKey HKLM SOFTWARE\WexusSoftware_HydraDesktop

  ; Remove files and uninstaller
  Delete $INSTDIR\HydraDesktop.exe.manifest
  Delete $INSTDIR\HydraDesktop.exe
  Delete $INSTDIR\hydratag.exe.manifest
  Delete $INSTDIR\hydratag.exe
  Delete $INSTDIR\hydraweb.exe.manifest
  Delete $INSTDIR\hydraweb.exe

  Delete $INSTDIR\Microsoft.VC90.CRT.manifest
  Delete $INSTDIR\msvcm90.dll
  Delete $INSTDIR\msvcp90.dll
  Delete $INSTDIR\msvcr90.dll

  Delete $INSTDIR\imageformats\Microsoft.VC90.CRT.manifest
  Delete $INSTDIR\imageformats\msvcm90.dll
  Delete $INSTDIR\imageformats\msvcp90.dll
  Delete $INSTDIR\imageformats\msvcr90.dll

  Delete $INSTDIR\QtCore4.dll
  Delete $INSTDIR\QtGui4.dll
  Delete $INSTDIR\QtXml4.dll

  Delete $INSTDIR\imageformats\qgif4.dll
  Delete $INSTDIR\imageformats\qjpeg4.dll
  Delete $INSTDIR\imageformats\qtiff4.dll

  Delete $INSTDIR\uninstall.exe

  ; Remove shortcuts, if any
  ;Delete "$SMPROGRAMS\Example2\*.*"
  Delete "$SMPROGRAMS\HydraDesktop.lnk"

  ; Remove directories used
  ;RMDir "$SMPROGRAMS\Example2"
  RMDir "$INSTDIR"

SectionEnd

