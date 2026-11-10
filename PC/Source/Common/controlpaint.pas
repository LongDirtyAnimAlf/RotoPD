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
  expressions, autofree, dynamic-array += .
}

unit ControlPaint;

{$mode objfpc}{$H+}
{$optimization level3}

interface

uses
  Classes, SysUtils, Types, Graphics, Controls, LCLType;

type
  TChevronDirection = (cdUp, cdDown, cdLeft, cdRight);
  // how the edges of the shapes blend into what is under them: not at all,
  // or with the share of each pixel they cover in 4, 9 or 256 steps
  TShapeAntialiasing = (aaOff, aaLow, aaMedium, aaHigh);
  // the smoothing of one control: after the theme, or its own choice
  TAntialiasSetting = (asTheme, asOff, asOn);
  // the names and separators a control formats dates, times and numbers
  // with: those of the locale of the system, or English with ISO dates and
  // a decimal point whatever the locale
  TFormatLanguage = (flSystem, flEnglish);
  // what a part painted by the application is in
  TDrawState = set of (dsHover, dsActive, dsSelected, dsPressed, dsDisabled, dsFocused, dsChecked);
  // paints one part of a control in place of the control; `handled` left
  // false lets the control paint the part as usual
  TCustomDrawEvent = procedure(Sender: TObject; canvas: TCanvas; const r: TRect; index: integer; state: TDrawState; var handled: boolean) of object;
  // a closed outline in the coordinates of the target canvas
  TOutline = array of TPointF;
  TOutlineArray = array of TOutline;

  { TSmoothPaint }

  // shapes painted straight into memory a line at a time, each with the
  // exact share of every pixel it covers, so the edges blend into what is
  // under them alike on every widgetset. Flush puts the whole area onto the
  // target at once; text and images go onto the target after it and keep
  // their native look. A direct painter draws with the canvas of the target
  // instead, without smoothing, for a shape over a native element
  TSmoothPaint = class
  private
    FTarget: TCanvas;
    FRect, FClip: TRect; // the area on the target, the clip in the memory
    FWidth, FHeight, FSteps: integer;
    FPixels: array of cardinal;
    FDirect, FFlushed: boolean;
    function box(const outlines: array of TOutline; out ox, oy, w, h: integer): boolean;
    procedure accumulate(const outlines: array of TOutline; ox, oy, w, h: integer);
    procedure rasterize(const outlines: array of TOutline; color: TColor);
    procedure rasterizeUnion(const pieces: array of TOutline; color: TColor);
    procedure directPen(color: TColor; width: single);
  public
    constructor Create(target: TCanvas; const r: TRect; background: TColor; quality: TShapeAntialiasing);
    constructor CreateDirect(target: TCanvas);
    destructor Destroy; override;
    // a color as the memory holds it
    class function PackColor(color: TColor): cardinal;
    // a rectangle on whole pixels, its edges not blended
    procedure FillRect(const r: TRect; color: TColor);
    // a rectangle anywhere, its edge pixels blended
    procedure FillRectF(left, top, right, bottom: single; color: TColor);
    procedure FillPolygon(const points: array of TPointF; color: TColor);
    // the area inside the outlines; an outline running the other way inside
    // another cuts a hole
    procedure FillOutlines(const outlines: array of TOutline; color: TColor);
    // a line through the points, round at its bends; a closed one returns
    // to the first point
    procedure StrokePolyline(const points: array of TPointF; width: single; color: TColor; closed: boolean = false; roundCaps: boolean = true);
    procedure FillEllipse(cx, cy, rx, ry: single; color: TColor);
    // the band of `width` inside the edge of the ellipse
    procedure FrameEllipse(cx, cy, rx, ry, width: single; color: TColor);
    procedure FillRoundRect(left, top, right, bottom, radius: single; color: TColor);
    // the band of `width` inside the edge of the round rect
    procedure FrameRoundRect(left, top, right, bottom, radius, width: single; color: TColor);
    // dashes along the middle of the band a frame of `width` would take
    procedure DashRoundRect(left, top, right, bottom, radius, width, dash, gap: single; color: TColor);
    // a line along an ellipse, angles in degrees counterclockwise from 3 o'clock
    procedure StrokeArc(cx, cy, rx, ry, startDeg, sweepDeg, width: single; color: TColor; roundCaps: boolean);
    // the part `from` of the image scaled into `into`
    procedure DrawImage(image: TRasterImage; const from, into: TRect);
    // text into the pixel buffer (or straight onto the canvas when Direct)
    // so shapes drawn after it sit on top; uses a temporary 32-bit bitmap
    procedure TextOut(x, y: integer; const text: string; font: TFont; color: TColor);
    // text centered in `r`, clipped to it
    procedure TextRect(const r: TRect; const text: string; font: TFont; color: TColor);
    // `w` by `h` pixels in the layout of the memory, a row after another,
    // with their top left corner at x, y
    procedure DrawPixels(x, y, w, h: integer; pixels: PCardinal);
    // the memory of row `y` from the left of the area, for code that writes
    // its pixels itself; nil outside the area and for a direct painter
    function Row(y: integer): PCardinal;
    // the shapes reach only into `r`
    procedure SetClip(const r: TRect);
    procedure ResetClip;
    // puts the area onto the target
    procedure Flush;
    property Direct: boolean read FDirect;
    property Area: TRect read FRect;
  end;

var
  // the smoothing of the controls that follow the theme; TAppTheme sets it
  shapeAntialiasing: TShapeAntialiasing = aaHigh;
  // the font quality of the text the controls draw; fqDefault leaves the
  // system choice
  textQuality: TFontQuality = fqDefault;

// a smooth painter over `r` of `canvas`, filled with `background`
function beginShapes(canvas: TCanvas; const r: TRect; background: TColor; setting: TAntialiasSetting): TSmoothPaint;
// the font quality of the theme onto a canvas before its text
procedure applyTextQuality(canvas: TCanvas);

// the fill under the mouse: the palette slot of the theme, otherwise the
// surface blended a little toward the text
function hoverColor(surface: TColor = clBtnFace): TColor;
// a border or a glyph under the mouse: `normal` moved toward `strong` as far
// as the theme lets a hover go; `strong` without a theme
function hoverShade(normal, strong: TColor): TColor;
// the ring of the focused control
function ringColor: TColor;
// the controls answer the mouse over them; a theme can switch that off
function hoverEnabled: boolean;
// the focused control carries a ring; a theme can switch that off
function ringEnabled: boolean;
// `color` unless it is clDefault, then `fallback`; both resolved to RGB
function pick(color, fallback: TColor): TColor;
// `a` moved `percent` of the way toward `b`
function blend(a, b: TColor; percent: integer): TColor;
// text that reads on `fill`: the window text on a light fill, white on a dark one
function contrastText(fill: TColor): TColor;
// the fill is on the dark side
function isDark(color: TColor): boolean;
// a pixel size at the control's DPI
function scaled(control: TControl; size: integer): integer;
// the format settings of a language, for FormatDateTime, Format and FloatToStr
function formatSettingsOf(language: TFormatLanguage): TFormatSettings;
// hands a part of a control to its custom draw handler over the painted
// background; true when the handler painted the part
function customDraw(control: TControl; canvas: TCanvas; handler: TCustomDrawEvent; const r: TRect; index: integer; state: TDrawState; background: TColor; antialiasing: TAntialiasSetting): boolean;
// a regular polygon of `sides` corners around a center, the first corner at
// `startDeg` (0 is to the right, 90 is up)
function polygonOutline(cx, cy, radius: single; sides: integer; startDeg: single): TOutline;

// the outlines of the shapes, in the coordinates of the target
function ellipseOutline(cx, cy, rx, ry: single; reverse: boolean = false): TOutline;
function roundRectOutline(left, top, right, bottom, radius: single; reverse: boolean = false): TOutline;

