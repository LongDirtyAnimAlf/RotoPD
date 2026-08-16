program Powerbank;

{$mode objfpc}{$H+}

uses
  {$IFDEF UNIX}
  cthreads,
  {$ENDIF}
  Interfaces,
  Forms, tachartlazaruspkg,
  main, ap33772s;

{$R *.res}

begin
  RequireDerivedFormResource:=True;
  Application.Scaled:=True;
  Application.Initialize;
  Application.CreateForm(TPowerbankMainForm, PowerbankMainForm);
  Application.Run;
end.

