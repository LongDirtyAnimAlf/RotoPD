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
  processframe_simple,
  fptimernew,
  dsLeds,
  dlLEDCtrl,
  usbcpd,
  pddevice,
  ap33772s,
  usbrotopd;

type
  TMachineData = record
    Voltage : single;
    Current : single;
    Power : single;
    Capacity : single;
    Energy : single;
    //Time:single;
    Elapsed:uint64;  // ms
  end;
  TMachineDataList = TList<TMachineData>;
  //TDataList = specialize TList<TMachineData>;
  //TDataList = specialize TFPGList<TMachineData>;

  TTestType = record
    Name:string;
    Current:word;
    Voltage:word;
  end;

  TMachineSet = record
    BoardAvailable         : boolean;
    Project                : string;
    Product                : string;
    Runname                : string;
    Runnumber              : string;
    Info                   : string;
    Filename               : string;
    MachineNumber          : byte;
    Active                 : boolean;
    Testing                : boolean;
    Last_time              : TDateTime;
    //AutoFileName           : boolean;
    ActiveTestType         : TTestType;
    LatestMeasurementData  : TMachineData;
    RunData                : TMachineDataList;
    DUT                    : TUSBPDDevice;
  end;
  PMachineSet = ^TMachineSet;

  TUSBPDController = class(TUSBPD)
  end;

  { TPowerbankMainForm }

  TPowerbankMainForm = class(TForm)
    AutoFileNameCheckBox: TCheckBox;
    btnRotoPDGetPDOList: TButton;
    btnTestDischarge: TSpeedButton;
    Button2: TButton;
    chkCurrentLimit: TCheckBox;
    chkVoltageLimit: TCheckBox;
    CurrentEdit: TEdit;
    DisplaysPanel: TPanel;
    Edit1: TEdit;
    FileLabel: TLabel;
    FilenameEdit: TEdit;
    GroupBox1: TGroupBox;
    grpEPRPDOs: TGroupBox;
    grpPDOs: TGroupBox;
    grpRotoPDControl: TGroupBox;
    grpTesting: TGroupBox;
    grpVAData: TGroupBox;
    ImageList1: TImageList;
    InfoLabel: TLabel;
    lblCurrent: TLabel;
    lblDischargeSettings: TLabel;
    lblVoltage: TLabel;
    MachineChart: TChart;
    MachineChartLineSeries1: TLineSeries;
    MachineChartLineSeries2: TLineSeries;
    MachineChartLineSeries3: TLineSeries;
    MachineGroupBox: TPanel;
    MachineInfoMemo: TMemo;
    MachineRemoveClick: TMenuItem;
    MemoUnhandled: TMemo;
    MT1: TAutoScaleAxisTransform;
    MyTransform1: TChartAxisTransformations;
    MyTransform2: TChartAxisTransformations;
    MyTransform2AutoScaleAxisTransform1: TAutoScaleAxisTransform;
    PageControl1: TPageControl;
    Panel1: TPanel;
    Panel2: TPanel;
    Panel3: TPanel;
    Panel4: TPanel;
    PopupMenu1: TPopupMenu;
    ChannelSheet: TTabSheet;
    ProductLabel: TLabel;
    Product_edit: TEdit;
    ProgressBar1: TProgressBar;
    ProjectLabel: TLabel;
    Project_edit: TEdit;
    RunLabel: TLabel;
    Runname_edit: TEdit;
    RunNumberLabel: TLabel;
    Runnumber_edit: TEdit;
    SamplesBox: TComboBox;
    SinkEPRPDODrawGrid: TDrawGrid;
    SinkPDODrawGrid: TDrawGrid;
    OverviewDrawGrid: TDrawGrid;
    SourceEPRPDODrawGrid: TDrawGrid;
    SourcePDODrawGrid: TDrawGrid;
    StartStopSpeedButton: TSpeedButton;
    StaticText1: TStaticText;
    TabControl1: TTabControl;
    TestInfoMemo: TMemo;
    TestsBox: TComboBox;
    tsOverview1: TTabSheet;
    tsOverview2: TTabSheet;
    DataStoreTimer: TTimer;
    ProgressStepperTimer: TTimer;
    TypesBox: TComboBox;
    UpdateTimer: TTimer;
    USBDebugLog: TMemo;
    VoltageEdit: TEdit;
    procedure AutoFileNameCheckBoxClick(Sender: TObject);
    procedure btnCleanLogsClick({%H-}Sender: TObject);
    procedure btnRotoPDGetPDOListClick(Sender: TObject);
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
    procedure OverviewDrawGridDrawCell(Sender: TObject; aCol, aRow: Integer;
      aRect: TRect; aState: TGridDrawState);
    procedure PDODrawGridButtonClick(Sender: TObject; aCol, aRow: Integer
      );
    procedure PDODrawGridDrawCell(Sender: TObject; aCol, aRow: Integer;
      aRect: TRect; aState: TGridDrawState);
    procedure EngageClick(Sender: TObject);
    procedure DataStoreTimerTimer(Sender: TObject);
    procedure EditChangeEmptyCheck(Sender: TObject);
    procedure SamplesBoxSelect(Sender: TObject);
    procedure TabControl1Change(Sender: TObject);
    procedure TabControl1Changing(Sender: TObject; var AllowChange: Boolean);
    procedure TestsBoxChange(Sender: TObject);
    procedure ProgressStepperTimerTimer({%H-}Sender: TObject);
    procedure TestsBoxDblClick(Sender: TObject);
    procedure TestsBoxSelect(Sender: TObject);
    procedure tsOverview2Resize(Sender: TObject);
    procedure TypesBoxDblClick(Sender: TObject);
    procedure TypesBoxSelect(Sender: TObject);
    procedure UpdateTimerTimer({%H-}Sender: TObject);

    procedure FormCreate({%H-}Sender: TObject);
    procedure FormClose(Sender: TObject; var CloseAction: TCloseAction);
  strict private
    MachineBox          : array[1..MAXBOARDS] of TFrameSimple;
    MachineSet          : array[1..MAXBOARDS] of TMachineSet;

    DD                  : TDataDevice;

    PCurrentMachineSet  : PMachineSet;
    PCurrentMachineBox  : PFrameSimple;

    TestTypes           : array of TTestType;

    FOldOnIdle          : TIdleEvent;

    LocalFormatSettings : TFormatSettings;

    RealVoltageDisplay    : TdsSevenSegmentMultiDisplay;
    RealCurrentDisplay    : TdsSevenSegmentMultiDisplay;
    NewPowerDisplay       : TLEDDisplay;
    NewEnergyDisplay      : TLEDDisplay;
    NewAHDisplay          : TLEDDisplay;
    NewTemperatureDisplay : TLEDDisplay;

    NumRate             : integer;

    ApplicationDirectory:string;

    procedure GraphBaseClick(Sender: TObject);
    procedure SaveMachineData;

    procedure SetControls(Sender: TObject);

    procedure AfterShow(Sender: TObject; var {%H-}Done: Boolean);
    procedure Startup;

    procedure UpdateUSB(Sender: TObject;datacarrier:integer);
    procedure UpdateData(Sender: TObject; {%H-}ReportID: Byte; const Data: Pointer; {%H-}Size: Word);

    procedure SetChartAxis({%H-}Sender:TObject);
    procedure CreateDataFile(Sender: TObject);
    procedure AllStop(Sender: TObject);
    procedure SaveBatteryData(LocalPMS: PMachineSet);

    function  CorrectVoltage(value:double):double;
    function  CorrectCurrent(value:double):double;

    procedure SetActiveTestType(const TT:TTestType);

    procedure SetVoltage(value:double);
    procedure SetCurrent(value:double);
    procedure SetCapacity(value:double);
    procedure SetEnergy(value:double);
    procedure SetPower(value:double);
    procedure SetTemperature(value:double);
  public
    property GUIVoltage             : double write SetVoltage;
    property GUICurrent             : double write SetCurrent;
    property GUICapacity            : double write SetCapacity;
    property GUIEnergy              : double write SetEnergy;
    property GUIPower               : double write SetPower;
    property GUITemperature         : double write SetTemperature;
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
  ENERGYCOLOR                  = clYellow;
  POWERCOLOR                   = clFuchsia;

