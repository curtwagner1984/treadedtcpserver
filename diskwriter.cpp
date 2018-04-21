#include "diskwriter.h"
#include <QFile>
#include <QDebug>

DiskWriter::DiskWriter(QObject *parent) : QObject(parent)
{

}

void DiskWriter::processQueue()
{

    while(false == this->payloadWriteRequests.isEmpty()){

        bool writeSuccess = true;

        PayloadWriteRequest *currentWriteRequest = this->payloadWriteRequests.dequeue();

        // /home/alexds9/projects/treadedtcpserver/
        QFile file(QString("%1.txt").arg(currentWriteRequest->fileIdentifier));

        if (file.open(QIODevice::ReadWrite)){

            file.seek(currentWriteRequest->payloadIndex * PAYLOAD_SIZE);
            if(-1 == (file.write((char*)(&currentWriteRequest->payload),PAYLOAD_SIZE))){

                writeSuccess = false; //failed to write to file
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

            if((fileWrittenTable->size == (currentWriteRequest->payloadIndex - 1)) && currentWriteRequest->fileSize % PAYLOAD_SIZE > 0){
                // This is the last chunk and it can't be devided equally by PAYLOAD_SIZE, so we only need to write the reminder of the devision and not the whole PAYLOAD SIZE

                file.seek(currentWriteRequest->payloadIndex * PAYLOAD_SIZE);
                if(-1 == (file.write((char*)(&currentWriteRequest->payload),currentWriteRequest->fileSize % PAYLOAD_SIZE))){

                    writeSuccess = false; //failed to write to file
                }

            }else{

                // This is a regular chunk, so we write the whole PAYLOAD_SIZE

                file.seek(currentWriteRequest->payloadIndex * PAYLOAD_SIZE);
                if(-1 == (file.write((char*)(&currentWriteRequest->payload),currentWriteRequest->fileSize % PAYLOAD_SIZE))){

                    writeSuccess = false; //failed to write to file
                }

            }

            if (false == fileWrittenTable->allWritten[currentWriteRequest->payloadIndex] && true == writeSuccess )
            {
//                fileWrittenTable->writtenChunksTable->replace(currentWriteRequest->payloadIndex,true);
                fileWrittenTable->allWritten[currentWriteRequest->payloadIndex] = true;
                fileWrittenTable->notWritten--;
            }

            if ( 0 == fileWrittenTable->notWritten )
            {

                //All parts was written


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
