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

	bool copyFile(QDir &srcDir, QString & srcFilePath, QProgressDialog &pgDlg, quint64 &curSize);
	bool zipFiles(QDir &srcDir, QString & srcFilePath, QProgressDialog &pgDlg, quint64 &curSize) const;
	//void on_dirPathLineEdit_textChanged(const QString& arg1);

private:

	QList<BackupInfo> backupInfoList;
	QStringList getFiles(const BackupInfo& backupInfo) const;
	QStringList getFiles(const QString & srcDirPath, const int & nDays, const bool & isAll) const;
	int getMaxDay(const QString &path) const;
	quint64 getDiskFreeSpace(QString & driver) const;	//driver free space in bytes
	quint64 getNeedSpace() const;	//need space in bytes
	QString desDirPath;
	quint64 lfNeedSpace;
	quint64 lfFreeSpace;

	QLabel *freeSpaceLabel;
	QLineEdit *dirPathLineEdit;
	QPushButton *copyBtn;
};

#endif // BACKUPLOGDIALOG_H
