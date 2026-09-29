#include "CanOpenNode.h"
#include <Arduino.h>

// Initialize static instance pointer for ISR
CanOpenNode* CanOpenNode::instance = nullptr;

CanOpenNode::CanOpenNode(uint8_t csPin, uint8_t intPin, uint8_t baudRate, uint8_t nodeId) :
    can(10 * 1000 * 1000),
    intPin(intPin),
    baudRate(baudRate),
    nodeId((nodeId >= 1 && nodeId <= 127) ? nodeId : DEFAULT_NODE_ID),
    flagRecv(false),
    nmtState(NMT::Mode::BOOT),
    recv(&can),
    // Temporary placeholders - real COB-IDs & cycle times assigned below
    sdo(0),
    nmt(0, HEARTBEAT_CYCLE_TIME),
    emcy(0),
    txPdo{
        PDO(0, PDO::Type::TX, PDO1_TX_CYCLE_TIME),
        PDO(0, PDO::Type::TX, PDO2_TX_CYCLE_TIME),
        PDO(0, PDO::Type::TX, PDO3_TX_CYCLE_TIME),
        PDO(0, PDO::Type::TX, PDO4_TX_CYCLE_TIME)
    },
    rxPdo{
        PDO(0, PDO::Type::RX, PDO1_RX_CYCLE_TIME),
        PDO(0, PDO::Type::RX, PDO2_RX_CYCLE_TIME),
        PDO(0, PDO::Type::RX, PDO3_RX_CYCLE_TIME),
        PDO(0, PDO::Type::RX, PDO4_RX_CYCLE_TIME)
    },
    nmtOn(true),
    mappingOn(true)
{
    // Calculate all COB-IDs from the runtime Node-ID (CiA 301 pre-defined connection set)
    cobIdEmcy   = BASE_EMCY_ID    + this->nodeId;
    cobIdSdoTx  = BASE_SDO_TX_ID  + this->nodeId;   // slave response
    cobIdSdoRx  = BASE_SDO_RX_ID  + this->nodeId;   // master request
    cobIdNmt    = BASE_NMT_ID     + this->nodeId;   // heartbeat / boot-up

    cobIdTxPdo[0] = BASE_PDO1_TX_ID + this->nodeId;
    cobIdTxPdo[1] = BASE_PDO2_TX_ID + this->nodeId;
    cobIdTxPdo[2] = BASE_PDO3_TX_ID + this->nodeId;
    cobIdTxPdo[3] = BASE_PDO4_TX_ID + this->nodeId;

    cobIdRxPdo[0] = BASE_PDO1_RX_ID + this->nodeId;
    cobIdRxPdo[1] = BASE_PDO2_RX_ID + this->nodeId;
    cobIdRxPdo[2] = BASE_PDO3_RX_ID + this->nodeId;
    cobIdRxPdo[3] = BASE_PDO4_RX_ID + this->nodeId;

    // Assign calculated IDs to the message objects
    sdo.id  = cobIdSdoTx;
    nmt.id  = cobIdNmt;
    emcy.id = cobIdEmcy;

    for (int i = 0; i < 4; ++i) {
        txPdo[i].id = cobIdTxPdo[i];
        rxPdo[i].id = cobIdRxPdo[i];
    }

    // Load cycle times from Object Dictionary (fallback to #defines already set)
    nmt.cycleTime       = getProducerHeartbeatTime();

    const uint16_t txComm[4] = {TX_PDO1_COMM_INDEX, TX_PDO2_COMM_INDEX, TX_PDO3_COMM_INDEX, TX_PDO4_COMM_INDEX};
    const uint16_t rxComm[4] = {RX_PDO1_COMM_INDEX, RX_PDO2_COMM_INDEX, RX_PDO3_COMM_INDEX, RX_PDO4_COMM_INDEX};

    for (int i = 0; i < 4; ++i) {
        txPdo[i].cycleTime        = getPdoEventTimer(txComm[i]);
        txPdo[i].transmissionType = getPdoTransmissionType(txComm[i]);
        txPdo[i].inhibitTime      = getPdoInhibitTime(txComm[i]);

        rxPdo[i].cycleTime        = getPdoEventTimer(rxComm[i]);
        rxPdo[i].transmissionType = getPdoTransmissionType(rxComm[i]);
        rxPdo[i].inhibitTime      = getPdoInhibitTime(rxComm[i]);
    }

    instance = this;
}

