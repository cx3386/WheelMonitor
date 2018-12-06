#include "identification.h"
#include "stdafx.h"
#include <QNetworkInterface>
#include <intrin.h>

const char CPU_ID[] = "BFEBFBFF506E3";
const char LOCAL_MAC[] = "1C:1B:0D:6C:23:B1";

bool Identification::check()
{
    unsigned int dwBuf[4];
    getCPUID(dwBuf, 1);
    auto cpu_id = QString::number(dwBuf[3], 16).toUpper() + QString::number(dwBuf[0], 16).toUpper();
    auto local_mac = getLocalMAC();
    //qDebug() << "CPUID: " << cpu_id << "MAC: " << local_mac;
    return (cpu_id == CPU_ID) && (local_mac == LOCAL_MAC);
}

void Identification::getCPUID(unsigned int CPUInfo[4], unsigned int InfoType)
{
#if defined(__GNUC__) // GCC
    __cpuid(InfoType, CPUInfo[0], CPUInfo[1], CPUInfo[2], CPUInfo[3]);
#elif defined(_MSC_VER) // MSVC
#if _MSC_VER >= 1400 //VC2005才支持__cpuid
    __cpuid((int*)(void*)CPUInfo, (int)(InfoType));
#else
    getCPUIDEX(CPUInfo, InfoType, 0);
#endif
#endif
}

void Identification::getCPUIDEX(unsigned int CPUInfo[4], unsigned int InfoType, unsigned int ECXValue)
{
#if defined(_MSC_VER) // MSVC
#if defined(_WIN64) // 64位下不支持内联汇编. 1600: VS2010, 据说VC2008 SP1之后才支持__cpuidex.
    __cpuidex((int*)(void*)CPUInfo, (int)InfoType, (int)ECXValue);
#else
    if (NULL == CPUInfo)
        return;
    _asm {
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

QString Identification::getLocalMAC()
{
    QList<QNetworkInterface> nets = QNetworkInterface::allInterfaces();
    QString local_mac;
    foreach (QNetworkInterface ni, nets) {
        if (!(ni.humanReadableName().contains("VMware", Qt::CaseInsensitive)))
            if ((ni.humanReadableName() == QStringLiteral("以太网"))) {
                local_mac = ni.hardwareAddress();
            }
    }
    return local_mac;
}