// fills `r` and frames it; a border of clNone leaves no frame
procedure paintRoundRect(sp: TSmoothPaint; const r: TRect; radius: integer; fill, border: TColor; borderWidth: integer=1);
// a pill: a round rect with the radius of its shorter side
procedure paintPill(sp: TSmoothPaint; const r: TRect; fill, border: TColor);
// a circle around the middle of pixel cx, cy
procedure paintCircle(sp: TSmoothPaint; cx, cy, radius: integer; fill, border: TColor; borderWidth: integer=1);
// a line through the middles of the two pixels
procedure paintLine(sp: TSmoothPaint; x1, y1, x2, y2: integer; color: TColor; width: integer=1);
// a chevron glyph centered in `r`, `size` wide
procedure paintChevron(sp: TSmoothPaint; const r: TRect; dir: TChevronDirection; size: integer; color: TColor);
// a cross glyph centered in `r`
procedure paintCross(sp: TSmoothPaint; const r: TRect; size: integer; color: TColor; thickness: integer=2);
// a check mark glyph centered in `r`
procedure paintCheck(sp: TSmoothPaint; const r: TRect; size: integer; color: TColor; thickness: integer=2);
// a magnifier glyph centered in `r`
procedure paintMagnifier(sp: TSmoothPaint; const r: TRect; size: integer; color: TColor);
// the ring around a focused control, one pixel inside `r`
procedure paintRing(sp: TSmoothPaint; const r: TRect; radius: integer);
// an arc of a circle, angles in degrees counterclockwise from 3 o'clock
procedure paintArc(sp: TSmoothPaint; const box: TRect; startDeg, sweepDeg: integer; color: TColor; width: integer; roundCaps: boolean);
// text centered in `r`, clipped to it
procedure paintCenteredText(canvas: TCanvas; const r: TRect; const text: string; color: TColor);
// the height of a line in `font` and the width of `text` in it, measured
// once and then remembered; a layout asks for them on every pass
function lineHeightOf(font: TFont): integer;
function textWidthOf(font: TFont; const text: string): integer;
// the first fixed pitch face the system has, from the ones that render well
function monospaceFontName: string;
// a popup window the clicks never activate, so the form behind keeps its
// look and the focus; nothing to do outside Windows
procedure keepInactive(wnd: HWND);
// a translucent top level window whose pixels of `keyColor` are see-through;
// a layered window ignores its region, so the key cuts its corners instead.
// Call it again after every change of the alpha, the LCL resets the key.
// False where the window system has no such windows
function layeredWindow(wnd: HWND; alpha: byte; keyColor: TColor): boolean;
// paints at once what waits to be painted in the control and the windows in
// it; while the mouse keeps moving, the paint messages wait for it to stop
procedure paintNow(control: TWinControl);

implementation

uses
  Math, LCLIntf, IntfGraphics, GraphType, FPImage, Forms;

// -- the rasterizer ----------------------

// The memory of a shape's box holds the signed area every edge leaves in
// each cell; the running sum along a row is then the share of each pixel
// inside the shape. A mark for every 16 cells of a row tells where the
// edges are, so a row goes over the cells between them at once. The
// buffers are left zeroed after each use
const
  BLOCK_SHIFT = 4;
  BLOCK_MASK = 15;

var
  areaBuf: array of single;
  markBuf: array of byte;
  unionBuf: array of single;
  spanBuf: array of integer; // the first and the last cell taken in each row of the union

procedure needArea(cells, marks: integer);
begin
  if length(areaBuf) < cells then begin
    areaBuf := nil;
    SetLength(areaBuf, cells);
  end;
  if length(markBuf) < marks then begin
    markBuf := nil;
    SetLength(markBuf, marks);
  end;
end;

procedure needUnion(cells, rows: integer);
begin
  if length(unionBuf) < cells then begin
    unionBuf := nil;
    SetLength(unionBuf, cells);
  end;
  if length(spanBuf) < 2*rows then SetLength(spanBuf, 2*rows);
end;

// adds the area an edge from x0,y0 to x1,y1 leaves in the cells of a box of
// `w` cells by `h` rows, and marks the blocks it touches; the rows are
// `stride` cells apart, two more than w, and have `blocks` marks each
procedure addEdge(acc: PSingle; marks: PByte; stride, blocks, w, h: integer; x0, y0, x1, y1: single);
var
  dir, dxdy, x, xnext, d, dy, xa, xb, x0f, x1f, s, a0, a1, a2, am, top: single;
  y, ystart, yend, x0i, x1i, i: integer;
  row: PSingle;
begin
  if y0 = y1 then exit;
  if y0 < y1 then dir := 1
  else begin
    dir := -1;
    s := x0;
    x0 := x1;
    x1 := s;
    s := y0;
    y0 := y1;
    y1 := s;
  end;
  if (y1 <= 0) or (y0 >= h) then exit;
  dxdy := (x1-x0)/(y1-y0);
  x := x0;
  ystart := 0;
  if y0 < 0 then x := x0-y0*dxdy
  else ystart := Trunc(y0);
  yend := Trunc(y1);
  if yend < y1 then Inc(yend);
  if yend > h then yend := h;
  for y := ystart to yend-1 do begin
    row := acc+y*stride;
    if (y > y0) then top:=y else top:=y0;
    if (y+1 < y1) then dy:=y+1 else dy:=y1;
    dy := dy - top;
    xnext := x+dxdy*dy;
    d := dy*dir;
    if x < xnext then begin
      xa := x;
      xb := xnext;
    end else begin
      xa := xnext;
      xb := x;
    end;
    // an edge beside the box counts at its side
    if xa < 0 then xa := 0 else if xa > w then xa := w;
    if xb < 0 then xb := 0 else if xb > w then xb := w;
    x0i := Trunc(xa);
    x1i := Trunc(xb);
    if x1i < xb then Inc(x1i);
    if x1i <= x0i+1 then begin
      s := 0.5*(xa+xb)-x0i;
      row[x0i] := row[x0i] + (d-d*s);
      row[x0i+1] := row[x0i+1] + (d*s);
      x1i := x0i+1;
    end else begin
      s := 1/(xb-xa);
      x0f := xa-x0i;
      a0 := 0.5*s*(1-x0f)*(1-x0f);
      x1f := xb-x1i+1;
      am := 0.5*s*x1f*x1f;
      row[x0i] := row[x0i] + (d*a0);
      if x1i = x0i+2 then row[x0i+1] := row[x0i+1] + (d*(1-a0-am))
      else begin
        a1 := s*(1.5-x0f);
        row[x0i+1] := row[x0i+1] + (d*(a1-a0));
        for i := x0i+2 to x1i-2 do row[i] := row[i] + (d*s);
        a2 := a1+(x1i-x0i-3)*s;
        row[x1i-1] := row[x1i-1] + (d*(1-a2-am));
      end;
      row[x1i] := row[x1i] + (d*am);
    end;
    for i := x0i shr BLOCK_SHIFT to x1i shr BLOCK_SHIFT do marks[y*blocks+i] := 1;
    x := xnext;
  end;
end;

// the weight of a coverage after the smoothing steps, 0 to 256
function weight(cover: single; steps: integer): cardinal; inline;
begin
  if cover < 0 then cover := -cover;
  if cover >= 1 then begin
    Result := 256;
    Exit;
  end;
  case steps of
    0: Result := Trunc(cover*256+0.5);
    1: if cover >= 0.5 then Result:=256 else Result:=0;
  else
    Result := Trunc(Round(cover*steps)/steps*256+0.5);
  end;
end;

// `color` laid over `pixel` with `a` of 256; two channels go at once
function mixPixel(pixel, color, a: cardinal): cardinal; inline;
var
  b: cardinal;
begin
  b := 256-a;
  Result := (((color and $FF00FF)*a+(pixel and $FF00FF)*b) shr 8) and $FF00FF;
  Result := Result or ((((color and $FF00)*a+(pixel and $FF00)*b) shr 8) and $FF00) or $FF000000;
end;

class function TSmoothPaint.PackColor(color: TColor): cardinal;
var
  c: cardinal;
begin
  c := cardinal(ColorToRGB(color));
  Result := $FF000000 or ((c and $FF) shl 16) or (c and $FF00) or ((c shr 16) and $FF);
end;

constructor TSmoothPaint.Create(target: TCanvas; const r: TRect; background: TColor; quality: TShapeAntialiasing);
var
  c: cardinal;
  p: PCardinal;
  i: integer;
begin
  inherited Create;
  FTarget := target;
  FRect := r;
  FWidth := Max(r.Width, 0);
  FHeight := Max(r.Height, 0);
  case quality of
    aaOff: FSteps:=1;
    aaLow: FSteps:=4;
    aaMedium: FSteps:=9;
  else
    FSteps:=0;
  end;
  ResetClip;
  if (FWidth = 0) or (FHeight = 0) then Exit;
  SetLength(FPixels, FWidth*FHeight);
  c := PackColor(background);
  p := PCardinal(@FPixels[0]);
  for i := 0 to FWidth*FHeight-1 do p[i] := c;
