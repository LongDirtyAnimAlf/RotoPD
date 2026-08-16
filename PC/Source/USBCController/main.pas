unit main;

{$mode objfpc}{$H+}

interface

uses
  {$ifdef Windows}
  Windows,
  {$endif}
  Classes, SysUtils, Forms, Controls, Graphics, Dialogs, StdCtrls, ExtCtrls,
  Grids, Buttons, ComCtrls, Menus, TAGraph, TASeries, Types,
  SynEdit,
  //SynEditMiscClasses,
  fptimernew,
  dsLeds,
  dlLEDCtrl,
  usbcpd,
  pddevice,
  hp66332,
  usbrotopd,
  lazserial;

type
  TTestType = record
    Name:string;
    Current:word;
    Voltage:word;
  end;

  TUSBPDController = class(TUSBPD)
  end;

  { TPowerbankMainForm }

  TPowerbankMainForm = class(TForm)
    btnConnectKC003C: TButton;
    btnInit: TButton;
    btnTestDischarge: TSpeedButton;
    Chart1: TChart;
    Chart1LineSeries1: TLineSeries;
    Chart1LineSeries2: TLineSeries;
    Chart1LineSeries3: TLineSeries;
    chkCurrentLimit: TCheckBox;
    chkVoltageLimit: TCheckBox;
    cmboSerialPorts: TComboBox;
    cmboSerialPorts1: TComboBox;
    CurrentEdit: TEdit;
    DisplaysPanel: TPanel;
    Edit1: TEdit;
    GroupBox1: TGroupBox;
    grpEPRPDOs: TGroupBox;
    grpTesting: TGroupBox;
    grpRotoPDControl: TGroupBox;
    grpVAData: TGroupBox;
    grpPDOs: TGroupBox;
    lblCurrent: TLabel;
    lblDischargeSettings: TLabel;
    lblVoltage: TLabel;
    USBDebugLog: TMemo;
    MemoUnhandled: TMemo;
    Panel1: TPanel;
    Panel2: TPanel;
    Panel3: TPanel;
    SourceEPRPDODrawGrid: TDrawGrid;
    SourcePDODrawGrid: TDrawGrid;
    SinkPDODrawGrid: TDrawGrid;
    SinkEPRPDODrawGrid: TDrawGrid;
    TestInfoMemo: TMemo;
    ProgressBar1: TProgressBar;
    SamplesBox: TComboBox;
    StartStopButton: TSpeedButton;
    StaticText1: TStaticText;
    TestsBox: TComboBox;
    StoreTimer: TTimer;
    TestTimer: TTimer;
    UpdateTimer: TTimer;
    TypesBox: TComboBox;
    VoltageEdit: TEdit;
    procedure btnCleanLogsClick({%H-}Sender: TObject);
    procedure btnConnectKC003CClick(Sender: TObject);
    procedure btnConnectSTM32Click(Sender: TObject);
    procedure btnInitClick(Sender: TObject);
    procedure btnTestDischargeClick(Sender: TObject);
    procedure DataEditKeyPress(Sender: TObject; var Key: char);
    procedure dgFlagsDrawCell(Sender: TObject; aCol, aRow: Integer;
      aRect: TRect; aState: TGridDrawState);
    procedure dgFlagsGetCheckboxState(Sender: TObject; ACol, ARow: Integer;
      var Value: TCheckboxState);
    procedure FormShow(Sender: TObject);
    procedure gridPDOResize(Sender: TObject);
    procedure grpVADataResize(Sender: TObject);
    procedure ListBox1DrawItem(Control: TWinControl; Index: Integer;
      ARect: TRect; State: TOwnerDrawState);
    procedure ListView1CustomDrawItem(Sender: TCustomListView; Item: TListItem;
      State: TCustomDrawState; var DefaultDraw: Boolean);
    procedure ListView1KeyDown(Sender: TObject; var Key: Word;
      Shift: TShiftState);
    procedure PDODrawGridButtonClick(Sender: TObject; aCol, aRow: Integer
      );
    procedure PDODrawGridDrawCell(Sender: TObject; aCol, aRow: Integer;
      aRect: TRect; aState: TGridDrawState);
    procedure StartStopButtonClick(Sender: TObject);
    procedure StoreTimerTimer(Sender: TObject);
    procedure TestsBoxChange(Sender: TObject);
    procedure TestTimerTimer({%H-}Sender: TObject);
    procedure UpdateTimerTimer({%H-}Sender: TObject);

    procedure FormCreate({%H-}Sender: TObject);
    procedure FormClose(Sender: TObject; var CloseAction: TCloseAction);
  private
    FOldOnIdle          : TIdleEvent;

    LocalFS             : TFormatSettings;

    FSTM32BoardSerial   : string;

    RealVoltageDisplay  : TdsSevenSegmentMultiDisplay;
    RealCurrentDisplay  : TdsSevenSegmentMultiDisplay;

    NewPowerDisplay       : TLEDDisplay;
    NewEnergyDisplay      : TLEDDisplay;
    NewTemperatureDisplay : TLEDDisplay;

    Led                 : TShape;

    FSystemActive       : boolean;

    FVoltage            : double;
    FCurrent            : double;
    FEnergy             : double;
    FPower              : double;
    FTemperature        : double;

    NumRate             : integer;
    StartTime           : TDateTime;
    LastTime            : TDateTime;

    DUT                 : TUSBPDDevice;

    HPsource            : THP66332;
    HPComport           : string;

    STM32               : TLazSerial;
    STMComport          : string;
    RotoPDController    : TUSBPDController;

    PDTimer             : TFPTimer;
    DataTimer           : TFPTimer;

    TestTypes           : array of TTestType;
    ActiveTestType      : TTestType;
    BatteryDataFile     : string;

    DD:TDataDevice;

    procedure AfterShow(Sender: TObject; var Done: Boolean);
    procedure Startup;

    procedure UpdateDevice(Sender: TObject;datacarrier:integer);
    procedure UpdateData(Sender: TObject; ReportID: Byte; const Data: Pointer; {%H-}Size: Word);

    procedure SetEnable(Sender: TObject; value:boolean);
    procedure SetChartAxis({%H-}Sender:TObject);
    procedure CreateDataFile(Sender: TObject);
    procedure AllStop(Sender: TObject);
    procedure Measure;
    procedure SaveBatteryData(Elapsed:longword);

    function  CorrectVoltage(value:double):double;
    function  CorrectCurrent(value:double):double;


    procedure SetActive(value:boolean);

    procedure SetVoltage(value:double);
    function  GetVoltage:double;
    procedure SetCurrent(value:double);
    function  GetCurrent:double;
    procedure SetEnergy(value:double);
    function  GetEnergy:double;
    procedure SetPower(value:double);
    function  GetPower:double;
    procedure SetTemperature(value:double);
    function  GetTemperature:double;

    procedure Connect(Sender: TObject);
    procedure DisConnect({%H-}Sender: TObject);

    procedure DataTimerTimer({%H-}Sender: TObject);
    procedure CheckTimerTimer({%H-}Sender: TObject);
  public
    Capacity                     : double;
    property SystemActive        : boolean read FSystemActive write SetActive;
    property Voltage             : double read GetVoltage write SetVoltage;
    property Current             : double read GetCurrent write SetCurrent;
    property Energy              : double read GetEnergy write SetEnergy;
    property Power               : double read GetPower write SetPower;
    property Temperature         : double read GetTemperature write SetTemperature;
  end;

var
  PowerbankMainForm: TPowerbankMainForm;

implementation

{$R *.lfm}

uses
  TAChartAxisUtils,
  TypInfo,
  IniFiles,
  StrUtils,
  DateUtils,
  Clipbrd,
  LCLType,
  SPLASH,
  Tools;

