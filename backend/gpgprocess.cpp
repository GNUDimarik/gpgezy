#include "gpgprocess.h"
#include <QProcess>
#include <QFileInfo>
#include <QDebug>
#include <QFile>

#define PROGRAM_NAME "gpg"

PGPProcess::PGPProcess(QObject *parent) :
    QObject(parent)
{
}

int PGPProcess::importKey(const QString& fileName)
{
    initProcess();
    QStringList args;
    args << "--import" << fileName;
    process_->start(PROGRAM_NAME, args);
    process_->waitForFinished();
    return process_->exitCode();
}

void PGPProcess::initProcess()
{
    if (process_.isNull()) {
        process_ = new QProcess(this);
        process_->setProcessChannelMode(QProcess::MergedChannels);
    }
}

