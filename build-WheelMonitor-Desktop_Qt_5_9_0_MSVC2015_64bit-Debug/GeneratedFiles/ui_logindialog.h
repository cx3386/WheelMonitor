/********************************************************************************
** Form generated from reading UI file 'logindialog.ui'
**
** Created by: Qt User Interface Compiler version 5.9.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LOGINDIALOG_H
#define UI_LOGINDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>

QT_BEGIN_NAMESPACE

class Ui_LoginDialog
{
public:
    QGridLayout *gridLayout;
    QSpacerItem *verticalSpacer_2;
    QLabel *tittleLabel;
    QLabel *label_2;
    QLineEdit *pwdLineEdit;
    QLineEdit *idLineEdit;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *loginBtn;
    QSpacerItem *horizontalSpacer_7;
    QPushButton *quitBtn;
    QSpacerItem *horizontalSpacer_4;
    QLabel *label;
    QSpacerItem *verticalSpacer_3;
    QSpacerItem *horizontalSpacer_3;
    QSpacerItem *horizontalSpacer_2;
    QSpacerItem *horizontalSpacer_5;
    QSpacerItem *verticalSpacer;
    QSpacerItem *verticalSpacer_4;
    QSpacerItem *horizontalSpacer_6;

    void setupUi(QDialog *LoginDialog)
    {
        if (LoginDialog->objectName().isEmpty())
            LoginDialog->setObjectName(QStringLiteral("LoginDialog"));
        LoginDialog->resize(597, 439);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(LoginDialog->sizePolicy().hasHeightForWidth());
        LoginDialog->setSizePolicy(sizePolicy);
        QFont font;
        font.setPointSize(14);
        LoginDialog->setFont(font);
        LoginDialog->setWindowOpacity(1);
        LoginDialog->setAutoFillBackground(false);
        LoginDialog->setStyleSheet(QStringLiteral(""));
        gridLayout = new QGridLayout(LoginDialog);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        gridLayout->setVerticalSpacing(9);
        gridLayout->setContentsMargins(0, 0, 0, 0);
        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout->addItem(verticalSpacer_2, 13, 0, 1, 1);

        tittleLabel = new QLabel(LoginDialog);
        tittleLabel->setObjectName(QStringLiteral("tittleLabel"));
        tittleLabel->setFont(font);
        tittleLabel->setStyleSheet(QStringLiteral("color: rgb(255, 255, 127);"));

        gridLayout->addWidget(tittleLabel, 4, 1, 1, 4);

        label_2 = new QLabel(LoginDialog);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setFont(font);

        gridLayout->addWidget(label_2, 10, 1, 3, 1);

        pwdLineEdit = new QLineEdit(LoginDialog);
        pwdLineEdit->setObjectName(QStringLiteral("pwdLineEdit"));
        pwdLineEdit->setFont(font);

        gridLayout->addWidget(pwdLineEdit, 10, 3, 3, 2);

        idLineEdit = new QLineEdit(LoginDialog);
        idLineEdit->setObjectName(QStringLiteral("idLineEdit"));
        idLineEdit->setFont(font);

        gridLayout->addWidget(idLineEdit, 7, 3, 3, 2);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        loginBtn = new QPushButton(LoginDialog);
        loginBtn->setObjectName(QStringLiteral("loginBtn"));
        QFont font1;
        font1.setFamily(QStringLiteral("Agency FB"));
        font1.setPointSize(14);
        loginBtn->setFont(font1);
        loginBtn->setStyleSheet(QStringLiteral("background-color: rgb(0, 255, 0);"));

        horizontalLayout->addWidget(loginBtn);

        horizontalSpacer_7 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_7);

        quitBtn = new QPushButton(LoginDialog);
        quitBtn->setObjectName(QStringLiteral("quitBtn"));
        quitBtn->setFont(font1);
        quitBtn->setStyleSheet(QStringLiteral("background-color: rgb(255, 0, 0);"));

        horizontalLayout->addWidget(quitBtn);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_4);


        gridLayout->addLayout(horizontalLayout, 16, 1, 1, 4);

        label = new QLabel(LoginDialog);
        label->setObjectName(QStringLiteral("label"));
        label->setFont(font);

        gridLayout->addWidget(label, 7, 1, 3, 2);

        verticalSpacer_3 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout->addItem(verticalSpacer_3, 17, 1, 1, 1);

        horizontalSpacer_3 = new QSpacerItem(53, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_3, 11, 0, 1, 1);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_2, 8, 0, 1, 1);

        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_5, 8, 5, 1, 1);

        verticalSpacer = new QSpacerItem(20, 13, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout->addItem(verticalSpacer, 6, 1, 1, 1);

        verticalSpacer_4 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout->addItem(verticalSpacer_4, 3, 1, 1, 1);

        horizontalSpacer_6 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_6, 11, 5, 1, 1);

#ifndef QT_NO_SHORTCUT
        label_2->setBuddy(pwdLineEdit);
        label->setBuddy(idLineEdit);
#endif // QT_NO_SHORTCUT
        QWidget::setTabOrder(idLineEdit, pwdLineEdit);
        QWidget::setTabOrder(pwdLineEdit, loginBtn);
        QWidget::setTabOrder(loginBtn, quitBtn);

        retranslateUi(LoginDialog);

        QMetaObject::connectSlotsByName(LoginDialog);
    } // setupUi

    void retranslateUi(QDialog *LoginDialog)
    {
        LoginDialog->setWindowTitle(QApplication::translate("LoginDialog", "LoginDialog", Q_NULLPTR));
        tittleLabel->setText(QApplication::translate("LoginDialog", "\346\254\242\350\277\216\344\275\277\347\224\250\345\256\235\351\222\242\347\216\257\345\206\267\346\234\272\345\217\260\350\275\246\350\275\256\345\255\220\347\212\266\346\200\201\346\243\200\346\265\213\347\263\273\347\273\237!", Q_NULLPTR));
        label_2->setText(QApplication::translate("LoginDialog", "\345\257\206\347\240\201\357\274\232", Q_NULLPTR));
        loginBtn->setText(QApplication::translate("LoginDialog", "\347\231\273\345\275\225", Q_NULLPTR));
        quitBtn->setText(QApplication::translate("LoginDialog", "\351\200\200\345\207\272", Q_NULLPTR));
        label->setText(QApplication::translate("LoginDialog", "\347\224\250\346\210\267\345\220\215\357\274\232", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class LoginDialog: public Ui_LoginDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOGINDIALOG_H
