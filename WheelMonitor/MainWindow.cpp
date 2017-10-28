#include "stdafx.h"
#include "MainWindow.h"
#include "SettingDialog.h"

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
	, bRunningState(false)
{
	qRegisterMetaType<HWND>("HWND");
	qRegisterMetaType<PLCSerial::AlarmColor>("PLCSerial::AlarmColor");
	ui.setupUi(this);
	readSettings();
	configWindow();
	//auto start
	//on_action_Start_triggered();
}

MainWindow::~MainWindow()
{
	emit setAlarm(PLCSerial::ALarmOFF);	//不能从线程外操作
	//recLabel->deleteLater();
	videoCaptureThread.quit();
	videoCaptureThread.wait();
	imageProcessThread.quit();
	imageProcessThread.wait();
	plcSerialThread.quit();
	plcSerialThread.wait();
	outputMessageThread.quit();
	outputMessageThread.wait();
}
void MainWindow::configWindow()
{
	/***************setup reclabel****************/
	recLabel = new QLabel(ui.playerTab);
	recLabel->setObjectName(QStringLiteral("recLabel"));
	recLabel->setGeometry(20, 20, 50, 35);
	recLabel->setScaledContents(true);
	recLabel->setVisible(false);
	onRecStop();//init as grey
	//recLabel->raise();
	//recLabel->setAttribute(Qt::WA_TranslucentBackground);

	/***************update now*****************/
	//make dir for capture save
	QString nowDate = QDateTime::currentDateTime().toString("yyyyMMdd");
	QString saveDir = QStringLiteral("D:/Capture/%1").arg(nowDate);
	makeDir(saveDir);

	//定时任务，每天00:00触发
	int time_2_24 = QTime::currentTime().msecsTo(QTime(23, 59, 59, 999)) + 1;
	QTimer::singleShot(time_2_24, this, SLOT(update24()));

	////Only for testing 定时emit startsave and stopsave
	//QTimer *timerTMP1 = new QTimer(this);
	//connect(timerTMP1, SIGNAL(timeout()), this, SLOT(startOrStopSave()));
	//timerTMP1->start(10000);

	connect(ui.action_About_Qt, &QAction::triggered, qApp, &QApplication::aboutQt);
	//ui.imageMatchesLabel->setFixedSize(ui.imageMatchesLabel->size());
	ui.errorTextBrowser->setOpenLinks(false);
	connect(ui.errorTextBrowser, SIGNAL(anchorClicked(const QUrl&)), this, SLOT(anchorClickedSlot(const QUrl&)));

	realPlayHandle = (HWND)ui.playerWidget->winId();
	//care start order

	outputMessage = new MyMessageOutput;

	imageProcess = new ImageProcess;
	videoCapture = new HikVideoCapture;
	plcSerial = new PLCSerial;

	outputMessage->moveToThread(&outputMessageThread);
	imageProcess->moveToThread(&imageProcessThread);
	videoCapture->moveToThread(&videoCaptureThread);
	plcSerial->moveToThread(&plcSerialThread);

	connect(&outputMessageThread, &QThread::finished, outputMessage, &QObject::deleteLater);
	connect(&imageProcessThread, &QThread::finished, imageProcess, &QObject::deleteLater);
	connect(&videoCaptureThread, &QThread::finished, videoCapture, &QObject::deleteLater);
	connect(&plcSerialThread, &QThread::finished, plcSerial, &QObject::deleteLater);

	connect(this, &MainWindow::installLogSystem, outputMessage, &MyMessageOutput::installMesageHandler);
	connect(MyMessageOutput::pMyMessageOutput, &MyMessageOutput::logMessage, this, &MainWindow::uiShowLogMessage);
	connect(MyMessageOutput::pMyMessageOutput, &MyMessageOutput::errorMessage, this, &MainWindow::uiShowErrorMessage);

	connect(HikVideoCapture::pVideoCapture, &HikVideoCapture::imageNeedProcess, imageProcess, &ImageProcess::doImageProcess);
	connect(imageProcess, &ImageProcess::imageProcessReady, videoCapture, &HikVideoCapture::imageProcessReady);

	connect(this, &MainWindow::startCap, videoCapture, &HikVideoCapture::startCap);
	connect(videoCapture, &HikVideoCapture::isStartCap, this, &MainWindow::isStartCap);
	connect(this, &MainWindow::stopCap, videoCapture, &HikVideoCapture::stopCap);
	connect(videoCapture, &HikVideoCapture::isStopCap, this, &MainWindow::isStopCap);
	connect(videoCapture, &HikVideoCapture::wheelTimeout, imageProcess, &ImageProcess::wheelTimeout);

	//connect(this, &MainWindow::startSave, videoCapture, &HikVideoCapture::startSave);
	//connect(this, &MainWindow::stopSave, videoCapture, &HikVideoCapture::stopSave);

	connect(this, &MainWindow::startProcess, imageProcess, &ImageProcess::startImageProcess);
	connect(this, &MainWindow::stopProcess, imageProcess, &ImageProcess::stopImageProcess);

	connect(this, &MainWindow::initPlcSerial, plcSerial, &PLCSerial::init);
	connect(this, &MainWindow::startWheelSensor, plcSerial, &PLCSerial::startWheelSensor);
	connect(plcSerial, &PLCSerial::isStartWheelSensor, this, &MainWindow::isStartWheelSensor);
	connect(this, &MainWindow::stopWheelSensor, plcSerial, &PLCSerial::stopWheelSensor);
	connect(plcSerial, &PLCSerial::isStopWheelSensor, this, &MainWindow::isStopWheelSensor);

	connect(imageProcess, &ImageProcess::showImageMatches, this, &MainWindow::uiShowMatches);
	connect(imageProcess, &ImageProcess::speedClcReady, this, &MainWindow::uiShowLastSpeed);
	connect(imageProcess, &ImageProcess::realSpeedReady, this, &MainWindow::uiShowRealSpeed);
	connect(imageProcess, &ImageProcess::setAlarmLight, plcSerial, &PLCSerial::Alarm);

	connect(plcSerial, &PLCSerial::startSave, videoCapture, &HikVideoCapture::startSave);
	connect(plcSerial, &PLCSerial::startSave, imageProcess, &ImageProcess::sensorIN);	//认为同一个轮子
	connect(plcSerial, &PLCSerial::startSave, this, &MainWindow::onRecStart);	//认为同一个轮子

	connect(plcSerial, &PLCSerial::stopSave, videoCapture, &HikVideoCapture::stopSave);
	connect(plcSerial, &PLCSerial::stopSave, imageProcess, &ImageProcess::sensorOUT);		//认为结束了该个轮子
	connect(plcSerial, &PLCSerial::stopSave, this, &MainWindow::onRecStop);	//认为同一个轮子

	connect(plcSerial, &PLCSerial::setUiAlarm, this, &MainWindow::uiAlarmLight);
	connect(this, &MainWindow::setAlarm, plcSerial, &PLCSerial::Alarm);

	outputMessageThread.start();
	emit installLogSystem();
	imageProcessThread.start();
	videoCaptureThread.start();
	plcSerialThread.start();
	emit initPlcSerial();
}

