#include "stdafx.h"
#include "playbackwidget.h"
#include "player.h"
#include <QtSql>
#include "common.h"
#include "mysqltablemodel.h"

PlayBackWidget::PlayBackWidget(QWidget *parent)
	: QWidget(parent)
{
	model = new MySqlTableModel(this);

	model->setTable("wheels");
	model->setEditStrategy(QSqlTableModel::OnManualSubmit);
	model->select();
	view = new QTableView(this);
	view->setModel(model);
	//view->setWindowTitle("");
	view->setFont(QFont("Arial Narrow", 14));
	view->horizontalHeader()->setFont(QFont(QStringLiteral("宋体"), 14));
	view->setEditTriggers(QAbstractItemView::NoEditTriggers);
	view->setSortingEnabled(true);
	view->setSelectionBehavior(QAbstractItemView::SelectRows);
	view->setSelectionMode(QAbstractItemView::SingleSelection);
	view->setShowGrid(true);
	view->setAlternatingRowColors(true);
	view->horizontalHeader()->setStretchLastSection(true);
	//connect(view, &QTableView::clicked, this, &FaultPlayBackWidget::readVideoPath);
	connect(view, &QTableView::activated, this, &PlayBackWidget::readVideoPath); //double-click or enter

	showAlarm();

	QComboBox *viewCb = new QComboBox(this);
	viewCb->setEditable(false);
	viewCb->addItem(QStringLiteral("报警"));
	viewCb->addItem(QStringLiteral("全部"));
	viewCb->setCurrentIndex(0);
	connect(viewCb, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [&](const int i) {
		switch (i)
		{
		case 0: showAlarm(); break;
		case 1: showAll(); break;
		default:
			break;
		}});
	///failure level indication
	QFormLayout *fLayout = new QFormLayout;
	QLabel *lv1Label = new QLabel(this);
	lv1Label->setFixedSize(30, 18);
	lv1Label->setStyleSheet("background-color:rgb(255,255,0);");
	QLabel *lv1TextLabel = new QLabel(this);
	lv1TextLabel->setText(QStringLiteral("轻微"));
	lv1TextLabel->setStyleSheet("font:12pt;");
	fLayout->addRow(lv1Label, lv1TextLabel);
	QLabel *lv2Label = new QLabel(this);
	lv2Label->setFixedSize(30, 20);
	lv2Label->setStyleSheet("background-color:rgb(255,192,0);");
	QLabel *lv2TextLabel = new QLabel(this);
	lv2TextLabel->setText(QStringLiteral("一般"));
	lv2TextLabel->setStyleSheet("font:12pt;");
	fLayout->addRow(lv2Label, lv2TextLabel);
	QLabel *lv3Label = new QLabel(this);
	lv3Label->setFixedSize(30, 20);
	lv3Label->setStyleSheet("background-color:rgb(255,0,0);");
	QLabel *lv3TextLabel = new QLabel(this);
	lv3TextLabel->setText(QStringLiteral("严重"));
	lv3TextLabel->setStyleSheet("font:12pt;");
	fLayout->addRow(lv3Label, lv3TextLabel);

	QPushButton *checkSelBtn = new QPushButton(this);
	checkSelBtn->setText(QStringLiteral("解除"));  //delete
	connect(checkSelBtn, &QPushButton::clicked, this, &PlayBackWidget::setSelectedChecked);
	QPushButton *checkAllBtn = new QPushButton(this);
	checkAllBtn->setText(QStringLiteral("全清"));  //clear all
	connect(checkAllBtn, &QPushButton::clicked, this, &PlayBackWidget::setAllChecked);
	QHBoxLayout *toolLayout = new QHBoxLayout;
	toolLayout->addWidget(viewCb);
	toolLayout->addWidget(checkSelBtn);
	toolLayout->addWidget(checkAllBtn);
	toolLayout->addLayout(fLayout);
	toolLayout->addStretch();

	player = new Player(this);
	QHBoxLayout *hLayout = new QHBoxLayout;
	hLayout->addWidget(view);
	hLayout->addWidget(player);
	hLayout->setStretchFactor(view, 1);
	hLayout->setStretchFactor(player, 1);
	//#TESTONLY
	//auto* tmpLabel = new QLabel(this);
	//tmpLabel->setScaledContents(true);
	//tmpLabel->setPixmap(QPixmap(":/images/Resources/images/tmp.png"));
	QVBoxLayout *vLayout = new QVBoxLayout;
	vLayout->addLayout(toolLayout);
	vLayout->addLayout(hLayout);
	//vLayout->addWidget(tmpLabel);
	setLayout(vLayout);
}

void PlayBackWidget::showAlarm()
{
	view->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	view->resizeColumnsToContents();
	model->setFilter("checkstate=1");
	model->setHeaderData(Wheels_Num, Qt::Horizontal, QStringLiteral("序号"));
	model->setHeaderData(Wheels_Time, Qt::Horizontal, QStringLiteral("报警时间"));
	model->setHeaderData(Wheels_CalcSpeed, Qt::Horizontal, QStringLiteral("测量"));
	model->setHeaderData(Wheels_RefSpeed, Qt::Horizontal, QStringLiteral("参考"));
	model->setHeaderData(Wheels_Error, Qt::Horizontal, QStringLiteral("±%"));
	view->hideColumn(0);
	for (int i = Wheels_AlarmLevel; i <= Wheels_VideoPath; ++i)
	{
		view->hideColumn(i);
	}
}

