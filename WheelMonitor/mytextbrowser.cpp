#include "stdafx.h"
#include "mytextbrowser.h"


myTextBrowser::myTextBrowser(QWidget *parent)
	: QTextBrowser(parent)
{
}

myTextBrowser::~myTextBrowser()
{
}

void myTextBrowser::contextMenuEvent(QContextMenuEvent * event)
{
	QMenu *menu = createStandardContextMenu();
	QAction *clear = menu->addAction("&Clear", this, SLOT(clear()));
	clear->setEnabled(!document()->isEmpty());
	menu->exec(event->globalPos());
	delete menu;
}
