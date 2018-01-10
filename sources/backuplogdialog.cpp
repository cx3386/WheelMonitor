#include "backuplogdialog.h"
#include "common.h"
#include "JlCompress.h"

BackupLogDialog::BackupLogDialog(QWidget *parent)
	: QDialog(parent)
	, lfNeedSpace(0.0), lfFreeSpace(0.0)
{
	BackupInfo bifs[3] = { {logDirPath,true,true,0,0},{videoDirPath,false,true,0,0},{ocrDirPath,false,true,0,0} };
	for (auto&& bif : std::as_const(bifs))
	{
		backupInfoList << bif;
	}
	for (auto& bif : backupInfoList)//to write the bif, use deduction to forwarding reference, and NOT use std::as_const
	{
		bif.max_day = getMaxDay(bif.dirPath);
		bif.day = std::min(10, bif.max_day);
	}

	/************************************************************************/
	/* UI                                                                     */
	/************************************************************************/
	setWindowTitle(QStringLiteral("������־"));

	QStringList itemsList;
	itemsList << QStringLiteral("��־�ļ�(log)")
		<< QStringLiteral("¼���ļ�")
		<< QStringLiteral("�ַ�ʶ���ļ�");
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
		label_save->setText(QStringLiteral("����"));
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
			backupInfoList[id].day = d; //QList::at(int) ��ֻ������
		});

		QLabel *label_day = new QLabel(this);
		label_day->setText(QStringLiteral("��"));
		//radioButoon, select the all or some days
		QRadioButton *someDaysBtn = new QRadioButton(this);
		QRadioButton *allDaysBtn = new QRadioButton(this);
		allDaysBtn->setText(QStringLiteral("ȫ��"));
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
	contentGroupBox->setTitle(QStringLiteral("ѡ��Ҫ���ݵ�����"));
	contentGroupBox->setLayout(contentLayout);

	QLabel *label_saveto = new QLabel(this);
	label_saveto->setText(QStringLiteral("��ѡ������ݿ�������"));
	dirPathLineEdit = new QLineEdit(this);
	dirPathLineEdit->setFocusPolicy(Qt::NoFocus);	//can't be select
	QPushButton *dirPathBtn = new QPushButton(this);
	dirPathBtn->setText(QStringLiteral("ѡ��Ŀ¼"));
	connect(dirPathBtn, &QPushButton::clicked, this, &BackupLogDialog::chooseDirPath);
	QHBoxLayout *dirHLayout = new QHBoxLayout;
	dirHLayout->addWidget(dirPathLineEdit);
	dirHLayout->addWidget(dirPathBtn);

	freeSpaceLabel = new QLabel(this);
	freeSpaceLabel->setText(QStringLiteral("��Ҫ�ռ�: %1 GB|���ô��̿ռ�: %2 GB").arg(QString::number(0, 'f', 2)).arg(QString::number(0, 'f', 2))); //arg(lfFreeSpace, 0, 'f', 2)
	QHBoxLayout *hLayout_2 = new QHBoxLayout;
	hLayout_2->addStretch();
	hLayout_2->addWidget(freeSpaceLabel);

	QVBoxLayout *vLayout_2 = new QVBoxLayout;
	vLayout_2->addWidget(label_saveto);
	vLayout_2->addLayout(dirHLayout);
	vLayout_2->addLayout(hLayout_2);

	QGroupBox *dirPathGroupBox = new QGroupBox(this);
	dirPathGroupBox->setTitle(QStringLiteral("����Ŀ¼"));
	dirPathGroupBox->setLayout(vLayout_2);

	copyBtn = new QPushButton(this);
	copyBtn->setText(QStringLiteral("��ʼ����"));
	copyBtn->setEnabled(false);
	connect(copyBtn, &QPushButton::clicked, this, &BackupLogDialog::startCopy);
	QPushButton *cancelBtn = new QPushButton(this);
	cancelBtn->setText(QStringLiteral("ȡ��"));
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
		freeSpaceLabel->setText(QStringLiteral("<font color = red>��Ҫ�ռ�: %1 GB|���ô��̿ռ�: %2 GB</font>").arg(QString::number(needGB, 'f', 2)).arg(QString::number(freeGB, 'f', 2))); //arg(lfFreeSpace, 0, 'f', 2)
	}
	else
	{
		freeSpaceLabel->setText(QStringLiteral("��Ҫ�ռ�: %1 GB|���ô��̿ռ�: %2 GB").arg(QString::number(needGB, 'f', 2)).arg(QString::number(freeGB, 'f', 2))); //arg(lfFreeSpace, 0, 'f', 2)
	}
	if (lfFreeSpace < lfNeedSpace)
	{
		QMessageBox::warning(this, QStringLiteral("������־"), QStringLiteral("�ռ䲻�㣬������ѡ��"), QStringLiteral("ȷ��"));
		return false;
	}

	QProgressDialog pgDlg(QStringLiteral("�����ļ���..."), QStringLiteral("ȡ��"), 0, (int)(lfNeedSpace / 1024), this);
	pgDlg.setMinimumDuration(0);	//time(ms) before the dialog appears
	pgDlg.setWindowModality(Qt::WindowModal);
	quint64 curSize = 0;
	QDir srcDir(appDirPath);
	for (auto&& bif : std::as_const(backupInfoList))
	{
		if (bif.isSelected)
		{
			for (auto&& qs : getFiles(bif))//fileList must start with srcPath
			{
				if (copyOneFile(srcDir, qs, pgDlg, curSize)) return true;
			}
		}
	}
	if (copyOneFile(srcDir, databaseFilePath, pgDlg, curSize)) return true;
	QMessageBox::information(this, QStringLiteral("������־"), QStringLiteral("�ļ�������ɣ�"), QStringLiteral("ȷ��"));
	return true;
}

