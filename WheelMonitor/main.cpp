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
	//if (ide.flag_cpu_mac == 0)
	//{
	//	QMessageBox::critical(nullptr, QStringLiteral("���ֻ����̨������ת�ټ��"), QStringLiteral("�������ֹ��δ����Ȩ��ƽ̨��ʹ�ã�����ϵ����������Ա��"), QStringLiteral("ȷ��"));
	//	//a.exit(0);
	//	return 0;
	//}
	if (!initDataBase())	//���ӵ����ݿ�
		return 0;
	MainWindow w;
	QObject::connect(&a, &SingleApplication::instanceStarted, [&w]() {
		w.showMinimized();
		w.showMaximized();
	});
	if (argc > 1)//restart
	{
		std::string str(argv[1]);
		if (str == "restart")
		{
			w.showMaximized();
			return a.exec();
		}
	}
	else
	{
		LoginDialog dlg(&w);
		if (dlg.exec() == QDialog::Accepted)
		{
			//w.setWindowFlags(w.windowFlags()& ~Qt::WindowMaximizeButtonHint);
			w.showMaximized();
			//w.showNormal();
			return a.exec();
		}
		return 0;
	}
	//w.showMaximized();
	//return a.exec();
}