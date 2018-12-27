#include "stdafx.h"

#include "AlarmManager.h"
#include "Plc.h"
#include "ViewSensorImage.h"
#include "backuplogdialog.h"
#include "common.h"
#include "confighelper.h"
#include "database.h"
#include "hikvideocapture.h"
#include "imageprocess.h"
#include "mainwindow.h"
#include "mysqltablemodel.h"
#include "outlierhelper.h"
#include "playbackwidget.h"
#include "settingdialog.h"

MainWindow::MainWindow(ConfigHelper* _configHelper, QWidget* parent /*= Q_NULLPTR*/)
	: QMainWindow(parent)
	, configHelper(_configHelper)
	, plc(new Plc(configHelper))
	, alarmManager(new AlarmManager(this))
{
	//setWindowOpacity(1);
	ui.setupUi(this);
	configWindow();
	const auto deskRect = QApplication::desktop()->availableGeometry();
	// �ֱ�������
	if (deskRect.width() < 1920) {
		setWindowFlag(Qt::MSWindowsFixedSizeDialogHint);
		resize(deskRect.size());
	}

	//auto start
	if (configHelper->startAtLaunch) {
		on_action_Start_triggered();
	}
}

MainWindow::~MainWindow()
{
	//bool b = true;
	//connect(plc, &Plc::setUiAlarm, this, [&b] { b = false; }); //before setUiAlarm finished, DONOT delete the pointer "this". �������쳣: ��ȡ����Ȩ�޳�ͻ��**this** �� 0xFFFFFFFFFFFFFFFF
	//emit setAlarm(AlarmOFF); //cannot operate out of thread. use Signal-Slot.
	//while (b) //wait until light is set off
	//{
	//    QCoreApplication::processEvents();
	//}
	imageProcessThread[0]->quit();
	imageProcessThread[0]->wait();
	imageProcessThread[1]->quit();
	imageProcessThread[1]->wait();
	//����improc�ڶ��ϣ�����û��qparent�������Ҫ�ֶ�����
	imageProcess[0]->deleteLater();
	imageProcess[1]->deleteLater();
	plcThread->quit();
	plcThread->wait();
	plc->deleteLater();//���̵߳����ˣ�����Ŷ
	dbWatcherThread->quit();
	dbWatcherThread->wait();
	watcher->deleteLater();
}

