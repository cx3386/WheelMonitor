#include "stdafx.h"
#include "mainwindow.h"
#include "common.h"
#include "backuplogdialog.h"
#include "playbackwidget.h"
#include "mysqltablemodel.h"
#include "hikvideocapture.h"
#include "imageprocess.h"
#include "outlierhelper.h"
#include "plcserial.h"
#include "settingdialog.h"
#include "confighelper.h"
#include "database.h"

MainWindow::MainWindow(ConfigHelper *_configHelper, QWidget *parent /*= Q_NULLPTR*/)
	: QMainWindow(parent)
	, configHelper(_configHelper)
	, plcSerial(new PLCSerial)
{
	//setWindowOpacity(1);
	ui.setupUi(this);
	configWindow();
	const auto deskRect = QApplication::desktop()->availableGeometry();
	if (deskRect.width() < 1920)
	{
		setWindowFlag(Qt::MSWindowsFixedSizeDialogHint);
		resize(deskRect.size());
	}

	//auto start
	if (configHelper->startAtLaunch) { on_action_Start_triggered(); }
}

MainWindow::~MainWindow()
{
	bool b = true;
	connect(plcSerial, &PLCSerial::setUiAlarm, this, [&b] {b = false; });  //before setUiAlarm finished, DONOT delete the pointer "this". 引发了异常: 读取访问权限冲突。**this** 是 0xFFFFFFFFFFFFFFFF
	emit setAlarm(AlarmOFF); //cannot operate out of thread. use Signal-Slot.
	while (b) //wait until light is set off
	{
		QCoreApplication::processEvents();
	}
	imageProcessThread[0]->quit();
	imageProcessThread[0]->wait();
	imageProcessThread[1]->quit();
	imageProcessThread[1]->wait();
	plcSerialThread->quit();
	plcSerialThread->wait();
	dbWatcherThread->quit();
	dbWatcherThread->wait();
}

