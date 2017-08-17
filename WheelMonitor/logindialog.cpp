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
	setWindowFlags(Qt::FramelessWindowHint);
	//setAttribute(Qt::WA_TranslucentBackground, true);
	QPalette palette(this->palette());
	palette.setColor(QPalette::Background, QColor(0, 170, 255));
	setPalette(palette);
	ui.idLineEdit->setFocus();
	ui.pwdLineEdit->setEchoMode(QLineEdit::Password);
	ui.loginBtn->setDefault(true);
}
//	minButton = new QToolButton(this);   //��С����ť
//	closeButton = new QToolButton(this); //�رհ�ť
//	int width = this->width();           //��ȡ���ռ�����ش�С
//	isMousePressed = false;
//
///*************************************����ͼ��********************************/
//	QPixmap closePix = style()->standardPixmap(QStyle::SP_TitleBarCloseButton);
//	closeButton->setIcon(closePix);
//	QPixmap minPix = style()->standardPixmap(QStyle::SP_TitleBarMinButton);
//	minButton->setIcon(minPix);
//
///*************************************����λ��********************************/
//	minButton->setGeometry(width - 100, 0, 50, 50);
//	closeButton->setGeometry(width - 50, 0, 50, 50);
//	//setPalette(QPalette(Qt::darkCyan));                      //���ñ���ɫ
//	setAutoFillBackground(true);
///*************************************�ź��������******************************/
//	connect(minButton, SIGNAL(clicked(bool)), SLOT(actionMin()));          //��С����ť
//	connect(closeButton, SIGNAL(clicked(bool)), SLOT(actionClose()));      //�رհ�ť
// /*************************************����ɫ�븸�Ի���һ��************************/
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
//			"border - radius:5px; padding:2px 4px;}"/*border-radius����Բ�Ǵ�С*/
//"QToolButton:hover{"  /*�����Ϻ�*/
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
//"QToolButton:pressed{"/*���°�ť��*/
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
//"QToolButton:checked{"    /*ѡ�к�*/
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