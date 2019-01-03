#include "stdafx.h"

#include "common.h"
#include "database.h"
#include "mysqltablemodel.h"
#include "playbackwidget.h"
#include "player.h"
#include <QtSql>

PlayBackWidget::PlayBackWidget(QWidget* parent)
	: QWidget(parent)
{
	initAlarmTable();
	initAllTable();

	//UI
	//failure level indication
	auto* labelHLayout = new QHBoxLayout;
	QLabel* indicateLabel = new QLabel(this);
	indicateLabel->setText(QStringLiteral("报警等级"));
	labelHLayout->addWidget(indicateLabel);
	//labelHLayout->addStretch();
	QString levelTexts[4] = { QStringLiteral("正常"), QStringLiteral("轻微"), QStringLiteral("一般"), QStringLiteral("严重") };
	QString styleSheets[4] = { "background-color:rgb(0,176,80);" /*green*/
		,
		"background-color:rgb(255,255,0);" /*yellow*/
		,
		"background-color:rgb(255,192,0);" /*orange*/
		,
		"background-color:rgb(255,0,0);" /*red*/ };
	for (int i = 0; i < 4; ++i) {
		QLabel* lvColorLabel = new QLabel(this);
		lvColorLabel->setFixedSize(30, 18);
		lvColorLabel->setStyleSheet(styleSheets[i]);
		QLabel* lvTextLabel = new QLabel(this);
		lvTextLabel->setText(levelTexts[i]);
		//lvTextLabel->setStyleSheet("font-size:12pt;");
		labelHLayout->addWidget(lvColorLabel, 0, Qt::AlignCenter);
		labelHLayout->addWidget(lvTextLabel, 0, Qt::AlignCenter);
	}

	selectTableCb = new QComboBox(this);
	selectTableCb->setEditable(false);
	selectTableCb->addItem(QStringLiteral("报警"));
	selectTableCb->addItem(QStringLiteral("全部"));
	checkSelBtn = new QPushButton(this);
	checkSelBtn->setText(QStringLiteral("解除")); //delete
	connect(checkSelBtn, &QPushButton::clicked, this, &PlayBackWidget::setSelectedChecked);
	checkAllBtn = new QPushButton(this);
	checkAllBtn->setText(QStringLiteral("全清")); //clear all
	connect(checkAllBtn, &QPushButton::clicked, this, &PlayBackWidget::setAllChecked);
	auto* toolLayout = new QHBoxLayout;
	toolLayout->addWidget(selectTableCb);
	toolLayout->addWidget(checkSelBtn);
	toolLayout->addWidget(checkAllBtn);
	toolLayout->addLayout(labelHLayout);
	toolLayout->addStretch();

	auto* tableLayout = new QStackedLayout;
	tableLayout->addWidget(alarmView);
	tableLayout->addWidget(allView);
	//connect(viewCb, SIGNAL(activated(int)), tableLayout, SLOT(setCurrentIndex(int)));
	//connect(selectTableCb, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, [=](int index) {});
	connect(selectTableCb, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [=](int index) {
		clearMedia();
		tableLayout->setCurrentIndex(index);
		bool enableCheck = false;
		switch (index)
		{
		case 0:enableCheck = true; break;
		case 1:enableCheck = false; break;
		default: break;
		}
		checkSelBtn->setEnabled(enableCheck);
		checkAllBtn->setEnabled(enableCheck);
	});
	///player
	player = new Player(this);
	auto* hLayout = new QHBoxLayout;
	hLayout->addLayout(tableLayout);
	hLayout->addWidget(player);
	hLayout->setStretchFactor(tableLayout, 1);
	hLayout->setStretchFactor(player, 1);
	auto* vLayout = new QVBoxLayout;
	vLayout->addLayout(toolLayout);
	vLayout->addLayout(hLayout);
	//vLayout->addWidget(tmpLabel);
	setLayout(vLayout);
}

