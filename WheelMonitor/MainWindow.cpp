#include "stdafx.h"
#include "MainWindow.h"
#include "player.h"
#include "SettingDialog.h"
#include "outlierdetection.h"
#include "datatablewidget.h"
#include "common.h"

QString appName;
QString appDirPath;
QString appFilePath;
QString captureDirPath;
QString logDirPath;

bool MainWindow::bAppAutoRun = true;
bool MainWindow::bVerboseLog = true;

int const MainWindow::EXIT_CODE_REBOOT = -123456789;

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent), bIsRunning(false)
{
	//setWindowOpacity(1);
	appName = qApp->applicationName();
	appDirPath = qApp->applicationDirPath();				//the directory contains the app.exe, '/'e.g. C:/QQ
	appFilePath = qApp->applicationFilePath();				//the file path of app.exe, '/'e.g. C:/QQ/qq.exe
	captureDirPath = QString("%1/Capture").arg(appDirPath); //capture dir
	logDirPath = QString("%1/Log").arg(appDirPath);

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
	emit setAlarm(PLCSerial::AlarmOFF); //不能从线程外操作
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
	//appAutoRun(true);
	/***************setup reclabel****************/
	recLabel = new QLabel(ui.playerTab);
	recLabel->setObjectName(QStringLiteral("recLabel"));
	recLabel->setGeometry(20, 20, 50, 35);
	recLabel->setScaledContents(true);
	recLabel->setVisible(false);
	onRecStop(); //init as grey
	//recLabel->raise();
	//recLabel->setAttribute(Qt::WA_TranslucentBackground);
	//ui.playerWidget->setGeometry(0, 0, 0, 0);	//to initilize the geometry of the playerwidget, make the drawRoiArea() correct
	/************roi rect**************/
	lineL = new QFrame(ui.playerTab);
	lineL->setFrameShape(QFrame::VLine);
	lineL->setFrameShadow(QFrame::Plain);
	lineL->setLineWidth(2);
	lineL->setStyleSheet("color: rgb(255, 0, 0);");
	lineR = new QFrame(ui.playerTab);
	lineR->setFrameShape(QFrame::VLine);
	lineR->setFrameShadow(QFrame::Plain);
	lineR->setLineWidth(2);
	lineR->setStyleSheet("color: rgb(255, 0, 0);");
	lineT = new QFrame(ui.playerTab);
	lineT->setFrameShape(QFrame::HLine);
	lineT->setFrameShadow(QFrame::Plain);
	lineT->setLineWidth(2);
	lineT->setStyleSheet("color: rgb(255, 0, 0);");
	lineB = new QFrame(ui.playerTab);
	lineB->setFrameShape(QFrame::HLine);
	lineB->setFrameShadow(QFrame::Plain);
	lineB->setLineWidth(2);
	lineB->setStyleSheet("color: rgb(255, 0, 0);");
	//drawRoiArea();
	connect(ui.playerWidget, &MyWidget::myResize, this, &MainWindow::drawRoiArea);
	/*********playbackTab**********/
	Player *playbackWidget = new Player(ui.playbackTab);
	DataTableWidget *dataTableWidget = new DataTableWidget(ui.playbackTab);
	connect(dataTableWidget, &DataTableWidget::showVideo, playbackWidget, &Player::setUrl);
	//playbackWidget->setUrl(QUrl::fromLocalFile("e:/1.mp4"));
	QHBoxLayout *hLayout = new QHBoxLayout;
	hLayout->addWidget(dataTableWidget);
	hLayout->addWidget(playbackWidget);
	ui.playbackTab->setLayout(hLayout);
	ui.centralTabWidget->setCurrentIndex(0);
	//ui.playbackTab->setEnabled(false);	//unclickable and unchosenable
	ui.centralTabWidget->setTabEnabled(2, false); //cant toggle to this tab
	/**********end playbackTab**********/

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
	QTimer::singleShot(msTo12, this, SLOT(setup24timer()));

	////Only for testing 定时emit startsave and stopsave
	//QTimer *timerTMP1 = new QTimer(this);
	//connect(timerTMP1, SIGNAL(timeout()), this, SLOT(startOrStopSave()));
	//timerTMP1->start(10000);

	connect(ui.action_About_Qt, &QAction::triggered, qApp, &QApplication::aboutQt);
	//ui.imageMatchesLabel->setFixedSize(ui.imageMatchesLabel->size());
	ui.errorTextBrowser->setOpenLinks(false);
	connect(ui.errorTextBrowser, SIGNAL(anchorClicked(const QUrl &)), this, SLOT(anchorClickedSlot(const QUrl &)));

	realPlayHandle = (HWND)ui.playerWidget->winId();
	//care start order

	outputMessage = new MyMessageOutput;

	imageProcess = new ImageProcess(dataTableWidget->model);
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

	connect(this, &MainWindow::SyncCameraTime, videoCapture, &HikVideoCapture::syncCameraTime); //2017.11.10
	connect(this, &MainWindow::startCap, videoCapture, &HikVideoCapture::startCap);
	connect(videoCapture, &HikVideoCapture::isStartCap, this, &MainWindow::isStartCap);
	connect(this, &MainWindow::stopCap, videoCapture, &HikVideoCapture::stopCap);
	connect(videoCapture, &HikVideoCapture::isStopCap, this, &MainWindow::isStopCap);
	connect(videoCapture, &HikVideoCapture::wheelTimeout, imageProcess, &ImageProcess::wheelTimeout);
	connect(videoCapture, &HikVideoCapture::saveStopped, this, &MainWindow::onRecStop); //when stop saving video(sensorOUT or saveTimeOut), update UI.REC.gray

	//connect(this, &MainWindow::startSave, videoCapture, &HikVideoCapture::startSave);
	//connect(this, &MainWindow::stopSave, videoCapture, &HikVideoCapture::stopSave);

	connect(this, &MainWindow::startProcess, imageProcess, &ImageProcess::startImageProcess);
	connect(this, &MainWindow::stopProcess, imageProcess, &ImageProcess::stopImageProcess);

	connect(this, &MainWindow::initPlcSerial, plcSerial, &PLCSerial::init);
	connect(this, &MainWindow::connectPLC, plcSerial, &PLCSerial::connectPLC);
	connect(plcSerial, &PLCSerial::isConnectPLC, this, &MainWindow::isConnectPLC);
	connect(this, &MainWindow::disconnectPLC, plcSerial, &PLCSerial::disconnectPLC);
	connect(plcSerial, &PLCSerial::isDisconnectPLC, this, &MainWindow::isDisconnectPLC);

	connect(imageProcess, &ImageProcess::showAlarmNum, this, &MainWindow::uiAlarmNum);
	connect(imageProcess, &ImageProcess::showImageMatches, this, &MainWindow::uiShowMatches);
	connect(imageProcess, &ImageProcess::speedClcReady, this, &MainWindow::uiShowLastSpeed);
	connect(imageProcess, &ImageProcess::rtSpeedReady, this, &MainWindow::uiShowRtSpeed);
	connect(plcSerial, &PLCSerial::ADSpeedReady, this, &MainWindow::uiShowCartSpeed);
	connect(imageProcess, &ImageProcess::setAlarmLight, plcSerial, &PLCSerial::Alarm);

	//When wheel enters detect area, i.e., lighten the LEFT sensor
	connect(plcSerial, &PLCSerial::sensorIN, videoCapture, &HikVideoCapture::startSave); //1. start to save video
	connect(plcSerial, &PLCSerial::sensorIN, imageProcess, &ImageProcess::sensorIN);	 //2. start image process
	connect(plcSerial, &PLCSerial::sensorIN, this, &MainWindow::onRecStart);			 //3. update UI.REC.red

	//When wheel leaves detect area, i.e., leaves the RIGHT sensor
	connect(plcSerial, &PLCSerial::sensorOUT, videoCapture, &HikVideoCapture::stopSave); //1. stop to save video
	connect(plcSerial, &PLCSerial::sensorOUT, imageProcess, &ImageProcess::sensorOUT);   //2. stop image process
	//connect(plcSerial, &PLCSerial::sensorOUT, this, &MainWindow::onRecStop);			    //3. update UI.REC.gray

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
	//pixmap = pixmap.scaled(256, 256);
	ui.alarmPushButton->setIcon(QIcon(pixmap));
}