void MainWindow::configWindow()
{
	//����Dockλ��
	m_docks << ui.dockWidget_dashboard << ui.dockWidget_alarm << ui.dockWidget_plc;
	for (auto dock : m_docks) {
		removeDockWidget(dock);
	}
	addDockWidget(Qt::BottomDockWidgetArea, m_docks[0]);
	splitDockWidget(m_docks[0], m_docks[1], Qt::Horizontal);
	splitDockWidget(m_docks[1], m_docks[2], Qt::Horizontal);
	for (auto dock : m_docks) {
		dock->show();
	}
	ui.dockWidget_match0->hide();//Ĭ������matchresult
	ui.dockWidget_match1->hide();//Ĭ������matchresult
	/* Setup the record(save) indication of VideoCapture */
	recLabel_pre[0] = new QLabel(ui.cam0Tab);
	recLabel_pre[1] = new QLabel(ui.cam1Tab);
	for (auto i : recLabel_pre) {
		i->setGeometry(20, 20, 50, 35);
		i->setScaledContents(true); //scale its contents to fill all available space.
		i->setVisible(false);
	}
	recLabel_input[0] = new QLabel(ui.realVideoLabel_0);
	recLabel_input[1] = new QLabel(ui.realVideoLabel_1);
	for (auto i : recLabel_input) {
		i->setGeometry(10, 10, 40, 28);
		i->setScaledContents(true);
		i->setVisible(false);
	}
	onRecordOFF(0);
	onRecordOFF(1);
	/* playbackTab */
	playBackWidget = new PlayBackWidget(ui.playbackTab);
	auto* playBackLayout = new QGridLayout(ui.playbackTab);
	playBackLayout->addWidget(playBackWidget);
	connect(ui.centralTabWidget, &QTabWidget::currentChanged, playBackWidget, &PlayBackWidget::clearMedia);
	setupDataMapper();

	//CARE init order: cap, plc, improc
	/* video capture */
	videoCapture[0] = new HikVideoCapture(configHelper, 0, (HWND)ui.cam0HWND->winId(), this);
	videoCapture[1] = new HikVideoCapture(configHelper, 1, (HWND)ui.cam1HWND->winId(), this);
	connect(videoCapture[0], &HikVideoCapture::recordON, this, [=]() { onRecordON(0); });
	connect(videoCapture[1], &HikVideoCapture::recordON, this, [=]() { onRecordON(1); });
	connect(videoCapture[0], &HikVideoCapture::recordOFF, this, [=]() { onRecordOFF(0); });
	connect(videoCapture[1], &HikVideoCapture::recordOFF, this, [=]() { onRecordOFF(1); });

	/* image process */
	//plcserial�ڳ�ʼ���б���
	imageProcess[0] = new ImageProcess(configHelper, videoCapture[0], plc); // don't let parent = this. or can't move to QThread
	imageProcess[1] = new ImageProcess(configHelper, videoCapture[1], plc);
	connect(imageProcess[0], &ImageProcess::showFrame, this, [=]() { uiShowRealtimeImage(0); });
	connect(imageProcess[1], &ImageProcess::showFrame, this, [=]() { uiShowRealtimeImage(1); });
	ui.matchViewer0->bindDev(imageProcess[0]);
	ui.matchViewer1->bindDev(imageProcess[1]);
	/* image process thread */
	imageProcessThread[0] = new QThread(this);
	imageProcessThread[1] = new QThread(this);
	imageProcess[0]->moveToThread(imageProcessThread[0]);
	imageProcess[1]->moveToThread(imageProcessThread[1]);
	//connect(imageProcessThread[0], &QThread::finished, imageProcess[0], &QObject::deleteLater);
	//connect(imageProcessThread[1], &QThread::finished, imageProcess[1], &QObject::deleteLater);

	/* plc thread */
	plcThread = new QThread(this);
	plc->moveToThread(plcThread);
	//connect(plcSerialThread, &QThread::finished, plcSerial, &QObject::deleteLater);
	connect(plc, &Plc::truckSpeedReady, this, &MainWindow::uiShowTruckSpeed); //bind plc::adSpeed to cartSpeed

	/* alarmLight */
	alarmManager->bindMainWindow(this);
	alarmManager->bindPLC(plc);
	alarmManager->bindPlayBack(playBackWidget);
	alarmManager->bindImProc(imageProcess[0]);
	alarmManager->bindImProc(imageProcess[1]);
	/* sensor light */
	connect(plc, &Plc::cio0Update, this, &MainWindow::uiShowCio0); //��ʾcio0
	connect(plc->handleSensorDevice[0], &HandleSensorDevice::sensorUpdate, this, [=](int state) {uiShowSensor(state, 0); }); //�жϴ������Ƿ�������
	connect(plc->handleSensorDevice[1], &HandleSensorDevice::sensorUpdate, this, [=](int state) {uiShowSensor(state, 1); }); //�жϴ������Ƿ�������
	connect(plc, &Plc::connectError, this, [=](int errorId) {
		if (errorId == 0) {
			ui.statusBar->showMessage(QStringLiteral("������PLC"));
			ui.action_Start->setEnabled(true);
		}
		else if (errorId == 1) {
			ui.statusBar->showMessage(QStringLiteral("δ����PLC"));
		}
	});
	/* start thread */
	plcThread->start();
	plc->connect();
	// 500ms����������ȷ��

	imageProcessThread[0]
		->start();
	imageProcessThread[1]->start();
	imageProcess[0]->init();
	imageProcess[1]->init();

	setupDatabaseWatcher();

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
	for (const auto& i : list) {
		bool ok;
		int nFileDay = i.fileName().toInt(&ok);
		if (ok && (nFileDay <= nDirName - nDays) && nFileDay >= 19700000) {
			QDir selDir = i.absoluteFilePath();
			if (!selDir.removeRecursively())
				r = false;
		}
	}
	if (false == r) {
		qDebug() << "MainWindow(cleanDir): error";
	}
	return r;
}

void MainWindow::closeEvent(QCloseEvent* event)
{
	if (!(QMessageBox::information(this, QStringLiteral("�˳���"), QStringLiteral("���Ҫ�˳���\n������˳����򣬼�ؽ���ֹ��"), QStringLiteral("ȷ��"), QStringLiteral("ȡ��")))) {
		configHelper->save();
		event->accept();
		//qApp->exit(0);//�����޸ģ�ֱ���˳�
	}
	else {
		event->ignore();
	}
}

