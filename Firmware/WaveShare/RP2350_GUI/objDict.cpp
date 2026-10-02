#include "./src/CANOpen/object.hpp"

Object dictionary[] = {
//  |             INDEX             | SUB |          DATA          |       TYPE       |      PERMISSIONS      |
    // Application objects
    // ------------------------------------------------------------------
    // Application Objects (CiA 453 DC Electronic Load)
    // ------------------------------------------------------------------
    // Operating Mode (1 = Constant Current)
    {OPERATING_MODE,                 SUBIDX_DEFAULT, {0x01, 0x00, 0x00, 0x00}, DataType::INT8,   ObjectPermissions::READ_WRITE},
    // Controlword
    {CONTROLWORD,                    SUBIDX_DEFAULT, {0x00, 0x00, 0x00, 0x00}, DataType::UINT16, ObjectPermissions::READ_WRITE},
    // Statusword
    {STATUSWORD,                     SUBIDX_DEFAULT, {0x00, 0x00, 0x00, 0x00}, DataType::UINT16, ObjectPermissions::READ},
    // Load Output Enable
    {LOAD_OUTPUT_ENABLE,             SUBIDX_DEFAULT, {0x00, 0x00, 0x00, 0x00}, DataType::UINT8,  ObjectPermissions::READ_WRITE},
    // Target Current
    {TARGET_CURRENT,                 SUBIDX_DEFAULT, {0x00, 0x00, 0x00, 0x00}, DataType::INT32,  ObjectPermissions::READ_WRITE},
    // Target Voltage
    {TARGET_VOLTAGE,                 SUBIDX_DEFAULT, {0x00, 0x00, 0x00, 0x00}, DataType::INT32,  ObjectPermissions::READ_WRITE},
    // Target Power
    {TARGET_POWER,                   SUBIDX_DEFAULT, {0x00, 0x00, 0x00, 0x00}, DataType::INT32,  ObjectPermissions::READ_WRITE},
    // Actual Voltage
    {ACTUAL_VOLTAGE,                 SUBIDX_DEFAULT, {0x00, 0x00, 0x00, 0x00}, DataType::INT32,  ObjectPermissions::READ},
    // Actual Current
    {ACTUAL_CURRENT,                 SUBIDX_DEFAULT, {0x00, 0x00, 0x00, 0x00}, DataType::INT32,  ObjectPermissions::READ},
    // Actual Power
    {ACTUAL_POWER,                   SUBIDX_DEFAULT, {0x00, 0x00, 0x00, 0x00}, DataType::INT32,  ObjectPermissions::READ},
    // Voltage Limits
    {VOLTAGE_LIMITS,                 SUBIDX_DEFAULT, {0x01, 0x00, 0x00, 0x00}, DataType::UINT8,  ObjectPermissions::READ},
    {VOLTAGE_LIMITS,                 SUBIDX_1,       {0xB8, 0x0B, 0x00, 0x00}, DataType::INT32,  ObjectPermissions::READ_WRITE}, // 3000 mV (0x0BB8)
    
    // Communication profile area (CiA 301)
    {DEVICE_TYPE_INDEX,              SUBIDX_DEFAULT, {0x00, 0x00, 0x00, 0x00}, DataType::UINT32, ObjectPermissions::READ},
    {ERROR_REGISTER_INDEX,           SUBIDX_DEFAULT, {0x00, 0x00, 0x00, 0x00}, DataType::UINT8,  ObjectPermissions::READ},
    {SYNC_COBID_INDEX,               SUBIDX_DEFAULT, {0x80, 0x00, 0x00, 0x00}, DataType::UINT32, ObjectPermissions::READ_WRITE},
    {COMM_CYCLE_PERIOD_INDEX,        SUBIDX_DEFAULT, {0x00, 0x00, 0x00, 0x00}, DataType::UINT32, ObjectPermissions::READ_WRITE},
    {GUARD_TIME_INDEX,               SUBIDX_DEFAULT, {0x64, 0x00, 0x00, 0x00}, DataType::UINT16, ObjectPermissions::READ_WRITE},
    {LIFE_TIME_FACTOR_INDEX,         SUBIDX_DEFAULT, {0x03, 0x00, 0x00, 0x00}, DataType::UINT8,  ObjectPermissions::READ_WRITE},
    {EMERGENCY_COBID_INDEX,          SUBIDX_DEFAULT, {0x81, 0x00, 0x00, 0x00}, DataType::UINT32, ObjectPermissions::READ_WRITE},
    {CONSUMER_HEARTBEAT_INDEX,       SUBIDX_DEFAULT, {0x00, 0x00, 0x00, 0x00}, DataType::UINT8,  ObjectPermissions::READ_WRITE},

    // 0x1017 Producer Heartbeat Time : default 500 ms (0x01F4)
    {PRODUCER_HEARTBEAT_INDEX,       SUBIDX_DEFAULT, {0xF4, 0x01, 0x00, 0x00}, DataType::UINT16, ObjectPermissions::READ_WRITE},

    {IDENTITY_OBJECT_INDEX,          SUBIDX_IDENTITY_COUNT, {0x04, 0x00, 0x00, 0x00}, DataType::UINT8,  ObjectPermissions::READ}, // Number of entries (1…4). Default usually 4.
    {IDENTITY_OBJECT_INDEX,          SUBIDX_IDENTITY_VENDOR_ID, {0x34, 0x12, 0x00, 0x00}, DataType::UINT32, ObjectPermissions::READ},  // Vendor ID
    {IDENTITY_OBJECT_INDEX,          SUBIDX_IDENTITY_PRODUCT_CODE, {0x78, 0x56, 0x00, 0x00}, DataType::UINT32, ObjectPermissions::READ},  // Product Code
    {IDENTITY_OBJECT_INDEX,          SUBIDX_IDENTITY_REVISION, {0x01, 0x00, 0x00, 0x00}, DataType::UINT32, ObjectPermissions::READ},  // Revision
    {IDENTITY_OBJECT_INDEX,          SUBIDX_IDENTITY_SERIAL, {0xAB, 0xCD, 0xEF, 0x00}, DataType::UINT32, ObjectPermissions::READ},  // Serial Number

    {ERROR_BEHAVIOR_INDEX,           SUBIDX_DEFAULT, {0x00, 0x00, 0x00, 0x00}, DataType::UINT8,  ObjectPermissions::READ_WRITE},

    // ------------------------------------------------------------------
    // TPDO Communication Parameters (CiA 301)
    // sub 02h = Transmission type (default 0xFF = event-driven)
    // sub 03h = Inhibit time (units of 100 µs, default 0 = disabled)
    // sub 05h = Event timer (ms, default 500ms)
    // ------------------------------------------------------------------
    {TX_PDO1_COMM_INDEX, SUBIDX_HIGHEST_SUPPORTED,     {0x05, 0x00, 0x00, 0x00}, DataType::UINT8,  ObjectPermissions::READ},   // Highest sub-index supported
    // Bit 31: Valid/Invalid (0=Valid, 1=Invalid)
    // Bit 30: RTR Enable (0=RTR allowed, 1=RTR forbidden)
    // Bit 29: Frame Type (0=11-bit, 1=29-bit) 
    // Bits 0-10: Actual CAN Identifier = NodeID.
    // COB-ID (e.g., 40000181h: rtr invalid/disabled by default until operational at NodeID = 1)    
    {TX_PDO1_COMM_INDEX, SUBIDX_PDO_COB_ID,            {0x81, 0x01, 0x00, 0x40}, DataType::UINT32, ObjectPermissions::READ},
    {TX_PDO1_COMM_INDEX, SUBIDX_PDO_TRANSMISSION_TYPE, {PDO_TXTYPE_ASYNC_DEVICE_PROFILE, 0x00, 0x00, 0x00}, DataType::UINT8,  ObjectPermissions::READ_WRITE},  // Transmission Type
    {TX_PDO1_COMM_INDEX, SUBIDX_PDO_INHIBIT_TIME,      {0x00, 0x00, 0x00, 0x00}, DataType::UINT16, ObjectPermissions::READ_WRITE},  // Inhibit Time (in 100 µs steps)
    {TX_PDO1_COMM_INDEX, SUBIDX_PDO_RESERVED,          {0x00, 0x00, 0x00, 0x00}, DataType::UINT8,  ObjectPermissions::READ},   // Compatibility entry (Reserved / 0x00)
    {TX_PDO1_COMM_INDEX, SUBIDX_PDO_EVENT_TIMER,       {0xF4, 0x01, 0x00, 0x00}, DataType::UINT16, ObjectPermissions::READ_WRITE},   // Event Timer (500 milliseconds)


    {TX_PDO2_COMM_INDEX, SUBIDX_HIGHEST_SUPPORTED,     {0x05, 0x00, 0x00, 0x00}, DataType::UINT8,  ObjectPermissions::READ},   // Highest sub-index supported
    {TX_PDO2_COMM_INDEX, SUBIDX_PDO_TRANSMISSION_TYPE, {PDO_TXTYPE_ASYNC_DEVICE_PROFILE, 0x00, 0x00, 0x00}, DataType::UINT8,  ObjectPermissions::READ_WRITE},
    {TX_PDO2_COMM_INDEX, SUBIDX_PDO_INHIBIT_TIME,      {0x00, 0x00, 0x00, 0x00}, DataType::UINT16, ObjectPermissions::READ_WRITE},
    {TX_PDO2_COMM_INDEX, SUBIDX_PDO_EVENT_TIMER,       {0xF4, 0x01, 0x00, 0x00}, DataType::UINT16, ObjectPermissions::READ_WRITE}, // 500 ms

    {TX_PDO3_COMM_INDEX, SUBIDX_HIGHEST_SUPPORTED,     {0x05, 0x00, 0x00, 0x00}, DataType::UINT8,  ObjectPermissions::READ},   // Highest sub-index supported
    {TX_PDO3_COMM_INDEX, SUBIDX_PDO_TRANSMISSION_TYPE, {PDO_TXTYPE_ASYNC_DEVICE_PROFILE, 0x00, 0x00, 0x00}, DataType::UINT8,  ObjectPermissions::READ_WRITE},
    {TX_PDO3_COMM_INDEX, SUBIDX_PDO_INHIBIT_TIME,      {0x00, 0x00, 0x00, 0x00}, DataType::UINT16, ObjectPermissions::READ_WRITE},
    {TX_PDO3_COMM_INDEX, SUBIDX_PDO_EVENT_TIMER,       {0xF4, 0x01, 0x00, 0x00}, DataType::UINT16, ObjectPermissions::READ_WRITE}, // 500 ms

    {TX_PDO4_COMM_INDEX, SUBIDX_HIGHEST_SUPPORTED,     {0x05, 0x00, 0x00, 0x00}, DataType::UINT8,  ObjectPermissions::READ},   // Highest sub-index supported
    {TX_PDO4_COMM_INDEX, SUBIDX_PDO_TRANSMISSION_TYPE, {PDO_TXTYPE_ASYNC_DEVICE_PROFILE, 0x00, 0x00, 0x00}, DataType::UINT8,  ObjectPermissions::READ_WRITE},
    {TX_PDO4_COMM_INDEX, SUBIDX_PDO_INHIBIT_TIME,      {0x00, 0x00, 0x00, 0x00}, DataType::UINT16, ObjectPermissions::READ_WRITE},
    {TX_PDO4_COMM_INDEX, SUBIDX_PDO_EVENT_TIMER,       {0xF4, 0x01, 0x00, 0x00}, DataType::UINT16, ObjectPermissions::READ_WRITE}, // 500 ms

    // ------------------------------------------------------------------
    // RPDO Communication Parameters (CiA 301)
    // ------------------------------------------------------------------
    {RX_PDO1_COMM_INDEX, SUBIDX_HIGHEST_SUPPORTED,     {0x05, 0x00, 0x00, 0x00}, DataType::UINT8,  ObjectPermissions::READ},   // Highest sub-index supported
    {RX_PDO1_COMM_INDEX, SUBIDX_PDO_TRANSMISSION_TYPE, {PDO_TXTYPE_ASYNC_DEVICE_PROFILE, 0x00, 0x00, 0x00}, DataType::UINT8,  ObjectPermissions::READ_WRITE},
    {RX_PDO1_COMM_INDEX, SUBIDX_PDO_INHIBIT_TIME,      {0x00, 0x00, 0x00, 0x00}, DataType::UINT16, ObjectPermissions::READ_WRITE},
    {RX_PDO1_COMM_INDEX, SUBIDX_PDO_EVENT_TIMER,       {0x78, 0x05, 0x00, 0x00}, DataType::UINT16, ObjectPermissions::READ_WRITE}, // 1400 ms

    {RX_PDO2_COMM_INDEX, SUBIDX_HIGHEST_SUPPORTED,     {0x05, 0x00, 0x00, 0x00}, DataType::UINT8,  ObjectPermissions::READ},   // Highest sub-index supported
    {RX_PDO2_COMM_INDEX, SUBIDX_PDO_TRANSMISSION_TYPE, {PDO_TXTYPE_ASYNC_DEVICE_PROFILE, 0x00, 0x00, 0x00}, DataType::UINT8,  ObjectPermissions::READ_WRITE},
    {RX_PDO2_COMM_INDEX, SUBIDX_PDO_INHIBIT_TIME,      {0x00, 0x00, 0x00, 0x00}, DataType::UINT16, ObjectPermissions::READ_WRITE},
    {RX_PDO2_COMM_INDEX, SUBIDX_PDO_EVENT_TIMER,       {0x78, 0x05, 0x00, 0x00}, DataType::UINT16, ObjectPermissions::READ_WRITE}, // 1400 ms

    {RX_PDO3_COMM_INDEX, SUBIDX_HIGHEST_SUPPORTED,     {0x05, 0x00, 0x00, 0x00}, DataType::UINT8,  ObjectPermissions::READ},   // Highest sub-index supported
    {RX_PDO3_COMM_INDEX, SUBIDX_PDO_TRANSMISSION_TYPE, {PDO_TXTYPE_ASYNC_DEVICE_PROFILE, 0x00, 0x00, 0x00}, DataType::UINT8,  ObjectPermissions::READ_WRITE},
    {RX_PDO3_COMM_INDEX, SUBIDX_PDO_INHIBIT_TIME,      {0x00, 0x00, 0x00, 0x00}, DataType::UINT16, ObjectPermissions::READ_WRITE},
    {RX_PDO3_COMM_INDEX, SUBIDX_PDO_EVENT_TIMER,       {0x78, 0x05, 0x00, 0x00}, DataType::UINT16, ObjectPermissions::READ_WRITE}, // 1400 ms

    {RX_PDO4_COMM_INDEX, SUBIDX_HIGHEST_SUPPORTED,     {0x05, 0x00, 0x00, 0x00}, DataType::UINT8,  ObjectPermissions::READ},   // Highest sub-index supported
    {RX_PDO4_COMM_INDEX, SUBIDX_PDO_TRANSMISSION_TYPE, {PDO_TXTYPE_ASYNC_DEVICE_PROFILE, 0x00, 0x00, 0x00}, DataType::UINT8,  ObjectPermissions::READ_WRITE},
    {RX_PDO4_COMM_INDEX, SUBIDX_PDO_INHIBIT_TIME,      {0x00, 0x00, 0x00, 0x00}, DataType::UINT16, ObjectPermissions::READ_WRITE},
    {RX_PDO4_COMM_INDEX, SUBIDX_PDO_EVENT_TIMER,       {0x78, 0x05, 0x00, 0x00}, DataType::UINT16, ObjectPermissions::READ_WRITE}, // 1400 ms


    // TPDO mapping parameters examples
    {TX_PDO1_MAPPING_INDEX, SUBIDX_PDO_MAP_COUNT, {0x02, 0x00, 0x00, 0x00}, DataType::UINT8,  ObjectPermissions::READ_WRITE},
    // TPDO1 Mapping -> ACTUAL_VOLTAGE (0x6080:00) + ACTUAL_CURRENT (0x6081:00)
    {TX_PDO1_MAPPING_INDEX, SUBIDX_PDO_MAP_1,     {SIZE_BIT_INT32, SUBIDX_DEFAULT, (uint8_t)(ACTUAL_VOLTAGE & 0xFF), (uint8_t)(ACTUAL_VOLTAGE >> 8)}, DataType::UINT32, ObjectPermissions::READ_WRITE},
    {TX_PDO1_MAPPING_INDEX, SUBIDX_PDO_MAP_2,     {SIZE_BIT_INT32, SUBIDX_DEFAULT, (uint8_t)(ACTUAL_CURRENT & 0xFF), (uint8_t)(ACTUAL_CURRENT >> 8)}, DataType::UINT32, ObjectPermissions::READ_WRITE},

    {TX_PDO2_MAPPING_INDEX, SUBIDX_PDO_MAP_COUNT, {0x03, 0x00, 0x00, 0x00}, DataType::UINT8,  ObjectPermissions::READ_WRITE},
    // TPDO2 Mapping -> ACTUAL_POWER (0x6082:00) + STATUSWORD (0x6041:00) + LOAD_OUTPUT_ENABLE (0x6044:00)
    {TX_PDO1_MAPPING_INDEX, SUBIDX_PDO_MAP_1,     {SIZE_BIT_INT32, SUBIDX_DEFAULT, (uint8_t)(ACTUAL_POWER & 0xFF), (uint8_t)(ACTUAL_POWER >> 8)}, DataType::UINT32, ObjectPermissions::READ_WRITE},
    {TX_PDO2_MAPPING_INDEX, SUBIDX_PDO_MAP_2,     {SIZE_BIT_UINT16, SUBIDX_DEFAULT, (uint8_t)(STATUSWORD & 0xFF), (uint8_t)(STATUSWORD >> 8)}, DataType::UINT32, ObjectPermissions::READ_WRITE},
    {TX_PDO2_MAPPING_INDEX, SUBIDX_PDO_MAP_3,     {SIZE_BIT_UINT8, SUBIDX_DEFAULT, (uint8_t)(LOAD_OUTPUT_ENABLE & 0xFF), (uint8_t)(LOAD_OUTPUT_ENABLE >> 8)}, DataType::UINT32, ObjectPermissions::READ_WRITE},

    {TX_PDO3_MAPPING_INDEX, SUBIDX_PDO_MAP_COUNT, {0x00, 0x00, 0x00, 0x00}, DataType::UINT8,  ObjectPermissions::READ_WRITE},

    {TX_PDO4_MAPPING_INDEX, SUBIDX_PDO_MAP_COUNT, {0x00, 0x00, 0x00, 0x00}, DataType::UINT8,  ObjectPermissions::READ_WRITE},


    // ------------------------------------------------------------------
    // RPDO Mapping Parameters
    // ------------------------------------------------------------------
    // RPDO1 Mapping -> TARGET_CURRENT (0x6070:00, 32-bit = 0x20)
    {RX_PDO1_MAPPING_INDEX, SUBIDX_PDO_MAP_COUNT, {0x01, 0x00, 0x00, 0x00}, DataType::UINT8,  ObjectPermissions::READ_WRITE},
    {RX_PDO1_MAPPING_INDEX, SUBIDX_PDO_MAP_1,     {SIZE_BIT_INT32, SUBIDX_DEFAULT, (uint8_t)(TARGET_CURRENT & 0xFF), (uint8_t)(TARGET_CURRENT >> 8)}, DataType::UINT32, ObjectPermissions::READ_WRITE},

    // RPDO2 Mapping -> LOAD_OUTPUT_ENABLE (0x6044:00, 8-bit = 0x08)
    {RX_PDO2_MAPPING_INDEX, SUBIDX_PDO_MAP_COUNT, {0x01, 0x00, 0x00, 0x00}, DataType::UINT8,  ObjectPermissions::READ_WRITE},
    {RX_PDO2_MAPPING_INDEX, SUBIDX_PDO_MAP_1,     {SIZE_BIT_UINT8, SUBIDX_DEFAULT, (uint8_t)(LOAD_OUTPUT_ENABLE & 0xFF), (uint8_t)(LOAD_OUTPUT_ENABLE >> 8)}, DataType::UINT32, ObjectPermissions::READ_WRITE},

    // RPDO3 Mapping -> CONTROLWORD (0x6040:00, 16-bit = 0x10)
    {RX_PDO3_MAPPING_INDEX, SUBIDX_PDO_MAP_COUNT, {0x01, 0x00, 0x00, 0x00}, DataType::UINT8,  ObjectPermissions::READ_WRITE},
    {RX_PDO3_MAPPING_INDEX, SUBIDX_PDO_MAP_1,     {SIZE_BIT_UINT16, SUBIDX_DEFAULT, (uint8_t)(CONTROLWORD & 0xFF), (uint8_t)(CONTROLWORD >> 8)}, DataType::UINT32, ObjectPermissions::READ_WRITE},

    {RX_PDO4_MAPPING_INDEX, SUBIDX_PDO_MAP_COUNT, {0x00, 0x00, 0x00, 0x00}, DataType::UINT8,  ObjectPermissions::READ_WRITE}
};

constexpr uint8_t NUM_OBJS = sizeof(dictionary) / sizeof(Object);