procedure ReplaceImageFromResource(AImageList: TImageList; AIndex: Integer; const AResName: string);
var
  Stream: TResourceStream;
  Png: TPortableNetworkGraphic;
begin
  if (AIndex < 0) or (AIndex >= AImageList.Count) then Exit;

  Stream := TResourceStream.Create(HInstance, AResName, RT_RCDATA);
  try
    Png := TPortableNetworkGraphic.Create;
    try
      Png.LoadFromStream(Stream);
      Png.Transparent := True;
      AImageList.Replace(AIndex, Png, nil);   // replaces the image
    finally
      Png.Free;
    end;
  finally
    Stream.Free;
  end;
end;

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
begin
  ApplicationDirectory:=ExtractFilePath(Application.ExeName);

  LocalFormatSettings:=DefaultFormatSettings;

  LocalFormatSettings.ShortDateFormat:='dd-mm-yyyy';
  LocalFormatSettings.LongTimeFormat:='hh:nn:ss';
  LocalFormatSettings.DecimalSeparator:=',';
  LocalFormatSettings.ListSeparator:=';';

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
    ColorLED:=POWERCOLOR;
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
    ColorLED:=ENERGYCOLOR;
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

  Runnumber_edit.Text:='1';

  Ini := TIniFile.Create( ChangeFileExt( Application.ExeName, '.ini' ) );
  try
    NumRate           := Ini.ReadInteger('General', 'NumRate', 10);
    Project_edit.Text := Ini.ReadString('Project', 'ID', '260000');
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

  for i:=Low(MachineSet) to High(MachineSet) do
  begin
     // Create Machine datasets
    MachineSet[i].RunData:=TList<TMachineData>.Create();
    MachineSet[i].DUT:=TUSBPDDevice.Create;
    MachineSet[i].Project:=Project_edit.Text;
    MachineSet[i].Runnumber:=Runnumber_edit.Text;
  end;

  for i:=Low(MachineBox) to High(MachineBox) do
  begin
    // Create overview frames
    MachineBox[i]:=TFrameSimple.Create(tsOverview1);
    MachineBox[i].Number:=i;
    //MachineBox[i].Name:='NewFrame'+InttoStr(i);
    MachineBox[i].Parent:=tsOverview1;
    //MachineBox[i].DoubleBuffered:=True;
    //MachineBox[i].Align:=alClient;
    MachineBox[i].Enable:=false;
    MachineBox[i].SetClick(GraphBaseClick);
  end;

  // Create HID manager
  DD:=TDataDevice.Create(Application.GetNamePath);
  DD.DataSource.MaxBoards:=8;
  //DD.OnDeviceChange:=UpdateDevice;
  DD.OnDeviceChange:=UpdateUSB;
  DD.OnDataReceived:=UpdateData;
  // Done in startup
  //DD.Enabled:=True;

  PCurrentMachineSet:=@MachineSet;
  PCurrentMachineBox:=@MachineBox;

  PageControl1.TabIndex:=2;

  DataStoreTimer.Interval:=NumRate*1000;
  DataStoreTimer.Enabled:=True;
end;

procedure TPowerbankMainForm.FormDestroy(Sender: TObject);
var
  x:integer;
begin
  for x:=Low(MachineSet) to High(MachineSet) do
  begin
    MachineSet[x].RunData.Destroy;
    MachineSet[x].DUT.Destroy;
  end;
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

procedure TPowerbankMainForm.OverviewDrawGridDrawCell(Sender: TObject; aCol,
  aRow: Integer; aRect: TRect; aState: TGridDrawState);
var
  aDrawGrid      : TDrawGrid;
  s              : string;
  PMS            : PMachineSet;
begin
  if ((aRow>0) AND (aRow<=High(MachineSet))) then
  begin
    PMS := @MachineSet[aRow];
    aDrawGrid:=TDrawGrid(Sender);
    s:='';

    case aCol of
      0:
      begin
        if (PMS^.MachineNumber=0) then
          s:='-'
        else
          s:=InttoStr(PMS^.MachineNumber-1);
      end;
      1: s:=PMS^.Product;
      4: s:=FloatToStrF(PMS^.LatestMeasurementData.Voltage,ffFixed, 2, 1);
      5: s:=FloatToStrF(PMS^.LatestMeasurementData.Current,ffFixed, 1, 2);
      6: s:=FloatToStrF(PMS^.LatestMeasurementData.Power,ffFixed, 4, 1);
      7: s:=FloatToStrF(PMS^.LatestMeasurementData.Energy,ffFixed, 4, 1);
      8: s:=FloatToStrF(PMS^.LatestMeasurementData.Capacity,ffFixed, 4, 1);
    end;

    if (PMS^.MachineNumber = 0) then
    begin
      aDrawGrid.Canvas.Font.Color:=clRed;
      aDrawGrid.Canvas.Font.Style:=[fsBold];
    end
    else
    begin
      aDrawGrid.Canvas.Font.Color:=clLime;
      aDrawGrid.Canvas.Font.Style:=[fsBold];
    end;


    InflateRect(aRect, -constCellpadding, -constCellPadding);
    aDrawGrid.Canvas.TextRect(aRect, aRect.Left, aRect.Top, s);
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
  PDUT            : PUSBPDDevice;
begin
  PDOCurrent:=0;
  PDOVoltage:=0;

  aSourcePDO.Raw:=0;
  aSinkPDO.Raw:=0;

  PDONumber:=0;

  PDUT := @PCurrentMachineSet^.DUT;

  if Sender=SourcePDODrawGrid then aSourcePDO:=PDUT^.SourcePDOs[aRow];
  if Sender=SourceEPRPDODrawGrid then aSourcePDO:=PDUT^.SourceEPRPDOs[aRow];

  if Sender=SinkPDODrawGrid then aSinkPDO:=PDUT^.SinkPDOs[aRow];
  if Sender=SinkEPRPDODrawGrid then aSinkPDO:=PDUT^.SinkEPRPDOs[aRow];


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

    DD.SetMaxPDO(PCurrentMachineSet^.MachineNumber,PDONumber);
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

  PDUT            : PUSBPDDevice;
