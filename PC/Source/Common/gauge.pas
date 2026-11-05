{ Part of the Unleashed Pascal IDE
  Copyright (c) 2026 Unleashed Pascal Contributors <https://unleashedpascal.org>
  License: MPL-2.0

  This Source Code Form is subject to the terms of the Mozilla Public License,
  v. 2.0. If a copy of the MPL was not distributed with this file, You can
  obtain one at https://mozilla.org/MPL/2.0/.

  These notices must be kept in copies and modified versions (MPL-2.0 sec. 3.1);
  add yours below.

  Converted to standard Free Pascal (objfpc/delphi) syntax.
  Unleashed features removed: inline var, for-var-in, match, statement
  expressions, autofree, compound += .
}

unit Gauge;

{$mode objfpc}{$H+}

interface

uses
  Classes, SysUtils, Types, Controls, Graphics, ExtCtrls, ControlPaint;

type
  TGauge = class;

  { TGaugeZone }

  // a colored stretch of the scale, from one value to another
  TGaugeZone = class(TCollectionItem)
  private
    FFrom, FTo: double;
    FColor: TColor;
    procedure setFrom(value: double);
    procedure setTo(value: double);
    procedure setColor(value: TColor);
  protected
    function GetDisplayName: string; override;
  public
    constructor Create(ACollection: TCollection); override;
    procedure Assign(Source: TPersistent); override;
  published
    property FromValue: double read FFrom write setFrom;
    property ToValue: double read FTo write setTo;
    property Color: TColor read FColor write setColor default clRed;
  end;

  { TGaugeZones }

  TGaugeZones = class(TOwnedCollection)
  private
    function getItem(index: integer): TGaugeZone;
    procedure setItem(index: integer; value: TGaugeZone);
  protected
    procedure Update(Item: TCollectionItem); override;
  public
    constructor Create(newowner: TGauge);
    function Add: TGaugeZone;
    function AddZone(fromValue, toValue: double; color: TColor): TGaugeZone;
    property Items[index: integer]: TGaugeZone read getItem write setItem; default;
  end;

  TGaugeTextEvent = procedure(Sender: TObject; value: double; var text: string) of object;
  // the dial: a needle over the arc, the arc filled up to the value, or an
  // arc of segments lit up to the value
  TGaugeStyle = (gsDial, gsFilled, gsSegments);

  { TGauge }

  // a dial: an arc scale from StartAngle through SweepAngle, colored zones
  // on it, a needle at the value and the number below. The needle glides to
  // a new value when Animated
  TGauge = class(TCustomControl)
  private
    FMin, FMax, FValue, FShown: double;
    FStartAngle, FSweepAngle, FArcWidth, FMajorTicks, FMinorTicks: integer;
    FZones: TGaugeZones;
    FShowNeedle, FShowValue, FShowLabels, FAnimated, FShowArc: boolean;
    FValueFormat, FLabelFormat: string;
    FArcColor, FNeedleColor, FTickColor, FTextColor: TColor;
    FTimer: TTimer;
    FAntialiasing: TAntialiasSetting;
    FStyle: TGaugeStyle;
    FLanguage: TFormatLanguage;
    FOnGetText: TGaugeTextEvent;
    procedure setRange(index: integer; value: double);
    function getRange(index: integer): double;
    procedure setMetric(index: integer; value: integer);
    function getMetric(index: integer): integer;
    procedure setFlag(index: integer; value: boolean);
    function getFlag(index: integer): boolean;
    procedure setZones(value: TGaugeZones);
    procedure setText(index: integer; const value: string);
    function getText(index: integer): string;
    procedure setPartColor(index: integer; value: TColor);
    function getPartColor(index: integer): TColor;
    procedure setAntialiasing(value: TAntialiasSetting);
    procedure setStyle(value: TGaugeStyle);
    procedure setLanguage(value: TFormatLanguage);
    function fraction(v: double): double;
    function angleOf(v: double): double;
    procedure tick(Sender: TObject);
    procedure syncTimer;
    procedure zonesChanged;
    function valueStored(index: integer): boolean;
  protected
    procedure Paint; override;
    class function GetControlClassDefaultSize: TSize; override;
    procedure Loaded; override;
  public
    constructor Create(AOwner: TComponent); override;
    destructor Destroy; override;
  published
    property Min: double index 0 read getRange write setRange stored valueStored;
    property Max: double index 1 read getRange write setRange stored valueStored;
    property Value: double index 2 read getRange write setRange stored valueStored;
    // degrees clockwise from the top where Min sits
    property StartAngle: integer index 0 read getMetric write setMetric default -120;
    property SweepAngle: integer index 1 read getMetric write setMetric default 240;
    property ArcWidth: integer index 2 read getMetric write setMetric default 10;
    // ticks with labels along the scale; 0 for none
    property MajorTicks: integer index 3 read getMetric write setMetric default 5;
    // ticks between two major ones
    property MinorTicks: integer index 4 read getMetric write setMetric default 4;
    property Zones: TGaugeZones read FZones write setZones;
    property ShowArc: boolean index 0 read getFlag write setFlag default true;
    property ShowNeedle: boolean index 1 read getFlag write setFlag default true;
    property ShowValue: boolean index 2 read getFlag write setFlag default true;
    property ShowLabels: boolean index 3 read getFlag write setFlag default true;
    property Animated: boolean index 4 read getFlag write setFlag default true;
    // Format patterns with the value as a float argument
    property ValueFormat: string index 0 read getText write setText;
    property LabelFormat: string index 1 read getText write setText;
    // clDefault takes the colors of the theme
    property ArcColor: TColor index 0 read getPartColor write setPartColor default clDefault;
    property NeedleColor: TColor index 1 read getPartColor write setPartColor default clDefault;
    property TickColor: TColor index 2 read getPartColor write setPartColor default clDefault;
    property TextColor: TColor index 3 read getPartColor write setPartColor default clDefault;
    property Antialiasing: TAntialiasSetting read FAntialiasing write setAntialiasing default asTheme;
    property Style: TGaugeStyle read FStyle write setStyle default gsDial;
    // the names and separators of dates, times and numbers: those of the
    // locale of the system, or English
    property Language: TFormatLanguage read FLanguage write setLanguage default flSystem;
    property OnGetText: TGaugeTextEvent read FOnGetText write FOnGetText;
    property Align;
    property Anchors;
    property BorderSpacing;
    property Color;
    property Constraints;
    property Enabled;
    property Font;
    property Hint;
    property ParentColor;
    property ParentFont;
    property ParentShowHint;
    property PopupMenu;
    property ShowHint;
    property Visible;
    property OnClick;
    property OnContextPopup;
    property OnDblClick;
    property OnMouseDown;
    property OnMouseEnter;
    property OnMouseLeave;
    property OnMouseMove;
    property OnMouseUp;
    property OnResize;
  end;

