#pragma once

#include "MyMessageOutput.h"
#include "HikVideoCapture.h"
#include "ImageProcess.h"
#include "PLCSerial.h"
#include "ui_MainWindow.h"
#include <QtWidgets/QMainWindow>



class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	MainWindow(QWidget* parent = Q_NULLPTR);
	~MainWindow();
	
	//void myMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg);

private:
	Ui::MainWindowClass ui;
	bool bRunningState;
	QLabel *recLabel;
	/*****worker******/
	MyMessageOutput* outputMessage; 
	ImageProcess* imageProcess;
	HikVideoCapture* videoCapture;
	PLCSerial* plcSerial;
	/***work thread***/
	QThread videoCaptureThread;
	QThread imageProcessThread;
	QThread plcSerialThread;
	QThread outputMessageThread;

	HWND realPlayHandle;
	QMutex mutex;
	void configWindow();
	void readSettings();
	void writeSettings();
	
	
	void clearLog(int nDays);	//保留最近n天的日志
	bool delCapDir(int nDays);	//保留最近n天的录像文件
	bool makeDir(QString fullPath);
	//for test
	//QPushButton *startSaveBtn;
	//QPushButton *stopSaveBtn;

protected:
	void closeEvent(QCloseEvent * event);

public slots:
	void uiAlarmLight(PLCSerial::AlarmColor alarmColor);
	void uiShowMatches();
	void uiShowLastSpeed(double speed);
	void uiShowRealSpeed(double speed);
	void uiShowLogMessage(const QString &message);
	void uiShowErrorMessage(const QString &message);

private slots:
	void anchorClickedSlot(const QUrl& url);
	void update24();
	void on_action_Start_triggered();
	void on_action_Stop_triggered();
	void on_action_Property_triggered();
	void on_action_Quit_triggered();
	void on_action_About_triggered();
	void on_alarmPushButton_clicked();
	void on_errorTextBrowser_textChanged();
	void on_logTextBrowser_textChanged();
	bool isStartCap(bool result);
	bool isStopCap(bool result);
	void onRecStart();
	void onRecStop();
	bool isStartWheelSensor(bool r);
	bool isStopWheelSensor(bool r);

	//void startOrStopSave();

signals:
	void installLogSystem();
	void startCap(HWND handle);
	void stopCap();
	void startProcess();
	void stopProcess();
	//void startSave(); //自动认为save阶段不会出错，不需要做处理 
	//void stopSave();
	void initPlcSerial();
	void startWheelSensor();
	void stopWheelSensor(); 
	void setAlarm(const char* lightcolor);
};
