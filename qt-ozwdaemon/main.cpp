#include <QCoreApplication>
#include <QLoggingCategory>
#include <QCommandLineParser>
#include "qtozwdaemon.h"
#include "mqttpublisher.h"
#include <qt-openzwave/qt-openzwavedatabase.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("ozwdaemon");
    QCoreApplication::setApplicationVersion("0.1");
    QCoreApplication::setOrganizationName("OpenZWave");
    QCoreApplication::setOrganizationDomain("openzwave.com");

    QCommandLineParser parser;
    parser.setApplicationDescription("QT OpenZWave Remote Daemon");
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption serialPort(QStringList() << "s" << "serial-port",
        "Serial Port of USB Stick",
        "serialPort"
    );

    parser.addOption(serialPort);

    QCommandLineOption configDir(QStringList() << "c" << "config-dir",
        "Directory containing the OZW Config Files",
        "configDir"
    );

    parser.addOption(configDir);

    QCommandLineOption userDir(QStringList() << "u" << "user-dir",
        "Directory for the OZW User Files",
        "userDir"
    );

    parser.addOption(userDir);

    QCommandLineOption MQTTServer(QStringList() << "mqtt-server",
        "MQTT Server Hostname/IP Address",
        "IP/Hostname"
    );

    parser.addOption(MQTTServer);

    QCommandLineOption MQTTPort(QStringList() << "mqtt-port",
        "MQTT Server Port",
        "Port"
    );


    parser.addOption(MQTTPort);


    parser.process(a);
    if (!parser.isSet(serialPort)) {
        fputs(qPrintable("Serial Port is Required\n"), stderr);
        fputs("\n\n", stderr);
        fputs(qPrintable(parser.helpText()), stderr);
        exit(-1);
    }
    QSettings settings;
    if (parser.isSet(MQTTServer)) {
        settings.setValue("MQTTServer", parser.value(MQTTServer));
    }
    if (parser.isSet(MQTTPort)) {
        settings.setValue("MQTTPort", parser.value(MQTTPort).toInt());
    }

#if 0
    QLoggingCategory::setFilterRules("qt.remoteobjects.debug=true\n"
                                     "qt.remoteobjects.warning=true\n"
                                     "qt.remoteobjects.models.debug=true\n"
                                     "qt.remoteobjects.models.debug=true\n"
                                     "qt.remoteobjects.io.debug=true\n"
                                     "default.debug=true");
#else
    QLoggingCategory::setFilterRules("default.debug=true");
#endif

    QStringList PossibleDBPaths;
    if (parser.isSet(configDir))
        PossibleDBPaths << parser.value(configDir);
    PossibleDBPaths << "./config/";
    PossibleDBPaths << settings.value("openzwave/ConfigPath", QDir::toNativeSeparators("../../../config/")).toString().append("/");
    PossibleDBPaths << QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);

    QString path, dbPath, userPath;
    foreach(path, PossibleDBPaths) {
        qDebug() << "Checking " << QFileInfo(QDir::toNativeSeparators(path+"/config/manufacturer_specific.xml")).absoluteFilePath() << " for manufacturer_specific.xml";
        if (QFileInfo(QDir::toNativeSeparators(path+"/config/manufacturer_specific.xml")).exists()) {
            dbPath = QFileInfo(QDir::toNativeSeparators(path+"/config/manufacturer_specific.xml")).absoluteFilePath();
            break;
        }
        qDebug() << "Checking " << QFileInfo(QDir::toNativeSeparators(path+"../config/manufacturer_specific.xml")).absoluteFilePath() << " for manufacturer_specific.xml";
        if (QFile(QDir::toNativeSeparators(path+"/../config/manufacturer_specific.xml")).exists()) {
            dbPath = QFileInfo(QDir::toNativeSeparators(path+"/../config/manufacturer_specific.xml")).absoluteFilePath();
            break;
        }
    }
    PossibleDBPaths.clear();
    if (parser.isSet(userDir))
        PossibleDBPaths << parser.value(userDir);
    PossibleDBPaths << "./config/";
    PossibleDBPaths << settings.value("openzwave/UserPath", QDir::toNativeSeparators("../../../config/")).toString().append("/");
    PossibleDBPaths << QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);

    foreach(path, PossibleDBPaths) {
        qDebug() << "Checking " << QFileInfo(QDir::toNativeSeparators(path+"/config/Options.xml")).absoluteFilePath() << " for options.xml";
        if (QFileInfo(QDir::toNativeSeparators(path+"/config/options.xml")).exists()) {
            userPath = QFileInfo(QDir::toNativeSeparators(path+"/config/options.xml")).absoluteFilePath();
            break;
        }
        qDebug() << "Checking " << QFileInfo(QDir::toNativeSeparators(path+"/../config/options.xml")).absoluteFilePath() << " for options.xml";
        if (QFile(QDir::toNativeSeparators(path+"/../config/options.xml")).exists()) {
            userPath = QFileInfo(QDir::toNativeSeparators(path+"/../config/options.xml")).absoluteFilePath();
            break;
        }
    }

//    if (userPath.isEmpty()) {
//        fputs(qPrintable("userPath is Not Set or Missing\n"), stderr);
//        exit(-1);
//    }
    if (dbPath.isEmpty()) {
        copyConfigDatabase(QFileInfo("./").absoluteFilePath().append("/"));
        dbPath = "./config/";
        userPath = "./config/";
    }
    qDebug() << "DBPath: " << dbPath;
    qDebug() << "userPath: " << userPath;


    QTOZWOptions options(QTOZWOptions::Local);
    options.setUserPath(userPath);
    options.setConfigPath(dbPath);



    qtozwdaemon daemon;
    mqttpublisher mqttpublisher;
    mqttpublisher.setOZWDaemon(&daemon);
    daemon.setSerialPort(parser.value(serialPort));
    daemon.startOZW();
    return a.exec();
}