begin
  if ((aRow>0) AND (aCol>0)) then
  begin
    PDUT := @PCurrentMachineSet^.DUT;

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
        if (Sender=SourcePDODrawGrid) then aSourcePDO:=PDUT^.SourcePDOs[aRow];
        if (Sender=SourceEPRPDODrawGrid) then aSourcePDO:=PDUT^.SourceEPRPDOs[aRow];
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
        if (Sender=SinkPDODrawGrid) then aSinkPDO:=PDUT^.SinkPDOs[aRow];
        if (Sender=SinkEPRPDODrawGrid) then aSinkPDO:=PDUT^.SinkEPRPDOs[aRow];
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

    if (((Sender=SourcePDODrawGrid) AND (PDUT^.RDOPosition=aRow)) OR ((Sender=SourceEPRPDODrawGrid) AND (PDUT^.RDOPosition=aRow+7))) then
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
  //FilenameEdit.Enabled:=(NOT AutoFileNameCheckBox.Checked);
  //PCurrentMachineSet^.AutoFileName:=AutoFileNameCheckBox.Checked;
end;

procedure TPowerbankMainForm.btnRotoPDGetPDOListClick(Sender: TObject);
begin
  // Remove all previous data
  PCurrentMachineSet^.DUT.Cleanup;
  SourcePDODrawGrid.Invalidate;
  SourceEPRPDODrawGrid.Invalidate;
  DD.GetPDOList(PCurrentMachineSet^.MachineNumber);
end;

procedure TPowerbankMainForm.Button2Click(Sender: TObject);
begin
  //DD.SetMaxPDO(PCurrentMachineSet^.MachineNumber,1);
  DD.SetPPSPDO(PCurrentMachineSet^.MachineNumber,6,3000,8500);
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
  PDUT: PUSBPDDevice;
begin
  if aCol=0 then
  begin
    PDUT := @PCurrentMachineSet^.DUT;
    state:=false;
    aPDO:=PDUT^.GetFixed5VSRCPDO;
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
  FOldOnIdle := Application.OnIdle; // Save GUICurrent OnIdle.
  Application.OnIdle:=AfterShow;
end;

procedure TPowerbankMainForm.FormClose(Sender: TObject; var CloseAction: TCloseAction);
var
  Ini               : TIniFile;
  LocalMachine      : integer;
  LocalPMS          : PMachineSet;
begin
  for LocalMachine:=Low(MachineSet) to High(MachineSet) do
  begin
    LocalPMS:=@MachineSet[LocalMachine];
    if ((LocalPMS^.Active) OR (LocalPMS^.Testing)) then
    begin
       CloseAction:=CaNone;
       break;
    end;
  end;

  if (CloseAction<>CaNone) then
  if MessageDlg ('Are you REALLY SURE you want to exit ?'+
                  chr(13)+'(This is your last change to stay with us!)',
                  mtConfirmation, [mbYes,mbNo],0)=idNo
     then CloseAction :=CaNone
     else
     begin
       AllStop(nil);

       UpdateTimer.Enabled:=False;

       //DD.Enabled:=False;
       DD.OnDataReceived:=nil;
       DD.OnDeviceChange:=nil;
       DD.Destroy;

       Ini := TIniFile.Create( ChangeFileExt( Application.ExeName, '.ini' ) );
       try
         ini.WriteInteger(Self.Name,'Top',Self.Top);
         ini.WriteInteger(Self.Name,'Left',Self.Left);
         ini.WriteInteger(Self.Name,'Width',Self.Width);
         ini.WriteInteger(Self.Name,'Height',Self.Height);

         Ini.WriteInteger('General', 'NumRate', NumRate);
         Ini.WriteString('Project', 'ID', Project_edit.Text);

       finally
         Ini.Free;
       end;


       CloseAction:=caFree;
     end;
end;

procedure TPowerbankMainForm.EngageClick(Sender: TObject);
var
  F        : textfile;
  runtext  : string;
  SummaryFilename:string;
