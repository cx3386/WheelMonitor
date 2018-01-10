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
	view->setFont(QFont("Times", 16));
	view->horizontalHeader()->setFont(QFont("Times", 16));
	view->setEditTriggers(QAbstractItemView::NoEditTriggers);
	view->setSortingEnabled(true);
	view->setSelectionBehavior(QAbstractItemView::SelectRows);
	view->setSelectionMode(QAbstractItemView::SingleSelection);
	view->setShowGrid(true);
	view->setAlternatingRowColors(true);
	view->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	view->horizontalHeader()->setStretchLastSection(true);
	view->resizeColumnsToContents();
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
	QPushButton *checkSelBtn = new QPushButton(this);
	checkSelBtn->setText(QStringLiteral("解除当前警报"));
	connect(checkSelBtn, &QPushButton::clicked, this, &PlayBackWidget::setSelectedChecked);
	QPushButton *checkAllBtn = new QPushButton(this);
	checkAllBtn->setText(QStringLiteral("全部解除"));
	connect(checkAllBtn, &QPushButton::clicked, this, &PlayBackWidget::setAllChecked);
	QHBoxLayout *toolLayout = new QHBoxLayout;
	toolLayout->addWidget(viewCb);
	toolLayout->addWidget(checkSelBtn);
	toolLayout->addWidget(checkAllBtn);
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
	model->setFilter("checked=0");
	model->setHeaderData(Wheels_Num, Qt::Horizontal, QStringLiteral("序号"));
	model->setHeaderData(Wheels_Time, Qt::Horizontal, QStringLiteral("时间"));
	model->setHeaderData(Wheels_CalcSpeed, Qt::Horizontal, QStringLiteral("测速"));
	model->setHeaderData(Wheels_RefSpeed, Qt::Horizontal, QStringLiteral("参考"));
	model->setHeaderData(Wheels_Error, Qt::Horizontal, QStringLiteral("误差(%)"));
	model->setHeaderData(Wheels_AlarmLevel, Qt::Horizontal, QStringLiteral("故障等级"));
	view->hideColumn(0);
	for (int i = Wheels_Checked; i <= Wheels_VideoPath; ++i)
	{
		view->hideColumn(i);
	}
}

void PlayBackWidget::showAll()
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

PlayBackWidget::~PlayBackWidget()
= default;

bool PlayBackWidget::insertRecord(const QString &num, int alarmLevel, double absError, double refSpeed, int fragment, int totalMatch, int validMatch, const QString &savedSpeeds, const QString &videoPath)
{
	QSqlRecord record = model->record();
	QString time = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
	auto calSpeed_rough = QString::number(absError + refSpeed, 'f', 1).toDouble();
	auto refSpeed_rough = QString::number(refSpeed, 'f', 1).toDouble();
	auto relError = QString::number(absError / refSpeed * 100, 'f', 0).toDouble();
	record.setValue(Wheels_Num, QVariant(num));
	record.setValue(Wheels_Time, QVariant(time));
	record.setValue(Wheels_CalcSpeed, QVariant(calSpeed_rough));
	record.setValue(Wheels_RefSpeed, QVariant(refSpeed_rough));
	record.setValue(Wheels_Error, QVariant(relError));
	record.setValue(Wheels_AlarmLevel, QVariant(alarmLevel));
	if (alarmLevel > 0) {
		record.setValue(Wheels_Checked, QVariant(false));
	}
	record.setValue(Wheels_Speeds, QVariant(savedSpeeds));
	record.setValue(Wheels_Fragment, QVariant(fragment));
	record.setValue(Wheels_TotalMatch, QVariant(totalMatch));
	record.setValue(Wheels_ValidMatch, QVariant(validMatch));
	record.setValue(Wheels_VideoPath, QVariant(videoPath)); //relative path, e.g. 20170912/241343.mp4
	bool r = model->insertRecord(-1, record);
	r &= model->submitAll(); //on manualsubmit need to submitAll()
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
	QModelIndexList selected = view->selectionModel()->selectedRows();
	for (auto&& slc : selected)
	{
		model->setData(model->index(slc.row(), Wheels_Checked), QVariant(true));
		//QSqlRecord record = model->record(slc.row());
	}
	model->submitAll();
}

void PlayBackWidget::setAllChecked()
{
	auto rows = model->rowCount();
	for (int i = 0; i < rows; ++i)
	{
		model->setData(model->index(i, Wheels_Checked), QVariant(true));
	}
	model->submitAll();
}