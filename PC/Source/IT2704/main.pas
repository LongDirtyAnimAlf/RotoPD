unit main;

{$mode objfpc}{$H+}

interface

uses
  Classes, SysUtils, Forms, Controls, Graphics, Dialogs, StdCtrls,
  lazserial;

type

  { TForm1 }

  TForm1 = class(TForm)
    btnConnect: TButton;
    btnAskVoltageData: TButton;
    Button1: TButton;
    Button2: TButton;
    Button3: TButton;
    Button4: TButton;
    cmboSerialPorts: TComboBox;
    Memo1: TMemo;
    procedure btnAskVoltageDataClick(Sender: TObject);
    procedure btnConnectClick({%H-}Sender: TObject);
    procedure Button1Click(Sender: TObject);
    procedure Button2Click(Sender: TObject);
    procedure Button3Click(Sender: TObject);
    procedure Button4Click(Sender: TObject);
    procedure FormCreate({%H-}Sender: TObject);
    procedure FormDestroy(Sender: TObject);
  private
    comm:TLazSerial;
    procedure InitMain({%H-}Data: PtrInt);
    procedure SerialRxData({%H-}Sender: TObject);
  public

  end;

var
  Form1: TForm1;

implementation

{$R *.lfm}

uses
  Bits,Tools;


type
  //S-0-0011, Class 1 diagnostics
  TOperationStatus = bitpacked record
      case integer of
          1 : (  Data : record
                   ACQ_WTG          : T1BITS; // 0, no meaning, to be defined
                   ARB_WTG          : T1BITS; // ARB waiting trigger
                   DLOG_WTG         : T1BITS; // 0, no meaning, to be defined
                   ACQ_Active       : T1BITS; // 0, no meaning, to be defined
                   ARB_Active       : T1BITS; // ARB has triggered, executing
                   DLOG_Active      : T1BITS; // 0, no meaning, to be defined
                   OUTPUT_ENABLED   : T1BITS; // ON/OFF state, on:1, off:0
                   MODE_CC          : T1BITS; // Current working mode: CC mode, plus current
                   MODE_CV          : T1BITS; // Current working mode: CV mode, plus voltage
                   MODE_CW          : T1BITS; // Current working mode: CW mode, plus power
                   MODE_CR          : T1BITS; // Current working mode: CR mode
                   MODE_CC_NOTUSED  : T1BITS;
                   MODE_CP_NOTUSED  : T1BITS;
                   CAL_STATUS       : T1BITS; // Calibration state, executing: 1, unexecuted: 0
                   Priority         : T1BITS; // Working priority, CV: 0, CC: 1.
                   Reserved         : T1BITS;
                 end
              );
          2 : (
               Bits            : bitpacked array[0..15] of T1BITS;
              );
          3 : (
               Raw             : Word;
              );

  end;

  TQuestionableStatus = bitpacked record
      case integer of
          1 : (  Data : record
                    OV         : T1BITS; //  Overvoltage Protection
                    OCPos      : T1BITS; //  Positive Overcurrent Protection
                    OCNeg      : T1BITS; //  Negative Overcurrent Protection
                    OPPos      : T1BITS; //  Positive Overpower Protection
                    OPNeg      : T1BITS; //  Negative Overpower Protection
                    UV         : T1BITS; //  Undervoltage Protection
                    OT         : T1BITS; //  Over Temperature Protection
                    UC         : T1BITS; //  Undercurrent Protection
                    Errsense   : T1BITS; //  Sense Fault
                    Share      : T1BITS; //  Current sharing fault
                    Rvs        : T1BITS; //  The output is reversed
                    INH        : T1BITS; //  Externally inhibited output
                    PS         : T1BITS; //  Fault protection bit (protect shutdown)
                    OSC        : T1BITS; //  Loop oscillation failure
                    Hardware   : T1BITS; //  Unknown internal fault of the instrument (Hardware)
                    Reserved   : T1BITS;
                 end
               );
           2 : (
                Bits            : bitpacked array[0..15] of T1BITS;
               );
           3 : (
                Raw             : Word;
               );

   end;



const
  IT2704_ADDRESS = 1;

function SwapBytes32(const Value: Cardinal): Cardinal;
begin
  Result := ((Value and $000000FF) shl 24) or
            ((Value and $0000FF00) shl  8) or
            ((Value and $00FF0000) shr  8) or
            ((Value and $FF000000) shr 24);
end;


function HexToSingle(const HexValue: Cardinal): Single;
var
  Converter: record
    case Boolean of
      False: (AsCardinal: Cardinal);
      True:  (AsSingle:   Single);
  end;
begin
  Converter.AsCardinal := HexValue;
  Result := Converter.AsSingle;
end;

function SingleToHex(const Value: Single): string;
var
  Converter: record
    case Boolean of
      False: (AsSingle:   Single);
      True:  (AsCardinal: Cardinal);
  end;
begin
  Converter.AsSingle := Value;
  Result := IntToHex(SwapBytes32(Converter.AsCardinal), 8);  // always 8 hex digits
end;


{ TForm1 }

