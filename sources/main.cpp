#include "stdafx.h"
#include "mainwindow.h"
#include <QtWidgets/QApplication>
#include "3rdparty/singleapplication/singleapplication.h"
#include "identification.h"
#include "logindialog.h"
#include "database.h"
#include "common.h"
#include "confighelper.h"

QString appName;
QString appDirPath;
QString appFilePath;
QString captureDirPath; //cap dir
QString configDirPath; //cap dir
QString videoDirPath; //video dir
QString matchDirPath;	//match dir
//QString imageDirPath; //imageDirPath
QString logDirPath;
QString ocrPatternDirPath;
QString ocrDirPath;
QString databaseFilePath;

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);

int main(int argc, char *argv[])
{
	qRegisterMetaType<HWND>("HWND");
	qRegisterMetaType<QVector<int>>("QVector<int>");
	//qRegisterMetaType<AlarmColor>("AlarmColor");
	qRegisterMetaType<AlarmEvent>("AlarmEvent");
	qRegisterMetaType<ImProfile>("ImProfile");
	qRegisterMetaTypeStreamOperators<ImProfile>("ImProfile");
	qRegisterMetaType<OcrProfile>("OcrProfile");
	qRegisterMetaTypeStreamOperators<OcrProfile>("OcrProfile");
	qRegisterMetaType<CamProfile>("CamProfile");
	qRegisterMetaTypeStreamOperators<CamProfile>("CamProfile");
	qInstallMessageHandler(myMessageOutput);

	// actually works for the "restart" command -- this command must handle by 1s after the previous instance killed
	if (argc > 1)
	{
		for (int i = 1; i < argc; i++)
		{
			std::string str(argv[1]);
			if (str == "--restart" || str == "-r")
			{
				Sleep(1000);
				break;
			}
		}
	}
	SingleApplication a(argc, argv);

	//setupApplication
	qApp->setApplicationName(FILE_SPEC);
	qApp->setApplicationDisplayName(PRODUCT_NAME);
	qApp->setApplicationVersion(PRODUCT_VER);

	//define the declaration in common.h
	appName = qApp->applicationName();
	//use cleanPath() or absolutePath() or canonicalPath() to remove the redudant . or ..
	appDirPath = QDir::cleanPath(qApp->applicationDirPath());				//the directory contains the app.exe, '/'e.g. C:/QQ
	appFilePath = QDir::cleanPath(qApp->applicationFilePath());				//the file path of app.exe, '/'e.g. C:/QQ/qq.exe
	captureDirPath = QString("%1/Capture").arg(appDirPath); //capture dir
	configDirPath = appDirPath;
	videoDirPath = QString("%1/Video").arg(captureDirPath);
	matchDirPath = QString("%1/Match").arg(captureDirPath);
	ocrDirPath = QString("%1/Ocr").arg(captureDirPath);
	logDirPath = QString("%1/Log").arg(appDirPath);
	ocrPatternDirPath = QString("%1/OcrPattern").arg(appDirPath);
	databaseFilePath = QString("%1/WheelMonitor.db3").arg(appDirPath);

	//command line parser
	QCommandLineParser parser;
	parser.setApplicationDescription(PRODUCT_NAME);
	parser.addHelpOption();
	parser.addVersionOption();
	//-a --auto
	QCommandLineOption autoOption(QStringList() << "a"
		<< "auto", "auto start.");
	parser.addOption(autoOption);
	//-r --restart
	QCommandLineOption restartOption(QStringList() << "r"
		<< "restart", "restart app.");
	parser.addOption(restartOption);
	//-c
	QCommandLineOption configFileOption("c", "specify configuration file.", "config.ini");
	parser.addOption(configFileOption);
	//--no-identity
	QCommandLineOption identityOption("no-identity", "skip identity.");
	parser.addOption(identityOption);
	parser.process(a);
	bool bAuto = parser.isSet(autoOption);
	bool bRestart = parser.isSet(restartOption);
	bool bNoIdentity = parser.isSet(identityOption);
	QString configFile = parser.value(configFileOption);
	if (configFile.isEmpty())
	{
		configFile = configDirPath + "/config.ini";
	}

	//if the app is auto run when powerboot, should sleep for 1s
	//no longer use

	if (!bNoIdentity)
	{
		if (!Identification::check())
		{
			QMessageBox::critical(nullptr, PRODUCT_NAME, QStringLiteral("本软件禁止在未经授权的平台上使用，请联系你的软件管理员！"), QStringLiteral("确定"));
			return 0;
		}
	}
	if (!initMainDb()) //connect to database
	{
		return 0;
	}
	ConfigHelper conf(configFile);
	MainWindow w(&conf);
	QObject::connect(&a, &SingleApplication::instanceStarted, [&w]() {
		w.raise();
		w.activateWindow();
		w.showMaximized();	//if the window is minimized, show max
	});
	w.showMinimized();

	if (!bRestart)
	{//restart won't show the login interface
		LoginDialog dlg(&w);
		if (dlg.exec() != QDialog::Accepted)
			return 0;
	}
	//w.setWindowFlags(w.windowFlags()& ~Qt::WindowMaximizeButtonHint);
	w.showMaximized();

	return a.exec();
}
void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	static QMutex mutex;
	QMutexLocker loker(&mutex);
	QString currentDateTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss.zzz");
	QString typeStr;
	switch (type) {
	case QtDebugMsg:
		typeStr = "Debug";
		break;
	case QtInfoMsg:
		typeStr = "Info";
		break;
	case QtWarningMsg:
		typeStr = "Warning";
		break;
	case QtCriticalMsg:
		typeStr = "Critical";
		break;
	case QtFatalMsg:
		typeStr = "Fatal";
		abort();
	}
	QString msgStr = QStringLiteral("[%1][%2]%3").arg(currentDateTime).arg(typeStr).arg(msg);
	QString today = QDate::currentDate().toString("yyyyMMdd");
	QString logFilePath = QStringLiteral("%1/%2/%3.log").arg(logDirPath).arg(today).arg(today);
	QFile outfile(logFilePath);
	outfile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);	//On Windows, all '\n' characters are written as '\r\n' if QTextStream's device or string is opened using the QIODevice::Text flag.
	QTextStream text_stream(&outfile);
	text_stream << msgStr << endl;	//endl doesn't work without flag QIODevice::Text
	//outfile.flush(); endl will flush to the device
	outfile.close();
}