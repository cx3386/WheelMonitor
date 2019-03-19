#include "stdafx.h"

#include "common.h"
#include "AlarmLCDBoard.h"
#include "ByteArrayWidget.h"

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
	auto style = QString{ "color:red;background:transparent;" };
	// 代理：用于接收设备号
	devIndex = new QLineEdit(this);
	devIndex->setVisible(false);
	connect(devIndex, &QLineEdit::textChanged, this, [=](QString devIndex_str) {
		devName->setText(getDeviceMark(devIndex_str.toInt()));
	});
	// 代理：用于接收车牌
	plate_ba = new ByteArrayWidget(this);
	plate_ba->setVisible(false);
	connect(plate_ba, &ByteArrayWidget::dataChanged, this, [=](QByteArray data) {
		QImage im;
		im.loadFromData(data, "JPG");
		plate->setPixmap(QPixmap::fromImage(im));
	});
	//显示内外圈字段
	devName = new QLineEdit(this);
	devName->setObjectName(QStringLiteral("alarmIOLineEdit"));
	devName->setStyleSheet(style);
	devName->setAlignment(Qt::AlignRight);
	devName->setFrame(false);
	devName->setReadOnly(true);
	//显示报警号码字段
	alarmNum = new QLineEdit(this);
	alarmNum->setObjectName(QStringLiteral("alarmNumLineEdit"));
	QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	alarmNum->setSizePolicy(sizePolicy);
	//font:75 68pt \"Agency FB\"
	alarmNum->setStyleSheet(style);
	alarmNum->setFrame(false);
	alarmNum->setReadOnly(true);
	//显示plate图像
	plate = new QLabel(this);
	plate->setObjectName(QStringLiteral("lb_plate"));
	//sizePolicy = QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	//plate->setSizePolicy(sizePolicy);
	plate->setScaledContents(true);
	plate->setFixedWidth(100);
	plate->setFixedHeight(53);
	//布局
	hLayout->addWidget(devName);
	hLayout->addWidget(alarmNum);
	hLayout->addWidget(plate);
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
		}
		else {
			return false; //没有处理，往上传递
		}
	}
	else {
		// pass the event on to the parent class
		return QFrame::eventFilter(watched, event);
	}
}