#ifndef SERVER_H
#define SERVER_H

#include "diskwriter.h"
#include <QTcpServer>

class Server: public QTcpServer
{
    Q_OBJECT
public:
    explicit Server(QTcpServer *parent = nullptr);
    void startServer();
    // QTcpServer interface

private:
    DiskWriter* diskWriter;

protected:
    void incomingConnection(qintptr handle);
};

#endif // SERVER_H
