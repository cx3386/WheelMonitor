# WheelMonitor

WheelMonitor是用于宝钢环冷机台车车轮状态监测项目的应用软件。

## 支持平台

Windows10 64位

## 编译

### 编译环境

- VS2017
- Qt 5.12.0

### 环境的配置

1. 下载并安装VS和QT,注意，需要将<QT_DIR\bin>加入PATH。这里的<QT_DIR>为QT的安装地址，如“C:\Qt\Qt5.12.0\5.12.0\msvc2017_64”。
1. 安装VS插件:Qt Visual Studio Tools和Microsoft Visual Studio Installer Projects。步骤：
   1. 打开VS，工具→扩展和更新→联机→搜索，搜索以上两个插件并分别下载，然后关闭VS后自动安装，完成后重新打开VS，VS菜单栏多了一个Qt VS Tools。
   1. 配置Qt VS Tools。Qt VS Tools→Qt Option→Add，在弹出的Path中填入Qt的安装目录（即上文的<QT_DIR>）。</br>![Qt Option](https://ws1.sinaimg.cn/large/9e24c3aaly1fyu37g0jsrj20bx095jr8.jpg)

### 项目的配置

1. 克隆[WheelMonitor项目](https://github.com/cx3386/WheelMonitor.git)。如果不熟悉git，此处提供一个方法：打开VS→视图→团队资源管理器→连接→本地GIT存储库→克隆→填入项目地址和本地地址→克隆。</br>![团队资源管理器](https://ws1.sinaimg.cn/large/9e24c3aaly1fyu3ul6l4jj208g027gli.jpg)</br>![仓库克隆](https://ws1.sinaimg.cn/large/9e24c3aaly1fyu3k4by6lj208f05ejri.jpg)
1. 打开WheelMonitor.Sln解决方案，并分别生成quazip和wheelmonitor项目的release版本。如果需要调试，请参考[开发](#开发)
1. 生成安装文件。步骤：
   1. 运行脚本：WheelMonitorSetup\beforeSetup.bat。
   1. 生成WheelMonitorSetup项目。如果生成失败，提示有文件不存在，请重新加载该项目。
   1. 运行脚本：WheelMonitorSetup\deploytoinstall.bat,该脚本将生成的msi和其他文件部署到installl文件夹中。
   1. 将install\WheelMonitorSetup-x64-\<version\>压缩为zip，即得到可发行的安装包。

## 开发

所有开发中使用到的调试工具和配置工具放在tools文件夹中。开发的步骤包括：

1. 配置/测试硬件
   - 将网线连接至电脑
   - 测试网络摄像机的连通性
   - 虚拟串口
   - PLC
1. 开发/调试软件

### 软件调试

在生成的可执行文件（.exe）的目录下，至少需要config.ini和OcrPattern以及两个空文件夹Capture和Log才能调试，这些文件位于dependencies\config下。

### 待解决的问题

1. 必须定期擦灰
1. 很多磁铁脱落
1. 光电开关：6号坏了，2号时好时坏

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

已经集成在本解决方案中。项目名称为WheelmonitorSetup。编译前请参阅文件夹下的readme文档

#### 解决依赖问题

1. Qt依赖项和VC runtime。工具：windeployqt, x64 Native Tools Command Prompt for VS 2017。在VS的CMD工具中使用windeployqt /path/wheelmonitor.exe，将自动生成qt依赖项和vcredist_x64.exe
1. process explorer。查看运行依赖库。用于软件打包时解决运行依赖的问题，将所有依赖dll直接复制到可执行文件夹中
1. 海康依赖项。除了HCNetSDK.dll、HCCore.dll、PlayCtrl.dll、SuperRender.dll、AudioRender.dll这几个可以用process explorer查到的dll，**HCNetSDKCom文件夹**（包含里面的功能组件dll库文件）需要和HCNetSDK.dll、HCCore.dll一起加载，放在同一个目录下，且HCNetSDKCom文件夹名不能修改。

#### 软件打包

使用vs2017的Microsoft Visual Studio 2017 Instaler Project。

### 其他

- 服务器PC电脑密码1
- 专用qq号3192316445，密码baosteel123，用于传输软件和其他资料。
- 海康网络摄像机 IP192.168.1.64/65, 用户名admin，密码baosteel123

## 第三方组件

- [QuaZIP](https://github.com/stachenov/quazip)
- [SingleApplication](https://github.com/itay-grudev/SingleApplication)
- [QCustomPlot](https://www.qcustomplot.com)