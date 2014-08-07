#-------------------------------------------------
#
# Project created by QtCreator 2014-07-23T12:19:45
#
#-------------------------------------------------

QT       += core gui
TARGET = gpgezy
CONFIG   -= app_bundle
CONFIG   += crypto
INCLUDEPATH += C:/qca-win32/qca/include/QtCrypto
TEMPLATE = app
windows:LIBS += -ladvapi32
windows:CONFIG += console


LIBS += -LC:/qca-2.0.3-win32/lib -lqca2

SOURCES += \
    gpgezy.cpp \
    environment.cpp \
    gpgprocess.cpp \
    main.cpp

HEADERS += \
    gpgezy.h \
    constants.h \
    environment.h \
    gpgprocess.h

INCLUDEPATH += . build/moc
MOC_DIR = build/moc
UI_DIR  = build/ui
OBJECTS_DIR = build/obj
RCC_DIR = build/obj

RESOURCES +=

OTHER_FILES += \
    database.sql \
    files_table.sql \
    qca/README
