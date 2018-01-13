#include "backuplogdialog.h"
#include "common.h"
#include "quazip.h"
#include "quazipfile.h"

BackupLogDialog::BackupLogDialog(QWidget *parent)
	: QDialog(parent)
	, lfNeedSpace(0.0), lfFreeSpace(0.0)
{
	BackupInfo bifs[3] = { {logDirPath,true,true},{videoDirPath,false,true},{ocrDirPath,false,true} };
	for (auto&& bif : std::as_const(bifs))
	{
		backupInfoList << bif;
	}
	for (auto& bif : backupInfoList)//to write the bif, use deduction to forwarding reference, and NOT use std::as_const
	{
		bif.max_day = getDayList(bif.path).size();
		bif.day = std::min(10, bif.max_day);
	}

	/************************************************************************/
	/* UI                                                                     */
	/************************************************************************/
	setWindowTitle(QStringLiteral("备份日志"));

	QStringList itemsList;
	itemsList << QStringLiteral("日志文件(log)")
		<< QStringLiteral("录像文件")
		<< QStringLiteral("字符识别文件");
	QVBoxLayout *contentLayout = new QVBoxLayout;

	int i = 0;
	for (auto&& backupInfo : std::as_const(backupInfoList)) //create a ui for each backupInfo
	{
		//checkbox
		QCheckBox *selCheckBox = new QCheckBox;
		selCheckBox->setText(itemsList.at(i));
		selCheckBox->setObjectName(QString("%1").arg(i));
		selCheckBox->setChecked(backupInfo.isSelected);
		/************************************************************************/
		/* isSel                                                                     */
		/************************************************************************/
		connect(selCheckBox, &QCheckBox::toggled, this, [&](const bool b) {
			QCheckBox *cb = qobject_cast<QCheckBox *>(sender());
			int id = cb->objectName().toInt();
			backupInfoList[id].isSelected = b;
		});

		QLabel *label_save = new QLabel(this);
		label_save->setText(QStringLiteral("保存"));
		QSpinBox *spinBox = new QSpinBox(this);
		spinBox->setObjectName(QString("%1").arg(i));
		spinBox->setMinimum(0);
		spinBox->setMaximum(backupInfo.max_day);
		spinBox->setValue(backupInfo.day);
		spinBox->setEnabled(false);
		/************************************************************************/
		/* day                                                                     */
		/************************************************************************/
		connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [&](const int d) {
			QSpinBox *sp = qobject_cast<QSpinBox *>(sender()); //without holding up sender, the cast will fail
			int id = sp->objectName().toInt();
			backupInfoList[id].day = d; //QList::at(int) 是只读访问
		});

		QLabel *label_day = new QLabel(this);
		label_day->setText(QStringLiteral("天"));
		//radioButoon, select the all or some days
		QRadioButton *someDaysBtn = new QRadioButton(this);
		QRadioButton *allDaysBtn = new QRadioButton(this);
		allDaysBtn->setText(QStringLiteral("全部"));
		allDaysBtn->setChecked(backupInfo.isAll);

		QButtonGroup *btnGroup = new QButtonGroup(this);
		btnGroup->setObjectName(QString("%1").arg(i));
		btnGroup->addButton(someDaysBtn, 0);
		btnGroup->addButton(allDaysBtn, 1);
		/************************************************************************/
		/* all                                                                     */
		/************************************************************************/
		connect(btnGroup, static_cast<void (QButtonGroup::*)(int, bool)>(&QButtonGroup::buttonToggled), this, [&](const int btnID, bool state) {
			QButtonGroup *bg = qobject_cast<QButtonGroup *>(sender());
			int id = bg->objectName().toInt();
			switch (btnID)
			{
			case 0:
				this->findChild<QSpinBox *>(QString("%1").arg(id))->setEnabled(state);
				break;
			case 1:
				backupInfoList[id].isAll = state;
			default:
				break;
			}
		});

		QHBoxLayout *layout = new QHBoxLayout;
		layout->addWidget(selCheckBox);
		layout->addStretch();
		layout->addWidget(someDaysBtn);
		layout->addWidget(label_save);
		layout->addWidget(spinBox);
		layout->addWidget(label_day);
		layout->addWidget(allDaysBtn);
		contentLayout->addLayout(layout);
		++i;
	}
	QGroupBox *contentGroupBox = new QGroupBox(this);
	contentGroupBox->setTitle(QStringLiteral("选择要备份的内容"));
	contentGroupBox->setLayout(contentLayout);

	QLabel *label_saveto = new QLabel(this);
	label_saveto->setText(QStringLiteral("将选择的内容拷贝到："));
	dirPathLineEdit = new QLineEdit(this);
	dirPathLineEdit->setFocusPolicy(Qt::NoFocus);	//can't be select
	QPushButton *dirPathBtn = new QPushButton(this);
	dirPathBtn->setText(QStringLiteral("选择目录"));
	connect(dirPathBtn, &QPushButton::clicked, this, &BackupLogDialog::chooseDirPath);
	QHBoxLayout *dirHLayout = new QHBoxLayout;
	dirHLayout->addWidget(dirPathLineEdit);
	dirHLayout->addWidget(dirPathBtn);

	freeSpaceLabel = new QLabel(this);
	freeSpaceLabel->setText(QStringLiteral("需要空间: %1 GB|可用磁盘空间: %2 GB").arg(QString::number(0, 'f', 2)).arg(QString::number(0, 'f', 2))); //arg(lfFreeSpace, 0, 'f', 2)
	QHBoxLayout *hLayout_2 = new QHBoxLayout;
	hLayout_2->addStretch();
	hLayout_2->addWidget(freeSpaceLabel);

	QVBoxLayout *vLayout_2 = new QVBoxLayout;
	vLayout_2->addWidget(label_saveto);
	vLayout_2->addLayout(dirHLayout);
	vLayout_2->addLayout(hLayout_2);

	QGroupBox *dirPathGroupBox = new QGroupBox(this);
	dirPathGroupBox->setTitle(QStringLiteral("备份目录"));
	dirPathGroupBox->setLayout(vLayout_2);

	copyBtn = new QPushButton(this);
	copyBtn->setText(QStringLiteral("开始拷贝"));
	copyBtn->setEnabled(false);
	connect(copyBtn, &QPushButton::clicked, this, &BackupLogDialog::startCopy);
	QPushButton *cancelBtn = new QPushButton(this);
	cancelBtn->setText(QStringLiteral("取消"));
	connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
	QHBoxLayout *hLayout_ok = new QHBoxLayout;
	hLayout_ok->addStretch();
	hLayout_ok->addWidget(copyBtn);
	hLayout_ok->addWidget(cancelBtn);

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(contentGroupBox);
	mainLayout->addWidget(dirPathGroupBox);
	mainLayout->addLayout(hLayout_ok);
	setLayout(mainLayout);
	setMinimumSize(20, 20);
}

