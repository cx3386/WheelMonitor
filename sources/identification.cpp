#include "stdafx.h"
#include "identification.h"

identification::identification()
{
	QString cpu_id = "";
	unsigned int dwBuf[4];
	unsigned long long ret;
	getcpuid(dwBuf, 1);
	ret = dwBuf[3];
	ret = ret << 32;
	cpu_id = QString::number(dwBuf[3], 16).toUpper();
	cpu_id = cpu_id + QString::number(dwBuf[0], 16).toUpper();
	//qWarning() << cpu_id;
	//qWarning() << get_localmachine_mac();
	if ((cpu_id == "BFEBFBFF506E3") && (get_localmachine_mac() == "1C:1B:0D:6C:23:B1"))
	//if (cpu_id == "BFEBFBFF406E3")
	{
		//qDebug() << "cpu is right";
		flag_cpu_mac = 1;
	}
	else
		flag_cpu_mac = 0;
}

identification::~identification()
{

}

void identification::getcpuid(unsigned int CPUInfo[4], unsigned int InfoType)
{
#if defined(__GNUC__)// GCC
	__cpuid(InfoType, CPUInfo[0], CPUInfo[1], CPUInfo[2], CPUInfo[3]);
#elif defined(_MSC_VER)// MSVC
#if _MSC_VER >= 1400 //VC2005才支持__cpuid
	__cpuid((int*)(void*)CPUInfo, (int)(InfoType));
#else //其他使用getcpuidex
	getcpuidex(CPUInfo, InfoType, 0);
#endif
#endif
}

void identification::getcpuidex(unsigned int CPUInfo[4], unsigned int InfoType, unsigned int ECXValue)
{
#if defined(_MSC_VER) // MSVC
#if defined(_WIN64)	// 64位下不支持内联汇编. 1600: VS2010, 据说VC2008 SP1之后才支持__cpuidex.
	__cpuidex((int*)(void*)CPUInfo, (int)InfoType, (int)ECXValue);
#else
	if (NULL == CPUInfo)	return;
	_asm{
		// load. 读取参数到寄存器.
		mov edi, CPUInfo;
		mov eax, InfoType;
		mov ecx, ECXValue;
		// CPUID
		cpuid;
		// save. 将寄存器保存到CPUInfo
		mov[edi], eax;
		mov[edi + 4], ebx;
		mov[edi + 8], ecx;
		mov[edi + 12], edx;
	}
#endif
#endif
}



QString identification::get_localmachine_mac()
{
	QList<QNetworkInterface> nets = QNetworkInterface::allInterfaces();
	QString local_mac;
	foreach(QNetworkInterface ni, nets)
	{
		if (!(ni.humanReadableName().contains("VMware", Qt::CaseInsensitive)))
			if ((ni.humanReadableName() == QStringLiteral("以太网")))
			{
				//qWarning() << ni.name() << ni.hardwareAddress() << ni.humanReadableName();
				local_mac = ni.hardwareAddress();
			}
	}
	
	return local_mac;
}
