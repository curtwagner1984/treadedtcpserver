#include "diskwriter.h"
#include <QFile>
#include <QDebug>

DiskWriter::DiskWriter(QObject *parent) : QObject(parent)
{

}

void DiskWriter::processQueue()
{

    while(false == this->payloadWriteRequests.isEmpty()){

        PayloadWriteRequest *currentWriteRequest = this->payloadWriteRequests.dequeue();

        // /home/alexds9/projects/treadedtcpserver/
        QFile file(QString("%1.txt").arg(currentWriteRequest->fileIdentifier));

        if (!file.open(QIODevice::ReadWrite)){
             //flag and free later
        }

        file.seek(currentWriteRequest->payloadIndex * PAYLOAD_SIZE);
        if(-1 == (file.write((char*)(&currentWriteRequest->payload),PAYLOAD_SIZE))){
                //flag and free later
        }

        if(false == this->fileWrittenTables.contains(currentWriteRequest->fileIdentifier)){
            FileWrittenTable* fileWrittenTable =(FileWrittenTable*)malloc(sizeof(FileWrittenTable));

            fileWrittenTable->size = currentWriteRequest->fileSize / PAYLOAD_SIZE;
            if (currentWriteRequest->fileSize % PAYLOAD_SIZE > 0){
                fileWrittenTable->size++;
            }

            fileWrittenTable->notWritten = fileWrittenTable->size;
            fileWrittenTable->allWritten = (bool*)malloc(sizeof(bool) * fileWrittenTable->size);
            for (int i = 0; i < fileWrittenTable->size; i++){
                fileWrittenTable->allWritten[i] = false;
            }

            fileWrittenTables[currentWriteRequest->fileIdentifier] = fileWrittenTable;
        }


        FileWrittenTable* fileWrittenTable =  this->fileWrittenTables[currentWriteRequest->fileIdentifier];

        if (false == fileWrittenTable->allWritten[currentWriteRequest->payloadIndex])
        {
            fileWrittenTable->allWritten[currentWriteRequest->payloadIndex] = true;
            fileWrittenTable->notWritten--;
        }

        if ( 0 == fileWrittenTable->notWritten )
        {
            // TODO: send signal to worker

            // free memory
            free(fileWrittenTable->allWritten);
            fileWrittenTable->allWritten = NULL;

            free(fileWrittenTable);
            fileWrittenTable = NULL;

            this->fileWrittenTables.remove(currentWriteRequest->fileIdentifier);
        }

        free(currentWriteRequest);
        currentWriteRequest = NULL;

        file.close();

    }

}

void DiskWriter::doWork()
{
    qDebug() << tr("Do work function in DiskWriter was called ...");
    this->processQueue();

}

void DiskWriter::addPayloadWriteRequestToQueue(PayloadWriteRequest *payloadWriteRequest)
{
    qDebug() << tr("Add Payload write request was called...");
    this->payloadWriteRequests.append(payloadWriteRequest);
    this->processQueue();

}
