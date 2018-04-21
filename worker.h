#ifndef WORKER_H
#define WORKER_H


#include <QObject>
#include <QTcpSocket>

#include "shared_definitions.h"
#include "diskwriter.h"


class Worker : public QObject

{


    Q_OBJECT
public:
    explicit Worker(qintptr socketHandel, DiskWriter *diskWriter, QObject *parent = nullptr);
    void init();

private:

    qintptr         socketHandel;
    QTcpSocket      *socket;
    State           currentState;

    HeaderStruct    header;
    PayloadStruct   payload;

    DiskWriter      *diskWriter;

    TYPE_PAYLOAD_TRANSFER_SIZE      transferDataSize;
    char sha256[SHA256_SIZE];
    TYPE_PAYLOAD_FILE_INDENTIFIER   fileIdentifier;

    bool socketDisconnected = false;

//    void messageHandler(QByteArray recievedByteArray);

//    HeaderStruct bytesToHeader(QByteArray recivedHeaderByteArray);

//    PayloadStructOpOne bytesToOpOnePayload(QByteArray recivedPayloadBytes);

//    quint16 getChecksumForByteArray(QByteArray byteArrayToCheck);

    bool isHeaderChecksumMatch();

    bool isPayloadChecksumMatch();

    bool isSeenMagicNumber = false;

    bool canAcceptTransfer();

    void preExit();

    void sendReplyHeaderToClient(TYPE_OPCODE opCode, TYPE_PAYLOAD_INDEX payloadIndex,TYPE_PAYLOAD_CHECKSUM payloadCheckSum,
    TYPE_HEADER_STATUS headerStatus);



signals:
    void workFinished();
    void signalToAdvnaceStateMachine();
    void addPayloadWriteRequestToWriter(PayloadWriteRequest* payloadRequest);

public slots:
    void doWork();

private slots:
    void onReadReady();
    void onSocketDisconnected();
    void advanceStateMachine();
};

#endif // WORKER_H
