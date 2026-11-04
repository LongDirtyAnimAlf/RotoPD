unit main;

{$ifdef FPC}
{$mode Delphi}
//{$mode objfpc}
{$endif}

{$H+}

interface

uses
  {$ifdef Windows}
  Windows,
  {$endif}
  Classes, SysUtils, Forms, Controls, Graphics, Dialogs, StdCtrls, ExtCtrls,
  Grids, Buttons, ComCtrls, Menus, Types,
  TATransformations, TAGraph,
  TASeries,TACustomSeries, TATools,
  LMessages, LCLType,
  SynEdit,
  Generics.Collections,
  //fgl,
  //SynEditMiscClasses,
  processframe, processframe_simple,
  fptimernew,
  dsLeds,
  dlLEDCtrl,
  usbcpd,
  pddevice,
  ap33772s,
  usbrotopd;

type
  TMachineData = record
    Time:single;
    Temperature1 : single;
    Temperature2 : single;
    Water1 : single;
    Water2 : single;
    Energy : single;
  end;
  TDataList = TList<TMachineData>;
  //TDataList = specialize TList<TMachineData>;
  //TDataList = specialize TFPGList<TMachineData>;

  TTestType = record
    Name:string;
    Current:word;
    Voltage:word;
  end;

  TUSBPDController = class(TUSBPD)
  end;

  {$ifdef MSWindows}
  TPageControl = class(ComCtrls.TPageControl)
  private
    procedure CNDrawItem(var Message: TLMDrawItems); message LM_DRAWITEM;
  end;
  {$endif}

  { TPowerbankMainForm }

  TPowerbankMainForm = class(TForm)
    AutoFileNameCheckBox: TCheckBox;
    btnRotoPDGetPDOList: TButton;
    btnTestDischarge: TSpeedButton;
    Button2: TButton;
    MachineChart: TChart;
    MachineChartLineSeries1: TLineSeries;
    MachineChartLineSeries2: TLineSeries;
    MachineChartLineSeries3: TLineSeries;
    chkCurrentLimit: TCheckBox;
    chkVoltageLimit: TCheckBox;
    CurrentEdit: TEdit;
    DisplaysPanel: TPanel;
    Edit1: TEdit;
    FileLabel: TLabel;
    FilenameEdit: TEdit;
    GroupBox1: TGroupBox;
    grpEPRPDOs: TGroupBox;
    grpTesting: TGroupBox;
    grpRotoPDControl: TGroupBox;
    grpVAData: TGroupBox;
    grpPDOs: TGroupBox;
    InfoLabel: TLabel;
    lblCurrent: TLabel;
    lblDischargeSettings: TLabel;
    lblVoltage: TLabel;
    MachineInfoMemo: TMemo;
    MachineRemoveClick: TMenuItem;
    MT1: TAutoScaleAxisTransform;
    MyTransform1: TChartAxisTransformations;
    MyTransform2: TChartAxisTransformations;
    MyTransform2AutoScaleAxisTransform1: TAutoScaleAxisTransform;
    MemoUnhandled: TMemo;
    PageControl1: TPageControl;
    Panel1: TPanel;
    Panel2: TPanel;
    Panel3: TPanel;
    Panel4: TPanel;
    MachineGroupBox: TPanel;
    PopupMenu1: TPopupMenu;
    ProductLabel: TLabel;
    Product_edit: TEdit;
    ProjectLabel: TLabel;
    Project_edit: TEdit;
    RunLabel: TLabel;
    Runname_edit: TEdit;
    RunNumberLabel: TLabel;
    Runnumber_edit: TEdit;
    SourceEPRPDODrawGrid: TDrawGrid;
    SourcePDODrawGrid: TDrawGrid;
    SinkPDODrawGrid: TDrawGrid;
    SinkEPRPDODrawGrid: TDrawGrid;
    Machine1Sheet: TTabSheet;
    tsOverview1: TTabSheet;
    tsOverview2: TTabSheet;
    TestInfoMemo: TMemo;
    ProgressBar1: TProgressBar;
    SamplesBox: TComboBox;
    StartStopSpeedButton: TSpeedButton;
    StaticText1: TStaticText;
    TestsBox: TComboBox;
    StoreTimer: TTimer;
    TestTimer: TTimer;
    UpdateTimer: TTimer;
    TypesBox: TComboBox;
    USBDebugLog: TMemo;
    VoltageEdit: TEdit;
    procedure AutoFileNameCheckBoxClick(Sender: TObject);
    procedure btnCleanLogsClick({%H-}Sender: TObject);
    procedure btnRotoPDGetPDOListClick(Sender: TObject);
    procedure btnTestDischargeClick(Sender: TObject);
    procedure Button2Click(Sender: TObject);
    procedure DataEditKeyPress(Sender: TObject; var Key: char);
    procedure dgFlagsDrawCell(Sender: TObject; aCol, aRow: Integer;
      aRect: TRect; aState: TGridDrawState);
    procedure dgFlagsGetCheckboxState(Sender: TObject; ACol, ARow: Integer;
      var Value: TCheckboxState);
    procedure FormDestroy(Sender: TObject);
    procedure FormShow(Sender: TObject);
    procedure gridPDOResize(Sender: TObject);
    procedure grpVADataResize(Sender: TObject);
    procedure ListBox1DrawItem(Control: TWinControl; Index: Integer;
      ARect: TRect; State: TOwnerDrawState);
    procedure ListView1CustomDrawItem(Sender: TCustomListView; Item: TListItem;
      State: TCustomDrawState; var DefaultDraw: Boolean);
    procedure ListView1KeyDown(Sender: TObject; var Key: Word;
      Shift: TShiftState);
    procedure MachineRemoveClickClick(Sender: TObject);
    procedure PageControl1Change(Sender: TObject);
    procedure PageControl1Changing(Sender: TObject; var AllowChange: Boolean);
    procedure PageControl1DragDrop(Sender, Source: TObject; X, Y: Integer);
    procedure PageControl1DragOver(Sender, Source: TObject; X, Y: Integer;
      State: TDragState; var Accept: Boolean);
    procedure PageControl1MouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure PDODrawGridButtonClick(Sender: TObject; aCol, aRow: Integer
      );
    procedure PDODrawGridDrawCell(Sender: TObject; aCol, aRow: Integer;
      aRect: TRect; aState: TGridDrawState);
    procedure StartStopSpeedButtonClick(Sender: TObject);
    procedure StoreTimerTimer(Sender: TObject);
    procedure TestsBoxChange(Sender: TObject);
    procedure TestTimerTimer({%H-}Sender: TObject);
    procedure UpdateTimerTimer({%H-}Sender: TObject);

    procedure FormCreate({%H-}Sender: TObject);
    procedure FormClose(Sender: TObject; var CloseAction: TCloseAction);
  private
    MachineBox: array of TFrameSimple;

    Machineset : array of
    record
      Startup        : boolean;
      Running        : boolean;
      Start_time     : TDateTime;
      Running_time   : TDateTime;
      Energy         : single;
      Water1         : single;
      Water2         : single;
      Temperature1   : single;
      Temperature2   : single;
      MaxTemp        : single;
      AutoFileName   : boolean;
      AutoStart      : boolean;
      AutoStartReady : boolean;
      Project        : string;
      Product        : string;
      Runname        : string;
      Runnumber      : string;
      Info           : string;
      Filename       : string;
      Water1Cal      : double;
      Water2Cal      : double;
      EnergyCal      : double;
      //TemperatureCal : double;
      TemperatureCalNew: double;
      Data           : TDataList;
    end;

    FOldOnIdle          : TIdleEvent;

    LocalFormatSettings : TFormatSettings;

    RealVoltageDisplay  : TdsSevenSegmentMultiDisplay;
    RealCurrentDisplay  : TdsSevenSegmentMultiDisplay;

    NewPowerDisplay       : TLEDDisplay;
    NewEnergyDisplay      : TLEDDisplay;
    NewAHDisplay          : TLEDDisplay;
    NewTemperatureDisplay : TLEDDisplay;

    Led                 : TShape;

    FSystemActive       : boolean;

    FVoltage            : double;
    FCurrent            : double;
    FCapacity           : double;
    FEnergy             : double;
    FPower              : double;
    FTemperature        : double;

    NumRate             : integer;
    StartTime           : TDateTime;
    LastTime            : TDateTime;

    DUT                 : TUSBPDDevice;

    ActiveBoard         : word;
    VisibleMachine      : word;
    ApplicationDirectory:string;

    RotoPDController    : TUSBPDController;

    TestTypes           : array of TTestType;
    ActiveTestType      : TTestType;
    BatteryDataFile     : string;

    DD                  : TDataDevice;

    NumberOfMachines    : integer;
    BaseMachineNumber   : byte;


    procedure GraphBaseClick(Sender: TObject);
    procedure SaveMachineData;
    procedure SetControls(Sender:TObject;localsetting:boolean);

    procedure AfterShow(Sender: TObject; var Done: Boolean);
    procedure Startup;

    procedure UpdateDevice(Sender: TObject;datacarrier:integer);
    procedure UpdateUSB(Sender: TObject;datacarrier:integer);
    procedure UpdateData(Sender: TObject; ReportID: Byte; const Data: Pointer; {%H-}Size: Word);

    procedure SetEnable(Sender: TObject; value:boolean);
    procedure SetChartAxis({%H-}Sender:TObject);
    procedure CreateDataFile(Sender: TObject);
    procedure AllStop(Sender: TObject);
    procedure SaveBatteryData(Elapsed:longword);

    function  CorrectVoltage(value:double):double;
    function  CorrectCurrent(value:double):double;

    procedure SetActive(value:boolean);
    procedure SetActiveTestType(const TT:TTestType);

    procedure SetVoltage(value:double);
    function  GetVoltage:double;
    procedure SetCurrent(value:double);
    function  GetCurrent:double;
    procedure SetCapacity(value:double);
    function  GetCapacity:double;
    procedure SetEnergy(value:double);
    function  GetEnergy:double;
    procedure SetPower(value:double);
    function  GetPower:double;
    procedure SetTemperature(value:double);
    function  GetTemperature:double;
  public
    property SystemActive        : boolean read FSystemActive write SetActive;
    property Voltage             : double read GetVoltage write SetVoltage;
    property Current             : double read GetCurrent write SetCurrent;
    property Capacity                  : double read GetCapacity write SetCapacity;
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
  SPLASH,
  Bits,
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
  ts:TTabSheet;
