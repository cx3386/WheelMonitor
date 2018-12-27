Set ws=WScript.CreateObject("WScript.shell")
cmd="K-Lite_Codec_Pack_1455_Basic.exe"
ws.run cmd
cmd="msiexec.exe /i ZLVircom4.96_x64.msi"
ws.run cmd
cmd="msiexec.exe /i WheelMonitorSetup-x64-1.0.2.msi"
ws.run cmd