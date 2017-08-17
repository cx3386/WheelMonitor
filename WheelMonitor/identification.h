#ifndef IDENTIFICATION_H
#define IDENTIFICATION_H

#include <QtWidgets/QWidget>
#include <QCoreApplication>
#include <QStringList>
#include <QString>
#include <qdebug.h>
#include <QNetworkInterface>
#include <intrin.h>
#include <QtWidgets/QApplication>

class identification
{
public:
	identification();
	~identification();
	int flag_cpu_mac = -1;

	void getcpuid(unsigned int CPUInfo[4], unsigned int InfoType);
	void getcpuidex(unsigned int CPUInfo[4], unsigned int InfoType, unsigned int ECXValue);

	QString get_localmachine_mac();

};

#endif // IDENTIFICATION_H