begin
  PageControl1.OwnerDraw:=True;

  ApplicationDirectory:=ExtractFilePath(Application.ExeName);

  LocalFormatSettings:=DefaultFormatSettings;

  LocalFormatSettings.ShortDateFormat:='dd-mm-yyyy';
  LocalFormatSettings.LongTimeFormat:='hh:nn:ss';
  LocalFormatSettings.DecimalSeparator:=',';
  LocalFormatSettings.ListSeparator:=';';

  ActiveBoard := 1;

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
  MachineChartLineSeries1.SeriesColor:=VBUSCOLOR;
  MachineChartLineSeries1.LinePen.Color:=VBUSCOLOR;
  MachineChart.LeftAxis.Title.LabelFont.Color:=VBUSCOLOR;
  MachineChart.LeftAxis.Marks.LabelFont.Color:=VBUSCOLOR;


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
  MachineChartLineSeries2.SeriesColor:=IBUSCOLOR;
  MachineChartLineSeries2.LinePen.Color:=IBUSCOLOR;
  MachineChart.AxisList[2].Title.LabelFont.Color:=IBUSCOLOR;
  MachineChart.AxisList[2].Marks.LabelFont.Color:=IBUSCOLOR;

  NewTemperatureDisplay := TLEDDisplay.Create(Panel3);
  with NewTemperatureDisplay do begin
    Parent := Panel3;
    Align := alClient;
    ColorLED:=clBlue;
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
    ColorLED:=clRed;
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
    Value:=0;
    FractionDigits:=3;
    Hint:='Power [W]';
    ShowHint:=True;
  end;

  NewEnergyDisplay := TLEDDisplay.Create(Panel2);
  with NewEnergyDisplay do begin
    Parent := Panel2;
    Align := alClient;
    ColorLED:=clRed;
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
    Value:=0;
    FractionDigits:=3;
    Hint:='Energy [WH]';
    ShowHint:=True;
  end;

  NewAHDisplay := TLEDDisplay.Create(Panel4);
  with NewAHDisplay do begin
    Parent := Panel4;
    Align := alClient;
    ColorLED:=clRed;
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
    Value:=0;
    FractionDigits:=3;
    Hint:='Capacity [AH]';
    ShowHint:=True;
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

    SetLength(TestTypes,i+2);
    for i:=Low(TestTypes) to High(TestTypes) do
    begin
      if (i=Low(TestTypes)) then
      begin
        TestTypes[i].Name:='Default';
        TestTypes[i].Voltage:=5000;
        TestTypes[i].Current:=1500;
      end
      else
      if (i=(Low(TestTypes)+1)) then
      begin
        TestTypes[i].Name:='PDMax';
        TestTypes[i].Voltage:=0;
        TestTypes[i].Current:=0;
      end
      else
      begin
        TestTypes[i].Name:=Ini.ReadString('Test'+InttoStr(i-1), 'Name', '');
        TestTypes[i].Voltage:=Ini.ReadInteger('Test'+InttoStr(i-1), 'Voltage', 0);
        TestTypes[i].Current:=Ini.ReadInteger('Test'+InttoStr(i-1), 'Current', 0);
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

  Ini := TIniFile.Create( ChangeFileExt( Application.ExeName, '.ini' ) );
  try
    Self.Top          := ini.ReadInteger(Self.Name,'Top',Self.Top);
    Self.Left         := ini.ReadInteger(Self.Name,'Left',Self.Left);
    Self.Width        := ini.ReadInteger(Self.Name,'Width',Self.Width);
    Self.Height       := ini.ReadInteger(Self.Name,'Height',Self.Height);
  finally
    Ini.Free;
  end;

  BaseMachineNumber := 1;
  NumberOfMachines  := 8;

  SetLength(MachineBox,NumberOfMachines+1);
  SetLength(Machineset,NumberOfMachines+1);

  for i:=1 to NumberOfMachines do with Machineset[i] do
  begin
    Water1Cal := 1;
    Water2Cal := 1;
    EnergyCal := 1;
    TemperatureCalNew := 0;
  end;

  for i:=1 to NumberOfMachines do
  begin
    if i>1 then
    begin
      ts:=TTabSheet.Create(Self);
      ts.Parent:=PageControl1;
      ts.Name:='Machine'+InttoStr(i)+'Sheet';
      ts.Caption:='Machine '+InttoStr(i);
      ts.PageControl:=PageControl1;
    end
    else
    begin
      ts:=Machine1Sheet;
    end;
    ts.Tag:=0;

    (*
    with TFrame1.Create(ts) do
    begin
      Parent:=ts;
      Align:=alClient;
      Name:='Frame1_'+InttoStr(x);
    end;
    *)
  end;

  for i:=1 to NumberOfMachines do
  begin
    Machineset[i].Data:=TList<TMachineData>.Create();
    MachineBox[i]:=TFrameSimple.Create(tsOverview1);
    MachineBox[i].Number:=i;
    //MachineBox[i].Name:='NewFrame'+InttoStr(i);
    MachineBox[i].Parent:=tsOverview1;
    //MachineBox[i].DoubleBuffered:=True;
    //MachineBox[i].Align:=alClient;
    MachineBox[i].Enable:=false;
    MachineBox[i].GraphBase.OnClick := GraphBaseClick;
    //MachineBox[i].Panel1.OnClick := GraphBaseClick;
    MachineBox[i].TemperatureBase.OnClick := GraphBaseClick;
    MachineBox[i].EnergyBase.OnClick := GraphBaseClick;
    MachineBox[i].Water1Base.OnClick := GraphBaseClick;
    MachineBox[i].Water2Base.OnClick := GraphBaseClick;
  end;

  PageControl1.TabIndex:=2;
  VisibleMachine:=1;
  PageControl1Change(PageControl1);


  // Create HID manager
  DD:=TDataDevice.Create(Application.GetNamePath);
  //DD.OnDeviceChange:=UpdateDevice;
  DD.OnDeviceChange:=UpdateUSB;
  DD.OnDataReceived:=UpdateData;
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

