#include <QtCore/QCoreApplication>

#include "serialserver.h"
#define DEFAULT_PORT 8778
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    int port;
    port = DEFAULT_PORT;
    if(argc >= 2){
        bool ok;
        port = QString(argv[1]).toInt(&ok);
        if(!ok)
            port = DEFAULT_PORT;
 }

    SerialServer *server = new SerialServer(port);


    return a.exec();
}