end;

constructor TSmoothPaint.CreateDirect(target: TCanvas);
begin
  inherited Create;
  FTarget := target;
  FDirect := True;
end;

destructor TSmoothPaint.Destroy;
begin
  Flush;
  inherited Destroy;
end;

procedure TSmoothPaint.SetClip(const r: TRect);
begin
  FClip := Rect(Max(r.Left-FRect.Left, 0), Max(r.Top-FRect.Top, 0), Min(r.Right-FRect.Left, FWidth), Min(r.Bottom-FRect.Top, FHeight));
end;

procedure TSmoothPaint.ResetClip;
begin
  FClip := Rect(0, 0, FWidth, FHeight);
end;

function TSmoothPaint.Row(y: integer): PCardinal;
begin
  if FDirect or (y < FRect.Top) or (y >= FRect.Bottom) or (FWidth = 0) then begin
    Result := nil;
    Exit;
  end;
  Result := @FPixels[(y-FRect.Top)*FWidth];
end;

// the pixels the outlines may touch, in the memory, within the clip
function TSmoothPaint.box(const outlines: array of TOutline; out ox, oy, w, h: integer): boolean;
var
  minX, minY, maxX, maxY: single;
  o: TOutline;
  p: TPointF;
  i, j: integer;
begin
  minX := MaxSingle;
  minY := MaxSingle;
  maxX := -MaxSingle;
  maxY := -MaxSingle;
  for i := 0 to High(outlines) do begin
    o := outlines[i];
    for j := 0 to High(o) do begin
      p := o[j];
      if p.X < minX then minX := p.X;
      if p.X > maxX then maxX := p.X;
      if p.Y < minY then minY := p.Y;
      if p.Y > maxY then maxY := p.Y;
    end;
  end;
  if minX > maxX then begin
    Result := False;
    Exit;
  end;
  ox := Max(Floor(minX)-FRect.Left, FClip.Left);
  oy := Max(Floor(minY)-FRect.Top, FClip.Top);
  w := Min(Ceil(maxX)-FRect.Left, FClip.Right)-ox;
  h := Min(Ceil(maxY)-FRect.Top, FClip.Bottom)-oy;
  Result := (w > 0) and (h > 0);
end;

// the edges of the outlines into the cells of the box at ox, oy of the
// memory, w by h
procedure TSmoothPaint.accumulate(const outlines: array of TOutline; ox, oy, w, h: integer);
var
  stride, blocks, n, i, oi: integer;
  acc: PSingle;
  marks: PByte;
  fx, fy: single;
  o: TOutline;
  a, b: TPointF;
begin
  stride := w+2;
  blocks := (stride-1) shr BLOCK_SHIFT + 1;
  needArea(stride*h, blocks*h);
  acc := PSingle(@areaBuf[0]);
  marks := PByte(@markBuf[0]);
  fx := FRect.Left+ox;
  fy := FRect.Top+oy;
  for oi := 0 to High(outlines) do begin
    o := outlines[oi];
    n := Length(o);
    if n < 3 then Continue;
    for i := 0 to n-1 do begin
      a := o[i];
      if i = n-1 then b := o[0] else b := o[i+1];
      addEdge(acc, marks, stride, blocks, w, h, a.X-fx, a.Y-fy, b.X-fx, b.Y-fy);
    end;
  end;
end;

procedure TSmoothPaint.rasterize(const outlines: array of TOutline; color: TColor);
var
  ox, oy, w, h, stride, blocks, y, x, e, i: integer;
  c, a: cardinal;
  localrow: PSingle;
  marks: PByte;
  pix: PCardinal;
  sum: single;
begin
  if not box(outlines, ox, oy, w, h) then Exit;
  accumulate(outlines, ox, oy, w, h);
  stride := w+2;
  blocks := (stride-1) shr BLOCK_SHIFT + 1;
  c := PackColor(color);
  for y := 0 to h-1 do begin
    localrow := PSingle(@areaBuf[y*stride]);
    marks := PByte(@markBuf[y*blocks]);
    pix := PCardinal(@FPixels[(oy+y)*FWidth+ox]);
    sum := 0;
    a := 0;
    x := 0;
    while x < w do begin
      e := Min((x or BLOCK_MASK)+1, w);
      if marks[x shr BLOCK_SHIFT] <> 0 then
        for i := x to e-1 do begin
          if localrow[i] <> 0 then begin
            sum := sum + localrow[i];
            localrow[i] := 0;
            a := weight(sum, FSteps);
          end;
          if a >= 256 then pix[i] := c
          else if a > 0 then pix[i] := mixPixel(pix[i], c, a);
        end
      else begin
        // no edge in these cells and the ones after, the share stays
        while (e < w) and (marks[e shr BLOCK_SHIFT] = 0) do e := Min(e+BLOCK_MASK+1, w);
        if a >= 256 then FillDWord(pix[x], e-x, c)
        else if a > 0 then
          for i := x to e-1 do pix[i] := mixPixel(pix[i], c, a);
      end;
      x := e;
    end;
    localrow[w] := 0;
    localrow[w+1] := 0;
    FillChar(marks^, blocks, 0);
  end;
end;

// pieces that overlap, as the parts of a stroke, joined by their larger
// coverage so the overlaps do not darken the edges
procedure TSmoothPaint.rasterizeUnion(const pieces: array of TOutline; color: TColor);
var
  ux, uy, uw, uh, ox, oy, w, h, stride, blocks, y, x, e, i, j, line, first, last: integer;
  union: PSingle;
  spans: PInteger;
  piece: TOutline;
  localrow: PSingle;
  marks: PByte;
  dst: PSingle;
  sum, cover: single;
  c, a: cardinal;
  pix: PCardinal;
  src: PSingle;
begin
  if not box(pieces, ux, uy, uw, uh) then Exit;
  needUnion(uw*uh, uh);
  union := PSingle(@unionBuf[0]);
  spans := PInteger(@spanBuf[0]);
  for y := 0 to uh-1 do begin
    spans[2*y] := uw;
    spans[2*y+1] := -1;
  end;
  for i := 0 to High(pieces) do begin
    piece := pieces[i];
    if Length(piece) < 3 then Continue;
    if not box([piece], ox, oy, w, h) then Continue;
    accumulate([piece], ox, oy, w, h);
    stride := w+2;
    blocks := (stride-1) shr BLOCK_SHIFT + 1;
    for y := 0 to h-1 do begin
      localrow := PSingle(@areaBuf[y*stride]);
      marks := PByte(@markBuf[y*blocks]);
      line := oy-uy+y;
      dst := union + line*uw + (ox-ux);
      sum := 0;
      cover := 0;
      first := w;
      last := -1;
      x := 0;
      while x < w do begin
        e := Min((x or BLOCK_MASK)+1, w);
        if marks[x shr BLOCK_SHIFT] <> 0 then
          for j := x to e-1 do begin
            if localrow[j] <> 0 then begin
              sum := sum + localrow[j];
              localrow[j] := 0;
              cover := Abs(sum);
              // what the rounding leaves past a shape is nothing
              if cover < 1/1024 then cover := 0;
            end;
            if cover > 0 then begin
              if cover > dst[j] then dst[j] := cover;
              if j < first then first := j;
              last := j;
            end;
          end
        else if cover > 0 then begin
          for j := x to e-1 do if cover > dst[j] then dst[j] := cover;
          if x < first then first := x;
          last := e-1;
        end;
        x := e;
      end;
      localrow[w] := 0;
      localrow[w+1] := 0;
      FillChar(marks^, blocks, 0);
      if last >= first then begin
        spans[2*line] := Min(spans[2*line], first+ox-ux);
        spans[2*line+1] := Max(spans[2*line+1], last+ox-ux);
      end;
    end;
  end;
  c := PackColor(color);
  for y := 0 to uh-1 do begin
    src := union + y*uw;
    pix := PCardinal(@FPixels[(uy+y)*FWidth+ux]);
    for x := spans[2*y] to spans[2*y+1] do begin
      a := weight(src[x], FSteps);
      src[x] := 0;
      if a >= 256 then pix[x] := c
      else if a > 0 then pix[x] := mixPixel(pix[x], c, a);
    end;
  end;
end;

// -- outlines ----------------------

// chords of a curve of radius `r` stay within a tenth of a pixel of it
function segmentsFor(r, sweepRad: single): integer;
var
  stepAngle: single;
