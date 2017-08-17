#include "stdafx.h"
#include "MainWindow.h"
#include "SettingDialog.h"

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
	, bIsTimer24First(true)
{
	qRegisterMetaType<HWND>("HWND");
	qRegisterMetaType<PLCSerial::AlarmColor>("AlarmColor");
		//startSaveBtn = new QPushButton;
	//connect(startSaveBtn, SIGNAL(clicked()), this, SLOT(on_startSaveBtn_clicked()));
	//startSaveBtn->setText(QStringLiteral("开始保存"));
	//stopSaveBtn = new QPushButton;
	//connect(stopSaveBtn, SIGNAL(clicked()), this, SLOT(on_stopSaveBtn_clicked()));
	//stopSaveBtn->setText(QStringLiteral("停止保存"));
	//ui.horizontalLayout->addWidget(startSaveBtn);
	//ui.horizontalLayout->addWidget(stopSaveBtn);
	//action or menu

	ui.setupUi(this);
	configWindow();
	readSettings();




	realPlayHandle = (HWND)ui.playerFrame->winId();
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

MainWindow::~MainWindow()
{
	emit setAlarm(ALARM_LIGHT_OFF);	//不能从线程外操作
	recBtn->deleteLater();
	videoCaptureThread.quit();
	videoCaptureThread.wait();
	imageProcessThread.quit();
	imageProcessThread.wait();
	plcSerialThread.quit();
	plcSerialThread.wait();
	outputMessageThread.quit();
	outputMessageThread.wait();
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
	ImageProcess::angleHighThreshold = settings.value("ImageProcess/angleHighThreshold", 3.0).toDouble();
	ImageProcess::angleLowThreshold = settings.value("ImageProcess/angleLowThreshold", 2.0).toDouble();
	ImageProcess::radius_max = settings.value("ImageProcess/radius_max", 350).toInt();
	ImageProcess::radius_min = settings.value("ImageProcess/radius_min", 250).toInt();

	HikVideoCapture::capInterval = settings.value("VideoCapture/capInterval", 7).toInt();
}

void MainWindow::writeSettings()
{
	QSettings settings(QCoreApplication::applicationDirPath().append("/config.ini"), QSettings::IniFormat);
	settings.beginGroup("ImageProcess");
	settings.setValue("sensorTriggered", ImageProcess::sensorTriggered);
	settings.setValue("angleHighThreshold", ImageProcess::angleHighThreshold);
	settings.setValue("angleLowThreshold", ImageProcess::angleLowThreshold);
	settings.setValue("radius_max", ImageProcess::radius_max);
	settings.setValue("radius_min", ImageProcess::radius_min);
	settings.endGroup();

	settings.beginGroup("VideoCapture");
	settings.setValue("capInterval", HikVideoCapture::capInterval);
	settings.endGroup();
}

void MainWindow::configWindow()
{
	//setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
	//setWindowState(Qt::WindowMaximized);
	////showMaximized();
	////去掉窗口边框  
	////setWindowFlags(Qt::FramelessWindowHint);
	////获取界面的宽度  
	//int width = this->width();
	////构建最小化、最大化、关闭按钮  
	//QToolButton *minButton = new QToolButton(this);
	//QToolButton *closeButton = new QToolButton(this);
	//minButton->setIconSize(QSize(40, 40));
	//closeButton->setIconSize(QSize(40, 40));
	////获取最小化、关闭按钮图标  
	//QPixmap minPix = style()->standardPixmap(QStyle::SP_TitleBarMinButton);
	//QPixmap closePix = style()->standardPixmap(QStyle::SP_TitleBarCloseButton);
	//minPix = minPix.scaled(20, 20);
	//closePix = closePix.scaled(20, 20);
	////设置最小化、关闭按钮图标  
	//minButton->setIcon(minPix);
	//closeButton->setIcon(closePix);
	////设置最小化、关闭按钮在界面的位置  
	//minButton->setGeometry(800 - 46, 10, 20, 20);
	//closeButton->setGeometry(800 - 25, 10, 20, 20);
	////设置鼠标移至按钮上的提示信息  
	//minButton->setToolTip(QStringLiteral("最小化"));
	//closeButton->setToolTip(QStringLiteral("关闭"));
	////设置最小化、关闭按钮的样式  
	//minButton->setStyleSheet("background-color:transparent;");
	//closeButton->setStyleSheet("background-color:transparent;");
	////关联最小化、关闭按钮的槽函数  
	//connect(minButton, SIGNAL(clicked()), this, SLOT(showMinimized()));
	//connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));
	recBtn = new QLabel(ui.playerTab);
	//recBtn->raise();
	recBtn->setAttribute(Qt::WA_TranslucentBackground);
	recBtn->setStyleSheet("background:transparent");
	recBtn->setScaledContents(true);
	recBtn->setGeometry(20, 20, 50, 50);
	QTimer *timer11 = new QTimer(this);
	connect(timer11, SIGNAL(timeout()), this, SLOT(reverseLabel()));
	timer11->start(2000);
	
	//定时任务，每天00:00触发
	int time_2_24 = QTime::currentTime().msecsTo(QTime(23, 59, 59, 999));
	QTimer::singleShot(time_2_24, this, SLOT(update24()));

	connect(ui.action_About_Qt, &QAction::triggered, qApp, &QApplication::aboutQt);
	//ui.imageMatchesLabel->setFixedSize(ui.imageMatchesLabel->size());
	ui.errorTextBrowser->setOpenLinks(false);
	connect(ui.errorTextBrowser, SIGNAL(anchorClicked(const QUrl&)), this, SLOT(anchorClickedSlot(const QUrl&)));
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
	//static QVector<QRgb> sColorTable;
	//// only create our color table once
	//if (sColorTable.isEmpty()) {
	//	for (int i = 0; i < 256; ++i)
	//		sColorTable.push_back(qRgb(i, i, i));
	//}
	mutex.lock();
	QImage image(imageProcess->imageMatches.data, imageProcess->imageMatches.cols, imageProcess->imageMatches.rows, imageProcess->imageMatches.step, QImage::Format_RGB888);
	//cv::imshow("matches", imageProcess->imageMatches);
	//qWarning() << imageProcess->imageMatches.type();	//CV_8UC3
	mutex.unlock();
	image.rgbSwapped();
	//image.setColorTable(sColorTable);
	image = image.scaled(ui.imageMatchesLabel->size(), Qt::KeepAspectRatio); //需要赋值
	ui.imageMatchesLabel->setPixmap(QPixmap::fromImage(image));		//必须label->setscaledcontent()
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
	if (bIsTimer24First)
	{
		QTimer *timer24 = new QTimer(this);
		connect(timer24, SIGNAL(timeout()), this, SLOT(update24()));
		timer24->start(24 * 3600 * 1000);
		bIsTimer24First = false;
	}
	else
	{
		//delete log
		clearLog(3);

		//restart
		on_action_Stop_triggered();
		on_action_Start_triggered();
	}
}

void MainWindow::on_action_Start_triggered()
{
	//开始顺序： start cap process sensor

	emit startCap(realPlayHandle);
}

void MainWindow::on_action_Stop_triggered()
{
	//结束顺序: stop sensor cap precess
	emit stopWheelSensor();
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
	emit setAlarm(ALARM_LIGHT_GREEN);
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
		recBtn->setVisible(true);
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
		ui.action_Start->setEnabled(true);
		ui.action_Stop->setEnabled(false);
		emit setAlarm(ALARM_LIGHT_OFF);
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
	recBtn->setPixmap(QPixmap(":/images/Resources/images/rec_red.png"));
	//recBtn->show();
}

void MainWindow::onRecStop()
{
	//QPainter p(recBtn);
	//p.drawPixmap(0, 0, QPixmap(":/images/Resources/images/rec_grey.png"));
	recBtn->setPixmap(QPixmap(":/images/Resources/images/rec_grey.png"));
	//recBtn->show();
}

bool MainWindow::isStartWheelSensor(bool r)
{
	if (r)
	{
		emit setAlarm(ALARM_LIGHT_GREEN);
		ui.action_Start->setEnabled(false);
		ui.action_Stop->setEnabled(true);
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
		recBtn->setVisible(false);
		emit stopProcess();
		emit stopCap();
	}
	return r;
}

//void MainWindow::on_startSaveBtn_clicked()
//{
//	plcSerial->emit startSave();
//}
//
//void MainWindow::on_stopSaveBtn_clicked()
//{
//	plcSerial->emit stopSave();
//}

void MainWindow::on_action_About_triggered()
{
	QMessageBox::about(this, QStringLiteral("关于"),
		QStringLiteral("<h3>宝钢环冷机台车轮子状态检测软件</h3>"
			"<p>版本号：<b>v1.1.0</b>"
			"<p>Copyright &copy; 2017 ZJU SKL."
			"<p>本软件由浙江大学开发，如果问题请联系cx3386@163.com"));
}

void MainWindow::reverseLabel()
{
	static bool bRec = false;
	if (bRec)
		onRecStart();
	else
		onRecStop();
	bRec = !bRec;
}
