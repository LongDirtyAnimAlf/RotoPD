unit processframe_simple;

{$mode ObjFPC}{$H+}

interface

uses
  Classes, SysUtils, Forms, Controls, StdCtrls, ExtCtrls, Graphics,
  TAGraph, TASeries, TATransformations, TAIntervalSources,
  Gauge,
  SpinEx, BGRAKnob, BGRAShape, BGRAImageList, SuperGaugeCommon, SuperGauge,
  dlLEDCtrl;

type

  { TFrameSimple }

  TFrameSimple = class(TFrame)
    DateTimeIntervalChartSource2: TDateTimeIntervalChartSource;
    MT1: TAutoScaleAxisTransform;
    MyTransform1: TChartAxisTransformations;
    MyTransform2: TChartAxisTransformations;
    MyTransform2AutoScaleAxisTransform1: TAutoScaleAxisTransform;
    Panel1: TPanel;
    Panel2: TPanel;
    EnergySeries: TLineSeries;
    panelVoltage: TPanel;
    panelCurrent: TPanel;
    WaterSeries1: TLineSeries;
    WaterSeries2: TLineSeries;
    TemperatureSeries: TLineSeries;
    procedure GraphBaseBeforeDrawBackground(ASender: TChart; ACanvas: TCanvas;
      const ARect: TRect; var ADoDefaultDrawing: Boolean);
    procedure GraphBaseBeforeDrawBackWall(ASender: TChart; ACanvas: TCanvas;
      const ARect: TRect; var ADoDefaultDrawing: Boolean);
    procedure Panel1Resize(Sender: TObject);
    procedure Panel2Resize(Sender: TObject);
  private
    fEnabled                  : boolean;
    fNewVoltageDisplay        : TLEDDisplay;
    fNewCurrentDisplay        : TLEDDisplay;
    //fPowerGauge               : TGauge;
    fNewEnergyDisplay         : TLEDDisplay;

    fPowerGauge            : TSuperGauge;

    procedure OnGaugeText(Sender: TObject; value: double; var labeltext: string);
    procedure SetVoltage(AValue: single);
    procedure SetCurrent(AValue: single);
    procedure SetPower(AValue: single);
    procedure SetEnergy(AValue: single);
    procedure SetEnable(value:boolean);
    procedure SetNumber(value:integer);
    function GetNumber:integer;
    procedure SetTitle(value:string);
  public
    constructor Create(TheOwner: TComponent); override;
    procedure SetClick(Handler: TNotifyEvent);
    procedure SaveAsBitMap(aWidth,aHeight:integer);
    property Enable:boolean read fEnabled write SetEnable;
    property Number:integer read GetNumber write SetNumber;
    property Title:string write SetTitle;
    property Voltage:single write SetVoltage;
    property Current:single write SetCurrent;
    property Power:single write SetPower;
    property Energy:single write SetEnergy;
  end;
  PFrameSimple = ^TFrameSimple;

implementation

{$R *.lfm}

uses
  FPimage,
  FPReadBMP, FPReadJPEG, FPReadPNG, FPReadGif, FPReadTiff,
  FPWriteBMP, FPWriteJPEG,
  DateUtils,
  printform;

const
  //IBUSCOLOR                    = TColor($4040FF);
  //VBUSCOLOR                    = TColor($80FF80);
  //ENERGYCOLOR                  = clYellow;
  //POWERCOLOR                   = clFuchsia;
  IBUSCOLOR                    = clRed;
  VBUSCOLOR                    = clRed;
  ENERGYCOLOR                  = clRed;
  POWERCOLOR                   = clRed;
  ACTIVECOLOR                  = TColor($E0F0FF); // clMenuHighlight
  //INACTIVECOLOR                = TColor($404040); // clDefault
  INACTIVECOLOR                = clLtGray;


{ TFrameSimple }

procedure TFrameSimple.OnGaugeText(Sender: TObject; value: double; var labeltext: string);
begin
  labeltext := labeltext + ' Watt';
end;

