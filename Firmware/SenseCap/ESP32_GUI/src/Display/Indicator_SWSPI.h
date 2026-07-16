/*
 * start rewrite from:
 * https://github.com/adafruit/Adafruit-GFX-Library.git
 */
#ifndef _Indicator_SWSPI_H_
#define _Indicator_SWSPI_H_

#include "Arduino_DataBus.h"

static const uint8_t st7701_indicator_init_operations[] = {
  BEGIN_WRITE,
  WRITE_COMMAND_8, 0xFF,
  WRITE_BYTES, 5, 0x77, 0x01, 0x00, 0x00, 0x10,

  WRITE_C8_D16, 0xC0, 0x3B, 0x00,
  WRITE_C8_D16, 0xC1, 0x0D, 0x02,
  WRITE_C8_D16, 0xC2, 0x31, 0x05,
  WRITE_C8_D8, 0xCD, 0x08,

  WRITE_COMMAND_8, 0xB0, // Positive Voltage Gamma Control
  WRITE_BYTES, 16,
  0x00, 0x11, 0x18, 0x0E,
  0x11, 0x06, 0x07, 0x08,
  0x07, 0x22, 0x04, 0x12,
  0x0F, 0xAA, 0x31, 0x18,

  WRITE_COMMAND_8, 0xB1, // Negative Voltage Gamma Control
  WRITE_BYTES, 16,
  0x00, 0x11, 0x19, 0x0E,
  0x12, 0x07, 0x08, 0x08,
  0x08, 0x22, 0x04, 0x11,
  0x11, 0xA9, 0x32, 0x18,

  // PAGE1
  WRITE_COMMAND_8, 0xFF,
  WRITE_BYTES, 5, 0x77, 0x01, 0x00, 0x00, 0x11,

  WRITE_C8_D8, 0xB0, 0x60, // Vop=4.7375v
  WRITE_C8_D8, 0xB1, 0x32, // VCOM=32
  WRITE_C8_D8, 0xB2, 0x07, // VGH=15v
  WRITE_C8_D8, 0xB3, 0x80,
  WRITE_C8_D8, 0xB5, 0x49, // VGL=-10.17v
  WRITE_C8_D8, 0xB7, 0x85,
  WRITE_C8_D8, 0xB8, 0x21, // AVDD=6.6 & AVCL=-4.6
  WRITE_C8_D8, 0xC1, 0x78,
  WRITE_C8_D8, 0xC2, 0x78,

  WRITE_COMMAND_8, 0xE0,
  WRITE_BYTES, 3, 0x00, 0x1B, 0x02,

  WRITE_COMMAND_8, 0xE1,
  WRITE_BYTES, 11,
  0x08, 0xA0, 0x00, 0x00,
  0x07, 0xA0, 0x00, 0x00,
  0x00, 0x44, 0x44,

  WRITE_COMMAND_8, 0xE2,
  WRITE_BYTES, 12,
  0x11, 0x11, 0x44, 0x44,
  0xED, 0xA0, 0x00, 0x00,
  0xEC, 0xA0, 0x00, 0x00,

  WRITE_COMMAND_8, 0xE3,
  WRITE_BYTES, 4, 0x00, 0x00, 0x11, 0x11,

  WRITE_C8_D16, 0xE4, 0x44, 0x44,

  WRITE_COMMAND_8, 0xE5,
  WRITE_BYTES, 16,
  0x0A, 0xE9, 0xD8, 0xA0,
  0x0C, 0xEB, 0xD8, 0xA0,
  0x0E, 0xED, 0xD8, 0xA0,
  0x10, 0xEF, 0xD8, 0xA0,

  WRITE_COMMAND_8, 0xE6,
  WRITE_BYTES, 4, 0x00, 0x00, 0x11, 0x11,

  WRITE_C8_D16, 0xE7, 0x44, 0x44,

  WRITE_COMMAND_8, 0xE8,
  WRITE_BYTES, 16,
  0x09, 0xE8, 0xD8, 0xA0,
  0x0B, 0xEA, 0xD8, 0xA0,
  0x0D, 0xEC, 0xD8, 0xA0,
  0x0F, 0xEE, 0xD8, 0xA0,

  WRITE_COMMAND_8, 0xEB,
  WRITE_BYTES, 7,
  0x02, 0x00, 0xE4, 0xE4,
  0x88, 0x00, 0x40,

  WRITE_C8_D16, 0xEC, 0x3C, 0x00,

  WRITE_COMMAND_8, 0xED,
  WRITE_BYTES, 16,
  0xAB, 0x89, 0x76, 0x54,
  0x02, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0x20,
  0x45, 0x67, 0x98, 0xBA,

  //-----------VAP & VAN---------------
  WRITE_COMMAND_8, 0xFF,
  WRITE_BYTES, 5, 0x77, 0x01, 0x00, 0x00, 0x13,

  WRITE_C8_D8, 0xE5, 0xE4,

  WRITE_COMMAND_8, 0xFF,
  WRITE_BYTES, 5, 0x77, 0x01, 0x00, 0x00, 0x00,

  // Mirror Y
  WRITE_COMMAND_8, 0xFF,
  WRITE_BYTES, 5, 0x77, 0x01, 0x00, 0x00, 0x10,
  WRITE_C8_D8, 0xC7, 0x04,

  // Mirror X
  WRITE_COMMAND_8, 0xFF,
  WRITE_BYTES, 5, 0x77, 0x01, 0x00, 0x00, 0x00,
  WRITE_C8_D8, 0x36, 0x10,

  WRITE_COMMAND_8, 0x21,   // 0x20 normal, 0x21 IPS
  WRITE_C8_D8, 0x3A, 0x60, // 0x70 RGB888, 0x60 RGB666, 0x50 RGB565

  WRITE_COMMAND_8, 0x11, // Sleep Out
  END_WRITE,

  DELAY, 120,

  BEGIN_WRITE,
  WRITE_COMMAND_8, 0x29, // Display On
  END_WRITE};

