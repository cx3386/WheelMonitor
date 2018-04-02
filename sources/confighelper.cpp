#include "stdafx.h"
#include "confighelper.h"
#include "common.h"

ConfigHelper::ConfigHelper(const QString &configuration, QObject *parent /*= Q_NULLPTR*/)
	: QObject(parent)
	, configFile(configuration) // 初始化列表有顺序之分
	, settings(new QSettings(configFile, QSettings::IniFormat, this)) //ptr必须在堆上
{
	read();
}

ConfigHelper::~ConfigHelper()
{
	// 因为ConfigHelper是QObject子类，类指针成员会被qt内存管理，不需要delete/deleteLater
	settings->deleteLater();
}

void ConfigHelper::read()
{
	/*Common*/
	settings->beginGroup("Common");
	launchAtLogin = settings->value("launchAtLogin", true).toBool();
	startAtLaunch = settings->value("startAtLaunch", true).toBool();
	verboseLog = settings->value("verboseLog", true).toBool();
	settings->endGroup();

	int size = settings->beginReadArray("DevSpec");
	if (size != 2)
	{
		qCritical("ConfigHelper: read() error");
		return;
	}
	for (int i = 0; i < size; ++i)
	{
		settings->setArrayIndex(i);
		dev[i].devNum = settings->value("devNum").toInt();
		dev[i].camProf = settings->value("CamProfile").value<CamProfile>();
		dev[i].imProf = settings->value("ImProfile").value<ImProfile>();
		dev[i].ocrProf = settings->value("OcrProfile").value<OcrProfile>();
	}
	settings->endArray();
}

void ConfigHelper::save() const
{
	takeEffect();
	settings->beginGroup("Common");
	settings->setValue("launchAtLogin", QVariant(launchAtLogin));
	settings->setValue("startAtLaunch", QVariant(startAtLaunch));
	settings->setValue("verboseLog", QVariant(verboseLog));
	settings->endGroup();

	settings->beginReadArray("DevSpec");
	for (int i = 0; i < 2; ++i)
	{
		settings->setArrayIndex(i);
		settings->setValue("devNum", QVariant(dev[i].devNum));
		settings->setValue("CamProfile", QVariant::fromValue<CamProfile>(dev[i].camProf));
		settings->setValue("ImProfile", QVariant::fromValue<ImProfile>(dev[i].imProf));
		settings->setValue("OcrProfile", QVariant::fromValue<OcrProfile>(dev[i].ocrProf));
	}
	settings->endArray();
}

void ConfigHelper::takeEffect() const
{
	effect_launchAtLogin();
}

void ConfigHelper::effect_launchAtLogin() const
{
	//HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Run
	QSettings *reg = new QSettings(R"(HKEY_CURRENT_USER\SOFTWARE\Microsoft\Windows\CurrentVersion\Run)", QSettings::NativeFormat);

	if (launchAtLogin)
	{
		QString strAppPath = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
		//strAppPath.replace(QChar('/'), QChar('\\'), Qt::CaseInsensitive);
		reg->setValue(appName, QString("%1 --auto").arg(strAppPath));
	}
	else
	{
		reg->remove(appName);
	}
	reg->deleteLater();
}