implementation

uses
  Math;

{ TGaugeZone }

constructor TGaugeZone.Create(ACollection: TCollection);
begin
  inherited Create(ACollection);
  FColor := clRed;
end;

procedure TGaugeZone.Assign(Source: TPersistent);
begin
  if Source is TGaugeZone then begin
    FFrom := TGaugeZone(Source).FFrom;
    FTo := TGaugeZone(Source).FTo;
    FColor := TGaugeZone(Source).FColor;
    Changed(False);
  end else inherited Assign(Source);
end;

function TGaugeZone.GetDisplayName: string;
begin
  Result := Format('%g .. %g', [FFrom, FTo]);
end;

procedure TGaugeZone.setFrom(value: double);
begin
  if FFrom = value then Exit;
  FFrom := value;
  Changed(False);
end;

procedure TGaugeZone.setTo(value: double);
begin
  if FTo = value then Exit;
  FTo := value;
  Changed(False);
end;

procedure TGaugeZone.setColor(value: TColor);
begin
  if FColor = value then Exit;
  FColor := value;
  Changed(False);
end;

{ TGaugeZones }

constructor TGaugeZones.Create(newowner: TGauge);
begin
  inherited Create(newowner, TGaugeZone);
end;

function TGaugeZones.getItem(index: integer): TGaugeZone;
begin
  Result := TGaugeZone(inherited Items[index]);
end;

procedure TGaugeZones.setItem(index: integer; value: TGaugeZone);
begin
  inherited Items[index] := value;
end;

function TGaugeZones.Add: TGaugeZone;
begin
  Result := TGaugeZone(inherited Add);
end;

function TGaugeZones.AddZone(fromValue, toValue: double; color: TColor): TGaugeZone;
begin
  Result := Add;
  Result.FFrom := fromValue;
  Result.FTo := toValue;
  Result.FColor := color;
  Result.Changed(False);
end;

procedure TGaugeZones.Update(Item: TCollectionItem);
begin
  inherited Update(Item);
  if Owner is TGauge then TGauge(Owner).zonesChanged;
end;

{ TGauge }

