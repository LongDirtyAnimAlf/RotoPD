#include "CanOpenNode.h"
#include <Arduino.h>

// Initialize static instance pointer for ISR
CanOpenNode* CanOpenNode::instance = nullptr;

CanOpenNode::CanOpenNode(uint8_t csPin, uint8_t intPin, uint8_t baudRate, uint8_t nodeId) :
    can(10 * 1000 * 1000),
    intPin(intPin),
    baudRate(baudRate),
    nodeId((nodeId >= 1 && nodeId <= 127) ? nodeId : DEFAULT_NODE_ID),
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
    mappingOn(true),
    pdoMapped(false)
{
    // Calculate all COB-IDs from the runtime Node-ID (CiA 301 pre-defined connection set)
    cobIdEmcy   = CO_CAN_ID_EMERGENCY    + this->nodeId;
    cobIdSdoTx  = CO_CAN_ID_SDO_SRV  + this->nodeId;   // slave response
    cobIdSdoRx  = CO_CAN_ID_SDO_CLI  + this->nodeId;   // master request
    cobIdNmt    = CO_CAN_ID_HEARTBEAT     + this->nodeId;   // heartbeat / boot-up

    cobIdTxPdo[0] = CO_CAN_ID_TPDO_1 + this->nodeId;
    cobIdTxPdo[1] = CO_CAN_ID_TPDO_2 + this->nodeId;
    cobIdTxPdo[2] = CO_CAN_ID_TPDO_3 + this->nodeId;
    cobIdTxPdo[3] = CO_CAN_ID_TPDO_4 + this->nodeId;

    cobIdRxPdo[0] = CO_CAN_ID_RPDO_1 + this->nodeId;
    cobIdRxPdo[1] = CO_CAN_ID_RPDO_2 + this->nodeId;
    cobIdRxPdo[2] = CO_CAN_ID_RPDO_3 + this->nodeId;
    cobIdRxPdo[3] = CO_CAN_ID_RPDO_4 + this->nodeId;

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
    bool messageavailable = recv.read();

    if (nmtOn) {

        if (messageavailable) nmtController();

        switch (nmtState) {
            case NMT::Mode::BOOT:
                sendMsg(nmt);                       // Boot-up message
                nmtState = NMT::Mode::PRE_OPERATIONAL;
                nmt.changeMode(nmtState);
                pdoMapped = false;                 // Trigger re-map on boot transition
                break;

            case NMT::Mode::PRE_OPERATIONAL:
                if (mappingOn && !pdoMapped) {
                    mapPDOs();
                }
                // Handle SDO Read (Upload Request)
                if (messageavailable) sdoHandler();
                // Heartbeat is allowed in Pre-Operational (CiA 301)
                heartBeat();
                break;

            case NMT::Mode::OPERATIONAL:
                if (mappingOn && !pdoMapped) {
                    mapPDOs();
                }
                heartBeat();
                for (int i = 0; i < 4; ++i) {
                    sendPDO(txPdo[i]);
                    if (messageavailable) receivePDO(rxPdo[i]);
                }
                // Handle SDO Read (Upload Request)
                if (messageavailable) sdoHandler();
                break;

            case NMT::Mode::STOPPED:
                // Only NMT + Heartbeat are active in Stopped state
                heartBeat();
                break;
        }
    } else {
        // NMT handling disabled - force Operational behaviour
        if (mappingOn && !pdoMapped) {
            mapPDOs();
        }
        nmt.changeMode(NMT::Mode::OPERATIONAL);
        heartBeat();
        for (int i = 0; i < 4; ++i) {
            sendPDO(txPdo[i]);
            if (messageavailable) receivePDO(rxPdo[i]);
        }
        // Handle SDO Read (Upload Request)
        if (messageavailable) sdoHandler();
    }

    if (messageavailable) 
    {
        if ( (nmtState == NMT::Mode::OPERATIONAL) || (nmtState == NMT::Mode::PRE_OPERATIONAL) )
        {
            // We might not have handle the message
            // Delete it always in these modes to prevent endless loops and CPU burn
            recv.clearMsg();
        }
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
    for (int i = 0; i < CO_PDO_MAX_SIZE; i++) {
        if (pdo->getObject(i) != nullptr) pdo->numObjects++;
    }

    // Keep Object Dictionary Sub-Index 0 (Map Count) in sync
    uint16_t baseMapIndex = (type == PDO::Type::TX) ? (TX_PDO1_MAPPING_INDEX + pdoNum) 
                                                    : (RX_PDO1_MAPPING_INDEX + pdoNum);
    updateOdUint8(baseMapIndex, SUBIDX_PDO_MAP_COUNT, pdo->numObjects);
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

    // Invalidate PDO mapping cache if mapping objects are modified
    bool isRxMapping = (obj.index >= RX_PDO1_MAPPING_INDEX && obj.index <= RX_PDO4_MAPPING_INDEX);
    bool isTxMapping = (obj.index >= TX_PDO1_MAPPING_INDEX && obj.index <= TX_PDO4_MAPPING_INDEX);

    if (isRxMapping || isTxMapping) {
        // Drop the cached mapping status flag so mapPDOs() re-evaluates the array next loop
        pdoMapped = false;

        // If the master writes to sub-index 0 (Map Count), sync the matching runtime PDO object count
        if (obj.subIndex == SUBIDX_PDO_MAP_COUNT) {
            uint8_t pdoNum = 0;
            if (isTxMapping) {
                pdoNum = obj.index - TX_PDO1_MAPPING_INDEX;
                if (pdoNum < 4) txPdo[pdoNum].numObjects = obj.data[0];
            } else {
                pdoNum = obj.index - RX_PDO1_MAPPING_INDEX;
                if (pdoNum < 4) rxPdo[pdoNum].numObjects = obj.data[0];
            }
        }
    }

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

    static bool toggleBit = true;

    if (recv.id == CO_CAN_ID_NMT_SERVICE) { // NMT Master Command ID
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
    else
    {
        bool ext = (recv.id & CAN_EFF_FLAG);
        bool rtr = (recv.id & CAN_RTR_FLAG);
        uint32_t id = (recv.id & (ext ? CAN_EFF_MASK : CAN_SFF_MASK));

        if (rtr){

            if (id == cobIdNmt){ // NMT Node Guarding Request ; Uses an RTR frame
                // Send back the NMT Mode
                Message nmtmsg;
                nmtmsg.id = id;
                nmtmsg.dlc = CO_NMT_MAX_SIZE;
                nmtmsg.data[0] = static_cast<uint8_t>(nmtState);             
                toggleBit = !toggleBit;
                if (toggleBit) nmtmsg.data[0] += 0x80;
                sendMsg(nmtmsg);

                recv.clearMsg();
            }
            for (int i = 0; i < 4; ++i) {
                if (id == cobIdTxPdo[i]){
                    txPdo[i].updateData();
                    sendMsg(txPdo[i]);
                    recv.clearMsg();
                    break;
                }
            }
        }
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
            if (currentBytePosition >= CO_PDO_MAX_SIZE) break;

            int entryIndex = Object::findIndex(baseMapIndex, sub);
            if (entryIndex < 0) continue;

            Object& entry = dictionary[entryIndex];
            uint8_t bitLength  = entry.data[0]; // in bits
            uint8_t subIndex   = entry.data[1];
            uint16_t objIndex  = (entry.data[3] << 8) | entry.data[2];

            uint8_t byteLength = bitLength / 8;

            if (currentBytePosition + byteLength > CO_PDO_MAX_SIZE) break;

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
    mapPDOArray(txPdo, TX_PDO1_MAPPING_INDEX); // TPDOs
    mapPDOArray(rxPdo, RX_PDO1_MAPPING_INDEX); // RPDOs
    pdoMapped = true;
}

// Send txPDOs at specified cycle times
void CanOpenNode::sendPDO(PDO &txPdo) {
    if (txPdo.type != PDO::Type::TX) return;
    if (txPdo.numObjects == 0) return;

    // Only event-driven / manufacturer event types use the Event Timer path
    // (254/255). Synchronous types would need SYNC handling (not implemented).
    if (txPdo.transmissionType < PDO_TXTYPE_ASYNC_MANUFACTURER) return;

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

    for (int i = 0; i < CO_PDO_MAX_SIZE; i++) {
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
    can.handleInterrupt();
}

void CanOpenNode::ISRhandler() {
    if (instance != nullptr) {
        instance->MCP2515_ISR();
    }
}