bool BackupLogDialog::copyOneFile(QDir &srcDir, QString & srcFilePath, QProgressDialog &pgDlg, quint64 &curSize)
{
	QString relativeFilePath = srcDir.relativeFilePath(srcFilePath);
	QString desFileName = QString("%1/%2").arg(desDirPath).arg(relativeFilePath);//get the relative path
	QFileInfo desFileInfo(desFileName);
	QFile desFile(desFileName);
	//desFile.setPermissions(QFile::WriteOwner);
	if (!desFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
	{
		QDir dir;
		dir.mkpath(desFileInfo.absolutePath());//desFileInfo.canonicalPath() //returns "."
		desFile.open(QIODevice::WriteOnly);
	}
	QFile srcFile(srcFilePath);
	srcFile.open(QIODevice::ReadOnly);

	QByteArray byteArray;
	int count = 0;
	while (!srcFile.atEnd())
	{
		count++;
		byteArray = srcFile.read(1024);
		desFile.write(byteArray);
		pgDlg.setValue(curSize / 1024 + count);

		if (pgDlg.wasCanceled())
		{
			desFile.close();
			srcFile.close();
			QFile::remove(desFileName);
			pgDlg.setValue(lfNeedSpace / 1024);	//close
			return true;
		}
		qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
	}

	desFile.close();
	srcFile.close();
	curSize += desFileInfo.size();
	return false;
}

void BackupLogDialog::chooseDirPath()
{
	copyBtn->setEnabled(false);
	QString usbDrivePath(QDir::drives().back().filePath());//the last of the drives
	desDirPath = QFileDialog::getExistingDirectory(this, QStringLiteral("ѡ�񱸷�Ŀ¼"), usbDrivePath, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (desDirPath.isEmpty())
		return;

	if (desDirPath.endsWith("/")) //if des is root, i.e.,"c:/", it will endswith /
	{
		desDirPath.chop(1);
	}
	desDirPath.append("/").append(BACKUP_FOLDER_NAME);
	dirPathLineEdit->setText(desDirPath);
	lfNeedSpace = getNeedSpace();
	lfFreeSpace = getDiskFreeSpace(desDirPath.left(3));
	auto needGB = (double)lfNeedSpace / 1024 / 1024 / 1024;
	auto freeGB = (double)lfFreeSpace / 1024 / 1024 / 1024;
	if (lfFreeSpace < lfNeedSpace)
	{
		freeSpaceLabel->setText(QStringLiteral("<font color = red>��Ҫ�ռ�: %1 GB|���ô��̿ռ�: %2 GB</font>").arg(QString::number(needGB, 'f', 2)).arg(QString::number(freeGB, 'f', 2))); //arg(lfFreeSpace, 0, 'f', 2)
	}
	else
	{
		freeSpaceLabel->setText(QStringLiteral("��Ҫ�ռ�: %1 GB|���ô��̿ռ�: %2 GB").arg(QString::number(needGB, 'f', 2)).arg(QString::number(freeGB, 'f', 2))); //arg(lfFreeSpace, 0, 'f', 2)
	}
	copyBtn->setEnabled(true);
}

QStringList BackupLogDialog::getFiles(const BackupInfo & info) const
{//overload
	return getFiles(info.dirPath, info.day, info.isAll);
}

QStringList BackupLogDialog::getFiles(const QString & path, const int & day, const bool & isAll) const
{//filter the dirs by retain days
	QStringList files;
	QDir dir(path);
	dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot); //prevent deleting the '.' and '..' dirs, very important
	auto list = dir.entryInfoList();
	int nToday = QDate::currentDate().toString("yyyyMMdd").toInt();
	int minDay;
	if (isAll) { minDay = 19700000 - 1; }
	else { minDay = nToday - day; }

	for (auto&& info : std::as_const(list))
	{
		bool ok;
		int nFileDay = info.fileName().toInt(&ok);
		if (ok && (nFileDay > minDay) && nFileDay <= 21000000) {
			QDirIterator it(info.absoluteFilePath(), QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
			while (it.hasNext()) {
				it.next();
				files << it.filePath();
			}
		}
	}
	return files;
}

int BackupLogDialog::getMaxDay(const QString &path) const
{
	int count = 0;
	QDir dir(path);
	dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
	auto list = dir.entryInfoList();
	int nToday = QDate::currentDate().toString("yyyyMMdd").toInt();
	for (auto&& info : std::as_const(list))
	{
		bool ok;
		int nFileDay = info.fileName().toInt(&ok);
		if (ok && (nFileDay <= nToday) && nFileDay >= 19700000) {
			++count;
		}
	}
	return count;
}

quint64 BackupLogDialog::getNeedSpace() const
{//get the total size of fileList
	quint64 size = 0;
	for (auto&& bif : std::as_const(backupInfoList)) {
		if (!bif.isSelected)
		{
			continue;
		}
		for (auto&& qsFilePath : getFiles(bif))
		{
			QFileInfo info(qsFilePath);
			size += info.size();
		}
	}
	QFileInfo info(databaseFilePath);
	size += info.size();
	return size;
}

quint64 BackupLogDialog::getDiskFreeSpace(QString & driver) const
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