void MainWindow::uiAlarmLight(PLCSerial::AlarmColor alarmColor) //1-green; 2-red; 4-yellow
{
	QPixmap pixmap;
	switch (alarmColor) {
	case 1:
		pixmap = QPixmap(":/images/Resources/images/green.png");
		break;
	case 2:
		pixmap = QPixmap(":/images/Resources/images/red.png");
		break;
	case 4:
		pixmap = QPixmap(":/images/Resources/images/yellow.png");
		break;
	default:
		break;
	}
	//pixmap = pixmap.scaled(256, 256);
	ui.alarmPushButton->setIcon(QIcon(pixmap));
}

void MainWindow::readSettings()
{
	QSettings settings(QCoreApplication::applicationDirPath().append("/config.ini"), QSettings::IniFormat);
	ImageProcess::sensorTriggered = settings.value("ImageProcess/sensorTriggered", false).toBool();
	ImageProcess::angleBigRatio = settings.value("ImageProcess/angleBigRatio", 1.2).toDouble();
	ImageProcess::angleSmallRatio = settings.value("ImageProcess/angleSmallRatio", 0.8).toDouble();
	ImageProcess::radius_max = settings.value("ImageProcess/radius_max", 350).toInt();
	ImageProcess::radius_min = settings.value("ImageProcess/radius_min", 250).toInt();
	ImageProcess::roiRect = cv::Rect(settings.value("ImageProcess/roiRect_x", 220).toInt(),
		settings.value("ImageProcess/roiRect_y", 0).toInt(),
		settings.value("ImageProcess/roiRect_w", 800).toInt(),
		settings.value("ImageProcess/roiRect_h", 720).toInt());

	HikVideoCapture::capInterval = settings.value("VideoCapture/capInterval", 7).toInt();
	ImageProcess::angle2Speed = 60 * (M_PI * 0.650 / 360) / ((HikVideoCapture::capInterval + 1) / 25.0);
}

