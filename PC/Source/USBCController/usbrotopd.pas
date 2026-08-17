unit usbrotopd;

interface

{$ifdef FPC}
{$mode Delphi}
{$endif}

{$scopedEnums on}

uses
  SysUtils, Classes,
  contnrs,
  usb,
  usbboard;

const
  COMMANDPOSITION  = 0;
  INDEXPOSITION    = 1;
  LENGTHPOSITION   = 2;
  DATASTART        = 3;

  COMMAND_SIZE     = 64;

type

  TCommands = (
    CMD_unknown          = $00,
    CMD_error            = $40,
    CMD_get_data         = $50,
    CMD_get_status,
    CMD_get_hardware,
    CMD_get_firmware,
    CMD_set_value,
    CMD_set_energy,
    CMD_get_PDOList,
    CMD_read_PDOList,
    CMD_set_FIXEDPDO,
    CMD_set_PPSPDO,
    CMD_set_AVSPDO,
    CMD_set_MAXPDO,
    CMD_controller_reset = $B0
  );

type
  TDeviceEvent    = procedure(Sender: TObject;datacarrier:integer) of object;

  TControllerData = class
  protected
    DebugInfo     : string;
    Vref          : word;
    Vmax          : word;
    Imax          : word;
    Calibration   : ansistring;
  public
    constructor Create;
    destructor Destroy;override;
  end;

  TDataDevice=class
  strict private
    //function  ReadSerial(board:word):string;
    //function  ReadFirmware(board:word):word;
    ErrorCounter       : word;
    //FUSBBoards         : TList;
    FUSBBoards         : TObjectList;
    FDataSource        : TMyUSB;
    FOnDeviceChange    : TDeviceEvent;
    FOnData            : TDataEvent;
    FEmulation         : boolean;
    FEnabled           : boolean;
    FAddressReset      : boolean;

    function CheckCRCError(aReport:TReport):boolean;
    function HandleDataRequest(Ctrl: TUSBController; Final:boolean=false):boolean;
  type
    BatteryStatus    = (BSOff=0,BSCurrent,BSPower,BSResistor,BSReserve,BSCharge,BSVoltage,BSPulse);
  private
    NumTypes         : word;

    function  GetErrors:String;
    procedure AddErrors(data:string);

    function  GetInfo:String;
    procedure AddInfo(data:string);

    procedure UpdateUSBDevice(Sender: TObject;Board:TUSBController);

    procedure OnDeviceData(Sender: TObject; ReportID: Byte; const Data: Pointer; {%H-}Size: Word);

    procedure SetEnabled(Value: Boolean);

    function  FGetImax(board,position:word):word;
    function  FGetCaldate(board,position:word):ansistring;

    function  SetValueFine(board,position:word;whattoset:BatteryStatus;value:dword;fine:word; out realvalue:UInt64):boolean;overload;
  public
    MaxErrors:word;


    constructor Create(AppPath:string);
    destructor Destroy;override;

    function  CheckParameters(board:word):boolean;overload;

    function  ToTotal(board:word; out total:word):boolean;
    function  FromTotal(total:word; out board:word):boolean;

    property  DataSource:TMyUSB read FDataSource;

    property  Emulation:boolean read FEmulation;
    property  AddressReset:boolean read FAddressReset write FAddressReset;

    property  Errors:String read GetErrors;
    property  Info:String read GetInfo;

    property  Enabled: Boolean read FEnabled write SetEnabled;

    property  OnDeviceChange: TDeviceEvent read FOnDeviceChange write FOnDeviceChange;
    property  OnDataReceived : TDataEvent read FOnData write FOnData;

    function  GetPDOList(board:word):boolean;

    function  SetPDO(board,PDOIndex,PDOType:byte;MaxCurrent_mA:dword=0;Voltage_mV:dword=0):boolean;


    function  SetMaxPDO(board,PDO:word):boolean;
    function  SetFixedPDO(board,PDO:word;MaxCurrent_mA:dword):boolean;

    function  ReadBatteryData(board,position:word;out voltage,current,energy,temperature:double):boolean;overload;
    function  ReadBatteryData(board,position:word;out voltage,current:double):boolean;overload;

    function  SetEnergy(board,position:word;const value:double=0):boolean;
    function  SetOff(board,position:word):boolean;
    function  SetCurrent(board,position:word;value:dword;fine:word; out realvalue:UInt64):boolean;overload;
    function  SetCurrent(board,position:word;value:dword;fine:word):boolean;overload;
    function  SetCurrent(board,position:word;value:dword):boolean;overload;
    function  SetPower(board,position:word;value:dword; out realvalue:UInt64):boolean;
    function  SetResistor(board,position:word;value:dword; out realvalue:UInt64):boolean;
    function  SetChargeCurrent(board,position:word;value:dword;fine:word; out realvalue:UInt64):boolean;
  end;

