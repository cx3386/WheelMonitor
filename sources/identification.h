#ifndef IDENTIFICATION_H
#define IDENTIFICATION_H

#include <QObject>

class QString;
class Identification :public QObject
{
	Q_OBJECT
public:
	Identification(QObject *parent = Q_NULLPTR) :QObject(parent)
	{
	}
	~Identification() = default;
	static bool check();
	static void getCPUID(unsigned int CPUInfo[4], unsigned int InfoType);
	static void getCPUIDEX(unsigned int CPUInfo[4], unsigned int InfoType, unsigned int ECXValue);
	static QString getLocalMAC();
};

#endif // IDENTIFICATION_H