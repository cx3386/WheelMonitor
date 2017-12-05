#include "stdafx.h"
#include "datatablewidget.h"
#include <QtSql>

DataTableWidget::DataTableWidget(QWidget *parent)
	: QWidget(parent)
{
	model = new QSqlTableModel(this);

	model->setTable("wheels");
	model->setEditStrategy(QSqlTableModel::OnManualSubmit);
	model->select();

	model->setHeaderData(0, Qt::Horizontal, QStringLiteral("ID"));
	model->setHeaderData(1, Qt::Horizontal, QStringLiteral("车轮序号"));
	model->setHeaderData(2, Qt::Horizontal, QStringLiteral("时间"));
	model->setHeaderData(3, Qt::Horizontal, QStringLiteral("fragment"));
	model->setHeaderData(4, Qt::Horizontal, QStringLiteral("匹配数"));
	model->setHeaderData(5, Qt::Horizontal, QStringLiteral("有效数"));
	model->setHeaderData(6, Qt::Horizontal, QStringLiteral("误差"));
	model->setHeaderData(7, Qt::Horizontal, QStringLiteral("台车速度"));
	model->setHeaderData(8, Qt::Horizontal, QStringLiteral("speedsStr"));
	model->setHeaderData(9, Qt::Horizontal, QStringLiteral("alarm"));
	model->setHeaderData(10, Qt::Horizontal, QStringLiteral("checked"));
	model->setHeaderData(11, Qt::Horizontal, QStringLiteral("录像位置"));

	QTableView *view = new QTableView(this);
	view->setModel(model);
	view->setEditTriggers(QAbstractItemView::NoEditTriggers);
	view->setSortingEnabled(true);
	view->setSelectionBehavior(QAbstractItemView::SelectRows);
	view->setSelectionMode(QAbstractItemView::SingleSelection);
	view->setShowGrid(true);
	view->verticalHeader()->hide();
	view->setAlternatingRowColors(true);

	view->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
	view->resizeColumnToContents(2);
	view->resizeColumnToContents(3);
	view->hideColumn(5);
	view->hideColumn(6);
	view->hideColumn(7);
	view->hideColumn(8);
	view->hideColumn(9);
	//connect(view, &QTableView::clicked, this, &DataTableWidget::readVideoPath);
	connect(view, &QTableView::activated, this, &DataTableWidget::readVideoPath);
	QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(view, 0, 0);
	setLayout(layout);
}

DataTableWidget::~DataTableWidget()
{
}

bool DataTableWidget::insertRecord(QString num, QString time, int fragment, int totalmatch, int validmatch, double error, double cartspeed, QString speeds, bool alarm, bool checked, QString videopath)
{
	QSqlRecord record = model->record();
	record.setValue("num", QVariant(num));
	record.setValue("time", QVariant(time));
	record.setValue("fragment", QVariant(fragment));
	record.setValue("totalmatch", QVariant(totalmatch));
	record.setValue("validmatch", QVariant(validmatch));
	if (validmatch)
	{ //if no validmatch, error is null
		record.setValue("error", QVariant(error));
	}
	if (totalmatch)
	{ //if no match, cartspeed is null	//in fact there always is cartspeed frome ad
		record.setValue("cartspeed", QVariant(cartspeed));
	}
	record.setValue("speeds", QVariant(speeds));
	record.setValue("alarm", QVariant(alarm));
	record.setValue("checked", QVariant(checked));
	record.setValue("videopath", QVariant(videopath)); //relative path 20170912/241343.mp4
	return model->insertRecord(-1, record);			   //no need to submitAll()
}

void DataTableWidget::readVideoPath(QModelIndex index)
{
	QSqlRecord record = model->record(index.row());

	QString path = record.value("videopath").toString();

	emit showVideo(QUrl::fromLocalFile(path));
}