begin
  if r < 0.5 then begin
    Result := Max(Ceil(Abs(sweepRad)/(Pi/4)), 2);
    Exit;
  end;
  stepAngle := 2*ArcCos(Max(1-0.1/r, -1));
  Result := Max(Ceil(Abs(sweepRad)/stepAngle), Max(Ceil(Abs(sweepRad)/(Pi/4)), 2));
end;

function ellipseOutline(cx, cy, rx, ry: single; reverse: boolean): TOutline;
var
  n, i: integer;
  a: single;
begin
  n := segmentsFor(Max(rx, ry), 2*Pi);
  SetLength(Result, n);
  for i := 0 to n-1 do begin
    a := 2*Pi*i/n;
    if reverse then a := -a;
    Result[i] := PointF(cx+rx*Cos(a), cy+ry*Sin(a));
  end;
end;

// the four corners as quarter circles, clockwise on the screen
function roundRectOutline(left, top, right, bottom, radius: single; reverse: boolean): TOutline;
var
  n, k, corner, i: integer;
  centers: array[0..3] of TPointF;
  a: single;
  t: TPointF;
begin
  radius := Min(radius, Min(right-left, bottom-top)/2);
  if radius <= 0.01 then begin
    SetLength(Result, 4);
    Result[0] := PointF(left, top);
    Result[1] := PointF(right, top);
    Result[2] := PointF(right, bottom);
    Result[3] := PointF(left, bottom);
  end else begin
    n := segmentsFor(radius, Pi/2);
    SetLength(Result, 4*(n+1));
    centers[0] := PointF(left+radius, top+radius);
    centers[1] := PointF(right-radius, top+radius);
    centers[2] := PointF(right-radius, bottom-radius);
    centers[3] := PointF(left+radius, bottom-radius);
    k := 0;
    for corner := 0 to 3 do
      for i := 0 to n do begin
        // from 180 degrees on, a quarter for each corner
        a := Pi + corner*Pi/2 + (Pi/2)*i/n;
        Result[k] := PointF(centers[corner].X+radius*Cos(a), centers[corner].Y+radius*Sin(a));
        Inc(k);
      end;
  end;
  if reverse then
    for i := 0 to Length(Result) div 2 - 1 do begin
      t := Result[i];
      Result[i] := Result[Length(Result)-1-i];
      Result[Length(Result)-1-i] := t;
    end;
end;

// the pieces of a stroke: a quad for every segment, a disc at every bend
// and at the ends when the caps are round
function strokePieces(const points: array of TPointF; width: single; closed, roundCaps: boolean): TOutlineArray;
var
  n, segments, i, len: integer;
  hw, dx, dy, nx, ny: single;
  a, b: TPointF;
  quad: TOutline;
  pieces: array of TOutline;
begin
  SetLength(pieces, 0);
  n := Length(points);
  if n = 0 then begin
    Result := pieces;
    Exit;
  end;
  hw := width/2;
  if closed then segments := n else segments := n-1;
  for i := 0 to segments-1 do begin
    a := points[i];
    b := points[(i+1) mod n];
    dx := b.X-a.X;
    dy := b.Y-a.Y;
    len := Round(Sqrt(dx*dx+dy*dy));
    if len < 1e-4 then Continue;
    nx := -dy/len*hw;
    ny := dx/len*hw;
    SetLength(quad, 4);
    quad[0] := PointF(a.X+nx, a.Y+ny);
    quad[1] := PointF(b.X+nx, b.Y+ny);
    quad[2] := PointF(b.X-nx, b.Y-ny);
    quad[3] := PointF(a.X-nx, a.Y-ny);
    len := Length(pieces);
    SetLength(pieces, len+1);
    pieces[len] := quad;
  end;
  for i := 0 to n-1 do begin
    if closed or ((i > 0) and (i < n-1)) or roundCaps then begin
      len := Length(pieces);
      SetLength(pieces, len+1);
      pieces[len] := ellipseOutline(points[i].X, points[i].Y, hw, hw);
    end;
  end;
  Result := pieces;
end;

// -- painting ----------------------

procedure TSmoothPaint.directPen(color: TColor; width: single);
begin
  FTarget.Pen.Style := psSolid;
  FTarget.Pen.Color := color;
  FTarget.Pen.Width := Max(Round(width), 1);
end;

procedure TSmoothPaint.FillRect(const r: TRect; color: TColor);
var
  x0, x1, y0, y1, y, x: integer;
  c: cardinal;
  pix: PCardinal;
begin
  if FDirect then begin
    FTarget.Brush.Style := bsSolid;
    FTarget.Brush.Color := color;
    FTarget.FillRect(r);
    Exit;
  end;
  x0 := Max(r.Left-FRect.Left, FClip.Left);
  x1 := Min(r.Right-FRect.Left, FClip.Right);
  y0 := Max(r.Top-FRect.Top, FClip.Top);
  y1 := Min(r.Bottom-FRect.Top, FClip.Bottom);
  if (x1 <= x0) or (y1 <= y0) then Exit;
  c := PackColor(color);
  for y := y0 to y1-1 do begin
    pix := PCardinal(@FPixels[y*FWidth]);
    for x := x0 to x1-1 do pix[x] := c;
  end;
end;

procedure TSmoothPaint.FillRectF(left, top, right, bottom: single; color: TColor);
var
  r: TOutline;
begin
  if FDirect then FillRect(Rect(Round(left), Round(top), Round(right), Round(bottom)), color)
  else begin
    SetLength(r, 4);
    r[0] := PointF(left, top);
    r[1] := PointF(right, top);
    r[2] := PointF(right, bottom);
    r[3] := PointF(left, bottom);
    rasterize([r], color);
  end;
end;

procedure TSmoothPaint.FillPolygon(const points: array of TPointF; color: TColor);
var
  pts: array of TPoint;
  i: integer;
  o: TOutline;
begin
  if FDirect then begin
    SetLength(pts, Length(points));
    for i := 0 to High(points) do pts[i] := Point(Trunc(points[i].X), Trunc(points[i].Y));
    FTarget.Brush.Style := bsSolid;
    FTarget.Brush.Color := color;
    directPen(color, 1);
    FTarget.Polygon(pts);
    Exit;
  end;
  SetLength(o, Length(points));
  for i := 0 to High(points) do o[i] := points[i];
  rasterize([o], color);
end;

procedure TSmoothPaint.FillOutlines(const outlines: array of TOutline; color: TColor);
var
  i: integer;
begin
  if FDirect then begin
    for i := 0 to High(outlines) do FillPolygon(outlines[i], color);
    Exit;
  end;
  rasterize(outlines, color);
end;

procedure TSmoothPaint.StrokePolyline(const points: array of TPointF; width: single; color: TColor; closed, roundCaps: boolean);
var
  pts: array of TPoint;
  i: integer;
begin
  if FDirect then begin
    SetLength(pts, Length(points));
    for i := 0 to High(points) do pts[i] := Point(Trunc(points[i].X), Trunc(points[i].Y));
    if closed and (Length(pts) > 0) then begin
      SetLength(pts, Length(pts)+1);
      pts[High(pts)] := pts[0];
    end;
    directPen(color, width);
    if roundCaps then FTarget.Pen.EndCap := pecRound else FTarget.Pen.EndCap := pecFlat;
    FTarget.Polyline(pts);
    FTarget.Pen.EndCap := pecRound;
    FTarget.Pen.Width := 1;
    Exit;
  end;
  rasterizeUnion(strokePieces(points, width, closed, roundCaps), color);
end;

procedure TSmoothPaint.FillEllipse(cx, cy, rx, ry: single; color: TColor);
begin
  if FDirect then begin
    FTarget.Brush.Style := bsSolid;
    FTarget.Brush.Color := color;
    FTarget.Pen.Style := psClear;
    FTarget.Ellipse(Round(cx-rx), Round(cy-ry), Round(cx+rx), Round(cy+ry));
    FTarget.Pen.Style := psSolid;
    Exit;
  end;
  rasterize([ellipseOutline(cx, cy, rx, ry)], color);
end;

procedure TSmoothPaint.FrameEllipse(cx, cy, rx, ry, width: single; color: TColor);
var
  half: integer;
