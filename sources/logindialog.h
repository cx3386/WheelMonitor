#pragma once

#include <QDialog>
#include "ui_logindialog.h"

class LoginDialog : public QDialog
{
	Q_OBJECT

public:
	LoginDialog(QWidget *parent = Q_NULLPTR);
	~LoginDialog();

	//private:
	//	QPoint mousePosition;            //自己实现拖动操作
	//	bool isMousePressed;             //自己实现拖动操作
	//	QToolButton *minButton;          //最小化按钮
	//	QToolButton *closeButton;        //关闭按钮
	//protected:
	//	//重新实现拖动操作
	//	void mouseMoveEvent(QMouseEvent*event);
	//	void mousePressEvent(QMouseEvent*event);
	//	void mouseReleaseEvent(QMouseEvent*event);
	private slots:
	void on_loginBtn_clicked();
	void on_quitBtn_clicked();
	//void actionMin();                  //最小化窗口
	//void actionClose();              //关闭窗口

private:
	Ui::LoginDialog ui;
};