constructor TGauge.Create(AOwner: TComponent);
begin
  inherited Create(AOwner);
  ControlStyle := ControlStyle - [csOpaque, csSetCaption];
  FMax := 100;
  FStartAngle := -120;
  FSweepAngle := 240;
  FArcWidth := 10;
  FMajorTicks := 5;
  FMinorTicks := 4;
  FZones := TGaugeZones.Create(Self);
  FShowArc := True;
  FShowNeedle := True;
  FShowValue := True;
  FShowLabels := True;
  FAnimated := True;
  FValueFormat := '%.0f';
  FLabelFormat := '%.0f';
  FArcColor := clDefault;
  FNeedleColor := clDefault;
  FTickColor := clDefault;
  FTextColor := clDefault;
  FTimer := TTimer.Create(Self);
  FTimer.Enabled := False;
  FTimer.Interval := 30;
  FTimer.OnTimer := @tick;
  ParentColor := True;
  with GetControlClassDefaultSize do SetInitialBounds(0, 0, cx, cy);
end;

destructor TGauge.Destroy;
begin
  FTimer.Enabled := False;
  FZones.Free;
  inherited Destroy;
end;

class function TGauge.GetControlClassDefaultSize: TSize;
begin
  Result.cx := 120;
  Result.cy := 110;
end;

procedure TGauge.Loaded;
begin
  inherited Loaded;
  FShown := FValue;
end;

function TGauge.valueStored(index: integer): boolean;
begin
  Result := True;
end;

// -- geometry ----------------------

function TGauge.fraction(v: double): double;
begin
  if FMax > FMin then
    Result := EnsureRange((v - FMin) / (FMax - FMin), 0, 1)
  else
    Result := 0;
end;

// radians counterclockwise from 3 o'clock, as the canvas counts
function TGauge.angleOf(v: double): double;
begin
  Result := DegToRad(90 - (FStartAngle + fraction(v) * FSweepAngle));
end;

// -- painting ----------------------

procedure TGauge.Paint;
var
  clrArc, clrNeedle, clrTick, clrText, clrAccent: TColor;
  sp: TSmoothPaint;
  labelH, size, cx, cy, arcW, radius, startDeg, deg, parts, total, i, len: integer;
  box: TRect;
  z: TGaugeZone;
  a, sweep, v, side, hubR, hx, hy, tipR, tailR, tickOuter: double;
  major: boolean;
  valueText: string;
  fs: TFormatSettings;
  tw, th: integer;
  lr: double;
  vr: TRect;
  needlePts: array of TPointF;
