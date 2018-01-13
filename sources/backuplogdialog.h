#ifndef BACKUPLOGDIALOG_H
#define BACKUPLOGDIALOG_H

#include <QDialog>

struct BackupInfo
{
	QString path;
	bool isSelected;
	bool isAll;
	int day;
	int max_day;
	//QStringList files;
};

class BackupLogDialog : public QDialog {
	Q_OBJECT

public:
	explicit BackupLogDialog(QWidget* parent = nullptr);
	~BackupLogDialog();

	private slots:
	void chooseDirPath();

	bool startCopy();

	bool zipFiles(QString fileCompressd, QStringList files, QString srcRootPath);
	//void on_dirPathLineEdit_textChanged(const QString& arg1);

private:

	QList<BackupInfo> backupInfoList;
	QStringList getBackupFiles() const;
	QStringList getDayList(QString path, int day = 36500) const;
	quint64 getDiskFreeSpace(QString driver) const;	//driver free space in bytes
	quint64 getNeedSpace() const;	//need space in bytes
	QString desDirPath;
	quint64 lfNeedSpace;
	quint64 lfFreeSpace;

	QLabel *freeSpaceLabel;
	QLineEdit *dirPathLineEdit;
	QPushButton *copyBtn;
};

#endif // BACKUPLOGDIALOG_H
