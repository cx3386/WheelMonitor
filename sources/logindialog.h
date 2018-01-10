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
	//	QPoint mousePosition;            //�Լ�ʵ���϶�����
	//	bool isMousePressed;             //�Լ�ʵ���϶�����
	//	QToolButton *minButton;          //��С����ť
	//	QToolButton *closeButton;        //�رհ�ť
	//protected:
	//	//����ʵ���϶�����
	//	void mouseMoveEvent(QMouseEvent*event);
	//	void mousePressEvent(QMouseEvent*event);
	//	void mouseReleaseEvent(QMouseEvent*event);
	private slots:
	void on_loginBtn_clicked();
	void on_quitBtn_clicked();
	//void actionMin();                  //��С������
	//void actionClose();              //�رմ���

private:
	Ui::LoginDialog ui;
};