void MainWindow::configWindow()
{
	/* alarm num ui show*/
	alarmNumUiSetup();

	/* Setup the record(save) indication of VideoCapture */
	recLabel_pre[0] = new QLabel(ui.cam0Tab);
	recLabel_pre[1] = new QLabel(ui.cam1Tab);
	for (auto i : recLabel_pre)
	{
		i->setGeometry(20, 20, 50, 35);
		i->setScaledContents(true); //scale its contents to fill all available space.
		i->setVisible(false);
	}
	recLabel_input[0] = new QLabel(ui.realVideoLabel_0);
	recLabel_input[1] = new QLabel(ui.realVideoLabel_1);
	for (auto i : recLabel_input)
	{
		i->setGeometry(10, 10, 40, 28);
		i->setScaledContents(true);
		i->setVisible(false);
	}
	onRecordOFF(0);
	onRecordOFF(1);

	/* playbackTab */
	playBackWidget = new PlayBackWidget(ui.playbackTab);
	auto *playBackLayout = new QGridLayout(ui.playbackTab);
	playBackLayout->addWidget(playBackWidget);
	connect(ui.centralTabWidget, &QTabWidget::currentChanged, playBackWidget, &PlayBackWidget::clearMedia);

	// monitorTab
	/* setup the mapper */
	QDataWidgetMapper *allMapper = new QDataWidgetMapper(this);
	allMapper->setModel((QSqlTableModel*)(playBackWidget->allModel));
	allMapper->addMapping(ui.numLineEdit, Wheel_Num);
	allMapper->addMapping(ui.lastSpeedLineEdit, Wheel_CalcSpeed);
	allMapper->toLast();
	QDataWidgetMapper *alarmMapper = new QDataWidgetMapper(this);
	alarmMapper->setModel((QSqlTableModel *)(playBackWidget->alarmModel));
	alarmMapper->addMapping(alarmIOLineEdit, Wheel_I_O);
	alarmMapper->addMapping(alarmNumLineEdit, Wheel_Num);
	alarmMapper->toLast();
	ui.numBackwardBtn->setEnabled(alarmMapper->currentIndex() > 0);
	ui.numForwardBtn->setEnabled(false);
	connect(ui.numBackwardBtn, &QPushButton::clicked, alarmMapper, &QDataWidgetMapper::toPrevious);
	connect(ui.numForwardBtn, &QPushButton::clicked, alarmMapper, &QDataWidgetMapper::toNext);
	connect(alarmMapper, &QDataWidgetMapper::currentIndexChanged, this, [&](int row) {
		ui.numBackwardBtn->setEnabled(row > 0);
		ui.numForwardBtn->setEnabled(row < playBackWidget->alarmModel->rowCount() - 1);
	});

	//CARE init order: cap, plc, improc
	/* video capture */
	videoCapture[0] = new HikVideoCapture(configHelper, 0, (HWND)ui.cam0HWND->winId(), this);
	videoCapture[1] = new HikVideoCapture(configHelper, 1, (HWND)ui.cam1HWND->winId(), this);
	connect(videoCapture[0], &HikVideoCapture::recordON, this, [&]() {onRecordON(0); });
	connect(videoCapture[1], &HikVideoCapture::recordON, this, [&]() {onRecordON(1); });
	connect(videoCapture[0], &HikVideoCapture::recordOFF, this, [&]() {onRecordOFF(0); });
	connect(videoCapture[1], &HikVideoCapture::recordOFF, this, [&]() {onRecordOFF(1); });

	/* image process */
	//plcserial在初始化列表中
	imageProcess[0] = new ImageProcess(configHelper, videoCapture[0], plcSerial); // don't let parent = this. or can't move to QThread
	imageProcess[1] = new ImageProcess(configHelper, videoCapture[1], plcSerial);
	connect(imageProcess[0], &ImageProcess::showRealtimeImage, this, [&]() {uiShowRealtimeImage(0); });
	connect(imageProcess[1], &ImageProcess::showRealtimeImage, this, [&]() {uiShowRealtimeImage(1); });
	/* image process thread */
	imageProcessThread[0] = new QThread(this);
	imageProcessThread[1] = new QThread(this);
	imageProcess[0]->moveToThread(imageProcessThread[0]);
	imageProcess[1]->moveToThread(imageProcessThread[1]);
	//connect(imageProcessThread[0], &QThread::finished, imageProcess[0], &QObject::deleteLater);
	//connect(imageProcessThread[1], &QThread::finished, imageProcess[1], &QObject::deleteLater);

	/* plc */
	connect(this, &MainWindow::startPLC, plcSerial, &PLCSerial::startTimer);
	connect(this, &MainWindow::stopPLC, plcSerial, &PLCSerial::stopTimer);
	/* plc thread */
	plcSerialThread = new QThread(this);
	plcSerial->moveToThread(plcSerialThread);
	//connect(plcSerialThread, &QThread::finished, plcSerial, &QObject::deleteLater);
	connect(plcSerial, &PLCSerial::trolleySpeedReady, this, &MainWindow::uiShowCartSpeed);	//bind plc::adSpeed to cartSpeed

	/* plc <-> video capture */
	connect(plcSerial, &PLCSerial::sensorIN, videoCapture[0], &HikVideoCapture::startRecord);
	connect(plcSerial, &PLCSerial::sensorIN, videoCapture[1], &HikVideoCapture::startRecord);
	connect(plcSerial, &PLCSerial::sensorOUT, videoCapture[0], &HikVideoCapture::stopRecord);
	connect(plcSerial, &PLCSerial::sensorOUT, videoCapture[1], &HikVideoCapture::stopRecord);

	/* alarmLight */
	connect(this, &MainWindow::alarmUi2PLC, plcSerial, &PLCSerial::Alarm);
	connect(playBackWidget, &PlayBackWidget::setAlarmLight, plcSerial, &PLCSerial::Alarm);
	connect(plcSerial, &PLCSerial::setUiAlarm, this, &MainWindow::uiAlarmLight);

	/* start thread */
	plcSerialThread->start();
	connect(this, &MainWindow::initPlcSerial, plcSerial, &PLCSerial::init);
	emit initPlcSerial();
	imageProcessThread[0]->start();
	imageProcessThread[1]->start();
	imageProcess[0]->emit initModel();
	imageProcess[1]->emit initModel();

	/* database file watcher */
	dbWatcherThread = new QThread(this);
	auto * watcher = new QFileSystemWatcher;
	watcher->addPath(databaseFilePath);
	watcher->moveToThread(dbWatcherThread);
	//connect(dbWatcherThread, &QThread::finished, watcher, &QFileSystemWatcher::deleteLater); // watcher在堆上，由this管理，不要去管它
	connect(watcher, &QFileSystemWatcher::fileChanged, playBackWidget, &PlayBackWidget::dbChanged);
	dbWatcherThread->start();

	setupScheduler(12, 00, true, true); //setup the daily planningTask at 12,00
}

