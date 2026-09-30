#ifndef CONFIG_H
#define CONFIG_H

// #define DEBUG // Uncomment to print debug messages
#define PRINT // Uncomment to print CAN messages 

// Interrupt pin on MCP2515
#define CAN_INT   2
#define CAN_CS    9

// Application-specific Object Dictionary indices
#define DISPLAYED_CHARGE               0x2520
#define VOLTAGE                        0x2002
#define CURRENT                        0x2003
#define POWER                          0x2004
#define STATUS                         0x2005
#define TEMPERATURE                    0x6010
#define MIN_VOLTAGE                    0x2006

// NMT command specifiers (CiA 301)
#define StartNode                      0x01
#define StopNode                       0x02
#define EnterPreOperational            0x80
#define ResetNode                      0x81
#define ResetCommunication             0x82

// Standard CANopen Object Dictionary Indices (CiA 301)
#define DEVICE_TYPE_INDEX              0x1000
#define ERROR_REGISTER_INDEX           0x1001
#define SYNC_COBID_INDEX               0x1005
#define COMM_CYCLE_PERIOD_INDEX        0x1006
#define GUARD_TIME_INDEX               0x100C
#define LIFE_TIME_FACTOR_INDEX         0x100D
#define EMERGENCY_COBID_INDEX          0x1014
#define CONSUMER_HEARTBEAT_INDEX       0x1016
#define PRODUCER_HEARTBEAT_INDEX       0x1017
#define IDENTITY_OBJECT_INDEX          0x1018
#define ERROR_BEHAVIOR_INDEX           0x1029

// TPDO Communication Parameter indices (CiA 301)
// Sub-index 5 = Event Timer (ms) � used as cyclic transmission period
#define TX_PDO1_COMM_INDEX             0x1800
#define TX_PDO2_COMM_INDEX             0x1801
#define TX_PDO3_COMM_INDEX             0x1802
#define TX_PDO4_COMM_INDEX             0x1803

// RPDO Communication Parameter indices (CiA 301)
#define RX_PDO1_COMM_INDEX             0x1400
#define RX_PDO2_COMM_INDEX             0x1401
#define RX_PDO3_COMM_INDEX             0x1402
#define RX_PDO4_COMM_INDEX             0x1403

#define SUBIDX_DEFAULT                 0x00   // single-value objects (recommended)
#define SUBIDX_1                       0x01   // first data entry of a record
#define SUBIDX_2                       0x02
#define SUBIDX_3                       0x03
#define SUBIDX_4                       0x04
#define SUBIDX_5                       0x05
#define SUBIDX_6                       0x06
#define SUBIDX_7                       0x07
#define SUBIDX_8                       0x08
#define SUBIDX_9                       0x09
#define SUBIDX_10                      0x0A
#define SUBIDX_11                      0x0B
#define SUBIDX_12                      0x0C
#define SUBIDX_13                      0x0D
#define SUBIDX_14                      0x0E
#define SUBIDX_15                      0x0F

// ------------------------------------------------------------------
// Standard CiA 301 sub-indexes
// ------------------------------------------------------------------

// Generic / common
#define SUBIDX_HIGHEST_SUPPORTED       0x00   // Number of entries / highest sub-index
#define SUBIDX_VALUE                   0x00   // For simple VAR objects (the value itself)

// PDO Communication Parameter record (0x1400.. / 0x1800..)
#define SUBIDX_PDO_COB_ID              0x01   // COB-ID used by PDO
#define SUBIDX_PDO_TRANSMISSION_TYPE   0x02   // Transmission type
#define SUBIDX_PDO_INHIBIT_TIME        0x03   // Inhibit time (multiples of 100 �s)
#define SUBIDX_PDO_RESERVED            0x04   // Reserved
#define SUBIDX_PDO_EVENT_TIMER         0x05   // Event timer (ms)
#define SUBIDX_PDO_SYNC_START_VALUE    0x06   // SYNC start value

// PDO Mapping Parameter record (0x1600.. / 0x1A00..)
#define SUBIDX_PDO_MAP_COUNT           0x00   // Number of mapped application objects
#define SUBIDX_PDO_MAP_1               0x01   // 1st mapped object
#define SUBIDX_PDO_MAP_2               0x02
#define SUBIDX_PDO_MAP_3               0x03
#define SUBIDX_PDO_MAP_4               0x04
#define SUBIDX_PDO_MAP_5               0x05
#define SUBIDX_PDO_MAP_6               0x06
#define SUBIDX_PDO_MAP_7               0x07
#define SUBIDX_PDO_MAP_8               0x08