begin
  clrArc := pick(FArcColor, blend(clBtnFace, clBtnShadow, 40));
  clrNeedle := pick(FNeedleColor, clWindowText);
  clrTick := pick(FTickColor, clGrayText);
  clrText := pick(FTextColor, clWindowText);
  if not Enabled then begin
    clrNeedle := ColorToRGB(clGrayText);
    clrText := ColorToRGB(clGrayText);
  end;
  sp := beginShapes(Canvas, ClientRect, GetRGBColorResolvingParent, FAntialiasing);
  try
    if FShowLabels then labelH := Canvas.TextHeight('0') else labelH := 0;
    size := Math.Min(ClientWidth - labelH * 2, ClientHeight - labelH);
    if size < 10 then Exit;
    cx := ClientWidth div 2;
    // the dial centers on the sweep: a half dial sits lower than a full one
    if FSweepAngle <= 200 then
      cy := (ClientHeight - labelH) div 2 + size div 5
    else
      cy := (ClientHeight - labelH) div 2;
    arcW := scaled(Self, FArcWidth);
    radius := size div 2 - arcW div 2 - 1;
    box := Rect(cx - radius, cy - radius, cx + radius, cy + radius);
    startDeg := 90 - FStartAngle;
    if FShowArc then begin
      paintArc(sp, box, startDeg, -FSweepAngle, clrArc, arcW, False);
      for i := 0 to FZones.Count - 1 do begin
        z := FZones[i];
        a := startDeg - Round(fraction(z.FromValue) * FSweepAngle);
        sweep := Round((fraction(z.ToValue) - fraction(z.FromValue)) * FSweepAngle);
        if sweep > 0 then
          paintArc(sp, box, Round(a), -Round(sweep), ColorToRGB(z.Color), arcW, False);
      end;
    end;
    // the value on the arc itself: a filled part or lit segments
    clrAccent := ColorToRGB(clHighlight);
    if not Enabled then clrAccent := blend(clrAccent, clBtnFace, 50);
    case FStyle of
      gsFilled: begin
        deg := Round(fraction(FShown) * FSweepAngle);
        if deg > 0 then
          paintArc(sp, box, startDeg, -deg, clrAccent, arcW, False);
      end;
      gsSegments: begin
        parts := Math.Max(FSweepAngle div 10, 8);
        for i := 0 to parts - 1 do begin
          if fraction(FShown) > i / parts then
            paintArc(sp, box, startDeg - Round(i * FSweepAngle / parts),
              -Round(FSweepAngle / parts * 0.7), clrAccent, arcW, False)
          else
            paintArc(sp, box, startDeg - Round(i * FSweepAngle / parts),
              -Round(FSweepAngle / parts * 0.7), clrArc, arcW, False);
        end;
      end;
    else
      ; // gsDial – needle drawn below
    end;
    // the ticks sit just inside the arc
    tickOuter := radius - arcW div 2 - scaled(Self, 2);
    if FMajorTicks > 1 then begin
      total := (FMajorTicks - 1) * (FMinorTicks + 1);
      for i := 0 to total do begin
        major := (i mod (FMinorTicks + 1)) = 0;
        v := FMin + (FMax - FMin) * i / total;
        a := angleOf(v);
        if major then len := scaled(Self, 8) else len := scaled(Self, 4);
        if major then
          paintLine(sp,
            cx + Round(Cos(a) * (tickOuter - len)),
            cy - Round(Sin(a) * (tickOuter - len)),
            cx + Round(Cos(a) * tickOuter),
            cy - Round(Sin(a) * tickOuter),
            clrTick, 2)
        else
          paintLine(sp,
            cx + Round(Cos(a) * (tickOuter - len)),
            cy - Round(Sin(a) * (tickOuter - len)),
            cx + Round(Cos(a) * tickOuter),
            cy - Round(Sin(a) * tickOuter),
            clrTick, 1);
      end;
    end;
    if FShowNeedle and (FStyle = gsDial) then begin
      a := angleOf(FShown);
      tipR := tickOuter - scaled(Self, 10);
      tailR := radius div 6;
      // a slim triangle from a hub to the tip
      side := DegToRad(90) + a;
      hubR := Math.Max(radius div 12, 3);
      // around the middle of the hub pixel
      hx := cx + 0.5;
      hy := cy + 0.5;
      SetLength(needlePts, 4);
      needlePts[0] := PointF(hx + Cos(a) * tipR, hy - Sin(a) * tipR);
      needlePts[1] := PointF(hx + Cos(side) * hubR * 0.6, hy - Sin(side) * hubR * 0.6);
      needlePts[2] := PointF(hx - Cos(a) * tailR, hy + Sin(a) * tailR);
      needlePts[3] := PointF(hx - Cos(side) * hubR * 0.6, hy + Sin(side) * hubR * 0.6);
      sp.FillPolygon(needlePts, clrNeedle);
      // black border-line around the needle
      sp.StrokePolyline(needlePts, 1.5, clBlack, True, True);
      paintCircle(sp, cx, cy, Round(hubR), clrNeedle, clBlack);
      paintCircle(sp, cx, cy, Math.Max(Round(hubR) div 2, 1), blend(clrNeedle, clWhite, 50), clBlack);
    end;
    sp.Flush;
  finally
    sp.Free;
  end;
  applyTextQuality(Canvas);
  fs := formatSettingsOf(FLanguage);
  if FShowLabels and (FMajorTicks > 1) then begin
    Canvas.Brush.Style := bsClear;
    Canvas.Font.Color := clrText;
    for i := 0 to FMajorTicks - 1 do begin
      v := FMin + (FMax - FMin) * i / (FMajorTicks - 1);
      a := angleOf(v);
      if FLabelFormat = '' then
        valueText := FloatToStr(v, fs)
      else
        valueText := Format(FLabelFormat, [v], fs);
      lr := tickOuter - scaled(Self, 22);
      tw := Canvas.TextWidth(valueText);
      Canvas.TextOut(
        cx + Round(Cos(a) * lr) - tw div 2,
        cy - Round(Sin(a) * lr) - labelH div 2,
        valueText);
    end;
    Canvas.Brush.Style := bsSolid;
  end;
  if FShowValue then begin
    if FValueFormat = '' then
      valueText := FloatToStr(FValue, fs)
    else
      valueText := Format(FValueFormat, [FValue], fs);
    if Assigned(FOnGetText) then FOnGetText(Self, FValue, valueText);
    th := Canvas.TextHeight(valueText);
    if FSweepAngle <= 200 then
      vr := Rect(0, cy + scaled(Self, 4), ClientWidth, cy + scaled(Self, 4) + th)
    else
      vr := Rect(0, cy + radius div 3, ClientWidth, cy + radius div 3 + th);
    paintCenteredText(Canvas, vr, valueText, clrText);
  end;
