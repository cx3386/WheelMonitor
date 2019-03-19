#include "stdafx.h"

#include "common.h"
#include "AlarmLCDBoard.h"
#include "ByteArrayWidget.h"

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
	auto style = QString{ "color:red;background:transparent;" };
	// �������ڽ����豸��
	devIndex = new QLineEdit(this);
	devIndex->setVisible(false);
	connect(devIndex, &QLineEdit::textChanged, this, [=](QString devIndex_str) {
		devName->setText(getDeviceMark(devIndex_str.toInt()));
	});
	// �������ڽ��ճ���
	plate_ba = new ByteArrayWidget(this);
	plate_ba->setVisible(false);
	connect(plate_ba, &ByteArrayWidget::dataChanged, this, [=](QByteArray data) {
		QImage im;
		im.loadFromData(data, "JPG");
		plate->setPixmap(QPixmap::fromImage(im));
	});
	//��ʾ����Ȧ�ֶ�
	devName = new QLineEdit(this);
	devName->setObjectName(QStringLiteral("alarmIOLineEdit"));
	devName->setStyleSheet(style);
	devName->setAlignment(Qt::AlignRight);
	devName->setFrame(false);
	devName->setReadOnly(true);
	//��ʾ���������ֶ�
	alarmNum = new QLineEdit(this);
	alarmNum->setObjectName(QStringLiteral("alarmNumLineEdit"));
	QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	alarmNum->setSizePolicy(sizePolicy);
	//font:75 68pt \"Agency FB\"
	alarmNum->setStyleSheet(style);
	alarmNum->setFrame(false);
	alarmNum->setReadOnly(true);
	//��ʾplateͼ��
	plate = new QLabel(this);
	plate->setObjectName(QStringLiteral("lb_plate"));
	//sizePolicy = QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	//plate->setSizePolicy(sizePolicy);
	plate->setScaledContents(true);
	plate->setFixedWidth(100);
	plate->setFixedHeight(53);
	//����
	hLayout->addWidget(devName);
	hLayout->addWidget(alarmNum);
	hLayout->addWidget(plate);
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
		}
		else {
			return false; //û�д������ϴ���
		}
	}
	else {
		// pass the event on to the parent class
		return QFrame::eventFilter(watched, event);
	}
}