void MainWindow::uiShowRealtimeImage(int deviceIndex)
{
	cv::Mat src = imageProcess[deviceIndex]->getFrameToShow();
	QImage im(src.data, src.cols, src.rows, src.step, QImage::Format_RGB888);
	//QImage dstImage = image.copy(); //deep copy
	im = im.rgbSwapped();
	switch (deviceIndex) {
	case 0:
		im = im.scaled(ui.realVideoLabel_0->size(), Qt::KeepAspectRatio); //Note: not like rezise, scaled need assignment
		ui.realVideoLabel_0->setPixmap(QPixmap::fromImage(im));
		break;
	case 1:
		im = im.scaled(ui.realVideoLabel_1->size(), Qt::KeepAspectRatio); //Note: not like rezise, scaled need assignment
		ui.realVideoLabel_1->setPixmap(QPixmap::fromImage(im));
		break;
	default:
		break;
	}
}

void MainWindow::uiShowAlarmLight(AlarmColor alarmColor)
{
	QIcon green = QIcon(QPixmap(":/WheelMonitor/Resources/images/green.png"));
	QIcon red = QIcon(QPixmap(":/WheelMonitor/Resources/images/red.png"));
	QIcon gray = QIcon(QPixmap(":/WheelMonitor/Resources/images/gray.png"));
	QIcon yellow = QIcon(QPixmap(":/WheelMonitor/Resources/images/yellow.png"));
	QIcon color;
	switch (alarmColor) {
	case AlarmColor::Unkown:
		color = gray;
		break;
	case AlarmColor::Green:
		color = green;
		break;
	case AlarmColor::Red:
		color = red;
		break;
	case AlarmColor::Yellow:
		color = yellow;
		break;
	case AlarmColor::Gray:
		color = gray;
		break;
	default:
		break;
	}
	ui.alarmPushButton->setIcon(color);
	ui.alarmPushButton->setEnabled(bIsRunning);
}

void MainWindow::uiShowTruckSpeed()
{
	QString str = QString::number(plc->getTruckSpeed(0), 'f', 2); //��Ȧ�ٶ�
	ui.cartSpeedLineEdit->setText(str);
}

void MainWindow::uiShowCio0(int cio0)
{
	//return;
	QPixmap green = QPixmap(":/WheelMonitor/Resources/images/green.png");
	//QPixmap red = QPixmap(":/WheelMonitor/Resources/images/red.png");
	QPixmap gray = QPixmap(":/WheelMonitor/Resources/images/gray.png");
	bool b;
	b = cio0 >> 1 & 1;
	if (b) {
		ui.Btn_CIO1_1->setIcon(QIcon(green));
	}
	else {
		ui.Btn_CIO1_1->setIcon(QIcon(gray));
	}
	b = cio0 >> 2 & 1;
	if (b) {
		ui.Btn_CIO1_2->setIcon(QIcon(green));
	}
	else {
		ui.Btn_CIO1_2->setIcon(QIcon(gray));
	}
	b = cio0 >> 3 & 1;
	if (b) {
		ui.Btn_CIO1_3->setIcon(QIcon(green));
	}
	else {
		ui.Btn_CIO1_3->setIcon(QIcon(gray));
	}
	b = cio0 >> 4 & 1;
	if (b) {
		ui.Btn_CIO1_4->setIcon(QIcon(green));
	}
	else {
		ui.Btn_CIO1_4->setIcon(QIcon(gray));
	}
	b = cio0 >> 5 & 1;
	if (b) {
		ui.Btn_CIO1_5->setIcon(QIcon(green));
	}
	else {
		ui.Btn_CIO1_5->setIcon(QIcon(gray));
	}
	b = cio0 >> 6 & 1;
	if (b) {
		ui.Btn_CIO1_6->setIcon(QIcon(green));
	}
	else {
		ui.Btn_CIO1_6->setIcon(QIcon(gray));
	}
	b = cio0 >> 7 & 1;
	if (b) {
		ui.Btn_CIO1_7->setIcon(QIcon(green));
	}
	else {
		ui.Btn_CIO1_7->setIcon(QIcon(gray));
	}
	b = cio0 >> 8 & 1;
	if (b) {
		ui.Btn_CIO1_8->setIcon(QIcon(green));
	}
	else {
		ui.Btn_CIO1_8->setIcon(QIcon(gray));
	}
}

