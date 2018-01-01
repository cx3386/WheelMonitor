#include "stdafx.h"
#include "faultplaybackwidget.h"
#include "player.h"
#include <QtSql>
#include "common.h"

FaultPlayBackWidget::FaultPlayBackWidget(QWidget *parent)
	: QWidget(parent)
{
	model = new QSqlTableModel(this);

	model->setTable("wheels");
	model->setEditStrategy(QSqlTableModel::OnManualSubmit);
	model->select();
	//TODO
	//connect(model, &QSqlTableModel::rowsInserted, this, [&](QModelIndex &index, int first, int last) {
	//	QSqlRecord record = model->record(index.row());
	//	if (record.value(Wheels_AlarmLevel).toInt())
	//	{
	//		view->ite->
	//	}
	//});
	view = new QTableView(this);
	view->setModel(model);
	//view->setWindowTitle("");
	view->setFont(QFont("Times", 16));
	view->horizontalHeader()->setFont(QFont("Times", 16));
	view->setEditTriggers(QAbstractItemView::NoEditTriggers);
	view->setSortingEnabled(true);
	view->setSelectionBehavior(QAbstractItemView::SelectRows);
	view->setSelectionMode(QAbstractItemView::SingleSelection);
	view->setShowGrid(true);
	//view->verticalHeader()->hide();
	view->setAlternatingRowColors(true);
	view->horizontalHeader()->setStretchLastSection(true);
	view->resizeColumnsToContents();
	//connect(view, &QTableView::clicked, this, &FaultPlayBackWidget::readVideoPath);
	connect(view, &QTableView::activated, this, &FaultPlayBackWidget::readVideoPath); //double-click or enter

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
	QPushButton *checkBtn = new QPushButton(this);
	checkBtn->setText(QStringLiteral("解除选中的警报"));
	connect(checkBtn, &QPushButton::clicked, this, &FaultPlayBackWidget::setChecked);
	QHBoxLayout *toolLayout = new QHBoxLayout;
	toolLayout->addWidget(viewCb);
	toolLayout->addWidget(checkBtn);
	toolLayout->addStretch();

	player = new Player(this);
	QHBoxLayout *hLayout = new QHBoxLayout;
	hLayout->addWidget(view);
	hLayout->addWidget(player);

	//TESTONLY
	auto* tmpLabel = new QLabel(this);
	tmpLabel->setScaledContents(true);
	tmpLabel->setPixmap(QPixmap(":/images/Resources/images/tmp.png"));
	QVBoxLayout *vLayout = new QVBoxLayout;
	vLayout->addLayout(toolLayout);
	vLayout->addLayout(hLayout);
	vLayout->addWidget(tmpLabel);
	setLayout(vLayout);
}

void FaultPlayBackWidget::showAlarm()
{
	model->setFilter("checked=0");
	model->setHeaderData(Wheels_ID, Qt::Horizontal, QStringLiteral("ID"));
	model->setHeaderData(Wheels_Num, Qt::Horizontal, QStringLiteral("车轮序号"));
	model->setHeaderData(Wheels_Time, Qt::Horizontal, QStringLiteral("时间"));
	model->setHeaderData(Wheels_CalcSpeed, Qt::Horizontal, QStringLiteral("测量速度"));
	model->setHeaderData(Wheels_RefSpeed, Qt::Horizontal, QStringLiteral("参考速度"));
	model->setHeaderData(Wheels_Error, Qt::Horizontal, QStringLiteral("误差"));
	model->setHeaderData(Wheels_AlarmLevel, Qt::Horizontal, QStringLiteral("警报等级"));
	view->hideColumn(0);
	for (int i = Wheels_Checked; i <= Wheels_VideoPath; ++i)
	{
		view->hideColumn(i);
	}
}

void FaultPlayBackWidget::showAll()
{
	model->setFilter("");
	model->setHeaderData(Wheels_ID, Qt::Horizontal, QStringLiteral("ID"));
	model->setHeaderData(Wheels_Num, Qt::Horizontal, QStringLiteral("num"));
	model->setHeaderData(Wheels_Time, Qt::Horizontal, QStringLiteral("time"));
	model->setHeaderData(Wheels_CalcSpeed, Qt::Horizontal, QStringLiteral("calcspeed"));
	model->setHeaderData(Wheels_RefSpeed, Qt::Horizontal, QStringLiteral("refspeed"));
	model->setHeaderData(Wheels_Error, Qt::Horizontal, QStringLiteral("error"));
	model->setHeaderData(Wheels_AlarmLevel, Qt::Horizontal, QStringLiteral("alarmlevel"));
	for (int i = 0; i <= Wheels_VideoPath; ++i)
	{
		view->setColumnHidden(i, false);
	}
}

FaultPlayBackWidget::~FaultPlayBackWidget()
{
}

bool FaultPlayBackWidget::insertRecord(QString num, int fragment, int totalmatch, int validmatch, int alarmLevel, double error, double refspeed, QString speeds, QString videopath)
{
	QSqlRecord record = model->record();
	record.setValue(Wheels_Num, QVariant(num));
	QString time = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
	record.setValue(Wheels_Time, QVariant(time));
	record.setValue(Wheels_Fragment, QVariant(fragment));
	record.setValue(Wheels_TotalMatch, QVariant(totalmatch));
	record.setValue(Wheels_ValidMatch, QVariant(validmatch));
	if (validmatch)
	{ //if no validmatch, the following is null
		record.setValue(Wheels_Error, QVariant(error));
		record.setValue(Wheels_CalcSpeed, QVariant(error + refspeed));
		record.setValue(Wheels_AlarmLevel, QVariant(alarmLevel));
	}
	//refspeed is read directly from ad, never null
	record.setValue(Wheels_RefSpeed, QVariant(refspeed));
	record.setValue(Wheels_Speeds, QVariant(speeds));
	if (alarmLevel) { record.setValue(Wheels_Checked, QVariant(false)); }
	//else { record.setNull(Wheels_Checked); }
	record.setValue("videopath", QVariant(videopath)); //relative path, e.g. 20170912/241343.mp4
	bool r = model->insertRecord(-1, record);
	r &= model->submitAll();
	return r; //on manualsubmit need to submitAll()
}

bool FaultPlayBackWidget::isLastAlarm(const QString & num) const
{
	QSqlTableModel tmpModel;
	tmpModel.setTable("wheels");
	tmpModel.setFilter(QString("num='%1' and warn is not null and alarm is not null").arg(num));
	tmpModel.select();
	int row = tmpModel.rowCount();
	if (row > 0)
	{
		auto record = tmpModel.record(row - 1);
		if (record.value(Wheels_AlarmLevel).toInt())
			return true;
	}
	return false;
}

void FaultPlayBackWidget::readVideoPath(QModelIndex index)
{
	QSqlRecord record = model->record(index.row());

	QString relativePath = record.value(Wheels_VideoPath).toString();
	QString fullPath = QString("%1/%2").arg(videoDirPath).arg(relativePath);

	player->setUrl(QUrl::fromLocalFile(fullPath));
}

void FaultPlayBackWidget::setChecked()
{
	QModelIndexList selected = view->selectionModel()->selectedRows();
	if (selected.empty())
	{
		return;
	}
	for (auto&& index : selected)
	{
		QSqlRecord record = model->record(index.row());
		record.setValue(Wheels_Checked, QVariant(true));
		model->setRecord(index.row(), record);
	}
	model->submitAll();
}