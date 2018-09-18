#pragma once

#include <QObject>
#include <QSettings>
#include "improfile.h"
#include "ocrprofile.h"
#include "camprofile.h"

/**
 * \class ConfigHelper
 *
 * \brief 用于设置文件ini的相关操作，包括读取和保存
 *
 * \author cx3386
 * \date 九月 2018
 */
class ConfigHelper : public QObject
{
	Q_OBJECT

public:
	/**
	 * \brief construct a confighelper object using specified file
	 *
	 * this constructor will auto read() config.ini.
	 */
	ConfigHelper(const QString &configuration, QObject *parent = Q_NULLPTR);
	~ConfigHelper();
	void read(); ///< read from ini
	void save(); ///< save to ini

private:
	QString configFile;
	void takeEffect() const; ///< take the indirect change into effect, including common/launchAtLogin, etc. perhaps use signal, or implement by self.

	void effect_launchAtLogin() const;

public:
	/* common */
	bool launchAtLogin; ///< run this app on start up
	bool startAtLaunch;	///< alias click "start" after launch this app
	bool verboseLog; ///< record verbose log

	/* blow will not show in the dialog, but can be set by .ini file. plz restart app for applying*/
	QString pc2plc_portName; // default com3

	DevSpec device[2];
};

/**
* \struct DevSpec
*
* \brief device specific parameters
* include camera,image process, ocr process
*
* \author cx3386
* \date 三月 2018
*/
struct DevSpec
{
	CamProfile camProfile;
	ImProfile imProfile{ camProfile.frameInterv };
	OcrProfile ocrProfile;
};