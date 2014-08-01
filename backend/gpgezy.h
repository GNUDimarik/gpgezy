#ifndef GPGEZY_H
#define GPGEZY_H

#include <QObject>

class Gpgezy : public QObject
{
    Q_OBJECT
public:
    explicit Gpgezy(QObject *parent = 0);
public Q_SLOTS:
    void showUsage();
    void doWork(const QStringList& args);
    void start();
    QString checkIsKeyFileKeyringAndAddifNot(const QString& fileName, bool showMessageIfKeyAlreadyAdded = false);
private:
    void setReturnStatus(int status);
};

#endif // GPGEZY_H
