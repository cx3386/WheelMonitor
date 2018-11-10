#include "AlarmLCDBoard.h"
#include "stdafx.h"

AlarmLCDBoard::AlarmLCDBoard(QWidget* parent)
    : QFrame(parent)
{
    ui.setupUi(this);
    //�ѵ�����
    auto stackedLayout = new QStackedLayout(this);
    stackedLayout->setContentsMargins(0, 0, 0, 0);
    stackedLayout->setSpacing(0);
    stackedLayout->setStackingMode(QStackedLayout::StackAll);
    //�ײ㻭��
    auto canvas = new QWidget(this);
    auto hLayout = new QHBoxLayout(canvas);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(0);
    //��ʾ����Ȧ�ֶ�
    devName = new QLineEdit(this);
    devName->setObjectName(QStringLiteral("alarmIOLineEdit"));
    devName->setStyleSheet("color:red;font:40pt");
    devName->setAlignment(Qt::AlignRight);
    devName->setFrame(false);
    devName->setReadOnly(true);
    //��ʾ���������ֶ�
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
    //͸���ɰ棬���ڽ���clicked�ź�
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
            return true; //������ϣ��������ϴ�
        } else {
            return false; //û�д������ϴ���
        }
    } else {
        // pass the event on to the parent class
        return QFrame::eventFilter(watched, event);
    }
}