// ------------------------------------------------------------------
// Helpers – Object Dictionary access for cycle times
// ------------------------------------------------------------------
uint16_t CanOpenNode::getProducerHeartbeatTime() const {
    int idx = Object::findIndex(PRODUCER_HEARTBEAT_INDEX, SUBIDX_VALUE);
    if (idx < 0) return HEARTBEAT_CYCLE_TIME;
    return static_cast<uint16_t>(dictionary[idx].data[0] |
                                (dictionary[idx].data[1] << 8));
}

uint16_t CanOpenNode::getPdoEventTimer(uint16_t commIndex) const {
    int idx = Object::findIndex(commIndex, SUBIDX_PDO_EVENT_TIMER);   // sub-index 5 = Event Timer
    if (idx < 0) {
        // Fallback to the corresponding #define
        switch (commIndex) {
            case TX_PDO1_COMM_INDEX: return PDO1_TX_CYCLE_TIME;
            case TX_PDO2_COMM_INDEX: return PDO2_TX_CYCLE_TIME;
            case TX_PDO3_COMM_INDEX: return PDO3_TX_CYCLE_TIME;
            case TX_PDO4_COMM_INDEX: return PDO4_TX_CYCLE_TIME;
            case RX_PDO1_COMM_INDEX: return PDO1_RX_CYCLE_TIME;
            case RX_PDO2_COMM_INDEX: return PDO2_RX_CYCLE_TIME;
            case RX_PDO3_COMM_INDEX: return PDO3_RX_CYCLE_TIME;
            case RX_PDO4_COMM_INDEX: return PDO4_RX_CYCLE_TIME;
            default: return 500;
        }
    }
    return static_cast<uint16_t>(dictionary[idx].data[0] |
                                (dictionary[idx].data[1] << 8));
}

uint8_t CanOpenNode::getPdoTransmissionType(uint16_t commIndex) const {
    int idx = Object::findIndex(commIndex, SUBIDX_PDO_TRANSMISSION_TYPE);
    if (idx < 0) return 0xFF;   // default event-driven
    return dictionary[idx].data[0];
}

uint16_t CanOpenNode::getPdoInhibitTime(uint16_t commIndex) const {
    int idx = Object::findIndex(commIndex, SUBIDX_PDO_INHIBIT_TIME);
    if (idx < 0) return 0;
    return static_cast<uint16_t>(dictionary[idx].data[0] |
                                (dictionary[idx].data[1] << 8));
}

void CanOpenNode::updateOdUint8(uint16_t index, uint8_t sub, uint8_t value) {
    int idx = Object::findIndex(index, sub);
    if (idx < 0) return;
    dictionary[idx].data[0] = value;
    dictionary[idx].data[1] = 0;
    dictionary[idx].data[2] = 0;
    dictionary[idx].data[3] = 0;
}

void CanOpenNode::updateOdUint16(uint16_t index, uint8_t sub, uint16_t value) {
    int idx = Object::findIndex(index, sub);
    if (idx < 0) return;
    dictionary[idx].data[0] = value & 0xFF;
    dictionary[idx].data[1] = (value >> 8) & 0xFF;
    dictionary[idx].data[2] = 0;
    dictionary[idx].data[3] = 0;
}

// ------------------------------------------------------------------
// Public setters (also keep OD in sync)
// ------------------------------------------------------------------
void CanOpenNode::setHeartbeatTime(uint16_t ms) {
    nmt.cycleTime = ms;
    updateOdUint16(PRODUCER_HEARTBEAT_INDEX, 0x00, ms);
}

