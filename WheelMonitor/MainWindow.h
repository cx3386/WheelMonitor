#pragma once

#include "MyMessageOutput.h"
#include "HikVideoCapture.h"
#include "ImageProcess.h"
#include "PLCSerial.h"
#include "ui_MainWindow.h"
#include <QtWidgets/QMainWindow>
#include <ocr.h>

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = Q_NULLPTR);
	~MainWindow();
	static bool bAppAutoRun;
	static bool bVerboseLog;

	//void myMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg);

private:
	Ui::MainWindowClass ui;
	bool bIsRunning;
	QLabel *recLabel;
	/*****worker******/
	MyMessageOutput *outputMessage;
	ImageProcess *imageProcess;
	HikVideoCapture *videoCapture;
	PLCSerial *plcSerial;
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

	void clearLog(int nDays);  //�������n�����־
	bool delCapDir(int nDays); //�������n���¼���ļ�
	bool makeDir(QString fullPath);
	void appAutoRun(bool bAutoRun);

	QFrame *lineL;
	QFrame *lineR;
	QFrame *lineT;
	QFrame *lineB;
	//for test
	//QPushButton *startSaveBtn;
	//QPushButton *stopSaveBtn;

protected:
	void closeEvent(QCloseEvent *event);
	//void resizeEvent(QResizeEvent *event);

	public slots:
	void uiAlarmLight(PLCSerial::AlarmColor alarmColor);
	void uiAlarmNum(const QString &num);
	void uiShowMatches();
	void uiShowLastSpeed(double speed);
	void uiShowRtSpeed(double speed);
	void uiShowCartSpeed(double speed);
	void uiShowLogMessage(const QString &message);
	void uiShowErrorMessage(const QString &message);
	//roi area
	void drawRoiArea();

	private slots:
	void anchorClickedSlot(const QUrl &url);
	void update24();
	void on_action_Start_triggered();
	void on_action_Stop_triggered();
	void on_action_Restart_triggered();
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
	bool isConnectPLC(bool r);
	bool isDisconnectPLC(bool r);

	void startOrStopSave();

signals:
	void installLogSystem();
	void startCap(HWND handle);
	void SyncCameraTime();
	void stopCap();
	void startProcess();
	void stopProcess();
	//void startSave(); //2017.11.10 start imgprocess whatever the save conseq is.
	//void stopSave();
	void initPlcSerial();
	void connectPLC();
	void disconnectPLC();
	void setAlarm(PLCSerial::AlarmColor alarmcolor);
};
