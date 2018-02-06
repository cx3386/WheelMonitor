#include "stdafx.h"
#include "wheelmonitor.h"
#include "common.h"
#include "backuplogdialog.h"
#include "playbackwidget.h"
#include "hikvideocapture.h"
#include "imageprocess.h"
#include "outlierhelper.h"
#include "plcserial.h"
#include "settingdialog.h"
#include "mysqltablemodel.h"
#include "ocr.h"
#include "database.h"

bool WheelMonitor::bAppAutoRun = true;
bool WheelMonitor::bVerboseLog = true;

WheelMonitor::WheelMonitor(QWidget *parent)
	: QMainWindow(parent), bIsRunning(false)
{
	//setWindowOpacity(1);

	qRegisterMetaType<HWND>("HWND");
	qRegisterMetaType<QVector<int>>("QVector<int>");
	qRegisterMetaType<AlarmColor>("AlarmColor");

	ui.setupUi(this);
	readSettings();
	configWindow();
	const auto deskRect = QApplication::desktop()->availableGeometry();
	if (deskRect.width() < 1920)
	{
		setWindowFlag(Qt::MSWindowsFixedSizeDialogHint);
		resize(deskRect.size());
	}
	//auto start
	//on_action_Start_triggered();
}

WheelMonitor::~WheelMonitor()
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
void WheelMonitor::configWindow()
{
	// Setup the record(save) indication of VideoCapture
	recLabel_pre = new QLabel(ui.previewTab);
	recLabel_pre->setGeometry(20, 20, 50, 35);
	recLabel_pre->setScaledContents(true); //scale its contents to fill all available space.
	recLabel_pre->setVisible(false);
	recLabel_input = new QLabel(ui.realVideoLabel);
	recLabel_input->setGeometry(10, 10, 40, 28);
	recLabel_input->setScaledContents(true);
	recLabel_input->setVisible(false);
	onRecStop(); //init as gray

	// playbackTab
	playBackWidget = new PlayBackWidget(ui.playbackTab);
	auto *playBackLayout = new QGridLayout(ui.playbackTab);
	playBackLayout->addWidget(playBackWidget);
	connect(ui.centralTabWidget, &QTabWidget::currentChanged, playBackWidget, &PlayBackWidget::clearMedia);

	// monitorTab
	QDataWidgetMapper *allMapper = new QDataWidgetMapper(this);
	allMapper->setModel(static_cast<QSqlTableModel*>(playBackWidget->allModel));
	allMapper->addMapping(ui.numLineEdit, Wheel_Num);
	allMapper->addMapping(ui.lastSpeedLineEdit, Wheel_CalcSpeed);
	allMapper->toLast();
	QDataWidgetMapper *alarmMapper = new QDataWidgetMapper(this);
	alarmMapper->setModel(static_cast<QSqlTableModel*>(playBackWidget->alarmModel));
	alarmMapper->addMapping(ui.lcdNumber, Wheel_Num);
	allMapper->toLast();

	connect(ui.action_About_Qt, &QAction::triggered, qApp, &QApplication::aboutQt);

	setupPlanningTasks(12, 00, true, true); //setup the daily planningTask at 12,00

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

	realPlayHandle = (HWND)ui.previewWidget->winId();
	//care start order
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

	connect(HikVideoCapture::pVideoCapture, &HikVideoCapture::imageNeedProcess, imageProcess, &ImageProcess::doImageProcess);
	connect(imageProcess, &ImageProcess::imageProcessReady, videoCapture, &HikVideoCapture::imageProcessReady);

	connect(this, &WheelMonitor::SyncCameraTime, videoCapture, &HikVideoCapture::syncCameraTime); //2017.11.10
	connect(this, &WheelMonitor::startCap, videoCapture, &HikVideoCapture::startCap);
	connect(videoCapture, &HikVideoCapture::isStartCap, this, &WheelMonitor::isStartCap);
	connect(this, &WheelMonitor::stopCap, videoCapture, &HikVideoCapture::stopCap);
	connect(videoCapture, &HikVideoCapture::isStopCap, this, &WheelMonitor::isStopCap);
	connect(videoCapture, &HikVideoCapture::wheelTimeout, imageProcess, &ImageProcess::wheelTimeout);
	connect(videoCapture, &HikVideoCapture::saveStopped, this, &WheelMonitor::onRecStop); //when stop saving video(sensorOUT or saveTimeOut), update UI.REC.gray

	//connect(this, &MainWindow::startSave, videoCapture, &HikVideoCapture::startSave);
	//connect(this, &MainWindow::stopSave, videoCapture, &HikVideoCapture::stopSave);

	connect(this, &WheelMonitor::startProcess, imageProcess, &ImageProcess::startImageProcess);
	connect(this, &WheelMonitor::stopProcess, imageProcess, &ImageProcess::stopImageProcess);

	connect(this, &WheelMonitor::initPlcSerial, plcSerial, &PLCSerial::init);
	connect(this, &WheelMonitor::connectPLC, plcSerial, &PLCSerial::connectPLC);
	connect(plcSerial, &PLCSerial::isConnectPLC, this, &WheelMonitor::isConnectPLC);
	connect(this, &WheelMonitor::disconnectPLC, plcSerial, &PLCSerial::disconnectPLC);
	connect(plcSerial, &PLCSerial::isDisconnectPLC, this, &WheelMonitor::isDisconnectPLC);

	connect(imageProcess, &ImageProcess::showAlarmNum, this, &WheelMonitor::uiShowAlarmNum);
	connect(imageProcess, &ImageProcess::showRealtimeImage, this, &WheelMonitor::uiShowRealtimeImage);
	connect(imageProcess, &ImageProcess::showImageMatches, this, &WheelMonitor::uiShowMatches);
	connect(imageProcess, &ImageProcess::showWheelSpeed, this, &WheelMonitor::uiShowWheelSpeed);
	connect(imageProcess, &ImageProcess::showWheelNum, this, &WheelMonitor::uiShowWheelNum);
	connect(plcSerial, &PLCSerial::ADSpeedReady, this, &WheelMonitor::uiShowCartSpeed);	//bind plc::adSpeed to cartSpeed

	//When wheel enters detect area, i.e., lighten the LEFT sensor
	connect(plcSerial, &PLCSerial::sensorIN, videoCapture, &HikVideoCapture::startSave); //1. start to save video
	connect(plcSerial, &PLCSerial::sensorIN, imageProcess, &ImageProcess::sensorIN);	 //2. start image process
	connect(plcSerial, &PLCSerial::sensorIN, this, &WheelMonitor::onRecStart);			 //3. update UI.REC.red

	//When wheel leaves detect area, i.e., leaves the RIGHT sensor
	connect(plcSerial, &PLCSerial::sensorOUT, videoCapture, &HikVideoCapture::stopSave); //1. stop to save video
	connect(plcSerial, &PLCSerial::sensorOUT, imageProcess, &ImageProcess::sensorOUT);   //2. stop image process
	//connect(plcSerial, &PLCSerial::sensorOUT, this, &MainWindow::onRecStop);			    //3. update UI.REC.gray

	///alarmLight
	connect(imageProcess, &ImageProcess::setAlarmLight, plcSerial, &PLCSerial::Alarm);
	connect(this, &WheelMonitor::setAlarm, plcSerial, &PLCSerial::Alarm);
	connect(playBackWidget, &PlayBackWidget::setAlarmLight, plcSerial, &PLCSerial::Alarm);
	connect(plcSerial, &PLCSerial::setUiAlarm, this, &WheelMonitor::uiAlarmLight);

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

void WheelMonitor::readSettings()
{
	QSettings settings(QString("%1/config.ini").arg(configDirPath), QSettings::IniFormat);
	bAppAutoRun = settings.value("Common/appAutoRun", true).toBool();
	appAutoRun(bAppAutoRun);
	bVerboseLog = settings.value("Common/verboseLog", true).toBool();
	/*implement of verboselog*/
	ImageProcess::g_imgParam.sensorTriggered = settings.value("ImageProcess/sensorTriggered", false).toBool();
	ImageProcess::g_imgParam.warningRatio = settings.value("ImageProcess/warningRatio", 0.05).toDouble();
	ImageProcess::g_imgParam.alarmRatio = settings.value("ImageProcess/alarmRatio", 0.10).toDouble();
	ImageProcess::g_imgParam.radius_max = settings.value("ImageProcess/radius_max", 350).toInt();
	ImageProcess::g_imgParam.radius_min = settings.value("ImageProcess/radius_min", 250).toInt();
	ImageProcess::g_imgParam.roiRect = cv::Rect(settings.value("ImageProcess/roiRect_x", 220).toInt(),
		settings.value("ImageProcess/roiRect_y", 0).toInt(),
		settings.value("ImageProcess/roiRect_w", 800).toInt(),
		settings.value("ImageProcess/roiRect_h", 720).toInt());
	OCR::p.plate_x_min = settings.value("ocr_parameters/plate_x_min", 100).toInt();
	OCR::p.plate_x_max = settings.value("ocr_parameters/plate_x_max", 250).toInt();
	OCR::p.plate_y_min = settings.value("ocr_parameters/plate_y_min", 190).toInt();
	OCR::p.plate_y_max = settings.value("ocr_parameters/plate_y_max", 250).toInt();
	OCR::p.plate_width_min = settings.value("ocr_parameters/plate_width_min", 160).toInt();
	OCR::p.plate_width_max = settings.value("ocr_parameters/plate_width_max", 190).toInt();
	OCR::p.plate_height_min = settings.value("ocr_parameters/plate_height_min", 110).toInt();
	OCR::p.plate_height_max = settings.value("ocr_parameters/plate_height_max", 130).toInt();
	OCR::p.num_width_min = settings.value("ocr_parameters/num_width_min", 20).toInt();
	OCR::p.num_width_max = settings.value("ocr_parameters/num_width_max", 50).toInt();
	OCR::p.num_height_min = settings.value("ocr_parameters/num_height_min", 50).toInt();
	OCR::p.num_height_max = settings.value("ocr_parameters/num_height_max", 75).toInt();

	HikVideoCapture::capInterval = settings.value("VideoCapture/capInterval", 7).toInt();
	ImageProcess::g_imgParam.angle2Speed = 60 * (M_PI * 0.650 / 360) / ((HikVideoCapture::capInterval + 1) / 25.0);
}

void WheelMonitor::writeSettings()
{
	QSettings settings(QString("%1/config.ini").arg(appDirPath), QSettings::IniFormat);
	settings.beginGroup("Common");
	settings.setValue("appAutoRun", bAppAutoRun);
	settings.setValue("verboseLog", bVerboseLog);
	settings.endGroup();

	settings.beginGroup("ImageProcess");
	settings.setValue("sensorTriggered", ImageProcess::g_imgParam.sensorTriggered);
	settings.setValue("warningRatio", ImageProcess::g_imgParam.warningRatio);
	settings.setValue("alarmRatio", ImageProcess::g_imgParam.alarmRatio);
	settings.setValue("radius_max", ImageProcess::g_imgParam.radius_max);
	settings.setValue("radius_min", ImageProcess::g_imgParam.radius_min);
	settings.setValue("roiRect_x", ImageProcess::g_imgParam.roiRect.x);
	settings.setValue("roiRect_y", ImageProcess::g_imgParam.roiRect.y);
	settings.setValue("roiRect_w", ImageProcess::g_imgParam.roiRect.width);
	settings.setValue("roiRect_h", ImageProcess::g_imgParam.roiRect.height);
	settings.endGroup();

	settings.beginGroup("ocr_parameters");
	settings.setValue("plate_x_min", OCR::p.plate_x_min);
	settings.setValue("plate_x_max", OCR::p.plate_x_max);
	settings.setValue("plate_y_min", OCR::p.plate_y_min);
	settings.setValue("plate_y_max", OCR::p.plate_y_max);
	settings.setValue("plate_width_min", OCR::p.plate_width_min);
	settings.setValue("plate_width_max", OCR::p.plate_width_max);
	settings.setValue("plate_height_min", OCR::p.plate_height_min);
	settings.setValue("plate_height_max", OCR::p.plate_height_max);
	settings.setValue("num_width_min", OCR::p.num_width_min);
	settings.setValue("num_width_max", OCR::p.num_width_max);
	settings.setValue("num_height_min", OCR::p.num_height_min);
	settings.setValue("num_height_max", OCR::p.num_height_max);
	settings.endGroup();

	settings.beginGroup("VideoCapture");
	settings.setValue("capInterval", HikVideoCapture::capInterval);
	settings.endGroup();
}

bool WheelMonitor::cleanDir(QString dirPath, int nDays)
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

void WheelMonitor::appAutoRun(bool bAutoRun) ///< autorun when computer start
{
	//HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Run
	QSettings *reg = new QSettings(R"(HKEY_CURRENT_USER\SOFTWARE\Microsoft\Windows\CurrentVersion\Run)", QSettings::NativeFormat);

	if (bAutoRun)
	{
		QString strAppPath = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
		//strAppPath.replace(QChar('/'), QChar('\\'), Qt::CaseInsensitive);
		reg->setValue(appName, QString("%1 --auto").arg(strAppPath));
	}
	else
	{
		//reg->setValue("WheelMonitor", "");
		reg->remove(appName);
	}
	reg->deleteLater();
}

void WheelMonitor::closeEvent(QCloseEvent *event)
{
	if (!(QMessageBox::information(this, QStringLiteral("宝钢环冷机台车轮子转速监测"), QStringLiteral("真的要退出吗？"), QStringLiteral("确定"), QStringLiteral("取消"))))
	{
		writeSettings();
		event->accept();
	}
	else
	{
		event->ignore();
	}
}

void WheelMonitor::uiShowRealtimeImage()
{
	mutex.lock();
	QImage image(imageProcess->frameToShow.data, imageProcess->frameToShow.cols, imageProcess->frameToShow.rows, imageProcess->frameToShow.step, QImage::Format_RGB888);
	mutex.unlock();
	QImage dstImage = image.copy(); //deep copy
	dstImage = dstImage.rgbSwapped();
	dstImage = dstImage.scaled(ui.realVideoLabel->size(), Qt::KeepAspectRatio); //Note: not like rezise, scaled need assignment
	ui.realVideoLabel->setPixmap(QPixmap::fromImage(dstImage));
}

void WheelMonitor::uiShowMatches()
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

void WheelMonitor::uiAlarmLight(AlarmColor alarmColor) //1-green; 2-red; 4-yellow
{
	QPixmap pixmap;
	switch (alarmColor)
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
}

void WheelMonitor::uiShowAlarmNum(const QString &num)
{
	ui.lcdNumber->display(num);
}

void WheelMonitor::uiShowWheelNum(const QString &s)
{
	ui.numLineEdit->setText(s);
}

void WheelMonitor::uiShowWheelSpeed(double speed)
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

void WheelMonitor::uiShowCartSpeed(double speed)
{
	QString str = QString::number(speed*0.954, 'f', 1);
	ui.cartSpeedLineEdit->setText(str);
}

void WheelMonitor::execPlanningTasks()
{
	//sync camera time
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

void WheelMonitor::setupPlanningTasks(int hh, int mm, bool execnow /*= true*/, bool everyday /*= true*/)
{
	if (execnow)
	{
		execPlanningTasks();
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
			execPlanningTasks();
			auto *timer24 = new QTimer(this);
			connect(timer24, SIGNAL(timeout()), this, SLOT(execPlanningTasks()));
			timer24->start(24 * 60 * 60 * 1000);
		});
	}
	else
	{
		QTimer::singleShot(ms_to_hhmm, this, SLOT(execPlanningTasks()));
	}
}

