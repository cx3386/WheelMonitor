@echo off
set tpath="..\tools\"
set dst="..\install\"
echo ----�������⴮�ڰ�װ��----
copy %tpath%ZLVircom\ZLVircom4.96_x64.msi %dst%
echo ----������������װ��----
copy %tpath%K-Lite\K-Lite_Codec_Pack_1455_Basic.exe %dst%
echo ----�����������װ��----
copy Release %dst%
REM pause