procedure TForm1.FormCreate(Sender: TObject);
begin
  comm:=TLazSerial.Create(Self);

  // Get comport list after form has been created
  Application.QueueAsyncCall(@InitMain,0);
end;

procedure TForm1.FormDestroy(Sender: TObject);
begin
  // Command to stop the IT2704 going
  // ID   = 00000000
  // DLC  = 2
  // Data = 0201  ; 02 = Stop Remote Node. 01 = Device instrument address
  // This stops a regular sending of data from IT2704 to PC over CAN.
  comm.WriteString('t'+'000'+'2'+'0205'+#13);
  comm.WriteString('O'+#13);

  // Command to stop the Gravity CAN interface
  comm.WriteString('C'+#13);
end;


procedure TForm1.InitMain(Data: PtrInt);
{$ifdef UNIX}
var
  com:string;
  i:integer;
{$endif UNIX}
begin
  EnumerateCOMPorts(cmboSerialPorts.Items);
  if (cmboSerialPorts.Items.Count>0) then cmboSerialPorts.ItemIndex:=0;
  {$ifdef UNIX}
  // Make life easy on RPi: pick first available USB serial port.
  // Not necessary correct, but ok for testing.
  i:=0;
  for com in cmboSerialPorts.Items do
  begin
    if (Pos('ttyUSB',com)>0) then
    begin
      cmboSerialPorts.ItemIndex:=i;
      break;
    end;
    Inc(i);
  end;
  {$endif UNIX}
end;


procedure TForm1.btnConnectClick(Sender: TObject);
var
  s:string;
begin
  if (cmboSerialPorts.ItemIndex<>-1) then
  begin
    comm.Active:=False;

    s:=cmboSerialPorts.Text;
    {$ifdef UNIX}
    comm.Device:='/dev/'+s;
    {$else}
    comm.Device:=s;
    {$endif}

    comm.BaudRate:=br115200;
    comm.FlowControl:=fcNone;
    comm.Parity:=pNone;
    comm.DataBits:=db8bits;
    comm.StopBits:=sbOne;
    comm.OnRxData:=@SerialRxData;
    comm.Async:=True;
    comm.Active:=True;

    // Command to get the Gravity CAN interface going
    //comm.WriteString('S5'+#13);
    //comm.WriteString('O'+#13);

    // Command to get the IT2704 going
    // ID   = 00000000
    // DLC  = 2
    // Data = 0101  ; 01 = Start Remote Node. 01 = Device instrument address
    // This starts a regular sending of data from IT2704 to PC over CAN.
    //comm.WriteString('t'+'000'+'2'+'0101'+#13);
    //comm.WriteString('O'+#13);


  end;
end;

procedure TForm1.Button1Click(Sender: TObject);
begin
  // Send a Node Guarding Request
  //comm.WriteString('t'+'701'+'0'+'00'+'0000'+'00'+'00000000'+#13);
  comm.WriteString('t'+'701'+'0'+#13);
end;

procedure TForm1.Button2Click(Sender: TObject);
begin
  // Set IT2700 CANopen remote control mode on
  comm.WriteString('t'+'000'+'2'+'0101'+#13);
  // Set Set the transmission type of TPDO1 to 254
  comm.WriteString('t'+'601'+'8'+'2F001802FE000000'+#13);
  //Modify TPDO1 timer cycle as 0ms
  comm.WriteString('t'+'601'+'8'+'2B00180500000000'+#13);


  comm.WriteString('t'+'601'+'8'+'2F011802FE000000'+#13);
  comm.WriteString('t'+'601'+'8'+'2B01180500000000'+#13);
  comm.WriteString('t'+'601'+'8'+'2F021802FE000000'+#13);
  comm.WriteString('t'+'601'+'8'+'2B02180500000000'+#13);
  comm.WriteString('t'+'601'+'8'+'2F031802FE000000'+#13);
  comm.WriteString('t'+'601'+'8'+'2B03180500000000'+#13);

end;

procedure TForm1.Button3Click(Sender: TObject);

begin
  Memo1.Lines.Append(SingleToHex(2.5));
  comm.WriteString('t'+'601'+'8'+'23023001'+SingleToHex(0.2)+#13);
  comm.WriteString('t'+'601'+'8'+'2F08300101000000'+#13);
end;

procedure TForm1.Button4Click(Sender: TObject);
begin
  comm.WriteString('t'+'601'+'8'+'2F08300100000000'+#13);
end;

procedure TForm1.btnAskVoltageDataClick(Sender: TObject);
begin
  // This will only work in pre-operational and operational mode !!

  // Force set pre-operational mode !!
  //comm.WriteString('t'+'000'+'2'+'8001'+#13);

  // Ask for data through correct SDO
  //comm.WriteString('t'+'601'+'8'+'40'+'0220'+'01'+'00000000'+#13);


  comm.WriteString('t'+'601'+'8'+'40'+'0B30'+'01'+'00000000'+#13);


end;

procedure TForm1.SerialRxData(Sender: TObject);
begin
  Memo1.Lines.Append(comm.Data);
end;

end.

