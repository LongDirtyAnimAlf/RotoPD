unit main;

{$mode objfpc}{$H+}

interface

uses
  Classes, SysUtils, Forms, Controls, Graphics, Dialogs, StdCtrls, ExtCtrls,
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
    Memo2: TMemo;
    Timer1: TTimer;
    Timer2: TTimer;
    TimerHeartbeat: TTimer;
    ToggleBox1: TToggleBox;
    procedure btnAskVoltageDataClick(Sender: TObject);
    procedure btnConnectClick({%H-}Sender: TObject);
    procedure Button1Click(Sender: TObject);
    procedure Button2Click(Sender: TObject);
    procedure Button3Click(Sender: TObject);
    procedure Button4Click(Sender: TObject);
    procedure FormCreate({%H-}Sender: TObject);
    procedure FormDestroy(Sender: TObject);
    procedure Timer1Timer(Sender: TObject);
    procedure Timer2Timer(Sender: TObject);
    procedure TimerHeartbeatTimer(Sender: TObject);
    procedure ToggleBox1Change(Sender: TObject);
    procedure ToggleBox1Click(Sender: TObject);
  private
    V:double;
    I:double;
    C:double;
    E:double;
    P:double;
    comm:TLazSerial;
    procedure InitMain({%H-}Data: PtrInt);
    procedure SerialRxData({%H-}Sender: TObject);
    procedure SerialTxData(s:string);
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
  SerialTxData('t'+'000'+'2'+'0205');
  SerialTxData('O');

  // Command to stop the Gravity CAN interface
  SerialTxData('C');
end;

procedure TForm1.Timer1Timer(Sender: TObject);
begin
  // Simulate IT2704 !!
  E:=E+0.01;
  SerialTxData('t'+'181'+'8'+SingleToHex(E)+SingleToHex(E+1));
  SerialTxData('t'+'281'+'8'+SingleToHex(26.4)+SingleToHex(88.2));
  SerialTxData('t'+'381'+'8'+SingleToHex(26.4)+SingleToHex(88.2));
  SerialTxData('t'+'481'+'8'+SingleToHex(26.4)+SingleToHex(88.2));
end;

procedure TForm1.Timer2Timer(Sender: TObject);
begin
  V:=Random(20000)/1000;
  I:=Random(5000)/1000;
  P:=123+Random(5000)/1000;
  SerialTxData('t'+'581'+'8'+'430B3001'+SingleToHex(V));
  SerialTxData('t'+'581'+'8'+'430B3002'+SingleToHex(I));
  SerialTxData('t'+'581'+'8'+'430B3003'+SingleToHex(P));
  SerialTxData('t'+'581'+'8'+'430B300C'+SingleToHex(55.8));
end;

procedure TForm1.TimerHeartbeatTimer(Sender: TObject);
begin
  SerialTxData('t'+'701'+'1'+'00');
end;

procedure TForm1.ToggleBox1Change(Sender: TObject);
begin
  Timer1.Enabled:=TToggleBox(Sender).Checked;
  //Timer2.Enabled:=TToggleBox(Sender).Checked;
  TimerHeartbeat.Enabled:=TToggleBox(Sender).Checked;
end;

procedure TForm1.ToggleBox1Click(Sender: TObject);
begin

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

    comm.FlowControl:=fcHardware;
    comm.Parity:=pNone;
    comm.DataBits:=db8bits;
    comm.StopBits:=sbOne;
    comm.OnRxData:=@SerialRxData;
    comm.Async:=True;

    (*
    comm.BaudRate:=br115200;
    comm.Active:=True;
    SerialTxData('Ub9');
    comm.Active:=False;
    *)

    comm.BaudRate:=br921600;
    comm.Active:=True;

    // Command to get the Gravity CAN interface going
    //SerialTxData('S5');
    SerialTxData('O');
    SerialTxData('h');

    // Command to get the IT2704 going
    // ID   = 00000000
    // DLC  = 2
    // Data = 0101  ; 01 = Start Remote Node. 01 = Device instrument address
    // This starts a regular sending of data from IT2704 to PC over CAN.
    //SerialTxData('t'+'000'+'2'+'0101');
    //SerialTxData('O');


  end;