// Identity Object (0x1018)
#define SUBIDX_IDENTITY_COUNT          0x00   // Number of identity objects
#define SUBIDX_IDENTITY_VENDOR_ID      0x01
#define SUBIDX_IDENTITY_PRODUCT_CODE   0x02
#define SUBIDX_IDENTITY_REVISION       0x03
#define SUBIDX_IDENTITY_SERIAL         0x04


#define TX_PDO1_MAPPING_INDEX          0x1A00
#define TX_PDO2_MAPPING_INDEX          0x1A01
#define TX_PDO3_MAPPING_INDEX          0x1A02
#define TX_PDO4_MAPPING_INDEX          0x1A03
#define RX_PDO1_MAPPING_INDEX          0x1600
#define RX_PDO2_MAPPING_INDEX          0x1601
#define RX_PDO3_MAPPING_INDEX          0x1602
#define RX_PDO4_MAPPING_INDEX          0x1603

// Standard EMCY Error Register Bits (CiA 301)
#define EMCY_GENERIC_ERR_BIT         0x01  // Bit 0
#define EMCY_VOLTAGE_ERR_BIT         0x02  // Bit 1
#define EMCY_CURRENT_ERR_BIT         0x04  // Bit 2
#define EMCY_TEMP_ERR_BIT            0x08  // Bit 3
#define EMCY_COMM_ERR_BIT            0x20  // Bit 5
#define EMCY_DEVICE_ERR_BIT          0x80  // Bit 7


// ------------------------------------------------------------------
// PDO Transmission Types (CiA 301, sub-index 02h)
// ------------------------------------------------------------------
// Synchronous
#define PDO_TXTYPE_SYNC_ACYCLIC           0x00   // 0   – after SYNC only if event occurred
#define PDO_TXTYPE_SYNC_CYCLIC_1          0x01   // 1   – every SYNC
#define PDO_TXTYPE_SYNC_CYCLIC_2          0x02   // 2   – every 2nd SYNC
// … values 3 … 240 are “every n-th SYNC”
#define PDO_TXTYPE_SYNC_CYCLIC_MAX        0xF0   // 240 – every 240th SYNC
// Reserved
// 0xF1 … 0xFB (241 … 251) – reserved by CiA
// RTR-related
#define PDO_TXTYPE_SYNC_RTR_ONLY          0xFC   // 252 – SYNC + remote request only
#define PDO_TXTYPE_ASYNC_RTR_ONLY         0xFD   // 253 – remote request only
// Event-driven / asynchronous
#define PDO_TXTYPE_ASYNC_MANUFACTURER     0xFE   // 254 – manufacturer-specific event
#define PDO_TXTYPE_ASYNC_DEVICE_PROFILE   0xFF   // 255 – device-profile / application event (most common default)

#define PDO_TXTYPE_EVENT_DRIVEN           PDO_TXTYPE_ASYNC_DEVICE_PROFILE  // 255
#define PDO_TXTYPE_CYCLIC_EVERY_SYNC      PDO_TXTYPE_SYNC_CYCLIC_1         // 1


// Manufacturer-specific error field templates (5 bytes)
#define MFG_ERROR_GENERIC            {0x00, 0x00, 0x00, 0x00, 0x01}
#define MFG_ERROR_OVERCURRENT        {0x00, 0x00, 0x00, 0x02, 0x02}
#define MFG_ERROR_VOLTAGE            {0x00, 0x00, 0x00, 0x03, 0x03}
#define MFG_ERROR_TEMPERATURE        {0x00, 0x00, 0x00, 0x04, 0x04}
#define MFG_ERROR_COMM               {0x00, 0x00, 0x00, 0x05, 0x05}
#define MFG_ERROR_DEVICE             {0x00, 0x00, 0x00, 0x06, 0x06}

// Message lengths (CiA 301)
#define PDO_LEN   8
#define SDO_LEN   8
#define EMCY_LEN  8
#define NMT_LEN   1