//! ���½���Ĺ�紫����״̬
void MainWindow::uiShowSensor(int state, int devId)
{
	QIcon green = QIcon(QPixmap(":/WheelMonitor/Resources/images/green.png"));
	QIcon red = QIcon(QPixmap(":/WheelMonitor/Resources/images/red.png"));
	//QIcon gray = QIcon(QPixmap(":/WheelMonitor/Resources/images/gray.png"));
	bool bit;
	QIcon icon;
	//ֻ������Ȧ
	if (devId == 0) {
		bit = state & 1;
		icon = bit ? green : red;
		ui.btn_sol0->setIcon(icon); //1
		state = state >> 1;
		bit = state & 1;
		icon = bit ? green : red;
		ui.btn_sol1->setIcon(icon); //2
		state = state >> 1;
		bit = state & 1;
		icon = bit ? green : red;
		ui.btn_sor0->setIcon(icon); //3
		state = state >> 1;
		bit = state & 1;
		icon = bit ? green : red;
		ui.btn_sor1->setIcon(icon); //4
	}
	//ֻ������Ȧ
	else if (devId == 1) {
		state = state >> 4;
		bit = state & 1;
		icon = bit ? green : red;
		ui.btn_sir0->setIcon(icon); //5
		state = state >> 1;
		bit = state & 1;
		icon = bit ? green : red;
		ui.btn_sir1->setIcon(icon); //6
		state = state >> 1;
		bit = state & 1;
		icon = bit ? green : red;
		ui.btn_sil0->setIcon(icon); //7
		state = state >> 1;
		bit = state & 1;
		icon = bit ? green : red;
		ui.btn_sil1->setIcon(icon); //8
	}
}
//! ��ʾCio100�ĵơ����пر����ƣ�1Ϊ����0Ϊ��
void MainWindow::uiShowCio100(int cio100)
{
	QIcon green = QIcon(QPixmap(":/WheelMonitor/Resources/images/green.png"));
	QIcon red = QIcon(QPixmap(":/WheelMonitor/Resources/images/red.png"));
	QIcon yellow = QIcon(QPixmap(":/WheelMonitor/Resources/images/yellow.png"));
	bool bit;
	QIcon icon;
	//c100.4���
	bit = (cio100 >> 4) & 1;
	icon = bit ? yellow : green;
	ui.Btn_CIO100_4->setIcon(icon);
	//c100.5���
	bit = (cio100 >> 5) & 1;
	icon = bit ? red : green;
	ui.Btn_CIO100_5->setIcon(icon);
	//c100.6�ڻ�
	bit = (cio100 >> 6) & 1;
	icon = bit ? yellow : green;
	ui.Btn_CIO100_6->setIcon(icon);
	//c100.7�ں�
	bit = (cio100 >> 7) & 1;
	icon = bit ? red : green;
	ui.Btn_CIO100_7->setIcon(icon);
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
	dirPathList << videoDirPath << ocrDirPath;
	for (auto&& path : std::as_const(dirPathList)) {
		cleanDir(path, 30); //clean video(, matched), logs //can be operated alone
		QString saveDir = QStringLiteral("%1/%2").arg(path).arg(today);
		QDir dir;
		dir.mkpath(saveDir); //create directory path recursively, if exists, returns true.
		saveDir = QStringLiteral("%1/%2").arg(path).arg(tomorrow); //create the directory path everyday
		dir.mkpath(saveDir);
	}
	// log�ļ���Ϊ�����ģ����Բ��ô��� [12/6/2018 cx3386]
	//QDir logDir;
	//logDir.mkpath(logDirPath);

	//restart every day
	//on_action_Restart_triggered();
}