end;

// -- animation ----------------------

procedure TGauge.tick(Sender: TObject);
var
  delta: double;
begin
  delta := FValue - FShown;
  if Abs(delta) < (FMax - FMin) / 2000 then begin
    FShown := FValue;
    FTimer.Enabled := False;
  end else
    FShown := FShown + delta * 0.2;
  Invalidate;
end;

procedure TGauge.syncTimer;
begin
  if not FAnimated or (csDesigning in ComponentState) or
     (csLoading in ComponentState) or not Visible then begin
    FShown := FValue;
    FTimer.Enabled := False;
    Invalidate;
    Exit;
  end;
  FTimer.Enabled := FShown <> FValue;
end;

procedure TGauge.zonesChanged;
begin
  Invalidate;
end;

// -- properties ----------------------

function TGauge.getRange(index: integer): double;
begin
  case index of
    0: Result := FMin;
    1: Result := FMax;
  else
    Result := FValue;
  end;
end;

procedure TGauge.setRange(index: integer; value: double);
begin
  if getRange(index) = value then Exit;
  case index of
    0: begin
      FMin := value;
      if FMax < FMin then FMax := FMin;
    end;
    1: begin
      FMax := value;
      if FMin > FMax then FMin := FMax;
    end;
  else
    FValue := EnsureRange(value, FMin, FMax);
  end;
  if index <> 2 then
    FValue := EnsureRange(FValue, FMin, FMax);
  syncTimer;
  Invalidate;
end;

function TGauge.getMetric(index: integer): integer;
begin
  case index of
    0: Result := FStartAngle;
    1: Result := FSweepAngle;
    2: Result := FArcWidth;
    3: Result := FMajorTicks;
  else
    Result := FMinorTicks;
  end;
end;

procedure TGauge.setMetric(index: integer; value: integer);
begin
  if getMetric(index) = value then Exit;
  case index of
    0: FStartAngle := value;
    1: FSweepAngle := EnsureRange(value, 1, 360);
    2: FArcWidth := Math.Max(value, 1);
    3: FMajorTicks := Math.Max(value, 0);
  else
    FMinorTicks := Math.Max(value, 0);
  end;
  Invalidate;
end;

function TGauge.getFlag(index: integer): boolean;
begin
  case index of
    0: Result := FShowArc;
    1: Result := FShowNeedle;
    2: Result := FShowValue;
    3: Result := FShowLabels;
  else
    Result := FAnimated;
  end;
end;

procedure TGauge.setFlag(index: integer; value: boolean);
begin
  if getFlag(index) = value then Exit;
  case index of
    0: FShowArc := value;
    1: FShowNeedle := value;
    2: FShowValue := value;
    3: FShowLabels := value;
  else
    FAnimated := value;
  end;
  syncTimer;
  Invalidate;
end;

procedure TGauge.setZones(value: TGaugeZones);
begin
  FZones.Assign(value);
end;

function TGauge.getText(index: integer): string;
begin
  if index = 0 then Result := FValueFormat else Result := FLabelFormat;
end;

procedure TGauge.setText(index: integer; const value: string);
begin
  if getText(index) = value then Exit;
  if index = 0 then FValueFormat := value else FLabelFormat := value;
  Invalidate;
end;

function TGauge.getPartColor(index: integer): TColor;
begin
  case index of
    0: Result := FArcColor;
    1: Result := FNeedleColor;
    2: Result := FTickColor;
  else
    Result := FTextColor;
  end;
end;

procedure TGauge.setPartColor(index: integer; value: TColor);
begin
  if getPartColor(index) = value then Exit;
  case index of
    0: FArcColor := value;
    1: FNeedleColor := value;
    2: FTickColor := value;
  else
    FTextColor := value;
  end;
  Invalidate;
end;

procedure TGauge.setLanguage(value: TFormatLanguage);
begin
  if value = FLanguage then Exit;
  FLanguage := value;
  Invalidate;
end;

procedure TGauge.setStyle(value: TGaugeStyle);
begin
  if FStyle = value then Exit;
  FStyle := value;
  Invalidate;
end;

procedure TGauge.setAntialiasing(value: TAntialiasSetting);
begin
  if FAntialiasing = value then Exit;
  FAntialiasing := value;
  Invalidate;
end;

end.
