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

class QLabel;
class QLineEdit;
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

	QStringList getDayList(QString path, bool isAll, int nDays) const; ///< get day list dirs for n days.
	QStringList getDayList(QString path, int day) const; ///< for some days.
	QStringList getDayList(QString path) const; ///< for all days.
	quint64 getDiskFreeSpace(QString driver) const;	//driver free space in bytes
	quint64 getNeedSpace() const;	//need space in bytes
	QString desFilePath;
	quint64 lfNeedSpace;
	quint64 lfFreeSpace;

	QLabel *freeSpaceLabel;
	QLineEdit *dirPathLineEdit;
	QPushButton *copyBtn;
};

#endif // BACKUPLOGDIALOG_H