procedure TPowerbankMainForm.MachineRemoveClickClick(Sender: TObject);
var
  localmachine:word;
  localtabmachine:word;
  x:word;
begin
  localmachine:=StrToIntDef(RightStr(TMenuItem(Sender).Caption,2),0);
  if localmachine>0 then
  begin
    //USBCounter.ChangeAddress(localmachine,0);
    for x:=0 to (PageControl1.PageCount-1) do
    begin
      if Pos('Machine',PageControl1.Pages[x].Caption)>0 then
      begin
        localtabmachine:=StrToIntDef(RightStr(PageControl1.Pages[x].Caption,2),0);
        if localtabmachine=localmachine then
        begin
          PageControl1.Pages[x].Tag:=0;
          PageControl1.Repaint;
          exit;
        end;
      end;
    end;
  end;
end;

procedure TPowerbankMainForm.PageControl1Change(Sender: TObject);
var
  z:integer;
  ChartTime,StartTime:TDateTime;
  PG:TPageControl;
begin
  PG:=TPageControl(Sender);

  if PG.OwnerDraw then PG.Invalidate;

  (*
  if Pos('Machine',PG.ActivePage.Caption)>0
     then VisibleMachine:=StrToIntDef(RightStr(PG.ActivePage.Caption,2),0)
     else VisibleMachine:=0;
  *)
  if (PG.TabIndex>1)then
   VisibleMachine:=(PG.TabIndex-1)
  else
    VisibleMachine:=0;

  if VisibleMachine>0 then
  begin
    MachineGroupBox.Parent:=PG.ActivePage;
    SetControls(Sender,Machineset[VisibleMachine].Running OR Machineset[VisibleMachine].Startup);

    //MachineChart.UndoZoom;
    if MachineChart.IsZoomed then MachineChart.ZoomFull(false);
    TLineSeries(MachineChart.Series[0]).Clear;
    TLineSeries(MachineChart.Series[1]).Clear;

    StartTime:=Machineset[VisibleMachine].Start_time;

    if Machineset[VisibleMachine].Data.Count>0 then
    begin
    for z:=0 to Machineset[VisibleMachine].Data.Count-1 do
    begin
      //if AbsoluteTime.Checked
      //   then ChartTime:=IncSecond(StartTime,round(Machineset[VisibleMachine].Data[z].Time))
      //   else ChartTime:=IncSecond(DateOf(Now),round(Machineset[VisibleMachine].Data[z].Time));
      ChartTime:=IncSecond(DateOf(Now),round(Machineset[VisibleMachine].Data[z].Time));

      TLineSeries(MachineChart.Series[0]).AddXY(ChartTime,Machineset[VisibleMachine].Data[z].Energy,'',clBlack);
      TLineSeries(MachineChart.Series[1]).AddXY(ChartTime,Machineset[VisibleMachine].Data[z].Water1,'',clAqua);
      TLineSeries(MachineChart.Series[2]).AddXY(ChartTime,Machineset[VisibleMachine].Data[z].Water2,'',clBlue);
      TLineSeries(MachineChart.Series[3]).AddXY(ChartTime,Machineset[VisibleMachine].Data[z].Temperature1,'',clRed);
    end;

    if ((NOT Machineset[VisibleMachine].Running) AND (NOT Machineset[VisibleMachine].Startup)) then with Machineset[VisibleMachine] do
    begin
      Energy:=Data.Last.Energy;
      Water1:=Data.Last.Water1;
      Water2:=Data.Last.Water2;
      Temperature1:=Data.Last.Temperature1;
      Temperature2:=Data.Last.Temperature2;
      Start_time:=StartTime;
      Running_time:=IncSecond(StartTime,round(Data.Last.Time));
    end;
    end;

    AutoFileNameCheckBox.Checked:=Machineset[VisibleMachine].AutoFileName;
    FilenameEdit.Text:=ExtractFileName(Machineset[VisibleMachine].Filename);
    Project_edit.Text:=Machineset[VisibleMachine].Project;
    Product_edit.Text:=Machineset[VisibleMachine].Product;
    Runname_edit.Text:=Machineset[VisibleMachine].Runname;
    Runnumber_edit.Text:=Machineset[VisibleMachine].Runnumber;
    MachineInfoMemo.Lines.CommaText:=Machineset[VisibleMachine].Info;
  end;
