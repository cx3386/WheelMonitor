#include "stdafx.h"
#include "PlcWatcher.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	PlcWatcher w;
	w.show();
	return a.exec();
}