void CanOpenNode::setTxPdoCycleTime(uint8_t num, uint16_t ms) {
    if (num < 1 || num > 4) return;
    txPdo[num - 1].cycleTime = ms;
    static const uint16_t indices[4] = {
        TX_PDO1_COMM_INDEX, TX_PDO2_COMM_INDEX,
        TX_PDO3_COMM_INDEX, TX_PDO4_COMM_INDEX
    };
    updateOdUint16(indices[num - 1], SUBIDX_PDO_EVENT_TIMER, ms);
}

void CanOpenNode::setRxPdoCycleTime(uint8_t num, uint16_t ms) {
    if (num < 1 || num > 4) return;
    rxPdo[num - 1].cycleTime = ms;
    static const uint16_t indices[4] = {
        RX_PDO1_COMM_INDEX, RX_PDO2_COMM_INDEX,
        RX_PDO3_COMM_INDEX, RX_PDO4_COMM_INDEX
    };
    updateOdUint16(indices[num - 1], SUBIDX_PDO_EVENT_TIMER, ms);
}

void CanOpenNode::setPdoTransmissionType(PDO::Type type, uint8_t num, uint8_t transmissionType) {
    if (num < 1 || num > 4) return;
    static const uint16_t txIdx[4] = {TX_PDO1_COMM_INDEX, TX_PDO2_COMM_INDEX, TX_PDO3_COMM_INDEX, TX_PDO4_COMM_INDEX};
    static const uint16_t rxIdx[4] = {RX_PDO1_COMM_INDEX, RX_PDO2_COMM_INDEX, RX_PDO3_COMM_INDEX, RX_PDO4_COMM_INDEX};

    if (type == PDO::Type::TX) {
        txPdo[num - 1].transmissionType = transmissionType;
        updateOdUint8(txIdx[num - 1], SUBIDX_PDO_TRANSMISSION_TYPE, transmissionType);
    } else {
        rxPdo[num - 1].transmissionType = transmissionType;
        updateOdUint8(rxIdx[num - 1], SUBIDX_PDO_TRANSMISSION_TYPE, transmissionType);
    }
}

void CanOpenNode::setPdoInhibitTime(PDO::Type type, uint8_t num, uint16_t inhibitTime) {
    if (num < 1 || num > 4) return;
    static const uint16_t txIdx[4] = {TX_PDO1_COMM_INDEX, TX_PDO2_COMM_INDEX, TX_PDO3_COMM_INDEX, TX_PDO4_COMM_INDEX};
    static const uint16_t rxIdx[4] = {RX_PDO1_COMM_INDEX, RX_PDO2_COMM_INDEX, RX_PDO3_COMM_INDEX, RX_PDO4_COMM_INDEX};

    if (type == PDO::Type::TX) {
        txPdo[num - 1].inhibitTime = inhibitTime;
        updateOdUint16(txIdx[num - 1], SUBIDX_PDO_INHIBIT_TIME, inhibitTime);
    } else {
        rxPdo[num - 1].inhibitTime = inhibitTime;
        updateOdUint16(rxIdx[num - 1], SUBIDX_PDO_INHIBIT_TIME, inhibitTime);
    }
}

// ------------------------------------------------------------------
// Begin / Run
// ------------------------------------------------------------------
void CanOpenNode::begin() {
    can.begin();
    if (MCP2515::ERROR_OK == can.reset()) {
        can.setBitrate(CAN_250KBPS);
        can.setOperatingMode(MCP2515::CAN_MODE_NORMAL);
        can.enableInterrupt(BSP_XL2515_INT_PIN, ISRhandler);
        Serial.println("MCP2515 Initialized Successfully.");
        Serial.print("CANopen Node-ID = 0x");
        Serial.println(nodeId, HEX);
    } else {
        Serial.println("MCP2515 Initialize Failure !!!");
    }
}