implementation

uses
  DateUtils,
  IniFiles,
  bits,
  ap33772s;

const
  USBErrorTimeout               = 50;
  Bitssss                          = 16;

function CRC16(Buffer: String): word;  // CRC16 XMODEM
const
  Initial = $FFFF;
  Polynom = $1021;   // 0001 0000 0010 0001  (0, 5, 12)
var
  i,j: Integer;
  crc: Cardinal;
begin
  crc:=Initial;
  for i:=1 to Length(Buffer) do
  begin
    crc:=crc xor (ord(buffer[i]) shl 8);
    for j:=0 to 7 do
    begin
      if (crc and $8000)<>0 then
        crc:=(crc shl 1) xor Polynom
      else
        crc:=crc shl 1;
    end;
  end;
  crc :=crc and $ffff;
  CRC16 := crc;
end;

function CRC16Microchip(p:pbyte;len:integer;initial:word=0): Word;
const
  polynomial = $1021;   // 0001 0000 0010 0001  (0, 5, 12)
var
  crc: Word;
  I, J: Integer;
  b: Byte;
  bit:word;
  c15: Boolean;
begin
  crc := initial; // initial value
  for I := 0 to len-1 do
  begin
    b := p[I];
    for J := 0 to 7 do
    begin
      c15 := (((crc shr 15) and 1) = 1);  // test hoogste bit accu
      crc := crc shl 1;               // shift accu
      bit := (b shr (7-J)) and 1;     // or met hoogste bit data
      crc:=crc or bit;
      if c15 then                     // indien test dan polyxor.
        begin
          crc := crc xor polynomial;
        end;
    end;
  end;
  Result := crc and $ffff;
end;

function crc_sick( const input_str:PByte; num_bytes:integer ):word;
const
  CRC_POLY_SICK = $8005;
  CRC_START_SICK = $FFFF;
var
  low_byte,high_byte,short_c,short_p:word;
  ptr:PByte;
  a:integer;
begin
  Result := CRC_START_SICK;
  ptr:= input_str;
  short_p := 0;
  if ptr <> nil then for a := 0 to pred(num_bytes) do
  begin
    short_c := $00FF and word(ptr^);
    if ( Result and $8000 ) <> 0 then
      Result := ( Result << 1 ) xor CRC_POLY_SICK
	else
	  Result := Result << 1;
	Result := Result xor ( short_c or short_p );
	short_p := short_c << 8;
    inc(ptr);
  end;

  low_byte  := (Result and $FF00) >> 8;
  high_byte := (Result and $00FF) << 8;
  Result := low_byte or high_byte;
end;

constructor TControllerData.Create;
begin
  //inherited Create;
end;

destructor TControllerData.Destroy;
begin
  inherited Destroy;
end;

constructor TDataDevice.Create(AppPath:string);
var
  Ini:TIniFile;
  BoardCount:word;
begin
  MaxErrors     := 2;

  FAddressReset := true;

  Ini := TIniFile.Create(AppPath+'PBSettings.ini');
  try
    MaxErrors     := Ini.ReadInteger( 'General', 'NumError', MaxErrors );
    if  (NOT Ini.ValueExists('General', 'NumError')) then Ini.WriteInteger( 'General', 'NumError', MaxErrors );
    NumTypes      := Ini.ReadInteger( 'General', 'NumTypes', 0 );
  finally
    Ini.Free;
  end;
  if NumTypes<1 then NumTypes  := 1;

  FUSBBoards:=TObjectList.Create;
  FUSBBoards.OwnsObjects:=False;
  //FUSBBoards:=TList.Create;
  BoardCount:=NumTypes;
  Inc(BoardCount); // we do not use board zero.

  //FUSBBoards.Count:=BoardCount;

  FDataSource:=TMyUSB.Create;
  DataSource.MaxBoards:=BoardCount;
  DataSource.MaxErrors:=MaxErrors;
end;

destructor TDataDevice.Destroy;
var
  Ctrl:TUSBController;
  I: integer;
