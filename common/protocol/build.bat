@set dir=%cd%/proto
@set out_cs=./cs

@bin\protoc.exe --version
@for /R %%s in (*.proto) do @if exist %%s ( 
	@echo Build %%s
	bin\protoc.exe -I=%dir% --csharp_out=%out_cs% %%s 
	)
@echo Build OK
@echo.
@pause
