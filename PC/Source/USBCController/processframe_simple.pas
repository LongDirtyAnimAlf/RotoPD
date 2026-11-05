unit processframe_simple;

{$mode ObjFPC}{$H+}

interface

uses
  Classes, SysUtils, Forms, Controls, StdCtrls, ExtCtrls, Graphics,
  TAGraph, TASeries, TATransformations, TAIntervalSources,
  Gauge,
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
    TemperatureBase: TEdit;
    Water2Base: TEdit;
    Water1Base: TEdit;
    EnergyBase: TEdit;
    EnergySeries: TLineSeries;
    WaterSeries1: TLineSeries;
    WaterSeries2: TLineSeries;
    TemperatureSeries: TLineSeries;
    procedure GraphBaseBeforeDrawBackground(ASender: TChart; ACanvas: TCanvas;
      const ARect: TRect; var ADoDefaultDrawing: Boolean);
    procedure GraphBaseBeforeDrawBackWall(ASender: TChart; ACanvas: TCanvas;
      const ARect: TRect; var ADoDefaultDrawing: Boolean);
    procedure Panel2Resize(Sender: TObject);
  private
    fEnabled                  : boolean;
    fPowerGauge               : TGauge;
    fNewEnergyDisplay         : TLEDDisplay;
    procedure SetEnable(value:boolean);
    procedure SetEnergy(AValue: single);
    procedure SetNumber(value:integer);
    function GetNumber:integer;
    procedure SetPower(AValue: single);
    procedure SetTitle(value:string);
  public
    constructor Create(TheOwner: TComponent); override;
    procedure SetClick(Handler: TNotifyEvent);
    procedure SaveAsBitMap(aWidth,aHeight:integer);
    property Enable:boolean read fEnabled write SetEnable;
    property Number:integer read GetNumber write SetNumber;
    property Title:string write SetTitle;
    property Power:single write SetPower;
    property Energy:single write SetEnergy;
  end;

implementation

{$R *.lfm}

uses
  FPimage,
  FPReadBMP, FPReadJPEG, FPReadPNG, FPReadGif, FPReadTiff,
  FPWriteBMP, FPWriteJPEG,
  DateUtils,
  printform;

{ TFrameSimple }

constructor TFrameSimple.Create(TheOwner: TComponent);
begin
  inherited Create(TheOwner);
  Tag:=-1;
  fEnabled:=false;

  fPowerGauge:=TGauge.Create(Panel2);
  fPowerGauge.Parent:=Panel2;

  with fPowerGauge do
  begin
    Align := alClient;
    Max := 240;
    StartAngle := -140;
    SweepAngle := 280;
    MajorTicks := 13;
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
      Color := clYellow;
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

  fNewEnergyDisplay := TLEDDisplay.Create(fPowerGauge);
  with fNewEnergyDisplay do begin
    Parent := fPowerGauge;
    //Align := alBottom;
    ColorLED:=clRed;
    NumDigits := 5;
    LeadingZeros := false;
    LEDContrast := 7;
    //DigitLineWidth := 8;
    //DigitHeight := Parent.Height;
    //DigitWidth := DigitHeight * 2 div 4 - 1;
    Borderstyle := bsNone;
    BevelStyle := bvLowered;
    Angle := 0;
    Clear;
    Value:=0;
    FractionDigits:=3;
    Hint:='Energy [WH]';
    ShowHint:=True;
  end;
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
    Water1Base.Enabled:=value;
    Water2Base.Enabled:=value;
    EnergyBase.Enabled:=value;
    TemperatureBase.Enabled:=value;
    Panel1.Enabled:=value;
    Panel2.Enabled:=value;
    if value then
    begin
      Panel1.Color:=clMenuHighlight;
    end
    else
    begin
      Panel1.Color:=clGray;
    end;
  end;
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
    TemperatureBase.Tag:=value;
    Water2Base.Tag:=value;
    Water1Base.Tag:=value;
    EnergyBase.Tag:=value;
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

procedure TFrameSimple.SetClick(Handler: TNotifyEvent);
begin
  fNewEnergyDisplay.OnClick := Handler;
  fPowerGauge.OnClick := Handler;
  TemperatureBase.OnClick := Handler;
  EnergyBase.OnClick := Handler;
  Water1Base.OnClick := Handler;
  Water2Base.OnClick := Handler;
end;


end.

