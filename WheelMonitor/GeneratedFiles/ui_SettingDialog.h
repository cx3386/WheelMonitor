/********************************************************************************
** Form generated from reading UI file 'SettingDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SETTINGDIALOG_H
#define UI_SETTINGDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SettingDialog
{
public:
    QGridLayout *gridLayout;
    QTabWidget *tabWidget;
    QWidget *tab;
    QGridLayout *gridLayout_4;
    QGroupBox *imageProcessGroupBox;
    QGridLayout *gridLayout_2;
    QCheckBox *sensorCheckBox;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QLabel *angleHighLabel;
    QSpacerItem *horizontalSpacer_3;
    QDoubleSpinBox *angleHighSpinBox;
    QHBoxLayout *horizontalLayout_2;
    QLabel *angleLowLabel;
    QSpacerItem *horizontalSpacer_4;
    QDoubleSpinBox *angleLowSpinBox;
    QHBoxLayout *horizontalLayout_3;
    QLabel *radiusMinLabel;
    QSpacerItem *horizontalSpacer_6;
    QSpinBox *radiusMinSpinBox;
    QHBoxLayout *horizontalLayout_4;
    QLabel *radiusMaxLabel;
    QSpacerItem *horizontalSpacer_8;
    QSpinBox *radiusMaxSpinBox;
    QHBoxLayout *horizontalLayout_7;
    QLabel *roiLabel_1;
    QSpinBox *roiSpinBox_x;
    QLabel *roiLabel_2;
    QSpinBox *roiSpinBox_y;
    QLabel *roiLabel_3;
    QSpinBox *roiSpinBox_w;
    QLabel *roiLabel_4;
    QSpinBox *roiSpinBox_h;
    QLabel *roiLabel_5;
    QSpacerItem *horizontalSpacer_10;
    QGroupBox *videoCaptureGroupBox;
    QGridLayout *gridLayout_3;
    QHBoxLayout *horizontalLayout_6;
    QLabel *capIntervalLabel;
    QSpacerItem *horizontalSpacer_9;
    QSpinBox *capIntervalSpinBox;
    QSpacerItem *horizontalSpacer_2;
    QSpacerItem *horizontalSpacer;
    QSpacerItem *verticalSpacer;
    QWidget *tab_2;
    QHBoxLayout *horizontalLayout_5;
    QSpacerItem *horizontalSpacer_5;
    QPushButton *okBtn;
    QPushButton *cancelBtn;

    void setupUi(QDialog *SettingDialog)
    {
        if (SettingDialog->objectName().isEmpty())
            SettingDialog->setObjectName(QStringLiteral("SettingDialog"));
        SettingDialog->setWindowModality(Qt::ApplicationModal);
        SettingDialog->setEnabled(true);
        SettingDialog->resize(529, 470);
        gridLayout = new QGridLayout(SettingDialog);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        tabWidget = new QTabWidget(SettingDialog);
        tabWidget->setObjectName(QStringLiteral("tabWidget"));
        tab = new QWidget();
        tab->setObjectName(QStringLiteral("tab"));
        gridLayout_4 = new QGridLayout(tab);
        gridLayout_4->setSpacing(6);
        gridLayout_4->setContentsMargins(11, 11, 11, 11);
        gridLayout_4->setObjectName(QStringLiteral("gridLayout_4"));
        imageProcessGroupBox = new QGroupBox(tab);
        imageProcessGroupBox->setObjectName(QStringLiteral("imageProcessGroupBox"));
        gridLayout_2 = new QGridLayout(imageProcessGroupBox);
        gridLayout_2->setSpacing(6);
        gridLayout_2->setContentsMargins(11, 11, 11, 11);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        sensorCheckBox = new QCheckBox(imageProcessGroupBox);
        sensorCheckBox->setObjectName(QStringLiteral("sensorCheckBox"));

        gridLayout_2->addWidget(sensorCheckBox, 0, 0, 1, 1);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(6);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        angleHighLabel = new QLabel(imageProcessGroupBox);
        angleHighLabel->setObjectName(QStringLiteral("angleHighLabel"));

        horizontalLayout->addWidget(angleHighLabel);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_3);

        angleHighSpinBox = new QDoubleSpinBox(imageProcessGroupBox);
        angleHighSpinBox->setObjectName(QStringLiteral("angleHighSpinBox"));

        horizontalLayout->addWidget(angleHighSpinBox);


        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        angleLowLabel = new QLabel(imageProcessGroupBox);
        angleLowLabel->setObjectName(QStringLiteral("angleLowLabel"));

        horizontalLayout_2->addWidget(angleLowLabel);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_4);

        angleLowSpinBox = new QDoubleSpinBox(imageProcessGroupBox);
        angleLowSpinBox->setObjectName(QStringLiteral("angleLowSpinBox"));

        horizontalLayout_2->addWidget(angleLowSpinBox);


        verticalLayout->addLayout(horizontalLayout_2);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        radiusMinLabel = new QLabel(imageProcessGroupBox);
        radiusMinLabel->setObjectName(QStringLiteral("radiusMinLabel"));

        horizontalLayout_3->addWidget(radiusMinLabel);

        horizontalSpacer_6 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_6);

        radiusMinSpinBox = new QSpinBox(imageProcessGroupBox);
        radiusMinSpinBox->setObjectName(QStringLiteral("radiusMinSpinBox"));
        radiusMinSpinBox->setMaximum(1000);

        horizontalLayout_3->addWidget(radiusMinSpinBox);


        verticalLayout->addLayout(horizontalLayout_3);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(6);
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        radiusMaxLabel = new QLabel(imageProcessGroupBox);
        radiusMaxLabel->setObjectName(QStringLiteral("radiusMaxLabel"));

        horizontalLayout_4->addWidget(radiusMaxLabel);

        horizontalSpacer_8 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer_8);

        radiusMaxSpinBox = new QSpinBox(imageProcessGroupBox);
        radiusMaxSpinBox->setObjectName(QStringLiteral("radiusMaxSpinBox"));
        radiusMaxSpinBox->setMaximum(1000);

        horizontalLayout_4->addWidget(radiusMaxSpinBox);


        verticalLayout->addLayout(horizontalLayout_4);

        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setSpacing(6);
        horizontalLayout_7->setObjectName(QStringLiteral("horizontalLayout_7"));
        roiLabel_1 = new QLabel(imageProcessGroupBox);
        roiLabel_1->setObjectName(QStringLiteral("roiLabel_1"));

        horizontalLayout_7->addWidget(roiLabel_1);

        roiSpinBox_x = new QSpinBox(imageProcessGroupBox);
        roiSpinBox_x->setObjectName(QStringLiteral("roiSpinBox_x"));
        roiSpinBox_x->setMaximum(1280);

        horizontalLayout_7->addWidget(roiSpinBox_x);

        roiLabel_2 = new QLabel(imageProcessGroupBox);
        roiLabel_2->setObjectName(QStringLiteral("roiLabel_2"));

        horizontalLayout_7->addWidget(roiLabel_2);

        roiSpinBox_y = new QSpinBox(imageProcessGroupBox);
        roiSpinBox_y->setObjectName(QStringLiteral("roiSpinBox_y"));
        roiSpinBox_y->setMaximum(720);

        horizontalLayout_7->addWidget(roiSpinBox_y);

        roiLabel_3 = new QLabel(imageProcessGroupBox);
        roiLabel_3->setObjectName(QStringLiteral("roiLabel_3"));

        horizontalLayout_7->addWidget(roiLabel_3);

        roiSpinBox_w = new QSpinBox(imageProcessGroupBox);
        roiSpinBox_w->setObjectName(QStringLiteral("roiSpinBox_w"));
        roiSpinBox_w->setMaximum(1280);

        horizontalLayout_7->addWidget(roiSpinBox_w);

        roiLabel_4 = new QLabel(imageProcessGroupBox);
        roiLabel_4->setObjectName(QStringLiteral("roiLabel_4"));

        horizontalLayout_7->addWidget(roiLabel_4);

        roiSpinBox_h = new QSpinBox(imageProcessGroupBox);
        roiSpinBox_h->setObjectName(QStringLiteral("roiSpinBox_h"));
        roiSpinBox_h->setMaximum(720);

        horizontalLayout_7->addWidget(roiSpinBox_h);

        roiLabel_5 = new QLabel(imageProcessGroupBox);
        roiLabel_5->setObjectName(QStringLiteral("roiLabel_5"));

        horizontalLayout_7->addWidget(roiLabel_5);


        verticalLayout->addLayout(horizontalLayout_7);


        gridLayout_2->addLayout(verticalLayout, 1, 0, 1, 1);

        horizontalSpacer_10 = new QSpacerItem(37, 17, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_2->addItem(horizontalSpacer_10, 1, 1, 1, 1);


        gridLayout_4->addWidget(imageProcessGroupBox, 0, 0, 1, 2);

        videoCaptureGroupBox = new QGroupBox(tab);
        videoCaptureGroupBox->setObjectName(QStringLiteral("videoCaptureGroupBox"));
        gridLayout_3 = new QGridLayout(videoCaptureGroupBox);
        gridLayout_3->setSpacing(6);
        gridLayout_3->setContentsMargins(11, 11, 11, 11);
        gridLayout_3->setObjectName(QStringLiteral("gridLayout_3"));
        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setSpacing(6);
        horizontalLayout_6->setObjectName(QStringLiteral("horizontalLayout_6"));
        capIntervalLabel = new QLabel(videoCaptureGroupBox);
        capIntervalLabel->setObjectName(QStringLiteral("capIntervalLabel"));

        horizontalLayout_6->addWidget(capIntervalLabel);

        horizontalSpacer_9 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_6->addItem(horizontalSpacer_9);

        capIntervalSpinBox = new QSpinBox(videoCaptureGroupBox);
        capIntervalSpinBox->setObjectName(QStringLiteral("capIntervalSpinBox"));
        capIntervalSpinBox->setMaximum(1000);

        horizontalLayout_6->addWidget(capIntervalSpinBox);


        gridLayout_3->addLayout(horizontalLayout_6, 0, 0, 1, 1);

        horizontalSpacer_2 = new QSpacerItem(111, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_3->addItem(horizontalSpacer_2, 0, 1, 1, 1);


        gridLayout_4->addWidget(videoCaptureGroupBox, 1, 0, 1, 1);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_4->addItem(horizontalSpacer, 1, 1, 1, 1);

        verticalSpacer = new QSpacerItem(20, 213, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_4->addItem(verticalSpacer, 3, 0, 1, 1);

        tabWidget->addTab(tab, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QStringLiteral("tab_2"));
        tabWidget->addTab(tab_2, QString());

        gridLayout->addWidget(tabWidget, 0, 0, 1, 1);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setSpacing(6);
        horizontalLayout_5->setObjectName(QStringLiteral("horizontalLayout_5"));
        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_5->addItem(horizontalSpacer_5);

        okBtn = new QPushButton(SettingDialog);
        okBtn->setObjectName(QStringLiteral("okBtn"));

        horizontalLayout_5->addWidget(okBtn);

        cancelBtn = new QPushButton(SettingDialog);
        cancelBtn->setObjectName(QStringLiteral("cancelBtn"));

        horizontalLayout_5->addWidget(cancelBtn);


        gridLayout->addLayout(horizontalLayout_5, 1, 0, 1, 1);

#ifndef QT_NO_SHORTCUT
        angleHighLabel->setBuddy(angleHighSpinBox);
        angleLowLabel->setBuddy(angleLowSpinBox);
        radiusMinLabel->setBuddy(radiusMinSpinBox);
        radiusMaxLabel->setBuddy(radiusMaxSpinBox);
        capIntervalLabel->setBuddy(capIntervalSpinBox);
#endif // QT_NO_SHORTCUT
        QWidget::setTabOrder(tabWidget, sensorCheckBox);
        QWidget::setTabOrder(sensorCheckBox, angleHighSpinBox);
        QWidget::setTabOrder(angleHighSpinBox, angleLowSpinBox);
        QWidget::setTabOrder(angleLowSpinBox, radiusMinSpinBox);
        QWidget::setTabOrder(radiusMinSpinBox, radiusMaxSpinBox);
        QWidget::setTabOrder(radiusMaxSpinBox, capIntervalSpinBox);
        QWidget::setTabOrder(capIntervalSpinBox, okBtn);
        QWidget::setTabOrder(okBtn, cancelBtn);

        retranslateUi(SettingDialog);
        QObject::connect(okBtn, SIGNAL(clicked()), SettingDialog, SLOT(accept()));
        QObject::connect(cancelBtn, SIGNAL(clicked()), SettingDialog, SLOT(reject()));

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(SettingDialog);
    } // setupUi

    void retranslateUi(QDialog *SettingDialog)
    {
        SettingDialog->setWindowTitle(QApplication::translate("SettingDialog", "SettingDialog", Q_NULLPTR));
        imageProcessGroupBox->setTitle(QApplication::translate("SettingDialog", "Image Process", Q_NULLPTR));
        sensorCheckBox->setText(QApplication::translate("SettingDialog", "Sensor triggered", Q_NULLPTR));
        angleHighLabel->setText(QApplication::translate("SettingDialog", "highRatio", Q_NULLPTR));
        angleLowLabel->setText(QApplication::translate("SettingDialog", "lowRatio", Q_NULLPTR));
        radiusMinLabel->setText(QApplication::translate("SettingDialog", "&radius_min", Q_NULLPTR));
        radiusMaxLabel->setText(QApplication::translate("SettingDialog", "&radius_max", Q_NULLPTR));
        roiLabel_1->setText(QApplication::translate("SettingDialog", "roiRect(", Q_NULLPTR));
        roiLabel_2->setText(QApplication::translate("SettingDialog", ",", Q_NULLPTR));
        roiLabel_3->setText(QApplication::translate("SettingDialog", ",", Q_NULLPTR));
        roiLabel_4->setText(QApplication::translate("SettingDialog", ",", Q_NULLPTR));
        roiLabel_5->setText(QApplication::translate("SettingDialog", ")", Q_NULLPTR));
        videoCaptureGroupBox->setTitle(QApplication::translate("SettingDialog", "Video Capture", Q_NULLPTR));
        capIntervalLabel->setText(QApplication::translate("SettingDialog", "&capInterval", Q_NULLPTR));
        tabWidget->setTabText(tabWidget->indexOf(tab), QApplication::translate("SettingDialog", "Tab 1", Q_NULLPTR));
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QApplication::translate("SettingDialog", "Tab 2", Q_NULLPTR));
        okBtn->setText(QApplication::translate("SettingDialog", "OK", Q_NULLPTR));
        cancelBtn->setText(QApplication::translate("SettingDialog", "Cancel", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class SettingDialog: public Ui_SettingDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SETTINGDIALOG_H