end;

procedure TForm1.Button1Click(Sender: TObject);
begin
  // Send a Node Guarding Request
  //SerialTxData('t'+'701'+'0'+'00'+'0000'+'00'+'00000000');
  SerialTxData('t'+'701'+'0');
end;

procedure TForm1.Button2Click(Sender: TObject);
begin
  // Set IT2700 CANopen remote control mode on
  SerialTxData('t'+'000'+'2'+'0101');
  // Set Set the transmission type of TPDO1 to 254
  SerialTxData('t'+'601'+'8'+'2F001802FE000000');
  //Modify TPDO1 timer cycle as 0ms
  SerialTxData('t'+'601'+'8'+'2B00180500000000');


  SerialTxData('t'+'601'+'8'+'2F011802FE000000');
  SerialTxData('t'+'601'+'8'+'2B01180500000000');
  SerialTxData('t'+'601'+'8'+'2F021802FE000000');
  SerialTxData('t'+'601'+'8'+'2B02180500000000');
  SerialTxData('t'+'601'+'8'+'2F031802FE000000');
  SerialTxData('t'+'601'+'8'+'2B03180500000000');

end;

procedure TForm1.Button3Click(Sender: TObject);

begin
  Memo1.Lines.Append(SingleToHex(2.5));
  SerialTxData('t'+'601'+'8'+'23023001'+SingleToHex(0.2));
  SerialTxData('t'+'601'+'8'+'2F08300101000000');
end;

procedure TForm1.Button4Click(Sender: TObject);
begin
  SerialTxData('t'+'601'+'8'+'2F08300100000000');
end;

procedure TForm1.btnAskVoltageDataClick(Sender: TObject);
begin
  // This will only work in pre-operational and operational mode !!

  // Force set pre-operational mode !!
  //SerialTxData('t'+'000'+'2'+'8001');

  // Ask for data through correct SDO
  //SerialTxData('t'+'601'+'8'+'40'+'0220'+'01'+'00000000');


  SerialTxData('t'+'601'+'8'+'40'+'0B30'+'01'+'00000000');


end;

procedure TForm1.SerialTxData(s:string);
begin
  comm.WriteString(s+#13);
  Memo2.Lines.Append(s);
end;

procedure TForm1.SerialRxData(Sender: TObject);
var
  s:string;
begin
  s:=Trim(comm.Data);
  if Length(s)>0 then
  begin
    if (s='t6018430B300100000000') then
    begin
      V:=Random(20000)/1000;
      SerialTxData('t'+'581'+'8'+'430B3001'+SingleToHex(V));
    end;
    if (s='t6018430B300200000000') then
    begin
      I:=Random(5000)/1000;
      SerialTxData('t'+'581'+'8'+'430B3002'+SingleToHex(I));
    end;

    if (s='t6018430B300300000000') then
    begin
      P:=123+Random(5000)/1000;
      SerialTxData('t'+'581'+'8'+'430B3003'+SingleToHex(P));
    end;

    if (s='t6018430B300C00000000') then
    begin
      SerialTxData('t'+'581'+'8'+'430B300C'+SingleToHex(55.8));
    end;

    if (s='t60182F08300100000000') then
    begin
      SerialTxData('t'+'581'+'8'+'6008300100000000');
    end;

    if (s='t60182F08300000000000') then
    begin
      SerialTxData('t'+'581'+'8'+'6008300000000000');
    end;

    if (Pos('t6018230230',s)=1) then
    begin
      SerialTxData('t'+'581'+'8'+'6002300000000000');
    end;

    Memo1.Lines.Append(s);
  end;
end;

end.

