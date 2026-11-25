#include "sdo.hpp"
#include "receiver.hpp"

// Smartly set the SDO object data fields based on the received message.
// Caller is responsible for verifying that recv.id is the correct SDO RX COB-ID.
bool SDO::setResponse(const Receiver &recv) {
    if (recv.dlc != dlc) {
        #ifdef DEBUG
        Serial.println("SDO Update Failed: DLC mismatch");
        #endif
        return false;
    }

    uint16_t recvIndex   = (recv.data[INDEX_HIGH] << 8) | recv.data[INDEX_LOW];
    uint8_t  recvSubIndex = recv.data[SUB_INDEX];

    // Locate the object in the dictionary
    int32_t index = Object::findIndex(recvIndex, recvSubIndex);
    if (index < 0) { // findIndex returns -1 if not found
        return false;
    }
    setObject(index);

    uint8_t recvCmd = recv.data[COMMAND];

    // Fill index / sub-index fields of the response
    data[INDEX_LOW]  = obj->index & 0xFF;
    data[INDEX_HIGH] = obj->index >> 8;
    data[SUB_INDEX]  = obj->subIndex;

    // Determine direction and fill data / command byte
    switch (recvCmd) {
        case WRITE_REQ_1BYTE_CMD:
        case WRITE_REQ_2BYTE_CMD:
        case WRITE_REQ_4BYTE_CMD:
            memset(&data[4], 0, 4); // clear data bytes for write response
            sendType = SendType::WRITE;
            break;

        case READ_REQ_ANY_CMD:
        case READ_REQ_1BYTE_CMD:
        case READ_REQ_2BYTE_CMD:
        case READ_REQ_4BYTE_CMD:
            data[DATA_4] = obj->data[0];
            data[DATA_5] = obj->data[1];
            data[DATA_6] = obj->data[2];
            data[DATA_7] = obj->data[3];
            sendType = SendType::READ;
            break;

        default:
            return false; // unknown command
    }

    data[COMMAND] = getCmd();
    return true;
}
