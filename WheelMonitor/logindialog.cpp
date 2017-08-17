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
	setWindowFlags(Qt::FramelessWindowHint);
	//setAttribute(Qt::WA_TranslucentBackground, true);
	QPalette palette(this->palette());
	palette.setColor(QPalette::Background, QColor(0, 170, 255));
	setPalette(palette);
	ui.idLineEdit->setFocus();
	ui.pwdLineEdit->setEchoMode(QLineEdit::Password);
	ui.loginBtn->setDefault(true);
}
//	minButton = new QToolButton(this);   //最小化按钮
//	closeButton = new QToolButton(this); //关闭按钮
//	int width = this->width();           //获取软件占得像素大小
//	isMousePressed = false;
//
///*************************************加载图标********************************/
//	QPixmap closePix = style()->standardPixmap(QStyle::SP_TitleBarCloseButton);
//	closeButton->setIcon(closePix);
//	QPixmap minPix = style()->standardPixmap(QStyle::SP_TitleBarMinButton);
//	minButton->setIcon(minPix);
//
///*************************************加载位置********************************/
//	minButton->setGeometry(width - 100, 0, 50, 50);
//	closeButton->setGeometry(width - 50, 0, 50, 50);
//	//setPalette(QPalette(Qt::darkCyan));                      //设置背景色
//	setAutoFillBackground(true);
///*************************************信号与槽连接******************************/
//	connect(minButton, SIGNAL(clicked(bool)), SLOT(actionMin()));          //最小化按钮
//	connect(closeButton, SIGNAL(clicked(bool)), SLOT(actionClose()));      //关闭按钮
// /*************************************背景色与父对话框一样************************/
//	minButton->setStyleSheet("QToolButton"
//		"{min - width:80px;"
//		"min - height:32px;}"
//		"QToolButton{"
//		"color:rgb(255, 255, 255);"
//		"min - height:20;"
//		"border - style:solid;"
//	"border - top - left - radius:2px;"
//	"border - top - right - radius:2px;"
//"background: qlineargradient(x1 : 0, y1 : 0, x2 : 0, y2 : 1, stop : 0 rgb(226,236,241),"
//	"stop : 0.3 rgb(160,160,160),"
//	"stop : 1 rgb(140,140,140));"
//		"border:1px;"
//			"border - radius:5px; padding:2px 4px;}"/*border-radius控制圆角大小*/
//"QToolButton:hover{"  /*鼠标放上后*/
//"color:rgb(255, 255, 255);"
//"min - height:20;"
//"border - style:solid;"
//"border - top - left - radius:2px;"
//"border - top - right - radius:2px;"
//"background: qlineargradient(x1 : 0, y1 : 0, x2 : 0, y2 : 1, stop : 0 rgb(226,236,241),"
//	"stop : 0.3 rgb(160,160,160),"
//	"stop : 1 rgb(120,120,120));"
//		"border:1px;"
//			"border - radius:5px; padding:2px 4px;}"
//"QToolButton:pressed{"/*按下按钮后*/
//"color:rgb(255, 255, 255);"
//"min - height:20;"
//"border - style:solid;"
//"border - top - left - radius:2px;"
//"border - top - right - radius:2px;"
//"background: qlineargradient(x1 : 0, y1 : 0, x2 : 0, y2 : 1, stop : 0 rgb(226,236,241),"
//	"stop : 0.3 rgb(190,190,190),"
//	"stop : 1 rgb(160,160,160));"
//		"border:1px;"
//			"border - radius:5px; padding:2px 4px;}"
//"QToolButton:checked{"    /*选中后*/
//"color:rgb(255, 255, 255);"
//"min - height:20;"
//"border - style:solid;"
//"border - top - left - radius:2px;"
//"border - top - right - radius:2px;"
//"background: qlineargradient(x1 : 0, y1 : 0, x2 : 0, y2 : 1, stop : 0 rgb(226,236,241),"
//	"stop : 0.3 rgb(190,190,190),"
//	"stop : 1 rgb(160,160,160));"
//		"border:1px;"
//			"border - radius:5px; padding:2px 4px;}");
//	closeButton->setStyleSheet("background-color:transparent");
//}

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