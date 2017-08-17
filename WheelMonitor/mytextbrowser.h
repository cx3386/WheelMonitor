#pragma once

#include <QTextBrowser>

class myTextBrowser : public QTextBrowser
{
	Q_OBJECT

public:
	myTextBrowser(QWidget *parent);
	~myTextBrowser();
protected:
	void contextMenuEvent(QContextMenuEvent *event);
};
