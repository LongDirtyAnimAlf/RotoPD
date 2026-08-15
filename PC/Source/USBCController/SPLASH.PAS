unit SPLASH;

interface

uses
  Classes, SysUtils, Graphics, Controls, Forms,
  Dialogs, StdCtrls, ExtCtrls;

type
  TSplashForm = class(TForm)
    Label1: TLabel;
    Label2: TLabel;
    Label3: TLabel;
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  SplashForm: TSplashForm;

implementation

{$ifdef FPC}
{$R *.lfm}
{$else}
{$R *.dfm}
{$endif}

end.
