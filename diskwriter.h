#ifndef DISKWRITER_H
#define DISKWRITER_H

#include <QObject>
#include <QQueue>
#include <QHash>
#include "shared_definitions.h"

class DiskWriter : public QObject
{
    Q_OBJECT
public:
    explicit DiskWriter(QObject *parent = nullptr);

private:

    QQueue <PayloadWriteRequest*> payloadWriteRequests;
    QHash<int, FileWrittenTable*> fileWrittenTables;

    void processQueue();


signals:
    void fileWasWrittenCorrectly();


public slots:

    void doWork();
    void addPayloadWriteRequestToQueue(PayloadWriteRequest* payloadWriteRequest);
};

#endif // DISKWRITER_H