bool MainWindow::cleanDir(QString dirPath, int nDays)
{
	//planA:only clean the file that auto generated by app
	//planB:clean all the file in the directory, include auto-generated and other files, NOT recommended
	bool r = true;

	QDir dir(dirPath);
	//QStringList filters;
	dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot); //prevent deleting the '.' and '..' dirs, very important
	//dir.setSorting(QDir::Name); //from little to big. //deprecated, because the dir we needed can happen in anywhere
	QFileInfoList list = dir.entryInfoList();
	int nDirName = QDate::currentDate().toString("yyyyMMdd").toInt();
	//if (list.length())	//the length can be 0 when the save dir is empty
	for (const auto & i : list) {
		bool ok;
		int nFileDay = i.fileName().toInt(&ok);
		if (ok && (nFileDay <= nDirName - nDays) && nFileDay >= 19700000) {
			QDir selDir = i.absoluteFilePath();
			if (!selDir.removeRecursively())
				r = false;
		}
	}
	if (false == r)
	{
		qDebug() << "MainWindow(cleanDir): error";
	}
	return r;
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
	if (watched == alarmFilter)
	{
		if (event->type() == QEvent::MouseButtonRelease)
		{
			emit alarmFilterClicked();
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return QMainWindow::eventFilter(watched, event);
	}
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	if (!(QMessageBox::information(this, PRODUCT_NAME, QStringLiteral("真的要退出吗？"), QStringLiteral("确定"), QStringLiteral("取消"))))
	{
		configHelper->save();
		event->accept();
	}
	else
	{
		event->ignore();
	}
}

void MainWindow::uiShowRealtimeImage(int deviceIndex)
{
	cv::Mat img2show = imageProcess[deviceIndex]->getFrameToShow();
	QImage image(img2show.data, img2show.cols, img2show.rows, img2show.step, QImage::Format_RGB888);
	QImage dstImage = image.copy(); //deep copy
	dstImage = dstImage.rgbSwapped();
	switch (deviceIndex)
	{
	case 0:
		dstImage = dstImage.scaled(ui.realVideoLabel_0->size(), Qt::KeepAspectRatio); //Note: not like rezise, scaled need assignment
		ui.realVideoLabel_0->setPixmap(QPixmap::fromImage(dstImage));
		break;
	case 1:
		dstImage = dstImage.scaled(ui.realVideoLabel_1->size(), Qt::KeepAspectRatio); //Note: not like rezise, scaled need assignment
		ui.realVideoLabel_1->setPixmap(QPixmap::fromImage(dstImage));
		break;
	default:
		break;
	}
}

void MainWindow::uiAlarmLight(AlarmColor alarmColor) //1-green; 2-red; 4-yellow
{
}

void MainWindow::uiShowCartSpeed()
{
	QString str = QString::number(plcSerial->getTrolleySpeed(), 'f', 1);
	ui.cartSpeedLineEdit->setText(str);
}

void MainWindow::onAlarmChanged(AlarmEvent alarmevent)
{
	QPixmap pixmap;
	switch (alarmevent)
	{
	case 1:
		pixmap = QPixmap(":/WheelMonitor/Resources/images/green.png");
		break;
	case 2:
		pixmap = QPixmap(":/WheelMonitor/Resources/images/red.png");
		break;
	case 4:
		pixmap = QPixmap(":/WheelMonitor/Resources/images/yellow.png");
		break;
	case 8:
		pixmap = QPixmap(":/WheelMonitor/Resources/images/gray.png");
	default:
		break;
	}
	ui.alarmPushButton->setIcon(QIcon(pixmap));
	case enablechanged:
		ui.alarmPushButton->setEnabled(bIsRunning);
}

void MainWindow::execScheduledTasks()
{
	//sync camera time
	videoCapture[0]->syncCameraTime();
	videoCapture[1]->syncCameraTime();
	//clean/make dirs
	QDate date = QDate::currentDate();
	QString today = date.toString("yyyyMMdd");
	QString tomorrow = date.addDays(1).toString("yyyyMMdd");
	QStringList dirPathList;
	dirPathList << logDirPath << videoDirPath << ocrDirPath;
	for (auto&& path : std::as_const(dirPathList)) {
		cleanDir(path, 30);//clean video(, matched), logs //can be operated alone
		QString saveDir = QStringLiteral("%1/%2").arg(path).arg(today);
		QDir dir;
		dir.mkpath(saveDir); //create directory path recursively, if exists, returns true.
		saveDir = QStringLiteral("%1/%2").arg(path).arg(tomorrow); //create the directory path everyday
		dir.mkpath(saveDir);
	}

	//restart every day
	//on_action_Restart_triggered();
}

