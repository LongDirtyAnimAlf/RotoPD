unit processframe_simple;

{$mode ObjFPC}{$H+}

interface

uses
  Classes, SysUtils, Forms, Controls, StdCtrls, ExtCtrls, Graphics,
  TAGraph, TASeries, TATransformations, TAIntervalSources;

type

  { TFrameSimple }

  TFrameSimple = class(TFrame)
    DateTimeIntervalChartSource2: TDateTimeIntervalChartSource;
    GraphBase: TChart;
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
  private
    fEnabled:boolean;
    procedure SetEnable(value:boolean);
    procedure SetNumber(value:integer);
    function GetNumber:integer;
    procedure SetTitle(value:string);
  public
    constructor Create(TheOwner: TComponent); override;
    procedure SaveAsBitMap(aWidth,aHeight:integer);
    property Enable:boolean read fEnabled write SetEnable;
    property Number:integer read GetNumber write SetNumber;
    property Title:string write SetTitle;
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

procedure TFrameSimple.SetEnable(value:boolean);
begin
  if (value<>fEnabled) then
  begin
    fEnabled:=value;
    Water1Base.Enabled:=value;
    Water2Base.Enabled:=value;
    EnergyBase.Enabled:=value;
    TemperatureBase.Enabled:=value;
    GraphBase.Enabled:=value;
    Panel1.Enabled:=value;
    Panel2.Enabled:=value;
    if value then
    begin
      Panel1.Color:=clMenuHighlight;
      GraphBase.Title.Brush.Color:=clLime;
    end
    else
    begin
      Panel1.Color:=clGray;
      GraphBase.Title.Brush.Color:=clGray;
    end;
  end;
end;

procedure TFrameSimple.SetNumber(value:integer);
begin
  if (value<>Tag) then
  begin
    Name:='NewFrame'+InttoStr(value);
    // Set all tags to identify [machine]number when clicked anywhere
    Tag:=value;
    GraphBase.Tag:=value;
    Panel1.Tag:=value;
    Panel2.Tag:=value;
    TemperatureBase.Tag:=value;
    Water2Base.Tag:=value;
    Water1Base.Tag:=value;
    EnergyBase.Tag:=value;
  end;
end;

function TFrameSimple.GetNumber:integer;
begin
  result:=Tag;
end;

procedure TFrameSimple.SetTitle(value:string);
begin
  GraphBase.Title.Text.Text:=value;
  {
  GraphBase.Title.Text.Clear;
  if length(value)>22
     then GraphBase.Title.Text.CommaText:=value
     else GraphBase.Title.Caption:=value;
  }
end;


end.

