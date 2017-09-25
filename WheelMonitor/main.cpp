#include "stdafx.h"
#include "MainWindow.h"
#include <QtWidgets/QApplication>
#include <singleapplication.h>
#include "identification.h"
#include "logindialog.h"
#include "connection.h"

int main(int argc, char *argv[])
{

	////QApplication::addLibraryPath("./plugins");	//very important
	SingleApplication a(argc, argv);
	MainWindow w;
	QObject::connect(&a, &SingleApplication::instanceStarted, [&w]() {
		w.raise();
		w.activateWindow();
	});
	//QFont font = a.font();
	//font.setPointSize(18);
	//a.setFont(font);
	identification ide;
	if (ide.flag_cpu_mac == 0)
	{
		QMessageBox::critical(&w, QStringLiteral("宝钢环冷机台车轮子状态检测"), QStringLiteral("本软件禁止在未经授权的平台上使用，请联系你的软件管理员！"), QStringLiteral("确定"));
		//a.exit(0);
		//return 0;
	}
	if (!creatConnection())	//连接到数据库
		return 0;
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
