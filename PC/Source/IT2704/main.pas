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
    cmboSerialPorts: TComboBox;
    Memo1: TMemo;
    procedure btnConnectClick({%H-}Sender: TObject);
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
  TDRIVEPARAMETER_0011 = bitpacked record
      case integer of
          1 : (  Data : record
                   Reserved0                               : T1BITS;
                   AmplifierOvertemperatureShutdown        : T1BITS;
                   MotorOvertemperatureShutdown            : T1BITS;
                   Reserved1                               : T1BITS;
                   ControlVoltageError                     : T1BITS;
                   FeedbackError                           : T1BITS;
                   Reserved2                               : T1BITS;
                   Overcurrent                             : T1BITS;
                   Reserved3                               : T1BITS;
                   UndervoltageError                       : T1BITS;
                   Reserved4                               : T1BITS;
                   ExcessiveDeviation                      : T1BITS;
                   CommunicationError                      : T1BITS;
                   TravelLimitSwitchExceeded               : T1BITS;
                   Reserved5                               : T1BITS;
                   ManufacturerSpecificError               : T1BITS;
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
    comm.WriteString('S5'+#13);
    comm.WriteString('O'+#13);

    // Command to get the IT2704 going
    // ID   = 00000000
    // DLC  = 2
    // Data = 0101  ; 01 = Start Remote Node. 01 = Device instrument address
    // This starts a regular sending of data from IT2704 to PC over CAN.
    comm.WriteString('t'+'000'+'2'+'0105'+#13);
    comm.WriteString('O'+#13);


  end;
end;

procedure TForm1.SerialRxData(Sender: TObject);
begin
  Memo1.Lines.Append(comm.Data);
end;

end.

