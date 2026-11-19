param([Parameter(Mandatory)][string]$Map, [Parameter(Mandatory)][string]$Name, [Parameter(Mandatory)][string]$Out,
      [string]$Game = 'D:\CoD\CODOL Builds\Call of Duty Online 1.1.1.13', [string[]]$Assets = @(), [string]$Loadscreen, [string]$DisplayName,
      [string]$Environment = 'urban', [string]$Cache, [string[]]$Extra = @(), [string]$Bsp, [string]$Assetc, [switch]$Fast, [string]$Project,
      [string]$Carry = 'xanim,sndcurve,loaded_sound,sound,fx,xmodel')
# A map with everything around it, from a .map:
#
#   <Out>\zone\<Name>.ff            the map zone: world, collision, lights, the minimap material compass_map_<Name> with
#                                   its picture ( rendered by the compiler from above ), the effect scripts
#   <Out>\zone\<Name>_load.ff       the loading screen zone; with -Loadscreen <picture>: $levelbriefing / $defeatbackdrop
#                                   show that picture ( image loadscreen_<Name>, inline in the zone )
#   <Out>\scripts\maps\mp\<Name>.gsc ( + _fx, createfx )   the level script for the dedicated server's script folder
#   <Out>\main\zz_*_sounds.iwd      the streamed sound files of the project, when it has any
#   <Out>\presence\                 what the create-room list needs, and INSTALL.txt: where every file goes
#
# -Assets: codol_assetc project zones. The compiler takes the materials and models the map uses from them; after it
#   everything a script can name - animations, sound aliases with their loaded sounds and curves, effects, models
#   ( -Carry: the asset types ) - is carried into the map zone with what it points at ( codol_assetc --carry ).
# -Project: the folder of an asset project ( the editor's mod\<project> ). From it:
#     generated\animtrees\<tree>.atr     -> rawfile animtrees/<tree>.atr of the map zone   ( written by the asset build:
#     generated\animlists\map.animlist   -> rawfile animlists/<Name>.animlist                the animtrees of its animations )
#     scripts\<path>                     -> rawfile <path> of the map zone and <Out>\scripts\<path>
#     vision\<Name>.vision               -> the map's vision file ( glow, film tweaks ); settings by name or number
#     sun\<Name>.sun                     -> the map's sun flare ( codol_bsp --vision / --sun; README "Vision file, sun flare" )
#     zz_*_sounds.iwd                    -> <Out>\main\
#   The rule for scripts: a file of the project's scripts\ is taken as it is and goes in front of a generated file of
#   the same path - the level script maps\mp\<Name>.gsc, the effect scripts, an animtree, the animlist. What the
#   compiler generated stays in <Out>\work\scripts for reference. A level script of your own has to call
#   maps\mp\<Name>_fx::main() and maps\mp\_load::main() itself.
# -Extra: further codol_bsp arguments. Nothing is written outside <Out>; the install is only read.
$ErrorActionPreference = 'Stop'
$root = (Resolve-Path "$PSScriptRoot\..\..").Path
# the tools: as given, else where the READMEs build them ( build\codol_bsp, build\codol_assets_x64 ), else next to the editor
$find = {
  param($name, $places)
  foreach ($p in $places) { if (Test-Path "$root\$p\$name") { return "$root\$p\$name" } }
  throw "$name is not built ( looked in $($places -join ', ') below $root )"
}
if (-not $Bsp) { $Bsp = & $find 'codol_bsp.exe' @('build\codol_bsp\Release', 'build-editor\Release') }
if (-not $Assetc) { $Assetc = & $find 'codol_assetc.exe' @('build\codol_assets\Release', 'build\codol_assets_x64\Release', 'build-editor\Release') }
foreach ($tool in $Bsp, $Assetc) { if (-not (Test-Path $tool)) { throw "$tool is not built" } }
if ($Name -notmatch '^[a-z0-9_]+$') { throw 'the map name may only hold a-z, 0-9 and _' }
if (-not $DisplayName) { $DisplayName = $Name }
if (-not $Cache) { $Cache = "$Out\work" }
$zone = "$Out\zone"; $work = "$Out\work"; $presence = "$Out\presence"
New-Item -ItemType Directory -Force $zone, $work, $presence, "$Out\scripts", "$work\scripts" | Out-Null
if ($Project -and -not (Test-Path $Project -PathType Container)) { throw "-Project $Project is not a folder" }