end;

procedure TPowerbankMainForm.PageControl1Changing(Sender: TObject;
  var AllowChange: Boolean);
begin
  SaveMachineData;
  //AllowChange := TemperatureEdit.ReadOnly
end;

procedure TPowerbankMainForm.PageControl1DragDrop(Sender, Source: TObject; X,
  Y: Integer);
var
  TabRect: TRect;
  j: Integer;
  OldBoard,NewBoard:integer;
  buttonselected:integer;
  TempTag:NativeInt;
begin
  //ShowMessage(InttoStr(PageControl1.ActivePage.PageIndex));
  if (Sender is TPageControl) then
  for j := 0 to PageControl1.PageCount - 1 do
  begin
    TabRect:=PageControl1.TabRect(j);
    if PtInRect(TabRect, Point(X, Y)) then
    begin
      if PageControl1.ActivePage.PageIndex <> j then
      begin
       //PageControl1.ActivePage.PageIndex := j;
       OldBoard:=StrToIntDef(RightStr(PageControl1.ActivePage.Caption,2),0);
       NewBoard:=StrToIntDef(RightStr(PageControl1.Pages[j].Caption,2),0);
       if OldBoard<>NewBoard then
       begin
         buttonSelected := MessageDlg('Confirm board change from '+InttoStr(OldBoard)+' to '+InttoStr(NewBoard),mtError, mbOKCancel, 0);
         if buttonSelected = mrOK then
         begin
           //USBCounter.ChangeAddressNew(OldBoard,NewBoard);
           //USBCounter.ChangeAddress(OldBoard,NewBoard);
           TempTag:=PageControl1.ActivePage.Tag;
           PageControl1.ActivePage.Tag:=PageControl1.Pages[j].Tag;
           PageControl1.Pages[j].Tag:=TempTag;
           PageControl1.Repaint;
         end;
       end;
      end;
      break;
    end;
  end;
end;

procedure TPowerbankMainForm.PageControl1DragOver(Sender, Source: TObject; X,
  Y: Integer; State: TDragState; var Accept: Boolean);
begin
  if (Sender is TPageControl) then Accept := True;
end;

