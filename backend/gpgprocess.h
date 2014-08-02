#ifndef PGPPROCESS_H
#define PGPPROCESS_H

#include <QObject>
#include <QPointer>

class PGPKey;
class QProcess;

class PGPProcess : public QObject
{
    Q_OBJECT
public:
    explicit PGPProcess(QObject *parent = 0);
    int importKey(const QString& fileName);
private:
    void initProcess();

    QPointer<QProcess> process_;
};

#endif
