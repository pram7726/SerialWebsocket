#include "serialserver.h"

SerialServer::SerialServer(int port)
{
    server = new QWsServer(this);

    if(server->listen(QHostAddress::Any, port))
    {
        qDebug()<<"Listening on port: " << port;
    } else
    {
        qDebug()<<"Could not start Websocket Server\n\t";
        qDebug()<<server->errorString();
    }

    connect(server, SIGNAL(newConnection()), this, SLOT(onClientConnection()));

}

SerialServer::~SerialServer()
{
    server->close();
    server->deleteLater();

    foreach(QWsSocket *s, clients){
        s->close();
        s->deleteLater();
    }
    clients.clear();

    foreach (QSet<QString> *l, clientPorts){
        l->clear();
    }
    clientPorts.clear();

    foreach (QextSerialPort *s, serialPorts){
        s->close();
        s->deleteLater();
    }
    serialPorts.clear();

    serialSettings.clear();

}

QVariantMap SerialServer::serialSettingToMap(SerialSettings *settings){
    QVariantMap ret;
    ret["serial"]       = settings->serial;
    ret["baudrate"]     = settings->baudrate;
    ret["databits"]     = settings->databits;

    switch (settings->flowcontrol){
    case FLOW_HARDWARE:
        ret["flowcontrol"]  = "HARDWARE";
        break;
    case FLOW_XONXOFF:
        ret["flowcontrol"]  = "XONXOFF";
        break;
    case FLOW_OFF:
        ret["flowcontrol"]  = "OFF";
        break;
    }
    switch(settings->parity){
    case PAR_NONE:
        ret["parity"]  = "NONE";
        break;
    case PAR_EVEN:
        ret["parity"]  = "EVEN";
        break;
    case PAR_ODD:
        ret["parity"]  = "ODD";
        break;
    case PAR_SPACE:
        ret["parity"]  = "SPACE";
        break;
    #ifdef Q_OS_WIN
    case PAR_MARK:
        ret["parity"]  = "MARK";
        break;
    #endif
    }

    switch(settings->stopbits){
    case STOP_1:
        ret["stopbits"]  = "1";
        break;
    case STOP_2:
        ret["stopbits"]  = "2";
        break;
    #ifdef Q_OS_WIN
    case STOP_1_5:
        ret["stopbits"]  = "1.5";
        break;
    #endif
    }


    return ret;
}

int SerialServer::connectSerial(SerialSettings *settings){
    return this->connectSerial(
                settings->serial,
                settings->baudrate,
                settings->stopbits,
                settings->parity,
                settings->flowcontrol,
                settings->databits);
}

int SerialServer::connectSerial(QString serial,
                                int baudrate,
                                StopBitsType stopbits,
                                ParityType parity,
                                FlowType flow,
                                int databits)
{

    if (serialPorts.contains(serial)){
        // Selected serialport is already connected
        return 2;
    } else {
        QextSerialPort *port = new QextSerialPort(serial, QextSerialPort::EventDriven);
        port->setBaudRate((BaudRateType) baudrate);
        port->setFlowControl(flow);
        port->setParity(parity);
        port->setDataBits((enum DataBitsType) databits);
        port->setStopBits(stopbits);

        if (port->open(QIODevice::ReadWrite)){
            QObject::connect(port, SIGNAL(readyRead()), this, SLOT(onReadyRead()));

            serialPorts.insert(serial.toUpper(), port);
            return 1;
        } else {
            return -1;
        }


    }
}

int SerialServer::connectSerial(QMap<QString, QVariant> *settings){
    SerialSettings serialSetting;
    bool ok;

    serialSetting.baudrate = settings->value("baudrate").toInt(&ok);
    if (!ok)
        serialSetting.baudrate = 57600;


    serialSetting.databits = settings->value("databits").toInt(&ok);
    if (!ok)
        serialSetting.databits = 8;

    serialSetting.serial       = settings->value("serial").toString();

    QString parity      = settings->value("parity").toString().toUpper();
    QString stopbits    = settings->value("stopbits").toString().toUpper();
    QString flowcontrol = settings->value("flowcontrol").toString().toUpper();

    serialSetting.parity        = PAR_NONE;
    serialSetting.stopbits      = STOP_1;
    serialSetting.flowcontrol   = FLOW_OFF;

    if (parity == "ODD")
        serialSetting.parity = PAR_ODD;
    else if (parity == "EVEN")
        serialSetting.parity = PAR_EVEN;
    else if (parity == "SPACE")
         serialSetting.parity = PAR_SPACE;
#if defined(Q_OS_WIN)
    else if (parity == "MARK")
        serialSetting.parity = PAR_MARK;
#endif

    if (flowcontrol == "HARDWARE")
        serialSetting.flowcontrol = FLOW_HARDWARE;
    else if (flowcontrol == "XONXOFF")
        serialSetting.flowcontrol = FLOW_XONXOFF;


    if (stopbits == "2")
        serialSetting.stopbits = STOP_2;
#if defined(Q_OS_WIN)
    else if (stopbits == "1.5")
        serialSetting.stopbits = STOP_1_5;
#endif

    return this->connectSerial(&serialSetting);

}

void SerialServer::sendSerialportList(QWsSocket *sock){
    QVariantList portsJ;

    QList<QextPortInfo>  ports = QextSerialEnumerator::getPorts();
    foreach(QextPortInfo port, ports)
    {
        QVariantMap p;
        p["port"]   = port.portName;
        p["desc"]   = port.friendName;
        portsJ.append(p);
    }

    QVariantMap json;
    json["type"] = "serialPorts";
    json["data"] = portsJ;
    QString js(Json::serialize(json));
    sock->write(js);

}