# 1. the map
$a = @('--map', $Map, '--exe', "$Game\codoMP_client_shipRetail.exe", '--install', $Game, '--out', "$zone\$Name.ff", '--name', $Name, '--cache', $Cache,
       '--report', "$work\$Name.report.txt", '--minimap', "$work\compass_map_$Name.tga", '--scripts', "$work\scripts")
foreach ($z in $Assets) { $a += @('--assets', $z) }
if ($Fast) { $a += '--fast' }
if ($Project -and (Test-Path "$Project\vision\$Name.vision")) { $a += @('--vision', "$Project\vision\$Name.vision") }
if ($Project -and (Test-Path "$Project\sun\$Name.sun")) { $a += @('--sun', "$Project\sun\$Name.sun") }
& $Bsp @a @Extra | Select-Object -Last 2
if ($LASTEXITCODE) { throw 'codol_bsp failed' }

# 2. into the map zone: the minimap material, what a script can name of the project zones, the project's rawfiles
Set-Content "$work\minimap.mtl" -Encoding ASCII -Value @"
// the compass of ${Name}: the picture codol_bsp rendered
material "compass_map_$Name"
{
	type "2d"
	colorMap compass_map_$Name clamp
}
"@
$b = @('--install', $Game, '--base', "$zone\$Name.ff", '--name', $Name, '--out', "$zone\$Name.ff", '--matlib', "$Cache\codol_matlib.txt",
       '--image', "compass_map_$Name=$work\compass_map_$Name.tga", '--material', "$work\minimap.mtl")
