#pragma once

#include <QObject>

class QProgressDialog;
class CopyFileWorker : public QObject
{
	Q_OBJECT

public:
	CopyFileWorker(QString& src, QString& des, QStringList& ls, QObject *parent = Q_NULLPTR);
	~CopyFileWorker();

	void setProgressDialog(QProgressDialog * val) { pgDlg = val; }
private:
	QString srcPath;
	QString desPath;
	QStringList fileList;
	QProgressDialog *pgDlg;

	private slots:
	bool startCopy();	//entrance;

signals:
	void setPgDlgValue(int val);
};