void MainWindow::writeSettings()
{
	QSettings settings(QCoreApplication::applicationDirPath().append("/config.ini"), QSettings::IniFormat);
	settings.beginGroup("ImageProcess");
	settings.setValue("sensorTriggered", ImageProcess::sensorTriggered);
	settings.setValue("angleBigRatio", ImageProcess::angleBigRatio);
	settings.setValue("angleSmallRatio", ImageProcess::angleSmallRatio);
	settings.setValue("radius_max", ImageProcess::radius_max);
	settings.setValue("radius_min", ImageProcess::radius_min);
	settings.setValue("roiRect_x", ImageProcess::roiRect.x);
	settings.setValue("roiRect_y", ImageProcess::roiRect.y);
	settings.setValue("roiRect_w", ImageProcess::roiRect.width);
	settings.setValue("roiRect_h", ImageProcess::roiRect.height);
	settings.endGroup();

	settings.beginGroup("VideoCapture");
	settings.setValue("capInterval", HikVideoCapture::capInterval);
	settings.endGroup();
}

void MainWindow::clearLog(int nDays)
{
	QDir dir("D:/");
	QStringList filters;
	dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
	filters << "*.log";//设置过滤类型
	dir.setNameFilters(filters);//设置文件名的过滤
	dir.setSorting(QDir::Name);
	QFileInfoList list = dir.entryInfoList();
	int nlogName = QDate::currentDate().toString("yyyyMMdd").toInt();
	if (list.length() != 0)
		for (int i = 0; i < list.size(); ++i)
		{
			if (list.at(i).baseName().toInt() <= (nlogName - nDays))
				//qDebug() << list.at(i).absoluteFilePath();
				QFile::remove(list.at(i).absoluteFilePath());
			else
				//continue;
				break;
		}
	else
	{
		qDebug() << "no file";
	}
}

bool MainWindow::delCapDir(int nDays)
{
	bool r = true;
	QDir dir("D:/Capture");	//必须保证该文件夹里面没有其他文件，否则会误删
	QStringList filters;
	dir.setFilter(QDir::Dirs);
	dir.setSorting(QDir::Name);	//from little to big
	QFileInfoList list = dir.entryInfoList();
	int nDirName = QDate::currentDate().toString("yyyyMMdd").toInt();
	if (list.length() != 0)
		for (int i = 0; i < list.size(); ++i) {
			//prevent to del the '.' and '..' dirs, very important
			if (list.at(i).fileName() == "." || list.at(i).fileName() == "..")
				continue;
			if (list.at(i).fileName().toInt() <= (nDirName - nDays)) {
				dir = list.at(i).absoluteFilePath();
				if (!dir.removeRecursively())
					r = false;
			}
			else
				break;
		}
	else {
		qDebug() << "no file";
		return false;
	}
	return r;
}

bool MainWindow::makeDir(QString fullPath)
{
	QDir dir(fullPath);
	if (dir.exists())
	{
		return true;
	}
	else
	{
		bool ok = dir.mkpath(fullPath);//创建多级目录
		return ok;
	}
}

void MainWindow::closeEvent(QCloseEvent * event)
{
	if (!(QMessageBox::information(this, QStringLiteral("宝钢环冷机台车轮子状态检测"), QStringLiteral("真的要退出吗？"), QStringLiteral("确定"), QStringLiteral("取消"))))
	{
		writeSettings();
		event->accept();
	}
	else
	{
		event->ignore();
	}
}

void MainWindow::uiShowMatches()
{
	mutex.lock();
	QImage image(imageProcess->imageMatches.data, imageProcess->imageMatches.cols, imageProcess->imageMatches.rows, imageProcess->imageMatches.step, QImage::Format_RGB888);
	mutex.unlock();
	image.rgbSwapped();
	//image.setColorTable(sColorTable);
	image = image.scaled(ui.imageMatchesLabel->size(), Qt::KeepAspectRatio); //需要赋值
	ui.imageMatchesLabel->setPixmap(QPixmap::fromImage(image));		//label->setscaledcontent()
}

void MainWindow::uiShowLastSpeed(double speed)
{
	QString str = QString::number(speed, 'f', 2);
	ui.lastSpeedLineEdit->setText(str);
}

void MainWindow::uiShowRealSpeed(double speed)
{
	QString str = QString::number(speed, 'f', 2);
	ui.realSpeedLineEdit->setText(str);
}

void MainWindow::uiShowLogMessage(const QString & message)
{
	ui.logTextBrowser->append(message);
}

void MainWindow::uiShowErrorMessage(const QString & message)
{
	QString errormsg = QStringLiteral("%1 <a href = \"%2\">Video</a>").arg(message).arg(videoCapture->saveDirLink);
	ui.errorTextBrowser->append(errormsg);
}

void MainWindow::anchorClickedSlot(const QUrl& url)
{
	QDesktopServices::openUrl(QUrl(url.toString(), QUrl::TolerantMode));
}