Const
  IBUSCOLOR                    = TColor($4040FF);
  VBUSCOLOR                    = TColor($80FF80);

function ChangeBrightness(lIn: tColor; factor:double): TColor;
var
  lR,lG,lB: byte;
begin
  lR := Red(lIn);
  lG := Green(lIn);
  lB := Blue(lIn);
  result := RGBToColor(Round(lR*factor),Round(lG*factor),Round(lB*factor));
end;

{ TPowerbankMainForm }

procedure TPowerbankMainForm.FormCreate(Sender: TObject);
var
  Ini     : TIniFile;
  i       : integer;
  s       : string;
  CList,CListDetails:TStringList;
begin
  LocalFS:=DefaultFormatSettings;

  LocalFS.ShortDateFormat:='dd-mm-yyyy';
  LocalFS.LongTimeFormat:='hh:nn:ss';
  LocalFS.DecimalSeparator:=',';
  LocalFS.ListSeparator:=';';

  DUT:=TUSBPDDevice.Create;
  DUT.Cleanup;

  RotoPDController:=TUSBPDController.Create;
  RotoPDController.Cleanup;

  Led:=TShape.Create(GroupBox1);
  Led.Parent:=GroupBox1;
  Led.Width := 30;
  Led.Height := 30;
  Led.Brush.Color := clLime;
  Led.Shape := stCircle;

  RealVoltageDisplay:=TdsSevenSegmentMultiDisplay.Create(grpVAData);
  with RealVoltageDisplay do
  begin
    Parent:=grpVAData;
    OnColor:=VBUSCOLOR;
    OffColor:=ChangeBrightness(OnColor,0.1);
    DisplayCount:=6;
    Hint:='VBUS';
    ShowHint:=True;
  end;
  Chart1LineSeries1.SeriesColor:=VBUSCOLOR;
  Chart1LineSeries1.LinePen.Color:=VBUSCOLOR;
  Chart1.LeftAxis.Title.LabelFont.Color:=VBUSCOLOR;
  Chart1.LeftAxis.Marks.LabelFont.Color:=VBUSCOLOR;


  RealCurrentDisplay:=TdsSevenSegmentMultiDisplay.Create(grpVAData);
  with RealCurrentDisplay do
  begin
    Parent:=grpVAData;
    OnColor:=IBUSCOLOR;
    OffColor:=ChangeBrightness(OnColor,0.1);
    SignDigit:=True;
    DisplayCount:=6;
    Hint:='IBUS';
    ShowHint:=True;
  end;
  Chart1LineSeries2.SeriesColor:=IBUSCOLOR;
  Chart1LineSeries2.LinePen.Color:=IBUSCOLOR;
  Chart1.AxisList[2].Title.LabelFont.Color:=IBUSCOLOR;
  Chart1.AxisList[2].Marks.LabelFont.Color:=IBUSCOLOR;

  NewTemperatureDisplay := TLEDDisplay.Create(Panel3);
  with NewTemperatureDisplay do begin
    Parent := Panel3;
    Align := alClient;
    ColorLED:=clRed;
    NumDigits := 4;
    LeadingZeros := false;
    LEDContrast := 7;
    DigitLineWidth := 4;
    if (Parent IS TPanel) then
    begin
      DigitHeight := Parent.Height-TPanel(Parent).BevelWidth*2;
      if (TPanel(Parent).BevelInner<>TPanelBevel.bvNone) then DigitHeight := DigitHeight-TPanel(Parent).BevelWidth*2;
      DigitWidth := Parent.Height * 2 div 4 - 1;
    end
    else
    begin
      DigitHeight := Parent.Height;
      DigitWidth := DigitHeight * 2 div 4 - 1;
    end;
    Borderstyle := bsNone;
    BevelStyle := bvNone;
    Angle := 0;
    Clear;
    Value:=23.4;
    FractionDigits:=1;
  end;


  NewPowerDisplay := TLEDDisplay.Create(Panel1);
  with NewPowerDisplay do begin
    Parent := Panel1;
    Align := alClient;
    ColorLED:=clSilver;
    NumDigits := 5;
    LeadingZeros := false;
    LEDContrast := 7;
    DigitLineWidth := 8;
    if (Parent IS TPanel) then
    begin
      DigitHeight := Parent.Height-TPanel(Parent).BevelWidth*2;
      if (TPanel(Parent).BevelInner<>TPanelBevel.bvNone) then DigitHeight := DigitHeight-TPanel(Parent).BevelWidth*2;
      DigitWidth := Parent.Height * 2 div 4 - 1;
    end
    else
    begin
      DigitHeight := Parent.Height;
      DigitWidth := DigitHeight * 2 div 4 - 1;
    end;
    Borderstyle := bsNone;
    BevelStyle := bvNone;
    Angle := 0;
    Clear;
    Value:=-123.4;
    FractionDigits:=2;
  end;

  NewEnergyDisplay := TLEDDisplay.Create(Panel2);
  with NewEnergyDisplay do begin
    Parent := Panel2;
    Align := alClient;
    ColorLED:=clBlue;
    NumDigits := 5;
    LeadingZeros := false;
    LEDContrast := 7;
    DigitLineWidth := 8;
    if (Parent IS TPanel) then
    begin
      DigitHeight := Parent.Height-TPanel(Parent).BevelWidth*2;
      if (TPanel(Parent).BevelInner<>TPanelBevel.bvNone) then DigitHeight := DigitHeight-TPanel(Parent).BevelWidth*2;
      DigitWidth := Parent.Height * 2 div 4 - 1;
    end
    else
    begin
      DigitHeight := Parent.Height;
      DigitWidth := DigitHeight * 2 div 4 - 1;
    end;
    Borderstyle := bsNone;
    BevelStyle := bvNone;
    Angle := 0;
    Clear;
    Value:=-123.4;
    FractionDigits:=2;
  end;

  if FileExists('types.dat')
     then TypesBox.Items.LoadFromFile('types.dat')
     else TypesBox.Items.SaveToFile('types.dat');
  if TypesBox.Items.Count=1 then
  begin
    TypesBox.ItemIndex:=0;
    TypesBox.Enabled:=False;
  end;

  if FileExists('samples.dat')
     then SamplesBox.Items.LoadFromFile('samples.dat')
     else SamplesBox.Items.SaveToFile('samples.dat');
  if SamplesBox.Items.Count=1 then
  begin
    SamplesBox.ItemIndex:=0;
    SamplesBox.Enabled:=False;
  end;


  Ini := TIniFile.Create( ChangeFileExt( Application.ExeName, '.ini' ) );
  try
    NumRate           := Ini.ReadInteger('General', 'NumRate', 10);
    Self.Top          := ini.ReadInteger(Self.Name,'Top',Self.Top);
    Self.Left         := ini.ReadInteger(Self.Name,'Left',Self.Left);
    Self.Width        := ini.ReadInteger(Self.Name,'Width',Self.Width);
    Self.Height       := ini.ReadInteger(Self.Name,'Height',Self.Height);

    i:=1;
    while (i<MaxInt) do
    begin
      if (NOT Ini.SectionExists('Test'+InttoStr(i))) then break;
      Inc(i);
    end;
    Dec(i);

    if (i=0) then
    begin
      i:=1;
      Ini.WriteString('Test'+InttoStr(i), 'Name', 'Default');
      Ini.WriteInteger('Test'+InttoStr(i), 'Voltage', 5000);
      Ini.WriteInteger('Test'+InttoStr(i), 'Current', 1500);
    end;

    if (i>0) then
    begin
      SetLength(TestTypes,i);
      for i:=Low(TestTypes) to High(TestTypes) do
      begin
        TestTypes[i].Name:=Ini.ReadString('Test'+InttoStr(i+1), 'Name', '');
        TestTypes[i].Voltage:=Ini.ReadInteger('Test'+InttoStr(i+1), 'Voltage', 0);
        TestTypes[i].Current:=Ini.ReadInteger('Test'+InttoStr(i+1), 'Current', 0);
      end;
    end;

  finally
    Ini.Free;
  end;

  if (Length(TestTypes)>0) then
  begin
    for i:=Low(TestTypes) to High(TestTypes) do
    begin
      TestsBox.Items.Append(TestTypes[i].Name);
    end;
  end;

  {$ifdef WITHKEITHLEY}
  Tek4020:=TKeithley2700.Create;
  {$endif}
  HPsource:=THP66332.Create;

  STMComport:='';

  //if ((Length(STMComport)=0) OR (Length(HPComport)=0)) then
  begin
    CLIst:=TStringList.Create;
    try
      EnumerateCOMPorts(CLIst);
      CListDetails:=TStringList.Create;
      try
        for i:=0 to Pred(CLIst.Count) do
        begin
          CListDetails.Delimiter:=DefaultFormatSettings.ListSeparator;
          CListDetails.StrictDelimiter:=True;
          CListDetails.DelimitedText:=CLIst[i];
          (*
          for s in CListDetails do
          begin
            STMComport:=s;
          end;
          *)
          if (CListDetails.Count>=4) then
          begin
            s:=CListDetails[3];
            {$ifdef MSWINDOWS}
            s:=StringReplace(s,'COM','',[rfReplaceAll]);
            {$else}
            s:=StringReplace(s,'ttyUSB','',[rfReplaceAll]);
            {$endif MSWINDOWS}
            cmboSerialPorts.Items.Append(s);
            if (CListDetails.Count>=5) then
            begin
              s:=CListDetails[4];
              if (Pos('ST-Link',s)=1) then STMComport:=CListDetails[3];
              //if (Pos('POWER-Z',s)=1) then KM003CComport:=CListDetails[3];
              //if (Pos('APP',s)=1) then KM003CComport:=CListDetails[3];
              //if (Pos('FNB-58',s)=1) then KM003CComport:=CListDetails[3];
            end;
            if (Length(STMComport)=0) then if CListDetails[2]='STMicroelectronics' then STMComport:=CListDetails[3];

          end;
        end;
      finally
        CListDetails.Free
      end;
    finally
      CLIst.Free;
    end;
  end;

  FSTM32BoardSerial        := 'UNKNOWN';

  Ini := TIniFile.Create( ChangeFileExt( Application.ExeName, '.ini' ) );
  try
    Self.Top          := ini.ReadInteger(Self.Name,'Top',Self.Top);
    Self.Left         := ini.ReadInteger(Self.Name,'Left',Self.Left);
    Self.Width        := ini.ReadInteger(Self.Name,'Width',Self.Width);
    Self.Height       := ini.ReadInteger(Self.Name,'Height',Self.Height);
  finally
    Ini.Free;
  end;

  STM32:=TLazSerial.Create(Self);
  STM32.Async:=false;

  PDTimer:=TFPTimer.Create(Self);
  PDTimer.Enabled:=false;
  PDTimer.UseTimerThread:=false;
  PDTimer.Interval:=40;
  PDTimer.OnTimer:=@CheckTimerTimer;

  DataTimer:=TFPTimer.Create(Self);
  DataTimer.Enabled:=false;
  DataTimer.UseTimerThread:=false;
  DataTimer.Interval:=200;
  DataTimer.OnTimer:=@DataTimerTimer;

  // Create HID manager
  DD:=TDataDevice.Create(Application.GetNamePath);
  DD.OnDeviceChange:=@UpdateDevice;
  DD.OnDataReceived:=@UpdateData;
