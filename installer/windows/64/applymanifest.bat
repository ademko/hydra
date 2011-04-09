rem this spreads the 1.manifest file to all the needed locations
rem windows sxs is brutal, fyi
copy 1.manifest hydradesktop.exe.manifest
copy 1.manifest hydratag.exe.manifest
copy 1.manifest hydraweb.exe.manifest
mt -manifest 1.manifest -outputresource:hydradesktop.exe;1
mt -manifest 1.manifest -outputresource:hydratag.exe;1
mt -manifest 1.manifest -outputresource:hydraweb.exe;1
mt -manifest 1.manifest -outputresource:QtCore4.dll;2
mt -manifest 1.manifest -outputresource:QtGui4.dll;2
mt -manifest 1.manifest -outputresource:QtXml4.dll;2
mt -manifest 1.manifest -outputresource:imageformats\qgif4.dll;2
mt -manifest 1.manifest -outputresource:imageformats\qico4.dll;2
mt -manifest 1.manifest -outputresource:imageformats\qjpeg4.dll;2
mt -manifest 1.manifest -outputresource:imageformats\qmng4.dll;2
mt -manifest 1.manifest -outputresource:imageformats\qsvg4.dll;2
mt -manifest 1.manifest -outputresource:imageformats\qtiff4.dll;2

