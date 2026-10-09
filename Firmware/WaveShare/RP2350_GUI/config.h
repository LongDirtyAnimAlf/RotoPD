#ifndef CONFIG_H
#define CONFIG_H

// #define DEBUG // Uncomment to print debug messages
#define PRINT // Uncomment to print CAN messages 

// NMT command specifiers (CiA 301)
#define STARTREMOTENODE                0x01
#define STOPREMOTENODE                 0x02
#define ENTERPREOPREMOTENODE           0x80
#define RESETREMOTENODE                0x81
#define RESETCOMMSREMOTENODE           0x82

#define CO_NMT_NO_COMMAND              0x00
#define CO_NMT_ENTER_OPERATIONAL       0x01
#define CO_NMT_ENTER_STOPPED           0x02
#define CO_NMT_ENTER_PRE_OPERATIONAL   0x80
#define CO_NMT_RESET_NODE              0x81
#define CO_NMT_RESET_COMMUNICATION     0x82

#define CO_NMT_UNKNOWN                 -1
#define CO_NMT_INITIALIZING            0x00
#define CO_NMT_PRE_OPERATIONAL         0x7F
#define CO_NMT_OPERATIONAL             0x05
#define CO_NMT_STOPPED                 0x04


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

#define SUBIDX_0                       0x00   
#define SUBIDX_1                       0x01  // first data entry of a record
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
#define SUBIDX_DEFAULT                 SUBIDX_0   // single-value objects (recommended)
#define SUBIDX_HIGHEST_SUPPORTED       SUBIDX_0   // Number of entries / highest sub-index
#define SUBIDX_VALUE                   SUBIDX_0   // For simple VAR objects (the value itself)

// PDO Communication Parameter record (0x1400.. / 0x1800..)
#define SUBIDX_PDO_COB_ID              SUBIDX_1   // COB-ID used by PDO
#define SUBIDX_PDO_TRANSMISSION_TYPE   SUBIDX_2   // Transmission type
#define SUBIDX_PDO_INHIBIT_TIME        SUBIDX_3   // Inhibit time (multiples of 100 �s)
#define SUBIDX_PDO_RESERVED            SUBIDX_4   // Reserved
#define SUBIDX_PDO_EVENT_TIMER         SUBIDX_5   // Event timer (ms)
#define SUBIDX_PDO_SYNC_START_VALUE    SUBIDX_6   // SYNC start value

// PDO Mapping Parameter record (0x1600.. / 0x1A00..)
#define SUBIDX_PDO_MAP_COUNT           SUBIDX_HIGHEST_SUPPORTED   // Number of mapped application objects
#define SUBIDX_PDO_MAP_1               SUBIDX_1   // 1st mapped object
#define SUBIDX_PDO_MAP_2               SUBIDX_2
#define SUBIDX_PDO_MAP_3               SUBIDX_3
#define SUBIDX_PDO_MAP_4               SUBIDX_4
#define SUBIDX_PDO_MAP_5               SUBIDX_5
#define SUBIDX_PDO_MAP_6               SUBIDX_6
#define SUBIDX_PDO_MAP_7               SUBIDX_7
#define SUBIDX_PDO_MAP_8               SUBIDX_8

// Identity Object (0x1018)
#define SUBIDX_IDENTITY_COUNT          SUBIDX_HIGHEST_SUPPORTED   // Number of identity objects
#define SUBIDX_IDENTITY_VENDOR_ID      SUBIDX_1
#define SUBIDX_IDENTITY_PRODUCT_CODE   SUBIDX_2
#define SUBIDX_IDENTITY_REVISION       SUBIDX_3
#define SUBIDX_IDENTITY_SERIAL         SUBIDX_4

#define SIZE_BIT_INT32                 32  // 0x20 bits
#define SIZE_BIT_UINT32                32  // 0x20 bits
#define SIZE_BIT_UINT16                16  // 0x10 bits
#define SIZE_BIT_UINT8                 8   // 0x08 bits