void SerialServer::sendSupportedConfigs(QWsSocket *sock){
    QVariantMap json;
    QVariantMap data;
    QVariantList parity;
    QVariantList stopbits;
    QVariantList databits;
    QVariantList flowcontrol;

    stopbits << "1" << "2";
    parity << "ODD" << "EVEN" << "SPACE" << "NONE";
    databits << 5 << 6 << 7 << 8;
    flowcontrol << "HARDWARE" << "XONXOFF" << "OFF";

#if defined(Q_OS_WIN)
    stopbits << "1.5";
    parity << "MARK";
#endif

    data["parity"] = parity;
    data["stopbits"] = stopbits;
    data["databits"] = databits;
    data["flowcontrol"] = flowcontrol;

    json["type"] = "supportedConfiguration";
    json["data"] = data;

    QString js(Json::serialize(json));

    sock->write(js);

}

void SerialServer::onClientConnection()
{
    QWsSocket *client = (QWsSocket*) server->nextPendingConnection();

    QObject *clientObject = qobject_cast<QObject*>(client);

    connect(clientObject, SIGNAL(frameReceived(QString)), this, SLOT(onDataReceived(QString)));
    connect(clientObject, SIGNAL(disconnected()), this, SLOT(onClientDisconnection()));
    connect(clientObject, SIGNAL(pong(quint64)), this, SLOT(onPong(quint64)));

    clients.append(client);
    clientPorts.append( new QSet<QString>() );

    qDebug() << "Client nr #" << clients.length() << "connected";

}


void SerialServer::onReadyRead(){
    int nrOfSerialPorts = serialPorts.count();
    int nrOfClients = clientPorts.count();

    for(int i=0; i<nrOfSerialPorts; ++i){
        QextSerialPort *port = serialPorts.value(serialPorts.keys().value(i));

        QByteArray bytes;
        int bytesInQue = port->bytesAvailable();
        bytes.resize(bytesInQue);
        port->read(bytes.data(), bytes.size());

        QVariantMap serialData;
        serialData["serial"] = port->portName();
        serialData["base"] = QString(bytes).toAscii().toBase64();

        QVariantMap js;
        js["type"] = "serialData";
        js["data"] = serialData;

        QString json = Json::serialize(js);

        for(int c=0; c<nrOfClients; ++c){
            if(clientPorts.at(c)->contains(port->portName())){
                clients.at(c)->write(json);

            }
        }


    }

}

void SerialServer::onDataReceived(QString dataIn)
{
    QWsSocket* socket = qobject_cast<QWsSocket*>( sender() );

    if (socket == 0)
        return;

    bool ok;
    QVariantMap result = Json::parse(dataIn, ok).toMap();
    if (!ok)
        return;

    if (result.contains("type"))
    {
        QString cmd = result.value("type").toString();

        if( cmd == "listSerialPorts") {
            sendSerialportList(socket);

        } else if (cmd == "connect") {

            QMap<QString, QVariant> settings = result.value("data").toMap();
            QString port      = settings.value("serial").toString();
            int s = this->connectSerial(&settings);

            QVariantMap data;
            data["type"] = "connectResponse";
            data["serial"] = port;
            data["code"] = s;

            switch (s){
            case -1:
                data["error"] = "Could not Connect to serialport";
                break;
            case 1:
                data["error"] = "None";
                break;
            case 2:
                data["error"] = "Port was already conected";
                if(serialSettings.contains(port))
                    data["settings"] = serialSettingToMap(& serialSettings[port]);
                break;
            }

            socket->write(Json::serialize(data));

            if(s)
                clientPorts.at( clients.indexOf(socket) )->insert(port);

            // Connect
        } else if (cmd == "disconnect"){
            QMap<QString, QVariant> settings = result.value("data").toMap();
            QString port      = settings.value("serial").toString().toUpper();
            qDebug() << "Disconnect: " << port;


            // Remove from clients list
            clientPorts.at( clients.indexOf(socket) )->remove(port);

            // Check if someone else is listening on that port
            bool disconnect = true;
            int nrOfClients = clientPorts.count();

            for (int i=0; i<nrOfClients; ++i){
                if(clientPorts.at(i)->contains(port)){
                    disconnect = false;
                    continue;
                }
            }

            // No one was listening, lets close it
            if (disconnect && serialPorts.contains(port)){

                qDebug() << "No one is listening on: "<<port<< " lets close it";
                serialPorts[port]->close();
                serialPorts.remove(port);
            }

        // Disconnect
        } else if (cmd == "serial"){
            QMap<QString, QVariant> settings = result.value("data").toMap();
            QString port      = settings.value("portname").toString();
            QByteArray data = QByteArray::fromBase64(settings.value("data").toByteArray());

            if (serialPorts.contains(port)){
                serialPorts.value(port)->write(data);
            }
            // Data for serialport
        } else if (cmd == "supportedConfiguration"){

            sendSupportedConfigs(socket);
        }

    }
}

void SerialServer::onPong(quint64 elapsedTime)
{
    qDebug() << "ping: " << QString::number(elapsedTime) << " ms";
}

void SerialServer::onClientDisconnection()
{
    QWsSocket * socket = qobject_cast<QWsSocket*>(sender());
    if (socket == 0)
        return;

    int i = clients.indexOf(socket);
    // Remove clients ports, and client
    clients.removeAt(i);
    clientPorts.removeAt(i);

    socket->deleteLater();

    qDebug() << "Client disconnected (" << clients.length() << " clients left)";
}

