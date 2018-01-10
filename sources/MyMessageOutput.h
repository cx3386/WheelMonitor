#pragma once

#include <QObject>

class MyMessageOutput : public QObject
{
	Q_OBJECT

public:
	MyMessageOutput(QObject *parent = nullptr);
	~MyMessageOutput();
	static MyMessageOutput *pMyMessageOutput;	//used to emit static signals from static myMessageoutput() function

signals:
	void logMessage(const QString &message);
	void errorMessage(const QString &message);

	public slots:
	void installMesageHandler();
};
