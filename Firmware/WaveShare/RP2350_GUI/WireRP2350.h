#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "shared.h"
#include "./src/bsp/bsp_i2c.h"

class TwoWireRP2350 : public TwoWire {
public:
    TwoWireRP2350() :
        TwoWire(BSP_I2C_NUM, BSP_I2C_SDA_PIN, BSP_I2C_SCL_PIN),
        _addr(0),
        _tx_len(0),
        _rx_len(0),
        _rx_index(0),
        _last_reg(0),
        _has_reg(false) {}

    void begin() override {
        bsp_i2c_init();
    }

    void begin(uint8_t address) override {
        (void)address;
        bsp_i2c_init();
    }

    void end() override {}

    void setClock(uint32_t frequency) override {
        (void)frequency;
    }

    void beginTransmission(uint8_t address) override {
        _addr = address;
        _tx_len = 0;
        _has_reg = false;
    }

    size_t write(uint8_t data) override {
        if (_tx_len < sizeof(_tx_buffer)) {
            _tx_buffer[_tx_len++] = data;
            return 1;
        }
        return 0;
    }

    size_t write(const uint8_t *data, size_t quantity) override {
        size_t written = 0;
        while (written < quantity && _tx_len < sizeof(_tx_buffer)) {
            _tx_buffer[_tx_len++] = data[written++];
        }
        return written;
    }

    uint8_t endTransmission(bool sendStop = true) override {
        (void)sendStop;

        if (_tx_len == 0) return 0;

        if (_tx_len == 1) {
            _last_reg = _tx_buffer[0];
            _has_reg = true;
            bsp_i2c_write(_addr, _tx_buffer, 1);
        } else {
            bsp_i2c_write_reg8(_addr, _tx_buffer[0], &_tx_buffer[1], _tx_len - 1);
            _has_reg = false;
        }

        _tx_len = 0;
        return 0;   // 0 = success
    }

    // Exact signature required by the rp2040 core
    size_t requestFrom(uint8_t address, size_t quantity, bool stopBit) override {
        (void)stopBit;
        _addr = address;

        if (quantity > sizeof(_rx_buffer))
            quantity = sizeof(_rx_buffer);

        if (_has_reg) {
            bsp_i2c_read_reg8(_addr, _last_reg, _rx_buffer, quantity);
            _has_reg = false;
        } else {
            bsp_i2c_read_reg8(_addr, 0, _rx_buffer, quantity);
        }

        _rx_len = quantity;
        _rx_index = 0;
        return _rx_len;
    }

    // Convenience overloads
    size_t requestFrom(uint8_t address, size_t quantity) {
        return requestFrom(address, quantity, true);
    }

    size_t requestFrom(uint8_t address, uint8_t quantity, uint8_t sendStop) {
        return requestFrom(address, (size_t)quantity, sendStop != 0);
    }

    size_t requestFrom(uint8_t address, uint8_t quantity) {
        return requestFrom(address, (size_t)quantity, true);
    }

    int available() override {
        return (int)_rx_len - (int)_rx_index;
    }

    int read() override {
        if (_rx_index < _rx_len)
            return _rx_buffer[_rx_index++];
        return -1;
    }

    int peek() override {
        if (_rx_index < _rx_len)
            return _rx_buffer[_rx_index];
        return -1;
    }

    void flush() override {
        _tx_len = 0;
        _rx_len = 0;
        _rx_index = 0;
        _has_reg = false;
    }

private:
    uint8_t _addr;
    uint8_t _tx_buffer[COMMAND_SIZE];
    uint8_t _tx_len;

    uint8_t _rx_buffer[COMMAND_SIZE];
    uint8_t _rx_len;
    uint8_t _rx_index;

    uint8_t _last_reg;
    bool    _has_reg;
};

// Global instance
static TwoWireRP2350 WireRP2350;
