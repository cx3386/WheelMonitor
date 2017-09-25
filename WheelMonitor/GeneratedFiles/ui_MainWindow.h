/********************************************************************************
** Form generated from reading UI file 'MainWindow.ui'
**
** Created by: Qt User Interface Compiler version 5.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLCDNumber>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>
#include "mytextbrowser.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindowClass
{
public:
    QAction *action_Start;
    QAction *action_Stop;
    QAction *action_Property;
    QAction *action_About_Qt;
    QAction *action_About;
    QAction *action_Quit;
    QWidget *centralWidget;
    QGridLayout *gridLayout_2;
    QTabWidget *centralTabWidget;
    QWidget *monitorTab;
    QGridLayout *gridLayout_6;
    QTabWidget *logTabWidget;
    QWidget *logTab;
    QGridLayout *gridLayout_5;
    myTextBrowser *logTextBrowser;
    QWidget *errorTab;
    QGridLayout *gridLayout_3;
    myTextBrowser *errorTextBrowser;
    QGridLayout *gridLayout_10;
    QLabel *realSpeedLabel;
    QLabel *label;
    QLabel *label_2;
    QLineEdit *realSpeedLineEdit;
    QLineEdit *lastSpeedLineEdit;
    QLabel *lastSpeedLabel;
    QSpacerItem *horizontalSpacer;
    QLabel *imageMatchesLabel;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *alarmPushButton;
    QGroupBox *lcdGroupBox;
    QGridLayout *gridLayout;
    QLCDNumber *lcdNumber;
    QSpacerItem *horizontalSpacer_3;
    QWidget *playerTab;
    QGridLayout *gridLayout_4;
    QWidget *playerWidget;
    QMenuBar *menuBar;
    QMenu *menu_Main;
    QMenu *menu_Monitor;
    QMenu *menu_Setting;
    QMenu *menu_Help;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindowClass)
    {
        if (MainWindowClass->objectName().isEmpty())
            MainWindowClass->setObjectName(QStringLiteral("MainWindowClass"));
        MainWindowClass->resize(987, 700);
        QFont font;
        font.setPointSize(18);
        MainWindowClass->setFont(font);
        QIcon icon;
        icon.addFile(QStringLiteral(":/images/Resources/images/WheelMonitor.ico"), QSize(), QIcon::Normal, QIcon::Off);
        MainWindowClass->setWindowIcon(icon);
        action_Start = new QAction(MainWindowClass);
        action_Start->setObjectName(QStringLiteral("action_Start"));
        action_Start->setCheckable(false);
        action_Start->setFont(font);
        action_Stop = new QAction(MainWindowClass);
        action_Stop->setObjectName(QStringLiteral("action_Stop"));
        action_Stop->setEnabled(false);
        action_Stop->setFont(font);
        action_Property = new QAction(MainWindowClass);
        action_Property->setObjectName(QStringLiteral("action_Property"));
        action_Property->setFont(font);
        action_About_Qt = new QAction(MainWindowClass);
        action_About_Qt->setObjectName(QStringLiteral("action_About_Qt"));
        action_About_Qt->setFont(font);
        action_About = new QAction(MainWindowClass);
        action_About->setObjectName(QStringLiteral("action_About"));
        action_About->setFont(font);
        action_Quit = new QAction(MainWindowClass);
        action_Quit->setObjectName(QStringLiteral("action_Quit"));
        action_Quit->setFont(font);
        centralWidget = new QWidget(MainWindowClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        gridLayout_2 = new QGridLayout(centralWidget);
        gridLayout_2->setSpacing(6);
        gridLayout_2->setContentsMargins(11, 11, 11, 11);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        centralTabWidget = new QTabWidget(centralWidget);
        centralTabWidget->setObjectName(QStringLiteral("centralTabWidget"));
        centralTabWidget->setAutoFillBackground(false);
        monitorTab = new QWidget();
        monitorTab->setObjectName(QStringLiteral("monitorTab"));
        gridLayout_6 = new QGridLayout(monitorTab);
        gridLayout_6->setSpacing(6);
        gridLayout_6->setContentsMargins(11, 11, 11, 11);
        gridLayout_6->setObjectName(QStringLiteral("gridLayout_6"));
        logTabWidget = new QTabWidget(monitorTab);
        logTabWidget->setObjectName(QStringLiteral("logTabWidget"));
        logTabWidget->setMinimumSize(QSize(0, 0));
        logTab = new QWidget();
        logTab->setObjectName(QStringLiteral("logTab"));
        gridLayout_5 = new QGridLayout(logTab);
        gridLayout_5->setSpacing(6);
        gridLayout_5->setContentsMargins(11, 11, 11, 11);
        gridLayout_5->setObjectName(QStringLiteral("gridLayout_5"));
        logTextBrowser = new myTextBrowser(logTab);
        logTextBrowser->setObjectName(QStringLiteral("logTextBrowser"));
        logTextBrowser->setMinimumSize(QSize(200, 0));
        QFont font1;
        font1.setPointSize(9);
        logTextBrowser->setFont(font1);
        logTextBrowser->setFrameShape(QFrame::StyledPanel);
        logTextBrowser->setLineWrapMode(QTextEdit::NoWrap);
        logTextBrowser->setReadOnly(true);

        gridLayout_5->addWidget(logTextBrowser, 0, 0, 1, 1);

        logTabWidget->addTab(logTab, QString());
        errorTab = new QWidget();
        errorTab->setObjectName(QStringLiteral("errorTab"));
        gridLayout_3 = new QGridLayout(errorTab);
        gridLayout_3->setSpacing(6);
        gridLayout_3->setContentsMargins(11, 11, 11, 11);
        gridLayout_3->setObjectName(QStringLiteral("gridLayout_3"));
        errorTextBrowser = new myTextBrowser(errorTab);
        errorTextBrowser->setObjectName(QStringLiteral("errorTextBrowser"));
        errorTextBrowser->setMinimumSize(QSize(200, 0));
        errorTextBrowser->setFont(font1);
        errorTextBrowser->setFocusPolicy(Qt::StrongFocus);
        errorTextBrowser->setAutoFillBackground(false);
        errorTextBrowser->setFrameShape(QFrame::StyledPanel);
        errorTextBrowser->setFrameShadow(QFrame::Sunken);
        errorTextBrowser->setLineWrapMode(QTextEdit::NoWrap);

        gridLayout_3->addWidget(errorTextBrowser, 0, 0, 1, 1);

        logTabWidget->addTab(errorTab, QString());

        gridLayout_6->addWidget(logTabWidget, 0, 2, 3, 1);

        gridLayout_10 = new QGridLayout();
        gridLayout_10->setSpacing(6);
        gridLayout_10->setObjectName(QStringLiteral("gridLayout_10"));
        realSpeedLabel = new QLabel(monitorTab);
        realSpeedLabel->setObjectName(QStringLiteral("realSpeedLabel"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(realSpeedLabel->sizePolicy().hasHeightForWidth());
        realSpeedLabel->setSizePolicy(sizePolicy);

        gridLayout_10->addWidget(realSpeedLabel, 1, 0, 1, 1);

        label = new QLabel(monitorTab);
        label->setObjectName(QStringLiteral("label"));

        gridLayout_10->addWidget(label, 0, 2, 1, 1);

        label_2 = new QLabel(monitorTab);
        label_2->setObjectName(QStringLiteral("label_2"));

        gridLayout_10->addWidget(label_2, 1, 2, 1, 1);

        realSpeedLineEdit = new QLineEdit(monitorTab);
        realSpeedLineEdit->setObjectName(QStringLiteral("realSpeedLineEdit"));
        realSpeedLineEdit->setEnabled(true);
        realSpeedLineEdit->setMinimumSize(QSize(81, 0));
        realSpeedLineEdit->setMaximumSize(QSize(81, 16777215));
        realSpeedLineEdit->setReadOnly(true);

        gridLayout_10->addWidget(realSpeedLineEdit, 1, 1, 1, 1);

        lastSpeedLineEdit = new QLineEdit(monitorTab);
        lastSpeedLineEdit->setObjectName(QStringLiteral("lastSpeedLineEdit"));
        lastSpeedLineEdit->setEnabled(true);
        lastSpeedLineEdit->setMinimumSize(QSize(81, 0));
        lastSpeedLineEdit->setMaximumSize(QSize(81, 16777215));
        lastSpeedLineEdit->setReadOnly(true);

        gridLayout_10->addWidget(lastSpeedLineEdit, 0, 1, 1, 1);

        lastSpeedLabel = new QLabel(monitorTab);
        lastSpeedLabel->setObjectName(QStringLiteral("lastSpeedLabel"));
        sizePolicy.setHeightForWidth(lastSpeedLabel->sizePolicy().hasHeightForWidth());
        lastSpeedLabel->setSizePolicy(sizePolicy);

        gridLayout_10->addWidget(lastSpeedLabel, 0, 0, 1, 1);

        horizontalSpacer = new QSpacerItem(37, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_10->addItem(horizontalSpacer, 0, 3, 1, 1);


        gridLayout_6->addLayout(gridLayout_10, 1, 0, 1, 1);

        imageMatchesLabel = new QLabel(monitorTab);
        imageMatchesLabel->setObjectName(QStringLiteral("imageMatchesLabel"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(imageMatchesLabel->sizePolicy().hasHeightForWidth());
        imageMatchesLabel->setSizePolicy(sizePolicy1);
        imageMatchesLabel->setMinimumSize(QSize(0, 0));
        imageMatchesLabel->setFrameShape(QFrame::NoFrame);
        imageMatchesLabel->setScaledContents(false);

        gridLayout_6->addWidget(imageMatchesLabel, 0, 0, 1, 1);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        alarmPushButton = new QPushButton(monitorTab);
        alarmPushButton->setObjectName(QStringLiteral("alarmPushButton"));
        alarmPushButton->setEnabled(true);
        alarmPushButton->setMinimumSize(QSize(128, 128));
        alarmPushButton->setMaximumSize(QSize(128, 128));
        alarmPushButton->setCursor(QCursor(Qt::PointingHandCursor));
        alarmPushButton->setFocusPolicy(Qt::NoFocus);
        alarmPushButton->setContextMenuPolicy(Qt::NoContextMenu);
        QIcon icon1;
        icon1.addFile(QStringLiteral(":/images/Resources/images/green.png"), QSize(), QIcon::Normal, QIcon::Off);
        alarmPushButton->setIcon(icon1);
        alarmPushButton->setIconSize(QSize(128, 128));
        alarmPushButton->setFlat(true);

        horizontalLayout_2->addWidget(alarmPushButton);

        lcdGroupBox = new QGroupBox(monitorTab);
        lcdGroupBox->setObjectName(QStringLiteral("lcdGroupBox"));
        gridLayout = new QGridLayout(lcdGroupBox);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        lcdNumber = new QLCDNumber(lcdGroupBox);
        lcdNumber->setObjectName(QStringLiteral("lcdNumber"));
        lcdNumber->setMinimumSize(QSize(0, 0));
        QFont font2;
        font2.setPointSize(14);
        lcdNumber->setFont(font2);
        lcdNumber->setCursor(QCursor(Qt::PointingHandCursor));
        lcdNumber->setAutoFillBackground(true);
        lcdNumber->setStyleSheet(QLatin1String("color:rgb(255, 0, 0);\n"
""));
        lcdNumber->setLineWidth(3);
        lcdNumber->setMidLineWidth(1);
        lcdNumber->setDigitCount(3);
        lcdNumber->setSegmentStyle(QLCDNumber::Flat);
        lcdNumber->setProperty("intValue", QVariant(888));

        gridLayout->addWidget(lcdNumber, 0, 0, 1, 1);


        horizontalLayout_2->addWidget(lcdGroupBox);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_3);


        gridLayout_6->addLayout(horizontalLayout_2, 2, 0, 1, 2);

        centralTabWidget->addTab(monitorTab, QString());
        playerTab = new QWidget();
        playerTab->setObjectName(QStringLiteral("playerTab"));
        gridLayout_4 = new QGridLayout(playerTab);
        gridLayout_4->setSpacing(6);
        gridLayout_4->setContentsMargins(11, 11, 11, 11);
        gridLayout_4->setObjectName(QStringLiteral("gridLayout_4"));
        playerWidget = new QWidget(playerTab);
        playerWidget->setObjectName(QStringLiteral("playerWidget"));

        gridLayout_4->addWidget(playerWidget, 0, 0, 1, 1);

        centralTabWidget->addTab(playerTab, QString());

        gridLayout_2->addWidget(centralTabWidget, 0, 0, 1, 1);

        MainWindowClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindowClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 987, 52));
        menuBar->setFont(font);
        menuBar->setDefaultUp(false);
        menuBar->setNativeMenuBar(true);
        menu_Main = new QMenu(menuBar);
        menu_Main->setObjectName(QStringLiteral("menu_Main"));
        menu_Main->setFont(font);
        menu_Main->setTearOffEnabled(false);
        menu_Monitor = new QMenu(menu_Main);
        menu_Monitor->setObjectName(QStringLiteral("menu_Monitor"));
        menu_Monitor->setFont(font);
        menu_Monitor->setTearOffEnabled(false);
        menu_Setting = new QMenu(menuBar);
        menu_Setting->setObjectName(QStringLiteral("menu_Setting"));
        menu_Help = new QMenu(menuBar);
        menu_Help->setObjectName(QStringLiteral("menu_Help"));
        menu_Help->setFont(font);
        MainWindowClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindowClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        MainWindowClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindowClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        statusBar->setMaximumSize(QSize(16777215, 0));
        statusBar->setFont(font);
        MainWindowClass->setStatusBar(statusBar);
#ifndef QT_NO_SHORTCUT
        realSpeedLabel->setBuddy(realSpeedLineEdit);
        label->setBuddy(lastSpeedLineEdit);
        label_2->setBuddy(realSpeedLineEdit);
        lastSpeedLabel->setBuddy(lastSpeedLineEdit);
#endif // QT_NO_SHORTCUT

        menuBar->addAction(menu_Main->menuAction());
        menuBar->addAction(menu_Setting->menuAction());
        menuBar->addAction(menu_Help->menuAction());
        menu_Main->addAction(menu_Monitor->menuAction());
        menu_Main->addSeparator();
        menu_Main->addAction(action_Quit);
        menu_Monitor->addAction(action_Start);
        menu_Monitor->addAction(action_Stop);
        menu_Setting->addAction(action_Property);
        menu_Help->addAction(action_About);
        menu_Help->addAction(action_About_Qt);

        retranslateUi(MainWindowClass);

        centralTabWidget->setCurrentIndex(0);
        logTabWidget->setCurrentIndex(0);
        alarmPushButton->setDefault(false);


        QMetaObject::connectSlotsByName(MainWindowClass);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindowClass)
    {
        MainWindowClass->setWindowTitle(QApplication::translate("MainWindowClass", "\345\256\235\351\222\242\347\216\257\345\206\267\346\234\272\345\217\260\350\275\246\350\275\256\345\255\220\347\212\266\346\200\201\346\243\200\346\265\213", Q_NULLPTR));
        action_Start->setText(QApplication::translate("MainWindowClass", "\345\220\257\345\212\250(&A)", Q_NULLPTR));
        action_Stop->setText(QApplication::translate("MainWindowClass", "\345\201\234\346\255\242(&P)", Q_NULLPTR));
        action_Property->setText(QApplication::translate("MainWindowClass", "\345\261\236\346\200\247...", Q_NULLPTR));
        action_About_Qt->setText(QApplication::translate("MainWindowClass", "\345\205\263\344\272\216 &Qt", Q_NULLPTR));
#ifndef QT_NO_STATUSTIP
        action_About_Qt->setStatusTip(QString());
#endif // QT_NO_STATUSTIP
        action_About->setText(QApplication::translate("MainWindowClass", "\345\205\263\344\272\216(&A)", Q_NULLPTR));
        action_Quit->setText(QApplication::translate("MainWindowClass", "\351\200\200\345\207\272(&Q)", Q_NULLPTR));
        logTabWidget->setTabText(logTabWidget->indexOf(logTab), QApplication::translate("MainWindowClass", "\346\227\245\345\277\227", Q_NULLPTR));
        errorTextBrowser->setPlaceholderText(QString());
        logTabWidget->setTabText(logTabWidget->indexOf(errorTab), QApplication::translate("MainWindowClass", "\346\212\245\350\255\246", Q_NULLPTR));
        realSpeedLabel->setText(QApplication::translate("MainWindowClass", "\345\256\236\346\227\266\350\275\254\351\200\237", Q_NULLPTR));
        label->setText(QApplication::translate("MainWindowClass", "m/min", Q_NULLPTR));
        label_2->setText(QApplication::translate("MainWindowClass", "m/min", Q_NULLPTR));
        realSpeedLineEdit->setText(QString());
        lastSpeedLineEdit->setText(QString());
        lastSpeedLabel->setText(QApplication::translate("MainWindowClass", "\345\211\215\344\270\200\344\270\252\350\275\246\350\275\256\350\275\254\351\200\237", Q_NULLPTR));
        imageMatchesLabel->setText(QString());
#ifndef QT_NO_TOOLTIP
        alarmPushButton->setToolTip(QString());
#endif // QT_NO_TOOLTIP
        alarmPushButton->setText(QString());
        lcdGroupBox->setTitle(QApplication::translate("MainWindowClass", "\346\212\245\350\255\246\350\275\246\350\275\256\345\272\217\345\217\267", Q_NULLPTR));
        centralTabWidget->setTabText(centralTabWidget->indexOf(monitorTab), QApplication::translate("MainWindowClass", "\347\233\221\350\247\206", Q_NULLPTR));
        centralTabWidget->setTabText(centralTabWidget->indexOf(playerTab), QApplication::translate("MainWindowClass", "\351\242\204\350\247\210", Q_NULLPTR));
        menu_Main->setTitle(QApplication::translate("MainWindowClass", "\345\274\200\345\247\213(&F)", Q_NULLPTR));
        menu_Monitor->setTitle(QApplication::translate("MainWindowClass", "\347\233\221\346\265\213(&M)", Q_NULLPTR));
        menu_Setting->setTitle(QApplication::translate("MainWindowClass", "\350\256\276\347\275\256(&S)", Q_NULLPTR));
        menu_Help->setTitle(QApplication::translate("MainWindowClass", "\345\270\256\345\212\251(&H)", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class MainWindowClass: public Ui_MainWindowClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