begin
  if FDirect then begin
    half := Max(Round(width), 1) div 2;
    FTarget.Brush.Style := bsClear;
    directPen(color, width);
    FTarget.Ellipse(Round(cx-rx)+half, Round(cy-ry)+half, Round(cx+rx)-half, Round(cy+ry)-half);
    FTarget.Brush.Style := bsSolid;
    FTarget.Pen.Width := 1;
    Exit;
  end;
  if (rx-width <= 0) or (ry-width <= 0) then rasterize([ellipseOutline(cx, cy, rx, ry)], color)
  else rasterize([ellipseOutline(cx, cy, rx, ry), ellipseOutline(cx, cy, rx-width, ry-width, True)], color);
end;

procedure TSmoothPaint.FillRoundRect(left, top, right, bottom, radius: single; color: TColor);
begin
  if FDirect then begin
    FTarget.Brush.Style := bsSolid;
    FTarget.Brush.Color := color;
    FTarget.Pen.Style := psClear;
    if radius <= 0 then FTarget.Rectangle(Round(left), Round(top), Round(right), Round(bottom))
    else FTarget.RoundRect(Round(left), Round(top), Round(right), Round(bottom), Round(radius*2), Round(radius*2));
    FTarget.Pen.Style := psSolid;
    Exit;
  end;
  // a rectangle on whole pixels needs no blending at all
  if (radius <= 0) and (Frac(left) = 0) and (Frac(top) = 0) and (Frac(right) = 0) and (Frac(bottom) = 0) then
    FillRect(Rect(Trunc(left), Trunc(top), Trunc(right), Trunc(bottom)), color)
  else rasterize([roundRectOutline(left, top, right, bottom, radius)], color);
end;

procedure TSmoothPaint.FrameRoundRect(left, top, right, bottom, radius, width: single; color: TColor);
var
  half: integer;
begin
  if FDirect then begin
    half := Max(Round(width), 1) div 2;
    FTarget.Brush.Style := bsClear;
    directPen(color, width);
    if radius <= 0 then FTarget.Rectangle(Round(left)+half, Round(top)+half, Round(right)-half, Round(bottom)-half)
    else FTarget.RoundRect(Round(left)+half, Round(top)+half, Round(right)-half, Round(bottom)-half, Round(radius*2), Round(radius*2));
    FTarget.Brush.Style := bsSolid;
    FTarget.Pen.Width := 1;
    Exit;
  end;
  if (right-left <= 2*width) or (bottom-top <= 2*width) then rasterize([roundRectOutline(left, top, right, bottom, radius)], color)
  else rasterize([roundRectOutline(left, top, right, bottom, radius), roundRectOutline(left+width, top+width, right-width, bottom-width, Max(radius-width, 0), True)], color);
end;

procedure TSmoothPaint.DashRoundRect(left, top, right, bottom, radius, width, dash, gap: single; color: TColor);
var
  i, j, len: integer;
  half: single;
  path: TOutline;
  total, stretch, remaining, pos, t, lenSeg: single;
  pieces: array of TOutline;
  run: array of TPointF;
  inDash: boolean;
  a, b, p: TPointF;
  stroke: array of TOutline;
begin
  if FDirect or (dash <= 0) then begin
    FrameRoundRect(left, top, right, bottom, radius, width, color);
    Exit;
  end;
  // the middle line of the band, cut into dashes along its length
  half := width/2;
  path := roundRectOutline(left+half, top+half, right-half, bottom-half, Max(radius-half, 0));
  if Length(path) < 2 then Exit;
  len := Length(path);
  SetLength(path, len+1);
  path[len] := path[0];
  // the pattern stretched a little, so it closes on itself
  total := 0;
  for i := 0 to Length(path)-2 do
    total := total + Sqrt(Sqr(path[i+1].X-path[i].X)+Sqr(path[i+1].Y-path[i].Y));
  stretch := total / (Max(Round(total/(dash+gap)), 1)*(dash+gap));
  dash := dash * stretch;
  gap := gap * stretch;
  SetLength(pieces, 0);
  SetLength(run, 0);
  inDash := True;
  remaining := dash;
  for i := 0 to Length(path)-2 do begin
    a := path[i];
    b := path[i+1];
    lenSeg := Sqrt(Sqr(b.X-a.X)+Sqr(b.Y-a.Y));
    pos := 0;
    if inDash and (Length(run) = 0) then begin
      SetLength(run, 1);
      run[0] := a;
    end;
    while lenSeg-pos > remaining do begin
      pos := pos + remaining;
      t := pos/lenSeg;
      p := PointF(a.X+(b.X-a.X)*t, a.Y+(b.Y-a.Y)*t);
      if inDash then begin
        len := Length(run);
        SetLength(run, len+1);
        run[len] := p;
        stroke := strokePieces(run, width, False, False);
        for j := 0 to High(stroke) do begin
          SetLength(pieces, Length(pieces)+1);
          pieces[High(pieces)] := stroke[j];
        end;
        SetLength(run, 0);
        remaining := gap;
      end else begin
        SetLength(run, 1);
        run[0] := p;
        remaining := dash;
      end;
      inDash := not inDash;
    end;
    remaining := remaining - (lenSeg-pos);
    if inDash then begin
      len := Length(run);
      SetLength(run, len+1);
      run[len] := b;
    end;
  end;
  if inDash and (Length(run) > 1) then begin
    stroke := strokePieces(run, width, False, False);
    for j := 0 to High(stroke) do begin
      SetLength(pieces, Length(pieces)+1);
      pieces[High(pieces)] := stroke[j];
    end;
  end;
  rasterizeUnion(pieces, color);
end;

procedure TSmoothPaint.StrokeArc(cx, cy, rx, ry, startDeg, sweepDeg, width: single; color: TColor; roundCaps: boolean);
var
  half, n, i: integer;
  hw, a0, sweep, a, irx, iry: single;
  band: TOutline;
begin
  if FDirect then begin
    half := Max(Round(width), 1) div 2;
    directPen(color, width);
    if roundCaps then FTarget.Pen.EndCap := pecRound else FTarget.Pen.EndCap := pecFlat;
    FTarget.Arc(Round(cx-rx-half), Round(cy-ry-half), Round(cx+rx+half), Round(cy+ry+half), Round(startDeg*16), Round(sweepDeg*16));
    FTarget.Pen.EndCap := pecRound;
    FTarget.Pen.Width := 1;
    Exit;
  end;
  hw := width/2;
  a0 := DegToRad(startDeg);
  sweep := DegToRad(sweepDeg);
  n := segmentsFor(Max(rx, ry)+hw, sweep);
  // the outer edge forward and the inner one back; y grows downward, so the
  // angle turns counterclockwise with a minus on the sine
  SetLength(band, 0);
  for i := 0 to n do begin
    a := a0 + sweep*i/n;
    SetLength(band, Length(band)+1);
    band[High(band)] := PointF(cx+(rx+hw)*Cos(a), cy-(ry+hw)*Sin(a));
  end;
  irx := Max(rx-hw, 0);
  iry := Max(ry-hw, 0);
  for i := n downto 0 do begin
    a := a0 + sweep*i/n;
    SetLength(band, Length(band)+1);
    band[High(band)] := PointF(cx+irx*Cos(a), cy-iry*Sin(a));
  end;
  if not roundCaps then rasterize([band], color)
  else rasterizeUnion([band, ellipseOutline(cx+rx*Cos(a0), cy-ry*Sin(a0), hw, hw), ellipseOutline(cx+rx*Cos(a0+sweep), cy-ry*Sin(a0+sweep), hw, hw)], color);
end;

// each pixel of `into` takes the mean of the source area it covers, or the
// blend of the four nearest source pixels when the image grows
procedure TSmoothPaint.DrawImage(image: TRasterImage; const from, into: TRect);
var
  src: TLazIntfImage;
  sw, sh, x0, x1, y0, y1, y, x, yy, xx, k, px, py, ix, iy: integer;
  cells: array of cardinal;
  d: TRawImageDescription;
  fast: boolean;
  line: PCardinal;
  v: cardinal;
  c: TFPColor;
  scaleX, scaleY, sy0, sy1, sx0, sx1, r, g, b, total, wy, wx, cx, cy, fx, fy, wgt: single;
  pix: PCardinal;
