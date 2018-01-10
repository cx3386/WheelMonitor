#include "stdafx.h"
#include "mainwindow.h"
#include <QtWidgets/QApplication>
#include "3rdparty/singleapplication/singleapplication.h"
#include "identification.h"
#include "logindialog.h"
#include "database.h"
#include "common.h"

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
	qInstallMessageHandler(myMessageOutput);
	////QApplication::addLibraryPath("./plugins");	//very important
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

	qApp->setApplicationName("WheelMonitor");
	qApp->setApplicationVersion("1.1.0");

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

	QCommandLineParser parser;
	parser.setApplicationDescription(QStringLiteral("宝钢环冷机台车轮子转速监测软件"));
	parser.addHelpOption();
	parser.addVersionOption();
	//-a --auto
	QCommandLineOption autoOption(QStringList() << "a"
		<< "auto",
		QCoreApplication::translate("main", "auto start."));
	parser.addOption(autoOption);
	//-r --restart
	QCommandLineOption restartOption(QStringList() << "r"
		<< "restart",
		QCoreApplication::translate("main", "restart app."));
	parser.addOption(restartOption);
	QCommandLineOption identityOption(QStringList() << "no-identity",
		QCoreApplication::translate("main", "skip identity."));
	parser.addOption(identityOption);
	parser.process(a);
	bool bAuto = parser.isSet(autoOption);
	bool bRestart = parser.isSet(restartOption);
	bool bNoIdentity = parser.isSet(identityOption);

	//if (bAuto)
	//{//jundge if autorun
	//	Sleep(5000);
	//}

	if (!bNoIdentity)
	{
		identification ide;
		if (ide.flag_cpu_mac == 0)
		{
			QMessageBox::critical(nullptr, QStringLiteral("宝钢环冷机台车轮子转速监测"), QStringLiteral("本软件禁止在未经授权的平台上使用，请联系你的软件管理员！"), QStringLiteral("确定"));
			//a.exit(0);
			return 0;
		}
	}
	if (!initDataBase()) //connect to database
	{
		return 0;
	}
	MainWindow w;
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
	//w.showMaximized();
	w.resize(800, 600);
	w.show();

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