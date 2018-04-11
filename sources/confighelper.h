#pragma once

#include <QObject>
#include <QSettings>
#include "improfile.h"
#include "ocrprofile.h"
#include "camprofile.h"

/**
* \struct DevSpec
*
* \brief device specific parameters
* include camera,
*
* \author cx3386
* \date ÈýÔÂ 2018
*/
struct DevSpec
{
	CamProfile camProfile;
	ImProfile imProfile{ camProfile.frameInterv };
	OcrProfile ocrProfile;
};

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

	bool launchAtLogin; ///< run this app on start up
	bool startAtLaunch;	///< alias click "start" after launch this app
	bool verboseLog; ///< record verbose log

	DevSpec device[2];
};
