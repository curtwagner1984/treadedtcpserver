#include "worker.h"

#include <QTcpSocket>
#include <QHostAddress>
#include <QThread>
#include <QBuffer>

Worker::Worker(qintptr socketHandel, DiskWriter *diskWriter, QObject *parent) : QObject(parent)
{
    this->socketHandel   =  socketHandel;
    this->diskWriter     =  diskWriter;

}

void Worker::init()
{

    this->socket = new QTcpSocket(this);
    this->socket->setSocketDescriptor(this->socketHandel);

    this->currentState = STATE_FIRST_MESSAGE_WAITING_FOR_HEADER; // Initial state machine state when start listening.

    connect(this->socket,&QTcpSocket::readyRead,this,&Worker::onReadReady);
    connect(this->socket,&QTcpSocket::disconnected,this,&Worker::onSocketDisconnected);

    connect(this,&Worker::signalToAdvnaceStateMachine,this,&Worker::advanceStateMachine,Qt::QueuedConnection);
    connect(this,&Worker::addPayloadWriteRequestToWriter,this->diskWriter,&DiskWriter::addPayloadWriteRequestToQueue);


    qDebug() << tr("Thread: %1 : connected to client on %2:%3.").arg(long(QThread::currentThreadId())).arg(this->socket->peerAddress().toString()).arg(this->socket->peerPort());



}


bool Worker::isHeaderChecksumMatch()
{
//    TYPE_HEADER_CHECKSUM checksum = qChecksum((char*)(&this->header) + HEADER_CHECKSUM_OFFSET, HEADER_SIZE - HEADER_CHECKSUM_OFFSET);
//    return this->header.headerCheckSum == checksum;
    return true;
}

bool Worker::isPayloadChecksumMatch()
{
//    return this->header.payloadCheckSum == qChecksum((char*)(&this->payload),PAYLOAD_SIZE);
    return true;
}

bool Worker::canAcceptTransfer()
{
    return true;
}

void Worker::preExit()
{

    qDebug() << tr("Thread: %1 :Socket was disconnected... Exiting...").arg(long(QThread::currentThreadId()));
    this->socket->close();
    this->socket->deleteLater();
    emit this->workFinished();

}

void Worker::sendReplyHeaderToClient(quint16 opCode, quint32 payloadIndex, quint16 payloadCheckSum, quint16 headerStatus)
{

    HeaderStruct headerToFill;

    headerToFill.opCode =          opCode;
    headerToFill.payloadCheckSum = payloadCheckSum;
    headerToFill.payloadIndex =    payloadIndex;
    headerToFill.headerStatus =    headerStatus;
    headerToFill.headerCheckSum =  qChecksum((char*)(&headerToFill) + HEADER_CHECKSUM_OFFSET, HEADER_SIZE - HEADER_CHECKSUM_OFFSET);

    qDebug() << tr("Writing Reply Message To Client: Op Code: %1, Payload Checksum: %2, Payload Index: %3, Header Status: %4, Header Checksum %5 ")
                .arg(headerToFill.opCode).arg(headerToFill.payloadCheckSum).arg(headerToFill.headerStatus).arg(headerToFill.headerCheckSum);


    this->socket->write((char*)&(headerToFill),sizeof(headerToFill));



}



void Worker::doWork()
{
    this->init();
}

void Worker::onReadReady()
{
    emit signalToAdvnaceStateMachine();

}

void Worker::onSocketDisconnected()
{
    qDebug() << tr("Socket Disconnected... Setting 'socketDisconnected = true' ");
    this->socketDisconnected = true;

    if (this->socket->bytesAvailable() == 0){
        this->preExit();
    }


}

