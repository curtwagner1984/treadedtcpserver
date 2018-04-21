#include "server.h"
#include "worker.h"



#include <QDebug>
#include <QThread>


Server::Server(QTcpServer *parent) : QTcpServer(parent)
{
    this->diskWriter = new DiskWriter();

    QThread *thread = new QThread();

    this->diskWriter->moveToThread(thread);

//    connect(thread,&QThread::started,worker,&Worker::doWork);
//    connect(worker,&Worker::workFinished,thread,&QThread::quit);
//    connect(thread,&QThread::finished,worker,&Worker::deleteLater);
//    connect(thread,&QThread::finished,thread,&QThread::deleteLater);

    thread->start();



}

void Server::startServer()
{

    qDebug() << tr("Thread %1: Starting server ...").arg(long(QThread::currentThreadId()));


    if(!this->listen(QHostAddress::LocalHost,4000)){
        qDebug() << tr("Server could not start because '%1' ").arg(this->errorString());
    }

    qDebug() << tr("Server started and is listening at %1:%2").arg(this->serverAddress().toString()).arg(this->serverPort());

}


void Server::incomingConnection(qintptr handle)
{

    qDebug() << tr("Thread %1: incomingConnection funciton in Server has been called....").arg(long(QThread::currentThreadId()));


    QThread *thread = new QThread();

    Worker *worker = new Worker(handle,this->diskWriter);

    worker->moveToThread(thread);

    connect(thread,&QThread::started,worker,&Worker::doWork);
    connect(worker,&Worker::workFinished,thread,&QThread::quit);
    connect(thread,&QThread::finished,worker,&Worker::deleteLater);
    connect(thread,&QThread::finished,thread,&QThread::deleteLater);



    thread->start();


}