// Base COB-ID Offsets (CiA 301 Pre-defined Connection Set)
#define CO_CAN_ID_NMT_SERVICE 0x000U /**< 0x000 Network management */
#define CO_CAN_ID_GFC         0x001U /**< 0x001 Global fail-safe command */
#define CO_CAN_ID_SYNC        0x080U /**< 0x080 Synchronous message */
#define CO_CAN_ID_EMERGENCY   0x080U /**< 0x080 Emergency messages (+nodeID) */
#define CO_CAN_ID_TIME        0x100U /**< 0x100 Time message */
#define CO_CAN_ID_SRDO_1      0x0FFU /**< 0x0FF Default SRDO1 (+2*nodeID) */
#define CO_CAN_ID_TPDO_1      0x180U /**< 0x180 Default TPDO1 (+nodeID) */
#define CO_CAN_ID_RPDO_1      0x200U /**< 0x200 Default RPDO1 (+nodeID) */
#define CO_CAN_ID_TPDO_2      0x280U /**< 0x280 Default TPDO2 (+nodeID) */
#define CO_CAN_ID_RPDO_2      0x300U /**< 0x300 Default RPDO2 (+nodeID) */
#define CO_CAN_ID_TPDO_3      0x380U /**< 0x380 Default TPDO3 (+nodeID) */
#define CO_CAN_ID_RPDO_3      0x400U /**< 0x400 Default RPDO3 (+nodeID) */
#define CO_CAN_ID_TPDO_4      0x480U /**< 0x480 Default TPDO4 (+nodeID) */
#define CO_CAN_ID_RPDO_4      0x500U /**< 0x500 Default RPDO4 (+nodeID) */
#define CO_CAN_ID_SDO_SRV     0x580U /**< 0x580 SDO response from server (+nodeID) */
#define CO_CAN_ID_SDO_CLI     0x600U /**< 0x600 SDO request from client (+nodeID) */
#define CO_CAN_ID_HEARTBEAT   0x700U /**< 0x700 Heartbeat message */
#define CO_CAN_ID_LSS_SLV     0x7E4U /**< 0x7E4 LSS response from slave */
#define CO_CAN_ID_LSS_MST     0x7E5U /**< 0x7E5 LSS request from master */

#define BROADCAST         0x000
#define SYNCHRONIZE       0x080
#define TIMESTAMP         0x100

#define BASE_EMCY_ID      0x080
#define BASE_PDO1_TX_ID   0x180
#define BASE_PDO1_RX_ID   0x200
#define BASE_PDO2_TX_ID   0x280
#define BASE_PDO2_RX_ID   0x300
#define BASE_PDO3_TX_ID   0x380
#define BASE_PDO3_RX_ID   0x400
#define BASE_PDO4_TX_ID   0x480
#define BASE_PDO4_RX_ID   0x500
#define BASE_SDO_TX_ID    0x580
#define BASE_SDO_RX_ID    0x600
#define BASE_NMT_ID       0x700   // Heartbeat / Boot-up





// Default Node-ID (can be overridden at runtime via CanOpenNode constructor)
#define DEFAULT_NODE_ID   0x01

// SDO command bytes (CiA 301)
#define READ_REQ_ANY_CMD        0x40
#define READ_REQ_1BYTE_CMD      0x4F
#define READ_REQ_2BYTE_CMD      0x4B
#define READ_REQ_4BYTE_CMD      0x43

#define WRITE_REQ_1BYTE_CMD     0x2F
#define WRITE_REQ_2BYTE_CMD     0x2B
#define WRITE_REQ_4BYTE_CMD     0x23
#define WRITE_RESP_SUCCESS_CMD  0x60
#define WRITE_RESP_FAIL_CMD     0x80

// Default cycle times (ms) � used as initial / fallback values
// Actual values are taken from Object Dictionary and can be changed at runtime
#define HEARTBEAT_CYCLE_TIME    500   // ms  (OD 0x1017)
#define PDO1_TX_CYCLE_TIME      500   // ms  (OD 0x1800:05 Event Timer)
#define PDO2_TX_CYCLE_TIME      500   // ms  (OD 0x1801:05)
#define PDO3_TX_CYCLE_TIME      500   // ms  (OD 0x1802:05)
#define PDO4_TX_CYCLE_TIME      500   // ms  (OD 0x1803:05)
#define PDO1_RX_CYCLE_TIME      1400  // ms  (application timeout, not standard)
#define PDO2_RX_CYCLE_TIME      1400  // ms
#define PDO3_RX_CYCLE_TIME      1400  // ms
#define PDO4_RX_CYCLE_TIME      1400  // ms

#endif
