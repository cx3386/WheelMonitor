#include "backuplogdialog.h"
#include "common.h"
#include "playbackwidget.h"
#include "hikvideocapture.h"
#include "imageprocess.h"
#include "mainwindow.h"
#include "outlierhelper.h"
#include "plcserial.h"
#include "settingdialog.h"
#include "stdafx.h"
#include "testvideo.h"
#include "confighelper.h"

bool MainWindow::bAppAutoRun = true;
bool MainWindow::bVerboseLog = true;

MainWindow::MainWindow(const ConfigHelper &helper, QWidget *parent /*= Q_NULLPTR*/)
	: QMainWindow(parent)
	, configHelper(&helper)
{
	//setWindowOpacity(1);
	ui.setupUi(this);
	configWindow();
	const auto deskRect = QApplication::desktop()->availableGeometry();
	if (deskRect.width() < 1920)
	{
		this->setFixedSize(deskRect.size());
	}

	//auto start
	if (configHelper->startAtLaunch) { on_action_Start_triggered(); }
}

MainWindow::~MainWindow()
{
	bool b = true;
	connect(plcSerial, &PLCSerial::setUiAlarm, this, [&b] {b = false; });  //before setUiAlarm finished, DONOT delete the pointer "this". 引发了异常: 读取访问权限冲突。**this** 是 0xFFFFFFFFFFFFFFFF
	emit setAlarm(AlarmOFF); //cannot operate out of thread.so use Siganl-Slot.
	while (b) //wait until light is set off
	{
		QCoreApplication::processEvents();
	}
	videoCaptureThread->quit();
	videoCaptureThread->wait();
	imageProcessThread->quit();
	imageProcessThread->wait();
	plcSerialThread->quit();
	plcSerialThread->wait();
	dbWatcher->quit();
	dbWatcher->wait();
}
void MainWindow::configWindow()
{
	/// Setup the record(save) indication of VideoCapture
	recLabel_pre = new QLabel(ui.previewTab);
	recLabel_pre->setGeometry(20, 20, 50, 35);
	recLabel_pre->setScaledContents(true); //scale its contents to fill all available space.
	recLabel_pre->setVisible(false);
	recLabel_input = new QLabel(ui.realVideoLabel);
	recLabel_input->setGeometry(10, 10, 40, 28);
	recLabel_input->setScaledContents(true);
	recLabel_input->setVisible(false);
	onRecordOFF(); //init as gray
	/// Setup the record(save) indication of VideoCapture

	/// playbackTab
	playBackWidget = new PlayBackWidget(ui.playbackTab);
	auto *playBackLayout = new QGridLayout(ui.playbackTab);
	playBackLayout->addWidget(playBackWidget);
	connect(ui.centralTabWidget, &QTabWidget::currentChanged, playBackWidget, &PlayBackWidget::clearMedia);
	//ui.playbackTab->setEnabled(false);	//unclickable and unchosenable
	//ui.centralTabWidget->setTabEnabled(2, false); //cant toggle to this tab
	/// playbackTab

	/***************update now*****************/
	update24(); //1. update right away

	//start the daily mission timer24 at 12:00
	QDate date = QDate::currentDate();
	if (QTime::currentTime().msecsTo(QTime(12, 00, 00, 000)) <= 0)
	{							//if the current time is after 12:00,
		date = date.addDays(1); //2. than set date to tomorrow
	}
	qint64 msTo12;
	msTo12 = QDateTime::currentDateTime().msecsTo(QDateTime(date, QTime(12, 00))); //precision of 20ms
	QTimer::singleShot(msTo12, this, SLOT(start24timer()));	//once connect will detect if there is the slot or not

	////#TESTONLY 定时emit startsave and stopsave
	////QTimer *timerTMP1 = new QTimer(this);
	////connect(timerTMP1, SIGNAL(timeout()), this, SLOT(startOrStopSave()));
	////timerTMP1->start(10000);
	//testBtn = new QToolButton(ui.monitorTab);
	//testBtn->setCheckable(true);
	//testBtn->setText("SAVE");
	//auto reactToToggle = [&](const bool b) {
	//	if (b)
	//		testBtn->setText("STOP");
	//	else
	//		testBtn->setText("SAVE");
	//	startOrStopSave();
	//};
	//connect(testBtn, &QPushButton::toggled, this, reactToToggle);
	/***************************/

	connect(ui.action_About_Qt, &QAction::triggered, qApp, &QApplication::aboutQt);

	realPlayHandle = (HWND)ui.previewWidget->winId();
	//CARE start order
	imageProcess = new ImageProcess;
	videoCapture = new HikVideoCapture;
	plcSerial = new PLCSerial;
	videoCaptureThread = new QThread(this);
	imageProcessThread = new QThread(this);
	plcSerialThread = new QThread(this);
	imageProcess->moveToThread(imageProcessThread);
	videoCapture->moveToThread(videoCaptureThread);
	plcSerial->moveToThread(plcSerialThread);

	connect(imageProcessThread, &QThread::finished, imageProcess, &QObject::deleteLater);
	connect(videoCaptureThread, &QThread::finished, videoCapture, &QObject::deleteLater);
	connect(plcSerialThread, &QThread::finished, plcSerial, &QObject::deleteLater);

	connect(HikVideoCapture::pVideoCapture, &HikVideoCapture::captureOneImage, imageProcess, &ImageProcess::doImageProcess);
	connect(imageProcess, &ImageProcess::imageProcessReady, videoCapture, &HikVideoCapture::currentImageProcessReady);

	connect(this, &MainWindow::SyncCameraTime, videoCapture, &HikVideoCapture::syncCameraTime); //2017.11.10
	connect(this, &MainWindow::startCap, videoCapture, &HikVideoCapture::startCapture);
	connect(videoCapture, &HikVideoCapture::isStartCap, this, &MainWindow::isStartCap);
	connect(this, &MainWindow::stopCap, videoCapture, &HikVideoCapture::stopCapture);
	connect(videoCapture, &HikVideoCapture::isStopCap, this, &MainWindow::isStopCap);
	connect(videoCapture, &HikVideoCapture::recordTimeout, imageProcess, &ImageProcess::wheelTimeout);
	connect(videoCapture, &HikVideoCapture::recordON, this, &MainWindow::onRecordON); //when stop saving video(sensorOUT or saveTimeOut), update UI.REC.gray
	connect(videoCapture, &HikVideoCapture::recordOFF, this, &MainWindow::onRecordOFF); //when stop saving video(sensorOUT or saveTimeOut), update UI.REC.gray

	//connect(this, &MainWindow::startSave, videoCapture, &HikVideoCapture::startSave);
	//connect(this, &MainWindow::stopSave, videoCapture, &HikVideoCapture::stopSave);

	connect(this, &MainWindow::startProcess, imageProcess, &ImageProcess::startImageProcess);
	connect(this, &MainWindow::stopProcess, imageProcess, &ImageProcess::stopImageProcess);

	connect(this, &MainWindow::initPlcSerial, plcSerial, &PLCSerial::init);
	connect(this, &MainWindow::connectPLC, plcSerial, &PLCSerial::connectPLC);
	connect(plcSerial, &PLCSerial::isConnectPLC, this, &MainWindow::isConnectPLC);
	connect(this, &MainWindow::disconnectPLC, plcSerial, &PLCSerial::disconnectPLC);
	connect(plcSerial, &PLCSerial::isDisconnectPLC, this, &MainWindow::isDisconnectPLC);

	connect(imageProcess, &ImageProcess::showAlarmNum, this, &MainWindow::uiShowAlarmNum);
	connect(imageProcess, &ImageProcess::showRealtimeImage, this, &MainWindow::uiShowRealtimeImage);
	connect(imageProcess, &ImageProcess::showImageMatches, this, &MainWindow::uiShowMatches);
	connect(imageProcess, &ImageProcess::showWheelSpeed, this, &MainWindow::uiShowWheelSpeed);
	connect(imageProcess, &ImageProcess::showWheelNum, this, &MainWindow::uiShowWheelNum);
	connect(plcSerial, &PLCSerial::ADSpeedReady, this, &MainWindow::uiShowCartSpeed);	//bind plc::adSpeed to cartSpeed

	//When wheel enters detect area, i.e., lighten the LEFT sensor
	connect(plcSerial, &PLCSerial::sensorIN, videoCapture, &HikVideoCapture::startRecord); //1. start to save video
	connect(plcSerial, &PLCSerial::sensorIN, imageProcess, &ImageProcess::sensorIN);	 //2. start image process
	//connect(plcSerial, &PLCSerial::sensorIN, this, &MainWindow::onRecStart);			 //3. update UI.REC.red

	//When wheel leaves detect area, i.e., leaves the RIGHT sensor
	connect(plcSerial, &PLCSerial::sensorOUT, videoCapture, static_cast<void (HikVideoCapture::*)()>(&HikVideoCapture::stopRecord)); //1. stop to save video
	connect(plcSerial, &PLCSerial::sensorOUT, imageProcess, &ImageProcess::sensorOUT);   //2. stop image process
	//connect(plcSerial, &PLCSerial::sensorOUT, this, &MainWindow::onRecStop);			    //3. update UI.REC.gray

	///alarmLight
	connect(imageProcess, &ImageProcess::setAlarmLight, plcSerial, &PLCSerial::Alarm);
	connect(this, &MainWindow::setAlarm, plcSerial, &PLCSerial::Alarm);
	connect(playBackWidget, &PlayBackWidget::setAlarmLight, plcSerial, &PLCSerial::Alarm);
	connect(plcSerial, &PLCSerial::setUiAlarm, this, &MainWindow::uiAlarmLight);

	//outputMessageThread.start();
	//emit installLogSystem();
	imageProcessThread->start();
	imageProcess->emit initModel();
	videoCaptureThread->start();
	plcSerialThread->start();
	emit initPlcSerial();

	dbWatcher = new QThread(this);
	auto * watcher = new QFileSystemWatcher;
	watcher->addPath(databaseFilePath);
	watcher->moveToThread(dbWatcher);
	//connect(dbWatcher, &QThread::finished, watcher, &QFileSystemWatcher::deleteLater);   ///< playBackWidget is child of this, so it's handled by this, do not delete it manually.
	connect(watcher, &QFileSystemWatcher::fileChanged, playBackWidget, &PlayBackWidget::dbChanged);
	dbWatcher->start();
	/************************************************************************/
	/* for test                                                             */
	/************************************************************************/
	//emit startProcess();

	//TestVideo *testVideo = new TestVideo;
	//QThread *testVideoThread = new QThread(this);

	//testVideo->moveToThread(testVideoThread);
	//connect(testVideoThread, &QThread::finished, testVideo, &QObject::deleteLater);

	//testVideoThread->start();
	////QTimer::singleShot(0, testVideo, &TestVideo::processReady);
	//QTimer::singleShot(1000, testVideo, &TestVideo::sendVideoFrame);
}