end;

procedure TPowerbankMainForm.grpVADataResize(Sender: TObject);
begin
  RealVoltageDisplay.Top:=2;
  RealVoltageDisplay.Left:=5;

  RealVoltageDisplay.Width:=(TControl(Sender).Width)-6;
  RealVoltageDisplay.Height:=(TControl(Sender).Height DIV 2)-12;

  RealCurrentDisplay.Width:=RealVoltageDisplay.Width;
  RealCurrentDisplay.Height:=RealVoltageDisplay.Height;
  RealCurrentDisplay.Left:=RealVoltageDisplay.Left;
  RealCurrentDisplay.Top:=RealVoltageDisplay.Top+RealVoltageDisplay.Height;
end;

procedure TPowerbankMainForm.ListBox1DrawItem(Control: TWinControl;
  Index: Integer; ARect: TRect; State: TOwnerDrawState);
var
  lb: TListBox absolute Control;
  ts: TTextStyle;
begin
  lb.Canvas.Brush.Color := PtrInt(lb.Items.Objects[Index]);
  lb.Canvas.FillRect(ARect);

  ts := lb.Canvas.TextStyle;
  ts.Alignment := taLeftJustify;
  ts.Layout := tlCenter;
  lb.Canvas.Pen.Color := clBlack;
  lb.Canvas.TextRect(ARect, ARect.Left+2, ARect.Top, lb.Items[Index], ts);
end;

procedure TPowerbankMainForm.ListView1CustomDrawItem(Sender: TCustomListView;
  Item: TListItem; State: TCustomDrawState; var DefaultDraw: Boolean);
begin
  TListView(Sender).Canvas.Brush.Color := {%H-}PtrUInt(Item.Data);
end;

procedure TPowerbankMainForm.ListView1KeyDown(Sender: TObject; var Key: Word;
  Shift: TShiftState);
var
  s: string;
  Item: TListItem;
begin
  if (Key = VK_C) and (ssCtrl in Shift) then
  begin
    s := '';
    Item := TListView(Sender).Selected;
    while Assigned(Item) do
    begin
      //if (Item.Selected AND (Item.SubItems.Count>2)) then
      //  s := s + Item.SubItems[2] + LineEnding;
      if Item.Selected then
      begin
        if (Item.SubItems.Count>3) then
          s := s + Item.SubItems[3] + LineEnding
        else
         if (Item.SubItems.Count>0) then
           s := s + Item.SubItems[0] + LineEnding
      end;
      Item := TListView(Sender).GetNextItem(Item, sdAll, [lisSelected]);
    end;

    if s <> '' then
      Clipboard.AsText := s;
  end;
end;

procedure TPowerbankMainForm.PDODrawGridButtonClick(Sender: TObject;
  aCol, aRow: Integer);
var
  PDOVoltage      : integer;
  PDOCurrent      : integer;
  PDONumber       : byte;
  Buffer          : array[0..255] of byte;
  aSourcePDO      : TSOURCEPDO;
  aSinkPDO        : TSINKPDO;
  aPDOType        : TSUPPLY_TYPES;
  aAPDOType       : TAPDO_TYPES;
