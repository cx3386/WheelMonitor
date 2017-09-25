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
		QMessageBox::critical(&w, QStringLiteral("���ֻ����̨������״̬���"), QStringLiteral("�������ֹ��δ����Ȩ��ƽ̨��ʹ�ã�����ϵ����������Ա��"), QStringLiteral("ȷ��"));
		//a.exit(0);
		//return 0;
	}
	if (!creatConnection())	//���ӵ����ݿ�
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
