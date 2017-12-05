#ifndef BACKUPLOGDIALOG_H
#define BACKUPLOGDIALOG_H

#include <QDialog>

class BackupLogDialog : public QDialog {
	Q_OBJECT

public:
	explicit BackupLogDialog(QWidget* parent = 0);
	~BackupLogDialog();

	private slots:
	void chooseDirPath();

	void startCopy();

	void on_DirPathLineEdit_textChanged(const QString& arg1);

private:
	QString usbDrivePath;
	QString copyDirPath;

	QList<int> daysList;
	QList<bool> contentList;
	//QGroupBox *contentGroupBox;

	//QCheckBox *logCheckBox;
	//QCheckBox *vedioCheckBox;
	//QCheckBox *matchCheckBox;

	//QGroupBox *dirGroupBox;
	QLabel *freeSpaceLabel;
	QLineEdit *dirPathLineEdit;

	QPushButton *copyBtn;
};

#endif // BACKUPLOGDIALOG_H