BackupLogDialog::~BackupLogDialog()
{
}

bool BackupLogDialog::startCopy()
{
	lfNeedSpace = getNeedSpace();
	lfFreeSpace = getDiskFreeSpace(desDirPath.left(3));
	auto needGB = (double)lfNeedSpace / 1024 / 1024 / 1024;
	auto freeGB = (double)lfFreeSpace / 1024 / 1024 / 1024;
	if (lfFreeSpace < lfNeedSpace)
	{
		freeSpaceLabel->setText(QStringLiteral("<font color = red>需要空间: %1 GB|可用磁盘空间: %2 GB</font>").arg(QString::number(needGB, 'f', 2)).arg(QString::number(freeGB, 'f', 2))); //arg(lfFreeSpace, 0, 'f', 2)
	}
	else
	{
		freeSpaceLabel->setText(QStringLiteral("需要空间: %1 GB|可用磁盘空间: %2 GB").arg(QString::number(needGB, 'f', 2)).arg(QString::number(freeGB, 'f', 2))); //arg(lfFreeSpace, 0, 'f', 2)
	}
	if (lfFreeSpace < lfNeedSpace)
	{
		QMessageBox::warning(this, QStringLiteral("备份日志"), QStringLiteral("空间不足，请重新选择"), QStringLiteral("确认"));
		return false;
	}
	QDir desDir(desDirPath);
	auto zipFile = desDir.absoluteFilePath(BACKUP_ZIP_NAME);
	if (!zipFiles(zipFile, getBackupFiles(), appDirPath)) { return false; }
	QMessageBox::information(this, QStringLiteral("备份日志"), QStringLiteral("日志备份完成！"), QStringLiteral("确认"));
	return true;
}

