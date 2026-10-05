#ifndef CAN_OPEN_NODE_H
#define CAN_OPEN_NODE_H

#include "../CAN/mcp2515.h"

#include "../../config.h"
#include "message.hpp"
#include "receiver.hpp"
#include "pdo.hpp"
#include "sdo.hpp"
#include "nmt.hpp"
#include "emcy.hpp"
#include "object.hpp"

class CanOpenNode {
public:
    /**
     * @param csPin    MCP2515 chip-select pin
     * @param intPin   MCP2515 interrupt pin
     * @param baudRate (kept for future use / compatibility)
     * @param nodeId   CANopen Node-ID (1 � 127). Defaults to DEFAULT_NODE_ID.
     */
    CanOpenNode(uint8_t csPin, uint8_t intPin, uint8_t baudRate,
                uint8_t nodeId = DEFAULT_NODE_ID);

    // Initialize the MCP2515 CAN controller and set up the node
    void begin();

    // Run core communication loop
    void run();

    void clear(PDO::Type type, uint8_t num, uint8_t position);
    void clear(PDO::Type type, uint8_t num, uint16_t index, uint8_t subIndex);
    void clearAll(PDO::Type type, uint8_t num);

    // Manually set PDO mapping
    void set(PDO::Type type, uint8_t num, uint16_t index, uint8_t subIndex, uint8_t position);

    // Free-slot version – auto-places object in first available contiguous region
    void set(PDO::Type type, uint8_t num, uint16_t index, uint8_t subIndex);    

    // Public - Core communication functions
    bool sendMsg(const Message &msg);
    void writeData(Object &obj, uint8_t data[4]);

    // Node-ID accessor
    uint8_t getNodeId() const { return nodeId; }

    // ------------------------------------------------------------------
    // Runtime setters for cycle times (also writable via SDO)
    // ------------------------------------------------------------------
    /** Set Producer Heartbeat Time (ms). 0 = disabled. Also updates OD 0x1017. */
    void setHeartbeatTime(uint16_t ms);

    /** Set TPDO Event Timer (ms). num = 1�4. Also updates OD Event Timer (SUBIDX_PDO_EVENT_TIMER). */
    void setTxPdoCycleTime(uint8_t num, uint16_t ms);

    /** Set RPDO application timeout (ms). num = 1�4. Also updates OD Event Timer (SUBIDX_PDO_EVENT_TIMER). */
    void setRxPdoCycleTime(uint8_t num, uint16_t ms);

    /** Set TPDO/RPDO Transmission Type (CiA 301). num = 1�4. */
    void setPdoTransmissionType(PDO::Type type, uint8_t num, uint8_t transmissionType);

    /** Set TPDO/RPDO Inhibit Time (units of 100 �s). num = 1�4. 0 = disabled. */
    void setPdoInhibitTime(PDO::Type type, uint8_t num, uint16_t inhibitTime);

    // EMCY - Getters/Setters
    EMCY getEmcy() const { return emcy; }
    void setEmcyErr(const Error &err) { emcy.setError(err); }
    void setEmcyErr(const uint16_t code) { emcy.setError(code); }
    uint16_t getEmcyCode() const { return emcy.getErrCode(); }

    // PDO - Getters/Setters
    const PDO* getPDO(PDO::Type type, uint8_t num) const {
        if (num < 1 || num > 4) return nullptr;
        return (type == PDO::Type::TX) ? &txPdo[num - 1] : &rxPdo[num - 1];
    }

    // SDO - Getters/Setters
    SDO getSDO() const { return sdo; }

    // Enable/Disable Boolean flags (Default is enabled)
    void disableNMT(bool disable) { nmtOn = !disable; }
    void disableMapping(bool disable) { mappingOn = !disable; }

private:
    MCP2515 can;
    const uint8_t intPin;
    const uint8_t baudRate;
    uint8_t nodeId;                     // runtime Node-ID (1-127)

    // Calculated COB-IDs (CiA 301 pre-defined connection set)
    uint16_t cobIdEmcy;
    uint16_t cobIdSdoTx;                // slave ? master (response)
    uint16_t cobIdSdoRx;                // master ? slave (request)
    uint16_t cobIdNmt;                  // heartbeat / boot-up
    uint16_t cobIdTxPdo[4];
    uint16_t cobIdRxPdo[4];

    // Boolean flags
    bool nmtOn;                         // NMT handling enabled
    bool mappingOn;                     // PDO mapping enabled
    bool pdoMapped;

    // Current NMT state
    NMT::Mode nmtState;

    // CANopen message objects
    Receiver recv;
    SDO sdo;
    NMT nmt;
    EMCY emcy;
    PDO txPdo[4];
    PDO rxPdo[4];

    // Private � Core communication functions
    void nmtController();
    void heartBeat();

    void mapPDOArray(PDO* pdoArray, uint16_t baseIndex);
    void mapPDOs();
    void sendPDO(PDO &txPdo);
    void receivePDO(PDO &rxPdo);
    void writeData(Object &obj, uint8_t data[8], uint8_t start);

    void sdoHandler();

    uint16_t getProducerHeartbeatTime() const;
    uint16_t getPdoEventTimer(uint16_t commIndex) const;
    uint8_t  getPdoTransmissionType(uint16_t commIndex) const;
    uint16_t getPdoInhibitTime(uint16_t commIndex) const;

    void     updateOdUint8 (uint16_t index, uint8_t sub, uint8_t  value);
    void     updateOdUint16(uint16_t index, uint8_t sub, uint16_t value);

    // ISR Handling
    static CanOpenNode* instance;
    void MCP2515_ISR();
    static void ISRhandler();
};

#endif
