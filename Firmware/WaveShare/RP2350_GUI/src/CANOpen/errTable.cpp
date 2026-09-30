#include "emcy.hpp"

constexpr Error errorTable[] = {
//  |   ERROR CODE   |     ERROR REGISTER            |  MANUFACTURER DATA    |
    {CO_EMC_GENERIC,       CO_ERR_REG_GENERIC_ERR,   MFG_ERROR_GENERIC        },
    {CO_EMC_CURRENT,       CO_ERR_REG_CURRENT,       MFG_ERROR_OVERCURRENT    },
    {CO_EMC_VOLTAGE,       CO_ERR_REG_VOLTAGE,       MFG_ERROR_VOLTAGE        },
    {CO_EMC_TEMPERATURE,   CO_ERR_REG_TEMPERATURE,   MFG_ERROR_TEMPERATURE    },
    {CO_EMC_COMMUNICATION, CO_ERR_REG_COMMUNICATION, MFG_ERROR_COMM           },
    {0x8110,               CO_ERR_REG_MANUFACTURER,  MFG_ERROR_DEVICE         }
};

constexpr uint8_t NUM_EMCYS = sizeof(errorTable) / sizeof(Error);
