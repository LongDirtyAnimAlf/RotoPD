unit printform;

{$mode ObjFPC}{$H+}

interface

uses
  {$ifdef Windows}
  Windows,
  {$endif}
  Classes, SysUtils, Forms, Controls, Graphics, Dialogs,
  Messages;

type
  TLocalPrintForm = class(TForm)
  private
    {$ifdef MSWindows}
    procedure WMGetMinMaxInfo(var uMessage : TWMGetMinMaxInfo); message WM_GETMINMAXINFO;
    {$endif}
  public

  end;

var
  Form2: TLocalPrintForm;

implementation

{$R *.lfm}

{$ifdef MSWindows}
procedure TLocalPrintForm.WMGetMinMaxInfo(var uMessage : TWMGetMinMaxInfo);
begin
  uMessage.MinMaxInfo^.ptMaxSize.X:=5000;
  uMessage.MinMaxInfo^.ptMaxSize.Y:=5000;
  uMessage.MinMaxInfo^.ptMaxTrackSize:=uMessage.MinMaxInfo^.ptMaxSize;
end;
{$endif}

end.