// Run function to handle the core communication loop with NMT handling
void CanOpenNode::run() {
    recv.read(&flagRecv);

    if (nmtOn) {
        nmtController();

        switch (nmtState) {
            case NMT::Mode::BOOT:
                sendMsg(nmt);                       // Boot-up message
                nmtState = NMT::Mode::PRE_OPERATIONAL;
                nmt.changeMode(nmtState);
                break;

            case NMT::Mode::PRE_OPERATIONAL:
                if (mappingOn) mapPDOs();
                sdoHandler();
                // Heartbeat is allowed in Pre-Operational (CiA 301)
                heartBeat();
                break;

            case NMT::Mode::OPERATIONAL:
                heartBeat();
                for (int i = 0; i < 4; ++i) {
                    sendPDO(txPdo[i]);
                    receivePDO(rxPdo[i]);
                }
                sdoHandler();
                break;

            case NMT::Mode::STOPPED:
                // Only NMT + Heartbeat are active in Stopped state
                heartBeat();
                break;
        }
    } else {
        // NMT handling disabled � force Operational behaviour
        if (mappingOn) mapPDOs();
        nmt.changeMode(NMT::Mode::OPERATIONAL);
        heartBeat();
        for (int i = 0; i < 4; ++i) {
            sendPDO(txPdo[i]);
            receivePDO(rxPdo[i]);
        }
        sdoHandler();
    }
}

// Manually set PDO mapping
void CanOpenNode::set(PDO::Type type, uint8_t num, uint16_t index, uint8_t subIndex, uint8_t position) {
    PDO* pdo;
    uint8_t pdoNum = num - 1;
    if (pdoNum > 3) return; // PDO num is not between 1 and 4

    if (type == PDO::Type::RX) pdo = &rxPdo[pdoNum];
    else if (type == PDO::Type::TX) pdo = &txPdo[pdoNum];
    else return;

    pdo->set(index, subIndex, position);
    pdo->numObjects = 0;
    // update numObjects count
    for (int i = 0; i < 8; i++) {
        if (pdo->getObject(i) != nullptr) pdo->numObjects++;
    }
}

// Send a CAN message
bool CanOpenNode::sendMsg(const Message &msg) {
    struct can_frame cansendmessageframe;
    cansendmessageframe.can_id  = msg.id;
    cansendmessageframe.can_dlc = msg.dlc;
    memcpy(cansendmessageframe.data, msg.data, 8);
    if (MCP2515::ERROR_OK != can.sendMessage(&cansendmessageframe)) {
        return false;
    }
    return true;
}