begin
  PDOCurrent:=0;
  PDOVoltage:=0;

  aSourcePDO.Raw:=0;
  aSinkPDO.Raw:=0;

  PDONumber:=0;

  if Sender=SourcePDODrawGrid then aSourcePDO:=DUT.SourcePDOs[aRow];
  if Sender=SourceEPRPDODrawGrid then aSourcePDO:=DUT.SourceEPRPDOs[aRow];

  if Sender=SinkPDODrawGrid then aSinkPDO:=DUT.SinkPDOs[aRow];
  if Sender=SinkEPRPDODrawGrid then aSinkPDO:=DUT.SinkEPRPDOs[aRow];


  if ((aSourcePDO.Raw>0) OR (aSinkPDO.Raw>0)) then
  begin
    PDONumber:=aRow;
    // EPRs always start at 8 !!
    if ((Sender=SourceEPRPDODrawGrid) OR (Sender=SinkEPRPDODrawGrid)) then Inc(PDONumber,7);
  end;

  if aSourcePDO.Raw>0 then
  begin
    aPDOType:=TSUPPLY_TYPES(aSourcePDO.GenericPdo.SupplyType);

    if (aPDOType=TSUPPLY_TYPES.Fixed) then
    begin
      with aSourcePDO.FixedSupplyPdo do
      begin
        PDOCurrent:=(MaximumCurrentIn10mA*10);
        PDOVoltage:=(VoltageIn50mV*50);
      end;
    end
    else
    if (aPDOType=TSUPPLY_TYPES.Variable) then
    begin
      with aSourcePDO.VariableSupplyNonBatteryPdo do
      begin
        PDOCurrent:=(MaximumCurrentIn10mA*10);
        PDOVoltage:=(MaximumVoltageIn50mV*50);
      end;
    end
    else
    if (aPDOType=TSUPPLY_TYPES.APDO) then
    begin
      aAPDOType := TAPDO_TYPES(aSourcePDO.GenericAPdo.APOType);

      if aAPDOType=TAPDO_TYPES.SPRPPS then
      begin
        with aSourcePDO.SPRPPSPDO do
        begin
          PDOCurrent:=(MaximumCurrentIn50mA*50);
          PDOVoltage:=(MaximumVoltageIn100mV*100);
        end;
      end;

      if aAPDOType=TAPDO_TYPES.EPRAVS then
      begin
        with aSourcePDO.EPRAVSPDO do
        begin
          //PDOCurrent:=(MaximumCurrentIn50mA*50);
          PDOVoltage:=(MaximumVoltageIn100mV*100);
        end;
      end;

      if aAPDOType=TAPDO_TYPES.SPRAVS then
      begin
        with aSourcePDO.SPRAVSPDO do
        begin
          PDOCurrent:=(MaximumCurrentIn10mA15V20V*10);
          //PDOVoltage:=(MaximumVoltageIn100mV*100);
        end;
      end;


    end;

    if (PDONumber=0) then exit;

    USBDebugLog.Lines.Append('Requesting Source PDO #'+InttoStr(PDONumber)+' at '+InttoStr(PDOVoltage)+'mV and '+InttoStr(PDOCurrent)+'mA.');
  end;

end;

procedure TPowerbankMainForm.PDODrawGridDrawCell(Sender: TObject; aCol,
  aRow: Integer; aRect: TRect; aState: TGridDrawState);
var
  aDrawGrid      : TDrawGrid;
  s              : string;
  aSourcePDO     : TSOURCEPDO;
  aSinkPDO       : TSINKPDO;
  aPDOType       : TSUPPLY_TYPES;
  aAPDOType      : TAPDO_TYPES;

begin
  if ((aRow>0) AND (aCol>0)) then
  begin
    aDrawGrid:=TDrawGrid(Sender);
    s:='';
    if (aCol=1) then
    begin
      if ((Sender=SourceEPRPDODrawGrid) OR (Sender=SinkEPRPDODrawGrid)) then s:='EPDO #'+InttoStr(aRow);
      if ((Sender=SourcePDODrawGrid) OR (Sender=SinkPDODrawGrid)) then s:='PDO #'+InttoStr(aRow);
    end
    else
    begin
      s:='-';
      // aRow == PDO index !!
      // Easy ... ;-)

      if ((Sender=SourcePDODrawGrid) OR (Sender=SourceEPRPDODrawGrid)) then
      begin
        if (Sender=SourcePDODrawGrid) then aSourcePDO:=DUT.SourcePDOs[aRow];
        if (Sender=SourceEPRPDODrawGrid) then aSourcePDO:=DUT.SourceEPRPDOs[aRow];
        if (aSourcePDO.Raw>0) then
        begin
          aPDOType:=TSUPPLY_TYPES(aSourcePDO.GenericPdo.SupplyType);
          if aPDOType=TSUPPLY_TYPES.APDO then aAPDOType := TAPDO_TYPES(aSourcePDO.GenericAPdo.APOType);
          if (aCol=2) then
          begin
            s:=SUPPLY_TYPES[aPDOType];
            if aPDOType=TSUPPLY_TYPES.APDO then
            begin
              s:=APDO_TYPES[aAPDOType];
            end;
          end;
        end;
      end;

      if ((Sender=SinkPDODrawGrid) OR (Sender=SinkEPRPDODrawGrid)) then
      begin
        if (Sender=SinkPDODrawGrid) then aSinkPDO:=DUT.SinkPDOs[aRow];
        if (Sender=SinkEPRPDODrawGrid) then aSinkPDO:=DUT.SinkEPRPDOs[aRow];
        if (aSinkPDO.Raw>0) then
        begin
          aPDOType:=TSUPPLY_TYPES(aSinkPDO.GenericPdo.SupplyType);
          if (aCol=2) then s:=SUPPLY_TYPES[aPDOType];
        end;
      end;


      if Sender=SinkPDODrawGrid then
      begin
        if (aSinkPDO.Raw>0) then
        begin
          if (aPDOType=TSUPPLY_TYPES.Fixed) then
          begin
            with aSinkPDO.FixedSupplyPdo do
            begin
              if aCol=4 then s:=InttoStr(OperationalCurrentIn10mA*10)+ ' mA';
              if aCol=3 then s:=FloattoStrF(VoltageIn50mV*0.05,ffFixed,8,1)+' Volt';
            end;
          end
          else
          if (aPDOType=TSUPPLY_TYPES.Battery) then
          begin
            with aSinkPDO.BatterySupplyPdo do
            begin
              if aCol=5 then s:=UIntToStr(MaximumAllowablePowerIn250mW*250)+ 'mW';
              if aCol=3 then s:=
                FloattoStrF(MinimumVoltageIn50mV*0.05,ffFixed,8,2)+
                '-'+
                FloattoStrF(MaximumVoltageIn50mV*0.05,ffFixed,8,2)+
                ' Volt';
            end;
          end
          else
          if (aPDOType=TSUPPLY_TYPES.Variable) then
          begin
            with aSinkPDO.VariableSupplyNonBatteryPdo do
            begin
              if aCol=4 then s:=InttoStr(OperationalCurrentIn10mA*10)+ 'mA';
              if aCol=3 then s:=
                FloattoStrF(MinimumVoltageIn50mV*0.05,ffFixed,8,2)+
                '-'+
                FloattoStrF(MaximumVoltageIn50mV*0.05,ffFixed,8,2)+
                ' Volt';
            end;
          end;
        end;
      end;


      //if Sender=SourcePDODrawGrid then
      begin
        if (aSourcePDO.Raw>0) then
        begin
          if (aPDOType=TSUPPLY_TYPES.Fixed) then
          begin
            with aSourcePDO.FixedSupplyPdo do
            begin
              if aCol=4 then s:=InttoStr(MaximumCurrentIn10mA*10)+ ' mA';
              if aCol=3 then s:=InttoStr(VoltageIn50mV*50 DIV 1000)+' Volt';
            end;
          end
          else
          if (aPDOType=TSUPPLY_TYPES.Variable) then
          begin
            with aSourcePDO.VariableSupplyNonBatteryPdo do
            begin
              if aCol=4 then s:=InttoStr(MaximumCurrentIn10mA*10)+ ' mA';
              if aCol=3 then s:=InttoStr(MaximumVoltageIn50mV*50 DIV 1000)+' Volt';
            end;
          end
          else
          if (aPDOType=TSUPPLY_TYPES.APDO) then
          begin
            with aSourcePDO.SPRPPSPDO do
            begin
              if aCol=4 then s:=InttoStr(MaximumCurrentIn50mA*50)+ 'mA';
              if aCol=3 then s:=
                FloattoStrF(MinimumVoltageIn100mV*0.1,ffFixed,8,1)+
                '-'+
                FloattoStrF(MaximumVoltageIn100mV*0.1,ffFixed,8,1)+
                ' Volt';
            end;
          end;

        end;
      end;

      (*
      if Sender=SourceEPRPDODrawGrid then
      begin
        if (aSourcePDO.Raw>0) then
        begin
          if (aPDOType=TSUPPLY_TYPES.Fixed) then
          begin
            with aSourcePDO.FixedSupplyPdo do
            begin
              if aCol=4 then s:=InttoStr(MaximumCurrentIn10mA*10)+ ' mA';
              if aCol=3 then s:=InttoStr(VoltageIn50mV*50 DIV 1000)+' Volt';
            end;
          end

          if (aPDOType=TSUPPLY_TYPES.APDO) then
          begin
            if (TAPDO_TYPES(aSourcePDO.GenericAPdo.APOType)=TAPDO_TYPES.SPRPPS) then
            begin
              with aSourcePDO.SPRPPSPDO do
              begin
                if aCol=4 then s:=InttoStr(MaximumCurrentIn50mA*50)+ ' mA';
                if aCol=3 then s:=
                  FloattoStrF(MinimumVoltageIn100mV*0.1,ffFixed,8,1)+
                  '-'+
                  FloattoStrF(MaximumVoltageIn100mV*0.1,ffFixed,8,1)+
                  ' Volt';
              end;
            end;

            if (TAPDO_TYPES(aSourcePDO.GenericAPdo.APOType)=TAPDO_TYPES.EPRAVS) then
            begin
              with aSourcePDO.EPRAVSPDO do
              begin
                if aCol=4 then s:=
                   FloattoStrF(MinimumVoltageIn100mV*0.1,ffFixed,8,1)+
                   '-'+
                   FloattoStrF(MaximumVoltageIn100mV*0.1,ffFixed,8,1)+
                   ' Volt';
                if aCol=3 then s:=InttoStr(PDPInW)+' Watt';
              end;
            end;
          end;
        end;
      end;
      *)
    end;

    (*
    if DI then
    begin
      // We have invalid data
      // Show it by painting red !
      aDrawGrid.Canvas.Font.Color:=clWhite;
      aDrawGrid.Canvas.Font.Style:=[fsBold];
      aDrawGrid.Canvas.Brush.Color:=clRed;
      aDrawGrid.Canvas.FillRect(ARect);
    end;
    *)
    InflateRect(ARect, -constCellpadding, -constCellPadding);

    if (((Sender=SourcePDODrawGrid) AND (DUT.RDOPosition=aRow)) OR ((Sender=SourceEPRPDODrawGrid) AND (DUT.RDOPosition=aRow+7))) then
    begin
      aDrawGrid.Canvas.Font.Color:=clRed;
      aDrawGrid.Canvas.Font.Style:=[fsBold];
    end;

    aDrawGrid.Canvas.TextRect(ARect, ARect.Left, ARect.Top, s);
  end;