begin
  if (from.Width <= 0) or (from.Height <= 0) or (into.Width <= 0) or (into.Height <= 0) then Exit;
  if FDirect then begin
    FTarget.CopyRect(into, image.Canvas, from);
    Exit;
  end;
  src := image.CreateIntfImage;
  try
    sw := src.Width;
    sh := src.Height;
    SetLength(cells, sw*sh);
    // the source in the layout of the memory, a row at a time
    d := src.DataDescription;
    fast := (d.BitsPerPixel = 32) and (d.ByteOrder = riboLSBFirst) and (d.RedPrec = 8) and (d.GreenPrec = 8) and (d.BluePrec = 8);
    for y := 0 to sh-1 do
      if fast then begin
        line := PCardinal(src.GetDataLineStart(y));
        for x := 0 to sw-1 do begin
          v := line[x];
          cells[y*sw+x] := $FF000000 or (((v shr d.RedShift) and $FF) shl 16) or (((v shr d.GreenShift) and $FF) shl 8) or ((v shr d.BlueShift) and $FF);
        end;
      end else
        for x := 0 to sw-1 do begin
          c := src.Colors[x, y];
          cells[y*sw+x] := $FF000000 or (cardinal(c.Red shr 8) shl 16) or (cardinal(c.Green shr 8) shl 8) or cardinal(c.Blue shr 8);
        end;
    scaleX := from.Width / into.Width;
    scaleY := from.Height / into.Height;
    x0 := Max(into.Left-FRect.Left, FClip.Left);
    x1 := Min(into.Right-FRect.Left, FClip.Right);
    y0 := Max(into.Top-FRect.Top, FClip.Top);
    y1 := Min(into.Bottom-FRect.Top, FClip.Bottom);
    for y := y0 to y1-1 do begin
      pix := PCardinal(@FPixels[y*FWidth]);
      sy0 := from.Top + (y+FRect.Top-into.Top)*scaleY;
      sy1 := sy0 + scaleY;
      for x := x0 to x1-1 do begin
        sx0 := from.Left + (x+FRect.Left-into.Left)*scaleX;
        sx1 := sx0 + scaleX;
        r := 0; g := 0; b := 0; total := 0;
        if (scaleX >= 1) and (scaleY >= 1) then begin
          // the mean of the covered source pixels, the cut ones by their share
          for yy := Max(Floor(sy0), 0) to Min(Ceil(sy1), sh)-1 do begin
            wy := Min(sy1, yy+1) - Max(sy0, yy);
            if wy <= 0 then Continue;
            for xx := Max(Floor(sx0), 0) to Min(Ceil(sx1), sw)-1 do begin
              wx := Min(sx1, xx+1) - Max(sx0, xx);
              if wx <= 0 then Continue;
              v := cells[yy*sw+xx];
              wgt := wx*wy;
              r := r + ((v shr 16) and $FF)*wgt;
              g := g + ((v shr 8) and $FF)*wgt;
              b := b + (v and $FF)*wgt;
              total := total + wgt;
            end;
          end;
        end else begin
          // the blend of the four source pixels around the middle
          cx := (sx0+sx1)/2 - 0.5;
          cy := (sy0+sy1)/2 - 0.5;
          ix := Floor(cx);
          iy := Floor(cy);
          fx := cx - ix;
          fy := cy - iy;
          for k := 0 to 3 do begin
            px := EnsureRange(ix + (k and 1), 0, sw-1);
            py := EnsureRange(iy + (k shr 1), 0, sh-1);
            if (k and 1) = 0 then wgt := 1-fx else wgt := fx;
            if (k shr 1) = 0 then wgt := wgt * (1-fy) else wgt := wgt * fy;
            v := cells[py*sw+px];
            r := r + ((v shr 16) and $FF)*wgt;
            g := g + ((v shr 8) and $FF)*wgt;
            b := b + (v and $FF)*wgt;
            total := total + wgt;
          end;
        end;
        if total > 0 then
          pix[x] := $FF000000 or (cardinal(Round(r/total)) shl 16) or (cardinal(Round(g/total)) shl 8) or cardinal(Round(b/total));
      end;
    end;
  finally
    src.Free;
  end;
end;

{$ifdef WINDOWS}
type
  // BITMAPINFOHEADER
  TDibHeader = record
    size: longword;
    width, height: longint;
    planes, bitCount: word;
    compression, sizeImage: longword;
    xPelsPerMeter, yPelsPerMeter: longint;
    clrUsed, clrImportant: longword;
  end;

function setDIBitsToDevice(dc: HDC; x, y: longint; w, h: longword; srcX, srcY: longint; startScan, lines: longword; bits, info: Pointer; usage: longword): longint; stdcall; external 'gdi32' name 'SetDIBitsToDevice';
{$endif}

// the pixels onto the canvas at x, y
procedure drawRaw(canvas: TCanvas; x, y, w, h: integer; pixels: PCardinal);
{$ifdef WINDOWS}
var
  header: TDibHeader;
{$else}
var
  raw: TRawImage;
  bmp: TBitmap;
{$endif}
begin
  {$ifdef WINDOWS}
  // straight from the memory; a bitmap of the LCL made for every paint
  // costs many times more
  header := Default(TDibHeader);
  header.size := SizeOf(header);
  header.width := w;
  header.height := -h; // top down
  header.planes := 1;
  header.bitCount := 32;
  canvas.Changing;
  setDIBitsToDevice(canvas.Handle, x, y, w, h, 0, 0, 0, h, pixels, @header, 0);
  canvas.Changed;
  {$else}
  raw.Init;
  raw.Description.Init_BPP32_B8G8R8_BIO_TTB(w, h);
  raw.Data := PByte(pixels);
  raw.DataSize := PtrUInt(w)*PtrUInt(h)*4;
  bmp := TBitmap.Create;
  try
    bmp.LoadFromRawImage(raw, False);
    canvas.Draw(x, y, bmp);
  finally
    bmp.Free;
  end;
  {$endif}
end;

procedure TSmoothPaint.DrawPixels(x, y, w, h: integer; pixels: PCardinal);
var
  x0, x1, y0, y1, localrow: integer;
begin
  if (w <= 0) or (h <= 0) then Exit;
  if FDirect then begin
    drawRaw(FTarget, x, y, w, h, pixels);
    Exit;
  end;
  x0 := Max(x-FRect.Left, FClip.Left);
  x1 := Min(x+w-FRect.Left, FClip.Right);
  y0 := Max(y-FRect.Top, FClip.Top);
  y1 := Min(y+h-FRect.Top, FClip.Bottom);
  if (x1 <= x0) or (y1 <= y0) then Exit;
  for localrow := y0 to y1-1 do
    Move(pixels[(localrow+FRect.Top-y)*w + x0+FRect.Left-x], FPixels[localrow*FWidth+x0], (x1-x0)*SizeOf(cardinal));
end;

procedure TSmoothPaint.TextOut(x, y: integer; const text: string; font: TFont; color: TColor);
var
  bmp: TBitmap;
  tw, th, dx, dy, ix, iy, x0, x1, y0, y1: integer;
  srcPix: PCardinal;
  srcCol, packedColor, cover: cardinal;
  pb, pg, pr: byte;
begin
  if text = '' then Exit;
  if FDirect then begin
    applyTextQuality(FTarget);
    FTarget.Font.Assign(font);
    FTarget.Font.Color := color;
    FTarget.Brush.Style := bsClear;
    FTarget.TextOut(x, y, text);
    FTarget.Brush.Style := bsSolid;
    Exit;
  end;
  if FFlushed or (FWidth = 0) or (FHeight = 0) then Exit;

  bmp := TBitmap.Create;
  try
    bmp.PixelFormat := pf32bit;
    bmp.Canvas.Font.Assign(font);
    if textQuality <> fqDefault then
      bmp.Canvas.Font.Quality := textQuality;
    tw := Max(bmp.Canvas.TextWidth(text), 1);
    th := Max(bmp.Canvas.TextHeight(text), 1);
    bmp.SetSize(tw, th);
    // white glyphs on black – intensity becomes coverage for blending
    bmp.Canvas.Brush.Color := clBlack;
    bmp.Canvas.FillRect(0, 0, tw, th);
    bmp.Canvas.Font.Color := clWhite;
    bmp.Canvas.Brush.Style := bsClear;
    bmp.Canvas.TextOut(0, 0, text);

    packedColor := PackColor(color);
    dx := x - FRect.Left;
    dy := y - FRect.Top;
    x0 := Max(dx, FClip.Left);
    x1 := Min(dx + tw, FClip.Right);
    y0 := Max(dy, FClip.Top);
    y1 := Min(dy + th, FClip.Bottom);
    if (x1 <= x0) or (y1 <= y0) then Exit;

    for iy := y0 to y1 - 1 do begin
      srcPix := PCardinal(bmp.ScanLine[iy - dy]);
      for ix := x0 to x1 - 1 do begin
        srcCol := srcPix[ix - dx];
        // BGRA or ARGB: use max of low three bytes as coverage
        pb := srcCol and $FF;
        pg := (srcCol shr 8) and $FF;
        pr := (srcCol shr 16) and $FF;
        cover := pb;
        if pg > cover then cover := pg;
        if pr > cover then cover := pr;
        if cover = 0 then
          Continue;
        if cover >= 255 then
          FPixels[iy * FWidth + ix] := packedColor
        else
          FPixels[iy * FWidth + ix] :=
            mixPixel(FPixels[iy * FWidth + ix], packedColor, cover + 1);
      end;
    end;
  finally
    bmp.Free;
  end;