void MainWindow::readSettings()
{
	QSettings settings(QString("%1/config.ini").arg(appDirPath), QSettings::IniFormat);
	bAppAutoRun = settings.value("Common/appAutoRun", true).toBool();
	appAutoRun(bAppAutoRun);
	bVerboseLog = settings.value("Common/verboseLog", true).toBool();
	/*implement of verboselog*/
	ImageProcess::g_imgParam.sensorTriggered = settings.value("ImageProcess/sensorTriggered", false).toBool();
	ImageProcess::g_imgParam.angleBigRatio = settings.value("ImageProcess/angleBigRatio", 1.2).toDouble();
	ImageProcess::g_imgParam.angleSmallRatio = settings.value("ImageProcess/angleSmallRatio", 0.8).toDouble();
	ImageProcess::g_imgParam.radius_max = settings.value("ImageProcess/radius_max", 350).toInt();
	ImageProcess::g_imgParam.radius_min = settings.value("ImageProcess/radius_min", 250).toInt();
	ImageProcess::g_imgParam.roiRect = cv::Rect(settings.value("ImageProcess/roiRect_x", 220).toInt(),
		settings.value("ImageProcess/roiRect_y", 0).toInt(),
		settings.value("ImageProcess/roiRect_w", 800).toInt(),
		settings.value("ImageProcess/roiRect_h", 720).toInt());
	ocr::p.plate_x_min = settings.value("ocr_parameters/plate_x_min", 100).toInt();
	ocr::p.plate_x_max = settings.value("ocr_parameters/plate_x_max", 250).toInt();
	ocr::p.plate_y_min = settings.value("ocr_parameters/plate_y_min", 190).toInt();
	ocr::p.plate_y_max = settings.value("ocr_parameters/plate_y_max", 250).toInt();
	ocr::p.plate_width_min = settings.value("ocr_parameters/plate_width_min", 160).toInt();
	ocr::p.plate_width_max = settings.value("ocr_parameters/plate_width_max", 190).toInt();
	ocr::p.plate_height_min = settings.value("ocr_parameters/plate_height_min", 110).toInt();
	ocr::p.plate_height_max = settings.value("ocr_parameters/plate_height_max", 130).toInt();
	ocr::p.num_width_min = settings.value("ocr_parameters/num_width_min", 20).toInt();
	ocr::p.num_width_max = settings.value("ocr_parameters/num_width_max", 50).toInt();
	ocr::p.num_height_min = settings.value("ocr_parameters/num_height_min", 50).toInt();
	ocr::p.num_height_max = settings.value("ocr_parameters/num_height_max", 75).toInt();

	HikVideoCapture::capInterval = settings.value("VideoCapture/capInterval", 7).toInt();
	ImageProcess::g_imgParam.angle2Speed = 60 * (M_PI * 0.650 / 360) / ((HikVideoCapture::capInterval + 1) / 25.0);
}