//************************************
// Method:    zipFiles no-recursively
// FullName:  BackupLogDialog::zipFiles
// Access:    private
// Returns:   bool
// Qualifier: const
// Parameter: QString fileCompressd: absoluteFilePath of zip file
// Parameter: QStringList files: absolute/relativeFilePath List of files to zip
// Parameter: QString srcRootPath
//************************************
bool BackupLogDialog::zipFiles(QString fileCompressd, QStringList files, QString srcRootPath)
{
	QuaZip zip(fileCompressd);
	if (!zip.open(QuaZip::mdCreate)) {
		return false;
	}

	QuaZipFile outZipFile(&zip);

	// Copy file and folder to zip file
	QDir srcRootDir(srcRootPath);
	QProgressDialog pgDlg(QStringLiteral("正在复制备份文件..."), QStringLiteral("取消"), 0, (int)(lfNeedSpace / 1024), this);
	pgDlg.setMinimumDuration(0);	//time(ms) before the dialog appears
	pgDlg.setWindowModality(Qt::WindowModal);
	quint64 curSize = 0;
	for (auto && file : files) {
		auto relativeFile = srcRootDir.relativeFilePath(file);
		auto absoluteFile = srcRootDir.absoluteFilePath(file);
		QFileInfo info(absoluteFile);
		// FOLDER (this is the part that interests you!!!)
		if (info.isDir()) {
			if (!outZipFile.open(QIODevice::WriteOnly, QuaZipNewInfo(relativeFile + "/", absoluteFile))) {
				return false;
			}
			outZipFile.close();
		}
		// FILE
		else if (info.isFile()) {
			QFile inFile(absoluteFile);
			if (!inFile.open(QIODevice::ReadOnly)) {
				zip.close();
				return false;
			}
			if (!outZipFile.open(QIODevice::WriteOnly, QuaZipNewInfo(relativeFile, absoluteFile))) {
				inFile.close();
				zip.close();
				return false;
			}

			// Copy
			QByteArray buffer;
			while (!inFile.atEnd())
			{
				buffer = inFile.read(1024);
				outZipFile.write(buffer);
				curSize += buffer.count();
				pgDlg.setValue(curSize / 1024);
				qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
				if (pgDlg.wasCanceled())
				{
					outZipFile.close();
					inFile.close();
					pgDlg.setValue(lfNeedSpace / 1024);	//close
					return false;
				}
			}
			outZipFile.close();
			inFile.close();
		}
		else {
			// Probably symbolic, ignore
		}
	}
	zip.close();
	return true;
}

void BackupLogDialog::chooseDirPath()
{
	copyBtn->setEnabled(false);
	QString usbDrivePath(QDir::drives().back().filePath());//the last of the drives
	desDirPath = QFileDialog::getExistingDirectory(this, QStringLiteral("选择备份目录"), usbDrivePath, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (desDirPath.isEmpty())
		return;
	QDir desDir(desDirPath);
	dirPathLineEdit->setText(desDir.absoluteFilePath(BACKUP_ZIP_NAME));
	lfNeedSpace = getNeedSpace();
	lfFreeSpace = getDiskFreeSpace(desDirPath.left(3));
	auto needGB = (double)lfNeedSpace / 1024 / 1024 / 1024;
	auto freeGB = (double)lfFreeSpace / 1024 / 1024 / 1024;
	if (lfFreeSpace < lfNeedSpace)
	{
		freeSpaceLabel->setText(QStringLiteral("<font color = red>需要空间: %1 GB|可用磁盘空间: %2 GB</font>").arg(QString::number(needGB, 'f', 2)).arg(QString::number(freeGB, 'f', 2))); //arg(lfFreeSpace, 0, 'f', 2)
	}
	else
	{
		freeSpaceLabel->setText(QStringLiteral("需要空间: %1 GB|可用磁盘空间: %2 GB").arg(QString::number(needGB, 'f', 2)).arg(QString::number(freeGB, 'f', 2))); //arg(lfFreeSpace, 0, 'f', 2)
	}
	copyBtn->setEnabled(true);
}

QStringList BackupLogDialog::getBackupFiles() const
{
	QStringList files;
	for (auto &&backupInfo : backupInfoList)
	{
		if (!backupInfo.isSelected) { continue; }
		QStringList dirs;
		if (backupInfo.isAll) { dirs = getDayList(backupInfo.path); }
		else { dirs = getDayList(backupInfo.path, backupInfo.day); }
		for (auto && dir : dirs)
		{
			files << dir;
			QDirIterator it(dir, QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
			while (it.hasNext()) {
				it.next();
				files << it.filePath();
			}
		}
	}
	files << databaseFilePath;
	return files;
}

QStringList BackupLogDialog::getDayList(QString path, int day /*= 36500*/) const
{
	QDir dir(path);
	dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
	auto list = dir.entryInfoList();
	int nToday = QDate::currentDate().toString("yyyyMMdd").toInt();
	QStringList dayList;
	int count = 0;
	for (auto&& info : std::as_const(list))
	{
		bool ok;
		int nFileDay = info.fileName().toInt(&ok);
		if (ok && (nFileDay <= nToday) && nFileDay >= 19700000) {
			count++;
			if (count > day) break;
			dayList << info.filePath();
		}
	}
	return dayList;
}

quint64 BackupLogDialog::getNeedSpace() const
{//get the total size of fileList
	quint64 size = 0;
	auto files = getBackupFiles();
	for (auto && file : files) {
		QFileInfo info(file);
		size += info.size();
	}
	return size;
}

quint64 BackupLogDialog::getDiskFreeSpace(QString driver) const
{//e.g."C:\"
	LPCWSTR lpcwstrDriver = (LPCWSTR)driver.utf16();
	ULARGE_INTEGER liFreeBytesAvailable, liTotalBytes, liTotalFreeBytes;
	if (!GetDiskFreeSpaceEx(lpcwstrDriver, &liFreeBytesAvailable, &liTotalBytes, &liTotalFreeBytes))
	{
		qDebug() << "BackupLogDialog: Call to GetDiskFreeSpaceEx() failed.";
		return 0;
	}
	return (quint64)liTotalFreeBytes.QuadPart; //(quint64)
}