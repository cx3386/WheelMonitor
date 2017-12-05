#include "stdafx.h"
#include "MainWindow.h"
#include <QtWidgets/QApplication>
#include <singleapplication.h>
#include "identification.h"
#include "logindialog.h"
#include "database.h"
#include "common.h"

QString appName;
QString appDirPath;
QString appFilePath;
QString captureDirPath;
QString logDirPath;

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	static QMutex mutex;
	QMutexLocker loker(&mutex);
	QString currentDateTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
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
	QString logFilePath = QStringLiteral("%1/%2.log").arg(logDirPath).arg(today);
	QFile outfile(logFilePath);
	outfile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);	//On Windows, all '\n' characters are written as '\r\n' if QTextStream's device or string is opened using the QIODevice::Text flag.
	QTextStream text_stream(&outfile);
	text_stream << msgStr << endl;	//endl doesn't work without flag QIODevice::Text
	//outfile.flush(); endl will flush to the device
	outfile.close();
}

int main(int argc, char *argv[])
{
	qInstallMessageHandler(myMessageOutput);
	////QApplication::addLibraryPath("./plugins");	//very important
	if (argc > 1)
	{
		std::string str(argv[1]);
		if (str == "--restart" || str == "-r")
		{
			Sleep(100);
		}
	}
	SingleApplication a(argc, argv);

	qApp->setApplicationName("WheelMonitor");
	qApp->setApplicationVersion("1.1.0");

	appName = qApp->applicationName();
	appDirPath = qApp->applicationDirPath();				//the directory contains the app.exe, '/'e.g. C:/QQ
	appFilePath = qApp->applicationFilePath();				//the file path of app.exe, '/'e.g. C:/QQ/qq.exe
	captureDirPath = QString("%1/Capture").arg(appDirPath); //capture dir
	logDirPath = QString("%1/Log").arg(appDirPath);

	QCommandLineParser parser;
	parser.setApplicationDescription(QStringLiteral("���ֻ����̨������ת�ټ�����"));
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
	parser.process(a);
	bool bAuto = parser.isSet(autoOption);
	bool bRestart = parser.isSet(restartOption);

	//if (bAuto)
	//{//jundge if autorun
	//	Sleep(5000);
	//}

	bool bNeedIdentity = false; //should be set to true when release
	if (bNeedIdentity)
	{
		identification ide;
		if (ide.flag_cpu_mac == 0)
		{
			QMessageBox::critical(nullptr, QStringLiteral("���ֻ����̨������ת�ټ��"), QStringLiteral("�������ֹ��δ����Ȩ��ƽ̨��ʹ�ã�����ϵ����������Ա��"), QStringLiteral("ȷ��"));
			//a.exit(0);
			return 0;
		}
	}
	if (!initDataBase()) //���ӵ����ݿ�
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
	w.showMaximized();

	return a.exec();
}