void MainWindow::writeSettings()
{
	QSettings settings(QString("%1/config.ini").arg(appDirPath), QSettings::IniFormat);
	settings.beginGroup("Common");
	settings.setValue("appAutoRun", bAppAutoRun);
	settings.setValue("verboseLog", bVerboseLog);
	settings.endGroup();

	settings.beginGroup("ImageProcess");
	settings.setValue("sensorTriggered", ImageProcess::g_imgParam.sensorTriggered);
	settings.setValue("angleBigRatio", ImageProcess::g_imgParam.angleBigRatio);
	settings.setValue("angleSmallRatio", ImageProcess::g_imgParam.angleSmallRatio);
	settings.setValue("radius_max", ImageProcess::g_imgParam.radius_max);
	settings.setValue("radius_min", ImageProcess::g_imgParam.radius_min);
	settings.setValue("roiRect_x", ImageProcess::g_imgParam.roiRect.x);
	settings.setValue("roiRect_y", ImageProcess::g_imgParam.roiRect.y);
	settings.setValue("roiRect_w", ImageProcess::g_imgParam.roiRect.width);
	settings.setValue("roiRect_h", ImageProcess::g_imgParam.roiRect.height);
	settings.endGroup();

	settings.beginGroup("ocr_parameters");
	settings.setValue("plate_x_min", ocr::p.plate_x_min);
	settings.setValue("plate_x_max", ocr::p.plate_x_max);
	settings.setValue("plate_y_min", ocr::p.plate_y_min);
	settings.setValue("plate_y_max", ocr::p.plate_y_max);
	settings.setValue("plate_width_min", ocr::p.plate_width_min);
	settings.setValue("plate_width_max", ocr::p.plate_width_max);
	settings.setValue("plate_height_min", ocr::p.plate_height_min);
	settings.setValue("plate_height_max", ocr::p.plate_height_max);
	settings.setValue("num_width_min", ocr::p.num_width_min);
	settings.setValue("num_width_max", ocr::p.num_width_max);
	settings.setValue("num_height_min", ocr::p.num_height_min);
	settings.setValue("num_height_max", ocr::p.num_height_max);
	settings.endGroup();

	settings.beginGroup("VideoCapture");
	settings.setValue("capInterval", HikVideoCapture::capInterval);
	settings.endGroup();
}

