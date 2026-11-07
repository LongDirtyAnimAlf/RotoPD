unit usbboard;

interface

{$ifdef FPC}
{$mode Delphi}
{$endif}

{.$define USEREGISTRY}

uses
  SysUtils, Classes,
  usb;

type
  TMyUSB = class(TUSB)
  private
    FMaxBoards:word;
    FMaxErrors:word;
  public
    constructor Create;
    function CheckVendorProduct(const VID,PID:word):boolean;override;

    function SetBoardSerialRandom(const Ctrl: TUSBController; out SN:ansistring):boolean;
    function GetBoardSerial(const Ctrl: TUSBController; out SN:ansistring):boolean;
    function GetBoardFirmware(const Ctrl: TUSBController; out FW:word):boolean;
    function GetBoardNumber(const Ctrl: TUSBController; out BN:word):boolean;
    function SetBoardNumber(const Ctrl: TUSBController; const BN:word):boolean;

    property MaxBoards:word read FMaxBoards write FMaxBoards;
    property MaxErrors:word write FMaxErrors;
  end;

implementation

uses
  {$ifdef USEREGISTRY}
  {$ifndef FPC}
  System.StrUtils,
  System.Win.Registry,
  {$else}
  Registry,
  {$endif}
  {$else}
  IniFiles,
  {$endif}
  DateUtils;

type
  TBaseCommands = (CMD_get_serial=$64,CMD_set_serial=$65,CMD_get_board=$CA,CMD_set_board=$CB,CMD_get_firmware=$C8,CMD_error=$FF); // Battery databoards
  //TBaseCommands = (CMD_get_serial=$43,CMD_set_serial=$44,CMD_get_board=$41,CMD_set_board=$42,CMD_get_firmware=$C8,CMD_error=$FF); // SOHIT databoards

constructor TMyUSB.Create;
begin
  inherited;

  USBMasterController.DevThreadSleepTime:=150;

  //{$ifdef LINUX}
  WaitEx:=True;
  //{$endif}
end;

function TMyUSB.CheckVendorProduct(const VID,PID:word):boolean;
const
  VENDORID_BASE                 = $04D8;
  PRODUCTID_BASE                = $003E;
  VENDORID_ALT                  = $ABCD;
  PRODUCTID_ALT                 = $1234;
begin
  result:=
  (
  ( (VENDORID_BASE=VID) AND (PRODUCTID_BASE=PID) )
  OR
  ( (VENDORID_ALT=VID) AND (PRODUCTID_ALT=PID) )
  );
end;

function TMyUSB.SetBoardSerialRandom(const Ctrl: TUSBController; out SN:ansistring):boolean;
var
  error:boolean;
  PLocalData:^TReport;
  localserial:ansistring;
  i:integer;
  cmd:byte;
begin
  result:=false;
  Ctrl.FaultCounter:=1;
  cmd:=byte(TBaseCommands.CMD_set_serial);

  repeat
    PLocalData:=@Ctrl.LocalData;
    PLocalData^:=Default(TReport);
    PLocalData^.Data[0] := cmd;
    for i:=1 to 12 do PLocalData^.Data[i] := Random($FF);

    error:=HidReadWrite(Ctrl);

    if (NOT error) then with PLocalData^ do
    begin
      error:=(data[0]<>cmd);
      if (NOT error) then
      begin
        localserial:='';
        for i:=0 to 5 do
        begin
          localserial:=localserial+'-'+InttoHex(WORD(data[1+(i*2)]+data[2+(i*2)]*256),4);
        end;
        Delete(localserial,1,1);
        Info:='New USB serial succes. New serial: '+localserial;
      end;
    end;

    if error then Inc(Ctrl.FaultCounter);
  until ((NOT error) OR (Ctrl.FaultCounter>FMaxErrors) );

  if error then
  begin
    localserial:='error';
    Errors:='Something went wrong while creating USB serial !!';
  end;

  SN:=localserial;
  result:=error;
end;