void MainWindow::setupScheduler(int hh, int mm, bool execnow /*= true*/, bool everyday /*= true*/)
{
	if (execnow) {
		execScheduledTasks();
	}
	///start the daily mission at hh:mm today or tomorrow
	QDate date = QDate::currentDate();
	if (QTime::currentTime().msecsTo(QTime(hh, mm, 00, 000)) <= 0) { //if the current time is after 12:00,
		date = date.addDays(1); //2. than set date to tomorrow
	}
	auto ms_to_hhmm = QDateTime::currentDateTime().msecsTo(QDateTime(date, QTime(hh, mm))); //precision of 20ms
	if (everyday) {
		QTimer::singleShot(ms_to_hhmm, this, [=]() {
			execScheduledTasks();
			auto* timer24 = new QTimer(this);
			connect(timer24, SIGNAL(timeout()), this, SLOT(execScheduledTasks()));
			timer24->start(24 * 60 * 60 * 1000);
		});
	}
	else {
		QTimer::singleShot(ms_to_hhmm, this, SLOT(execScheduledTasks()));
	}
}

void MainWindow::on_action_Start_triggered()
{
	if (bIsRunning) {
		QMessageBox::warning(this, QStringLiteral("����"), QStringLiteral("���������У������ظ����У������쳣��������"), QStringLiteral("ȷ��"));
		return;
	}
	// ��ʼѭ����ȡ�豸�������Ϣ
	plc->start();
	//return;//���ԣ�ֻ��plc
	bool cap0 = videoCapture[0]->start();
	if (cap0) {
		imageProcess[0]->start();
		/* rec label visible */
		recLabel_pre[0]->setVisible(true);
		recLabel_input[0]->setVisible(true);
	}
	else {
		ui.statusBar->showMessage(QStringLiteral("��Ȧ��������Ӵ���"));
		return;
	}
	bool cap1 = videoCapture[1]->start();
	if (cap1) {
		imageProcess[1]->start();
		recLabel_pre[1]->setVisible(true);
		recLabel_input[1]->setVisible(true);
	}
	else {
		ui.statusBar->showMessage(QStringLiteral("��Ȧ��������Ӵ���"));
		return;
	}
	if (cap0 && cap1) {
		qDebug("start success");
		ui.statusBar->showMessage(QStringLiteral("���������"));
	}
	/* action */
	ui.action_Start->setEnabled(false);
	ui.action_Stop->setEnabled(true);
	// alarmlight
	ui.alarmPushButton->setEnabled(true);
	bIsRunning = true;
}

void MainWindow::on_action_Stop_triggered()
{
	if (!bIsRunning) {
		QMessageBox::warning(this, QStringLiteral("����"), QStringLiteral("�����Ѿ�ֹͣ�������ظ������������쳣��������"), QStringLiteral("ȷ��"));
		return;
	}
	imageProcess[0]->stop();
	imageProcess[1]->stop();
	plc->stop();
	if (!videoCapture[0]->stop())
		return;
	if (!videoCapture[1]->stop())
		return;
	/* rec label visible */
	recLabel_pre[0]->setVisible(false);
	recLabel_pre[1]->setVisible(false);
	recLabel_input[0]->setVisible(false);
	recLabel_input[1]->setVisible(false);
	/* action */
	ui.action_Start->setEnabled(true);
	ui.action_Stop->setEnabled(false);
	// alarmlight
	ui.alarmPushButton->setEnabled(false);
	bIsRunning = false;
	qDebug("stop success");
	ui.statusBar->showMessage(QStringLiteral("ֹͣ���"));
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
	QString logFilePath = QStringLiteral("%1/%2.log").arg(logDirPath).arg(today);
	QUrl url = QUrl::fromLocalFile(logFilePath);
	QDesktopServices::openUrl(url);
}

void MainWindow::on_action_Backup_Log_triggered()
{
	auto* backupLogDialog = new BackupLogDialog(this);

	backupLogDialog->deleteLater();
	backupLogDialog->exec(); //show
}

void MainWindow::on_action_About_triggered()
{
	QMessageBox::about(this, QStringLiteral("����"),
		QStringLiteral("<h3>%1</h3>"
			"<p>�汾�ţ�<b>%2</b>"
			"<p>%3"
			"<p>��������㽭��ѧ�����������������ϵ%4")
		.arg(PRODUCT_NAME)
		.arg(qApp->applicationVersion())
		.arg(COPYRIGHT)
		.arg(AUTHOR));
}

void MainWindow::on_action_About_Qt_triggered()
{
	qApp->aboutQt();
}

void MainWindow::on_alarmPushButton_clicked()
{
	ui.centralTabWidget->setCurrentIndex(2);
}