void WheelMonitor::on_action_Start_triggered()
{
	//防止程序在停止状态下多次调用stop
	if (!bIsRunning)
		//开始顺序： start cap process sensor
		emit startCap(realPlayHandle);
	else
		return;
}

void WheelMonitor::on_action_Stop_triggered()
{
	//防止程序在停止状态下多次调用stop
	if (bIsRunning)
		//结束顺序: stop sensor cap precess
		emit disconnectPLC();
	else
		return;
}
void WheelMonitor::on_action_Restart_triggered()
{
	//qApp->exit(EXIT_CODE_REBOOT);
	qApp->closeAllWindows();
	QProcess::startDetached(QDir::toNativeSeparators(qApp->applicationFilePath()), QStringList("--restart"));
}
void WheelMonitor::on_action_Property_triggered()
{
	settingDialog = new SettingDialog(this);
	settingDialog->deleteLater(); //will delete the connect when return from this funciton
	settingDialog->exec();
}
void WheelMonitor::on_action_Quit_triggered()
{
	this->close();
}
void WheelMonitor::on_action_Show_Log_triggered()
{
	QString today = QDate::currentDate().toString("yyyyMMdd");
	QString logFilePath = QStringLiteral("%1/%2/%3.log").arg(logDirPath).arg(today).arg(today);
	QUrl url = QUrl::fromLocalFile(logFilePath);
	QDesktopServices::openUrl(url);
}

