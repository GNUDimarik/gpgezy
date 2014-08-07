#include <QApplication>
#include "gpgezy.h"
#include "constants.h"
#include <QtCrypto>
#include <QDebug>
#include <QTimer>

#if QT_VERSION < 0x050000
#include <QTextCodec>
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCA::Initializer init;
    Q_UNUSED(init)

#if QT_VERSION < 0x050000
    //QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
  //  QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-16"));
//    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-16"));
#endif

    if (!QCA::isSupported("openpgp")) {
        qDebug() << "OPENPGP is not supported!";
        return EXIT_CODE_OPENPGP_IS_NOT_SUPORTED;
    }

    a.setApplicationName("GPGEZY");
    Gpgezy* gpgezy = new Gpgezy(&a);;
    QTimer::singleShot(2, gpgezy, SLOT(start()));
    return a.exec();
}
