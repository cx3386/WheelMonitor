#include "stdafx.h"
#include "MainWindow.h"
#include <QtWidgets/QApplication>
#include "identification.h"
#include "logindialog.h"
#include "connection.h"
//QPointer<MainWindow> w;
//void outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
//{
//	if (w)
//		w->myMessageOutput(type, context, msg);
//}


int main(int argc, char *argv[])
{
	QSharedMemory shared("WheelMonitor.exe");//随便填个名字就行
	if (shared.attach())
	{
		return 0;
	}
	shared.create(1);
	QApplication::addLibraryPath("./plugins");
	QApplication a(argc, argv);
	
	//QFont font = a.font();
	//font.setPointSize(18);
	//a.setFont(font);
	MainWindow w;
	identification ide;
	if (ide.flag_cpu_mac == 0)
	{
		QMessageBox::critical(NULL, QStringLiteral("宝钢环冷机台车轮子状态检测"), QStringLiteral("本软件禁止在未经授权的平台上使用，请联系你的软件管理员！"), QStringLiteral("确定"));
		//a.exit(0);
		return 0;
	}
	if (!creatConnection())	//连接到数据库
		//a.exit(0);
		return 0;
	LoginDialog dlg;
	if (dlg.exec() == QDialog::Accepted)
	{
		//w.setWindowFlags(w.windowFlags()& ~Qt::WindowMaximizeButtonHint);
		w.showMaximized();
		return a.exec();
	}
	else
	{
		return 0;
	}

	//w.resize(800, 600);
	//w.setWindowFlags(w.windowFlags()& ~Qt::WindowMaximizeButtonHint& ~Qt::WindowMinimizeButtonHint);
	//w.showMaximized();
	//return a.exec();
}
