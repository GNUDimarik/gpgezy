#include <QCoreApplication>
#include "gpgezy.h"
#include "constants.h"
#include <QtCrypto>
#include <QDebug>
#include <QTimer>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCA::Initializer init;
    Q_UNUSED(init)

    if (!QCA::isSupported("openpgp")) {
        qDebug() << "OPENPGP is not supported!";
        return EXIT_CODE_OPENPGP_IS_NOT_SUPORTED;
    }

    a.setApplicationName("GPGEzy");
    Gpgezy* gpgezy = new Gpgezy(&a);;
    QTimer::singleShot(2, gpgezy, SLOT(start()));
    //QMetaObject::invokeMethod(gpgezy, "doWork", Q_ARG(QStringList, qApp->arguments()));
    return a.exec();
}
