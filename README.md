# WheelMonitor

WheelMonitor�����ڱ��ֻ����̨������״̬�����Ŀ��Ӧ�������

## ֧��ƽ̨

Windows10 64λ

## ��װ

��װ���裺

1. ���а�װ��WheelMonitorSetup-[x64|x86]-[�汾��].msi���ڰ�װ�����У����Զ�����**K-Lite�����**�İ�װ����
1. ��װ��ɺ������ǰû�а�װ���⴮�����������ZLVircom4.96_x64.msi��
1. �����ϵͳ���������������ԣ�������IP���õ�ͬһ���Σ�192.168.1.*�����������Ƿ��ܹ�pingͨ�����
1. �����⴮��������������⴮��
1. �򿪼���������ʼ���

FAQ:

- ע�⣺**K-Lite�����**��**ZLVircom**��**��ҵ���**��
- �簲װ��������ʾ��Ҫ����������������أ������ֶ���װvc_redist.x64.exe�����ԡ�
- �������ϵͳ��ʾȨ�޲��㣬�롰�Թ���Ա������С��˳���

## ж��

�뵽���������ж�س���ע�⣺K-Lite��ZLVirCom��Ҫ�ֶ�ж��

## ����

���뻷��
- VS2017(with Qt VS TOOLS)
- Qt 5.10.0

��WheelMonitor.Sln�����뼴��

## ����

���п�����ʹ�õ��ĵ��Թ��ߺ����ù��߷���tools�ļ����С������Ĳ��������
1. ����/����Ӳ��
	- ����������������
	- �����������������ͨ��
	- ���⴮��
	- PLC
1. ����/�������


### CX-ONE/CX-Programer

ŷķ��PLC�ı�����������PLC�ı�������á�

#### ��CX-Programmer���������λ��PLC

1. ��װCX-ONE/CX-Programmer
1. ʹ��232����(XW2Z-200S-CV)����PLC��PC
1. ��CX-P��ѡ���Զ����ӵ���PLC������ͨ��ģʽΪ��λ����(SYSWAY)
1. ����PLC�ġ�����ģʽ��Ϊ�����ӡ�
1. ���á�����RS232C�˿ڡ���������115200,��ʽ7,2,E,ģʽHOSTLINK,��Ԫ��0
1. �����ã��Ϳյ�PLC�������ص�PLC�У���������Ч
1. ���ú�����ADת��ģ�飨�˲����ԣ���HOSTLINK����ʵ�֣�

#### ����PLC

���úú��ô����������ͨѶ�͸�ģ���Ӧ���Ƿ�������
(ASCII��)
����CPUģ���IO����
��ȡ��@00RR0000000141*\r
���أ�@00RR00dataFCS*\r data=FFFF
����ADģ���̨���ٶ�
��ʼ����@00WR0102800A800037*\r
��ȡ ��@00RR0002000143*\r
���أ�@00RR00dataFCS*\r data=FFFF data(hex)*3.59/6000=ת��

### ZLVircom

���ڷ�����ZLAN5103�����������������������COM�ڡ�������һ��ڵ������

### Serial Port Utility

��һ��ڵ������

### SQLiteStudio

SQLite���ݿ�Ŀ��ӻ���������ڱ༭/�鿴wheelmonitor.db3���ݿ��ļ�

### ����SDK

������Ƶ�����ݵĻ�ȡ�ͽ��룬�μ�SDKѹ����-->�����ĵ�-->IPC���ָ�� + SDKʹ���ֲᡣ���ڱ�����˵���Ѿ�����Ҫ���ļ���ȡ���������úá����е�include��lib���ڱ��룬dll��������

### �������

#### �����������

1. Qt�������VC runtime�����ߣ�windeployqt, x64 Native Tools Command Prompt for VS 2017����VS��CMD������ʹ��windeployqt /path/wheelmonitor.exe�����Զ�����qt�������vcredist_x64.exe
1. process explorer���鿴���������⡣����������ʱ����������������⣬����������dllֱ�Ӹ��Ƶ���ִ���ļ�����
1. �������������HCNetSDK.dll��HCCore.dll��PlayCtrl.dll��SuperRender.dll��AudioRender.dll�⼸��������process explorer�鵽��dll��**HCNetSDKCom�ļ���**����������Ĺ������dll���ļ�����Ҫ��HCNetSDK.dll��HCCore.dllһ����أ�����ͬһ��Ŀ¼�£���HCNetSDKCom�ļ����������޸ġ�

#### ������

ʹ��vs2017��Microsoft Visual Studio 2017 Instaler Project�����岽������

### ����

- ������PC��������1
- ר��qq��3192316445������baosteel123�����ڴ���������������ϡ�
- ������������� IP192.168.1.64/65, �û���admin������baosteel123

## ���������

- [QuaZIP](https://github.com/stachenov/quazip)
- [SingleApplication](https://github.com/itay-grudev/SingleApplication)
- [QCustomPlot](https://www.qcustomplot.com)