if ($Carry) { foreach ($z in $Assets) { $b += @('--carry', "$z,$Carry") } }
# rawfile name -> file; a later entry of the same name ( any case ) replaces an earlier one: the project's scripts last
$raw = [ordered]@{}
$mine = [ordered]@{}		# path below scripts\ -> file
if ($Project) {
  $root = (Resolve-Path $Project).Path.TrimEnd('\')
  if (Test-Path "$root\generated\animtrees") { Get-ChildItem "$root\generated\animtrees" -File -Filter *.atr | ForEach-Object { $raw["animtrees/$($_.Name)"] = $_.FullName } }
  if (Test-Path "$root\generated\animlists\map.animlist") { $raw["animlists/$Name.animlist"] = "$root\generated\animlists\map.animlist" }
  if (Test-Path "$root\scripts") {
    Get-ChildItem "$root\scripts" -File -Recurse | Sort-Object FullName | ForEach-Object {
      $rel = $_.FullName.Substring("$root\scripts\".Length)
      if ($rel -match '[=,"]') { throw "scripts\${rel}: a script path cannot hold = , or a quote" }
      $mine[$rel] = $_.FullName
      $raw[($rel -replace '\\', '/')] = $_.FullName
    }
  }
}
foreach ($k in $raw.Keys) { $b += @('--rawfile', "$k=$($raw[$k])") }
& $Assetc @b | Select-String -Pattern 'compass_map|carried|kept|warning|wrote|rror' | ForEach-Object { '  ' + $_.Line.Trim() }
if ($LASTEXITCODE) { throw 'adding the minimap material and the project assets to the map zone failed' }
foreach ($k in $raw.Keys) { "  rawfile $k  <-  $($raw[$k])" }

# 2b. the script folder of the package and the sound archive. Files this script put there in an earlier run and
#     does not make any more are removed ( work\package_files.txt lists them ); nothing else is deleted.
$made = @()
$put = {
  param($from, $rel)
  $to = Join-Path $Out $rel
  New-Item -ItemType Directory -Force (Split-Path $to) | Out-Null
  Copy-Item -LiteralPath $from -Destination $to -Force
  $script:made += $rel
}
Get-ChildItem "$work\scripts" -File -Recurse | ForEach-Object {
  $rel = $_.FullName.Substring("$work\scripts\".Length)
  if ($mine.Contains($rel)) { "  script $rel`: the project's, in front of the generated one ( kept in work\scripts )" } else { & $put $_.FullName "scripts\$rel" }
}
foreach ($rel in $mine.Keys) { & $put $mine[$rel] "scripts\$rel" }
$archives = @()
if ($Project) { $archives = @(Get-ChildItem $Project -File -Filter 'zz_*.iwd') }
foreach ($f in $archives) { & $put $f.FullName "main\$($f.Name)"; "  sound archive main\$($f.Name)  $($f.Length) bytes" }
$manifest = "$work\package_files.txt"
if (Test-Path $manifest) {
  Get-Content $manifest | Where-Object { $_ -and ($made -notcontains $_) -and ($_ -match '^(scripts|main)\\') -and ($_ -notmatch '\.\.') } | ForEach-Object {
    $old = Join-Path $Out $_
    if (Test-Path -LiteralPath $old -PathType Leaf) { Remove-Item -LiteralPath $old -Force; "  removed $_ ( of an earlier build )" }
  }
}
Set-Content $manifest -Encoding UTF8 -Value $made

# 3. the loading screen
if ($Loadscreen) {
  Set-Content "$work\load.mtl" -Encoding ASCII -Value @"
// the loading screen of $Name
material "`$levelbriefing"
{
	type "2d"
	colorMap loadscreen_$Name clamp
}

material "`$defeatbackdrop"
{
	type "2d"
	colorMap loadscreen_$Name clamp
}
"@
  Set-Content "$work\${Name}_load.txt" -Encoding ASCII -Value "codol_bsp $Name"
  & $Assetc --install $Game --name "${Name}_load" --out "$zone\${Name}_load.ff" --matlib "$Cache\codol_matlib.txt" `
    --image "loadscreen_$Name=$Loadscreen,nomips" --material "$work\load.mtl" --rawfile "${Name}_load=$work\${Name}_load.txt" |
    Select-String -Pattern 'loadscreen|levelbriefing|wrote|rror' | ForEach-Object { '  ' + $_.Line.Trim() }
  if ($LASTEXITCODE) { throw 'building the load zone failed' }
}

# 4. what the create-room list needs
${key} = 'USERMAPS_' + ($Name.ToUpper() -replace '[^A-Z0-9_]', '_')
$tokens = 'default', 'mainline', 'alpha2', 'alpha3', 'ce2', 'ce3', 'cb1', 'cb2', 'cb3'
$version = ($tokens | ForEach-Object { "${_}_1" }) -join '|'
$row = @('') * 31
$row[0] = '<next NUMBER>'; $row[1] = $key; $row[2] = $key; $row[3] = "sfproto_$Name.tga"; $row[4] = $Name; $row[5] = 'pvp'; $row[6] = '4'; $row[7] = '18'
$row[18] = $version; $row[20] = '0'; $row[21] = '0'; $row[22] = '0'; $row[23] = '0'; $row[30] = '<next pvp SEQUENCE>'
Set-Content "$presence\comaplist_row.csv" -Encoding ASCII -Value ($row -join ',')
Set-Content "$presence\basemaps_arena_entry.txt" -Encoding ASCII -Value @"
{
	map			"$Name"
	longname	"$key"
	gametype	"dm war sab sab2 dom sd sd2 hc thc ctf koth"
	description	"$key"
	mapimage	"blank"
	environment	"$Environment"
	allieschar	"socom_141_desert"
	axischar	"opforce_composite"
}
"@
Set-Content "$presence\localize.txt" -Encoding UTF8 -Value "$key`t$DisplayName"
Set-Content "$presence\usermap_info_entry.py" -Encoding UTF8 -Value @"
# for USERMAP_INFO / NEW_LOCALIZE of usermaps\build_usermaps_patch.py ( only needed for a display name, a description,
# a create-room picture or an environment of your own: without an entry the script makes the row below by itself,
# with the name "$Name", the picture sfproto_$Name.tga and the environment "urban" )
USERMAP_INFO["$Name"] = dict(name="$key", desc="${key}_DESC", image="sfproto_$Name.tga", env="$Environment")
NEW_LOCALIZE["$key"] = u"$DisplayName"
NEW_LOCALIZE["${key}_DESC"] = u"$DisplayName"
"@
$projectNote = ''
if ($mine.Count) {
  $projectNote += "   From the asset project ( in front of the generated files of the same path, and rawfiles of $Name.ff as well ):`r`n"
  $projectNote += (($mine.Keys | ForEach-Object { "     scripts\$_" }) -join "`r`n") + "`r`n"
  $projectNote += "   Copy the whole scripts\ folder into <game>\main\sh1x_rawfiles\ ( the paths below it are the script paths ).`r`n"
}
if ($archives.Count) {
  $projectNote += "`r`n2a. streamed sounds`r`n"
  $projectNote += (($archives | ForEach-Object { "     main\$($_.Name)" }) -join "`r`n") + "  ->  <game>\main\iw_sound_NN.iwd  ( NOT a plain copy )`r`n"
  $projectNote += "   The streamed sound files ( sound\<file> ) of the map's sound aliases. The game only opens archives named like`r`n"
  $projectNote += "   its own ( iw_sound_NN.iwd, NN up to 23; 00 .. 03 are the game's ), decrypts those - the name is part of the key -`r`n"
  $projectNote += "   and plays a streamed file only out of an archive the SERVER has in its main\ too ( else: `"Impure client`" ).`r`n"
  $projectNote += "   So the plain archive is written again under a free name, for client and server:`r`n"
  $projectNote += "     codol_assetc --install <game> --iwd <game>\main\iw_sound_23.iwd" + (($archives | ForEach-Object { " --iwd-add main\$($_.Name)" }) -join '') + "`r`n"
  $projectNote += "   Without it the aliases exist and play nothing. The editor's install does this by itself.`r`n"
}
Set-Content "$presence\INSTALL.txt" -Encoding ASCII -Value @"
$Name - where the files go ( <game> = the Call of Duty Online 1.1.1.13 folder, <server> = CoD4x_Server )

CoDOL Radiant does all of this for a game folder and a script folder you name: File > Install the built map ..
( console: map_install game=<game> scripts=<script folder> list ). It keeps a backup of every file it replaces
( <file>.pre-$Name ), and Uninstall ( map_uninstall ) puts everything back. By hand:

1. the zones
     zone\$Name.ff, zone\${Name}_load.ff  ->  <game>\usermaps\$Name\        ( client and server read usermaps\<map>\ )
   The client has to be started as codoMP_client_shipRetail.exe with the install's Kernel33.dll ( unsigned zones ).

2. the level script
     scripts\maps\mp\$Name.gsc            ->  <game>\main\sh1x_rawfiles\maps\mp\
     scripts\maps\mp\${Name}_fx.gsc, scripts\maps\createfx\${Name}_fx.gsc go next to it only when the server should
     run the effects itself; the client takes both from the zone ( they are rawfiles of $Name.ff ).
   Without the level script the map still runs ( "no level script" ), without minimap setup.
$projectNote
3. the create-room list ( map pool row, arena entry, localized name )
   The client uses the first zone of client_shipRetail\patch\ that holds mp/comaplist.csv: codol_common.ff where
   the lobby's build made one, else usermaps_patch.ff. The editor's install adds the three things to that zone
   ( step "list" above ). For usermaps_patch.ff the install's own script does the same from the folders under
   usermaps\:
     python <game>\usermaps\build_usermaps_patch.py
   It finds usermaps\$Name\$Name.ff and adds by itself: a row of mp/comaplist.csv ( ID $Name, TYPE pvp, 4 - 18
   players, on for every build token ), an entry of mp/basemaps.arena ( environment urban ) and the localize key
   ${key} = "$Name". For another display name, picture or environment put the lines of
   presence\usermap_info_entry.py into the script's USERMAP_INFO / NEW_LOCALIZE first.
   What the script writes, for reference ( made here, not installed ):
     presence\comaplist_row.csv         the 31 cells of the row ( NUMBER and SEQUENCE are the next free ones )
     presence\basemaps_arena_entry.txt  the arena entry
     presence\localize.txt              key <tab> text

4. pictures
   The minimap ( material compass_map_$Name ) and the loading picture are inside the zones: nothing to install.
   The create-room picture is an image sfproto_$Name ( 200 x 120 ): usermaps\build_map_images.py makes
   images\sfproto_${Name}_usermap.iwi from <map>_load.png for the map pack's .iwd, and the map has to be listed in
   MENU_IMAGE_MAPS of build_usermaps_patch.py. Without it the list shows the default picture.

5. playing it without the list: start the lobby with CODOL_FORCE_MAP=$Name ( any map name works that way ).
"@
"packaged $Name -> $Out"
Get-ChildItem $zone -Filter "$Name*.ff" | ForEach-Object { '  ' + $_.Name + '  ' + $_.Length + ' bytes' }