bool MainWindow::cleanDir(QString dirPath, int nDays)
{
	//planA:only clean the file that auto generated by app
	//planB:clean all the file include auto-generated and other files, NOT recommended
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

void MainWindow::closeEvent(QCloseEvent *event)
{
	if (!(QMessageBox::information(this, QStringLiteral("宝钢环冷机台车轮子转速监测"), QStringLiteral("真的要退出吗？"), QStringLiteral("确定"), QStringLiteral("取消"))))
	{
		configHelper->save();
		event->accept();
	}
	else
	{
		event->ignore();
	}
}

void MainWindow::uiShowRealtimeImage()
{
	mutex.lock();
	QImage image(imageProcess->frameToShow.data, imageProcess->frameToShow.cols, imageProcess->frameToShow.rows, imageProcess->frameToShow.step, QImage::Format_RGB888);
	mutex.unlock();
	QImage dstImage = image.copy(); //deep copy
	dstImage = dstImage.rgbSwapped();
	dstImage = dstImage.scaled(ui.realVideoLabel->size(), Qt::KeepAspectRatio); //Note: not like rezise, scaled need assignment
	ui.realVideoLabel->setPixmap(QPixmap::fromImage(dstImage));
}

void MainWindow::uiShowMatches()
{
	//read-only, need mutex, to prevent if the image is changed
	mutex.lock();
	QImage image(imageProcess->imageMatches.data, imageProcess->imageMatches.cols, imageProcess->imageMatches.rows, imageProcess->imageMatches.step, QImage::Format_RGB888);
	mutex.unlock();
	QImage dstImage = image.copy(); //deep copy
	dstImage = dstImage.rgbSwapped();
	dstImage = dstImage.scaled(ui.imageMatchesLabel->size(), Qt::KeepAspectRatio); //Note: not like rezise, scaled need assignment
	ui.imageMatchesLabel->setPixmap(QPixmap::fromImage(dstImage));				 //label->setscaledcontent()
}

void MainWindow::uiAlarmLight(AlarmColor alarmColor) //1-green; 2-red; 4-yellow
{
	QPixmap pixmap;
	switch (alarmColor)
	{
	case 1:
		pixmap = QPixmap(":/images/Resources/images/green.png");
		break;
	case 2:
		pixmap = QPixmap(":/images/Resources/images/red.png");
		break;
	case 4:
		pixmap = QPixmap(":/images/Resources/images/yellow.png");
		break;
	case 8:
		pixmap = QPixmap(":/images/Resources/images/gray.png");
	default:
		break;
	}
	ui.alarmPushButton->setIcon(QIcon(pixmap));
}

void MainWindow::uiShowAlarmNum(const QString &num)
{
	ui.lcdNumber->display(num);
}

void MainWindow::uiShowWheelNum(const QString &s)
{
	ui.numLineEdit->setText(s);
}

void MainWindow::uiShowWheelSpeed(double speed)
{
	if (speed == MISS_TEST_SPEED)
	{
		ui.lastSpeedLineEdit->setText("MISS");
	}
	else
	{
		QString str = QString::number(speed, 'f', 1);
		ui.lastSpeedLineEdit->setText(str);
	}
}

void MainWindow::uiShowCartSpeed(double speed)
{
	QString str = QString::number(speed*0.954, 'f', 1);
	ui.cartSpeedLineEdit->setText(str);
}

void MainWindow::start24timer()
{ //start24timer at 12 o'clock
	auto *timer24 = new QTimer(this);
	connect(timer24, SIGNAL(timeout()), this, SLOT(update24()));
	timer24->start(24 * 60 * 60 * 1000);
	update24();
}

void MainWindow::update24()
{	//sync camera time
	emit SyncCameraTime();

	//clear/make dirs
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

void MainWindow::on_action_Start_triggered()
{
	//防止程序在停止状态下多次调用stop
	if (!bIsRunning)
		//开始顺序： start cap process sensor
		emit startCap(realPlayHandle);
	else
		return;
}

void MainWindow::on_action_Stop_triggered()
{
	//防止程序在停止状态下多次调用stop
	if (bIsRunning)
		//结束顺序: stop sensor cap precess
		emit disconnectPLC();
	else
		return;
}
void MainWindow::on_action_Restart_triggered()
{
	//qApp->exit(EXIT_CODE_REBOOT);
	qApp->closeAllWindows();
	QProcess::startDetached(QDir::toNativeSeparators(qApp->applicationFilePath()), QStringList("--restart"));
}
void MainWindow::on_action_Property_triggered()
{
	auto sDlg = new SettingDialog(this);
	connect(sDlg, &SettingsDialog::finished,
		sDlg, &SettingsDialog::deleteLater);
	if (sDlg->exec()) {
		//TODO
		configHelper->save();
	}
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
			"<p>本软件由浙江大学开发，如果问题请联系%4").arg(ProductName).arg(ProductVer).arg(Copyright).arg(Author));
}
void MainWindow::on_alarmPushButton_clicked()
{
	//when not running, start the monitor.
	if (false == bIsRunning)
	{
		on_action_Start_triggered();
		return;
	}
	if (playBackWidget->hasAlarm())
	{
		ui.centralTabWidget->setCurrentIndex(2);
		QMessageBox::information(this, QStringLiteral("重置警报"), QStringLiteral("有未经排除的故障, 请确认清除后再重置警报"), QStringLiteral("确定"));
		return;
	}
	//start success
	emit setAlarm(AlarmColorGreen);
	//ui.logTabWidget->setCurrentIndex(0); //reset logTabWidget to log tab(not the error tab)
}