begin
  for I := 1 to FUSBBoards.Count - 1 do
  begin
    if Assigned(FUSBBoards.Items[I]) then
    begin
      Ctrl := (FUSBBoards.Items[I] AS TUSBController);
      Ctrl.Destroy;
    end;
    FUSBBoards.Items[I] := nil;
  end;

  FDataSource.Destroy;

  FUSBBoards.Destroy;
end;


function TDataDevice.CheckCRCError(aReport:TReport):boolean;
var
  CRCString:shortstring;
  i,CRC:word;
  cmd,length,slave_address,CRCHigh,CRCLow:byte;
begin
  // Default : error !!
  result:=true;

  with aReport do
  begin
    cmd:=data[0];
    slave_address:=(data[1]*2)+32;
    length:=data[2];

    // Old firmware does not send the CRC back, so ignore and return without error
    if ((data[length+3]=0) AND (data[length+4]=0)) then result:=false;

    if (result) then
    begin
      SetLength({%H-}CRCString,length+3);
      CRCString[1]:=Chr(slave_address);
      CRCString[2]:=Chr(cmd);
      CRCString[3]:=Chr(length);

      if (length>0) then
      begin
        for i:=1 to length do
        begin
          CRCString[3+i]:=Chr(data[2+i]);
        end;
      end;
      CRC := CRC16(CRCString);

      CRCHigh:=((CRC AND $FF00) SHR 8);
      CRCLow:=(CRC AND $00FF);

      if (data[length+3]=CRCHigh) AND (data[length+4]=CRCLow) then
      begin
        result:=false;
      end
      else
      begin
        AddErrors('CRC error check. Wrong CRC.');
      end;
    end;

  end;
end;

function TDataDevice.HandleDataRequest(Ctrl: TUSBController; Final:boolean):boolean;
var
  error:boolean;
  aCmd,aPos,aLength:byte;
  rCmd,rPos,rLength:byte;
begin
  result:=false;

  // Store some parameters of the data request
  aCmd:=Ctrl.LocalData.Data[0]; // command always at element 0
  aPos:=Ctrl.LocalData.Data[1]; // position always at element 1
  aLength:=Ctrl.LocalData.Data[2]; // length always at element 2

  // Execute command request
  error:=DataSource.HidReadWrite(Ctrl,Final);

  if (NOT Final) then
  begin
    rCmd:=Ctrl.LocalData.Data[0]; // command always at element 0
    rPos:=Ctrl.LocalData.Data[1]; // position always at element 1
    rLength:=Ctrl.LocalData.Data[2]; // length always at element 2

    // Check valid data
    if (NOT error) then
    begin
      if (rCmd=0) then
      begin
        AddErrors('Data error check. Missing command.');
        error:=true;
      end;
      // Length can never ever be > 16
      if (rLength>16) then
      begin
        AddErrors('Data error check. Wrong length.');
        error:=true;
      end;
    end;

    // Handle CRC errors
    if (NOT error) then error:=CheckCRCError(Ctrl.LocalData);

    // Handle command error
    if (NOT error) then
    begin
      if rCmd = byte(TCommands.CMD_error) then
      begin
        AddErrors('Data error check. Command error returned.');
        error := true;
      end;
    end;

    // Handle data error
    if (NOT error) then
    begin
      if ((rCmd <> aCmd) OR (rPos <> aPos)) then
      begin
        AddErrors('Data error check. Wrong data returned.');
        error := true;
      end;
    end;

    (*
    if (error AND (NOT Final)) then
    begin
      Ctrl.LocalData.Data[0] := Byte(TCommands.CMD_resend_data);
      Ctrl.LocalData.Data[1] := aPos;
      result:=HandleDataRequest(Ctrl,True);
    end;
    *)

  end;

  if (error) then
  begin
    SysUtils.Sleep(USBErrorTimeout-1);
    if (MainThreadID=GetCurrentThreadID) then CheckSynchronize(1);
  end;

  if error then Inc(ErrorCounter);

  result:=error;
end;

procedure TDataDevice.SetEnabled(Value: Boolean);
begin
  if (Value <> FEnabled) then
  begin
    FEnabled := Value;
    if FEnabled then
    begin
      DataSource.OnUSBDeviceChange:=UpdateUSBDevice;
    end
    else
    begin
      DataSource.OnUSBDeviceChange:=nil;
    end;
    DataSource.Enabled:=FEnabled;
  end;
end;

procedure TDataDevice.OnDeviceData(Sender: TObject; ReportID: Byte; const Data: Pointer; {%H-}Size: Word);
begin
  if Assigned(FOnData) then FOnData(Self,ReportID,Data,Size);
