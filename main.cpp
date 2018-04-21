#include <QCoreApplication>
#include "server.h"


#include <QDebug>
#include <QObject>

// Protocol Diagram link: https://docs.google.com/drawings/d/1JFnB76e7N34f9wLJvcDptR2KQLBF7b04gCru0ogJ6kU/edit?usp=sharing



int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Server *server = new Server();
    server->startServer();


    return a.exec();
}