#define TX_PDO1_MAPPING_INDEX          0x1A00
#define TX_PDO2_MAPPING_INDEX          0x1A01
#define TX_PDO3_MAPPING_INDEX          0x1A02
#define TX_PDO4_MAPPING_INDEX          0x1A03
#define RX_PDO1_MAPPING_INDEX          0x1600
#define RX_PDO2_MAPPING_INDEX          0x1601
#define RX_PDO3_MAPPING_INDEX          0x1602
#define RX_PDO4_MAPPING_INDEX          0x1603

// Standard EMCY Error Register Bits (CiA 301)
#define CO_ERR_REG_GENERIC_ERR          0x01U /**< bit 0, generic error */
#define CO_ERR_REG_CURRENT              0x02U /**< bit 1, current */
#define CO_ERR_REG_VOLTAGE              0x04U /**< bit 2, voltage */
#define CO_ERR_REG_TEMPERATURE          0x08U /**< bit 3, temperature */
#define CO_ERR_REG_COMMUNICATION        0x10U /**< bit 4, communication error */
#define CO_ERR_REG_DEV_PROFILE          0x20U /**< bit 5, device profile specific */
#define CO_ERR_REG_RESERVED             0x40U /**< bit 6, reserved (always 0) */
#define CO_ERR_REG_MANUFACTURER         0x80U /**< bit 7, manufacturer specific */

// Standard error codes according to CiA DS-301 and DS-401.
#define CO_EMC_NO_ERROR                 0x0000U /**< 0x00xx error Reset or No Error */
#define CO_EMC_GENERIC                  0x1000U /**< 0x10xx Generic Error */
#define CO_EMC_CURRENT                  0x2000U /**< 0x20xx Current */
#define CO_EMC_CURRENT_INPUT            0x2100U /**< 0x21xx Current device input side */
#define CO_EMC_CURRENT_INSIDE           0x2200U /**< 0x22xx Current inside the device */
#define CO_EMC_CURRENT_OUTPUT           0x2300U /**< 0x23xx Current device output side */
#define CO_EMC_VOLTAGE                  0x3000U /**< 0x30xx Voltage */
#define CO_EMC_VOLTAGE_MAINS            0x3100U /**< 0x31xx Mains Voltage */
#define CO_EMC_VOLTAGE_INSIDE           0x3200U /**< 0x32xx Voltage inside the device */
#define CO_EMC_VOLTAGE_OUTPUT           0x3300U /**< 0x33xx Output Voltage */
#define CO_EMC_TEMPERATURE              0x4000U /**< 0x40xx Temperature */
#define CO_EMC_TEMP_AMBIENT             0x4100U /**< 0x41xx Ambient Temperature */
#define CO_EMC_TEMP_DEVICE              0x4200U /**< 0x42xx Device Temperature */
#define CO_EMC_HARDWARE                 0x5000U /**< 0x50xx Device Hardware */
#define CO_EMC_SOFTWARE_DEVICE          0x6000U /**< 0x60xx Device Software */
#define CO_EMC_SOFTWARE_INTERNAL        0x6100U /**< 0x61xx Internal Software */
#define CO_EMC_SOFTWARE_USER            0x6200U /**< 0x62xx User Software */
#define CO_EMC_DATA_SET                 0x6300U /**< 0x63xx Data Set */
#define CO_EMC_ADDITIONAL_MODUL         0x7000U /**< 0x70xx Additional Modules */
#define CO_EMC_MONITORING               0x8000U /**< 0x80xx Monitoring */
#define CO_EMC_COMMUNICATION            0x8100U /**< 0x81xx Communication */
#define CO_EMC_CAN_OVERRUN              0x8110U /**< 0x8110 CAN Overrun (Objects lost) */
#define CO_EMC_CAN_PASSIVE              0x8120U /**< 0x8120 CAN in Error Passive Mode */
#define CO_EMC_HEARTBEAT                0x8130U /**< 0x8130 Life Guard Error or Heartbeat Error */
#define CO_EMC_BUS_OFF_RECOVERED        0x8140U /**< 0x8140 recovered from bus off */
#define CO_EMC_CAN_ID_COLLISION         0x8150U /**< 0x8150 CAN-ID collision */
#define CO_EMC_PROTOCOL_ERROR           0x8200U /**< 0x82xx Protocol Error */
#define CO_EMC_PDO_LENGTH               0x8210U /**< 0x8210 PDO not processed due to length error */
#define CO_EMC_PDO_LENGTH_EXC           0x8220U /**< 0x8220 PDO length exceeded */
#define CO_EMC_DAM_MPDO                 0x8230U /**< 0x8230 DAM MPDO not processed destination object not available */
#define CO_EMC_SYNC_DATA_LENGTH         0x8240U /**< 0x8240 Unexpected SYNC data length */
#define CO_EMC_RPDO_TIMEOUT             0x8250U /**< 0x8250 RPDO timeout */
#define CO_EMC_EXTERNAL_ERROR           0x9000U /**< 0x90xx External Error */
#define CO_EMC_ADDITIONAL_FUNC          0xF000U /**< 0xF0xx Additional Functions */
#define CO_EMC_DEVICE_SPECIFIC          0xFF00U /**< 0xFFxx Device specific */
#define CO_EMC401_OUT_CUR_HI            0x2310U /**< 0x2310 DS401 Current at outputs too high (overload) */
#define CO_EMC401_OUT_SHORTED           0x2320U /**< 0x2320 DS401 Short circuit at outputs */
#define CO_EMC401_OUT_LOAD_DUMP         0x2330U /**< 0x2330 DS401 Load dump at outputs */
#define CO_EMC401_IN_VOLT_HI            0x3110U /**< 0x3110 DS401 Input voltage too high */
#define CO_EMC401_IN_VOLT_LOW           0x3120U /**< 0x3120 DS401 Input voltage too low */
#define CO_EMC401_INTERN_VOLT_HI        0x3210U /**< 0x3210 DS401 Internal voltage too high */
#define CO_EMC401_INTERN_VOLT_LO        0x3220U /**< 0x3220 DS401 Internal voltage too low */
#define CO_EMC401_OUT_VOLT_HIGH         0x3310U /**< 0x3310 DS401 Output voltage too high */
#define CO_EMC401_OUT_VOLT_LOW          0x3320U /**< 0x3320 DS401 Output voltage too low */