end;

procedure TDataDevice.UpdateUSBDevice(Sender: TObject;Board:TUSBController);
const
  DEFAULTSERIALS : array[0..4] of ansistring =
  (
    '1FFF-2FFF-3FFF-4FFF-5FFF-6FFF',
    '0-0-0-0-0-0',
    '0000-0000-0000-0000-0000-0000',
    '65535-65535-65535-65535-65535-65535',
    'FFFF-FFFF-FFFF-FFFF-FFFF-FFFF'
  );
var
  error:boolean;
  localboard:integer;
  storedboard:word;
  localboardserial:ansistring;
  localfirmware:word;
  Ctrl:TUSBController;
function SerialDefault(s:string):boolean;
var
  ds:string;
  i:integer;
begin
  result:=false;
  i:=Length(s);
  if NOT result then result:=(i<>29);
  // We might also count dashed
  if NOT result then
  begin
    while (i>0) do
    begin
      result:=(NOT (s[i] in ['0'..'9','A'..'F','-']));
      if result then break;
      Dec(i);
    end;
  end;
  if NOT result then result:=((RightStr(s,4)='FFFF') AND (LeftStr(s,4)<>'FFFF'));
  if NOT result then
  begin
    for ds in DEFAULTSERIALS do
    begin
      result:=(s=ds);
      if result then break;
    end;
  end;
end;

begin
  localboard:=0;
  storedboard:=0;
  localfirmware:=0;
  localboardserial:='';
  error:=false;

  //if (Assigned(Board) AND Assigned(Board.HidCtrl)) then
  if Assigned(Board) then
  begin
    //Board.DisableReadThreading;
    // We might enable threaded reception of data
    //Board.EnableReadThreading;
    //Board.HidCtrl.ThreadSleepTime:=150; // 500 for Win64

    if Board.HidCtrl.IsPluggedIn then
    begin
      // Arrival
      // First, get the real USB (product-)serial from the HID-device itself
      localboardserial:=Board.ProductSerial;

      // Skip some special devices, if any
      error:=((LeftStr(localboardserial,4)='ENV-') OR (LeftStr(localboardserial,3)='BUR'));

      if (NOT error) then
      begin
        // Do we have a serial that is not unique or correct ?
        // Get the serial from eeprom !
        if SerialDefault(localboardserial) then
        begin
          error:=DataSource.GetBoardSerial(Board,localboardserial);
          if (error) then AddInfo('Board serial reception error');
        end;
      end;

      if (NOT error) then
      begin
        // We do NOT use boardnumbers for batteryboards
        //error:=DataSource.GetBoardNumber(Board,storedboard);
        //if (error) then AddInfo('Board number reception error');
      end;

      if (NOT error) then
      begin
        // Do we have a serial that is not unique or correct ?
        // Generate and store a new serial !
        if SerialDefault(localboardserial) then
        begin

          if AddressReset then
          begin
            // We will generate a new random address to be used
            // But first see if the old address is already assigned
            // Do we have a board fitting with the default address ?
            // To be done and implemented
            localboard:=DataSource.GetBoardNumberFromSerial(localboardserial);
            error:=DataSource.SetBoardSerialRandom(Board,localboardserial);
          end;

        end;
      end;

      if (NOT error) then
      begin
        localboard:=DataSource.CheckAddressNewer(localboardserial,storedboard);
        error:=(NOT (localboard>0));
        if (error) then AddInfo('Check address error');
      end;

      if (NOT error) then
      begin
        // Make room for the databoard into the list of FUSBBoards
        while FUSBBoards.Count<=localboard do FUSBBoards.Add(nil);
      end;

      if (NOT error) then
      begin
        if (NOT Assigned(FUSBBoards.Items[localboard])) then
        begin
          // Get firmware
          DataSource.GetBoardFirmware(Board,localfirmware);

          // Set the correct boardnumber on the board itself, if needed
          if (storedboard<>localboard) then
          begin
            // We do NOT yet use boardnumbers for RotoPD boards
            //AddInfo('Changing boardnumber from '+InttoStr(storedboard)+' to '+InttoStr(localboard));
            //DataSource.SetBoardNumber(Board,localboard);
          end;

          AddInfo('Board accepted. S/N of USB controller #'+InttoStr(localboard)+': '+localboardserial+'. FW: '+InttoStr((localfirmware SHR 8) AND $FF)+'-'+InttoStr(localfirmware AND $FF));
          Board.Accepted:=True;

          // As the firmware sends data by itself, allow auto-magic reception of this data
          Board.OnDataReceived:=OnDeviceData;

          Board.ControllerData:=TControllerData.Create;

          // Add accepted databoard to the list of FUSBBoards
          FUSBBoards.Items[localboard]:=Board;
        end
        else
        begin
          // In theory, we should never get her, but anyhow.
          raise EUSBException.Create('Databoard already assigned. Should never happen. Please check code !');
        end;

        with TControllerData(Board.ControllerData) do
        begin
          Vref:=0;
          Vmax:=0;
          Imax:=0;
          Calibration:='N/A';
        end;

        // We might enable threaded reception of data
        //Board.EnableReadThreading;
        //Board.HidCtrl.ThreadSleepTime:=150; // 500 for Win64
      end
      else
      begin
        AddInfo('Some unknown USB controllerboard error occured !!');
      end;

      if (Board.Accepted AND Assigned(FOnDeviceChange)) then FOnDeviceChange(Self,localboard);
    end
    else
    begin
      // Removal
      localboard:=FUSBBoards.Count;
      // Find the board with the right HID device
      while localboard>0 do
      begin
        Dec(localboard);
        Ctrl:=(FUSBBoards.Items[localboard] AS TUSBController);
        if NOT Assigned(Ctrl) then continue;
        if NOT Assigned(Ctrl.HidCtrl) then continue;
        if (Ctrl.HidCtrl=Board.HidCtrl) then
        begin
          // Got you !!
          Ctrl.Destroy;
          Ctrl:=nil;

          // Delete controller from list by setting nil
          FUSBBoards.Items[localboard]:=nil;

          AddInfo('Board [#'+InttoStr(localboard)+'] removed.');

          if Assigned(FOnDeviceChange) then FOnDeviceChange(Self,-1*localboard);

          break;
        end;
      end;

      if (localboard=0) then
      begin
        // In theory, we should never get here, but anyhow.
        raise EUSBException.Create('Databoard to be removed does not exist. Should never happen. Please check code !');
      end;
    end;

    if Assigned(Board) then
    begin
      if (Board.Accepted) then
        AddInfo('Correct device accepted. VID: '+InttoStr(Board.HidCtrl.Attributes.VendorID)+'. PID: '+InttoStr(Board.HidCtrl.Attributes.ProductID)+'.')
      else
        AddInfo('Correct device NOT accepted. VID: '+InttoStr(Board.HidCtrl.Attributes.VendorID)+'. PID: '+InttoStr(Board.HidCtrl.Attributes.ProductID)+'.');
    end;

    AddInfo('Done.');
  end;