constructor TFrameSimple.Create(TheOwner: TComponent);
begin
  inherited Create(TheOwner);
  Tag:=-1;
  fEnabled:=false;

  fPowerGauge:=TSuperGauge.Create(Panel2);
  fPowerGauge.Parent:=Panel2;
  with fPowerGauge do
  begin
    Align := alClient;
    MinValue := 0;
    MaxValue := 240;

    AutoScale := True;

    ScaleSettings.MainTickCount:=12;
    ScaleSettings.Step:=20;

    BandSettings1.Enabled := True;
    BandSettings1.EndValue := round(100*60/MaxValue);
    BandSettings1.EnableText := True;
    BandSettings1.Text := 'BASIC';
    BandSettings1.TextFont := 'default';
    BandSettings1.TextStyle := [fsBold];
    BandSettings1.TextColor := clWhite;
    BandSettings2.Enabled := True;
    BandSettings2.StartValue := round(100*60/MaxValue);
    BandSettings2.EndValue := round(100*100/MaxValue);
    BandSettings2.EnableText := True;
    BandSettings2.Text := 'PD';
    BandSettings2.TextFont := 'default';
    BandSettings2.TextStyle := [fsBold];
    BandSettings2.TextColor := clBlack;
    BandSettings2.BandColor := clYellow;
    BandSettings3.Enabled := True;
    BandSettings3.StartValue := round(100*100/MaxValue);
    BandSettings3.EnableText := True;
    BandSettings3.Text := 'EPR';
    BandSettings3.TextFont := 'default';
    BandSettings3.TextStyle := [fsBold];
    BandSettings3.TextColor := clWhite;
    BandSettings3.BandColor := clRed;
    BandSettings4.Text := 'Band 4';
    BandSettings4.TextSize := 18;
    BandSettings4.TextFont := 'default';
    BandSettings4.TextStyle := [fsBold];
    BandSettings4.TextRadius := 100;
    BandSettings4.TextColor := clBlack;
    BandSettings4.BandRadius := 90;
    TextSettings1.Enabled := True;
    TextSettings1.FontEx.Color := clWhite;
    //TextSettings1.FontEx.FontQuality := fqSystemClearType;
    TextSettings1.FontEx.Height := 24;
    TextSettings1.FontEx.Shadow := False;
    TextSettings1.FontEx.ShadowRadius := 5;
    TextSettings1.FontEx.ShadowOffsetX := 5;
    TextSettings1.FontEx.ShadowOffsetY := 5;
    TextSettings1.FontEx.Style := [fsBold];
    TextSettings1.Text := 'Watt';
    TextSettings1.OffsetX := 0;
    TextSettings1.OffsetY := -35;
    TextSettings2.Enabled := True;
    TextSettings2.FontEx.Color := 4227327;
    //TextSettings2.FontEx.FontQuality := fqSystemClearType;
    TextSettings2.FontEx.Height := 24;
    TextSettings2.FontEx.Shadow := False;
    TextSettings2.FontEx.ShadowRadius := 5;
    TextSettings2.FontEx.ShadowOffsetX := 5;
    TextSettings2.FontEx.ShadowOffsetY := 5;
    TextSettings2.FontEx.Style := [fsBold];
    TextSettings2.Text := '--';
    TextSettings2.OffsetX := 0;
    TextSettings2.OffsetY := 40;
    TextSettings3.FontEx.Color := clWhite;
    //TextSettings3.FontEx.FontQuality := fqSystemClearType;
    TextSettings3.FontEx.Height := 16;
    TextSettings3.FontEx.Shadow := False;
    TextSettings3.FontEx.ShadowRadius := 5;
    TextSettings3.FontEx.ShadowOffsetX := 5;
    TextSettings3.FontEx.ShadowOffsetY := 5;
    TextSettings3.FontEx.Style := [fsBold];
    TextSettings3.Text := 'x100';
    TextSettings3.OffsetX := 0;
    TextSettings3.OffsetY := 60;
    FrameSettings.OuterFrameColor := clGray;
    FrameSettings.MiddleFrameColor := clSilver;
    FrameSettings.InnerFrameColor := clMedGray;
    FrameSettings.OuterFrameThickness := 5;
    FrameSettings.MiddleFrameThickness := 5;
    FrameSettings.InnerFrameThickness := 5;
    ScaleSettings.TickColor := 8242399;
    ScaleSettings.TextColor := clSkyBlue;
    ScaleSettings.TextSize := 20;
    ScaleSettings.TextFont := 'default';
    ScaleSettings.TickArcColor := 8242399;
    ScaleSettings.Enabled := True;
    AuxScaleSettings.TickColor := 8242399;
    AuxScaleSettings.TextColor := 13881484;
    AuxScaleSettings.TextSize := 20;
    AuxScaleSettings.TextFont := 'default';
    AuxScaleSettings.TickArcColor := 8242399;
    AuxScaleSettings.Enabled := False;
    RangeLEDSettings.ActiveColor := clRed;
    RangeLEDSettings.InactiveColor := clBlack;
    RangeLEDSettings.BorderColor := clGray;
    RangeLEDSettings.Style := lsShaded;
    RangeLEDSettings.RangeStartValue := 55;
    RangeLEDSettings.RangeType := rcGreaterStart;
    MarkerSettings1.Enabled := True;
    MarkerSettings1.Color := clLime;
    MarkerSettings1.Style := msRight;
    MarkerSettings2.Enabled := True;
    MarkerSettings2.Color := clRed;
    MarkerSettings2.Style := msLeft;
    MarkerSettings3.Enabled := True;
    MarkerSettings3.Color := clYellow;
    MarkerSettings3.Height := 15;
    MarkerSettings3.Radius := 135;
    MarkerSettings3.Width := 5;
    PointerSettings.Color := 4227327;
    PointerSettings.Enabled := True;
    AuxPointerSettings.Color := clRed;
    AuxPointerSettings.Length := 100;
    AuxPointerSettings.ExtensionLength := 5;
    AuxPointerSettings.Width := 7;
    AuxPointerSettings.Enabled := False;
    PointerCapSettings.Radius := 20;
    //OverloadTriggered := SuperGaugeOverloadTriggered;
    //OverloadRecovered := SuperGaugeOverloadRecovered;
    //AuxOverloadTriggered := SuperGaugeAuxOverloadTriggered;
    //AuxOverloadRecovered := SuperGaugeAuxOverloadRecovered;
    //RangeLEDActive := SuperGaugeRangeLEDActive;
    //RangeLEDInactive := SuperGaugeRangeLEDInactive;
















  end;

  (*
  fPowerGauge:=TGauge.Create(Panel2);
  fPowerGauge.Parent:=Panel2;
  fPowerGauge.OnGetText:=@OnGaugeText;

  with fPowerGauge do
  begin
    Align := alClient;
    Max := 240;
    StartAngle := -140;
    SweepAngle := 280;
    MajorTicks := 13;
    NeedleColor:=POWERCOLOR;
    with Zones.Add do
    begin
      FromValue := 0;
      ToValue := 60;
      Color := clLime;
    end;
    with Zones.Add do
    begin
      FromValue := 61;
      ToValue := 140;
      Color := TColor($008CFF);
    end;
    with Zones.Add do
    begin
      FromValue := 141;
      ToValue := 240;
      Color := clRed;
    end;

    //Font.CharSet := ANSI_CHARSET;
    Font.Height := -19;
    Font.Pitch := fpVariable;
    Font.Quality := fqDraft;
    Font.Style := [fsBold];
    ParentFont := False;

    ValueFormat := '%.0f';
    LabelFormat := '%.0f';
  end;
  with fPowerGauge.Zones.Add do
  begin
    FromValue := 140;
    ToValue := 240;
  end;
  *)
  fNewVoltageDisplay := TLEDDisplay.Create(panelVoltage);
  with fNewVoltageDisplay do begin
    Parent := panelVoltage;
    Align := alClient;
    ColorLED:=VBUSCOLOR;
    NumDigits := 5;
    LeadingZeros := false;
    LEDContrast := 7;
    DigitLineWidth := 4;
    Borderstyle := bsNone;
    BevelStyle := bvLowered;
    Angle := 0;
    Clear;
    Value:=0;
    FractionDigits:=3;
    Hint:='Voltage [V]';
    ShowHint:=True;
  end;

  fNewCurrentDisplay := TLEDDisplay.Create(panelCurrent);
  with fNewCurrentDisplay do begin
    Parent := panelCurrent;
    Align := alClient;
    ColorLED:=IBUSCOLOR;
    NumDigits := 5;
    LeadingZeros := false;
    LEDContrast := 7;
    DigitLineWidth := 4;
    Borderstyle := bsNone;
    BevelStyle := bvLowered;
    Angle := 0;
    Clear;
    Value:=0;
    FractionDigits:=3;
    Hint:='Current [A]';
    ShowHint:=True;
  end;


  fNewEnergyDisplay := TLEDDisplay.Create(fPowerGauge);
  with fNewEnergyDisplay do begin
    Parent := fPowerGauge;
    //Align := alBottom;
    ColorLED:=ENERGYCOLOR;
    NumDigits := 5;
    LeadingZeros := false;
    LEDContrast := 7;
    DigitLineWidth := 4;
    //DigitHeight := Parent.Height;
    DigitWidth := 30;
    Borderstyle := bsNone;
    BevelStyle := bvLowered;
    Angle := 0;
    Clear;
    Value:=0;
    FractionDigits:=3;
    Hint:='Energy [WH]';
    ShowHint:=True;
  end;

  Enable:=false;