// Bits for internal indication of the error condition. Each error condition is specified by unique index from 0x00 up to 0xFF.
#define CO_EM_NO_ERROR                  0x00U /**< 0x00 Error Reset or No Error */
#define CO_EM_CAN_BUS_WARNING           0x01U /**< 0x01 communication info CAN bus warning limit reached */
#define CO_EM_RXMSG_WRONG_LENGTH        0x02U /**< 0x02 communication info Wrong data length of the received CAN frame */
#define CO_EM_RXMSG_OVERFLOW            0x03U /**< 0x03 communication info Previous received CAN frame wasn't processed */
#define CO_EM_RPDO_WRONG_LENGTH         0x04U /**< 0x04 communication info Wrong data length of received PDO */
#define CO_EM_RPDO_OVERFLOW             0x05U /**< 0x05 communication info Previous received PDO wasn't processed yet */
#define CO_EM_CAN_RX_BUS_PASSIVE        0x06U /**< 0x06 communication info CAN receive bus is passive */
#define CO_EM_CAN_TX_BUS_PASSIVE        0x07U /**< 0x07 communication info CAN transmit bus is passive */
#define CO_EM_NMT_WRONG_COMMAND         0x08U /**< 0x08 communication info Wrong NMT command received */
#define CO_EM_TIME_TIMEOUT              0x09U /**< 0x09 communication info TIME message timeout */
#define CO_EM_0A_unused                 0x0AU /**< 0x0A communication info (unused) */
#define CO_EM_0B_unused                 0x0BU /**< 0x0B communication info (unused) */
#define CO_EM_0C_unused                 0x0CU /**< 0x0C communication info (unused) */
#define CO_EM_0D_unused                 0x0DU /**< 0x0D communication info (unused) */
#define CO_EM_0E_unused                 0x0EU /**< 0x0E communication info (unused) */
#define CO_EM_0F_unused                 0x0FU /**< 0x0F communication info (unused) */

