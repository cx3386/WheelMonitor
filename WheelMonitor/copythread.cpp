#include "copythread.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>

CopyThread::CopyThread()
{
}

int CopyThread::copyStart()
{
	// loading filelist
	QString fileType;
	QString fileName;
	QDir fileDir;
	QFileInfoList fileInfoList;
	QFileInfo fileInfo;
	int i = 0;

	// check path
	fileInfo.setFile(srcPath);
	if (!fileInfo.isDir())
		return ERROR_SRC_PATH_NULL;
	fileInfo.setFile(desPath);
	if (!fileInfo.isDir())
		return ERROR_DES_PATH_NULL;

	fileList.clear();
	fileTotalSize = 0;
	curSize = 0;

	fileDir.setPath(srcPath);
	fileDir.setFilter(QDir::Files);         // ֻѡ���ļ������Ĺ��˵�
	fileInfoList = fileDir.entryInfoList(); // ��ȡ�ļ���Ϣ�б�

	do
	{
		fileInfo = fileInfoList.at(i);
		fileType = fileInfo.fileName().split(".").last();

		if (fileType == "jpg" || fileType == "avi") // jpg��avi
		{
			fileTotalSize += fileInfo.size() / 1024;
			fileName = srcPath + "/" + fileInfo.fileName();
			fileList << fileName;
		}
		else
		{
			fileInfoList.removeOne(fileInfo);
			continue;
		}
		i++;
	} while (i < fileInfoList.size());

	if (0 == fileList.count())
		return ERROR_NO_FILES;

	bStop = false;
	emit copyStationSig(COPY_START, NULL);
	this->start();
	emit copyStationSig(COPY_STOP, NULL);
	return SUCCESS;
}

void CopyThread::copyStop()
{
	bStop = true;
}

void CopyThread::run()
{
	int i;
	QString fileName;

	for (i = 0; i < fileList.count(); ++i)
	{
		fileName = fileList.at(i);
		emit copyStationSig(COPY_FILE_NAME, fileName.split("/").last());
		if (ERROR_MEM_FULL == fileCopy(fileName))
		{
			bStop = true;
			emit copyStationSig(COPY_ERROR_MEM_FULL, NULL);
			this->exit();
			return;
		}

		if (bStop)
			break;
	}

	emit copyStationSig(COPY_STOP, fileName);
}

quint64 CopyThread::dirFileSize(const QString& path)
{
	quint64 size = 0;
	QDirIterator it(path, QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
	while (it.hasNext()) {
		//qDebug() << it.next();
		QFileInfo info = it.fileInfo();
		size += info.size();
	}
	return size;
}

int CopyThread::fileCopy(QString fileName)
{
	QFileInfo fileInfo;
	QString desFileName;
	QByteArray byteArray;
	unsigned long fileSize;
	QString qsStaText;
	unsigned long count = 0;
	unsigned long freeSpace = 5 * 1024 * 1024;

	desFileName = desPath + "/" + fileName.split("/").last();

	fileInfo.setFile(fileName);
	fileSize = fileInfo.size() / 1024;

	if (fileSize > freeSpace)
		return ERROR_MEM_FULL;

	fileInfo.setFile(desFileName);
	if (fileInfo.isFile()) // ������ɾ��
	{
		qDebug() << desFileName << "���ļ��Ѵ���";
		QFile::remove(desFileName);
	}

	// ����Ŀ���ļ�
	QFile desFile(desFileName);
	desFile.open(QIODevice::WriteOnly);

	// ��ԭ�ļ�
	QFile srcFile(fileName);
	srcFile.open(QIODevice::ReadOnly);

	while (!srcFile.atEnd())
	{
		count++;
		byteArray = srcFile.read(1024);
		desFile.write(byteArray);

		qsStaText = QString::number(100 * count / fileSize);
		emit copyStationSig(COPY_SINGLE_RATE, qsStaText);

		qsStaText = QString::number(100 * (curSize + count) / fileTotalSize);
		emit copyStationSig(COPY_TOTAL_RATE, qsStaText);

		if (bStop)
		{
			desFile.close();
			srcFile.close();
			QFile::remove(desFileName);
			return SUCCESS;
		}
	}

	desFile.close();
	srcFile.close();

	curSize += fileSize;

	return SUCCESS;
}