function TMyUSB.GetBoardSerial(const Ctrl: TUSBController; out SN:ansistring):boolean;
var
  error:boolean;
  PLocalData:^TReport;
  localserial:ansistring;
  i:integer;
  cmd:byte;
begin
  error:=false;
  Ctrl.FaultCounter:=1;
  cmd:=byte(TBaseCommands.CMD_get_serial);

  localserial:='unknown';

  repeat
    PLocalData:=@Ctrl.LocalData;
    PLocalData^:=Default(TReport);
    PLocalData^.Data[0]:=cmd;

    error:=HidReadWrite(Ctrl);

    if (NOT error) then with PLocalData^ do
    begin
      error:=(data[0]<>cmd);
      if (NOT error) then
      begin
        localserial:='';
        for i:=0 to 5 do localserial:=localserial+'-'+InttoHex(WORD(data[1+(i*2)]+data[2+(i*2)]*256),4);
        Delete(localserial,1,1);
      end;
    end;
    if error then Inc(Ctrl.FaultCounter);
  until ((NOT error) OR (Ctrl.FaultCounter>FMaxErrors) );

  if error then
  begin
    localserial:='error';
    Errors:='Something went wrong while reading serial !!';
  end;

  SN:=localserial;
  result:=error;
end;

function TMyUSB.GetBoardFirmware(const Ctrl: TUSBController; out FW:word):boolean;
var
  error:boolean;
  PLocalData:^TReport;
  cmd:byte;
begin
  result:=false;
  Ctrl.FaultCounter:=1;
  cmd:=byte(TBaseCommands.CMD_get_firmware);

  repeat
    PLocalData:=@Ctrl.LocalData;
    PLocalData^:=Default(TReport);
    PLocalData^.Data[0] := cmd;

    error:=HidReadWrite(Ctrl);

    if (NOT error) then with PLocalData^ do
    begin
      error:=(data[0]<>cmd);
      if (NOT error) then FW:=(data[1]*256+data[2]);
    end;

    if error then Inc(Ctrl.FaultCounter);
  until ((NOT error) OR (Ctrl.FaultCounter>FMaxErrors) );

  if error then Errors:='Something went wrong while reading firmware version !!';

  result:=error;
end;

function TMyUSB.GetBoardNumber(const Ctrl: TUSBController; out BN:word):boolean;
var
  error:boolean;
  PLocalData:^TReport;
  cmd:byte;
begin
  result:=false;
  Ctrl.FaultCounter:=1;
  cmd:=byte(TBaseCommands.CMD_get_board);

  repeat
    PLocalData:=@Ctrl.LocalData;
    PLocalData^:=Default(TReport);
    PLocalData^.Data[0] := cmd;

    error:=HidReadWrite(Ctrl);

    if (NOT error) then with PLocalData^ do
    begin
      error:=(data[0]<>cmd);
      if (NOT error) then BN:=data[1];
    end;
    if error then Inc(Ctrl.FaultCounter);
  until ((NOT error) OR (Ctrl.FaultCounter>FMaxErrors) );

  if error then Errors:='Something went wrong while reading board number !!';

  result:=error;
end;

function TMyUSB.SetBoardNumber(const Ctrl: TUSBController; const BN:word):boolean;
var
  error:boolean;
  PLocalData:^TReport;
  cmd:byte;
begin
  result:=false;
  Ctrl.FaultCounter:=1;
  cmd:=byte(TBaseCommands.CMD_set_board);

  repeat
    PLocalData:=@Ctrl.LocalData;
    PLocalData^:=Default(TReport);
    PLocalData^.Data[0] := cmd;
    PLocalData^.Data[1] := BN;

    error:=HidReadWrite(Ctrl);

    if (NOT error) then with PLocalData^ do
    begin
      error:=(data[0]<>cmd);
      if (NOT error) then Info:='New USB boardnumber succes. New boardnumber: '+InttoStr(BN)
    end;

    if error then Inc(Ctrl.FaultCounter);
  until ((NOT error) OR (Ctrl.FaultCounter>FMaxErrors) );

  if error then Errors:='Something went wrong while setting board number !!';

  result:=error;
end;

end.