void MainWindow::setupScheduler(int hh, int mm, bool execnow /*= true*/, bool everyday /*= true*/)
{
	if (execnow)
	{
		execScheduledTasks();
	}
	///start the daily mission at hh:mm today or tomorrow
	QDate date = QDate::currentDate();
	if (QTime::currentTime().msecsTo(QTime(hh, mm, 00, 000)) <= 0)
	{							//if the current time is after 12:00,
		date = date.addDays(1); //2. than set date to tomorrow
	}
	auto ms_to_hhmm = QDateTime::currentDateTime().msecsTo(QDateTime(date, QTime(hh, mm))); //precision of 20ms
	if (everyday)
	{
		QTimer::singleShot(ms_to_hhmm, this, [&]()
		{
			execScheduledTasks();
			auto *timer24 = new QTimer(this);
			connect(timer24, SIGNAL(timeout()), this, SLOT(execScheduledTasks()));
			timer24->start(24 * 60 * 60 * 1000);
		});
	}
	else
	{
		QTimer::singleShot(ms_to_hhmm, this, SLOT(execScheduledTasks()));
	}
}

void MainWindow::on_action_Start_triggered()
{
	if (bIsRunning)
	{
		QMessageBox::warning(this, QStringLiteral("警告"), QStringLiteral("程序运行中，请勿重复运行！如有异常，请重启"), QStringLiteral("确定"));
		return;
	}
	if (!plcSerial->isConnect())
	{
		QMessageBox::critical(this, QStringLiteral("错误"), QStringLiteral("PLC未连接成功！"), QStringLiteral("确定"));
		return;
	}
	emit startPLC();
	if (!videoCapture[0]->startCapture())
	{
		QMessageBox::critical(this, QStringLiteral("错误"), QStringLiteral("外圈摄像头未连接成功！"), QStringLiteral("确定"));
		return;
	}
	if (!videoCapture[1]->startCapture())
	{
		QMessageBox::critical(this, QStringLiteral("错误"), QStringLiteral("内圈摄像头未连接成功！"), QStringLiteral("确定"));
		return;
	}
	imageProcess[0]->startProcess();
	imageProcess[1]->startProcess();
	/* rec label visible */
	recLabel_pre[0]->setVisible(true);
	recLabel_pre[1]->setVisible(true);
	recLabel_input[0]->setVisible(true);
	recLabel_input[1]->setVisible(true);
	/* action */
	ui.action_Start->setEnabled(false);
	ui.action_Stop->setEnabled(true);
	bIsRunning = true;
	onAlarmChanged();

	qDebug("start success");
}

void MainWindow::on_action_Stop_triggered()
{
	if (!bIsRunning)
	{
		QMessageBox::warning(this, QStringLiteral("警告"), QStringLiteral("程序已经停止，请勿重复操作！如有异常，请重启"), QStringLiteral("确定"));
		return;
	}
	imageProcess[0]->stopProcess();
	imageProcess[1]->stopProcess();
	emit stopPLC();
	if (!videoCapture[0]->stopCapture()) return;
	if (!videoCapture[1]->stopCapture()) return;
	/* rec label visible */
	recLabel_pre[0]->setVisible(false);
	recLabel_pre[1]->setVisible(false);
	recLabel_input[0]->setVisible(false);
	recLabel_input[1]->setVisible(false);
	/* action */
	ui.action_Start->setEnabled(true);
	ui.action_Stop->setEnabled(false);
	bIsRunning = false;
	onAlarmChanged();
	emit setAlarm(AlarmOFF);
	qDebug("stop success");
}
void MainWindow::on_action_Restart_triggered()
{
	//qApp->exit(EXIT_CODE_REBOOT);
	qApp->closeAllWindows();
	QProcess::startDetached(QDir::toNativeSeparators(qApp->applicationFilePath()), QStringList("--restart"));
}
void MainWindow::on_action_Property_triggered()
{
	auto sDlg = new SettingDialog(configHelper, this);
	connect(sDlg, &SettingDialog::finished, sDlg, &SettingDialog::deleteLater);
	sDlg->exec();
}
void MainWindow::on_action_Quit_triggered()
{
	this->close();
}
void MainWindow::on_action_Show_Log_triggered()
{
	QString today = QDate::currentDate().toString("yyyyMMdd");
	QString logFilePath = QStringLiteral("%1/%2/%3.log").arg(logDirPath).arg(today).arg(today);
	QUrl url = QUrl::fromLocalFile(logFilePath);
	QDesktopServices::openUrl(url);
}

