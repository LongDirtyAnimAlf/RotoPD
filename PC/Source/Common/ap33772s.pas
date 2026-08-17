unit ap33772s;

{$mode ObjFPC}{$H+}

interface

uses
  Classes, SysUtils;

const
  PDO_TYPE_FIXED = 0;
  PDO_TYPE_PPS   = 1;
  PDO_TYPE_AVS   = 2;
  PDO_TYPE_MAX   = 3;

type
  // ── Decoded, human-readable PDO ──────────────────────────────────────────────
  TAP33772S_PDO = record
      index: word;          // 1-based (1–13)
      valid: boolean;       // detect bit = 1
      isEPR: boolean;       // true for index 8–13
      ptype:byte;           // PDO_TYPE_FIXED / _PPS / _AVS
      minVoltage_mV:word;   // 0 for Fixed; 3300 for PPS; 15000 for AVS
      maxVoltage_mV:word;
      maxCurrent_mA:word;   // Approximate upper bound of current range
      currentCode:byte;     // Raw 4-bit current_max field (for CURRENT_SEL)
      raw:word;             // Raw 16-bit register value
  end;

  // Bit-packed records map bits sequentially in order (LSB to MSB).
  TPDOFixed = bitpacked record
    voltage_max  : 0..255; // 8 bits (bits 7:0)
    peak_current : 0..3;   // 2 bits (bits 9:8)
    current_max  : 0..15;  // 4 bits (bits 13:10)
    pdotype      : 0..1;   // 1 bit  (bit 14) — renamed to avoid reserved word
    detect       : 0..1;   // 1 bit  (bit 15)
  end;

  TPDOPps = bitpacked record
    voltage_max  : 0..255; // 8 bits (bits 7:0)
    voltage_min  : 0..3;   // 2 bits (bits 9:8)
    current_max  : 0..15;  // 4 bits (bits 13:10)
    pdotype      : 0..1;   // 1 bit  (bit 14)
    detect       : 0..1;   // 1 bit  (bit 15)
  end;

  TPDOAvs = bitpacked record
    voltage_max  : 0..255; // 8 bits (bits 7:0)
    voltage_min  : 0..3;   // 2 bits (bits 9:8)
    current_max  : 0..15;  // 4 bits (bits 13:10)
    pdotype      : 0..1;   // 1 bit  (bit 14)
    detect       : 0..1;   // 1 bit  (bit 15)
  end;

  TPDOBytes = record
    byte0: UInt8; // LSB
    byte1: UInt8; // MSB
  end;

  // 16-bit compressed PDO layout matching C union
  PDO_DATA_T = packed record
    case Byte of
      0: (fixed: TPDOFixed);
      1: (pps: TPDOPps);
      2: (avs: TPDOAvs);
      3: (bytes: TPDOBytes);
      4: (raw16: word);
  end;

procedure DecodePDONew(idx: UInt8; const raw: PDO_DATA_T; out dec: TAP33772S_PDO);

implementation

// Stub for current decoding logic referenced in your C++ code
function CurrentDecode(Code: UInt8): UInt16;
begin
  if Code = 0 then
    Exit(1000);  // < 1.25A, show as ~1A

  if Code = 15 then
    Exit(5000);  // ≥ 5A

  Result := UInt16(1250 + (Code - 1) * 250);
end;
// ── DecodePDONew — decode bit fields into human-readable struct ─────────────
procedure DecodePDONew(idx: UInt8; const raw: PDO_DATA_T; out dec: TAP33772S_PDO);
begin
  dec.index := idx + 1;
  dec.raw   := UInt16(raw.bytes.byte0) or (UInt16(raw.bytes.byte1) shl 8);
  dec.valid := (raw.fixed.detect = 1);
  dec.isEPR := (idx >= 7); // 0-based: 7–12 -> PDO8–PDO13

  if not dec.valid then
  begin
    dec.ptype       := 0;
    dec.minVoltage_mV := 0;
    dec.maxVoltage_mV := 0;
    dec.maxCurrent_mA := 0;
    dec.currentCode   := 0;
    Exit;
  end;

  dec.currentCode   := raw.fixed.current_max;
  dec.maxCurrent_mA := CurrentDecode(dec.currentCode);

  if raw.fixed.pdotype = 0 then
  begin
    // ── Fixed PDO ─────────────────────────────────────────────────────────
    dec.ptype           := PDO_TYPE_FIXED;
    if dec.isEPR then
      dec.maxVoltage_mV := UInt16(raw.fixed.voltage_max) * 200  // EPR: 200 mV/unit
    else
      dec.maxVoltage_mV := UInt16(raw.fixed.voltage_max) * 100; // SPR: 100 mV/unit

    dec.minVoltage_mV := 0; // Fixed = single point
  end
  else if not dec.isEPR then
  begin
    // ── PPS (type=1 in SPR slot, index 1–7) ──────────────────────────────
    dec.ptype         := PDO_TYPE_PPS;
    dec.maxVoltage_mV := UInt16(raw.pps.voltage_max) * 100; // 100 mV/unit

    // Decode minimum voltage from 2-bit indicator
    case raw.pps.voltage_min of
      1:       dec.minVoltage_mV := 3300;
      2:       dec.minVoltage_mV := 5000; // 3300mV < min <= 5000mV
      otherwise dec.minVoltage_mV := 3300;
    end;
  end
  else
  begin
    // ── AVS (type=1 in EPR slot, index 8–13) ─────────────────────────────
    dec.ptype         := PDO_TYPE_AVS;
    dec.maxVoltage_mV := UInt16(raw.avs.voltage_max) * 200; // 200 mV/unit

    case raw.avs.voltage_min of
      1:       dec.minVoltage_mV := 15000;
      2:       dec.minVoltage_mV := 20000; // 15000mV < min <= 20000mV
      otherwise dec.minVoltage_mV := 15000;
    end;
  end;
end;

end.