void MainWindow::update24()
{
	static bool bIsTimer24First = true;
	if (bIsTimer24First)
	{
		QTimer *timer24 = new QTimer(this);
		connect(timer24, SIGNAL(timeout()), this, SLOT(update24()));
		timer24->start(24 * 3600 * 1000);
		bIsTimer24First = false;
		update24();//当天也要触发
	}
	else
	{
		//delete log
		clearLog(30);
		//clear capVideo
		delCapDir(30);

		//mkpath for capVideo
		QString nowDate = QDateTime::currentDateTime().toString("yyyyMMdd");
		QString saveDir = QStringLiteral("D:/Capture/%1").arg(nowDate);
		makeDir(saveDir);

		//restart
		//on_action_Stop_triggered();
		////等待一段时间
		//on_action_Start_triggered();
	}
}

void MainWindow::on_action_Start_triggered()
{
	//防止程序在停止状态下多次调用stop
	if (!bRunningState)
		//开始顺序： start cap process sensor
		emit startCap(realPlayHandle);
	else
		return;
}

void MainWindow::on_action_Stop_triggered()
{
	//防止程序在停止状态下多次调用stop
	if (bRunningState)
		//结束顺序: stop sensor cap precess
		emit stopWheelSensor();
	else
		return;
}
void MainWindow::on_action_Property_triggered()
{
	SettingDialog *settingDialog = new SettingDialog(this); // 将this作为父窗
	//settingDialog->setAttribute(Qt::WA_DeleteOnClose);
	settingDialog->deleteLater();
	if (settingDialog->exec() == QDialog::Accepted)
	{
		settingDialog->setOption();
	}
}
void MainWindow::on_action_Quit_triggered()
{
	this->close();
}
void MainWindow::on_alarmPushButton_clicked()
{
	emit setAlarm(PLCSerial::AlarmColorGreen);
}

void MainWindow::on_errorTextBrowser_textChanged()
{
	ui.errorTextBrowser->moveCursor(QTextCursor::End);
}

void MainWindow::on_logTextBrowser_textChanged()
{
	ui.logTextBrowser->moveCursor(QTextCursor::End);
}

bool MainWindow::isStartCap(bool result)
{
	if (result)
	{
		recLabel->setVisible(true);
		emit startProcess();
		emit startWheelSensor();
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
		emit setAlarm(PLCSerial::ALarmOFF);
		ui.action_Start->setEnabled(true);
		ui.action_Stop->setEnabled(false);
		bRunningState = false;
		qWarning("stop success");
	}
	else
		qWarning("stop error");
	return result;
}

void MainWindow::onRecStart()
{
	//QImage srcImg("D:/WheelMonitor/WheelMonitor/Resources/images/rec_red.png");
	//QImage dstImg(500, 500, QImage::Format_ARGB32);
	//QPainter p(&dstImg);
	//p.fillRect(dstImg.rect(), Qt::blue);
	////p.setBackgroundMode(Qt::TransparentMode);
	//p.setCompositionMode(QPainter::CompositionMode_Source);//注意这一行代码  
	//p.drawImage(0, 0, srcImg);
	recLabel->setPixmap(QPixmap(":/images/Resources/images/rec_red.png"));
	//recLabel->show();
}

void MainWindow::onRecStop()
{
	//QPainter p(recLabel);
	//p.drawPixmap(0, 0, QPixmap(":/images/Resources/images/rec_grey.png"));
	recLabel->setPixmap(QPixmap(":/images/Resources/images/rec_grey.png"));
	//recLabel->show();
}

bool MainWindow::isStartWheelSensor(bool r)
{
	if (r)
	{
		emit setAlarm(PLCSerial::AlarmColorGreen);
		ui.action_Start->setEnabled(false);
		ui.action_Stop->setEnabled(true);
		bRunningState = true;
		qWarning("start success");
	}
	else
	{
		qWarning("start error, please check the connect to PLC");
	}
	return r;
}

bool MainWindow::isStopWheelSensor(bool r)
{
	if (r)
	{
		plcSerial->emit stopSave();
		recLabel->setVisible(false);
		emit stopProcess();
		emit stopCap();
	}
	return r;
}

//void MainWindow::startOrStopSave()
//{
//	static bool bRec = false;
//	if (bRec)
//		plcSerial->emit stopSave();
//	else
//		plcSerial->emit startSave();
//	bRec = !bRec;
//}

void MainWindow::on_action_About_triggered()
{
	QMessageBox::about(this, QStringLiteral("关于"),
		QStringLiteral("<h3>宝钢环冷机台车轮子状态检测软件</h3>"
			"<p>版本号：<b>v1.1.0</b>"
			"<p>Copyright &copy; 2017 ZJU SKL."
			"<p>本软件由浙江大学开发，如果问题请联系cx3386@163.com"));
}