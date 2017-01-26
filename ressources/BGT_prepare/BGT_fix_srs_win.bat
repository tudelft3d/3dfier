:: rename the files
move bgt_begroeidterreindeel.gml bgt_begroeidterreindeel_org.gml
move bgt_onbegroeidterreindeel.gml bgt_onbegroeidterreindeel_org.gml
:: add the srsDimension to the posList for the kruinlijn
powershell -Command "(gc 'bgt_begroeidterreindeel_org.gml') -replace '<gml:posList>', '<gml:posList srsDimension=\"2\">' | Out-File bgt_begroeidterreindeel.gml" -Encoding ASCII
powershell -Command "(gc 'bgt_onbegroeidterreindeel_org.gml') -replace '<gml:posList>', '<gml:posList srsDimension=\"2\">' | Out-File bgt_onbegroeidterreindeel.gml" -Encoding ASCII
powershell -Command "(gc 'bgt_ondersteunendwegdeel_org.gml') -replace '<gml:posList>', '<gml:posList srsDimension=\"2\">' | Out-File bgt_ondersteunendwegdeel.gml" -Encoding ASCII