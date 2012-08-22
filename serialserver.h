#ifndef SERIALSERVER_H
#define SERIALSERVER_H

#include <QObject>
#include <QList>
#include <QHash>

#include "qtwebsocket/QtWebSocket/QWsServer.h"
#include "qtwebsocket/QtWebSocket/QWsSocket.h"

#include "qextserialenumerator.h"
#include "qextserialport.h"
#include "qt-json/json.h"

using namespace QtJson;

typedef struct {
    QString serial;
    int baudrate;
    StopBitsType stopbits;
    ParityType parity;
    FlowType flowcontrol;
    int databits;
} SerialSettings;


class SerialServer : public QObject
{
    Q_OBJECT

public:

    SerialServer(int port);
    ~SerialServer();
    
signals:

public slots:
    void onClientConnection();
    void onDataReceived(QString data);
    void onPong(quint64 elapsedTime);
    void onClientDisconnection();

    void onReadyRead();

private:
    QVariantMap serialSettingToMap(SerialSettings *settings);

    void sendSerialportList(QWsSocket *sock);
    void sendSupportedConfigs(QWsSocket *sock);
    void sendConfig(QWsSocket *sock, QString serialport);

    int connectSerial(QString serial,
                      int baudrate,
                      StopBitsType stopbits,
                      ParityType parity,
                      FlowType flow,
                      int databits);
    int connectSerial(SerialSettings *settings);
    int connectSerial(QMap<QString, QVariant> *settings);

    QWsServer *server;
    QList<QWsSocket*> clients;

    QList<QSet<QString>*> clientPorts;

    QHash<QString, QextSerialPort*> serialPorts;
    QHash<QString, SerialSettings> serialSettings;


    
};

#endif // SERIALSERVER_H