bool MainWindow::isStartCap(bool result)
{
	if (result)
	{
		recLabel_pre->setVisible(true);
		recLabel_input->setVisible(true);
		emit startProcess();
		emit connectPLC();
	}
	else
	{
		qWarning("start error, please check the connect to CAMERA");
	}
	return result;
}

bool MainWindow::isStopCap(bool result)
{
	if (result)
	{
		recLabel_pre->setVisible(false);
		recLabel_input->setVisible(false);
		ui.action_Start->setEnabled(true);
		ui.action_Stop->setEnabled(false);
		bIsRunning = false;
		emit setAlarm(AlarmOFF);
		qDebug("stop success");
	}
	else
		qWarning("stop error");
	return result;
}

void MainWindow::onRecordON()
{
	recLabel_pre->setPixmap(QPixmap(":/images/Resources/images/rec_red.png"));
	recLabel_input->setPixmap(QPixmap(":/images/Resources/images/rec_red.png"));
}

void MainWindow::onRecordOFF()
{
	recLabel_pre->setPixmap(QPixmap(":/images/Resources/images/rec_grey.png"));
	recLabel_input->setPixmap(QPixmap(":/images/Resources/images/rec_grey.png"));
}

bool MainWindow::isConnectPLC(bool r)
{
	if (r)
	{
		ui.action_Start->setEnabled(false);
		ui.action_Stop->setEnabled(true);
		bIsRunning = true;
		on_alarmPushButton_clicked();
		qDebug("start success");
	}
	else
	{
		qWarning("start error, please check the connect to PLC");
	}
	return r;
}

bool MainWindow::isDisconnectPLC(bool r)
{
	if (r)
	{
		plcSerial->emit sensorOUT();
		emit stopProcess();
		emit stopCap();
	}
	return r;
}

void MainWindow::startOrStopSave()
{
	static bool bRec = false;
	if (bRec)
		plcSerial->emit sensorOUT();
	else
		plcSerial->emit sensorIN();
	bRec = !bRec;
}