// Write data to an object in the Object Dictionary
void CanOpenNode::writeData(Object &obj, uint8_t data[4]) {
    if (obj.access == ObjectPermissions::READ) return;
    memcpy(obj.data, data, obj.getTypeSize());

    // Keep runtime parameters in sync when the corresponding OD entries are written
    if (obj.index == PRODUCER_HEARTBEAT_INDEX && obj.subIndex == SUBIDX_VALUE) {
        nmt.cycleTime = static_cast<uint16_t>(obj.data[0] | (obj.data[1] << 8));
    }
    else if (obj.subIndex == SUBIDX_PDO_TRANSMISSION_TYPE) {
        uint8_t tt = obj.data[0];
        switch (obj.index) {
            case TX_PDO1_COMM_INDEX: txPdo[0].transmissionType = tt; break;
            case TX_PDO2_COMM_INDEX: txPdo[1].transmissionType = tt; break;
            case TX_PDO3_COMM_INDEX: txPdo[2].transmissionType = tt; break;
            case TX_PDO4_COMM_INDEX: txPdo[3].transmissionType = tt; break;
            case RX_PDO1_COMM_INDEX: rxPdo[0].transmissionType = tt; break;
            case RX_PDO2_COMM_INDEX: rxPdo[1].transmissionType = tt; break;
            case RX_PDO3_COMM_INDEX: rxPdo[2].transmissionType = tt; break;
            case RX_PDO4_COMM_INDEX: rxPdo[3].transmissionType = tt; break;
        }
    }
    else if (obj.subIndex == SUBIDX_PDO_INHIBIT_TIME) {
        uint16_t it = static_cast<uint16_t>(obj.data[0] | (obj.data[1] << 8));
        switch (obj.index) {
            case TX_PDO1_COMM_INDEX: txPdo[0].inhibitTime = it; break;
            case TX_PDO2_COMM_INDEX: txPdo[1].inhibitTime = it; break;
            case TX_PDO3_COMM_INDEX: txPdo[2].inhibitTime = it; break;
            case TX_PDO4_COMM_INDEX: txPdo[3].inhibitTime = it; break;
            case RX_PDO1_COMM_INDEX: rxPdo[0].inhibitTime = it; break;
            case RX_PDO2_COMM_INDEX: rxPdo[1].inhibitTime = it; break;
            case RX_PDO3_COMM_INDEX: rxPdo[2].inhibitTime = it; break;
            case RX_PDO4_COMM_INDEX: rxPdo[3].inhibitTime = it; break;
        }
    }
    else if (obj.subIndex == SUBIDX_PDO_EVENT_TIMER) {
        uint16_t value = static_cast<uint16_t>(obj.data[0] | (obj.data[1] << 8));
        switch (obj.index) {
            case TX_PDO1_COMM_INDEX: txPdo[0].cycleTime = value; break;
            case TX_PDO2_COMM_INDEX: txPdo[1].cycleTime = value; break;
            case TX_PDO3_COMM_INDEX: txPdo[2].cycleTime = value; break;
            case TX_PDO4_COMM_INDEX: txPdo[3].cycleTime = value; break;
            case RX_PDO1_COMM_INDEX: rxPdo[0].cycleTime = value; break;
            case RX_PDO2_COMM_INDEX: rxPdo[1].cycleTime = value; break;
            case RX_PDO3_COMM_INDEX: rxPdo[2].cycleTime = value; break;
            case RX_PDO4_COMM_INDEX: rxPdo[3].cycleTime = value; break;
        }
    }
}

// Write data with start position (used by RPDOs)
void CanOpenNode::writeData(Object &obj, uint8_t data[8], uint8_t start) {
    uint8_t size = obj.getTypeSize();
    if (start + size > 8) return;
    memcpy(obj.data, &data[start], size);
}

// Controls NMT state transitions
void CanOpenNode::nmtController() {
    if (recv.id == 0x000) { // NMT Master Command ID
        uint8_t command = recv.data[0];
        uint8_t target  = recv.data[1];

        if (target == nodeId || target == 0x00) { // 0x00 = broadcast
            switch (command) {
                case StartNode:            nmtState = NMT::Mode::OPERATIONAL;     break;
                case StopNode:             nmtState = NMT::Mode::STOPPED;         break;
                case EnterPreOperational:  nmtState = NMT::Mode::PRE_OPERATIONAL; break;
                case ResetNode:
                case ResetCommunication:   nmtState = NMT::Mode::BOOT;            break;
            }
            nmt.changeMode(nmtState);
        }
        recv.clearMsg();
    }
}

// Send an NMT heartbeat message at regular intervals
void CanOpenNode::heartBeat() {
    // According to CiA 301: if 0x1017 == 0 the producer is disabled
    if (nmt.cycleTime == 0) return;

    if ((millis() - nmt.timer) > nmt.cycleTime) {
        sendMsg(nmt);
        nmt.timer = millis();
    }
}

