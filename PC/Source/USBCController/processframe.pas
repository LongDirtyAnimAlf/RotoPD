unit processframe;

{$mode ObjFPC}{$H+}

interface

uses
  Classes, SysUtils, Forms, Controls, StdCtrls, ExtCtrls, Graphics,
  TAGraph, TASeries, TATransformations;

type

  { TFrame1 }

  TFrame1 = class(TFrame)
    MachineInfoMemo: TMemo;
    MT1: TAutoScaleAxisTransform;
    MyTransform1: TChartAxisTransformations;
    MyTransform2: TChartAxisTransformations;
    MyTransform2AutoScaleAxisTransform1: TAutoScaleAxisTransform;
    Project_edit: TEdit;
    FilenameEdit: TEdit;
    FileLabel: TLabel;
    InfoLabel: TLabel;
    Product_edit: TEdit;
    Run_edit: TEdit;
    RunLabel: TLabel;
    ProductLabel: TLabel;
    ProjectLabel: TLabel;
    Panel1: TPanel;
    MachineChart: TChart;
    TemperatureEditPeak1: TEdit;
    TimeEdit: TEdit;
    WaterEdit2: TEdit;
    WaterEdit1: TEdit;
    EnergyEdit: TEdit;
    EnergySeries: TLineSeries;
    WaterSeries1: TLineSeries;
    WaterSeries2: TLineSeries;
    TemperatureSeries: TLineSeries;
  private
  public
    procedure SaveAsBitMap(aWidth,aHeight:integer);
  end;

implementation

{$R *.lfm}

uses
  FPimage,
  FPReadBMP, FPReadJPEG, FPReadPNG, FPReadGif, FPReadTiff,
  FPWriteBMP, FPWriteJPEG,
  DateUtils,
  printform;

{ TFrame1 }

procedure TFrame1.SaveAsBitMap(aWidth,aHeight:integer);
var
  LocalForm:TLocalPrintForm;
  LocalFrame:TFrame1;
begin
  //LocalForm:=TLocalPrintForm.CreateNew(nil,1);
  LocalForm:=TLocalPrintForm.Create(Self);
  LocalForm.Name:='SohitPrintForm';

  LocalForm.Width:=aWidth;
  LocalForm.Height:=aHeight;

  LocalFrame:=TFrame1.Create(LocalForm);
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

end.