void WheelMonitor::on_action_Backup_Log_triggered()
{
	auto *backupLogDialog = new BackupLogDialog(this);

	backupLogDialog->deleteLater();
	backupLogDialog->exec();	//show
}

void WheelMonitor::on_action_About_triggered()
{
	QMessageBox::about(this, QStringLiteral("关于"),
		QStringLiteral("<h3>%1</h3>"
			"<p>版本号：<b>%2</b>"
			"<p>%3"
			"<p>本软件由浙江大学开发，如果问题请联系%4").arg(PRODUCT_NAME).arg(PRODUCT_VER).arg(COPYRIGHT).arg(AUTHOR));
}
void WheelMonitor::on_alarmPushButton_clicked()
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

bool WheelMonitor::isStartCap(bool result)
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

bool WheelMonitor::isStopCap(bool result)
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

void WheelMonitor::onRecStart()
{
	recLabel_pre->setPixmap(QPixmap(":/WheelMonitor/Resources/images/rec_red.png"));
	recLabel_input->setPixmap(QPixmap(":/WheelMonitor/Resources/images/rec_red.png"));
}

void WheelMonitor::onRecStop()
{
	recLabel_pre->setPixmap(QPixmap(":/WheelMonitor/Resources/images/rec_grey.png"));
	recLabel_input->setPixmap(QPixmap(":/WheelMonitor/Resources/images/rec_grey.png"));
}

bool WheelMonitor::isConnectPLC(bool r)
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

bool WheelMonitor::isDisconnectPLC(bool r)
{
	if (r)
	{
		plcSerial->emit sensorOUT();
		emit stopProcess();
		emit stopCap();
	}
	return r;
}

void WheelMonitor::startOrStopSave()
{
	static bool bRec = false;
	if (bRec)
		plcSerial->emit sensorOUT();
	else
		plcSerial->emit sensorIN();
	bRec = !bRec;
}