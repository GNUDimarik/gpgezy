#include "gpgezy.h"
#include "constants.h"
#include "environment.h"
#include "gpgprocess.h"
#include <memory>
#include <QCoreApplication>
#include <QStringList>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QtCrypto>
#include <QDesktopServices>
#include <QUrl>
#include <QDebug>
#include <QTextCodec>
#include <iostream>
#include <string>

Gpgezy::Gpgezy(QObject *parent) :
    QObject(parent)
{
    QDir dataDir(Environment::getDataDirectory());

    if (!dataDir.exists()) {
        if (!dataDir.mkdir(dataDir.absolutePath()))
            qDebug() << "Can't create data dir";
    }
}

void Gpgezy::showUsage()
{
    qDebug() << "--addkey key file name" << endl
             << "--encrypt file names  --keyid your key id or --keyname key file name" << endl
             << "--decrypt file names" << endl
             << "--export-key --keyid your key id --keyname key file name, key type --public or --private" << endl
             << "--create-key";
}

void Gpgezy::doWork(const QStringList& args)
{

    for (QStringList::const_iterator current = args.begin(); current != args.end(); ++ current) {

        if (*current == "--addkey") {
            ++ current;

            if (current->startsWith("--")) {
                qDebug() << "unrecognized command option" << *current;
                setReturnCode(EXIT_CODE_INVALID_ARGUMENT);
            }

            if (current == args.end())
                setReturnCode(EXIT_CODE_INVALID_ARGUMENT);

            while (current != args.end()) {
                QString fileName = *current;

                if (current == args.end() || fileName.isEmpty())
                    setReturnCode(EXIT_CODE_INVALID_ARGUMENT);

                if (!QFileInfo(fileName).exists()) {
                    qDebug() << "File " << fileName << "not exists";
                    setReturnCode(EXIT_CODE_INVALID_ARGUMENT);
                }

                QCA::PGPKey key(fileName);

                if (key.isNull()) {
                    qDebug() << "Key is null";
                    setReturnCode(EXIT_CODE_INVALID_ARGUMENT);
                }

                addKey(fileName);
                ++ current;
            }

            setReturnCode(EXIT_CODE_SUCCESS);
        } // --addkey

        else if (*current == "--encrypt") {
            ++ current;

            if (current == args.end())
                setReturnCode(EXIT_CODE_INVALID_ARGUMENT);

            QStringList files;
            QString keyId;
            QString keyFileName;
            QCA::KeyStoreEntry store_entry;
            QCA::KeyStoreManager *ksm = new QCA::KeyStoreManager(this);
            ksm->start();
            ksm->waitForBusyFinished();

            while (current != args.end()) {

                if (*current == "--keyname") {
                    ++ current;

                    if (current == args.end())
                        setReturnCode(EXIT_CODE_INVALID_ARGUMENT);

                    if (QFileInfo(*current).exists())
                        keyFileName = *current;

                    else {
                        qDebug() << "File " << *current << "not exists";
                        setReturnCode(EXIT_CODE_INVALID_ARGUMENT);
                    }
                }

                else if (*current == "--keyid") {
                    ++ current;

                    if (current == args.end())
                        setReturnCode(EXIT_CODE_INVALID_ARGUMENT);

                    keyId = *current;
                }

                else {

                    if (current->startsWith("--")) {
                        qDebug() << "unrecognized command option" << *current;
                        setReturnCode(EXIT_CODE_INVALID_ARGUMENT);
                    }

                    if (QFileInfo(*current).exists()) {

                        if (!files.contains(*current))
                            files << *current;
                        else
                            qDebug() << "file" << *current << "already in list";
                    }
                    else {
                        qDebug() << "File " << *current << "not exists";
                        setReturnCode(EXIT_CODE_INVALID_ARGUMENT);
                    }
                }

                ++ current;
            }

            if (keyId.isEmpty() && keyFileName.isEmpty()) {
                qDebug() << "Can't decrypt without key";
                setReturnCode(EXIT_CODE_INVALID_ARGUMENT);
            }

            QCA::KeyStore key_store( QString("qca-gnupg"), ksm );

            if (keyId.isEmpty() && !keyFileName.isEmpty()) {

                QCA::PGPKey key(keyFileName);

                if (!key.isNull())
                   keyId = key_store.writeEntry(key);
                else {                                        
                    qDebug() << "Key from" << keyFileName << "is null";                    
                    showMessageDiagnostingText(ksm->diagnosticText());
                    setReturnCode(EXIT_CODE_INVALID_ARGUMENT);
                }
            }

            Q_FOREACH(const QCA::KeyStoreEntry store_key,  key_store.entryList()) {

                if (store_key.id() == keyId) {
                    store_entry = store_key;
                    break;
                }
            }

            if (store_entry.isNull()) {
                qDebug() << "Invalid key";
                setReturnCode(EXIT_CODE_INVALID_ARGUMENT);
            }

            QCA::SecureMessageKey to;
            QCA::setProperty("pgp-always-trust", true);
            to.setPGPPublicKey(store_entry.pgpPublicKey());
            to.setPGPSecretKey(store_entry.pgpSecretKey());
            QCA::OpenPGP pgp;
            QCA::SecureMessage msg(&pgp);

            for (current = files.begin(); current != files.end(); ++ current) {
                QFile file(*current);
                QString outputFileName = QFileInfo(*current).absolutePath() + QDir::separator() +
                        QFileInfo(*current).fileName() + '.'+  gpgezy::encrypted_files_suffix;

                if (file.open(QIODevice::ReadOnly)) {
                    msg.setRecipient(to);
                    msg.setFormat(QCA::SecureMessage::Binary);
                    msg.startEncrypt();
                    msg.update(file.readAll());
                    msg.end();
                    msg.waitForFinished(-1);

                    if (msg.success()) {
                        QFile outputFile(outputFileName);

                        if (outputFile.open(QIODevice::WriteOnly)) {
                            outputFile.write(msg.read());
                            outputFile.flush();
                        }
                        else
                            qDebug() << "Can't open file" << outputFileName << "for write";
                    }
                    else
                        showMessageDiagnostingText(msg.diagnosticText());
                }

                else
                    qDebug() << "Can't open file" << *current << "for read";
            }

            setReturnCode(EXIT_CODE_SUCCESS);

        } // —encrypt

        else if (*current == "--decrypt") {
            ++ current;

            if (current == args.end())
                setReturnCode(EXIT_CODE_INVALID_ARGUMENT);

            QStringList files;
            bool openDecryptedFiles = true;
            bool overwriteExistingFiles = false;

            while (current != args.end()) {

                if (*current == "--donotview")
                    openDecryptedFiles = false;

                else if (*current == "--overwrite")
                    overwriteExistingFiles = true;

                else if (current->startsWith("--")) {
                    qDebug() << "unrecognized command option" << *current;
                    setReturnCode(EXIT_CODE_INVALID_ARGUMENT);
                }

                else {
                    QFileInfo fi(*current);

                    if (fi.exists()) {

                        if (fi.suffix() == gpgezy::encrypted_files_suffix) {

                            if (!files.contains(*current))
                                files << *current;
                            else
                                qDebug() << "file" << *current << "already in list";
                        }
                        else
                            qDebug() << "Only " << gpgezy::encrypted_files_suffix << "can be encrypted, file"
                                     << *current << "not added in list";
                    }

                    else
                        qDebug() << "file" << *current << "not exists";

                }

                ++ current;
            }

            QCA::OpenPGP pgp;
            QCA::setProperty("pgp-always-trust", true);
            QCA::SecureMessage msg(&pgp);

            Q_FOREACH(QString fileName, files) {
                QFile inputFile(fileName);

                if (inputFile.open(QIODevice::ReadOnly)) {
                    msg.setFormat(QCA::SecureMessage::Binary);
                    msg.startDecrypt();
                    msg.update(inputFile.readAll());
                    msg.end();
                    msg.waitForFinished(-1);

                    if (msg.success()) {
                        QFileInfo fi(fileName);
                        QString fileName = fi.absoluteFilePath().remove('.' + fi.suffix());
                        QString outputFileName = fileName;

                        if (!overwriteExistingFiles) {

                            QFileInfo finfo(fileName);
                            int i = 1;

                            while (finfo.exists()) {
                                QString baseName = fi.baseName();
                                baseName += QString("_%1").arg(i ++);
                                QString fname = finfo.fileName();
                                fname = fname.replace(finfo.baseName(), baseName);
                                finfo = QFileInfo(finfo.absolutePath() + QDir::separator() + fname);
                            }

                            outputFileName = finfo.absoluteFilePath();
                        }

                        QFile outputFile(outputFileName);

                        if (outputFile.open(QIODevice::WriteOnly)) {
                            QByteArray ba = msg.read();
                            qint64 ret = outputFile.write(ba.constData(), ba.length());

                            if (ret > -1) {
                                if (openDecryptedFiles)
                                    QDesktopServices::openUrl(QUrl::fromLocalFile(outputFileName));
                            }
                            else
                                qDebug() << "error occured during writing in output file" << outputFile.errorString();
                        }

                        else
                            qDebug() << "can't open file" << outputFile.fileName() << "for write";
                    }

                    else
                        showMessageDiagnostingText(msg.diagnosticText());
                }

                else
                    qDebug() << "can't open file" << fileName << "for read";
            }

            setReturnCode(EXIT_CODE_SUCCESS);

        } // --decrypt

        else if (*current == "--export-key") {

        } // --export-key

        else if (*current == "--create-key") {

        } // --create-key
    }

    setReturnCode(EXIT_CODE_INVALID_ARGUMENT);
}

void Gpgezy::start()
{
    doWork(qApp->arguments());
}

QString Gpgezy::addKey(const QString& fileName)
{
    QCA::PGPKey key(fileName);

    if (key.isNull())
        return QString();

    QCA::KeyStoreManager::start();
    QCA::KeyStoreManager ksm(this);
    ksm.waitForBusyFinished();
    QCA::KeyStore key_store( QString("qca-gnupg"), &ksm );
    QString str = key_store.writeEntry(key);
    PGPProcess process;
    process.importKey(fileName);
    qDebug() << "Key "  << str << "successfully added";
    return str;
}

// pivate members

void Gpgezy::setReturnCode(int status)
{
    if (status != EXIT_CODE_SUCCESS)
        showUsage();

    exit(status);
}

void Gpgezy::showMessageDiagnostingText(const QString& text)
{    
#ifndef Q_OS_WIN
    QByteArray ba = text.toLatin1();
    qDebug() << QString::fromUtf8(ba.constData(), ba.size());
#else
    qDebug() << text;
#endif

    QFile file("C:\\binaries\\binaries\\log.txt");

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream.setCodec(QTextCodec::codecForName("IBM 850"));
        stream << text;
    }

    else
        qDebug() << "Can't create log file";
}