end;

procedure TFrameSimple.SaveAsBitMap(aWidth,aHeight:integer);
var
  LocalForm:TLocalPrintForm;
  LocalFrame:TFrameSimple;
begin
  //LocalForm:=TLocalPrintForm.CreateNew(nil,1);
  LocalForm:=TLocalPrintForm.Create(Self);
  LocalForm.Name:='SohitPrintForm';

  LocalForm.Width:=aWidth;
  LocalForm.Height:=aHeight;

  LocalFrame:=TFrameSimple.Create(LocalForm);
  LocalFrame.Parent:=LocalForm;
  LocalFrame.Align:=alClient;

  //LocalForm.Show;

  //LocalForm.Repaint;

  // Does not work.
  // Only works when form is shown.
  with LocalForm.GetFormImage do
  begin
    SaveToFile('test2.bmp');
    Free;
  end;

  LocalForm.Free;
end;

procedure TFrameSimple.GraphBaseBeforeDrawBackground(ASender: TChart;
  ACanvas: TCanvas; const ARect: TRect; var ADoDefaultDrawing: Boolean);
begin
  if fEnabled then
    ACanvas.GradientFill(ARect, clYellow, clWhite, gdVertical)
  else
    ACanvas.Rectangle(ARect);
  ADoDefaultDrawing := false;