class Indicator_SWSPI : public Arduino_DataBus
{
public:
  Indicator_SWSPI(int8_t dc, int8_t cs, int8_t _sck, int8_t _mosi, int8_t _miso = GFX_NOT_DEFINED); // Constructor

  bool begin(int32_t speed = GFX_NOT_DEFINED, int8_t dataMode = GFX_NOT_DEFINED) override;
  void beginWrite() override;
  void endWrite() override;
  void writeCommand(uint8_t) override;
  void writeCommand16(uint16_t) override;
  void writeCommandBytes(uint8_t *data, uint32_t len) override;
  void write(uint8_t) override;
  void write16(uint16_t) override;
  void writeRepeat(uint16_t p, uint32_t len) override;
  void writePixels(uint16_t *data, uint32_t len) override;

#if !defined(LITTLE_FOOT_PRINT)
  void writeBytes(uint8_t *data, uint32_t len) override;
#endif // !defined(LITTLE_FOOT_PRINT)

private:
  GFX_INLINE void WRITE9BITCOMMAND(uint8_t c);
  GFX_INLINE void WRITE9BITDATA(uint8_t d);
  GFX_INLINE void WRITE(uint8_t d);
  GFX_INLINE void WRITE16(uint16_t d);
  GFX_INLINE void WRITE9BITREPEAT(uint16_t p, uint32_t len);
  GFX_INLINE void WRITEREPEAT(uint16_t p, uint32_t len);
  GFX_INLINE void DC_HIGH(void);
  GFX_INLINE void DC_LOW(void);
  GFX_INLINE void CS_HIGH(void);
  GFX_INLINE void CS_LOW(void);
  GFX_INLINE void SPI_MOSI_HIGH(void);
  GFX_INLINE void SPI_MOSI_LOW(void);
  GFX_INLINE void SPI_SCK_HIGH(void);
  GFX_INLINE void SPI_SCK_LOW(void);
  GFX_INLINE bool SPI_MISO_READ(void);

  int8_t _dc, _cs;
  int8_t _sck, _mosi, _miso;

  // CLASS INSTANCE VARIABLES --------------------------------------------

  // Here be dragons! There's a big union of three structures here --
  // one each for hardware SPI, software (bitbang) SPI, and parallel
  // interfaces. This is to save some memory, since a display's connection
  // will be only one of these. The order of some things is a little weird
  // in an attempt to get values to align and pack better in RAM.

#if defined(USE_FAST_PINIO)
#if defined(HAS_PORT_SET_CLR)
  PORTreg_t _csPortSet;   ///< PORT register for chip select SET
  PORTreg_t _csPortClr;   ///< PORT register for chip select CLEAR
  PORTreg_t _dcPortSet;   ///< PORT register for data/command SET
  PORTreg_t _dcPortClr;   ///< PORT register for data/command CLEAR
  PORTreg_t _mosiPortSet; ///< PORT register for MOSI SET
  PORTreg_t _mosiPortClr; ///< PORT register for MOSI CLEAR
  PORTreg_t _sckPortSet;  ///< PORT register for SCK SET
  PORTreg_t _sckPortClr;  ///< PORT register for SCK CLEAR
#if !defined(KINETISK)
  ARDUINOGFX_PORT_t _csPinMask;   ///< Bitmask for chip select
  ARDUINOGFX_PORT_t _dcPinMask;   ///< Bitmask for data/command
  ARDUINOGFX_PORT_t _mosiPinMask; ///< Bitmask for MOSI
  ARDUINOGFX_PORT_t _sckPinMask;  ///< Bitmask for SCK
#endif                            // !KINETISK
#else                             // !HAS_PORT_SET_CLR
  PORTreg_t _mosiPort;               ///< PORT register for MOSI
  PORTreg_t _sckPort;                ///< PORT register for SCK
  PORTreg_t _csPort;                 ///< PORT register for chip select
  PORTreg_t _dcPort;                 ///< PORT register for data/command
  ARDUINOGFX_PORT_t _csPinMaskSet;   ///< Bitmask for chip select SET (OR)
  ARDUINOGFX_PORT_t _csPinMaskClr;   ///< Bitmask for chip select CLEAR (AND)
  ARDUINOGFX_PORT_t _dcPinMaskSet;   ///< Bitmask for data/command SET (OR)
  ARDUINOGFX_PORT_t _dcPinMaskClr;   ///< Bitmask for data/command CLEAR (AND)
  ARDUINOGFX_PORT_t _mosiPinMaskSet; ///< Bitmask for MOSI SET (OR)
  ARDUINOGFX_PORT_t _mosiPinMaskClr; ///< Bitmask for MOSI CLEAR (AND)
  ARDUINOGFX_PORT_t _sckPinMaskSet;  ///< Bitmask for SCK SET (OR bitmask)
  ARDUINOGFX_PORT_t _sckPinMaskClr;  ///< Bitmask for SCK CLEAR (AND)
#endif                            // HAS_PORT_SET_CLR
  PORTreg_t _misoPort;            ///< PORT (PIN) register for MISO
#if !defined(KINETISK)
  ARDUINOGFX_PORT_t _misoPinMask; ///< Bitmask for MISO
#endif                            // !KINETISK
#endif                            // defined(USE_FAST_PINIO)
};

#endif // _Indicator_SWSPI_H_