begin
  TSpeedButton(Sender).Enabled:=False;

  try
    if TSpeedButton(Sender).Down then
    begin
      if (true) then
      begin
        SaveMachineData;

        // Do machinebox things
        with PCurrentMachineBox^ do
        begin
          Enable:=true;
          runtext:=PCurrentMachineSet^.Runname+' '+PCurrentMachineSet^.Runnumber;
          if Length(runtext)>1
             then Title:=PCurrentMachineSet^.Product+' '+runtext
             else Title:=PCurrentMachineSet^.Product;
        end;

        // Do MachineSet things
        with PCurrentMachineSet^ do
        begin
          RunData.Clear;
          LatestMeasurementData:=Default(TMachineData);
          if (Sender=StartStopSpeedButton) then Active:=True;
          if (Sender=btnTestDischarge) then Testing:=True;
          Last_time:=NowUTC;
        end;

        //MachineChart.UndoZoom;
        if MachineChart.IsZoomed then MachineChart.ZoomFull(false);
        MachineChartLineSeries1.Clear;
        MachineChartLineSeries2.Clear;
        MachineChartLineSeries3.Clear;

        SetControls(Sender);

        SetChartAxis(Sender);

        if (Sender=StartStopSpeedButton) then
        begin
          CreateDataFile(Sender);
          FilenameEdit.Text:=ExtractFileName(PCurrentMachineSet^.Filename);
          TestInfoMemo.Lines.Append('Filename: '+PCurrentMachineSet^.Filename);
        end;

        ImageList1.BeginUpdate;
        if PCurrentMachineSet^.BoardAvailable then
        begin
          ReplaceImageFromResource(ImageList1,{Channel=Index=}PCurrentMachineSet^.MachineNumber-1,'RUNNING_05_32');
        end;
        ImageList1.EndUpdate;
        TabControl1.Invalidate;

        if (Sender=StartStopSpeedButton) then
        begin
          // Reset IT2704 energy data
          DD.SetEnergy(PCurrentMachineSet^.MachineNumber,0);
          //Sleep(25);
          GUIPower:=0;
          GUIEnergy:=0;
          GUICapacity:=0;
        end;

        DD.SetThreshold(PCurrentMachineSet^.MachineNumber,TStageMode.smCurrent,TThresholdMode.tmMINV,True,900);
        DD.SetThreshold(PCurrentMachineSet^.MachineNumber,TStageMode.smCurrent,TThresholdMode.tmMAXV,True,19000);

        // Disable all buttons that can control the device
        // To prevent any disturbance of this running test
        DD.SetLocal(PCurrentMachineSet^.MachineNumber, false);

        // Set mode to easy and enable some GUI indications
        DD.SetMode(PCurrentMachineSet^.MachineNumber, TBatteryMode.bmActive);

        // Set the discharge value !!
        DD.SetCurrent(PCurrentMachineSet^.MachineNumber,PCurrentMachineSet^.ActiveTestType.Current,{Fine=}0);
        // Output already set with the above command.
        // So, SetOutput not needed here.
        //DD.SetOutput(PCurrentMachineSet^.MachineNumber, true);

        TestInfoMemo.Lines.Append('Test started.');

        // Do a forced first datastore event
        DataStoreTimerTimer(nil);

      end;
    end
    else
    begin
      TestInfoMemo.Lines.Append('Test stopped.');

      DD.SetOutput(PCurrentMachineSet^.MachineNumber, false);

      DD.SetOff(PCurrentMachineSet^.MachineNumber);

      // Set mode to easy and enable some GUI indications
      //DD.SetMode(PCurrentMachineSet^.MachineNumber, TBatteryMode.bmIdle);
      //DD.SetMode(PCurrentMachineSet^.MachineNumber, TBatteryMode.bmReady);

      // Enable all buttons that can control the device
      DD.SetLocal(PCurrentMachineSet^.MachineNumber, true);

      // Do MachineSet things
      with PCurrentMachineSet^ do
      begin
        if (Sender=StartStopSpeedButton) then Active:=False;
        if (Sender=btnTestDischarge) then Testing:=False;
      end;

      SetControls(Sender);

      PCurrentMachineBox^.Enable:=false;

      ImageList1.BeginUpdate;
      if PCurrentMachineSet^.BoardAvailable then
      begin
        ReplaceImageFromResource(ImageList1,{Channel=Index=}PCurrentMachineSet^.MachineNumber-1,'OK_02_32');
      end;
      ImageList1.EndUpdate;
      TabControl1.Invalidate;

      if (Sender=StartStopSpeedButton) then
      with PCurrentMachineSet^ do
      begin
        if RunData.Count>1 then
        begin
          SummaryFilename:=ApplicationDirectory+'Summary__';
          SummaryFilename:=SummaryFilename + Format('%.4d-%.2d-%.2d',[YearOf(Now), MonthOfTheYear(Now), DayOfTheMonth(Now)]);
          SummaryFilename:=SummaryFilename+'.dat';
          try
            AssignFile(F,SummaryFileName);
            if (NOT IsOpen(F)) then
            try
              if NOT FileExists(SummaryFileName) then
              begin
                Rewrite(F);
                Write(F, #$EF#$BB#$BF);
                if IOResult=0 then
                begin
                  Writeln(F, 'Overview of data collected on '+DateToStr(Now));
                  Writeln(F);
                  Write(F,'Project'+LocalFormatSettings.ListSeparator);
                  Write(F,'Product'+LocalFormatSettings.ListSeparator);
                  Write(F,'Run'+LocalFormatSettings.ListSeparator);
                  Write(F,'Location'+LocalFormatSettings.ListSeparator);
                  Write(F,'Filename'+LocalFormatSettings.ListSeparator);
                  Write(F,'Time [msec]'+LocalFormatSettings.ListSeparator);
                  Write(F,'Voltage [V]'+LocalFormatSettings.ListSeparator);
                  Write(F,'Current [A]'+LocalFormatSettings.ListSeparator);
                  Write(F,'Energy [kWh]'+LocalFormatSettings.ListSeparator);
                  Writeln(F,'Power [W]'+LocalFormatSettings.ListSeparator);
                end;
              end else Append(F);
              if IOResult=0 then
              begin
                Write(F,Project+LocalFormatSettings.ListSeparator);
                Write(F,Product+LocalFormatSettings.ListSeparator);
                Write(F,Runname+'_'+Runnumber+LocalFormatSettings.ListSeparator);
                Write(F,InttoStr(PCurrentMachineSet^.MachineNumber)+LocalFormatSettings.ListSeparator);
                Write(F,ExtractFileName(Filename)+LocalFormatSettings.ListSeparator);
                Write(F,InttoStr(LatestMeasurementData.Elapsed)+LocalFormatSettings.ListSeparator);
                Write(F,FloatToStrF(ActiveTestType.Voltage,ffFixed, 9, 3));
                Write(F,FloatToStrF(ActiveTestType.Current,ffFixed, 9, 1)+LocalFormatSettings.ListSeparator);
                Write(F,FloatToStrF(LatestMeasurementData.Energy,ffFixed, 9, 3)+LocalFormatSettings.ListSeparator);
                Writeln(F,FloatToStrF(LatestMeasurementData.Power,ffFixed, 9, 1)+LocalFormatSettings.ListSeparator);
              end;
            finally
              CloseFile(F);
            end;
          except
            CloseFile(F);
          end;
        end;
      end;

    end;

  finally
    TSpeedButton(Sender).Enabled:=True;
  end;
end;

procedure TPowerbankMainForm.DataStoreTimerTimer(Sender: TObject);
const
  STOPPERCENTAGE = 80;
var
  Elapsed           : longword;
  LocalMachine      : integer;
  LocalPMS          : PMachineSet;
  Running           : boolean;
begin
  Running:=false;

  for LocalMachine:=Low(MachineSet) to High(MachineSet) do
  begin
    // only perform the below if coming from the timer !
    if Assigned(Sender) then
    begin
      LocalPMS:=@MachineSet[LocalMachine];
      if ((LocalPMS^.Active) OR (LocalPMS^.Testing)) then
      begin
        Running:=true;

        // Calculate how much time has elapsed between two data storage moment
        Elapsed:=MilliSecondsBetween(NowUTC,LocalPMS^.Last_time);
        LocalPMS^.Last_time:=NowUTC;
        // We store elapsed cummulative
        LocalPMS^.LatestMeasurementData.Elapsed:=LocalPMS^.LatestMeasurementData.Elapsed+Elapsed;

        // Add the current data into the datastore
        LocalPMS^.RunData.Add(LocalPMS^.LatestMeasurementData);

        if (LocalMachine=PCurrentMachineSet^.MachineNumber) then
        begin
          // Add data into chart
          MachineChartLineSeries1.Add(PCurrentMachineSet^.LatestMeasurementData.Voltage);
          MachineChartLineSeries2.Add(PCurrentMachineSet^.LatestMeasurementData.Current);
        end;

        if (LocalPMS^.Active) then
        begin
          SaveBatteryData(LocalPMS);

          if (
             (chkVoltageLimit.Checked)
             AND
             (LocalPMS^.LatestMeasurementData.Voltage<(LocalPMS^.ActiveTestType.Voltage*(STOPPERCENTAGE/100)/1000))
             )
          then
          begin
            TestInfoMemo.Lines.Append('Voltage below threshold. Test ready.');
            // Do something !!

            // Set mode to easy and enable some GUI indications
            DD.SetMode(LocalPMS^.MachineNumber, TBatteryMode.bmReady);
          end;

          if (
             (chkCurrentLimit.Checked)
             AND
             (LocalPMS^.LatestMeasurementData.Current<(LocalPMS^.ActiveTestType.Current*(STOPPERCENTAGE/100)/1000))
             )
          then
          begin
            TestInfoMemo.Lines.Append('Current below threshold. Test ready.');
            // Do something !!

            // Set mode to easy and enable some GUI indications
            DD.SetMode(LocalPMS^.MachineNumber, TBatteryMode.bmReady);
          end;

        end;
      end;
    end;
  end;

  if Running then
  begin
    if (ProgressBar1.Position=ProgressBar1.Max) then
      ProgressBar1.Position:=1
    else
      ProgressBar1.StepIt;
  end;

end;

procedure TPowerbankMainForm.EditChangeEmptyCheck(Sender: TObject);
begin
  if
  (Length(Project_edit.Text)>0)
  AND
  (Length(Product_edit.Text)>0)
  AND
  (Length(Runname_edit.Text)>0)
  AND
  (Length(Runnumber_edit.Text)>0)
  then
    StartStopSpeedButton.Enabled:=true
  else
    StartStopSpeedButton.Enabled:=false;
end;

procedure TPowerbankMainForm.SamplesBoxSelect(Sender: TObject);
begin
  Runnumber_edit.Text:=SamplesBox.Text;
end;

procedure TPowerbankMainForm.TabControl1Change(Sender: TObject);
var
  z:integer;
  TC:TTabControl;
  LocalVisibleMachine:integer;
begin
  TC:=TTabControl(Sender);

  LocalVisibleMachine:=TC.TabIndex+1;

  if (LocalVisibleMachine <> 0) then
  begin
    if (LocalVisibleMachine<>PCurrentMachineSet^.MachineNumber) then
    begin
      // Point global dataset pointer towards the correct dataset
      PCurrentMachineSet:=@MachineSet[LocalVisibleMachine];
      PCurrentMachineBox:=@MachineBox[LocalVisibleMachine];

      SetControls(Sender);

      MachineChartLineSeries1.BeginUpdate;
      MachineChartLineSeries2.BeginUpdate;

      MachineChartLineSeries1.Clear;
      MachineChartLineSeries2.Clear;

      //MachineChart.UndoZoom;
      if MachineChart.IsZoomed then MachineChart.ZoomFull(false);

      if PCurrentMachineSet^.RunData.Count>0 then
      begin
        for z:=0 to PCurrentMachineSet^.RunData.Count-1 do
        begin
          MachineChartLineSeries1.Add(PCurrentMachineSet^.RunData[z].Voltage);
          MachineChartLineSeries2.Add(PCurrentMachineSet^.RunData[z].Current);
        end;
        // Why ?
        if (NOT PCurrentMachineSet^.Active) then
        begin
          PCurrentMachineSet^.LatestMeasurementData.Energy:=PCurrentMachineSet^.RunData.Last.Energy;
          PCurrentMachineSet^.LatestMeasurementData.Voltage:=PCurrentMachineSet^.RunData.Last.Voltage;
          PCurrentMachineSet^.LatestMeasurementData.Current:=PCurrentMachineSet^.RunData.Last.Current;
          PCurrentMachineSet^.LatestMeasurementData.Power:=PCurrentMachineSet^.RunData.Last.Power;
          PCurrentMachineSet^.LatestMeasurementData.Capacity:=PCurrentMachineSet^.RunData.Last.Capacity;
          PCurrentMachineSet^.LatestMeasurementData.Elapsed:=PCurrentMachineSet^.RunData.Last.Elapsed;
        end;

      end;

      MachineChartLineSeries2.EndUpdate;
      MachineChartLineSeries1.EndUpdate;


      //AutoFileNameCheckBox.Checked:=PCurrentMachineSet^.AutoFileName;

      FilenameEdit.Text:=ExtractFileName(PCurrentMachineSet^.Filename);
      Project_edit.Text:=PCurrentMachineSet^.Project;
      Product_edit.Text:=PCurrentMachineSet^.Product;
      Runname_edit.Text:=PCurrentMachineSet^.Runname;
      Runnumber_edit.Text:=PCurrentMachineSet^.Runnumber;
      MachineInfoMemo.Lines.CommaText:=PCurrentMachineSet^.Info;

      GUIVoltage:=PCurrentMachineSet^.LatestMeasurementData.Voltage;
      GUICurrent:=PCurrentMachineSet^.LatestMeasurementData.Current;
      GUIPower:=PCurrentMachineSet^.LatestMeasurementData.Power;
      GUIEnergy:=PCurrentMachineSet^.LatestMeasurementData.Energy;
      GUICapacity:=PCurrentMachineSet^.LatestMeasurementData.Capacity;
      GUITemperature:=0;

      SourcePDODrawGrid.Invalidate;
      SourceEPRPDODrawGrid.Invalidate;
    end;
  end;
end;

procedure TPowerbankMainForm.TabControl1Changing(Sender: TObject;
  var AllowChange: Boolean);
begin
  SaveMachineData;
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

procedure TPowerbankMainForm.ProgressStepperTimerTimer(Sender: TObject);
begin
  if (ProgressBar1.Position=ProgressBar1.Max) then
    ProgressBar1.Position:=1
  else
    ProgressBar1.StepIt;
end;

procedure TPowerbankMainForm.TestsBoxDblClick(Sender: TObject);
begin
  Runname_edit.Text:=TestsBox.Text;
end;

procedure TPowerbankMainForm.TestsBoxSelect(Sender: TObject);
begin
  Runname_edit.Text:=TestsBox.Text;
end;

function CalcCellHeight(AGrid: TDrawGrid; ACol, ARow: Integer): Integer;
var
  R: TRect;
  s: String;
begin
  R := AGrid.CellRect(ACol, ARow);
  R.Bottom := R.Top;
  //s := AGrid.Cells[ACol, ARow];
  s:='yolo';
  DrawText(AGrid.Canvas.Handle, PChar(s), Length(s), R, DT_CALCRECT or DT_WORDBREAK or DT_NOPREFIX);
  Result := R.Bottom - R.Top;

  Result := 100;

end;

function CalcRowHeight(AGrid: TDrawGrid; ARow: Integer): Integer;
var
  col: Integer;
  h, hmax: Integer;
begin
  hmax := AGrid.DefaultRowHeight;
  for col := 0 to AGrid.ColCount-1 do
  begin
    h := CalcCellHeight(AGrid, col, ARow);
    if h > hmax then hmax := h;
  end;
  Result := hmax;
end;

procedure AutoRowHeights(AGrid: TDrawGrid);
var
  row: Integer;
begin
  for row := 0 to AGrid.RowCount-1 do
    AGrid.RowHeights[row] := CalcRowHeight(AGrid, row) + 2*varCellPadding;
end;

procedure TPowerbankMainForm.tsOverview2Resize(Sender: TObject);
var
 x,Count:integer;
 rowheight,textsize,textwidth,textheight:integer;
begin
  //OverviewDrawGrid.BeginUpdate;
  //AutoRowHeights(OverviewDrawGrid);
  //OverviewDrawGrid.EndUpdate;

  //exit;
  OverviewDrawGrid.BeginUpdate;
  with OverviewDrawGrid do
  begin

    Count:=RowCount-1;
    //rowheight:=(Height-(Count+1)*GridLineWidth) DIV Count;
    rowheight:=Max(10,((Height-DefaultRowHeight-(Count)*GridLineWidth) DIV (Count)-1));

    //DefaultRowHeight:=rowheight;

    for x:=1 to Count do
    begin
      RowHeights[x]:=rowheight;
    end;

    Count:=ColCount;
    //DefaultColWidth:=Min(150,((tsOverview2.Width-(Count+1)*GridLineWidth) DIV Count)-1);
    //DefaultColWidth:=((Width-(Count+1)*GridLineWidth) DIV Count)-1;
    //Font.Size:=(DefaultRowHeight DIV 4)+2;
    {$ifdef FPC}
    textsize:=6;
    while true do
    begin
      OverviewDrawGrid.Font.Size:=textsize;
      textwidth:=OverviewDrawGrid.Canvas.TextWidth('0.000');
      textheight:=OverviewDrawGrid.Canvas.TextHeight('0.000');
      if (textheight>(rowheight-10)) then break;
      Inc(textsize);
    end;
    Dec(textsize);
    OverviewDrawGrid.Font.Size:=textsize;
    OverviewDrawGrid.TitleFont.Size:=OverviewDrawGrid.Font.Size;
    {$else}
    OverviewDrawGrid.Canvas.Font.Size:=Max(DefaultRowHeight-7-(Count DIV 12),6);
    {$endif}
    //with OverviewDrawGrid.Canvas.TextExtent('jQH'#$00B0';') do DefaultRowHeight := cy+4;

  end;
  OverviewDrawGrid.EndUpdate;
end;

procedure TPowerbankMainForm.TypesBoxDblClick(Sender: TObject);
begin
  Product_edit.Text:=TypesBox.Text;
end;

procedure TPowerbankMainForm.TypesBoxSelect(Sender: TObject);
var
  PC,PB,PS:string;
begin
  Product_edit.Text:=TypesBox.Text;
  PC:=ExtractWord(1,Product_edit.Text,[' ']);
  PB:=ExtractWord(2,Product_edit.Text,[' ']);
  PS:=ExtractWord(3,Product_edit.Text,[' ']);
  DD.SetProductInfo(PCurrentMachineSet^.MachineNumber,PC,PB,PS);
end;

procedure TPowerbankMainForm.UpdateTimerTimer(Sender: TObject);
begin
  Edit1.Text:='CONSULAB    ' + DateTimeToStr(NowUTC);
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
  RealVoltageDisplay.Value:=value;
end;

procedure TPowerbankMainForm.SetCurrent(value:double);
begin
  RealCurrentDisplay.Value:=value;
end;

procedure TPowerbankMainForm.SetCapacity(value:double);
begin
  NewAHDisplay.Value:=value;
end;

procedure TPowerbankMainForm.SetEnergy(value:double);
begin
  NewEnergyDisplay.Value:=value;
end;

procedure TPowerbankMainForm.SetPower(value:double);
begin
  NewPowerDisplay.Value:=value;
end;

procedure TPowerbankMainForm.SetTemperature(value:double);
begin
  NewTemperatureDisplay.Value:=value;
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
end;

procedure TPowerbankMainForm.CreateDataFile(Sender: TObject);
var
  BatteryDataFileBackup      : string;
  BatteryDataFile            : string;
  Oldfile,NewFile            : TFileStream;
  TimeString                 : string;
begin
  TimeString := Format('%.2d-%.2d-%.4d',[DayOfTheMonth(NowUTC), MonthOfTheYear(NowUTC), YearOf(NowUTC)])+'__';
  TimeString := TimeString + Format('%.2d-%.2d-%.2d',[HourOfTheDay(NowUTC), MinuteOfTheHour(NowUTC), SecondOfTheMinute(NowUTC)])+'__';

  //BatteryDataFile:=Copy(TypesBox.Text,1,pos(' ',TypesBox.Text)-1)+'_'+SamplesBox.Text;
  BatteryDataFile:=Copy(Product_edit.Text ,1,pos(' ',Product_edit.Text )-1);

  if Assigned(Sender) then
  begin
    //if Sender=btnCurve then BatteryDataFile:=BatteryDataFile+'_Curve';
    if Sender=StartStopSpeedButton then BatteryDataFile:=BatteryDataFile+'_Normal';
  end
  else
  begin
    BatteryDataFile:=BatteryDataFile+'_Special';
  end;
  BatteryDataFile:=BatteryDataFile+'_'+PCurrentMachineSet^.ActiveTestType.Name+'__'+TimeString;
  BatteryDataFile:=BatteryDataFile+'data.csv';

  BatteryDataFile:=StringReplace(BatteryDataFile,' ','-',[rfReplaceAll]);
  BatteryDataFile:=StringReplace(BatteryDataFile,'@','-',[rfReplaceAll]);
  BatteryDataFile:=StringReplace(BatteryDataFile,'/','-',[rfReplaceAll]);

  BatteryDataFile:=ExtractFilePath(Application.ExeName)+BatteryDataFile;

  PCurrentMachineSet^.Filename:=BatteryDataFile;

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
var
  MachineRunner:integer;
begin
  UpdateTimer.Enabled:=False;
  DataStoreTimer.Enabled:=False;
  ProgressStepperTimer.Enabled:=False;

  for MachineRunner:=Low(MachineSet) to High(MachineSet) do
  begin
    DD.SetOutput(MachineRunner,false);
    DD.SetOff(MachineRunner);
    MachineSet[MachineRunner].Active:=false;
    MachineSet[MachineRunner].Testing:=false;
    MachineSet[MachineRunner].RunData.Clear;
    MachineSet[MachineRunner].LatestMeasurementData:=Default(TMachineData);
  end;

  SetControls(Sender);

  btnTestDischarge.Down:=false;

  ProgressBar1.Position:=0;
end;

procedure TPowerbankMainForm.SaveBatteryData(LocalPMS: PMachineSet);
var
  F : textfile;
  PC,PB,PS:string;
  i:integer;
begin
  if (Length(LocalPMS^.Filename)=0) then CreateDataFile(nil);

  AssignFile(F,LocalPMS^.Filename );
  try

    if FileExists(LocalPMS^.Filename)
       then Append(F)
       else
       begin
         PC:=ExtractWord(1,LocalPMS^.Product,[' ']);
         PB:=ExtractWord(2,LocalPMS^.Product,[' ']);
         PS:=ExtractWord(3,LocalPMS^.Product,[' ']);

         Rewrite(F);
         Write(F, #$EF#$BB#$BF);
         writeln(F, 'Code: ',PC);
         writeln(F, 'Brand: ',PB);
         writeln(F, 'Model: ',PS);
         writeln(F, 'Product: ',LocalPMS^.Product);
         writeln(F, 'Sample: ',SamplesBox.Text);
         writeln(F, 'Test: ',LocalPMS^.ActiveTestType.Name);
         writeln(F, 'Discharge current: ',LocalPMS^.ActiveTestType.Current,' mA');
         writeln(F, 'Discharge voltage: ',LocalPMS^.ActiveTestType.Voltage,' mV');
         Writeln(F, 'Date: '+DateTimeToStr(NowUTC,LocalFormatSettings));
         Writeln(F, 'Machine: '+InttoStr(LocalPMS^.MachineNumber));
         Writeln(F, 'Info:');
         Writeln(F, LocalPMS^.Info);
         Writeln(F, '**********************************************');
         writeln(F);
         write(F,'Trigger moment UTC',LocalFormatSettings.ListSeparator);
         write(F,'Human time',LocalFormatSettings.ListSeparator);
         write(F,'Elapsed(msec)',LocalFormatSettings.ListSeparator);
         write(F,'Voltage(V)',LocalFormatSettings.ListSeparator);
         write(F,'Current(A)',LocalFormatSettings.ListSeparator);
         write(F,'Power(W)',LocalFormatSettings.ListSeparator);
         write(F,'Capacity(Ah)',LocalFormatSettings.ListSeparator);
         write(F,'Energy(Wh)',LocalFormatSettings.ListSeparator);
         writeln(F);
       end;

    write(F,FloattoStr(NowUTC,LocalFormatSettings),LocalFormatSettings.ListSeparator);
    write(F,FormatDateTime('dd-mm-yyyy hh:nn:ss',NowUTC),LocalFormatSettings.ListSeparator);
    write(F,FloattoStrF((LocalPMS^.LatestMeasurementData.Elapsed / 1000),ffFixed,6,1,LocalFormatSettings),LocalFormatSettings.ListSeparator);
    write(F,FloattoStrF(LocalPMS^.LatestMeasurementData.Voltage,ffFixed,10,3,LocalFormatSettings),LocalFormatSettings.ListSeparator);
    write(F,FloattoStrF(LocalPMS^.LatestMeasurementData.Current,ffFixed,10,3,LocalFormatSettings),LocalFormatSettings.ListSeparator);
    write(F,FloattoStrF(LocalPMS^.LatestMeasurementData.Power,ffFixed,10,3,LocalFormatSettings),LocalFormatSettings.ListSeparator);
    write(F,FloattoStrF(LocalPMS^.LatestMeasurementData.Capacity,ffFixed,10,3,LocalFormatSettings),LocalFormatSettings.ListSeparator);
    write(F,FloattoStrF(LocalPMS^.LatestMeasurementData.Energy,ffFixed,10,3,LocalFormatSettings),LocalFormatSettings.ListSeparator);

    writeln(F);
  finally
    CloseFile(F);
  end;
end;

procedure TPowerbankMainForm.SetActiveTestType(const TT:TTestType);
begin
  PCurrentMachineSet^.ActiveTestType.Name:=TT.Name;
  PCurrentMachineSet^.ActiveTestType.Voltage:=TT.Voltage;
  PCurrentMachineSet^.ActiveTestType.Current:=TT.Current;
  CurrentEdit.Text:=InttoStr(PCurrentMachineSet^.ActiveTestType.Current);
  VoltageEdit.Text:=InttoStr(PCurrentMachineSet^.ActiveTestType.Voltage);
  CurrentEdit.ReadOnly:=((Pos('Power',PCurrentMachineSet^.ActiveTestType.Name)=0) AND (Pos('Variable',PCurrentMachineSet^.ActiveTestType.Name)=0));
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
  MB:integer;
  PDOCount,PDOIndex    : byte;
  RawPDO               : PDO_DATA_T;
  PDO                  : TAP33772S_PDO;
  wd                   : TWordData;
  dwd                  : TDWordData;
  qwd                  : TQWordData;
  bd                   : TByteData;
  aSourcePDO           : ^TSOURCEPDO;
  LocalPMS             : PMachineSet;
begin
  // The boardnumber is an absolute indicator !!
  boardnumber:=PByteArray(data)^[INDEXPOSITION];
  // Point towards the correct dataset
  LocalPMS:=@MachineSet[boardnumber];

  cmd:=TCommands(PByteArray(data)^[COMMANDPOSITION]);
  length:=PByteArray(data)^[LENGTHPOSITION];

  //USBDebugLog.Lines.Append('Received data from board #'+InttoStr(boardnumber));


  case cmd of
    TCommands.CMD_get_data:
    begin
      Counter:=DATASTART;

      //RotoPD Voltage
      wd.Bytes[0]:=PByteArray(data)^[counter];
      wd.Bytes[1]:=PByteArray(data)^[counter+1];
      LocalPMS^.LatestMeasurementData.Voltage:=(wd.Raw/1000);

      Inc(counter,2);

      //RotoPD Current
      wd.Bytes[0]:=PByteArray(data)^[counter];
      wd.Bytes[1]:=PByteArray(data)^[counter+1];
      LocalPMS^.LatestMeasurementData.Current:=(wd.Raw/1000);

      Inc(counter,2);

      //RotoPD Power
      dwd.Bytes[0]:=PByteArray(data)^[counter];
      dwd.Bytes[1]:=PByteArray(data)^[counter+1];
      dwd.Bytes[2]:=PByteArray(data)^[counter+2];
      dwd.Bytes[3]:=PByteArray(data)^[counter+3];
      LocalPMS^.LatestMeasurementData.Power:=(dwd.Raw/1000);

      Inc(counter,4);

      //RotoPD PCB/chip temperature
      wd.Bytes[0]:=PByteArray(data)^[counter];
      wd.Bytes[1]:=PByteArray(data)^[counter+1];
      if (boardnumber=PCurrentMachineSet^.MachineNumber) then
      begin
        GUITemperature:=(wd.Raw/10);
      end;

      Inc(counter,2);

      if (length>13) then
      begin
        //IT2704 Voltage
        wd.Bytes[0]:=PByteArray(data)^[counter];
        wd.Bytes[1]:=PByteArray(data)^[counter+1];
        LocalPMS^.LatestMeasurementData.Voltage:=(wd.Raw/1000);

        Inc(counter,2);

        //IT2704 Current
        wd.Bytes[0]:=PByteArray(data)^[counter];
        wd.Bytes[1]:=PByteArray(data)^[counter+1];
        LocalPMS^.LatestMeasurementData.Current:=(wd.Raw/1000);

        Inc(counter,2);

        //IT2704 Power
        dwd.Bytes[0]:=PByteArray(data)^[counter];
        dwd.Bytes[1]:=PByteArray(data)^[counter+1];
        dwd.Bytes[2]:=PByteArray(data)^[counter+2];
        dwd.Bytes[3]:=PByteArray(data)^[counter+3];
        LocalPMS^.LatestMeasurementData.Power:=(dwd.Raw/1000);

        Inc(counter,4);

        //IT2704 Capacity
        dwd.Bytes[0]:=PByteArray(data)^[counter];
        dwd.Bytes[1]:=PByteArray(data)^[counter+1];
        dwd.Bytes[2]:=PByteArray(data)^[counter+2];
        dwd.Bytes[3]:=PByteArray(data)^[counter+3];
        LocalPMS^.LatestMeasurementData.Capacity:=(dwd.Raw/1000);

        Inc(counter,4);

        //IT2704 Energy
        dwd.Bytes[0]:=PByteArray(data)^[counter];
        dwd.Bytes[1]:=PByteArray(data)^[counter+1];
        dwd.Bytes[2]:=PByteArray(data)^[counter+2];
        dwd.Bytes[3]:=PByteArray(data)^[counter+3];
        LocalPMS^.LatestMeasurementData.Energy:=(dwd.Raw/1000);

        Inc(counter,4);
      end;

      if (boardnumber=PCurrentMachineSet^.MachineNumber) then
      begin
        USBDebugLog.Lines.Append('Received data for current board #'+InttoStr(boardnumber));

        // We got data addressed to our current visible machine
        // So update the GUI also
        GUIVoltage:=LocalPMS^.LatestMeasurementData.Voltage;
        GUICurrent:=LocalPMS^.LatestMeasurementData.Current;
        GUIPower:=LocalPMS^.LatestMeasurementData.Power;
        GUIEnergy:=LocalPMS^.LatestMeasurementData.Energy;
        GUICapacity:=LocalPMS^.LatestMeasurementData.Capacity;
      end;

      OverviewDrawGrid.InvalidateRow(boardnumber);

      // Update the overview frames also
      with MachineBox[boardnumber] do
      begin
        Voltage:=LocalPMS^.LatestMeasurementData.Voltage;
        Current:=LocalPMS^.LatestMeasurementData.Current;
        Power:=LocalPMS^.LatestMeasurementData.Power;
        Energy:=LocalPMS^.LatestMeasurementData.Energy;
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
          if (PCurrentMachineSet^.ActiveTestType.Name=TestTypes[(Low(TestTypes)+1)].Name) then
          begin
            SetActiveTestType(TestTypes[(Low(TestTypes)+1)]);
          end;
        end;

        if (PDO.isEPR) then
          aSourcePDO := @PCurrentMachineSet^.DUT.SourceEPRPDOs[PDO.index-8]
        else
          aSourcePDO := @PCurrentMachineSet^.DUT.SourcePDOs[PDO.index-1];

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
            AugmentedPowerDataObjectType := bd.Raw; //%01;  // 0x01=EPRAVS Adjustable GUIVoltage Supply
            bd.Raw := 0;
            bd.Bits[0] := 1;
            bd.Bits[1] := 1;
            AugmentedPowerDataObject     := bd.Raw; //%11;  // 0x11
          end;
        end;
      end;

      if (boardnumber=PCurrentMachineSet^.MachineNumber) then
      begin
        SourcePDODrawGrid.Invalidate;
        SourceEPRPDODrawGrid.Invalidate;
      end;
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


      LocalPMS^.DUT.ActiveSRCPDO:=PDOIndex;

      LocalPMS^.DUT.RDOPosition:=PDOIndex;

      TestTypes[(Low(TestTypes)+1)].Current:=PDO.maxCurrent_mA;
      TestTypes[(Low(TestTypes)+1)].Voltage:=PDO.maxVoltage_mV;

      if (PCurrentMachineSet^.ActiveTestType.Name=TestTypes[(Low(TestTypes)+1)].Name) then
      begin
        SetActiveTestType(TestTypes[(Low(TestTypes)+1)]);
      end;

      if (boardnumber=PCurrentMachineSet^.MachineNumber) then
      begin
        SourcePDODrawGrid.Invalidate;
        SourceEPRPDODrawGrid.Invalidate;
      end;
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
  BN:integer;
  LocalPMS:PMachineSet;
begin
  BN:=(Abs(datacarrier));

  if BN<>0 then
  begin
    if datacarrier>0 then Info:='Channel #'+InttoStr(BN-1)+' board added.';
    if datacarrier<0 then Info:='Channel #'+InttoStr(BN-1)+' board removed.';

    LocalPMS:=@MachineSet[BN];
    LocalPMS^.LatestMeasurementData:=Default(TMachineData);
    LocalPMS^.ActiveTestType:=Default(TTestType);
    LocalPMS^.RunData.Clear;
    LocalPMS^.BoardAvailable:=(datacarrier>0);

    ImageList1.BeginUpdate;
    if LocalPMS^.BoardAvailable then
    begin
      LocalPMS^.MachineNumber:=BN;
      ReplaceImageFromResource(ImageList1,{Channel=Index=}BN-1,'OK_02_32')
    end
    else
    begin
      LocalPMS^.MachineNumber:=0;
      ReplaceImageFromResource(ImageList1,{Channel=Index=}BN-1,'DELETE_01_32');
    end;
    ImageList1.EndUpdate;

    TabControl1.Invalidate;

    USBDebugLog.Lines.Append(Info);
    Info:=DD.Errors;
    if (Length(Info)>0) then USBDebugLog.Lines.Append(Info);
    Info:=DD.Info;
    if (Length(Info)>0) then USBDebugLog.Lines.Append(Info);

    if (NOT LocalPMS^.BoardAvailable) then
    begin
      LocalPMS^.DUT.Cleanup;
      if (LocalPMS^.MachineNumber = PCurrentMachineSet^.MachineNumber) then
      begin
        SourcePDODrawGrid.Invalidate;
        SourceEPRPDODrawGrid.Invalidate;
      end;
    end;

    OverviewDrawGrid.InvalidateRow(BN);
  end;
  if BN=0 then USBDebugLog.Lines.Append('Unknown board error');
  if BN=0 then Info:='Unknown data acquisitionboard error';

  {$ifdef MSWINDOWS}
  SplashForm:=TSplashForm.Create(application);
  Splashform.Label2.Caption:='Board '+InttoStr(BN)+' change';
  SplashForm.Update;
  //Splashform.Label3.Caption:='Init ready';
  SplashForm.Update;
  //Application.ProcessMessages;
  sleep(500);
  if Assigned(SplashForm) then SplashForm.Destroy;
  SplashForm:=nil;
  //Application.ProcessMessages;
  {$endif}
end;

procedure TPowerbankMainForm.GraphBaseClick(Sender: TObject);
var
  x:integer;
  MachineAsked:integer;
begin
  MachineAsked:=0;
  try
    if (Sender is TComponent) then MachineAsked:=(Sender as TComponent).Tag;
    if MachineAsked>0 then
    begin
      TabControl1.TabIndex:=(MachineAsked-1);
      PageControl1.TabIndex:=2;
    end;
  except
    on E: EInvalidOperation do begin end;
  end;
end;

procedure TPowerbankMainForm.SaveMachineData;
begin
  if (PCurrentMachineSet^.MachineNumber>0) then
  begin
    if (NOT PCurrentMachineSet^.Active) then with PCurrentMachineSet^ do
    begin
      Project:=Project_edit.Text;
      Product:=Product_edit.Text;
      Runname:=Runname_edit.Text;
      Runnumber:=Runnumber_edit.Text;
      Info:=MachineInfoMemo.Lines.CommaText;
      //AutoFileName:=AutoFileNameCheckBox.Checked;
    end;
  end;
end;

procedure TPowerbankMainForm.SetControls(Sender: TObject);
var
  MachineIdle:boolean;
begin
  if PCurrentMachineSet^.Active then
  begin
    if (Sender<>StartStopSpeedButton) then
    begin
      StartStopSpeedButton.Down:=true;
      StartStopSpeedButton.Enabled:=true;
    end;
    StartStopSpeedButton.Caption:='Stop';
    StartStopSpeedButton.Font.Color:=clRed;
  end
  else
  begin
    if (Sender<>StartStopSpeedButton) then
    begin
      StartStopSpeedButton.Down:=false;
      if (NOT PCurrentMachineSet^.Testing) then
      begin
        // StartStop might be enabled
        // But only if we have valid test-data-fields
        EditChangeEmptyCheck(Sender);
      end else StartStopSpeedButton.Enabled:=false;
    end;
    StartStopSpeedButton.Caption:='Start';
    StartStopSpeedButton.Font.Color:=clLime;
  end;

  if PCurrentMachineSet^.Testing then
  begin
    if (Sender<>btnTestDischarge) then
    begin
      btnTestDischarge.Down:=true;
      btnTestDischarge.Enabled:=true;
    end;
    btnTestDischarge.Caption:='Stop Testing';
    btnTestDischarge.Font.Color:=clRed;
  end
  else
  begin
    if (Sender<>btnTestDischarge) then
    begin
      btnTestDischarge.Down:=false;
      btnTestDischarge.Enabled:=(NOT PCurrentMachineSet^.Active);
    end;
    btnTestDischarge.Caption:='Start Testing';
    btnTestDischarge.Font.Color:=clLime;
  end;

  MachineIdle:=(NOT (PCurrentMachineSet^.Active OR PCurrentMachineSet^.Testing));

  //FilenameEdit.Enabled:=MachineIdle;
  Project_edit.Enabled:=MachineIdle;
  Product_edit.Enabled:=MachineIdle;
  Runname_edit.Enabled:=MachineIdle;
  Runnumber_edit.Enabled:=MachineIdle;
  MachineInfoMemo.Enabled:=MachineIdle;

  TypesBox.Enabled:=MachineIdle;
  SamplesBox.Enabled:=MachineIdle;
  TestsBox.Enabled:=MachineIdle;

  CurrentEdit.Enabled:=MachineIdle;
  VoltageEdit.Enabled:=MachineIdle;
  chkCurrentLimit.Enabled:=MachineIdle;
  chkVoltageLimit.Enabled:=MachineIdle;

  MachineIdle:=(NOT PCurrentMachineSet^.Active);

  btnRotoPDGetPDOList.Enabled:=MachineIdle;
  Button2.Enabled:=MachineIdle;
  SourcePDODrawGrid.Enabled:=MachineIdle;
  SourceEPRPDODrawGrid.Enabled:=MachineIdle;
end;

end.