end;

function TDataDevice.ToTotal(board:word; out total:word):boolean;
begin
  result:=false;
  total:=(board-1);
end;

function TDataDevice.FromTotal(total:word; out board:word):boolean;
begin
  result:=false;
  board:=total+1;
end;

function TDataDevice.CheckParameters(board:word):boolean;
var
  Ctrl:TUSBController;
begin
  result:=true;
  if FEmulation then exit;
  if (FUSBBoards.Count=0) then exit;
  if (board>=FUSBBoards.Count) then exit;
  Ctrl:=TUSBController(FUSBBoards.Items[board]);
  if (NOT Assigned(Ctrl)) then exit;
  result:=(NOT Assigned(Ctrl.HidCtrl));
end;

function TDataDevice.GetErrors:String;
begin
  result:=DataSource.Errors;
  DataSource.Errors:='';
end;

function TDataDevice.GetInfo:String;
begin
  result:=DataSource.Info;
  DataSource.Info:='';
end;

procedure TDataDevice.AddInfo(data:string);
begin
  if Length(data)>0 then
  begin
    DataSource.Info:=data;
  end;
end;

procedure TDataDevice.AddErrors(data:string);
begin
  if Length(data)>0 then
  begin
    DataSource.Errors:=data;
  end;
end;

function TDataDevice.FGetImax(board,position:word):word;
begin
  result:=TControllerData(TUSBController(FUSBBoards.Items[board]).ControllerData).Imax;
end;

function TDataDevice.FGetCaldate(board,position:word):ansistring;
begin
  result:=TControllerData(TUSBController(FUSBBoards.Items[board]).ControllerData).Calibration;
end;


function  TDataDevice.GetPDOList(board:word):boolean;
var
  error:boolean;
  cmd: TCommands;
  PLocalData:PReport;
  Ctrl:TUSBController;
