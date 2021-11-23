@echo off 
set path=%~1
cd ../build/bin/%path%

@echo on
copy iw3r.dll "%COD4_ROOT%\bin\"
copy iw3r.pdb "%COD4_ROOT%\bin\"