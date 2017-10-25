#include "stdafx.h"
#include "logindialog.h"
#include "database.h"

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
		query.exec(QStringLiteral("select * from user where username='%1' and pwd='%2';").arg(ui.idLineEdit->text()).arg(ui.pwdLineEdit->text()));
		if (query.next())
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