begin
  result:=false;

  if CheckParameters(board) then
  begin
    exit;
  end;

  ErrorCounter:=1;
  cmd := TCommands.CMD_get_PDOList;

  Ctrl:=TUSBController(FUSBBoards.Items[board]);
  with TControllerData(Ctrl.ControllerData) do
  begin
    PLocalData:=@Ctrl.LocalData;

    repeat
      PLocalData^:=Default(TReport);
      PLocalData^.Data[COMMANDPOSITION] := byte(cmd);

      error := HandleDataRequest(Ctrl,True);

      if (NOT error) then with PLocalData^ do
      begin
      end;

    until ((NOT error) OR (ErrorCounter>MaxErrors));
  end;

  result:=error;
end;

function  TDataDevice.SetPDO(board,PDOIndex,PDOType:byte;MaxCurrent_mA:dword;Voltage_mV:dword):boolean;
var
  error:boolean;
  cmd: TCommands;
  temp:TDWordData;
  PLocalData:PReport;
  Ctrl:TUSBController;
begin
  result:=false;

  if CheckParameters(board) then exit;

  ErrorCounter:=1;

  cmd := TCommands.CMD_unknown;

  case PDOType of
    PDO_TYPE_FIXED : cmd := TCommands.CMD_set_FIXEDPDO;
    PDO_TYPE_PPS   : cmd := TCommands.CMD_set_PPSPDO;
    PDO_TYPE_AVS   : cmd := TCommands.CMD_set_AVSPDO;
    PDO_TYPE_MAX   : cmd := TCommands.CMD_set_MAXPDO;
  end;

  if (cmd=TCommands.CMD_unknown) then exit;

  Ctrl:=TUSBController(FUSBBoards.Items[board]);
  with TControllerData(Ctrl.ControllerData) do
  begin
    PLocalData:=@Ctrl.LocalData;

    repeat
      PLocalData^:=Default(TReport);

      PLocalData^.Data[COMMANDPOSITION] := byte(cmd);
      PLocalData^.Data[INDEXPOSITION] := 0;           // position ... not needed

      if (cmd=TCommands.CMD_set_MAXPDO) then
      begin
        PLocalData^.Data[LENGTHPOSITION] := 1;        // data length
        PLocalData^.Data[DATASTART] := PDOIndex;      // data itself: PDO wanted
      end;

      if (cmd=TCommands.CMD_set_FIXEDPDO) then
      begin
        PLocalData^.Data[LENGTHPOSITION] := 1+4;        // data length
        PLocalData^.Data[DATASTART] := PDOIndex;        // data itself: PDO wanted

        temp.Raw:=MaxCurrent_mA;
        PLocalData^.Data[DATASTART]   :=(temp.Bytes[0]);
        PLocalData^.Data[DATASTART+1] :=(temp.Bytes[1]);
        PLocalData^.Data[DATASTART+2] :=(temp.Bytes[2]);
        PLocalData^.Data[DATASTART+3] :=(temp.Bytes[3]);
      end;

      if (cmd in [TCommands.CMD_set_PPSPDO,TCommands.CMD_set_AVSPDO]) then
      begin
        PLocalData^.Data[LENGTHPOSITION] := 1+4+4;        // data length
        PLocalData^.Data[DATASTART] := PDOIndex;          // data itself: PDO wanted

        temp.Raw:=MaxCurrent_mA;
        PLocalData^.Data[DATASTART]   :=(temp.Bytes[0]);
        PLocalData^.Data[DATASTART+1] :=(temp.Bytes[1]);
        PLocalData^.Data[DATASTART+2] :=(temp.Bytes[2]);
        PLocalData^.Data[DATASTART+3] :=(temp.Bytes[3]);

        temp.Raw:=Voltage_mV;
        PLocalData^.Data[DATASTART+4] :=(temp.Bytes[0]);
        PLocalData^.Data[DATASTART+5] :=(temp.Bytes[1]);
        PLocalData^.Data[DATASTART+6] :=(temp.Bytes[2]);
        PLocalData^.Data[DATASTART+7] :=(temp.Bytes[3]);
      end;

      error := HandleDataRequest(Ctrl,True);

      if (NOT error) then with PLocalData^ do
      begin
      end;

    until ((NOT error) OR (ErrorCounter>MaxErrors));
  end;

  result:=error;
end;

function TDataDevice.SetMaxPDO(board,PDO:word):boolean;
begin
  result:=SetPDO(board,PDO,PDO_TYPE_MAX);