void PlayBackWidget::showAll()
{
	view->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
	view->resizeColumnsToContents();
	model->setFilter("");
	model->setHeaderData(Wheels_ID, Qt::Horizontal, QStringLiteral("ID"));
	model->setHeaderData(Wheels_Num, Qt::Horizontal, QStringLiteral("num"));
	model->setHeaderData(Wheels_Time, Qt::Horizontal, QStringLiteral("time"));
	model->setHeaderData(Wheels_CalcSpeed, Qt::Horizontal, QStringLiteral("calcspeed"));
	model->setHeaderData(Wheels_RefSpeed, Qt::Horizontal, QStringLiteral("refspeed"));
	model->setHeaderData(Wheels_Error, Qt::Horizontal, QStringLiteral("error"));
	for (int i = 0; i <= Wheels_VideoPath; ++i)
	{
		view->setColumnHidden(i, false);
	}
}

PlayBackWidget::~PlayBackWidget()
= default;

bool PlayBackWidget::hasAlarm() const
{
	QSqlTableModel model;
	model.setTable("wheels");
	model.setFilter("checkstate=1");
	model.select();
	if (model.rowCount())
	{
		return true;
	}
	return false;
}

bool PlayBackWidget::insertRecord(const QString &num, int alarmLevel, double absError, double refSpeed, int fragment, int totalMatch, int validMatch, const QString &savedSpeeds, const QString &videoPath)
{
	QSqlRecord record = model->record();
	QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
	auto calSpeed_rough = QString::number(absError + refSpeed, 'f', 1).toDouble();
	auto refSpeed_rough = QString::number(refSpeed, 'f', 1).toDouble();
	auto relError = QString::number(absError / refSpeed * 100, 'f', 0).toDouble();
	record.setValue(Wheels_Num, QVariant(num));
	record.setValue(Wheels_Time, QVariant(time));
	record.setValue(Wheels_CalcSpeed, QVariant(calSpeed_rough));
	record.setValue(Wheels_RefSpeed, QVariant(refSpeed_rough));
	record.setValue(Wheels_Error, QVariant(relError));
	record.setValue(Wheels_AlarmLevel, QVariant(alarmLevel));
	int checkState;
	switch (alarmLevel)
	{
	case -2:
	case 1:
	case 2:
		checkState = NeedCheck;
		break;
	case -1:
	case 0:
	default:
		checkState = NoNeedCheck;
		break;
	}
	record.setValue(Wheels_CheckState, QVariant(checkState));
	record.setValue(Wheels_Speeds, QVariant(savedSpeeds));
	record.setValue(Wheels_Fragment, QVariant(fragment));
	record.setValue(Wheels_TotalMatch, QVariant(totalMatch));
	record.setValue(Wheels_ValidMatch, QVariant(validMatch));
	record.setValue(Wheels_VideoPath, QVariant(videoPath)); //relative path, e.g. 20170912/241343.mp4
	bool r = model->insertRecord(-1, record);
	r &= model->submitAll(); //on manualsubmit, use submitAll()
	return r;
}

void PlayBackWidget::readVideoPath(QModelIndex index)
{
	QString relativePath = model->data(model->index(index.row(), Wheels_VideoPath)).toString();
	QString fullPath = QString("%1/%2").arg(videoDirPath).arg(relativePath);

	player->setUrl(QUrl::fromLocalFile(fullPath));
}

void PlayBackWidget::setSelectedChecked()
{
	if (!hasAlarm())
	{
		QMessageBox::information(this, QStringLiteral("解除警报"), QStringLiteral("当前无故障"), QStringLiteral("确定"));
		return;
	}
	QModelIndexList selected = view->selectionModel()->selectedRows();
	if (selected.empty())
	{
		QMessageBox::information(this, QStringLiteral("解除警报"), QStringLiteral("请选中一行记录，确认无故障后删除"), QStringLiteral("确定"));
		return;
	}
	for (auto&& slc : selected)
	{
		model->setData(model->index(slc.row(), Wheels_CheckState), QVariant(Checked));
	}
	model->submitAll();
	if (!hasAlarm())
	{
		emit setAlarmLight(PLCSerial::AlarmColorGreen);
	}
}

void PlayBackWidget::setAllChecked()
{
	if (!hasAlarm())
	{
		QMessageBox::information(this, QStringLiteral("解除警报"), QStringLiteral("当前无故障"), QStringLiteral("确定"));
		return;
	}
	auto rows = model->rowCount();
	for (int i = 0; i < rows; ++i)
	{
		model->setData(model->index(i, Wheels_CheckState), QVariant(Checked));
	}
	model->submitAll();
	emit setAlarmLight(PLCSerial::AlarmColorGreen);
}