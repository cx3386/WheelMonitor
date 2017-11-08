#include "stdafx.h"
#include "MainWindow.h"
#include <QtWidgets/QApplication>
#include <singleapplication.h>
#include "identification.h"
#include "logindialog.h"
#include "database.h"

int main(int argc, char *argv[])
{
	////QApplication::addLibraryPath("./plugins");	//very important
	SingleApplication a(argc, argv);

	identification ide;
	if (ide.flag_cpu_mac == 0)
	{
		QMessageBox::critical(nullptr, QStringLiteral("宝钢环冷机台车轮子状态检测"), QStringLiteral("本软件禁止在未经授权的平台上使用，请联系你的软件管理员！"), QStringLiteral("确定"));
		//a.exit(0);
		//return 0;
	}
	if (!initDataBase())	//连接到数据库
		return 0;
	MainWindow w;
	QObject::connect(&a, &SingleApplication::instanceStarted, [&w]() {
		w.showMinimized();
		w.showMaximized();
	});
	LoginDialog dlg(&w);
	if (dlg.exec() == QDialog::Accepted)
	{
		//w.setWindowFlags(w.windowFlags()& ~Qt::WindowMaximizeButtonHint);
		w.showMaximized();
		return a.exec();
	}
	return 0;

	//w.setWindowFlags(w.windowFlags()& ~Qt::WindowMaximizeButtonHint& ~Qt::WindowMinimizeButtonHint);
	//w.showMaximized();
	//return a.exec();
}