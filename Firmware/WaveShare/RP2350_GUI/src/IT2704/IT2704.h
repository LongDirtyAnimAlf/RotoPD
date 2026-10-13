/**
 * AP33772S.h  —  Arduino Library for AP33772S USB PD 3.1 Sink Controller
 * ============================================================
 * Based on official CentyLab PicoPD library bit field definitions.
 * Targets: RP2040 (Arduino-Pico), ESP32 (Arduino)
 * Datasheet: DS46176 Rev. 9-2, Feb 2026  |  I2C: 0x52
 * ============================================================
 */

#ifndef IT2704_H
#define IT2704_H

#include <Arduino.h>

#include "../CANOpen/message.hpp"
#include "../CANOpen/pdo.hpp"
#include "../CANOpen/object.hpp"
#include "../CANOpen/canOpenNode.h"

#include "configIT2704.h"


// ─────────────────────────────────────────────────────────────────────────────
//  Main class
// ─────────────────────────────────────────────────────────────────────────────
class IT2704 {
public:
    explicit IT2704(CanOpenNode *node, uint8_t id, uint8_t channel);
    int8_t begin(void);
    void setID(uint8_t id);
    uint8_t getID(void);
    void setChannel(uint8_t channel);
    uint8_t getChannel(void);
    void initDevice(void);
    void setCurrent(uint16_t current);
    void setOutput(bool enabled);
    void collectData(void);
    void getData(uint16_t* I, uint16_t* V, uint32_t* P, uint32_t* E);
private:
    CanOpenNode * _node;
    uint8_t _channel;
    uint8_t _id;
    Object* findObject(uint16_t index, uint8_t subIndex);
};

#endif // IT2704_H