end;

procedure TSmoothPaint.TextRect(const r: TRect; const text: string; font: TFont; color: TColor);
var
  measureBmp: TBitmap;
  tw, th, tx, ty: integer;
begin
  if text = '' then Exit;
  if FDirect then begin
    applyTextQuality(FTarget);
    FTarget.Font.Assign(font);
    paintCenteredText(FTarget, r, text, color);
    Exit;
  end;
  measureBmp := TBitmap.Create;
  try
    measureBmp.Canvas.Font.Assign(font);
    tw := measureBmp.Canvas.TextWidth(text);
    th := measureBmp.Canvas.TextHeight(text);
  finally
    measureBmp.Free;
  end;
  tx := r.Left + (r.Right - r.Left - tw) div 2;
  ty := r.Top + (r.Bottom - r.Top - th) div 2;
  TextOut(tx, ty, text, font, color);
end;

procedure TSmoothPaint.Flush;
begin
  if FFlushed or FDirect then Exit;
  FFlushed := True;
  if (FWidth = 0) or (FHeight = 0) then Exit;
  drawRaw(FTarget, FRect.Left, FRect.Top, FWidth, FHeight, @FPixels[0]);
  FPixels := nil;
end;

function beginShapes(canvas: TCanvas; const r: TRect; background: TColor; setting: TAntialiasSetting): TSmoothPaint;
var
  quality: TShapeAntialiasing;
begin
  case setting of
    asOff: quality := aaOff;
    asOn:  quality := aaHigh;
  else
    quality := shapeAntialiasing;
  end;
  Result := TSmoothPaint.Create(canvas, r, background, quality);
end;

procedure applyTextQuality(canvas: TCanvas);
begin
  if textQuality <> fqDefault then canvas.Font.Quality := textQuality;
end;

// -- colors ----------------------

function hoverColor(surface: TColor): TColor;
begin
  Result := blend(surface, clWindowText, 20);
end;

function hoverShade(normal, strong: TColor): TColor;
begin
  Result := ColorToRGB(strong);
end;

function ringColor: TColor;
begin
  Result := blend(clBtnShadow, clHotLight, 50);
end;

function hoverEnabled: boolean;
begin
  Result := false;
end;

function ringEnabled: boolean;
begin
  Result := false;
end;

function pick(color, fallback: TColor): TColor;
begin
  if color = clDefault then Result := ColorToRGB(fallback)
  else Result := ColorToRGB(color);
end;

function blend(a, b: TColor; percent: integer): TColor;
var
  ca, cb: LongInt;
  ra, ga, ba, rb, gb, bb: integer;
begin
  if percent <= 0 then begin
    Result := ColorToRGB(a);
    Exit;
  end;
  if percent >= 100 then begin
    Result := ColorToRGB(b);
    Exit;
  end;
  ca := ColorToRGB(a);
  cb := ColorToRGB(b);
  ra := Red(ca);  ga := Green(ca);  ba := Blue(ca);
  rb := Red(cb);  gb := Green(cb);  bb := Blue(cb);
  Result := RGBToColor(
    ra + (rb - ra) * percent div 100,
    ga + (gb - ga) * percent div 100,
    ba + (bb - ba) * percent div 100);
end;

function isDark(color: TColor): boolean;
begin
  color := ColorToRGB(color);
  Result := (Red(color)*299 + Green(color)*587 + Blue(color)*114) div 1000 < 128;
end;

function contrastText(fill: TColor): TColor;
begin
  if isDark(fill) then Result := clWhite
  else Result := ColorToRGB(clWindowText);
end;

function scaled(control: TControl; size: integer): integer;
begin
  Result := control.Scale96ToFont(size);
end;

// -- shapes ----------------------

procedure paintRoundRect(sp: TSmoothPaint; const r: TRect; radius: integer; fill, border: TColor; borderWidth: integer);
var
  rad: integer;
begin
  rad := Max(radius, 0);
  if fill <> clNone then sp.FillRoundRect(r.Left, r.Top, r.Right, r.Bottom, rad, fill);
  if border <> clNone then sp.FrameRoundRect(r.Left, r.Top, r.Right, r.Bottom, rad, Max(borderWidth, 1), border);
end;

procedure paintPill(sp: TSmoothPaint; const r: TRect; fill, border: TColor);
var
  d: integer;
begin
  if r.Width < r.Height then d := r.Width else d := r.Height;
  paintRoundRect(sp, r, d div 2, fill, border);
end;

procedure paintCircle(sp: TSmoothPaint; cx, cy, radius: integer; fill, border: TColor; borderWidth: integer);
var
  rr: single;
begin
  rr := radius + 0.5;
  if fill <> clNone then sp.FillEllipse(cx+0.5, cy+0.5, rr, rr, fill);
  if border <> clNone then sp.FrameEllipse(cx+0.5, cy+0.5, rr, rr, Max(borderWidth, 1), border);
end;

procedure paintLine(sp: TSmoothPaint; x1, y1, x2, y2: integer; color: TColor; width: integer);
begin
  sp.StrokePolyline([PointF(x1+0.5, y1+0.5), PointF(x2+0.5, y2+0.5)], Max(width, 1), color);
end;

procedure paintChevron(sp: TSmoothPaint; const r: TRect; dir: TChevronDirection; size: integer; color: TColor);
var
  cx, cy, h, q: single;
begin
  cx := (r.Left+r.Right) div 2 + 0.5;
  cy := (r.Top+r.Bottom) div 2 + 0.5;
  h := size div 2;
  q := Max(size div 4, 1);
  case dir of
    cdDown: sp.StrokePolyline([PointF(cx-h, cy-q), PointF(cx, cy+q), PointF(cx+h, cy-q)], 2, color);
    cdUp:   sp.StrokePolyline([PointF(cx-h, cy+q), PointF(cx, cy-q), PointF(cx+h, cy+q)], 2, color);
    cdLeft: sp.StrokePolyline([PointF(cx+q, cy-h), PointF(cx-q, cy), PointF(cx+q, cy+h)], 2, color);
  else
    sp.StrokePolyline([PointF(cx-q, cy-h), PointF(cx+q, cy), PointF(cx-q, cy+h)], 2, color);
  end;
end;

procedure paintCross(sp: TSmoothPaint; const r: TRect; size: integer; color: TColor; thickness: integer);
var
  cx, cy, h: integer;
begin
  cx := (r.Left+r.Right) div 2;
  cy := (r.Top+r.Bottom) div 2;
  h := size div 2;
  paintLine(sp, cx-h, cy-h, cx+h+1, cy+h+1, color, thickness);
  paintLine(sp, cx-h, cy+h, cx+h+1, cy-h-1, color, thickness);
end;

procedure paintCheck(sp: TSmoothPaint; const r: TRect; size: integer; color: TColor; thickness: integer);
var
  cx, cy, h: single;
begin
  cx := (r.Left+r.Right) div 2 + 0.5;
  cy := (r.Top+r.Bottom) div 2 + 0.5;
  h := size div 2;
  sp.StrokePolyline([PointF(cx-h, cy), PointF((cx-h) / 3, (cy+h-h) / 3), PointF(cx+h, (cy-h+h) / 3)], Max(thickness, 1), color);
end;

procedure paintMagnifier(sp: TSmoothPaint; const r: TRect; size: integer; color: TColor);
var
  cx, cy, lens, lx, ly, d: integer;
begin
  cx := (r.Left+r.Right) div 2;
  cy := (r.Top+r.Bottom) div 2;
  // the lens sits up-left, the handle runs to the down-right corner
  lens := Max(size*5 div 16, 3);
  lx := cx - size div 8;
  ly := cy - size div 8;
  paintCircle(sp, lx, ly, lens, clNone, color);
  d := Round(lens*0.7);
  paintLine(sp, lx+d, ly+d, cx+size div 2, cy+size div 2, color, 2);