#define CO_EM_10_unused                 0x10U /**< 0x10 communication critical (unused) */
#define CO_EM_11_unused                 0x11U /**< 0x11 communication critical (unused) */
#define CO_EM_CAN_TX_BUS_OFF            0x12U /**< 0x12 communication critical CAN transmit bus is off */
#define CO_EM_CAN_RXB_OVERFLOW          0x13U /**< 0x13 communication critical CAN module receive buffer overflowed */
#define CO_EM_CAN_TX_OVERFLOW           0x14U /**< 0x14 communication critical CAN transmit buffer overflowed */
#define CO_EM_TPDO_OUTSIDE_WINDOW       0x15U /**< 0x15 communication critical TPDO is outside SYNC window */
#define CO_EM_16_unused                 0x16U /**< 0x16 communication critical (unused) */
#define CO_EM_RPDO_TIME_OUT             0x17U /**< 0x17 communication critical RPDO message timeout */
#define CO_EM_SYNC_TIME_OUT             0x18U /**< 0x18 communication critical SYNC message timeout */
#define CO_EM_SYNC_LENGTH               0x19U /**< 0x19 communication critical Unexpected SYNC data length */
#define CO_EM_PDO_WRONG_MAPPING         0x1AU /**< 0x1A communication critical Error with PDO mapping */
#define CO_EM_HEARTBEAT_CONSUMER        0x1BU /**< 0x1B communication critical Heartbeat consumer timeout */
#define CO_EM_HB_CONSUMER_REMOTE_RESET  0x1CU /**< 0x1C comm. critical Heartbeat consumer detected remote node reset */
#define CO_EM_SRDO_CONFIGURATION        0x1DU /**< 0x1D communication critical Error in SRDO configuration parameters */
#define CO_EM_1E_unused                 0x1EU /**< 0x1E communication critical (unused) */
#define CO_EM_1F_unused                 0x1FU /**< 0x1F communication critical (unused) */

#define CO_EM_EMERGENCY_BUFFER_FULL     0x20U /**< 0x20 generic info Emergency buffer is full or message wasn't sent */
#define CO_EM_21_unused                 0x21U /**< 0x21 generic info (unused) */
#define CO_EM_MICROCONTROLLER_RESET     0x22U /**< 0x22 generic info Microcontroller has just started */
#define CO_EM_23_unused                 0x23U /**< 0x23 generic info (unused) */
#define CO_EM_24_unused                 0x24U /**< 0x24 generic info (unused) */
#define CO_EM_25_unused                 0x25U /**< 0x25 generic info (unused) */
#define CO_EM_26_unused                 0x26U /**< 0x26 generic info (unused) */
#define CO_EM_NON_VOLATILE_AUTO_SAVE    0x27U /**< 0x27 generic info Automatic store to non-volatile memory failed */

#define CO_EM_WRONG_ERROR_REPORT        0x28U /**< 0x28 generic critical Wrong parameters to CO_errorReport() */
#define CO_EM_ISR_TIMER_OVERFLOW        0x29U /**< 0x29 generic critical Timer task has overflowed */
#define CO_EM_MEMORY_ALLOCATION_ERROR   0x2AU /**< 0x2A generic critical Unable to allocate memory for objects */
#define CO_EM_GENERIC_ERROR             0x2BU /**< 0x2B generic critical Generic error test usage */
#define CO_EM_GENERIC_SOFTWARE_ERROR    0x2CU /**< 0x2C generic critical Software error */
#define CO_EM_INCONSISTENT_OBJECT_DICT  0x2DU /**< 0x2D generic critical Object dict. does not match the software */
#define CO_EM_CALCULATION_OF_PARAMETERS 0x2EU /**< 0x2E generic critical Error in calculation of device parameters */
#define CO_EM_NON_VOLATILE_MEMORY       0x2FU /**< 0x2F generic critical Error with access to non volatile memory */