// Map PDOs from the Object Dictionary to the PDO structure
void CanOpenNode::mapPDOArray(PDO* pdoArray, uint16_t baseIndex) {
    for (int i = 0; i < 4; ++i) {
        PDO* pdo = &pdoArray[i];
        if (pdo->id == 0xFFF) continue; // Skip unconfigured PDOs

        uint16_t baseMapIndex = baseIndex + i;

        int mapCountIndex = Object::findIndex(baseMapIndex, SUBIDX_PDO_MAP_COUNT);
        if (mapCountIndex < 0) continue;

        uint8_t numMappedObjects = dictionary[mapCountIndex].data[0];
        pdo->numObjects = numMappedObjects;
        if (numMappedObjects == 0 || numMappedObjects > 8) continue;

        uint8_t currentBytePosition = 0;
        for (uint8_t sub = 1; sub <= numMappedObjects; ++sub) {
            if (currentBytePosition >= PDO_LEN) break;

            int entryIndex = Object::findIndex(baseMapIndex, sub);
            if (entryIndex < 0) continue;

            Object& entry = dictionary[entryIndex];
            uint8_t bitLength  = entry.data[0]; // in bits
            uint8_t subIndex   = entry.data[1];
            uint16_t objIndex  = (entry.data[3] << 8) | entry.data[2];

            uint8_t byteLength = bitLength / 8;

            if (currentBytePosition + byteLength > PDO_LEN) break;

            if (pdo->set(objIndex, subIndex, currentBytePosition)) {
                currentBytePosition += byteLength;
            } else {
                #ifdef DEBUG
                Serial.print("Failed to map object to PDO"); Serial.println(i + 1);
                #endif
                break;
            }
        }
    }
}

// Map Tx and Rx PDOs
void CanOpenNode::mapPDOs() {
    mapPDOArray(txPdo, 0x1A00); // TPDOs
    mapPDOArray(rxPdo, 0x1600); // RPDOs
}

// Send txPDOs at specified cycle times
void CanOpenNode::sendPDO(PDO &txPdo) {
    if (txPdo.type != PDO::Type::TX) return;
    if (txPdo.numObjects == 0) return;

    // Only event-driven / manufacturer event types use the Event Timer path
    // (254/255). Synchronous types would need SYNC handling (not implemented).
    if (txPdo.transmissionType < 254) return;

    if (txPdo.cycleTime == 0) return;   // Event Timer disabled

    uint32_t now = millis();

    // Inhibit time is in units of 100 µs → convert to ms (rounded down)
    uint32_t inhibitMs = txPdo.inhibitTime / 10;
    if (inhibitMs > 0 && (now - txPdo.timer) < inhibitMs) return;

    if ((now - txPdo.timer) >= txPdo.cycleTime) {
        txPdo.updateData();
        sendMsg(txPdo);
        txPdo.timer = now;
    }
}

// Receive rxPDOs and write data to the Object Dictionary
void CanOpenNode::receivePDO(PDO &rxPdo) {
    if (recv.id != rxPdo.id) return;
    if (rxPdo.type != PDO::Type::RX) return;

    for (int i = 0; i < 8; i++) {
        if (rxPdo.getObject(i) != nullptr) {
            writeData(*(rxPdo.getObject(i)), recv.data, i);
        }
    }
    recv.clearMsg();
}

// Updates SDO fields accordingly for requests and responses
void CanOpenNode::sdoHandler() {
    // Only accept SDO requests addressed to this node
    if (recv.id != cobIdSdoRx) return;

    if (sdo.setResponse(recv)) {
        if (sdo.sendType == SDO::SendType::WRITE) {
            // write message to object dictionary
            uint8_t tempData[4] = {
                recv.data[SDO::DATA_4], recv.data[SDO::DATA_5],
                recv.data[SDO::DATA_6], recv.data[SDO::DATA_7]
            };
            Object* obj = sdo.getObject();
            if (obj != nullptr) {
                writeData(*obj, tempData);   // also updates nmt.cycleTime if 0x1017
            }
        }
        if (sendMsg(sdo)) { // send response and clear
            recv.clearMsg();
        }
    }
}

// Interrupt Service Routine logic
void CanOpenNode::MCP2515_ISR() {
    Serial.println("Got frame interrupt.");
    can.handleInterrupt();
    flagRecv = true;
}

void CanOpenNode::ISRhandler() {
    if (instance != nullptr) {
        Serial.println("Set ISRhandler interrupt.");
        instance->MCP2515_ISR();
    }
}