end;

procedure paintRing(sp: TSmoothPaint; const r: TRect; radius: integer);
var
  inner: TRect;
begin
  inner := r;
  inner.Inflate(-1, -1);
  paintRoundRect(sp, inner, radius, clNone, ringColor);
end;

procedure paintArc(sp: TSmoothPaint; const box: TRect; startDeg, sweepDeg: integer; color: TColor; width: integer; roundCaps: boolean);
var
  w: integer;
  cx, cy: single;
begin
  w := Max(width, 1);
  cx := (box.Left+box.Right)/2;
  cy := (box.Top+box.Bottom)/2;
  // the stroke stays inside the box
  if sweepDeg >= 360 then sp.FrameEllipse(cx, cy, box.Width/2, box.Height/2, w, color)
  else sp.StrokeArc(cx, cy, (box.Width-w)/2, (box.Height-w)/2, startDeg, sweepDeg, w, color, roundCaps);
end;

{$ifdef WINDOWS}
function getWindowLongPtr(wnd: THandle; index: longint): PtrInt; stdcall; external 'user32' name {$ifdef CPU64}'GetWindowLongPtrW'{$else}'GetWindowLongW'{$endif};
function setWindowLongPtr(wnd: THandle; index: longint; value: PtrInt): PtrInt; stdcall; external 'user32' name {$ifdef CPU64}'SetWindowLongPtrW'{$else}'SetWindowLongW'{$endif};
function setLayeredWindowAttributes(wnd: THandle; key: cardinal; alpha: byte; flags: cardinal): longbool; stdcall; external 'user32' name 'SetLayeredWindowAttributes';
{$endif}

function layeredWindow(wnd: HWND; alpha: byte; keyColor: TColor): boolean;
begin
  Result := False;
  {$ifdef WINDOWS}
  // GWL_EXSTYLE and WS_EX_LAYERED, then LWA_COLORKEY or LWA_ALPHA
  setWindowLongPtr(wnd, -20, getWindowLongPtr(wnd, -20) or $00080000);
  Result := setLayeredWindowAttributes(wnd, cardinal(ColorToRGB(keyColor)), alpha, 3);
  {$endif}
end;

procedure keepInactive(wnd: HWND);
begin
  {$ifdef WINDOWS}
  // GWL_EXSTYLE and WS_EX_NOACTIVATE
  setWindowLongPtr(wnd, -20, getWindowLongPtr(wnd, -20) or $08000000);
  {$endif}
end;

procedure paintNow(control: TWinControl);
begin
  if control.HandleAllocated then RedrawWindow(control.Handle, nil, 0, RDW_UPDATENOW or RDW_ALLCHILDREN);
end;

procedure paintCenteredText(canvas: TCanvas; const r: TRect; const text: string; color: TColor);
var
  style: TTextStyle;
begin
  applyTextQuality(canvas);
  style := canvas.TextStyle;
  style.Alignment := taCenter;
  style.Layout := tlCenter;
  style.SingleLine := True;
  style.Clipping := True;
  style.Opaque := False;
  style.ShowPrefix := False;
  canvas.Brush.Style := bsClear;
  canvas.Font.Color := color;
  canvas.TextRect(r, r.Left, r.Top, text, style);
  canvas.Brush.Style := bsSolid;
end;

var
  measureBitmap: TBitmap;
  measured: TStringList; // the sizes by font and text, sorted

function measure(font: TFont; const text: string; height: boolean): integer;
var
  style, i: integer;
  key, hw: string;
  st: TFontStyle;
begin
  style := 0;
  for st := Low(TFontStyle) to High(TFontStyle) do
    if st in font.Style then style := style or (1 shl Ord(st));
  if height then hw := 'h' else hw := 'w';
  key := Format('%s|%d|%d|%d|%d|%d|%d|%d|%s|', [font.Name, font.Height, font.Size, font.PixelsPerInch, style, Ord(font.Pitch), Ord(font.Quality), font.CharSet, hw]) + text;
  if measured = nil then begin
    measured := TStringList.Create;
    measured.CaseSensitive := True;
    measured.Sorted := True;
  end;
  if measured.Find(key, i) then begin
    Result := integer(PtrInt(measured.Objects[i]));
    Exit;
  end;
  if measureBitmap = nil then begin
    measureBitmap := TBitmap.Create;
    measureBitmap.SetSize(1, 1);
  end;
  measureBitmap.Canvas.Font := font;
  if height then Result := measureBitmap.Canvas.TextHeight(text)
  else Result := measureBitmap.Canvas.TextWidth(text);
  if measured.Count >= 4096 then measured.Clear;
  measured.AddObject(key, TObject(PtrInt(Result)));
end;

function lineHeightOf(font: TFont): integer;
begin
  Result := measure(font, 'Hg', True);
end;

var
  monoFace: string = '';

function monospaceFontName: string;
var
  name: string;
  i, c: integer;
  candidates: array[0..9] of string = ('Cascadia Mono', 'Consolas', 'JetBrains Mono', 'Fira Code', 'DejaVu Sans Mono', 'Liberation Mono', 'Noto Sans Mono', 'Ubuntu Mono', 'Menlo', 'Courier New');
begin
  if monoFace <> '' then begin
    Result := monoFace;
    Exit;
  end;
  monoFace := 'Courier New';
  for c := Low(candidates) to High(candidates) do begin
    name := candidates[c];
    for i := 0 to Screen.Fonts.Count-1 do
      if SameText(Screen.Fonts[i], name) then begin
        monoFace := Screen.Fonts[i];
        Result := monoFace;
        Exit;
      end;
  end;
  Result := monoFace;
end;

function textWidthOf(font: TFont; const text: string): integer;
begin
  Result := measure(font, text, False);
end;

function customDraw(control: TControl; canvas: TCanvas; handler: TCustomDrawEvent; const r: TRect; index: integer; state: TDrawState; background: TColor; antialiasing: TAntialiasSetting): boolean;
var
  plain: TSmoothPaint;
begin
  Result := False;
  if not Assigned(handler) then Exit;
  plain := beginShapes(canvas, Rect(0, 0, control.Width, control.Height), background, antialiasing);
  try
    plain.Flush;
    handler(control, canvas, r, index, state, Result);
  finally
    plain.Free;
  end;
end;

function polygonOutline(cx, cy, radius: single; sides: integer; startDeg: single): TOutline;
var
  i: integer;
  a: single;
begin
  SetLength(Result, sides);
  for i := 0 to sides-1 do begin
    a := DegToRad(startDeg + i*360/sides);
    Result[i] := PointF(cx + Cos(a)*radius, cy - Sin(a)*radius);
  end;
end;

var
  englishSettings: TFormatSettings;
  englishReady: boolean = False;

function formatSettingsOf(language: TFormatLanguage): TFormatSettings;
const
  MONTHS: array[1..12] of string = ('January', 'February', 'March', 'April', 'May', 'June', 'July', 'August', 'September', 'October', 'November', 'December');
  DAYS: array[1..7] of string = ('Sunday', 'Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday');
var
  i: integer;
begin
  if language = flSystem then begin
    Result := DefaultFormatSettings;
    Exit;
  end;
  if not englishReady then begin
    englishSettings := DefaultFormatSettings;
    with englishSettings do begin
      DateSeparator := '-';
      TimeSeparator := ':';
      ShortDateFormat := 'yyyy-mm-dd';
      LongDateFormat := 'dddd, d mmmm yyyy';
      ShortTimeFormat := 'hh:nn';
      LongTimeFormat := 'hh:nn:ss';
      TimeAMString := 'AM';
      TimePMString := 'PM';
      DecimalSeparator := '.';
      ThousandSeparator := ',';
      ListSeparator := ',';
      for i := 1 to 12 do begin
        LongMonthNames[i] := MONTHS[i];
        ShortMonthNames[i] := Copy(MONTHS[i], 1, 3);
      end;
      for i := 1 to 7 do begin
        LongDayNames[i] := DAYS[i];
        ShortDayNames[i] := Copy(DAYS[i], 1, 3);
      end;
    end;
    englishReady := True;
  end;
  Result := englishSettings;
end;

finalization
  measured.Free;
  measureBitmap.Free;
end.