end;

procedure TPowerbankMainForm.btnCleanLogsClick(Sender: TObject);
begin
  USBDebugLog.Lines.Clear;
  MemoUnhandled.Lines.Clear;
  TestInfoMemo.Lines.Clear;
end;

procedure TPowerbankMainForm.btnConnectKC003CClick(Sender: TObject);
begin
  TButton(Sender).Enabled:=false;
  try
    Connect(Sender);
    PDTimer.Enabled:=True;
    DataTimer.Enabled:=True;
  finally
    TButton(Sender).Enabled:=true;
  end;
end;

procedure TPowerbankMainForm.btnConnectSTM32Click(Sender: TObject);
begin
  TButton(Sender).Enabled:=false;
  try
    Connect(Sender);
  finally
    TButton(Sender).Enabled:=true;
  end;
end;

procedure TPowerbankMainForm.btnInitClick(Sender: TObject);
var
  aPort:word;
begin
  if (Length(HPComport)=0) then
  begin
    aPort:=StrToIntDef(cmboSerialPorts.Text,0);
    if (aPort=0) then
    begin
      TestInfoMemo.Lines.Append('Please select serial port.');
      exit;
    end;
  end;

  TButton(Sender).Enabled:=False;
  try
    {$ifdef WITHKEITHLEY}
    TestInfoMemo.Lines.Append('Initializing DMM');
    TestInfoMemo.Invalidate;
    Tek4020.DisConnect;
    sleep(1000);
    Tek4020.Connect;
    Tek4020.Mode:=VoltageMode;
    Tek4020.Range:=3;
    Tek4020.Speed:=SlowSpeed;
    {$endif}

    if (Length(HPComport)>0) then
    begin
      TestInfoMemo.Lines.Append('Looking for HP.');
      TestInfoMemo.Invalidate;
      HPsource.DisConnect;
      sleep(1000);
      HPsource.SerialPortName:=HPComport;
      HPsource.Connect;

      if HPsource.Connected then
      begin
        TestInfoMemo.Lines.Append('Success. Connected with HP.');
        TestInfoMemo.Lines.Append('Brand: '+HPsource.Manufacturer+'.');
        TestInfoMemo.Lines.Append('Model: '+HPsource.Model+'.');
      end
      else
      begin
        TestInfoMemo.Lines.Append('HP failure.');
        TestInfoMemo.Lines.Append('Select correct port.');
      end;

      AllStop(Sender);

    end;

  finally
    TButton(Sender).Enabled:=True;
  end;
end;

procedure TPowerbankMainForm.btnTestDischargeClick(Sender: TObject);
begin
  if TSpeedButton(Sender).Down then
  begin
    if (
       (HPsource.Connected)
       //AND
       //(TypesBox.ItemIndex<>-1)
       //AND
       //(SamplesBox.ItemIndex<>-1)
       AND
       (Length(ActiveTestType.Name)>0)
       )
    then
    begin
      StartTime:=NowUTC;
      LastTime:=StartTime;
      SetChartAxis(Sender);
      SetEnable(Sender,false);
      TestInfoMemo.Lines.Append('Checking selected discharge.');
      SystemActive:=True;
      HPsource.SetOutput(SystemActive);
      HPsource.SetCurrentSlow((ActiveTestType.Current/1000));
      TestTimer.Enabled:=true;
    end else TSpeedButton(Sender).Down:=False;
  end
  else
  //if (NOT TSpeedButton(Sender).Down) then
  begin
    AllStop(Sender);
    TestInfoMemo.Lines.Append('Check finished.');
  end;
end;

