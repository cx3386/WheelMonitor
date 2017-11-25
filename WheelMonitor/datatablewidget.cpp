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

	model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
	model->setHeaderData(1, Qt::Horizontal, QObject::tr("³µÂÖÐòºÅ"));
	model->setHeaderData(2, Qt::Horizontal, QObject::tr("Last name"));

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

void DataTableWidget::readVideoPath(QModelIndex index)
{
	QSqlRecord record = model->record(index.row());

	QString path = record.value("videopath").toString();

	emit showVideo(QUrl::fromLocalFile(path));
}