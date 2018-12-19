help="/i to install, /x to uninstall."
err=false
Set args=WScript.Arguments
if args.Count=1 Then
opt=WScript.Arguments(0)
Select Case opt
Case "/i"
Case "/x"
Case else
err=true
End Select
Else err=true
End if

if err=true Then
MsgBox help
Else
Set ws=WScript.CreateObject("WScript.shell")
cmd="msiexec.exe " & opt & " ZLVircom4.96_x64.msi"
ws.run cmd
End if