end;

procedure TFrameSimple.GraphBaseBeforeDrawBackWall(ASender: TChart;
  ACanvas: TCanvas; const ARect: TRect; var ADoDefaultDrawing: Boolean);
begin
  ACanvas.Pen.Width:=1;
  ACanvas.Pen.Color:=clBlack;
  ACanvas.Frame(ARect);
  ADoDefaultDrawing:=false;
end;

procedure TFrameSimple.Panel1Resize(Sender: TObject);
begin
  with fNewCurrentDisplay do begin
    DigitHeight := Parent.Height - 4;
    DigitWidth := ((Parent.Width - 60) div NumDigits) + 8;
    DigitLineWidth := (DigitWidth div 8) + 2;
  end;
  with fNewVoltageDisplay do begin
    DigitHeight := Parent.Height - 4;
    DigitWidth := ((Parent.Width - 60) div NumDigits) + 8;
    DigitLineWidth := (DigitWidth div 8) + 2;
  end;
end;

procedure TFrameSimple.Panel2Resize(Sender: TObject);
var
  P:TPanel;
begin
  P:=TPanel(Sender);
  fNewEnergyDisplay.Top:=P.Height- fNewEnergyDisplay.Height - (P.BevelWidth*2) - 10;
  fNewEnergyDisplay.Left:=((P.Width - (P.BevelWidth*4)) DIV 2)-((fNewEnergyDisplay.Width + 2) DIV 2);
end;

procedure TFrameSimple.SetEnable(value:boolean);
begin
  if (value<>fEnabled) then
  begin
    fEnabled:=value;
    fNewVoltageDisplay.Enabled:=value;
    fNewCurrentDisplay.Enabled:=value;
    fPowerGauge.Enabled:=value;
    fNewEnergyDisplay.Enabled:=value;
    Panel1.Enabled:=value;
    Panel2.Enabled:=value;
    if value then
    begin
      //Panel1.Color:=clMenuHighlight;
      Panel1.Color:=ACTIVECOLOR;
      fPowerGauge.Color:=Panel1.Color;
    end
    else
    begin
      //Panel1.Color:=clGray;
      Panel1.Color:=INACTIVECOLOR;
      fPowerGauge.Color:=Panel1.Color;
    end;
  end;
end;

procedure TFrameSimple.SetCurrent(AValue: single);
begin
  fNewCurrentDisplay.Value:=AValue;
end;

procedure TFrameSimple.SetEnergy(AValue: single);
begin
  fNewEnergyDisplay.Value:=AValue;
end;

procedure TFrameSimple.SetNumber(value:integer);
begin
  if (value<>Tag) then
  begin
    Name:='NewFrame'+InttoStr(value);
    // Set all tags to identify [machine]number when clicked anywhere
    Tag:=value;
    Panel1.Tag:=value;
    Panel2.Tag:=value;
    fNewVoltageDisplay.Tag:=value;
    fNewCurrentDisplay.Tag:=value;
    fPowerGauge.Tag:=value;
    fNewEnergyDisplay.Tag:=value;
  end;
end;

function TFrameSimple.GetNumber:integer;
begin
  result:=Tag;
end;

procedure TFrameSimple.SetPower(AValue: single);
begin
  fPowerGauge.Value:=AValue;
end;

procedure TFrameSimple.SetTitle(value:string);
begin
end;

procedure TFrameSimple.SetVoltage(AValue: single);
begin
  fNewVoltageDisplay.Value:=AValue;
end;

procedure TFrameSimple.SetClick(Handler: TNotifyEvent);
begin
  fNewEnergyDisplay.OnClick := Handler;
  fPowerGauge.OnClick := Handler;
  fNewVoltageDisplay.OnClick := Handler;
  fNewCurrentDisplay.OnClick := Handler;
end;

end.

