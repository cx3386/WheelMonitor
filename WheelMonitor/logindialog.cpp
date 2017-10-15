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
	setWindowTitle(QStringLiteral("登录"));
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
		QMessageBox::information(this, QStringLiteral("请输入用户名"), QStringLiteral("请输入用户名后再登录!"), QStringLiteral("确定"));
		ui.idLineEdit->setFocus();
	}
	else if (ui.pwdLineEdit->text().isEmpty())
	{
		QMessageBox::information(this, QStringLiteral("请输入密码"), QStringLiteral("请输入密码后再登录!"), QStringLiteral("确定"));
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
			QMessageBox::warning(this, QStringLiteral("用户名或密码错误"), QStringLiteral("请输入正确用户名和密码后再登录!"), QStringLiteral("确定"));
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
///***************关闭************************/
//void LoginDialog::actionClose()
//{
//	close();    //关闭
//
//
//}
//
//
//
//
///***************鼠标移动********************/
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
///***************鼠标点击*******************/
//void LoginDialog::mousePressEvent(QMouseEvent *event)
//{
//	mousePosition = event->pos();  //当鼠标单击窗体准备拖动时，初始化鼠标在窗体中的相对位置
//								   //只对标题栏范围内的鼠标事件进行处理
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
///***************鼠标释放*******************/
//void LoginDialog::mouseReleaseEvent(QMouseEvent *event)
//{
//	isMousePressed = false;
//}