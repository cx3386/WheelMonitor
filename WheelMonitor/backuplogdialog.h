#ifndef BACKUPLOGDIALOG_H
#define BACKUPLOGDIALOG_H

#include <QDialog>

struct BackupInfo
{
	QString dirPath;
	bool isSelected;
	bool isAll;
	int day;
	int max_day;
	//QStringList files;
};

class BackupLogDialog : public QDialog {
	Q_OBJECT

public:
	explicit BackupLogDialog(QWidget* parent = 0);
	~BackupLogDialog();

	private slots:
	void chooseDirPath();

	bool startCopy();

	//void on_dirPathLineEdit_textChanged(const QString& arg1);

private:

	QList<BackupInfo> backupInfoList;
	QStringList getFiles(const BackupInfo& backupInfo);
	QStringList getFiles(const QString & srcDirPath, const int & nDays, const bool & isAll);
	int getMaxDay(const QString &path);
	quint64 getDiskFreeSpace(QString & driver);	//driver free space in bytes
	quint64 getNeedSpace();	//need space in bytes
	QString desDirPath;
	quint64 lfNeedSpace;
	quint64 lfFreeSpace;

	QLabel *freeSpaceLabel;
	QLineEdit *dirPathLineEdit;
	QPushButton *copyBtn;
};

#endif // BACKUPLOGDIALOG_H