void PlayBackWidget::initAlarmTable()
{
	alarmModel = new MySqlTableModel(this, QSqlDatabase::database(MAIN_CONNECTION_NAME));
	alarmModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
	alarmModel->setTable("wheels");
	alarmModel->setRelation(Wheel_I_O, QSqlRelation("devs", "devIndex", "name")); // 手动设置i_o的外键
	alarmModel->setFilter("checkstate=1");
	alarmModel->select();
	while (alarmModel->canFetchMore()) alarmModel->fetchMore();

	alarmModel->setHeaderData(Wheel_I_O, Qt::Horizontal, QStringLiteral("外/内圈"));
	alarmModel->setHeaderData(Wheel_Num, Qt::Horizontal, QStringLiteral("序号"));
	alarmModel->setHeaderData(Wheel_CalcSpeed, Qt::Horizontal, QStringLiteral("测量"));
	alarmModel->setHeaderData(Wheel_RefSpeed, Qt::Horizontal, QStringLiteral("参考"));
	alarmModel->setHeaderData(Wheel_Error, Qt::Horizontal, QStringLiteral("±%"));
	alarmModel->setHeaderData(Wheel_Time, Qt::Horizontal, QStringLiteral("报警时间"));

	alarmView = new QTableView(this);
	alarmView->setModel(alarmModel);
	//alarmView->setFont(QFont("Arial Narrow", 14));
	//alarmView->horizontalHeader()->setFont(QFont(QStringLiteral("宋体"), 14));
	alarmView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	alarmView->setSortingEnabled(true);
	alarmView->setSelectionBehavior(QAbstractItemView::SelectRows);
	alarmView->setSelectionMode(QAbstractItemView::SingleSelection);
	alarmView->setShowGrid(true);
	alarmView->setAlternatingRowColors(true);
	alarmView->horizontalHeader()->setStretchLastSection(true);
	alarmView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	alarmView->resizeColumnsToContents();
	alarmView->hideColumn(0);
	for (int i = Wheel_AlarmLevel; i <= Wheel_VideoPath; ++i) {
		alarmView->hideColumn(i);
	}
	connect(alarmView, &QTableView::activated, this, &PlayBackWidget::readVideoPath); //double-click or enter
}

void PlayBackWidget::initAllTable()
{
	allModel = new MySqlTableModel(this, QSqlDatabase::database(MAIN_CONNECTION_NAME));
	allModel->setTable("wheels");
	allModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
	allModel->select();
	while (allModel->canFetchMore()) allModel->fetchMore();

	allView = new QTableView(this);
	allView->setModel(allModel);
	//allView->setFont(QFont("Arial Narrow", 14));
	//allView->horizontalHeader()->setFont(QFont(QStringLiteral("宋体"), 14));
	allView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	allView->setSortingEnabled(true);
	allView->setSelectionBehavior(QAbstractItemView::SelectRows);
	allView->setSelectionMode(QAbstractItemView::SingleSelection);
	allView->setShowGrid(true);
	allView->setAlternatingRowColors(true);
	allView->horizontalHeader()->setStretchLastSection(true);
	allView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
	allView->resizeColumnsToContents();
	connect(allView, &QTableView::activated, this, &PlayBackWidget::readVideoPath); //double-click or enter
}

PlayBackWidget::~PlayBackWidget()
= default;

bool PlayBackWidget::hasAlarm() const
{
	return alarmModel->rowCount();
}

void PlayBackWidget::dbChanged()
{
	allModel->select();
	while (allModel->canFetchMore()) allModel->fetchMore();

	alarmModel->select();
	while (alarmModel->canFetchMore()) alarmModel->fetchMore();

	if (!hasAlarm()) {
		emit clearAlarm();
	}
}

void PlayBackWidget::clearMedia()
{
	QUrl url;
	player->setUrl(url);
}

void PlayBackWidget::readVideoPath(QModelIndex index) const
{
	QString path = allModel->data(allModel->index(index.row(), Wheel_VideoPath)).toString();
	QDir dir(videoDirPath);
	player->setUrl(QUrl::fromLocalFile(dir.absoluteFilePath(path)));
}

void PlayBackWidget::setSelectedChecked()
{
	if (!hasAlarm()) {
		QMessageBox::information(this, QStringLiteral("解除警报"), QStringLiteral("当前无故障"), QStringLiteral("确定"));
		return;
	}
	QModelIndexList selected = alarmView->selectionModel()->selectedRows();
	if (selected.empty()) {
		QMessageBox::information(this, QStringLiteral("解除警报"), QStringLiteral("请选中一行记录，确认无故障后删除"), QStringLiteral("确定"));
		return;
	}
	for (auto&& slc : selected) {
		alarmModel->setData(alarmModel->index(slc.row(), Wheel_CheckState), QVariant(Checked));
	}
	alarmModel->submitAll();
	while (alarmModel->canFetchMore())alarmModel->fetchMore();
}

void PlayBackWidget::setAllChecked()
{
	if (!hasAlarm()) {
		QMessageBox::information(this, QStringLiteral("解除警报"), QStringLiteral("当前无故障"), QStringLiteral("确定"));
		return;
	}
	auto rows = alarmModel->rowCount();
	for (int i = 0; i < rows; ++i) {
		alarmModel->setData(alarmModel->index(i, Wheel_CheckState), QVariant(Checked));
	}
	alarmModel->submitAll();
	while (alarmModel->canFetchMore())alarmModel->fetchMore();
}