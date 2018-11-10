#include "AlarmLCDBoard.h"
#include "stdafx.h"

AlarmLCDBoard::AlarmLCDBoard(QWidget* parent)
    : QFrame(parent)
{
    ui.setupUi(this);
    //堆叠布局
    auto stackedLayout = new QStackedLayout(this);
    stackedLayout->setContentsMargins(0, 0, 0, 0);
    stackedLayout->setSpacing(0);
    stackedLayout->setStackingMode(QStackedLayout::StackAll);
    //底层画版
    auto canvas = new QWidget(this);
    auto hLayout = new QHBoxLayout(canvas);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(0);
    //显示内外圈字段
    devName = new QLineEdit(this);
    devName->setObjectName(QStringLiteral("alarmIOLineEdit"));
    devName->setStyleSheet("color:red;font:40pt");
    devName->setAlignment(Qt::AlignRight);
    devName->setFrame(false);
    devName->setReadOnly(true);
    //显示报警号码字段
    alarmNum = new QLineEdit(this);
    alarmNum->setObjectName(QStringLiteral("alarmNumLineEdit"));
    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    alarmNum->setSizePolicy(sizePolicy);
    //alarmNumLineEdit->setFixedSize(130, 116);
    alarmNum->setStyleSheet("color:red;font:75 68pt \"Agency FB\";");
    alarmNum->setFrame(false);
    alarmNum->setReadOnly(true);
    hLayout->addWidget(devName);
    hLayout->addWidget(alarmNum);
    //透明蒙版，用于接受clicked信号
    masking = new QWidget(this);
    masking->setCursor(QCursor(Qt::PointingHandCursor));

    stackedLayout->addWidget(canvas);
    stackedLayout->addWidget(masking);
    masking->raise();
    masking->setWindowFlags(Qt::WindowStaysOnTopHint);
    masking->setStyleSheet("background:transparent");
    masking->installEventFilter(this);
}

AlarmLCDBoard::~AlarmLCDBoard()
{
}

bool AlarmLCDBoard::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == masking) {
        if (event->type() == QEvent::MouseButtonRelease) {
            emit clicked();
            return true; //处理完毕，不再向上传
        } else {
            return false; //没有处理，往上传递
        }
    } else {
        // pass the event on to the parent class
        return QFrame::eventFilter(watched, event);
    }
}