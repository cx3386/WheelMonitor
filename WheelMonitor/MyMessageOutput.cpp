#include "stdafx.h"
#include "MyMessageOutput.h"
#include "common.h"

MyMessageOutput* MyMessageOutput::pMyMessageOutput = new MyMessageOutput;	//need init

void myMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
	static QMutex mutex;
	mutex.lock();
	QString txt;
	QString uiLogMessage;
	QString uiErrorMessage;
	//output the warning message, i.e., when find a wheel stucked!
	//QString context_info = QString("File:(%1) Line:(%2)").arg(QString(context.file)).arg(context.line);
	QString current_date_time = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss"); //ddd is weekday
	QString current_date = QStringLiteral("[%1]").arg(current_date_time);
	switch (type) {
	case QtDebugMsg:
		txt = QStringLiteral("Debug:");
		break;
	case QtInfoMsg:
		txt = QStringLiteral("Info:");
		break;
	case QtWarningMsg:
		txt = QStringLiteral("Warning:");
		//uiLogMessage = QStringLiteral("%1\r\n%2").arg(current_date).arg(msg);
		MyMessageOutput::pMyMessageOutput->emit logMessage(uiLogMessage);
		//ui.logTextBrowser->moveCursor(QTextCursor::End);
		break;
	case QtCriticalMsg:
		txt = QStringLiteral("Critical:");
		//criticalTxt = QStringLiteral("");<a href = \"%2\">Video</a>videoCapture->saveDirLink
		uiErrorMessage = QStringLiteral("%1 %2").arg(current_date).arg(msg);
		MyMessageOutput::pMyMessageOutput->emit errorMessage(uiErrorMessage);
		//ui.errorTextBrowser->moveCursor(QTextCursor::End);
		break;
	case QtFatalMsg:
		txt = QStringLiteral("Fatal:");
		abort();
		//break;
	}
	QString message = QStringLiteral("%1 %2 %3").arg(current_date).arg(txt).arg(msg);

	//QFile outfile(QCoreApplication::applicationDirPath().append("/log.txt"));
	QString logName = QDate::currentDate().toString("yyyyMMdd");
	QFile outfile(QStringLiteral("%1/%2.log").arg(logDirPath).arg(logName));
	outfile.open(QIODevice::WriteOnly | QIODevice::Append);
	QTextStream text_stream(&outfile);
	text_stream << message << "\r\n";
	outfile.flush();
	outfile.close();
	mutex.unlock();
}

MyMessageOutput::MyMessageOutput(QObject *parent)
	: QObject(parent)
{
}

MyMessageOutput::~MyMessageOutput()
{
}

void MyMessageOutput::installMesageHandler()
{
	qInstallMessageHandler(myMessageOutput);
}