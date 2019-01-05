#pragma once

#include "camprofile.h"
#include "improfile.h"
#include "ocrprofile.h"
#include <QObject>
#include <QSettings>

/**
* \struct DevSpec
*
* \brief device specific parameters
* include camera,image process, ocr process
*
* \author cx3386
* \date 三月 2018
*/
struct DevSpec {
	CamProfile camProfile;
	ImProfile imProfile{ camProfile.frameInterv };
	OcrProfile ocrProfile;
};

/**
 * \class ConfigHelper
 *
 * \brief 用于设置文件ini的相关操作，包括读取和保存
 *
 * \author cx3386
 * \date 九月 2018
 */
class ConfigHelper : public QObject {
	Q_OBJECT

public:
	/**
	 * \brief construct a confighelper object using specified file
	 *
	 * this constructor will auto read() config.ini.
	 */
	ConfigHelper(const QString& configuration, QObject* parent = Q_NULLPTR);
	~ConfigHelper();
	void read(); ///< read from ini
	void save(); //！< save to ini.程序关闭时会调用一次

private:
	QString configFile;
	void takeEffect() const; //!< take the indirect change into effect, including common/launchAtLogin, etc. perhaps use signal, or implement by self.在保存时，向外主动输出（一次性），而不是供其他类（在调用时）查询

	void effect_launchAtLogin() const;

public:
	//properties
	/* common */
	bool launchAtLogin; //！< run this app on start up 开机自启的开关，保存时会立即生效
	bool startAtLaunch; ///< alias click "start" after launch this app
	bool verboseLog; ///< record verbose log

	DevSpec device[2]; //!< 所有的属性值都储存在这里，每次使用时都调用helper类，才能保证设置的生效的实时性。
	/* blow will not show in the dialog, but can be set by .ini file. plz restart app for applying*/
	QString pc2plc_portName; // PC上的串口号(e.g.COM3)
	double matchMaskInnerRatio; //!< 匹配时，抠出的内圆半径占车轮的比例
	double matchMaskOuterRatio; //!< 匹配时，抠出的外圆半径占车轮的比例
};
