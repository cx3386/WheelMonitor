#include "stdafx.h"
#include "logindialog.h"
#include "connection.h"

//using namespace std;
//const static int posMinX = 0;
//const static int posMaxX = 20000;
//
//
//const static int posMinY = 0;
//const static int posMaxY = 40;
LoginDialog::LoginDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	//setFixedSize(490, 220);
	setWindowTitle(QStringLiteral("��¼"));
	//setWindowFlags(Qt::FramelessWindowHint);
	setWindowState(Qt::WindowFullScreen);
	//setAttribute(Qt::WA_TranslucentBackground, true);
	QPalette palette(this->palette());
	palette.setColor(QPalette::Background, QColor(0, 170, 255));
	setPalette(palette);
	ui.idLineEdit->setText("BaoSteel");
	ui.idLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);
	ui.pwdLineEdit->setEchoMode(QLineEdit::Password);
	ui.pwdLineEdit->setFocus();
	ui.loginBtn->setDefault(true);
}

LoginDialog::~LoginDialog()
{
}

void LoginDialog::on_loginBtn_clicked()
{
	if (ui.idLineEdit->text().isEmpty())
	{
		QMessageBox::information(this, QStringLiteral("�������û���"), QStringLiteral("�������û������ٵ�¼!"), QStringLiteral("ȷ��"));
		ui.idLineEdit->setFocus();
	}
	else if (ui.pwdLineEdit->text().isEmpty())
	{
		QMessageBox::information(this, QStringLiteral("����������"), QStringLiteral("������������ٵ�¼!"), QStringLiteral("ȷ��"));
		ui.pwdLineEdit->setFocus();
	}
	else 
	{
		QSqlQuery query;
		query.exec(QStringLiteral("select * from user where id='%1' and pwd='%2';").arg(ui.idLineEdit->text()).arg(ui.pwdLineEdit->text()));
		if(query.next())
		{ 		
			QDialog::accept();
		}
		else
		{
			QMessageBox::warning(this, QStringLiteral("�û������������"), QStringLiteral("��������ȷ�û�����������ٵ�¼!"), QStringLiteral("ȷ��"));
			ui.idLineEdit->setFocus();
		}
	}

}

void LoginDialog::on_quitBtn_clicked()
{
	QDialog::reject();
}

//void LoginDialog::actionMin()
//{
//	showMinimized();
//}
//
//
//
//
///***************�ر�************************/
//void LoginDialog::actionClose()
//{
//	close();    //�ر�
//
//
//}
//
//
//
//
///***************����ƶ�********************/
//void LoginDialog::mouseMoveEvent(QMouseEvent *event)
//{
//	if (isMousePressed == true)
//	{
//		QPoint movePot = event->globalPos() - mousePosition;
//		move(movePot);
//	}
//
//
//}
//
//
//
//
///***************�����*******************/
//void LoginDialog::mousePressEvent(QMouseEvent *event)
//{
//	mousePosition = event->pos();  //����굥������׼���϶�ʱ����ʼ������ڴ����е����λ��
//								   //ֻ�Ա�������Χ�ڵ�����¼����д���
//	if (mousePosition.x() <= posMinX)
//	{
//		return;
//	}
//	if (mousePosition.x() >= posMaxX)
//	{
//		return;
//	}
//	if (mousePosition.y() <= posMinY)
//	{
//		return;
//	}
//	if (mousePosition.y() >= posMaxY)
//	{
//		return;
//	}
//
//
//	isMousePressed = true;
//}
//
//
//
//
///***************����ͷ�*******************/
//void LoginDialog::mouseReleaseEvent(QMouseEvent *event)
//{
//	isMousePressed = false;
//}