end;

function TDataDevice.SetFixedPDO(board,PDO:word;MaxCurrent_mA:dword):boolean;
begin
  result:=SetPDO(board,PDO,PDO_TYPE_FIXED,MaxCurrent_mA);
end;

function  TDataDevice.ReadBatteryData(board,position:word;out voltage,current,energy,temperature:double):boolean;
var
  error:boolean;
  cmd: TCommands;
  measuredvalue:double;
  datalength:word;
  PLocalData:PReport;
  Ctrl:TUSBController;
begin
  result:=false;

  if CheckParameters(board) then
  begin
    //if Emulation then
    begin
      voltage:=(random/5)+0.8;
      //current:=(random(1000)+25)/1000;
      //voltage:=1.234;
      current:=0.6789;
      //voltage:=1.5;
      temperature:=random+21;
      SysUtils.Sleep(1); // sleep a bit to simulate a real measurement
    end;
    exit;
  end;

  ErrorCounter:=1;
  cmd := TCommands.CMD_get_data;

  Ctrl:=TUSBController(FUSBBoards.Items[board]);
  with TControllerData(Ctrl.ControllerData) do
  begin
    PLocalData:=@Ctrl.LocalData;

    repeat
      PLocalData^:=Default(TReport);
      PLocalData^.Data[0] := byte(cmd);
      PLocalData^.Data[1] := position-1;
      PLocalData^.Data[2] := 0;

      error := HandleDataRequest(Ctrl);

      if (NOT error) then with PLocalData^ do
      begin
        datalength:=data[2];
        begin
          //voltage
          measuredvalue:=(UInt16(data[3])+UInt16(data[4])*256);
          measuredvalue:=measuredvalue/((1 shl Bitssss)-1);
          measuredvalue:=measuredvalue*Vmax;
          voltage:=measuredvalue/1000;

          //current
          measuredvalue:=(UInt16(data[5])+UInt16(data[6])*256);
          measuredvalue:=measuredvalue/((1 shl Bitssss)-1);
          measuredvalue:=measuredvalue*Imax;
          current:=measuredvalue/1000;
        end;

        // Energy
        measuredvalue:=(UInt32(data[7])+UInt32(data[8])*256+UInt32(data[9])*256*256+UInt32(data[10])*256*256*256);
        // Be carefull : due to differences in firmware, the energy calculations are more or less unpredictable.
        // Due to errors in firmware !!
        measuredvalue:=(measuredvalue*8*2);
        // Convert to mWh
        energy:=measuredvalue/(3600*1000);

        // Temperature
        temperature:=(UInt16(data[11])+UInt16(data[12])*256)/10;

        //if temperature>250 then temperature:=250;
        //if temperature<0 then temperature:=0;
      end;

    until ((NOT error) OR (ErrorCounter>MaxErrors));
  end;

  result:=error;
end;

function  TDataDevice.ReadBatteryData(board,position:word;out voltage,current:double):boolean;
var
  e,t:double;
begin
  result:=ReadBatteryData(board,position,voltage,current,e,t);
end;

function  TDataDevice.SetEnergy(board,position:word;const value:double=0):boolean;
var
  error:boolean;
  cmd: TCommands;
  temp:UInt64;
  PLocalData:PReport;
  Ctrl: TUSBController;
begin
  result:=false;

  if CheckParameters(board) then exit;

  ErrorCounter:=1;
  cmd := TCommands.CMD_set_energy;

  Ctrl:=TUSBController(FUSBBoards.Items[board]);
  begin
    PLocalData:=@Ctrl.LocalData;

    repeat
      PLocalData^:=Default(TReport);
      PLocalData^.Data[0] := Byte(cmd);
      PLocalData^.Data[1] := position-1;
      PLocalData^.Data[2] := 4;
      if value<>0 then
      begin
        temp:=round(value*(8*32*3600*1000));
        temp:=temp DIV 65536;
        PLocalData^.Data[3] :=(temp MOD 256);
        temp:=(temp DIV 256);
        PLocalData^.Data[4] :=(temp MOD 256);
        temp:=(temp DIV 256);
        PLocalData^.Data[5] :=(temp MOD 256);
        temp:=(temp DIV 256);
        PLocalData^.Data[6] :=(temp MOD 256);
      end;

      error := HandleDataRequest(Ctrl);

    until ((NOT error) OR (ErrorCounter>MaxErrors));
  end;

  result:=error;
end;

