:: change the filedate of all gfs files in the folder to now
powershell -Command "(Get-ChildItem "'.\'" -Filter *.gfs) | Foreach-Object {$_.LastWriteTime = (Get-Date)}"