void MainWindow::clearLog(int nDays)
{
	QDir dir(logDirPath);
	QStringList filters;
	dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
	filters << "*.log";			 //设置过滤类型
	dir.setNameFilters(filters); //设置文件名的过滤
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
	QDir dir(captureDirPath); //必须保证该文件夹里面没有其他文件，否则会误删
	QStringList filters;
	dir.setFilter(QDir::Dirs);
	dir.setSorting(QDir::Name); //from little to big
	QFileInfoList list = dir.entryInfoList();
	int nDirName = QDate::currentDate().toString("yyyyMMdd").toInt();
	if (list.length() != 0)
		for (int i = 0; i < list.size(); ++i)
		{
			//prevent deleting the '.' and '..' dirs, very important
			if (list.at(i).fileName() == "." || list.at(i).fileName() == "..")
				continue;
			if (list.at(i).fileName().toInt() <= (nDirName - nDays))
			{
				dir = list.at(i).absoluteFilePath();
				if (!dir.removeRecursively())
					r = false;
			}
			else
				break;
		}
	else
	{
		qDebug() << "MainWindow(delCapDir):no file";
		return false;
	}
	return r;
}

bool MainWindow::makeDir(QString fullPath)
{
	QDir dir(fullPath);
	bool ok = dir.mkpath(fullPath); //create directory path recursively, if exists, returns true.
	return ok;
}

void MainWindow::appAutoRun(bool bAutoRun) //autorun when computer start
{
	//HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Run
	QSettings *reg = new QSettings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);

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

void MainWindow::drawRoiArea()
{
	QRect srcRect = QRect(ImageProcess::g_imgParam.roiRect.x, ImageProcess::g_imgParam.roiRect.y, ImageProcess::g_imgParam.roiRect.width, ImageProcess::g_imgParam.roiRect.height);

	QRect rect = ui.playerWidget->geometry();
	QPoint tl = QPoint(srcRect.topLeft().x() / 1280.0 * rect.width(), srcRect.topLeft().y() / 720.0 * rect.height());
	QPoint tr = QPoint(srcRect.topRight().x() / 1280.0 * rect.width(), srcRect.topRight().y() / 720.0 * rect.height());
	QPoint bl = QPoint(srcRect.bottomLeft().x() / 1280.0 * rect.width(), srcRect.bottomLeft().y() / 720.0 * rect.height());
	//QPoint br = QPoint((srcRect.bottomRight.x() / 1280.0)*rect.width(), (srcRect.bottomRight.y() / 720.0)*rect.height());
	QSize size = rect.size();
	QPoint pt = rect.topLeft();
	lineL->setGeometry(pt.x() + tl.x(), pt.y() + tl.y(), 2, srcRect.height() / 720.0 * rect.height());
	lineR->setGeometry(pt.x() + tr.x(), pt.y() + tr.y(), 2, srcRect.height() / 720.0 * rect.height());
	lineT->setGeometry(pt.x() + tl.x(), pt.y() + tl.y(), srcRect.width() / 1280.0 * rect.width(), 2);
	lineB->setGeometry(pt.x() + bl.x(), pt.y() + bl.y(), srcRect.width() / 1280.0 * rect.width(), 2);
}

void MainWindow::closeEvent(QCloseEvent *event)
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

void MainWindow::uiAlarmNum(const QString &num)
{
	ui.lcdNumber->setStyleSheet("color:rgb(255, 0, 0);");
	ui.lcdNumber->display(num);
}

void MainWindow::uiShowMatches()
{
	mutex.lock();
	QImage image(imageProcess->imageMatches.data, imageProcess->imageMatches.cols, imageProcess->imageMatches.rows, imageProcess->imageMatches.step, QImage::Format_RGB888);
	mutex.unlock();
	image.rgbSwapped();
	//image.setColorTable(sColorTable);
	image = image.scaled(ui.imageMatchesLabel->size(), Qt::KeepAspectRatio); //Note: not like rezise, scaled need assignment
	ui.imageMatchesLabel->setPixmap(QPixmap::fromImage(image));				 //label->setscaledcontent()
}