// ------------------------------------------------------------------
// PDO Transmission Types (CiA 301, sub-index 02h)
// ------------------------------------------------------------------
// Synchronous

typedef enum {
    CO_PDO_TRANSM_TYPE_SYNC_ACYCLIC = 0U,     /**< synchronous (acyclic) */
    CO_PDO_TRANSM_TYPE_SYNC_1 = 1U,           /**< synchronous (cyclic every sync) */
    CO_PDO_TRANSM_TYPE_SYNC_240 = 0xF0U,      /**< synchronous (cyclic every 240-th sync) */
    CO_PDO_TRANSM_TYPE_SYNC_EVENT_LO = 0xFEU, /**< event-driven, lower value (manufacturer specific),  */
    CO_PDO_TRANSM_TYPE_SYNC_EVENT_HI = 0xFFU  /**< event-driven, higher value (device profile and application profile
                                                 specific) */
} CO_PDO_transmissionTypes_t;


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
#define CO_PDO_MAX_SIZE 8U
#define CO_SDO_MAX_SIZE 8U
#define CO_EMC_MAX_SIZE 8U
#define CO_NMT_MAX_SIZE 1U

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
#define PDO1_RX_CYCLE_TIME      0  // ms    (application receive timeout)
#define PDO2_RX_CYCLE_TIME      0  // ms
#define PDO3_RX_CYCLE_TIME      0  // ms
#define PDO4_RX_CYCLE_TIME      0  // ms



// This device !!
// Application-specific Object Dictionary indices (CiA 453 Electronic Load)
#define OPERATING_MODE                 0x6000
#define CONTROLWORD                    0x6040
#define STATUSWORD                     0x6041
#define LOAD_OUTPUT_ENABLE             0x6044
#define TARGET_CURRENT                 0x6070
#define TARGET_VOLTAGE                 0x6071
#define TARGET_POWER                   0x6072
#define ACTUAL_VOLTAGE                 0x6080
#define ACTUAL_CURRENT                 0x6081
#define ACTUAL_POWER                   0x6082
#define VOLTAGE_LIMITS                 0x6090

#define DMM_READINGS                   0x6130 // Readings from INA238 on RotoPD board

/* ---- controlword (6040h) bits ------------------------------------------ */
#define CW_SWITCH_ON            0x0001u
#define CW_ENABLE_VOLTAGE       0x0002u
#define CW_QUICK_STOP           0x0004u
#define CW_ENABLE_OPERATION     0x0008u
#define CW_OP_MODE_CC           0x0010u
#define CW_OP_MODE_CR           0x0020u
#define CW_OP_MODE_CP           0x0040u
#define CW_FAULT                0x0080u
#define CW_HALT                 0x0100u

/* ---- statusword (6041h) bits ------------------------------------------- */
#define SW_READY_TO_SWITCH_ON   0x0001u
#define SW_SWITCHED_ON          0x0002u
#define SW_OPERATION_ENABLED    0x0004u
#define SW_FAULT                0x0008u
#define SW_VOLTAGE_ENABLED      0x0010u
#define SW_QUICK_STOP           0x0020u
#define SW_SWITCH_ON_DISABLED   0x0040u
#define SW_WARNING              0x0080u
#define SW_REMOTE               0x0200u
#define SW_TARGET_REACHED       0x0400u
#define SW_INTERNAL_LIMIT       0x0800u
#define SW_OP_MODE_CC           0x1000u
#define SW_OP_MODE_CR           0x2000u
#define SW_OP_MODE_CP           0x4000u


#endif