void MainWindow::on_action_Backup_Log_triggered()
{
	auto *backupLogDialog = new BackupLogDialog(this);

	backupLogDialog->deleteLater();
	backupLogDialog->exec();	//show
}

void MainWindow::on_action_About_triggered()
{
	QMessageBox::about(this, QStringLiteral("关于"),
		QStringLiteral("<h3>%1</h3>"
			"<p>版本号：<b>%2</b>"
			"<p>%3"
			"<p>本软件由浙江大学开发，如果问题请联系%4").arg(PRODUCT_NAME).arg(PRODUCT_VER).arg(COPYRIGHT).arg(AUTHOR));
}

void MainWindow::on_action_About_Qt_triggered()
{
	qApp->aboutQt();
}

void MainWindow::on_alarmPushButton_clicked()
{
	if (currentAlarmEvent != normal)
	{
		ui.centralTabWidget->setCurrentIndex(2);
	}
}

void MainWindow::onRecordON(int deviceIndex)
{
	recLabel_pre[deviceIndex]->setPixmap(QPixmap(":/WheelMonitor/Resources/images/rec_red.png"));
	recLabel_input[deviceIndex]->setPixmap(QPixmap(":/WheelMonitor/Resources/images/rec_red.png"));
}

void MainWindow::onRecordOFF(int deviceIndex)
{
	recLabel_pre[deviceIndex]->setPixmap(QPixmap(":/WheelMonitor/Resources/images/rec_grey.png"));
	recLabel_input[deviceIndex]->setPixmap(QPixmap(":/WheelMonitor/Resources/images/rec_grey.png"));
}
void MainWindow::alarmNumUiSetup()
{
	auto stackedLayout = new QStackedLayout(ui.frame);
	stackedLayout->setContentsMargins(0, 0, 0, 0);
	stackedLayout->setSpacing(0);
	stackedLayout->setStackingMode(QStackedLayout::StackAll);

	auto canvas = new QWidget(ui.frame);
	auto hLayout = new QHBoxLayout(canvas);
	hLayout->setContentsMargins(0, 0, 0, 0);
	hLayout->setSpacing(0);

	alarmIOLineEdit = new QLineEdit(ui.frame);
	alarmIOLineEdit->setObjectName(QStringLiteral("alarmIOLineEdit"));
	alarmIOLineEdit->setStyleSheet("color:red;font:40pt");
	alarmIOLineEdit->setAlignment(Qt::AlignRight);
	alarmIOLineEdit->setFrame(false);
	alarmIOLineEdit->setReadOnly(true);

	alarmNumLineEdit = new QLineEdit(ui.frame);
	alarmNumLineEdit->setObjectName(QStringLiteral("alarmNumLineEdit"));
	QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	alarmNumLineEdit->setSizePolicy(sizePolicy);
	//alarmNumLineEdit->setFixedSize(130, 116);
	alarmNumLineEdit->setStyleSheet("color:red;font:75 68pt \"Agency FB\";");
	alarmNumLineEdit->setFrame(false);
	alarmNumLineEdit->setReadOnly(true);
	hLayout->addWidget(alarmIOLineEdit);
	hLayout->addWidget(alarmNumLineEdit);

	alarmFilter = new QWidget(ui.frame);
	alarmFilter->setCursor(QCursor(Qt::PointingHandCursor));

	stackedLayout->addWidget(canvas);
	stackedLayout->addWidget(alarmFilter);
	alarmFilter->raise();
	alarmFilter->setWindowFlags(Qt::WindowStaysOnTopHint);
	alarmFilter->setStyleSheet("background:transparent");
	ui.verticalLayout->addLayout(stackedLayout);

	alarmFilter->installEventFilter(this);
	connect(this, &MainWindow::alarmFilterClicked, this, [&]() {ui.centralTabWidget->setCurrentIndex(2); });
}