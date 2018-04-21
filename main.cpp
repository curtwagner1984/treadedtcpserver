#include <QCoreApplication>
#include "server.h"


#include <QDebug>
#include <QObject>





int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Server *server = new Server();
    server->startServer();


    return a.exec();
}
