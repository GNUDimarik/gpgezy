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
    QString addKey(const QString& fileName);
private:
    void setReturnCode(int status);
    void showMessageDiagnostingText(const QString& text);
};

#endif // GPGEZY_H