function TDataDevice.SetValueFine(board,position:word;whattoset:BatteryStatus;value:dword;fine:word; out realvalue:UInt64):boolean;
var
  temp:dword;
  error:boolean;
  cmd:TCommands;
  PLocalData:PReport;
  Ctrl: TUSBController;
  CRCString:shortstring;
  i:integer;
  CRC:word;
begin
  result:=false;

  if CheckParameters(board) then exit;

  ErrorCounter:=1;
  cmd := TCommands.CMD_set_value;

  Ctrl:=TUSBController(FUSBBoards.Items[board]);
  begin
    PLocalData:=@Ctrl.LocalData;

    repeat
      PLocalData^:=Default(TReport);
      PLocalData^.Data[0] := byte(cmd);
      PLocalData^.Data[1] := position-1;

      if (fine=0) then
         PLocalData^.Data[2] := 5
      else
          PLocalData^.Data[2] := 7;

      //if ((value=0) AND (fine=0) AND (whattoset<>BatteryStatus.BSOff)) then
      //begin
        //PLocalData^.Data[3] := Ord(BatteryStatus.BSOff);
        //temp:=0;
      //end
      //else
      begin
        PLocalData^.Data[3] := Ord(whattoset);

        temp:=value;

        PLocalData^.Data[4] :=(temp MOD 256);
        temp:=(temp DIV 256);
        PLocalData^.Data[5] :=(temp MOD 256);
        temp:=(temp DIV 256);
        PLocalData^.Data[6] :=(temp MOD 256);
        temp:=(temp DIV 256);
        PLocalData^.Data[7] :=(temp MOD 256);

        if (fine>0) then
        begin
          temp:=fine;

          PLocalData^.Data[8] :=(temp MOD 256);
          temp:=(temp DIV 256);
          PLocalData^.Data[9] :=(temp MOD 256);
        end;
      end;

      error := HandleDataRequest(Ctrl);

      if (NOT error) then with PLocalData^ do
      begin
        //realvalue:=(data[3]+data[4]*256);
        realvalue:=(UInt64(data[3])+UInt64(data[4])*256+UInt64(data[5])*256*256+UInt64(data[6])*256*256*256);

        SetLength({%H-}CRCString,data[2]+3);
        CRCString[1]:=Chr((data[1]*2)+32);
        CRCString[2]:=Chr(data[0]);
        CRCString[3]:=Chr(data[2]);

        if (data[2]>0) then
        begin
          for i:=1 to data[2] do
          begin
            CRCString[3+i]:=Chr(data[2+i]);
          end;
        end;
        CRC := CRC16(CRCString);

        if (data[data[2]+3]=((CRC AND $FF00) SHR 8)) AND (data[data[2]+4]=(CRC AND $00FF)) then
        begin
          i:=0;
        end
        else
        begin
          i:=1;
        end;

        //realvalue:=data[3];
      end;

    until ((NOT error) OR (ErrorCounter>MaxErrors));
  end;

  result:=error;
end;

function  TDataDevice.SetOff(board,position:word):boolean;
var
  realvalue:UInt64;
begin
  result:=SetValueFine(board,position,BatteryStatus.BSOff,0,0,realvalue);
end;

function  TDataDevice.SetCurrent(board,position:word;value:dword;fine:word; out realvalue:UInt64):boolean;
begin
  result:=SetValueFine(board,position,BatteryStatus.BSCurrent,value,fine,realvalue);
end;
function  TDataDevice.SetCurrent(board,position:word;value:dword;fine:word):boolean;
var
  realvalue:UInt64;
begin
  result:=SetCurrent(board,position,value,fine,realvalue);
end;
function  TDataDevice.SetCurrent(board,position:word;value:dword):boolean;
begin
  result:=SetCurrent(board,position,value,0);
end;

function  TDataDevice.SetPower(board,position:word;value:dword; out realvalue:UInt64):boolean;
begin
  result:=SetValueFine(board,position,BatteryStatus.BSPower,value,0,realvalue);
end;
function  TDataDevice.SetResistor(board,position:word;value:dword; out realvalue:UInt64):boolean;
begin
  result:=SetValueFine(board,position,BatteryStatus.BSResistor,value,0,realvalue);
end;
function  TDataDevice.SetChargeCurrent(board,position:word;value:dword;fine:word; out realvalue:UInt64):boolean;
begin
  result:=SetValueFine(board,position,BatteryStatus.BSCharge,value,fine,realvalue);
end;

end.
