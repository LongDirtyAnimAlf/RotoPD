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
  Tools;

{ TForm1 }

procedure TForm1.FormCreate(Sender: TObject);
begin
  comm:=TLazSerial.Create(Self);

  // Get comport list after form has been created
  Application.QueueAsyncCall(@InitMain,0);
end;

procedure TForm1.FormDestroy(Sender: TObject);
begin
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

    comm.WriteString('S5'+#13);
    comm.WriteString('O'+#13);

  end;
end;

procedure TForm1.SerialRxData(Sender: TObject);
begin
  Memo1.Lines.Append(comm.Data);
end;

end.

