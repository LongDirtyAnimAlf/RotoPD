#include "../../src/CANOpen/object.hpp"
#include "configIT2704.h"

Object dictionaryIT2704[] = {
    // COMMON
    {IT2704_COMMON                   , SUBIDX_HIGHEST_SUPPORTED, {0x0A, 0x00, 0x00, 0x00}, DataType::UINT8,  ObjectPermissions::READ},
    {IT2704_COMMON                   , COMMON_RESET                                           , {0x00, 0x00, 0x00, 0x00}, DataType::INT8  , ObjectPermissions::READ_WRITE},
    {IT2704_COMMON                   , COMMON_POWER_ON_MODE                                   , {0x00, 0x00, 0x00, 0x00}, DataType::INT32 , ObjectPermissions::READ_WRITE},
    {IT2704_COMMON                   , COMMON_SOURCE_SLOPE_TYPE                               , {0x00, 0x00, 0x00, 0x00}, DataType::INT32 , ObjectPermissions::READ_WRITE},
    {IT2704_COMMON                   , COMMON_LOAD_CURRENT_SYMBOL                             , {0x00, 0x00, 0x00, 0x00}, DataType::INT32 , ObjectPermissions::READ_WRITE},
    {IT2704_COMMON                   , COMMON_OUTPUT_COUPLE                                   , {0x00, 0x00, 0x00, 0x00}, DataType::INT32 , ObjectPermissions::READ_WRITE},
    {IT2704_COMMON                   , COMMON_OUTPUT_COUPLE_MODE                              , {0x00, 0x00, 0x00, 0x00}, DataType::INT32 , ObjectPermissions::READ_WRITE},
    {IT2704_COMMON                   , COMMON_OUTPUT_COUPLE_OFFSET                            , {0x00, 0x00, 0x00, 0x00}, DataType::INT32 , ObjectPermissions::READ_WRITE},
    {IT2704_COMMON                   , COMMON_INHIBIT_COUPLE                                  , {0x00, 0x00, 0x00, 0x00}, DataType::INT32 , ObjectPermissions::READ_WRITE},
    {IT2704_COMMON                   , COMMON_INHIBIT_COUPLE_MOE                              , {0x00, 0x00, 0x00, 0x00}, DataType::INT32 , ObjectPermissions::READ_WRITE},
    {IT2704_COMMON                   , COMMON_PROTECTION_COUPLE                               , {0x00, 0x00, 0x00, 0x00}, DataType::INT32 , ObjectPermissions::READ_WRITE},

    // SOURCE_CURRENT
    {IT2704_SOURCE_CURRENT           , SUBIDX_HIGHEST_SUPPORTED, {0x19, 0x00, 0x00, 0x00}, DataType::UINT8,  ObjectPermissions::READ},
    {IT2704_SOURCE_CURRENT           , IT2704_SOURCE_CURRENT_CURRENT_LEVEL                    , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_CURRENT           , IT2704_SOURCE_CURRENT_CURRENT_TRIGGRED                 , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_CURRENT           , IT2704_SOURCE_CURRENT_CURRENT_LIMIT_POSITIVE           , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_CURRENT           , IT2704_SOURCE_CURRENT_CURRENT_LIMIT_NEGATIVE           , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_CURRENT           , IT2704_SOURCE_CURRENT_CURRENT_LIMIT_COUPLE             , {0x00, 0x00, 0x00, 0x00}, DataType::INT8  , ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_CURRENT           , IT2704_SOURCE_CURRENT_CURRENT_MODE                     , {0x00, 0x00, 0x00, 0x00}, DataType::INT32 , ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_CURRENT           , IT2704_SOURCE_CURRENT_CURRENT_RANGE                    , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_CURRENT           , IT2704_SOURCE_CURRENT_CURRENT_SLEW_POSITIVE            , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_CURRENT           , IT2704_SOURCE_CURRENT_CURRENT_SLEW_NEGATIVE            , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_CURRENT           , IT2704_SOURCE_CURRENT_CURRENT_SLEW_TIME_POSITIVE       , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_CURRENT           , IT2704_SOURCE_CURRENT_CURRENT_SLEW_TIME_NEGATIVE       , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_CURRENT           , IT2704_SOURCE_CURRENT_CURRENT_SLEW_COUPLE              , {0x00, 0x00, 0x00, 0x00}, DataType::INT8  , ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_CURRENT           , IT2704_SOURCE_CURRENT_CURRENT_SLEW_POSITIVE_MAX        , {0x00, 0x00, 0x00, 0x00}, DataType::INT8  , ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_CURRENT           , IT2704_SOURCE_CURRENT_CURRENT_SLEW_NEGATIVE_MAX        , {0x00, 0x00, 0x00, 0x00}, DataType::INT8  , ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_CURRENT           , IT2704_SOURCE_CURRENT_SINK_RES_STATE                   , {0x00, 0x00, 0x00, 0x00}, DataType::INT8  , ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_CURRENT           , IT2704_SOURCE_CURRENT_SINK_RES_LEVEL                   , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_CURRENT           , IT2704_SOURCE_CURRENT_OVER_CURRENT_PROTECT_STATE       , {0x00, 0x00, 0x00, 0x00}, DataType::INT8  , ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_CURRENT           , IT2704_SOURCE_CURRENT_OVER_CURRENT_PROTECT_POSITIVE    , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_CURRENT           , IT2704_SOURCE_CURRENT_OVER_CURRENT_PROTECT_NEGATIVE    , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_CURRENT           , IT2704_SOURCE_CURRENT_OVER_CURRENT_PROTECT_DELAY       , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_CURRENT           , IT2704_SOURCE_CURRENT_UNDER_CURRENT_PROTECT_STATE      , {0x00, 0x00, 0x00, 0x00}, DataType::INT8  , ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_CURRENT           , IT2704_SOURCE_CURRENT_UNDER_CURRENT_PROTECT_POSITIVE   , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_CURRENT           , IT2704_SOURCE_CURRENT_UNDER_CURRENT_PROTECT_NEGATIVE   , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_CURRENT           , IT2704_SOURCE_CURRENT_UNDER_CURRENT_PROTECT_DELAY      , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_CURRENT           , IT2704_SOURCE_CURRENT_UNDER_CURRENT_PROTECT_WARM       , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},

    // SOURCE_VOLTAGE
    {IT2704_SOURCE_VOLTAGE           , SUBIDX_HIGHEST_SUPPORTED, {0x1F, 0x00, 0x00, 0x00}, DataType::UINT8,  ObjectPermissions::READ},
    {IT2704_SOURCE_VOLTAGE           , IT2704_SOURCE_VOLTAGE_VOLTAGE_LEVEL                    , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_VOLTAGE           , IT2704_SOURCE_VOLTAGE_VOLTAGE_TRIGGRED                 , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_VOLTAGE           , IT2704_SOURCE_VOLTAGE_VOLTAGE_LIMIT_POSITIVE           , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_VOLTAGE           , IT2704_SOURCE_VOLTAGE_VOLTAGE_LIMIT_NEGATIVE           , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_VOLTAGE           , IT2704_SOURCE_VOLTAGE_VOLTAGE_LIMIT_COUPLE             , {0x00, 0x00, 0x00, 0x00}, DataType::INT8  , ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_VOLTAGE           , IT2704_SOURCE_VOLTAGE_VOLTAGE_HIGH                     , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_VOLTAGE           , IT2704_SOURCE_VOLTAGE_VOLTAGE_LOW                      , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_VOLTAGE           , IT2704_SOURCE_VOLTAGE_VOLTAGE_MODE                     , {0x00, 0x00, 0x00, 0x00}, DataType::INT32 , ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_VOLTAGE           , IT2704_SOURCE_VOLTAGE_VOLTAGE_RANGE                    , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_VOLTAGE           , IT2704_SOURCE_VOLTAGE_VOLTAGE_SLEW_POSITIVE            , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_VOLTAGE           , IT2704_SOURCE_VOLTAGE_VOLTAGE_SLEW_NEGATIVE            , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_VOLTAGE           , IT2704_SOURCE_VOLTAGE_VOLTAGE_SLEW_TIME_POSITIVE       , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_VOLTAGE           , IT2704_SOURCE_VOLTAGE_VOLTAGE_SLEW_TIME_NEGATIVE       , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_VOLTAGE           , IT2704_SOURCE_VOLTAGE_VOLTAGE_SLEW_COUPLE              , {0x00, 0x00, 0x00, 0x00}, DataType::INT8  , ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_VOLTAGE           , IT2704_SOURCE_VOLTAGE_VOLTAGE_SLEW_POSITIVE_MAX        , {0x00, 0x00, 0x00, 0x00}, DataType::INT8  , ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_VOLTAGE           , IT2704_SOURCE_VOLTAGE_VOLTAGE_SLEW_NEGATIVE_MAX        , {0x00, 0x00, 0x00, 0x00}, DataType::INT8  , ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_VOLTAGE           , IT2704_SOURCE_VOLTAGE_INTER_RESISTANCE_STATE           , {0x00, 0x00, 0x00, 0x00}, DataType::INT8  , ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_VOLTAGE           , IT2704_SOURCE_VOLTAGE_INTER_RESISTANCE_LEVEL           , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_VOLTAGE           , IT2704_SOURCE_VOLTAGE_BSIM_LEVEL                       , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_VOLTAGE           , IT2704_SOURCE_VOLTAGE_INHIBIT_MODE                     , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_VOLTAGE           , IT2704_SOURCE_VOLTAGE_INHIBIT_VON                      , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_VOLTAGE           , IT2704_SOURCE_VOLTAGE_INHIBIT_VOFF                     , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_VOLTAGE           , IT2704_SOURCE_VOLTAGE_OVER_VOLTAGE_PROTECTON_STATE     , {0x00, 0x00, 0x00, 0x00}, DataType::INT8  , ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_VOLTAGE           , IT2704_SOURCE_VOLTAGE_OVER_VOLTAGE_PROTECTON_POSITIVE  , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_VOLTAGE           , IT2704_SOURCE_VOLTAGE_OVER_VOLTAGE_PROTECTON_NEGATIVE  , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_VOLTAGE           , IT2704_SOURCE_VOLTAGE_OVER_VOLTAGE_PROTECTON_DELAY     , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_VOLTAGE           , IT2704_SOURCE_VOLTAGE_UNDER_VOLTAGE_PROTECTION_STATE   , {0x00, 0x00, 0x00, 0x00}, DataType::INT8  , ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_VOLTAGE           , IT2704_SOURCE_VOLTAGE_UNDER_VOLTAGE_PROTECTION_POSITIVE, {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_VOLTAGE           , IT2704_SOURCE_VOLTAGE_UNDER_VOLTAGE_PROTECTION_NEGATIVE, {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_VOLTAGE           , IT2704_SOURCE_VOLTAGE_UNDER_VOLTAGE_PROTECTION_DELAY   , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_VOLTAGE           , IT2704_SOURCE_VOLTAGE_UNDER_VOLTAGE_PROTECTION_WARM    , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},

    // SOURCE_POWER
    {IT2704_SOURCE_POWER             , SUBIDX_HIGHEST_SUPPORTED, {0x0E, 0x00, 0x00, 0x00}, DataType::UINT8,  ObjectPermissions::READ},
    {IT2704_SOURCE_POWER             , IT2704_SOURCE_POWER_POWER_LEVEL                        , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_POWER             , IT2704_SOURCE_POWER_POWER_TRIGGRED                     , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_POWER             , IT2704_SOURCE_POWER_POWER_LIMIT_POSITIVE               , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_POWER             , IT2704_SOURCE_POWER_POWER_LIMIT_NEGATIVE               , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_POWER             , IT2704_SOURCE_POWER_POWER_LIMIT_COUPLE                 , {0x00, 0x00, 0x00, 0x00}, DataType::INT8  , ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_POWER             , IT2704_SOURCE_POWER_POWER_SLEW_POSITIVE                , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_POWER             , IT2704_SOURCE_POWER_POWER_SLEW_NEGATIVE                , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_POWER             , IT2704_SOURCE_POWER_POWER_SLEW_COUPLE                  , {0x00, 0x00, 0x00, 0x00}, DataType::INT8  , ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_POWER             , IT2704_SOURCE_POWER_POWER_SLEW_POSITIVE_MAX            , {0x00, 0x00, 0x00, 0x00}, DataType::INT8  , ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_POWER             , IT2704_SOURCE_POWER_POWER_SLEW_NEGATIVE_MAX            , {0x00, 0x00, 0x00, 0x00}, DataType::INT8  , ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_POWER             , IT2704_SOURCE_POWER_OVER_POWER_PROTECT_STATE           , {0x00, 0x00, 0x00, 0x00}, DataType::INT8  , ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_POWER             , IT2704_SOURCE_POWER_OVER_POWER_PROTECT_POSITIVE        , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_POWER             , IT2704_SOURCE_POWER_OVER_POWER_PROTECT_NEGATIVE        , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_POWER             , IT2704_SOURCE_POWER_OVER_POWER_PROTECT_DELAY           , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},

    // SOURCE_RESISTANCE
    {IT2704_SOURCE_RESISTANCE        , SUBIDX_HIGHEST_SUPPORTED, {0x07, 0x00, 0x00, 0x00}, DataType::UINT8,  ObjectPermissions::READ},
    {IT2704_SOURCE_RESISTANCE        , IT2704_SOURCE_RESISTANCE_RESISTANCE_LEVEL              , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_RESISTANCE        , IT2704_SOURCE_RESISTANCE_RESISTANCE_TRIGGRED           , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_RESISTANCE        , IT2704_SOURCE_RESISTANCE_RESISTANCE_SLEW_POSITIVE      , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_RESISTANCE        , IT2704_SOURCE_RESISTANCE_RESISTANCE_SLEW_NEGATIVE      , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_RESISTANCE        , IT2704_SOURCE_RESISTANCE_RESISTANCE_COUPLE             , {0x00, 0x00, 0x00, 0x00}, DataType::INT8  , ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_RESISTANCE        , IT2704_SOURCE_RESISTANCE_RESISTANCE_SLEW_POSITIVE_MAX  , {0x00, 0x00, 0x00, 0x00}, DataType::INT8  , ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_RESISTANCE        , IT2704_SOURCE_RESISTANCE_RESISTANCE_SLEW_NEGATIVE_MAX  , {0x00, 0x00, 0x00, 0x00}, DataType::INT8  , ObjectPermissions::READ_WRITE},

    // SOURCE_FUNCTION
    {IT2704_SOURCE_FUNCTION          , SUBIDX_HIGHEST_SUPPORTED, {0x01, 0x00, 0x00, 0x00}, DataType::UINT8,  ObjectPermissions::READ},
    {IT2704_SOURCE_FUNCTION          , IT2704_SOURCE_FUNCTION_FUNCTION                        , {0x00, 0x00, 0x00, 0x00}, DataType::INT32 , ObjectPermissions::READ_WRITE},

    // SOURCE_FILE
    {IT2704_SOURCE_FILE              , SUBIDX_HIGHEST_SUPPORTED, {0x03, 0x00, 0x00, 0x00}, DataType::UINT8,  ObjectPermissions::READ},
    {IT2704_SOURCE_FILE              , IT2704_SOURCE_FILE_SAVE_BANK                           , {0x00, 0x00, 0x00, 0x00}, DataType::INT32 , ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_FILE              , IT2704_SOURCE_FILE_RECALL_BANK                         , {0x00, 0x00, 0x00, 0x00}, DataType::INT32 , ObjectPermissions::READ_WRITE},
    {IT2704_SOURCE_FILE              , IT2704_SOURCE_FILE_DELETE_BANCK                        , {0x00, 0x00, 0x00, 0x00}, DataType::INT32 , ObjectPermissions::READ_WRITE},

    // OUTPUT
    {IT2704_OUTPUT                   , SUBIDX_HIGHEST_SUPPORTED, {0x0A, 0x00, 0x00, 0x00}, DataType::UINT8,  ObjectPermissions::READ},
    {IT2704_OUTPUT                   , IT2704_OUTPUT_ONOFF_STATE                              , {0x00, 0x00, 0x00, 0x00}, DataType::INT8  , ObjectPermissions::READ_WRITE},
    {IT2704_OUTPUT                   , IT2704_OUTPUT_PROTECT_CLEAR                            , {0x00, 0x00, 0x00, 0x00}, DataType::INT8  , ObjectPermissions::READ_WRITE},
    {IT2704_OUTPUT                   , IT2704_OUTPUT_SHORT_MODE                               , {0x00, 0x00, 0x00, 0x00}, DataType::INT32 , ObjectPermissions::READ_WRITE},
    {IT2704_OUTPUT                   , IT2704_OUTPUT_RISE_DELAY                               , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_OUTPUT                   , IT2704_OUTPUT_FALL_DELAY                               , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_OUTPUT                   , IT2704_OUTPUT_REGULAR_SPEED                            , {0x00, 0x00, 0x00, 0x00}, DataType::INT32 , ObjectPermissions::READ_WRITE},
    {IT2704_OUTPUT                   , IT2704_OUTPUT_RELAY_LOCK_STATE                         , {0x00, 0x00, 0x00, 0x00}, DataType::INT8  , ObjectPermissions::READ_WRITE},
    {IT2704_OUTPUT                   , IT2704_OUTPUT_VOLTAGE_RETURN_ZERO_LEVEL                , {0x00, 0x00, 0x00, 0x00}, DataType::INT8  , ObjectPermissions::READ_WRITE},
    {IT2704_OUTPUT                   , IT2704_OUTPUT_FOLDBACK_MODE                            , {0x00, 0x00, 0x00, 0x00}, DataType::INT32 , ObjectPermissions::READ_WRITE},
    {IT2704_OUTPUT                   , IT2704_OUTPUT_FOLDBACK_DELAY                           , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},

    // SENSE
    {IT2704_SENSE                    , SUBIDX_HIGHEST_SUPPORTED, {0x0A, 0x00, 0x00, 0x00}, DataType::UINT8,  ObjectPermissions::READ},
    {IT2704_SENSE                    , IT2704_SENSE_SENSE_STATE                               , {0x00, 0x00, 0x00, 0x00}, DataType::INT8  , ObjectPermissions::READ_WRITE},
    {IT2704_SENSE                    , IT2704_SENSE_SENSE_CURRENT_RANGE                       , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SENSE                    , IT2704_SENSE_SENSE_CURRENT_AUTO_RANGE                  , {0x00, 0x00, 0x00, 0x00}, DataType::INT8  , ObjectPermissions::READ_WRITE},
    {IT2704_SENSE                    , IT2704_SENSE_SENSE_VOLTAGE_RANGE                       , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SENSE                    , IT2704_SENSE_SENSE_VOLTAGE_AUTO_RANGE                  , {0x00, 0x00, 0x00, 0x00}, DataType::INT8  , ObjectPermissions::READ_WRITE},
    {IT2704_SENSE                    , IT2704_SENSE_SENSE_SWEEP_POINTS                        , {0x00, 0x00, 0x00, 0x00}, DataType::INT32 , ObjectPermissions::READ_WRITE},
    {IT2704_SENSE                    , IT2704_SENSE_SENSE_SWEEP_OFFSET_POINTS                 , {0x00, 0x00, 0x00, 0x00}, DataType::INT32 , ObjectPermissions::READ_WRITE},
    {IT2704_SENSE                    , IT2704_SENSE_SENSE_TIME_INTERVAL                       , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SENSE                    , IT2704_SENSE_SENSE_APERTURE                            , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},
    {IT2704_SENSE                    , IT2704_SENSE_SENSE_NPLC                                , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ_WRITE},

    // STATUS
    {IT2704_STATUS                   , SUBIDX_HIGHEST_SUPPORTED, {0x02, 0x00, 0x00, 0x00}, DataType::UINT8,  ObjectPermissions::READ},
    {IT2704_STATUS                   , IT2704_STATUS_OPERATION_REGISTER                       , {0x00, 0x00, 0x00, 0x00}, DataType::INT32 , ObjectPermissions::READ},
    {IT2704_STATUS                   , IT2704_STATUS_QUESTIONABLE_REGISTER                    , {0x00, 0x00, 0x00, 0x00}, DataType::INT32 , ObjectPermissions::READ},

    // METER
    {IT2704_METER                    , SUBIDX_HIGHEST_SUPPORTED, {0x0E, 0x00, 0x00, 0x00}, DataType::UINT8,  ObjectPermissions::READ},
    {IT2704_METER                    , IT2704_METER_VOLTAGE                                   , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ},
    {IT2704_METER                    , IT2704_METER_CURRENT                                   , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ},
    {IT2704_METER                    , IT2704_METER_POWER                                     , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ},
    {IT2704_METER                    , IT2704_METER_CURRENT_ACDC                              , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ},
    {IT2704_METER                    , IT2704_METER_VOLTAGE_ACDC                              , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ},
    {IT2704_METER                    , IT2704_METER_CURRENT_MAX                               , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ},
    {IT2704_METER                    , IT2704_METER_CURRENT_MIN                               , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ},
    {IT2704_METER                    , IT2704_METER_VOLTAGE_MAX                               , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ},
    {IT2704_METER                    , IT2704_METER_VOLTAGE_MIN                               , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ},
    {IT2704_METER                    , IT2704_METER_POWER_MAX                                 , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ},
    {IT2704_METER                    , IT2704_METER_POWER_MIN                                 , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ},
    {IT2704_METER                    , IT2704_METER_AHOUR                                     , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ},
    {IT2704_METER                    , IT2704_METER_WHOUR                                     , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ},
    {IT2704_METER                    , IT2704_METER_ON_TIME                                   , {0x00, 0x00, 0x00, 0x00}, DataType::FLOAT32, ObjectPermissions::READ},


    // ------------------------------------------------------------------
    // Device specific TPDO mapping parameters
    // ------------------------------------------------------------------

    // TPDO1 Mapping (0x1A00) - Channel 1..8 Onoff State
    {TX_PDO1_MAPPING_INDEX, SUBIDX_PDO_MAP_COUNT, {0x08, 0x00, 0x00, 0x00}, DataType::UINT8,  ObjectPermissions::READ_WRITE},
    {TX_PDO1_MAPPING_INDEX, SUBIDX_PDO_MAP_1,     {SIZE_BIT_INT8,  SUBIDX_1,       (uint8_t)(IT2704_OUTPUT & 0xFF), (uint8_t)(IT2704_OUTPUT >> 8)}, DataType::UINT32, ObjectPermissions::READ_WRITE},
    {TX_PDO1_MAPPING_INDEX, SUBIDX_PDO_MAP_2,     {SIZE_BIT_INT8,  SUBIDX_1,       (uint8_t)((IT2704_OUTPUT + 0x100) & 0xFF), (uint8_t)((IT2704_OUTPUT + 0x100) >> 8)}, DataType::UINT32, ObjectPermissions::READ_WRITE},
    {TX_PDO1_MAPPING_INDEX, SUBIDX_PDO_MAP_3,     {SIZE_BIT_INT8,  SUBIDX_1,       (uint8_t)((IT2704_OUTPUT + 0x200) & 0xFF), (uint8_t)((IT2704_OUTPUT + 0x200) >> 8)}, DataType::UINT32, ObjectPermissions::READ_WRITE},
    {TX_PDO1_MAPPING_INDEX, SUBIDX_PDO_MAP_4,     {SIZE_BIT_INT8,  SUBIDX_1,       (uint8_t)((IT2704_OUTPUT + 0x300) & 0xFF), (uint8_t)((IT2704_OUTPUT + 0x300) >> 8)}, DataType::UINT32, ObjectPermissions::READ_WRITE},
    {TX_PDO1_MAPPING_INDEX, SUBIDX_PDO_MAP_5,     {SIZE_BIT_INT8,  SUBIDX_1,       (uint8_t)((IT2704_OUTPUT + 0x400) & 0xFF), (uint8_t)((IT2704_OUTPUT + 0x400) >> 8)}, DataType::UINT32, ObjectPermissions::READ_WRITE},
    {TX_PDO1_MAPPING_INDEX, SUBIDX_PDO_MAP_6,     {SIZE_BIT_INT8,  SUBIDX_1,       (uint8_t)((IT2704_OUTPUT + 0x500) & 0xFF), (uint8_t)((IT2704_OUTPUT + 0x500) >> 8)}, DataType::UINT32, ObjectPermissions::READ_WRITE},
    {TX_PDO1_MAPPING_INDEX, SUBIDX_PDO_MAP_7,     {SIZE_BIT_INT8,  SUBIDX_1,       (uint8_t)((IT2704_OUTPUT + 0x600) & 0xFF), (uint8_t)((IT2704_OUTPUT + 0x600) >> 8)}, DataType::UINT32, ObjectPermissions::READ_WRITE},
    {TX_PDO1_MAPPING_INDEX, SUBIDX_PDO_MAP_8,     {SIZE_BIT_INT8,  SUBIDX_1,       (uint8_t)((IT2704_OUTPUT + 0x700) & 0xFF), (uint8_t)((IT2704_OUTPUT + 0x700) >> 8)}, DataType::UINT32, ObjectPermissions::READ_WRITE},

    // TPDO2 Mapping (0x1A01) - Operation Register & Questionable Register
    {TX_PDO2_MAPPING_INDEX, SUBIDX_PDO_MAP_COUNT, {0x02, 0x00, 0x00, 0x00}, DataType::UINT8,  ObjectPermissions::READ_WRITE},
    {TX_PDO2_MAPPING_INDEX, SUBIDX_PDO_MAP_1,     {SIZE_BIT_INT32, IT2704_STATUS_OPERATION_REGISTER,       (uint8_t)(IT2704_STATUS & 0xFF), (uint8_t)(IT2704_STATUS >> 8)}, DataType::UINT32, ObjectPermissions::READ_WRITE},
    {TX_PDO2_MAPPING_INDEX, SUBIDX_PDO_MAP_2,     {SIZE_BIT_INT32, IT2704_STATUS_QUESTIONABLE_REGISTER,    (uint8_t)(IT2704_STATUS & 0xFF), (uint8_t)(IT2704_STATUS >> 8)}, DataType::UINT32, ObjectPermissions::READ_WRITE},

    // TPDO3 Mapping (0x1A02) - Meter Voltage & Meter Current
    {TX_PDO3_MAPPING_INDEX, SUBIDX_PDO_MAP_COUNT, {0x02, 0x00, 0x00, 0x00}, DataType::UINT8,  ObjectPermissions::READ_WRITE},
    {TX_PDO3_MAPPING_INDEX, SUBIDX_PDO_MAP_1,     {SIZE_BIT_INT32, IT2704_METER_VOLTAGE,       (uint8_t)(IT2704_METER & 0xFF),  (uint8_t)(IT2704_METER >> 8)}, DataType::UINT32, ObjectPermissions::READ_WRITE},
    {TX_PDO3_MAPPING_INDEX, SUBIDX_PDO_MAP_2,     {SIZE_BIT_INT32, IT2704_METER_CURRENT,       (uint8_t)(IT2704_METER & 0xFF),  (uint8_t)(IT2704_METER >> 8)}, DataType::UINT32, ObjectPermissions::READ_WRITE},

    // TPDO4 Mapping (0x1A03) - Channel 1..8 Sense State
    {TX_PDO4_MAPPING_INDEX, SUBIDX_PDO_MAP_COUNT, {0x08, 0x00, 0x00, 0x00}, DataType::UINT8,  ObjectPermissions::READ_WRITE},
    {TX_PDO4_MAPPING_INDEX, SUBIDX_PDO_MAP_1,     {SIZE_BIT_INT8,  SUBIDX_1,       (uint8_t)(IT2704_SENSE & 0xFF),  (uint8_t)(IT2704_SENSE >> 8)}, DataType::UINT32, ObjectPermissions::READ_WRITE},
    {TX_PDO4_MAPPING_INDEX, SUBIDX_PDO_MAP_2,     {SIZE_BIT_INT8,  SUBIDX_1,       (uint8_t)((IT2704_SENSE + 0x100) & 0xFF), (uint8_t)((IT2704_SENSE + 0x100) >> 8)}, DataType::UINT32, ObjectPermissions::READ_WRITE},
    {TX_PDO4_MAPPING_INDEX, SUBIDX_PDO_MAP_3,     {SIZE_BIT_INT8,  SUBIDX_1,       (uint8_t)((IT2704_SENSE + 0x200) & 0xFF), (uint8_t)((IT2704_SENSE + 0x200) >> 8)}, DataType::UINT32, ObjectPermissions::READ_WRITE},
    {TX_PDO4_MAPPING_INDEX, SUBIDX_PDO_MAP_4,     {SIZE_BIT_INT8,  SUBIDX_1,       (uint8_t)((IT2704_SENSE + 0x300) & 0xFF), (uint8_t)((IT2704_SENSE + 0x300) >> 8)}, DataType::UINT32, ObjectPermissions::READ_WRITE},
    {TX_PDO4_MAPPING_INDEX, SUBIDX_PDO_MAP_5,     {SIZE_BIT_INT8,  SUBIDX_1,       (uint8_t)((IT2704_SENSE + 0x400) & 0xFF), (uint8_t)((IT2704_SENSE + 0x400) >> 8)}, DataType::UINT32, ObjectPermissions::READ_WRITE},
    {TX_PDO4_MAPPING_INDEX, SUBIDX_PDO_MAP_6,     {SIZE_BIT_INT8,  SUBIDX_1,       (uint8_t)((IT2704_SENSE + 0x500) & 0xFF), (uint8_t)((IT2704_SENSE + 0x500) >> 8)}, DataType::UINT32, ObjectPermissions::READ_WRITE},
    {TX_PDO4_MAPPING_INDEX, SUBIDX_PDO_MAP_7,     {SIZE_BIT_INT8,  SUBIDX_1,       (uint8_t)((IT2704_SENSE + 0x600) & 0xFF), (uint8_t)((IT2704_SENSE + 0x600) >> 8)}, DataType::UINT32, ObjectPermissions::READ_WRITE},
    {TX_PDO4_MAPPING_INDEX, SUBIDX_PDO_MAP_8,     {SIZE_BIT_INT8,  SUBIDX_1,       (uint8_t)((IT2704_SENSE + 0x700) & 0xFF), (uint8_t)((IT2704_SENSE + 0x700) >> 8)}, DataType::UINT32, ObjectPermissions::READ_WRITE}
};

constexpr uint8_t NUM_OBJS_IT2704 = sizeof(dictionaryIT2704) / sizeof(Object);