procedure TPowerbankMainForm.DataEditKeyPress(Sender: TObject; var Key: char);
begin
  if (not CharInSet(Key,[#8, '0'..'9', '-', FormatSettings.DecimalSeparator])) then
  begin
    Key := #0;
  end
  else if (Key = FormatSettings.DecimalSeparator) and
          (Pos(Key, (Sender as TEdit).Text) > 0) then
  begin
    Key := #0;
  end;
  {
  else if (Key = '-') and
          ((Sender as TEdit).SelStart <> 0) then
  begin
    ShowMessage('Only allowed at beginning of number: ' + Key);
    Key := #0;
  end;
  }
end;

procedure TPowerbankMainForm.dgFlagsDrawCell(Sender: TObject; aCol,
  aRow: Integer; aRect: TRect; aState: TGridDrawState);
var
  s:string;
begin
  if aCol=1 then
  begin
    case aRow of
      0: s:='DRS';
      1: s:='UCC';
      2: s:='UP';
      3: s:='EPR';
      4: s:='DRP';
    end;
  end;
  if aCol=2 then
  begin
    case aRow of
      0: s:='Data Role Swap';
      1: s:='Usb Communication Capable';
      2: s:='Unconstrained Power';
      3: s:='EPRMode Capable';
      4: s:='Dual Role Power';
    end;
  end;

  if aCol in [1,2] then
  begin
    InflateRect(ARect, -constCellpadding, -constCellPadding);
    TDrawGrid(Sender).Canvas.TextRect(ARect, ARect.Left, ARect.Top, s);
  end;

end;

procedure TPowerbankMainForm.dgFlagsGetCheckboxState(Sender: TObject; ACol,
  ARow: Integer; var Value: TCheckboxState);
var
  aPDO:TSOURCEPDO;
  state:boolean;
begin
  if aCol=0 then
  begin
    state:=false;
    aPDO:=DUT.GetFixed5VSRCPDO;
    case aRow of
      0: state:=(aPDO.FixedSupplyPdo.DataRoleSwap=1);
      1: state:=(aPDO.FixedSupplyPdo.UsbCommunicationCapable=1);
      2: state:=(aPDO.FixedSupplyPdo.UnconstrainedPower=1);
      3: state:=(aPDO.FixedSupplyPdo.EPRModeCapable=1);
      4: state:=(aPDO.FixedSupplyPdo.DualRolePower=1);
    end;
    if state then Value:=TCheckboxState.cbChecked else Value:=TCheckboxState.cbUnchecked;
  end;

end;

procedure TPowerbankMainForm.FormShow(Sender: TObject);
begin
  FOldOnIdle := Application.OnIdle; // Save current OnIdle.
  Application.OnIdle:=@AfterShow;
end;

procedure TPowerbankMainForm.FormClose(Sender: TObject; var CloseAction: TCloseAction);
var
  Ini : TIniFile;
begin
    if (false) then
    begin
      CloseAction :=CaNone;
    end
    else if MessageDlg ('Are you REALLY SURE you want to exit ?'+
                    chr(13)+'(This is your last change to stay with us!)',
                    mtConfirmation, [mbYes,mbNo],0)=idNo
       then CloseAction :=CaNone
       else
       begin
         SetEnable(Sender,false);

         AllStop(Sender);

         DisConnect(Sender);

         UpdateTimer.Enabled:=False;

         //DD.Enabled:=False;
         DD.OnDataReceived:=nil;
         DD.OnDeviceChange:=nil;
         DD.Destroy;
         HPsource.Destroy;
         DUT.Destroy;
         RotoPDController.Destroy;

         Ini := TIniFile.Create( ChangeFileExt( Application.ExeName, '.ini' ) );
         try
           ini.WriteInteger(Self.Name,'Top',Self.Top);
           ini.WriteInteger(Self.Name,'Left',Self.Left);
           ini.WriteInteger(Self.Name,'Width',Self.Width);
           ini.WriteInteger(Self.Name,'Height',Self.Height);

           Ini.WriteInteger('General', 'NumRate', NumRate);
         finally
           Ini.Free;
         end;


         CloseAction:=caFree;
       end;
end;

procedure TPowerbankMainForm.StartStopButtonClick(Sender: TObject);
begin
  TSpeedButton(Sender).Enabled:=False;
  try
    if TSpeedButton(Sender).Down then
    begin
      if (
         //(HPsource.Connected)
         //AND
         (TypesBox.ItemIndex<>-1)
         AND
         (SamplesBox.ItemIndex<>-1)
         AND
         (Length(ActiveTestType.Name)>0)
         )
      then
      begin
        SetEnable(Sender,false);

        Voltage:=0;
        Current:=0;

        Energy:=0;

        SetChartAxis(Sender);

        CreateDataFile(Sender);
        TestInfoMemo.Lines.Append('Filename: '+BatteryDataFile);

        SystemActive:=True;
        HPsource.SetOutput(SystemActive);
        HPsource.SetCurrentSlow(ActiveTestType.Current/1000);

        TestInfoMemo.Lines.Append('Test started.');

        StartTime:=NowUTC;
        LastTime:=StartTime;

        StoreTimer.Enabled:=False;
        StoreTimer.Interval:=NumRate*1000;
        StoreTimer.Enabled:=True;

        StoreTimerTimer(nil);

      end else TSpeedButton(Sender).Down:=false;
    end
    else
    begin
      TestInfoMemo.Lines.Append('Test stopped.');
    end;

    if TSpeedButton(Sender).Down then
    begin
      TSpeedButton(Sender).Caption:='Stop';
      TSpeedButton(Sender).Font.Color:=clRed;
    end
    else
    begin
      AllStop(Sender);
    end;

  finally
    TSpeedButton(Sender).Enabled:=True;
  end;
end;

procedure TPowerbankMainForm.StoreTimerTimer(Sender: TObject);
const
  STOPPERCENTAGE = 80;
var
  Elapsed:longword;
begin
  Measure;

  Chart1LineSeries1.Add(Voltage);
  Chart1LineSeries2.Add(Current);

  if SystemActive then
  begin
    // only perform the below if coming from the timer !
    if Assigned(Sender) then
    begin
      Elapsed:=MilliSecondsBetween(NowUTC,LastTime);
      LastTime:=NowUTC;

      Power:=Voltage*Current;
      Capacity:=Capacity+Current*(Elapsed/3600000);
      Energy:=Energy+Power*(Elapsed/3600000);

      SaveBatteryData(Elapsed);

      if (
         (chkVoltageLimit.Checked)
         AND
         (Voltage<(ActiveTestType.Voltage*(STOPPERCENTAGE/100)/1000))
         )
      then
      begin
        TestInfoMemo.Lines.Append('Voltage below threshold. Test ready.');
        AllStop(Sender);
      end;

      if (
         (chkCurrentLimit.Checked)
         AND
         (Current<(ActiveTestType.Current*(STOPPERCENTAGE/100)/1000))
         )
      then
      begin
        TestInfoMemo.Lines.Append('Current below threshold. Test ready.');
        AllStop(Sender);
      end;

    end;

  end;
end;

procedure TPowerbankMainForm.TestsBoxChange(Sender: TObject);
var
  aCombo:TComboBox;
begin
  aCombo:=nil;
  if (Sender<>nil) then aCombo:=TComboBox(Sender);
  if ((aCombo<>nil) AND (aCombo.ItemIndex<>-1)) then
  begin
    ActiveTestType.Name:=TestTypes[aCombo.ItemIndex].Name;
    ActiveTestType.Current:=TestTypes[aCombo.ItemIndex].Current;
    ActiveTestType.Voltage:=TestTypes[aCombo.ItemIndex].Voltage;
  end
  else
  begin
    ActiveTestType.Name:='';
    ActiveTestType.Current:=0;
    ActiveTestType.Voltage:=0;
  end;
  CurrentEdit.Text:=InttoStr(ActiveTestType.Current);
  VoltageEdit.Text:=InttoStr(ActiveTestType.Voltage);

  CurrentEdit.ReadOnly:=((Pos('Power',ActiveTestType.Name)=0) AND (Pos('Variable',ActiveTestType.Name)=0));
  VoltageEdit.ReadOnly:=CurrentEdit.ReadOnly;
end;

procedure TPowerbankMainForm.TestTimerTimer(Sender: TObject);
begin
  if (ProgressBar1.Position=ProgressBar1.Max) then
    ProgressBar1.Position:=1
  else
    ProgressBar1.StepIt;
end;

procedure TPowerbankMainForm.UpdateTimerTimer(Sender: TObject);
begin
  if (StartTime<>0) then
    Edit1.Text:='Running: '+InttoStr(SecondsBetween(NowUTC,StartTime))+' sec'
  else
    Edit1.Text:='CONSULAB    ' + DateTimeToStr(NowUTC);
end;

procedure TPowerbankMainForm.SetEnable(Sender: TObject; value:boolean);
begin
  if Sender<>nil then
  begin
    if (Sender<>btnInit) then btnInit.Enabled:=value;
    if (Sender<>cmboSerialPorts) then cmboSerialPorts.Enabled:=value;

    if (Sender<>StartStopButton) then StartStopButton.Enabled:=value;

    if (Sender<>btnTestDischarge) then btnTestDischarge.Enabled:=value;

    if (Sender<>TypesBox) then TypesBox.Enabled:=value;
    if (Sender<>SamplesBox) then SamplesBox.Enabled:=value;
    if (Sender<>TestsBox) then TestsBox.Enabled:=value;
    if (Sender<>CurrentEdit) then CurrentEdit.Enabled:=value;
    if (Sender<>VoltageEdit) then VoltageEdit.Enabled:=value;

    if (Sender<>chkVoltageLimit) then chkVoltageLimit.Enabled:=value;
    if (Sender<>chkCurrentLimit) then chkCurrentLimit.Enabled:=value;

    Application.ProcessMessages;
  end;
end;


procedure TPowerbankMainForm.gridPDOResize(Sender: TObject);
var
  aGrid:TStringGrid;
begin
  aGrid:=TStringGrid(Sender);
  aGrid.Height:=aGrid.DefaultRowHeight*aGrid.RowCount+4;
end;

function TPowerbankMainForm.CorrectVoltage(value:double):double;
begin
  result:=value;
end;
function TPowerbankMainForm.CorrectCurrent(value:double):double;
begin
  result:=value;
end;

procedure TPowerbankMainForm.SetVoltage(value:double);
begin
  if (FVoltage<>value) then
  begin
    FVoltage:=value;
    RealVoltageDisplay.Value:=value;
  end;
end;

function  TPowerbankMainForm.GetVoltage:double;
begin
  result:=FVoltage;
end;

procedure TPowerbankMainForm.SetCurrent(value:double);
begin
  if (FCurrent<>value) then
  begin
    FCurrent:=value;
    RealCurrentDisplay.Value:=value;
  end;
end;

function  TPowerbankMainForm.GetCurrent:double;
begin
  result:=FCurrent;
end;

procedure TPowerbankMainForm.SetEnergy(value:double);
begin
  if (FEnergy<>value) then
  begin
    FEnergy:=value;
    NewEnergyDisplay.Value:=value;
  end;
end;

function  TPowerbankMainForm.GetEnergy:double;
begin
  result:=FEnergy;
end;

procedure TPowerbankMainForm.SetPower(value:double);
begin
  if (FPower<>value) then
  begin
    FPower:=value;
    NewPowerDisplay.Value:=value;
  end;
end;

function  TPowerbankMainForm.GetPower:double;
begin
  result:=FPower;
end;

procedure TPowerbankMainForm.SetTemperature(value:double);
begin
  if (FTemperature<>value) then
  begin
    FTemperature:=value;
    NewTemperatureDisplay.Value:=value;
  end;
end;

function  TPowerbankMainForm.GetTemperature:double;
begin
  result:=FTemperature;
end;

procedure TPowerbankMainForm.DataTimerTimer(Sender: TObject);
var
  success               : boolean;
begin
  success:=false;
end;

procedure TPowerbankMainForm.CheckTimerTimer(Sender: TObject);
begin
end;

procedure TPowerbankMainForm.DisConnect(Sender: TObject);
begin
  PDTimer.Enabled:=False;
  DataTimer.Enabled:=False;

  STM32.Active:=False;
end;

procedure TPowerbankMainForm.Connect(Sender: TObject);
begin
  DisConnect(Sender);
end;

procedure TPowerbankMainForm.SetChartAxis(Sender:TObject);
begin
  Chart1LineSeries1.Clear;
  Chart1LineSeries2.Clear;
  Chart1LineSeries3.Clear;
  {$ifdef FPC}
  //Chart1LineSeries3.Active:=(Sender=btnCurve);
  {$else}
  //Chart1LineSeries3.Visible:=(Sender=btnCurve);
  {$endif}
  {$ifndef FPC}
  Chart1.UndoZoom;
  {$endif}
  {$ifdef FPC}
  Chart1.AxisList.GetAxisByAlign(calRight).Range.Min:=0;
  Chart1.AxisList.GetAxisByAlign(calRight).Range.UseMin:=True;
  Chart1.AxisList.GetAxisByAlign(calRight).Range.Max:=ActiveTestType.Current*1.2/1000;
  Chart1.AxisList.GetAxisByAlign(calRight).Range.UseMax:=True;
  Chart1.AxisList.GetAxisByAlign(calRight).Intervals.Count:=10;
  Chart1.AxisList.GetAxisByAlign(calLeft).Range.Min:=0;
  Chart1.AxisList.GetAxisByAlign(calLeft).Range.UseMin:=True;
  Chart1.AxisList.GetAxisByAlign(calLeft).Range.Max:=ActiveTestType.Voltage*1.2/1000;
  Chart1.AxisList.GetAxisByAlign(calLeft).Range.UseMax:=True;
  Chart1.AxisList.GetAxisByAlign(calLeft).Intervals.Count:=10;
  {$else}
  Chart1.RightAxis.SetMinMax(0,ActiveTestType.Current*1.2/1000);
  Chart1.RightAxis.Increment:=0.1;
  {$endif}
end;

procedure TPowerbankMainForm.CreateDataFile(Sender: TObject);
var
  BatteryDataFileBackup      : string;
  Oldfile,NewFile            : TFileStream;
  TimeString                 : string;
begin
  TimeString := Format('%.2d-%.2d-%.4d',[DayOfTheMonth(NowUTC), MonthOfTheYear(NowUTC), YearOf(NowUTC)])+'__';
  TimeString := TimeString + Format('%.2d-%.2d-%.2d',[HourOfTheDay(NowUTC), MinuteOfTheHour(NowUTC), SecondOfTheMinute(NowUTC)])+'__';

  BatteryDataFile:=Copy(TypesBox.Text,1,pos(' ',TypesBox.Text)-1)+'_'+SamplesBox.Text;
  if Assigned(Sender) then
  begin
    //if Sender=btnCurve then BatteryDataFile:=BatteryDataFile+'_Curve';
    if Sender=StartStopButton then BatteryDataFile:=BatteryDataFile+'_Normal';
  end
  else
  begin
    BatteryDataFile:=BatteryDataFile+'_Special';
  end;
  BatteryDataFile:=BatteryDataFile+'_'+ActiveTestType.Name+'__'+TimeString;
  BatteryDataFile:=BatteryDataFile+'data.csv';

  BatteryDataFile:=StringReplace(BatteryDataFile,' ','-',[rfReplaceAll]);
  BatteryDataFile:=StringReplace(BatteryDataFile,'@','-',[rfReplaceAll]);
  BatteryDataFile:=StringReplace(BatteryDataFile,'/','-',[rfReplaceAll]);

  BatteryDataFile:=ExtractFilePath(Application.ExeName)+BatteryDataFile;

  if FileExists(BatteryDataFile) then
  begin
    TestInfoMemo.Lines.Append('Making backup of batterydatafile.');

    //BatteryDataFileBackup := ExtractFileName(BatteryDataFile);
    //BatteryDataFileBackup := ExtractFilePath(Application.ExeName)+ChangeFileExt(BatteryDataFileBackup, '.bak');

    BatteryDataFileBackup := BatteryDataFile+'.bak';

    OldFile := TFileStream.Create(BatteryDataFile, fmOpenRead or fmShareDenyWrite);
    try
      NewFile := TFileStream.Create(BatteryDataFileBackup, fmCreate or fmShareDenyRead);
      try
        NewFile.CopyFrom(OldFile, OldFile.Size);
      finally
        FreeAndNil(NewFile);
      end;
    finally
      FreeAndNil(OldFile);
    end;
    DeleteFile(BatteryDataFile);
    TestInfoMemo.Lines.Append('Making backup of batterydatafile done.');
  end;
end;

procedure TPowerbankMainForm.AllStop(Sender: TObject);
begin
  StoreTimer.Enabled:=False;
  TestTimer.Enabled:=False;

  PDTimer.Enabled:=False;
  DataTimer.Enabled:=False;

  StartTime:=0;

  SystemActive:=False;
  if HPsource.Connected then
  begin
    HPsource.SetOutput(SystemActive);
  end;
  SetEnable(Sender,true);

  StartStopButton.Down:=false;
  StartStopButton.Caption:='Start';
  StartStopButton.Font.Color:=clLime;

  btnTestDischarge.Down:=false;

  ProgressBar1.Position:=0;
end;

procedure TPowerbankMainForm.Measure;
begin
  Voltage:=RealVoltageDisplay.Value;
  Current:=RealCurrentDisplay.Value;
  if (false) then
  begin
    //if HPsource.Connected then
    if true then
    begin
      HPsource.Measure;
      Current:=Abs(HPsource.Current);
      Voltage:=HPsource.Voltage;
    end
    else
    begin
      Sleep(100);
      Current:=1.456;
      Voltage:=5.123;
    end;
  end;
end;

procedure TPowerbankMainForm.SaveBatteryData(Elapsed:longword);
var
  F : textfile;
  PC,PB,PS:string;
  i:integer;
begin
  i:=pos(' ',TypesBox.Text);
  PC:=Copy(TypesBox.Text,1,i-1);
  PB:=Copy(TypesBox.Text,i+1,MaxInt);
  i:=pos(' ',PB);
  PS:=Copy(PB,i+1,MaxInt);
  PB:=Copy(PB,1,i-1);
  if (Length(BatteryDataFile)=0) then CreateDataFile(nil);

  AssignFile(F,BatteryDataFile );
  try

    if FileExists(BatteryDataFile)
       then Append(F)
       else
       begin
         Rewrite(F);
         writeln(F,'Code: ',PC);
         writeln(F,'Brand: ',PB);
         writeln(F,'Model: ',PS);
         writeln(F,'Sample: ',SamplesBox.Text);
         writeln(F,'Test: ',ActiveTestType.Name);
         writeln(F,'Current: ',ActiveTestType.Current,' mA');
         writeln(F,'Voltage: ',ActiveTestType.Voltage,' mV');
         //writeln(F,'Extra info:');
         //writeln(F,DataInfoMemo.Lines.Text);
         writeln(F);
         writeln(F,DateTimeToStr(NowUTC));
         writeln(F,'*************************************************');
         writeln(F);

         write(F,'B_code',LocalFS.ListSeparator);
         write(F,'Sample#',LocalFS.ListSeparator);
         write(F,'Battery mode',LocalFS.ListSeparator); // = CD
         write(F,'Battery mode value',LocalFS.ListSeparator);
         write(F,'Trigger moment UTC',LocalFS.ListSeparator);
         write(F,'Human time',LocalFS.ListSeparator);
         write(F,'Time(sec)',LocalFS.ListSeparator);
         write(F,'Voltage(V)',LocalFS.ListSeparator);
         write(F,'Current(mA)',LocalFS.ListSeparator);
         write(F,'Capacity(mAh)',LocalFS.ListSeparator);
         write(F,'Energy(mWh)',LocalFS.ListSeparator);
         writeln(F);
       end;

    if (ProgressBar1.Position=ProgressBar1.Max) then
      ProgressBar1.Position:=1
    else
      ProgressBar1.StepIt;

    write(F,PC,LocalFS.ListSeparator);
    write(F,SamplesBox.Text,LocalFS.ListSeparator);
    write(F,'CD',LocalFS.ListSeparator);
    write(F,ActiveTestType.Current,LocalFS.ListSeparator);
    write(F,FloattoStr(NowUTC,LocalFS),LocalFS.ListSeparator);
    write(F,FormatDateTime('dd-mm-yyyy hh:nn:ss',NowUTC),LocalFS.ListSeparator);
    write(F,FloattoStrF((Elapsed/1000),ffFixed,6,1,LocalFS),LocalFS.ListSeparator);
    write(F,FloattoStrF(Voltage,ffFixed,10,3,LocalFS),LocalFS.ListSeparator);
    write(F,FloattoStrF(Current,ffFixed,10,4,LocalFS),LocalFS.ListSeparator);
    write(F,FloattoStrF(Capacity,ffFixed,10,1,LocalFS),LocalFS.ListSeparator);
    write(F,FloattoStrF(Energy,ffFixed,10,1,LocalFS),LocalFS.ListSeparator);

    writeln(F);
  finally
    CloseFile(F);
  end;
end;

procedure TPowerbankMainForm.SetActive(value:boolean);
begin
  if value<>FSystemActive then
  begin
    FSystemActive:=value;
    if value
       then Led.Brush.Color := clRed
       else Led.Brush.Color := clLime;
  end;
end;

procedure TPowerbankMainForm.AfterShow(Sender: TObject; var Done: Boolean);
begin
  Application.OnIdle := FOldOnIdle; // Restore catched OnIdle.
  Startup;
end;


procedure TPowerbankMainForm.Startup;
begin
  if Assigned(DD) then
  begin
    if (NOT DD.Enabled) then
    begin
      DD.Enabled:=True;
    end;
  end;
  UpdateTimer.Enabled:=True;
end;

procedure TPowerbankMainForm.UpdateData(Sender: TObject; ReportID: Byte; const Data: Pointer; {%H-}Size: Word);
var
  measuredvalue:double;
begin
  if (PByteArray(data)^[0]=Ord(TCommands.CMD_get_data)) then
  begin
    //voltage
    measuredvalue:=(UInt16(PByteArray(data)^[3])+UInt16(PByteArray(data)^[4])*256);
    voltage:=measuredvalue/1000;

    //current
    measuredvalue:=(UInt16(PByteArray(data)^[5])+UInt16(PByteArray(data)^[6])*256);
    current:=measuredvalue/1000;

    //power
    measuredvalue:=(UInt32(PByteArray(data)^[7])+UInt32(PByteArray(data)^[8])*256+UInt32(PByteArray(data)^[9])*256*256+UInt32(PByteArray(data)^[10])*256*256*256);
    power:=measuredvalue/1000;
    //power:=measuredvalue/(3600*1000);

    // Temperature
    temperature:=(UInt16(PByteArray(data)^[11])+UInt16(PByteArray(data)^[12])*256)/10;
  end
  else
  begin
    USBDebugLog.Lines.Append('Got other data !!!!');
  end;
end;

procedure TPowerbankMainForm.UpdateDevice(Sender: TObject;datacarrier:integer);
var
  Info:string;
begin
  if datacarrier<>0 then
  begin
    if datacarrier>0 then Info:='#'+InttoStr(datacarrier)+' board added.';
    if datacarrier<0 then Info:='#'+InttoStr(-1*datacarrier)+' board removed.';
    USBDebugLog.Lines.Append(Info);
    Info:=DD.Errors;
    if (Length(Info)>0) then USBDebugLog.Lines.Append(Info);
    Info:=DD.Info;
    if (Length(Info)>0) then USBDebugLog.Lines.Append(Info);
  end;
  if datacarrier=0 then USBDebugLog.Lines.Append('Unknown board error');
  {$ifdef MSWINDOWS}
  SplashForm:=TSplashForm.Create(application);
  Splashform.Label2.Caption:='Board '+InttoStr(abs(datacarrier))+' change';
  SplashForm.Update;
  //Splashform.Label3.Caption:='Init ready';
  SplashForm.Update;
  sleep(500);
  if Assigned(SplashForm) then SplashForm.Destroy;
  SplashForm:=nil;
  {$endif}

  if (datacarrier>0) then
  begin
    Info:=DD.Errors;
    if (Length(Info)>0) then USBDebugLog.Lines.Append(Info);
    Info:=DD.Info;
    if (Length(Info)>0) then USBDebugLog.Lines.Append(Info);
  end;
  Application.ProcessMessages;
end;

end.