void Worker::advanceStateMachine()
{

    qDebug() << tr("Thread: %1 :Got read ready signal from socket...").arg(long(QThread::currentThreadId()));

    qDebug() << tr("Thread: %1 :Number of bytes availabe in the socket is %2").arg(long(QThread::currentThreadId())).arg(this->socket->bytesAvailable());

    int bytesAvaliable = this->socket->bytesAvailable();


    // State machine is wating for headers.
    if(this->currentState == State::STATE_FIRST_MESSAGE_WAITING_FOR_HEADER || this->currentState == State::STATE_RECEIVING_DATA_WAITING_FOR_HEADER ){



        while (false == this->isSeenMagicNumber && this->socket->bytesAvailable() >= HEADER_MAGIC_NUMBER_SIZE){

           quint32 potentialMagicNumber;
           qint64  bytesRead = this->socket->read((char*)&potentialMagicNumber, HEADER_MAGIC_NUMBER_SIZE);

           if (potentialMagicNumber == HEADER_MAGIC_NUMBER){
               this->isSeenMagicNumber = true;
           }
        }



        if (true == this->isSeenMagicNumber && this->socket->bytesAvailable() >= HEADER_SIZE ){

            this->isSeenMagicNumber = false;
            qint64 bytesRead = this->socket->read((char*)&this->header, HEADER_SIZE);

            qDebug() << tr("bytes read:  %1").arg(bytesRead);


            if (isHeaderChecksumMatch()){


                if(this->currentState == State::STATE_FIRST_MESSAGE_WAITING_FOR_HEADER ){
                    if (this->header.opCode == OpCodes::OP_CODE_FIRST_MESSAGE){
                        this->currentState = State::STATE_FIRST_MESSAGE_HEADER_RECIEVED;
                    }else{
                        qDebug() << tr("Incorrect OP CODE, was expecting OP CODE %1 and got OP CODE %2")
                                    .arg(OpCodes::OP_CODE_FIRST_MESSAGE).arg(this->header.opCode);

                        this->sendReplyHeaderToClient(OP_CODE_FIRST_MESSAGE_SERVER_ERROR,0,0,ERROR_INCORRECT_OP_CODE);

                    }

                } else if (this->currentState == State::STATE_RECEIVING_DATA_WAITING_FOR_HEADER ){

                    if (this->header.opCode == OpCodes::OP_CODE_CLIENT_SENDING_DATA){
                        this->currentState = State::STATE_RECEIVING_DATA_HEADER_RECEIVED;
                    }else{
                        qDebug() << tr("Incorrect OP CODE, was expecting OP CODE %1 and got OP CODE %2")
                                    .arg(OpCodes::OP_CODE_CLIENT_SENDING_DATA).arg(this->header.opCode);

                        this->sendReplyHeaderToClient(OP_CODE_SERVER_ERROR,0,0,ERROR_INCORRECT_OP_CODE);

                    }

                }

            }else{

                qDebug () << tr("Header checksum error... Return Error message OP code 03");
                this->sendReplyHeaderToClient(OP_CODE_FIRST_MESSAGE_SERVER_ERROR,0,0,ERROR_CHECKSUM_HEADER);


            }


        }




    }

    if(this->currentState == State::STATE_FIRST_MESSAGE_HEADER_RECIEVED || this->currentState == State::STATE_RECEIVING_DATA_HEADER_RECEIVED ){
        if (this->socket->bytesAvailable() >= PAYLOAD_SIZE ){

            qint64 bytesRead = this->socket->read((char*)&this->payload, PAYLOAD_SIZE); // reads PAYLOAD SIZE bytes of message.

            qDebug() << tr("bytes read:  %1").arg(bytesRead);

            if(isPayloadChecksumMatch()){

                if(this->currentState == State::STATE_FIRST_MESSAGE_HEADER_RECIEVED){
                   qDebug() << tr("Recived first message payload, file size is: %1 , sha checksum is: %2")
                               .arg(this->payload.firstPayload.transferDataSize).arg(payload.firstPayload.sha256);

                   // if can accept this files size, will send messge to client. Else will decline transfer.

                   if (this->canAcceptTransfer()){

                       this->transferDataSize = this->payload.firstPayload.transferDataSize;
                       memcpy(this->sha256,this->payload.firstPayload.sha256,SHA256_SIZE);
                       this->fileIdentifier = this->payload.firstPayload.fileIdentifier;

                       this->currentState = State::STATE_RECEIVING_DATA_WAITING_FOR_HEADER;

                       qDebug() << tr("Can accept transfer, Sending message to client authorzing transfer...");
                       this->sendReplyHeaderToClient(OP_CODE_AUTHORIZE_TRANSFER,this->header.payloadIndex,this->header.payloadCheckSum,ERROR_FILE_SIZE);


                   }else{
                       // if can not accept transfer, return to starting state

                       this->currentState = State::STATE_FIRST_MESSAGE_WAITING_FOR_HEADER;

                       qDebug() << tr("Can not accept transfer, sending message to client declining transfer...");
                       this->sendReplyHeaderToClient(OP_CODE_SERVER_ERROR,this->header.payloadIndex,this->header.payloadCheckSum,ERROR_FILE_SIZE);
                   }


                }else if(this->currentState == State::STATE_RECEIVING_DATA_HEADER_RECEIVED){


                    this->currentState = State::STATE_RECEIVING_DATA_WAITING_FOR_HEADER;

                    PayloadWriteRequest* payloadWriteRequest = (PayloadWriteRequest*)malloc(sizeof(PayloadWriteRequest));

                    payloadWriteRequest->fileIdentifier = this->fileIdentifier;
                    memcpy(&payloadWriteRequest->payload,&this->payload,PAYLOAD_SIZE);
                    payloadWriteRequest->payloadIndex = this->header.payloadIndex;
                    payloadWriteRequest->fileSize = this->transferDataSize;


                    emit this->addPayloadWriteRequestToWriter(payloadWriteRequest);


                }
            }else{
                qDebug() << tr("Payload checksum didn't match...  Sending Error Message to Client...");
                this->sendReplyHeaderToClient(OP_CODE_SERVER_ERROR,this->header.payloadIndex,this->header.payloadCheckSum,ERROR_CHECKSUM_PAYLOAD);


            }

        }



    }
    if (this->socket->bytesAvailable() > 0 && (this->currentState == STATE_RECEIVING_DATA_HEADER_RECEIVED || this->currentState == STATE_RECEIVING_DATA_WAITING_FOR_HEADER)){
        emit signalToAdvnaceStateMachine();
    }else if (this->socketDisconnected){
        this->preExit();
    }


}
