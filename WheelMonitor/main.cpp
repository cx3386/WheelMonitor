#include "stdafx.h"
#include "MainWindow.h"
#include <QtWidgets/QApplication>
#include <singleapplication.h>
#include "identification.h"
#include "logindialog.h"
#include "database.h"
#include "common.h"

int main(int argc, char *argv[])
{
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
			QMessageBox::critical(nullptr, QStringLiteral("宝钢环冷机台车轮子转速监测"), QStringLiteral("本软件禁止在未经授权的平台上使用，请联系你的软件管理员！"), QStringLiteral("确定"));
			//a.exit(0);
			return 0;
		}
	}
	if (!initDataBase()) //连接到数据库
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