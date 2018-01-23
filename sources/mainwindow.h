#pragma once

#include "ui_MainWindow.h"
#include <QtWidgets/QMainWindow>
#include <ocr.h>
#include "plcserial.h"

class PlayBackWidget;
class SettingDialog;
class ImageProcess;
class HikVideoCapture;
class PLCSerial;
class QThread;
class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	MainWindow(QWidget *parent = Q_NULLPTR);
	~MainWindow();
	static bool bAppAutoRun;
	static bool bVerboseLog;

	//static int const EXIT_CODE_REBOOT;
	//void myMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg);

private:

	Ui::MainWindowClass ui;
	bool bIsRunning;
	QLabel *recLabel_pre; //in preview tab
	QLabel *recLabel_input; //in realframe
	/*****worker******/
	//MyMessageOutput *outputMessage;
	ImageProcess *imageProcess;
	HikVideoCapture *videoCapture;
	PLCSerial *plcSerial;
	/***work thread***/
	QThread *videoCaptureThread;
	QThread *imageProcessThread;
	QThread *plcSerialThread;
	QThread *dbWatcher;
	PlayBackWidget * playBackWidget;

	//QThread outputMessageThread;

	SettingDialog* settingDialog;
	HWND realPlayHandle;
	QMutex mutex;
	void configWindow();
	void readSettings();
	void writeSettings();

	//void clearLog(int nDays);  //retain nDays log file //deprecated 2017/12/11
	bool cleanDir(QString dir, int nDays); //retain nDays save files, include video/match/log and so on, defualt retain for 0 days, which means clean all
	void appAutoRun(bool bAutoRun);

	//for test
	//QPushButton *startSaveBtn;
	//QPushButton *stopSaveBtn;

protected:
	void closeEvent(QCloseEvent *event) override;
	//void resizeEvent(QResizeEvent *event);

	private slots :

	void uiAlarmLight(AlarmColor alarmColor);
	void uiShowAlarmNum(const QString &num);
	void uiShowRealtimeImage();
	void uiShowMatches();
	void uiShowWheelSpeed(double speed);
	void uiShowWheelNum(const QString &s);
	void uiShowCartSpeed(double speed);
	//void anchorClickedSlot(const QUrl &url);
	void start24timer();
	void update24();
	void on_action_Start_triggered();
	void on_action_Stop_triggered();
	void on_action_Restart_triggered();
	void on_action_Property_triggered();
	void on_action_Quit_triggered();
	void on_action_Show_Log_triggered();
	void on_action_Backup_Log_triggered();
	void on_action_About_triggered();
	void on_alarmPushButton_clicked();
	//void on_errorTextBrowser_textChanged();
	//void on_logTextBrowser_textChanged();
	bool isStartCap(bool result);
	bool isStopCap(bool result);
	void onRecStart();
	void onRecStop();
	bool isConnectPLC(bool r);
	bool isDisconnectPLC(bool r);

	void startOrStopSave();

signals:
	//void installLogSystem();
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
	void setAlarm(AlarmColor alarmcolor);
};
