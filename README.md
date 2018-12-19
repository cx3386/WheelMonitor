# WheelMonitor

WheelMonitor是用于宝钢环冷机台车车轮状态监测项目的应用软件。

## 支持平台

Windows10 64位

## 安装

安装步骤：

1. 运行安装包WheelMonitorSetup-[x64|x86]-[版本号].msi。在安装过程中，会自动运行**K-Lite解码包**的安装程序。
1. 安装完成后，如果此前没有安装虚拟串口软件，运行ZLVircom4.96_x64.msi。
1. 将监测系统的网线连接至电脑，将电脑IP配置到同一网段（192.168.1.*），并测试是否能够ping通摄像机
1. 打开虚拟串口软件，配置虚拟串口
1. 打开监测软件，开始监测

FAQ:

- 注意：**K-Lite解码包**和**ZLVircom**是**商业软件**。
- 如安装过程中提示需要组件，请联网后下载；或者手动安装vc_redist.x64.exe后重试。
- 如果操作系统提示权限不足，请“以管理员身份运行”此程序。

## 卸载

请到控制面板中卸载程序。注意：K-Lite和ZLVirCom需要手动卸载

## 编译

编译环境
- VS2017(with Qt VS TOOLS)
- Qt 5.10.0

打开WheelMonitor.Sln，编译即可

## 开发

所有开发中使用到的调试工具和配置工具放在tools文件夹中。开发的步骤包括：
1. 配置/测试硬件
	- 将网线连接至电脑
	- 测试网络摄像机的连通性
	- 虚拟串口
	- PLC
1. 开发/调试软件


### CX-ONE/CX-Programer

欧姆龙PLC的编程软件，用于PLC的编程与设置。

#### 用CX-Programmer软件设置下位机PLC

1. 安装CX-ONE/CX-Programmer
1. 使用232线缆(XW2Z-200S-CV)连接PLC与PC
1. 打开CX-P，选择“自动连接到”PLC，串行通信模式为上位链接(SYSWAY)
1. 设置PLC的“启动模式”为“监视”
1. 设置“内置RS232C端口”：波特率115200,格式7,2,E,模式HOSTLINK,单元号0
1. 将设置（和空的PLC程序）下载到PLC中，重启后生效
1. 设置和启动AD转换模块（此步忽略，用HOSTLINK命令实现）

#### 测试PLC

设置好后，用串口软件测试通讯和各模块的应答是否正常。
(ASCII码)
测试CPU模块的IO输入
读取：@00RR0000000141*\r
返回：@00RR00dataFCS*\r data=FFFF
测试AD模块的台车速度
初始化：@00WR0102800A800037*\r
读取 ：@00RR0002000143*\r
返回：@00RR00dataFCS*\r data=FFFF data(hex)*3.59/6000=转速

### ZLVircom

串口服务器ZLAN5103的配置软件，用于配置虚拟COM口。内置了一款串口调试软件

### Serial Port Utility

另一款串口调试软件

### SQLiteStudio

SQLite数据库的可视化软件，用于编辑/查看wheelmonitor.db3数据库文件

### 海康SDK

用于视频流数据的获取和解码，参见SDK压缩包-->开发文档-->IPC编程指南 + SDK使用手册。对于编译来说，已经将需要的文件提取出来并配置好。其中的include和lib用于编译，dll用于运行

### 软件发行

#### 解决依赖问题

1. Qt依赖项和VC runtime。工具：windeployqt, x64 Native Tools Command Prompt for VS 2017。在VS的CMD工具中使用windeployqt /path/wheelmonitor.exe，将自动生成qt依赖项和vcredist_x64.exe
1. process explorer。查看运行依赖库。用于软件打包时解决运行依赖的问题，将所有依赖dll直接复制到可执行文件夹中
1. 海康依赖项。除了HCNetSDK.dll、HCCore.dll、PlayCtrl.dll、SuperRender.dll、AudioRender.dll这几个可以用process explorer查到的dll，**HCNetSDKCom文件夹**（包含里面的功能组件dll库文件）需要和HCNetSDK.dll、HCCore.dll一起加载，放在同一个目录下，且HCNetSDKCom文件夹名不能修改。

#### 软件打包

使用vs2017的Microsoft Visual Studio 2017 Instaler Project。具体步骤如下

### 其他

- 服务器PC电脑密码1
- 专用qq号3192316445，密码baosteel123，用于传输软件和其他资料。
- 海康网络摄像机 IP192.168.1.64/65, 用户名admin，密码baosteel123

## 第三方组件

- [QuaZIP](https://github.com/stachenov/quazip)
- [SingleApplication](https://github.com/itay-grudev/SingleApplication)
- [QCustomPlot](https://www.qcustomplot.com)