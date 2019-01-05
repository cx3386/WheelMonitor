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
* \date ���� 2018
*/
struct DevSpec {
	CamProfile camProfile;
	ImProfile imProfile{ camProfile.frameInterv };
	OcrProfile ocrProfile;
};

/**
 * \class ConfigHelper
 *
 * \brief ���������ļ�ini����ز�����������ȡ�ͱ���
 *
 * \author cx3386
 * \date ���� 2018
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
	void save(); //��< save to ini.����ر�ʱ�����һ��

private:
	QString configFile;
	void takeEffect() const; //!< take the indirect change into effect, including common/launchAtLogin, etc. perhaps use signal, or implement by self.�ڱ���ʱ���������������һ���ԣ��������ǹ������ࣨ�ڵ���ʱ����ѯ

	void effect_launchAtLogin() const;

public:
	//properties
	/* common */
	bool launchAtLogin; //��< run this app on start up ���������Ŀ��أ�����ʱ��������Ч
	bool startAtLaunch; ///< alias click "start" after launch this app
	bool verboseLog; ///< record verbose log

	DevSpec device[2]; //!< ���е�����ֵ�����������ÿ��ʹ��ʱ������helper�࣬���ܱ�֤���õ���Ч��ʵʱ�ԡ�
	/* blow will not show in the dialog, but can be set by .ini file. plz restart app for applying*/
	QString pc2plc_portName; // PC�ϵĴ��ں�(e.g.COM3)
	double matchMaskInnerRatio; //!< ƥ��ʱ���ٳ�����Բ�뾶ռ���ֵı���
	double matchMaskOuterRatio; //!< ƥ��ʱ���ٳ�����Բ�뾶ռ���ֵı���
};