procedure TPowerbankMainForm.PageControl1MouseDown(Sender: TObject;
  Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
var
   TabRect: TRect;
   j: Integer;
   NewBoard:integer;
   localpagecaption:string;
   AllowDrag:boolean;
begin
 if ssCtrl in Shift then
 begin
   AllowDrag:=true;
   for j:=1 to NumberOfMachines do
   begin
      if Machineset[j].Running then
      begin
        AllowDrag:=false;
        break;
      end
   end;
   if AllowDrag then PageControl1.BeginDrag(False);
 end
 else
 begin
   //if (Button = mbRight) and (Shift = []) then
   if (Button = mbRight) then
   begin
     NewBoard:=0;
     if (Sender is TPageControl) then
     for j := 0 to PageControl1.PageCount - 1 do
     begin
       TabRect:=PageControl1.TabRect(j);
       //ShowMessage('Counter '+InttoStr(j));
       if PtInRect(TabRect, Point(X, Y)) then
       begin
          localpagecaption:=PageControl1.Pages[j].Caption;
          if (Pos('Machine',localpagecaption)>0)
             then NewBoard:=StrToIntDef(RightStr(localpagecaption,2),0);
          if NewBoard>0 then
          begin
            PopupMenu1.Items[0].Caption:='Remove machine '+InttoStr(NewBoard);
            PopUpMenu1.Popup(Self.Left+PageControl1.Left+x,Self.Top+PageControl1.Top+y);
          end;
         exit;
       end;
     end;
   end;
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

    DD.SetMaxPDO(ActiveBoard,PDONumber);

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
              if aCol=3 then s:=
                FloattoStrF(MinimumVoltageIn50mV*0.05,ffFixed,8,2)+
                '-'+
                FloattoStrF(MaximumVoltageIn50mV*0.05,ffFixed,8,2)+
                ' Volt';
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

procedure TPowerbankMainForm.AutoFileNameCheckBoxClick(Sender: TObject);
begin
  FilenameEdit.Enabled:=(NOT AutoFileNameCheckBox.Checked);
  Machineset[VisibleMachine].AutoFileName:=AutoFileNameCheckBox.Checked;
end;

procedure TPowerbankMainForm.btnRotoPDGetPDOListClick(Sender: TObject);
begin
  DD.GetPDOList(ActiveBoard);
  //DD.SetEnergy(ActiveBoard,876456789);
end;

procedure TPowerbankMainForm.btnTestDischargeClick(Sender: TObject);
begin
  if TSpeedButton(Sender).Down then
  begin
    if (
       //(TypesBox.ItemIndex<>-1)
       //AND
       //(SamplesBox.ItemIndex<>-1)
       //AND
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
      DD.SetOutput(ActiveBoard,SystemActive);
      DD.SetCurrent(ActiveBoard,1,ActiveTestType.Current);
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

procedure TPowerbankMainForm.Button2Click(Sender: TObject);
begin
  //DD.SetMaxPDO(ActiveBoard,1);
  DD.SetPPSPDO(ActiveBoard,6,3000,8500);
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

procedure TPowerbankMainForm.FormDestroy(Sender: TObject);
var
  x:integer;
begin
  System.Finalize(MachineBox);

  for x:=1 to NumberOfMachines do
  begin
    Machineset[x].Data.Destroy;
  end;

  System.Finalize(Machineset);
end;

procedure TPowerbankMainForm.FormShow(Sender: TObject);
begin
  FOldOnIdle := Application.OnIdle; // Save current OnIdle.
  Application.OnIdle:=AfterShow;
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

         AllStop(nil);

         UpdateTimer.Enabled:=False;

         //DD.Enabled:=False;
         DD.OnDataReceived:=nil;
         DD.OnDeviceChange:=nil;
         DD.Destroy;
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

procedure TPowerbankMainForm.StartStopSpeedButtonClick(Sender: TObject);
var
  F                            : textfile;
  Oldfile,NewFile              : TFileStream;
  backupname,runtext           : string;
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
        if Sender=StartStopSpeedButton then
        begin
          SaveMachineData;
        end;

        //if AutoFileNameCheckBox.Checked then
        if Machineset[VisibleMachine].AutoFileName then
        begin
          backupname:=ApplicationDirectory+'Data';
          backupname:=backupname+'_MP'+InttoStr(VisibleMachine+BaseMachineNumber-1);
          if Length(Machineset[VisibleMachine].Project)>0 then backupname:=backupname+'_'+Machineset[VisibleMachine].Project;
          if Length(Machineset[VisibleMachine].Product)>0 then backupname:=backupname+'_'+Machineset[VisibleMachine].Product;
          runtext:=Machineset[VisibleMachine].Runname+'_'+Machineset[VisibleMachine].Runnumber;
          if Length(runtext)>1 then backupname:=backupname+'_'+runtext;
          backupname:=backupname+'_'+ FormatDateTime('dd-mm-yyyy', Now);
          backupname:=backupname+'_'+ FormatDateTime('HH', Now)+'h'+FormatDateTime('nn', Now);
          backupname:=backupname+'.dat';
          Machineset[VisibleMachine].Filename:=backupname;
          if Sender=StartStopSpeedButton then FilenameEdit.text:=ExtractFileName(backupname);
        end;

        if Sender=StartStopSpeedButton then
        begin
         //SaveMachineData;

          // Backup old file if needed !!!!
          if FileExists(Machineset[VisibleMachine].Filename) then
          begin
             if MessageDlg ('Are you REALLY SURE you want to overwrite the datafile ?'+
                         chr(13)+'(This is your last change to rescue your previous data!)',
                         mtConfirmation, [mbYes,mbNo],0)=idNo
             then
             begin
               TSpeedButton(Sender).Down:=False;
               Exit;
             end;

             backupname := ChangeFileExt(Machineset[VisibleMachine].Filename, '.bak');
             OldFile := TFileStream.Create(Machineset[VisibleMachine].Filename, fmOpenRead or fmShareDenyWrite);
             try
                {$WARN SYMBOL_PLATFORM OFF}
                NewFile := TFileStream.Create(backupname, fmCreate or fmShareDenyRead);
                {$WARN SYMBOL_PLATFORM ON}
                try
                  NewFile.CopyFrom(OldFile, OldFile.Size);
                finally
                  FreeAndNil(NewFile);
                end;
             finally
               FreeAndNil(OldFile);
             end;
          end;
        end;

        try
          AssignFile(F,Machineset[VisibleMachine].Filename);
          if (NOT IsOpen(F)) then
          try
            Rewrite(F);
            if IOResult=0 then
            begin
              Writeln(F, 'Project: '+Machineset[VisibleMachine].Project);
              Writeln(F, 'Product: '+Machineset[VisibleMachine].Product);
              Writeln(F, 'Run: '+Machineset[VisibleMachine].Runname+'_'+Machineset[VisibleMachine].Runnumber);
              Writeln(F, 'Info:');
              Writeln(F, Machineset[VisibleMachine].Info);
              Writeln(F, 'Date: '+DateTimeToStr(Machineset[VisibleMachine].Start_time,LocalFormatSettings));
              Writeln(F, 'Location: '+InttoStr(VisibleMachine+BaseMachineNumber-1));
              Writeln(F, '**********************************************');
              Write(F,'Time [sec]'+LocalFormatSettings.ListSeparator);
              Write(F,'Energy [kWh]'+LocalFormatSettings.ListSeparator);
              Write(F,'Water1 [l]'+LocalFormatSettings.ListSeparator);
              Write(F,'Water2 [l]'+LocalFormatSettings.ListSeparator);
              Write(F,'Temperature1 [°C]'+LocalFormatSettings.ListSeparator);
              Writeln(F,'Temperature2 [°C]');
            end;
          finally
            CloseFile(F);
          end;
        except
          CloseFile(F);
        end;

        with (MachineBox[VisibleMachine]) do
        begin
          Enable:=true;
          //GraphBase.UndoZoom;
          if GraphBase.IsZoomed then GraphBase.ZoomFull(false);
          TLineSeries(GraphBase.Series[0]).Clear;
          TLineSeries(GraphBase.Series[1]).Clear;
          TLineSeries(GraphBase.Series[2]).Clear;
          TLineSeries(GraphBase.Series[3]).Clear;
          runtext:=Machineset[VisibleMachine].Runname+' '+Machineset[VisibleMachine].Runnumber;
          if Length(runtext)>1
             then Title:=Machineset[VisibleMachine].Product+' '+runtext
             else Title:=Machineset[VisibleMachine].Product;
        end;

        if Sender=StartStopSpeedButton then
        begin
          //MachineChart.UndoZoom;
          if MachineChart.IsZoomed then MachineChart.ZoomFull(false);
          TLineSeries(MachineChart.Series[0]).Clear;
          TLineSeries(MachineChart.Series[1]).Clear;
          //TLineSeries(MachineChart.Series[2]).Clear;
          //TLineSeries(MachineChart.Series[3]).Clear;

          SetControls(Sender,true);

          ProgressBar1.Show;
        end;

        with Machineset[VisibleMachine] do
        begin
          Data.Clear;
          Temperature1:=0;
          Temperature2:=0;
          MaxTemp:=0;
          Water1:=0;
          Water2:=0;
          Energy:=0;
          Start_time:=Now;
          Running_time:=Start_time;
          Running:=False;
          Startup:=True;
          AutoStartReady:=(NOT AutoStart);

          (*
          reg:=TRegistry.Create;
          with reg do
          begin
            try
              RootKey := HKEY_CURRENT_USER;
              if OpenKey(REGSTOREPOSITION+'Board'+InttoStr(VisibleMachine)+'\', True) then
              begin
                WriteString('Project',Project);
                WriteString('Product',Product);
                WriteString('Runname',Runname);
                WriteString('Runnumber',Runnumber);
                WriteString('Info',Info);
                CloseKey;
              end;
            finally
              reg.Free;
            end;
          end;
          *)
        end;



        SetEnable(Sender,false);

        Power:=0;
        Energy:=0;
        Capacity:=0;

        SetChartAxis(Sender);

        CreateDataFile(Sender);

        TestInfoMemo.Lines.Append('Filename: '+BatteryDataFile);

        SystemActive:=True;

        DD.SetOutput(ActiveBoard,SystemActive);
        DD.SetCurrent(ActiveBoard,1,ActiveTestType.Current);

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
  qcalc:QWord;
begin
  MachineChartLineSeries1.Add(Voltage);
  MachineChartLineSeries2.Add(Current);

  if SystemActive then
  begin
    // only perform the below if coming from the timer !
    if Assigned(Sender) then
    begin
      Elapsed:=MilliSecondsBetween(NowUTC,LastTime);
      LastTime:=NowUTC;

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
    SetActiveTestType(TestTypes[aCombo.ItemIndex]);
  end
  else
  begin
    SetActiveTestType(Default(TTestType));
  end;
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
    if (Sender<>StartStopSpeedButton) then StartStopSpeedButton.Enabled:=value;

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

procedure TPowerbankMainForm.SetCapacity(value:double);
begin
  if (FCapacity<>value) then
  begin
    FCapacity:=value;
    NewAHDisplay.Value:=value;
  end;
end;

function  TPowerbankMainForm.GetCapacity:double;
begin
  result:=FCapacity;
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

procedure TPowerbankMainForm.SetChartAxis(Sender:TObject);
begin
  MachineChartLineSeries1.Clear;
  MachineChartLineSeries2.Clear;
  MachineChartLineSeries3.Clear;
  {$ifdef FPC}
  //MachineChartLineSeries3.Active:=(Sender=btnCurve);
  {$else}
  //MachineChartLineSeries3.Visible:=(Sender=btnCurve);
  {$endif}
  {$ifndef FPC}
  MachineChart.UndoZoom;
  {$endif}
  {$ifdef FPC}

  exit;

  MachineChart.AxisList.GetAxisByAlign(calRight).Range.Min:=0;
  MachineChart.AxisList.GetAxisByAlign(calRight).Range.UseMin:=True;
  MachineChart.AxisList.GetAxisByAlign(calRight).Range.Max:=ActiveTestType.Current*1.2/1000;
  MachineChart.AxisList.GetAxisByAlign(calRight).Range.UseMax:=True;
  MachineChart.AxisList.GetAxisByAlign(calRight).Intervals.Count:=10;
  MachineChart.AxisList.GetAxisByAlign(calLeft).Range.Min:=0;
  MachineChart.AxisList.GetAxisByAlign(calLeft).Range.UseMin:=True;
  MachineChart.AxisList.GetAxisByAlign(calLeft).Range.Max:=ActiveTestType.Voltage*1.2/1000;
  MachineChart.AxisList.GetAxisByAlign(calLeft).Range.UseMax:=True;
  MachineChart.AxisList.GetAxisByAlign(calLeft).Intervals.Count:=10;
  {$else}
  MachineChart.RightAxis.SetMinMax(0,ActiveTestType.Current*1.2/1000);
  MachineChart.RightAxis.Increment:=0.1;
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
    if Sender=StartStopSpeedButton then BatteryDataFile:=BatteryDataFile+'_Normal';
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

  StartTime:=0;

  SystemActive:=False;

  if Assigned(Sender) then
  begin
    DD.SetOutput(ActiveBoard,false);
    DD.SetOff(ActiveBoard,1);
  end;

  SetEnable(Sender,true);

  StartStopSpeedButton.Down:=false;
  StartStopSpeedButton.Caption:='Start';
  StartStopSpeedButton.Font.Color:=clLime;

  btnTestDischarge.Down:=false;

  ProgressBar1.Position:=0;
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

         write(F,'B_code',LocalFormatSettings.ListSeparator);
         write(F,'Sample#',LocalFormatSettings.ListSeparator);
         write(F,'Battery mode',LocalFormatSettings.ListSeparator); // = CD
         write(F,'Battery mode value',LocalFormatSettings.ListSeparator);
         write(F,'Trigger moment UTC',LocalFormatSettings.ListSeparator);
         write(F,'Human time',LocalFormatSettings.ListSeparator);
         write(F,'Time(sec)',LocalFormatSettings.ListSeparator);
         write(F,'Voltage(V)',LocalFormatSettings.ListSeparator);
         write(F,'Current(A)',LocalFormatSettings.ListSeparator);
         write(F,'Capacity(Ah)',LocalFormatSettings.ListSeparator);
         write(F,'Energy(Wh)',LocalFormatSettings.ListSeparator);
         writeln(F);
       end;

    if (ProgressBar1.Position=ProgressBar1.Max) then
      ProgressBar1.Position:=1
    else
      ProgressBar1.StepIt;

    write(F,PC,LocalFormatSettings.ListSeparator);
    write(F,SamplesBox.Text,LocalFormatSettings.ListSeparator);
    write(F,'CD',LocalFormatSettings.ListSeparator);
    write(F,ActiveTestType.Current,LocalFormatSettings.ListSeparator);
    write(F,FloattoStr(NowUTC,LocalFormatSettings),LocalFormatSettings.ListSeparator);
    write(F,FormatDateTime('dd-mm-yyyy hh:nn:ss',NowUTC),LocalFormatSettings.ListSeparator);
    write(F,FloattoStrF((Elapsed/1000),ffFixed,6,1,LocalFormatSettings),LocalFormatSettings.ListSeparator);
    write(F,FloattoStrF(Voltage,ffFixed,10,3,LocalFormatSettings),LocalFormatSettings.ListSeparator);
    write(F,FloattoStrF(Current,ffFixed,10,3,LocalFormatSettings),LocalFormatSettings.ListSeparator);
    write(F,FloattoStrF(Capacity,ffFixed,10,3,LocalFormatSettings),LocalFormatSettings.ListSeparator);
    write(F,FloattoStrF(Energy,ffFixed,10,3,LocalFormatSettings),LocalFormatSettings.ListSeparator);

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

procedure TPowerbankMainForm.SetActiveTestType(const TT:TTestType);
begin
  ActiveTestType.Name:=TT.Name;
  ActiveTestType.Voltage:=TT.Voltage;
  ActiveTestType.Current:=TT.Current;
  CurrentEdit.Text:=InttoStr(ActiveTestType.Current);
  VoltageEdit.Text:=InttoStr(ActiveTestType.Voltage);
  CurrentEdit.ReadOnly:=((Pos('Power',ActiveTestType.Name)=0) AND (Pos('Variable',ActiveTestType.Name)=0));
  VoltageEdit.ReadOnly:=CurrentEdit.ReadOnly;
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
  cmd                  : TCommands;
  boardnumber          : byte;
  length               : byte;
  counter              : integer;
  PDOCount,PDOIndex    : byte;
  RawPDO               : PDO_DATA_T;
  PDO                  : TAP33772S_PDO;
  wd                   : TWordData;
  dwd                  : TDWordData;
  qwd                  : TQWordData;
  bd                   : TByteData;
  aSourcePDO           : ^TSOURCEPDO;
begin
  cmd:=TCommands(PByteArray(data)^[COMMANDPOSITION]);
  boardnumber:=PByteArray(data)^[INDEXPOSITION];
  length:=PByteArray(data)^[LENGTHPOSITION];

  USBDebugLog.Lines.Append('Received data from board #'+InttoStr(boardnumber));

  case cmd of
    TCommands.CMD_get_data:
    begin
      Counter:=DATASTART;

      //RotoPD Voltage
      wd.Bytes[0]:=PByteArray(data)^[counter];
      wd.Bytes[1]:=PByteArray(data)^[counter+1];
      Voltage:=(wd.Raw/1000);

      Inc(counter,2);

      //RotoPD Current
      wd.Bytes[0]:=PByteArray(data)^[counter];
      wd.Bytes[1]:=PByteArray(data)^[counter+1];
      Current:=(wd.Raw/1000);

      Inc(counter,2);

      //RotoPD Power
      dwd.Bytes[0]:=PByteArray(data)^[counter];
      dwd.Bytes[1]:=PByteArray(data)^[counter+1];
      dwd.Bytes[2]:=PByteArray(data)^[counter+2];
      dwd.Bytes[3]:=PByteArray(data)^[counter+3];
      Power:=(dwd.Raw/1000);

      Inc(counter,4);

      //RotoPD Temperature
      wd.Bytes[0]:=PByteArray(data)^[counter];
      wd.Bytes[1]:=PByteArray(data)^[counter+1];
      Temperature:=(wd.Raw/10);

      Inc(counter,2);

      if (length>13) then
      begin
        //IT2704 Voltage
        wd.Bytes[0]:=PByteArray(data)^[counter];
        wd.Bytes[1]:=PByteArray(data)^[counter+1];
        Voltage:=(wd.Raw/1000);

        Inc(counter,2);

        //IT2704 Current
        wd.Bytes[0]:=PByteArray(data)^[counter];
        wd.Bytes[1]:=PByteArray(data)^[counter+1];
        Current:=(wd.Raw/1000);

        Inc(counter,2);

        //IT2704 Power
        dwd.Bytes[0]:=PByteArray(data)^[counter];
        dwd.Bytes[1]:=PByteArray(data)^[counter+1];
        dwd.Bytes[2]:=PByteArray(data)^[counter+2];
        dwd.Bytes[3]:=PByteArray(data)^[counter+3];
        Power:=(dwd.Raw/1000);

        Inc(counter,4);

        //IT2704 Capacity
        dwd.Bytes[0]:=PByteArray(data)^[counter];
        dwd.Bytes[1]:=PByteArray(data)^[counter+1];
        dwd.Bytes[2]:=PByteArray(data)^[counter+2];
        dwd.Bytes[3]:=PByteArray(data)^[counter+3];
        Capacity:=(dwd.Raw/1000);

        Inc(counter,4);

        //IT2704 Energy
        dwd.Bytes[0]:=PByteArray(data)^[counter];
        dwd.Bytes[1]:=PByteArray(data)^[counter+1];
        dwd.Bytes[2]:=PByteArray(data)^[counter+2];
        dwd.Bytes[3]:=PByteArray(data)^[counter+3];
        Energy:=(dwd.Raw/1000);

        Inc(counter,4);
      end;

    end;
    TCommands.CMD_get_PDOList:
    begin
      Counter:=DATASTART;
      PDOCount := PByteArray(data)^[Counter];
      Inc(Counter);

      while (PDOCount>0) do
      begin
        Dec(PDOCount);
        PDOIndex := PByteArray(data)^[Counter];
        Inc(Counter);
        RawPDO.bytes.byte0 := PByteArray(data)^[Counter];
        Inc(Counter);
        RawPDO.bytes.byte1 := PByteArray(data)^[Counter];
        Inc(Counter);
        DecodePDONew(PDOIndex,RawPDO,PDO);

        if (PDOIndex=1) then
        begin
          TestTypes[(Low(TestTypes)+1)].Current:=PDO.maxCurrent_mA;
          TestTypes[(Low(TestTypes)+1)].Voltage:=PDO.maxVoltage_mV;
          if (ActiveTestType.Name=TestTypes[(Low(TestTypes)+1)].Name) then
          begin
            SetActiveTestType(TestTypes[(Low(TestTypes)+1)]);
          end;
        end;

        if (PDO.isEPR) then
          aSourcePDO := @DUT.SourceEPRPDOs[PDO.index-8]
        else
          aSourcePDO := @DUT.SourcePDOs[PDO.index-1];

        if (PDO.ptype=PDO_TYPE_FIXED) then
        begin
          with aSourcePDO^.FixedSupplyPdo do
          begin
            MaximumCurrentIn10mA     := (PDO.maxCurrent_mA DIV 10);
            VoltageIn50mV            := (PDO.maxVoltage_mV DIV 50);
            PeakCurrent              := 0;
            EPRModeCapable           := Ord(PDO.isEPR);
            UnchunkedMessagesSupport := 0;
            DataRoleSwap             := 0;
            UsbCommunicationCapable  := 0;
            UnconstrainedPower       := 0;
            UsbSuspendSupported      := 0;
            DualRolePower            := 0;
            bd.Raw := 0;
            FixedSupply              := bd.Raw; // 0x00
          end;
        end
        else
        if (PDO.ptype=PDO_TYPE_PPS) then
        begin
          with aSourcePDO^.VariableSupplyNonBatteryPdo do
          begin
            MaximumCurrentIn10mA      := (PDO.maxCurrent_mA DIV 10);
            MinimumVoltageIn50mV      := (PDO.minVoltage_mV DIV 50);
            MaximumVoltageIn50mV      := (PDO.maxVoltage_mV DIV 50);
            bd.Raw := 0;
            bd.Bits[1] := 1;
            VariableSupportNonBattery := bd.Raw; //%10;  // 0x10
          end;
        end
        else
        if (PDO.ptype=PDO_TYPE_AVS) then
        begin
          with aSourcePDO^.EPRAVSPDO do
          begin
            //PDPInW                       : T8BITS;
            MinimumVoltageIn100mV        := (PDO.minVoltage_mV DIV 100);
            MaximumVoltageIn100mV        := (PDO.maxVoltage_mV DIV 100);
            //PeakCurrent                  := (PDO.maxCurrent_mA DIV 10);
            bd.Raw := 0;
            bd.Bits[0] := 1;
            AugmentedPowerDataObjectType := bd.Raw; //%01;  // 0x01=EPRAVS Adjustable Voltage Supply
            bd.Raw := 0;
            bd.Bits[0] := 1;
            bd.Bits[1] := 1;
            AugmentedPowerDataObject     := bd.Raw; //%11;  // 0x11
          end;
        end;
      end;

      SourcePDODrawGrid.Invalidate;
      SourceEPRPDODrawGrid.Invalidate;
    end;

    TCommands.CMD_set_MAXPDO,TCommands.CMD_set_FIXEDPDO,TCommands.CMD_set_PPSPDO,TCommands.CMD_set_AVSPDO:
    begin
      PDOIndex := PByteArray(data)^[DATASTART];

      wd.Bytes[0]:=PByteArray(data)^[DATASTART+1];
      wd.Bytes[1]:=PByteArray(data)^[DATASTART+2];
      USBDebugLog.Lines.Append('Req: '+InttoStr(wd.Raw)+'mA');

      wd.Bytes[0]:=PByteArray(data)^[DATASTART+3];
      wd.Bytes[1]:=PByteArray(data)^[DATASTART+4];
      USBDebugLog.Lines.Append('Req: '+InttoStr(wd.Raw)+'mV');

      RawPDO.bytes.byte0 := PByteArray(data)^[DATASTART+5];
      RawPDO.bytes.byte1 := PByteArray(data)^[DATASTART+6];
      DecodePDONew(PDOIndex,RawPDO,PDO);

      USBDebugLog.Lines.Append('AP33772S PDO index '+InttoStr(PDO.index));
      USBDebugLog.Lines.Append('AP33772S '+InttoStr(PDO.maxVoltage_mV)+'mV');
      USBDebugLog.Lines.Append('AP33772S '+InttoStr(PDO.maxCurrent_mA)+'mA');
      if PDO.isEPR then USBDebugLog.Lines.Append('AP33772S '+'(EPR)');

      DUT.ActiveSRCPDO:=PDOIndex;

      DUT.RDOPosition:=PDOIndex;

      TestTypes[(Low(TestTypes)+1)].Current:=PDO.maxCurrent_mA;
      TestTypes[(Low(TestTypes)+1)].Voltage:=PDO.maxVoltage_mV;

      if (ActiveTestType.Name=TestTypes[(Low(TestTypes)+1)].Name) then
      begin
        SetActiveTestType(TestTypes[(Low(TestTypes)+1)]);
      end;

      SourcePDODrawGrid.Invalidate;
      SourceEPRPDODrawGrid.Invalidate;
    end;

  else
    begin
      USBDebugLog.Lines.Append('Got other data !!!!');
    end;
  end;
end;

procedure TPowerbankMainForm.UpdateUSB(Sender: TObject;datacarrier:integer);
var
  Info:string;
  x:integer;
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


  //MachineInfoMemo.Lines.Append('Board:'+InttoStr(datacarrier));
  if datacarrier>0 then
  begin
    Info:='Data acquisitionboard '+InttoStr(datacarrier)+' added';
    for x:=0 to (PageControl1.PageCount-1) do
    begin
      if Pos('Machine',PageControl1.Pages[x].Caption)>0 then
      begin
        if StrToIntDef(RightStr(PageControl1.Pages[x].Caption,2),-1)=datacarrier then
        begin
          PageControl1.Pages[x].Tag:=1;
          break;
        end;
      end;
    end;
  end;
  if datacarrier<0 then
  begin
    Info:='Data acquisitionboard '+InttoStr(-1*datacarrier)+' removed';
    for x:=0 to (PageControl1.PageCount-1) do
    begin
      if Pos('Machine',PageControl1.Pages[x].Caption)>0 then
      begin
        if StrToIntDef(RightStr(PageControl1.Pages[x].Caption,2),-1)=abs(datacarrier) then
        begin
          PageControl1.Pages[x].Tag:=0;
          break;
        end;
      end;
    end;
  end;

  if datacarrier=0 then Info:='Unknown data acquisitionboard error';

  PageControl1.Repaint;

  //Sleep(250);
  //Application.ProcessMessages;
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

procedure TPowerbankMainForm.GraphBaseClick(Sender: TObject);
var
  x:integer;
  MachineAsked:integer;
begin
  MachineAsked:=0;
  try
    if (Sender is TComponent) then MachineAsked:=(Sender as TComponent).Tag;
    for x:=0 to (PageControl1.PageCount-1) do
    begin
      if (Pos('Machine',PageControl1.Pages[x].Caption) <> 1) then continue;
      if StrToIntDef(RightStr(PageControl1.Pages[x].Caption,2),-1)=MachineAsked then
      begin
        PageControl1.TabIndex:=x;
        //PageControl1.ActivePage:=PageControl1.Pages[x];
        PageControl1Change(PageControl1);
        break;
      end;
    end;
  except
    on E: EInvalidOperation do begin end;
  end;
end;

procedure TPowerbankMainForm.SaveMachineData;
begin
  if (VisibleMachine>0) then
  begin
    if ((NOT Machineset[VisibleMachine].Running) AND (NOT Machineset[VisibleMachine].Startup)) then with Machineset[VisibleMachine] do
    begin
      Project:=Project_edit.Text;
      Product:=Product_edit.Text;
      Runname:=Runname_edit.Text;
      Runnumber:=Runnumber_edit.Text;
      Info:=MachineInfoMemo.Lines.CommaText;
      AutoFileName:=AutoFileNameCheckBox.Checked;
    end;
  end;
end;

procedure TPowerbankMainForm.SetControls(Sender:TObject;localsetting:boolean);
begin
  if (Sender<>StartStopSpeedButton) then StartStopSpeedButton.Down:=localsetting;
  MachineInfoMemo.Enabled:=NOT localsetting;
  FilenameEdit.Enabled:=NOT localsetting;
  Project_edit.Enabled:=NOT localsetting;
  Product_edit.Enabled:=NOT localsetting;
  Runname_edit.Enabled:=NOT localsetting;
  Runnumber_edit.Enabled:=NOT localsetting;
  AutoFileNameCheckBox.Enabled:=NOT localsetting;
  ProgressBar1.Visible:=localsetting;
  //btnClear.Enabled:=NOT localsetting;

  if localsetting then
  begin
    //if (Sender<>StartStopSpeedButton) then
    begin
      StartStopSpeedButton.Caption:='Stop';
      StartStopSpeedButton.Font.Color:=clRed;
    end;
  end
  else
  begin
    //if (Sender<>StartStopSpeedButton) then
    begin
      StartStopSpeedButton.Caption:='Start';
      StartStopSpeedButton.Font.Color:=clLime;
    end;
    FilenameEdit.Enabled:=(NOT Machineset[VisibleMachine].AutoFileName);
  end;
end;

{$ifdef MSWindows}
procedure TPageControl.CNDrawItem(var Message: TLMDrawItems);
var
  FontHandle       : HFONT;
  TargetColor      : COLORREF;
  FontObject       : TLogFont;
  BrushHandle      : HBRUSH;
begin
  with Message.DrawItemStruct^ do
  begin
    GetObject(Font.Handle, SizeOf(FontObject), @FontObject);

    if itemID = DWORD(-1) then
    begin
      // ??
      TargetColor:=clBlack;
    end;

    if (Page[itemID].Tag=1) then
    begin
      if itemID = TabIndex then
      //if (Page[itemID].IsControlVisible) then
      begin
        TargetColor:=clLime;
      end
      else
      begin
        TargetColor:=clRed;
      end;
    end
    else
    begin
      if (itemID>1) then
        TargetColor := TColor($202020)
      else
        TargetColor := clBlue;
    end;


    BrushHandle := CreateSolidBrush(TargetColor);
    FillRect(_hDC, rcItem, BrushHandle);

    FontObject.lfWeight := FW_BOLD;
    FontHandle := CreateFontIndirect(FontObject);
    try
      SelectObject(_hDC, FontHandle);
      SetTextColor(_hDC, ColorToRGB(clWhite));
      SetBkMode(_hDC, TRANSPARENT);
      DrawTextEx(_hDC, PChar(Page[itemID].Caption), -1, rcItem, DT_CENTER or
        DT_VCENTER or DT_SINGLELINE, nil);
    finally
      DeleteObject(FontHandle);
    end;
  end;
  Message.Result := 1;
end;
{$endif}

end.

