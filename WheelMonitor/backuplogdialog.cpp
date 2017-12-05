#include "backuplogdialog.h"

BackupLogDialog::BackupLogDialog(QWidget *parent)
	: QDialog(parent)
{
	daysList << 10 << 10 << 10;
	contentList << true << false << false;
	usbDrivePath = QDir::drives().back().filePath(); //the last of the drives
	setWindowTitle(QStringLiteral("备份日志"));

	QStringList itemsList;
	itemsList << QStringLiteral("日志文件(log)")
		<< QStringLiteral("录像文件")
		<< QStringLiteral("匹配图片");
	QVBoxLayout *contentLayout = new QVBoxLayout;

	for (int i = 0; i < itemsList.size(); i++)
	{
		//checkbox
		QCheckBox *checkBox = new QCheckBox;
		checkBox->setText(itemsList.at(i));
		checkBox->setObjectName(QString("%1").arg(i));
		checkBox->setChecked(contentList.at(i));
		connect(checkBox, &QCheckBox::toggled, this, [&](const bool b)
		{
			QCheckBox *cb = qobject_cast<QCheckBox *>(sender());
			int id = cb->objectName().toInt();
			contentList[id] = b;
		});

		QLabel *label_save = new QLabel;
		label_save->setText(QStringLiteral("保存"));
		QSpinBox *spinBox = new QSpinBox;
		spinBox->setObjectName(QString("%1").arg(i));
		spinBox->setMinimum(1);
		spinBox->setValue(daysList.at(i));
		//spinBox->setMaximum(30);
		connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [&](const int d)
		{
			QSpinBox *sp = qobject_cast<QSpinBox *>(sender()); //without this, the cast will fail
			int id = sp->objectName().toInt();
			daysList[id] = d; //QList::at(int) 是只读访问
		});
		//connect(spinBox, static_cast<void (QSpinBox:: *)(int)>(&QSpinBox::valueChanged), this, &BackupLogDialog::daysChanged);

		QLabel *label_day = new QLabel;
		label_day->setText(QStringLiteral("天"));
		//radioButoon, select the all or some days
		QRadioButton *someDaysBtn = new QRadioButton;
		QRadioButton *allDaysBtn = new QRadioButton;
		allDaysBtn->setText(QStringLiteral("全部"));
		allDaysBtn->setChecked(true);

		QButtonGroup *btnGroup = new QButtonGroup(this);
		btnGroup->addButton(someDaysBtn, 0);
		btnGroup->addButton(allDaysBtn, 1);

		QHBoxLayout *layout = new QHBoxLayout;
		layout->addWidget(checkBox);
		layout->addStretch();
		layout->addWidget(someDaysBtn);
		layout->addWidget(label_save);
		layout->addWidget(spinBox);
		layout->addWidget(label_day);
		layout->addWidget(allDaysBtn);
		contentLayout->addLayout(layout);
	}
	QGroupBox *contentGroupBox = new QGroupBox;
	contentGroupBox->setTitle(QStringLiteral("选择要备份的内容"));
	contentGroupBox->setLayout(contentLayout);

	QLabel *label_saveto = new QLabel;
	label_saveto->setText(QStringLiteral("将选择的内容拷贝到："));
	dirPathLineEdit = new QLineEdit;
	dirPathLineEdit->setFocusPolicy(Qt::NoFocus);
	QPushButton *dirPathBtn = new QPushButton;
	dirPathBtn->setText(QStringLiteral("选择目录"));
	connect(dirPathBtn, &QPushButton::clicked, this, &BackupLogDialog::chooseDirPath);
	QHBoxLayout *dirHLayout = new QHBoxLayout;
	dirHLayout->addWidget(dirPathLineEdit);
	dirHLayout->addWidget(dirPathBtn);

	freeSpaceLabel = new QLabel;
	freeSpaceLabel->setText(QStringLiteral("可用磁盘空间：  GB"));
	QHBoxLayout *hLayout_2 = new QHBoxLayout;
	hLayout_2->addStretch();
	hLayout_2->addWidget(freeSpaceLabel);

	QVBoxLayout *vLayout_2 = new QVBoxLayout;
	vLayout_2->addWidget(label_saveto);
	vLayout_2->addLayout(dirHLayout);
	vLayout_2->addLayout(hLayout_2);

	QGroupBox *dirPathGroupBox = new QGroupBox;
	dirPathGroupBox->setTitle(QStringLiteral("备份目录"));
	dirPathGroupBox->setLayout(vLayout_2);

	copyBtn = new QPushButton;
	copyBtn->setText(QStringLiteral("开始拷贝"));
	connect(copyBtn, &QPushButton::clicked, this, &BackupLogDialog::startCopy);
	QPushButton *cancelBtn = new QPushButton();
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
}

BackupLogDialog::~BackupLogDialog()
{
}

void BackupLogDialog::chooseDirPath()
{
	QFileDialog fileDialog(this);
	fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
	fileDialog.setFileMode(QFileDialog::Directory);
	fileDialog.setOption(QFileDialog::ShowDirsOnly);
	fileDialog.setViewMode(QFileDialog::Detail);
	fileDialog.setWindowTitle(QStringLiteral("选择备份目录"));

	fileDialog.setDirectory(usbDrivePath);
	if (fileDialog.exec() == QDialog::Accepted)
	{
		copyDirPath = fileDialog.selectedFiles().at(0);
		dirPathLineEdit->setText(copyDirPath);
		copyBtn->setEnabled(true);
	}
}

void BackupLogDialog::startCopy()
{
	this->accept();
}
void BackupLogDialog::on_DirPathLineEdit_textChanged(const QString &arg1)
{
}