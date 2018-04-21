#ifndef SHARED_DEFINITIONS_H
#define SHARED_DEFINITIONS_H

#include <QObject>

// github update test

#define TYPE_OPCODE quint16
#define TYPE_HEADER_CHECKSUM quint16
#define TYPE_MAGIC_NUMBER quint32
#define TYPE_PAYLOAD_INDEX quint32
#define TYPE_PAYLOAD_CHECKSUM quint16
#define TYPE_HEADER_STATUS quint16

enum ErrorStatus{
    ERROR_CHECKSUM_HEADER =     1,
    ERROR_CHECKSUM_PAYLOAD =    2,
    ERROR_FILE_SIZE =           3,
    ERROR_INCORRECT_INDEX =     4,
    ERROR_INCORRECT_OP_CODE =   5,
};

enum OpCodes {
    OP_CODE_FIRST_MESSAGE =                         1,
    OP_CODE_FIRST_MESSAGE_SERVER_ERROR =            2,
    OP_CODE_SERVER_ERROR =                          3,
    OP_CODE_CLIENT_SENDING_DATA =                   4,
    OP_CODE_AUTHORIZE_TRANSFER =                    5


};
enum State {
    STATE_FIRST_MESSAGE_WAITING_FOR_HEADER,
    STATE_FIRST_MESSAGE_HEADER_RECIEVED,

    STATE_RECEIVING_DATA_WAITING_FOR_HEADER,
    STATE_RECEIVING_DATA_HEADER_RECEIVED,

    };


typedef struct __attribute__((packed, aligned(4)))
{

    TYPE_HEADER_CHECKSUM    headerCheckSum;
    TYPE_OPCODE             opCode;
    TYPE_PAYLOAD_INDEX      payloadIndex;
    TYPE_PAYLOAD_CHECKSUM   payloadCheckSum;
    TYPE_HEADER_STATUS      headerStatus;


}HeaderStruct;

#define HEADER_SIZE                 (sizeof(HeaderStruct))
#define HEADER_CHECKSUM_SIZE        (sizeof(TYPE_HEADER_CHECKSUM))
#define HEADER_MAGIC_NUMBER_SIZE    (sizeof(TYPE_MAGIC_NUMBER))
#define HEADER_CHECKSUM_OFFSET      (HEADER_CHECKSUM_SIZE)
#define PACKET_SIZE                 (512)
#define PAYLOAD_SIZE                ((PACKET_SIZE) - (HEADER_SIZE + HEADER_MAGIC_NUMBER_SIZE))
#define SHA256_SIZE                 (32)
#define HEADER_MAGIC_NUMBER         (0xDEADBEEF)

#define TYPE_PAYLOAD_TRANSFER_SIZE      quint64
#define TYPE_PAYLOAD_FILE_INDENTIFIER   quint32


typedef struct __attribute__((packed, aligned(4)))
{
    union {
        char payload[PAYLOAD_SIZE];
        struct
        {
            TYPE_PAYLOAD_TRANSFER_SIZE transferDataSize;
            TYPE_PAYLOAD_FILE_INDENTIFIER fileIdentifier;
            char sha256[SHA256_SIZE];
            char garbageLeftoverFromPayload[PAYLOAD_SIZE - sizeof(TYPE_PAYLOAD_TRANSFER_SIZE) - SHA256_SIZE - sizeof(TYPE_PAYLOAD_FILE_INDENTIFIER)];

        }firstPayload;
    };

}PayloadStruct;

typedef struct __attribute__((packed, aligned(4)))
{
    bool*   allWritten;
    quint32 size;
    quint32 notWritten;

}FileWrittenTable;



typedef struct __attribute__((packed, aligned(4)))
{

    TYPE_PAYLOAD_INDEX              payloadIndex;
    TYPE_PAYLOAD_FILE_INDENTIFIER   fileIdentifier;
    PayloadStruct                   payload;
    TYPE_PAYLOAD_TRANSFER_SIZE      fileSize;

}PayloadWriteRequest;




#endif // SHARED_DEFINITIONS_H