void MainWindow::showViewSensorImage()
{
	auto dlg = new ViewSensorImage(this);
	connect(dlg, &ViewSensorImage::finished, dlg, &ViewSensorImage::deleteLater);
	dlg->exec();
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
void MainWindow::setupDataMapper()
{
	// monitorTab
	/* alarm num ui show*/
	connect(ui.alarmLCDBoard, &AlarmLCDBoard::clicked, this, [=]() {
		ui.centralTabWidget->setCurrentIndex(2);
		playBackWidget->selectTableCb->setCurrentIndex(0);
		int row = alarmMapper->currentIndex();
		playBackWidget->alarmView->selectRow(row);
	});
	// �������ݿ�wheels���Model���ֱ�������ʾ����Ȧ
	// ��Ȧ
	outerModel = new QSqlTableModel(this, QSqlDatabase::database(MAIN_CONNECTION_NAME));
	outerModel->setTable("wheels");
	outerModel->setFilter("i_o=0");
	outerModel->select();
	// ��ʾ��Ȧ��model
	innerModel = new QSqlTableModel(this, QSqlDatabase::database(MAIN_CONNECTION_NAME));
	innerModel->setTable("wheels");
	innerModel->setFilter("i_o=1");
	innerModel->select();
	/* ��Model�е�����ӳ�䵽dashboard�� */
	outerMapper = new QDataWidgetMapper(this);
	outerMapper->setModel(outerModel);
	outerMapper->addMapping(ui.numLineEdit_o, Wheel_Num);
	outerMapper->addMapping(ui.lastSpeedLineEdit_o, Wheel_CalcSpeed);
	outerMapper->toLast();
	innerMapper = new QDataWidgetMapper(this);
	innerMapper->setModel(innerModel);
	innerMapper->addMapping(ui.numLineEdit_i, Wheel_Num);
	innerMapper->addMapping(ui.lastSpeedLineEdit_i, Wheel_CalcSpeed);
	innerMapper->toLast();
	/* �����ݿ⡰alarm������е�����ӳ�䵽AlarmNumBoard�� */
	alarmMapper = new QDataWidgetMapper(this);
	alarmMapper->setModel((QSqlTableModel*)(playBackWidget->alarmModel));
	alarmMapper->addMapping(ui.alarmLCDBoard->devName, Wheel_I_O);
	alarmMapper->addMapping(ui.alarmLCDBoard->alarmNum, Wheel_Num);
	alarmMapper->toLast(); //����ı���
	ui.numBackwardBtn->setEnabled(alarmMapper->currentIndex() > 0);
	ui.numForwardBtn->setEnabled(false);
	connect(ui.numBackwardBtn, &QPushButton::clicked, alarmMapper, &QDataWidgetMapper::toPrevious);
	connect(ui.numForwardBtn, &QPushButton::clicked, alarmMapper, &QDataWidgetMapper::toNext);
	connect(alarmMapper, &QDataWidgetMapper::currentIndexChanged, this, [=](int row) {
		ui.numBackwardBtn->setEnabled(row > 0);
		ui.numForwardBtn->setEnabled(row < playBackWidget->alarmModel->rowCount() - 1);
	});
}

void MainWindow::setupDatabaseWatcher()
{
	/* database file watcher */
	dbWatcherThread = new QThread(this);
	watcher = new QFileSystemWatcher;
	watcher->addPath(databaseFilePath);
	watcher->moveToThread(dbWatcherThread);
	//connect(dbWatcherThread, &QThread::finished, watcher, &QFileSystemWatcher::deleteLater); // watcher�ڶ��ϣ���this������Ҫȥ����
	//���model�����ˣ�dataWidgetMapperͬʱ���¡���������model�����ݿ���޸ĵ������Զ��ύ
	connect(watcher, &QFileSystemWatcher::fileChanged, this, [=]() {
		playBackWidget->dbChanged();//ע�⣺�����slot��this���ڵ��߳����������̲߳�����ô��
		alarmMapper->toLast();//���±����ź�������,ˢ�����ҷ�ҳ����ʾ���п��ܱ������
		int row = alarmMapper->currentIndex();
		ui.numBackwardBtn->setEnabled(row > 0);
		ui.numForwardBtn->setEnabled(row < playBackWidget->alarmModel->rowCount() - 1);

		outerModel->select();
		innerModel->select();
		outerMapper->toLast();
		innerMapper->toLast();
	});

	dbWatcherThread->start();
}