void MainWindow::uiShowLastSpeed(double speed)
{
	QString str = QString::number(speed, 'f', 2);
	ui.lastSpeedLineEdit->setText(str);
}

void MainWindow::uiShowRtSpeed(double speed)
{ //do not show the real time speed any more //2017/11/24
	QString str = QString::number(speed, 'f', 2);
	//ui.rtSpeedLineEdit->setText(str);
}

void MainWindow::uiShowCartSpeed(double speed)
{
	QString str = QString::number(speed, 'f', 2);
	ui.cartSpeedLineEdit->setText(str);
}

void MainWindow::uiShowLogMessage(const QString &message)
{
	ui.logTextBrowser->append(message);
}

void MainWindow::uiShowErrorMessage(const QString &message)
{
	QString errormsg = QStringLiteral("%1 <a href = \"%2\">Video</a>").arg(message).arg(videoCapture->saveDirLink);
	ui.errorTextBrowser->append(errormsg);
	ui.logTabWidget->setCurrentIndex(1);
}

void MainWindow::anchorClickedSlot(const QUrl &url)
{
	//QDesktopServices::openUrl(QUrl(url.toString(), QUrl::TolerantMode));
	QDesktopServices::openUrl(url);
}

void MainWindow::start24timer()
{ //start24timer at 12 o'clock
	QTimer *timer24 = new QTimer(this);
	connect(timer24, SIGNAL(timeout()), this, SLOT(update24()));
	timer24->start(24 * 60 * 60 * 1000);
	update24();
}

void MainWindow::update24()
{
	//delete log
	clearLog(30);
	//clear capVideo
	delCapDir(30);
	//sync cameratime
	emit SyncCameraTime();
	//mkpath for capVideo
	QDate date = QDate::currentDate();

	/*create the directory path everyday*/
	QString today = date.toString("yyyyMMdd");
	QString saveDir = QStringLiteral("%1/%2").arg(captureDirPath).arg(today);
	makeDir(saveDir);
	QString tomorrow = date.addDays(1).toString("yyyyMMdd");
	saveDir = QStringLiteral("%1/%2").arg(captureDirPath).arg(tomorrow);
	makeDir(saveDir);
	QString logDir = QStringLiteral("%1").arg(logDirPath);
	makeDir(logDir);

	//restart
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
	settingDialog = new SettingDialog(this);
	//settingDialog->setAttribute(Qt::WA_DeleteOnClose);
	connect(settingDialog, &SettingDialog::roiChanged, this, &MainWindow::drawRoiArea);
	settingDialog->deleteLater(); //will delete the connect when return from this funciton
	settingDialog->exec();
}
void MainWindow::on_action_Quit_triggered()
{
	this->close();
}
void MainWindow::on_alarmPushButton_clicked()
{
	//when not running, start the monitor.
	if (false == bIsRunning)
	{
		on_action_Start_triggered();
		return;
	}
	//start sucess
	emit setAlarm(PLCSerial::AlarmColorGreen);
	ui.lcdNumber->setStyleSheet("color:rgb(0, 255, 0);");
	ui.lcdNumber->display(QString("888"));
	ui.logTabWidget->setCurrentIndex(0); //reset logTabWidget to log tab(not the error tab)
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
		ui.action_Start->setEnabled(true);
		ui.action_Stop->setEnabled(false);
		bIsRunning = false;
		emit setAlarm(PLCSerial::AlarmOFF);
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
	//p.setCompositionMode(QPainter::CompositionMode_Source);//注意这一行代码??
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

bool MainWindow::isConnectPLC(bool r)
{
	if (r)
	{
		ui.action_Start->setEnabled(false);
		ui.action_Stop->setEnabled(true);
		bIsRunning = true;
		on_alarmPushButton_clicked();
		qWarning("start success");
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
		recLabel->setVisible(false);
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

void MainWindow::on_action_About_triggered()
{
	QMessageBox::about(this, QStringLiteral("关于"),
		QStringLiteral("<h3>宝钢环冷机台车轮子状态检测软件</h3>"
			"<p>版本号：<b>v1.1.0</b>"
			"<p>Copyright &copy; 2017 ZJU SKL."
			"<p>本软件由浙江大学开发，如果问题请联系cx3386@163.com"));
}