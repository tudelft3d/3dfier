:: rename the files
move bgt_begroeidterreindeel.gml bgt_begroeidterreindeel_org.gml
move bgt_onbegroeidterreindeel.gml bgt_onbegroeidterreindeel_org.gml

:: add the srsDimension to the posList for the kruinlijn
powershell -Command "(gc 'bgt_begroeidterreindeel_org.gml') -replace '<gml:posList>', '<gml:posList srsDimension=\"2\">' | Out-File bgt_begroeidterreindeel.gml" -Encoding ASCII
powershell -Command "(gc 'bgt_onbegroeidterreindeel_org.gml') -replace '<gml:posList>', '<gml:posList srsDimension=\"2\">' | Out-File bgt_onbegroeidterreindeel.gml" -Encoding ASCII

:: change the filedate of all gfs files in the folder to now
powershell -Command "(Get-ChildItem "'.\'" -Filter *.gfs) | Foreach-Object {$_.LastWriteTime = (Get-Date)}"

:: convert the files to sqlite, filter eindregistratie and stroke geometries
ogr2ogr -nlt CONVERT_TO_LINEAR -where "eindregistratie is NULL and OGR_GEOMETRY='CURVEPOLYGON'" -f sqlite bgt_ondersteunendwegdeel.sqlite bgt_ondersteunendwegdeel.gml
ogr2ogr -nlt CONVERT_TO_LINEAR -where "eindregistratie is NULL and OGR_GEOMETRY='CURVEPOLYGON'" -f sqlite bgt_overbruggingsdeel.sqlite bgt_overbruggingsdeel.gml
ogr2ogr -nlt CONVERT_TO_LINEAR -where "eindregistratie is NULL and OGR_GEOMETRY='MULTIPOLYGON'" -f sqlite bgt_pand.sqlite bgt_pand.gml
ogr2ogr -nlt CONVERT_TO_LINEAR -where "eindregistratie is NULL and OGR_GEOMETRY='CURVEPOLYGON'" -f sqlite bgt_onbegroeidterreindeel.sqlite bgt_onbegroeidterreindeel.gml
ogr2ogr -nlt CONVERT_TO_LINEAR -where "eindregistratie is NULL and OGR_GEOMETRY='CURVEPOLYGON'" -f sqlite bgt_ondersteunendwaterdeel.sqlite bgt_ondersteunendwaterdeel.gml
ogr2ogr -nlt CONVERT_TO_LINEAR -where "eindregistratie is NULL and OGR_GEOMETRY='CURVEPOLYGON'" -f sqlite bgt_overigbouwwerk.sqlite bgt_overigbouwwerk.gml
ogr2ogr -nlt CONVERT_TO_LINEAR -where "eindregistratie is NULL and OGR_GEOMETRY='CURVEPOLYGON'" -f sqlite bgt_begroeidterreindeel.sqlite bgt_begroeidterreindeel.gml
ogr2ogr -nlt CONVERT_TO_LINEAR -where "eindregistratie is NULL and OGR_GEOMETRY='CURVEPOLYGON'" -f sqlite bgt_scheiding.sqlite bgt_scheiding.gml
ogr2ogr -nlt CONVERT_TO_LINEAR -where "eindregistratie is NULL and OGR_GEOMETRY='CURVEPOLYGON'" -f sqlite bgt_wegdeel.sqlite bgt_wegdeel.gml
ogr2ogr -nlt CONVERT_TO_LINEAR -where "eindregistratie is NULL and OGR_GEOMETRY='CURVEPOLYGON'" -f sqlite bgt_waterdeel.sqlite bgt_waterdeel.gml
ogr2ogr -nlt CONVERT_TO_LINEAR -where "eindregistratie is NULL and OGR_GEOMETRY='CURVEPOLYGON'" -f sqlite bgt_tunneldeel.sqlite bgt_tunneldeel.gml
ogr2ogr -nlt CONVERT_TO_LINEAR -where "eindregistratie is NULL and OGR_GEOMETRY='CURVEPOLYGON'" -f sqlite bgt_kunstwerkdeel.sqlite bgt_kunstwerkdeel.gml