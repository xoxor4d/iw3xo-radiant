# iw3xo-radiant for Call of Duty Online 1.1.1.13

This branch (`codol-1.1.1.13`) turns iw3xo-radiant into a workbench for Call of Duty Online
1.1.1.13 ("C1", an IW4-derived engine, fastfile version 334). It keeps everything upstream does
and adds a **CoDOL** menu.

- [CoDOL Radiant: the standalone editor](#codol-radiant-the-standalone-editor) - [the tool chain](#the-tool-chain),
  [3D editing](#3d-editing), [a map of your own](#a-map-of-your-own) step by step, [your own assets](#your-own-assets)
- [What works](#what-works) ( the radiant plugin )
- [What does not exist yet](#what-does-not-exist-yet)
- [Setup](#setup)
- [Script Workbench](#script-workbench)
- [Map Entities](#map-entities)
- [Game window and hot reload](#game-window-and-hot-reload)
- [Console commands and dvars](#console-commands-and-dvars)
- [Code layout, build and tests](#code-layout-build-and-tests)
- [Roadmap](#roadmap)

## CoDOL Radiant: the standalone editor

`CoDOLRadiant.exe` is its own program. It does not load, hook or need CoD4 radiant or the CoD4 mod tools: maps,
models, materials and shaders are read from the 1.1.1.13 install and drawn with the game's own shaders.

```
cmake -S editor -B build-editor -G "Visual Studio 17 2022" -A x64
cmake --build build-editor --config Release
build-editor\Release\CoDOLRadiant.exe
```

Settings are in `codol_editor.cfg` next to the exe ( File > Settings ); the CoDOL install folder is the one that is
needed. What it is made of:

| Part | Role |
|---|---|
| `editor/` | the application: window, Direct3D 9 device, docking UI, 3D view, browsers |
| `codol_zone/` | fastfile reader ( and writer, in progress ) |
| `codol_render/` | materials, technique sets, shaders, images |
| `codol_view/` | map renderer with the editor API: picking, moving / hiding / adding models, meshes with game materials |
| `codol_edit/` | map document: `.map` reader / writer, brushes, patches, entities, selection, undo, editing operations, entity classes taken from the shipped maps |
| `src/codol`, script workbench, game bridge | shared with the radiant plugin |

### The tool chain

| CoD4 mod tools | here | what it does today |
|---|---|---|
| Radiant | `CoDOLRadiant.exe` | open a map of the install or one of your own, edit entities / models / brushes in 3D and 2D views with undo, light entities with a panel of their own, save as `.map` |
| `cod4map`, `cod4rad`, the map's linker step | `codol_bsp.exe` ( File > Build map ) | builds the whole zone of a map of your own from its `.map` alone: collision ( brushes, terrain patches, static models ), BSP, cells and portals, render geometry, light bake ( sun, lights, bounce in the materials' colours, light grid, reflection probes ), primary lights ( spot, omni, scriptable ), minimap picture, load zone, level scripts. See [A map of your own](#a-map-of-your-own). |
| editing a shipped map | `codol_mapc.exe` ( File > Compile map ) | applies the edits of a `.map` to the fastfile of the map it was imported from: entities, static models moved / deleted / added, collision brushes, render faces of new world brushes and of patches ( no patch collision ), and the project's assets ( animations, sounds, effects, models ). It patches the shipped zone; no light bake. |
| Asset Manager, `converter` | the Assets window, `codol_assetc.exe` | your own textures, materials, models, animations, sounds and effects: PNG / TGA / DDS, `.mtl` or the material entries of a CoD4 `.gdt`, `xmodel_export`, `xanim_export`, soundalias `.csv` + `.wav`, `.efx` -> native assets in `mod\<map>\<map>_assets.ff`. Materials and models are shown by the editor at once; animations, sounds and effects are for the level script of the built map. See [Your own assets](#your-own-assets). |
| the zone source `.csv` of a map, `linker_pc` on it | `tools\package_map.ps1` ( part of File > Build map ) | puts the project's assets, animtrees, animlist and scripts into the map zone ( `codol_assetc --carry`, `--rawfile` ), adds the minimap material, builds the load zone and writes the package |
| `linker_pc` | `codol_linker.exe` | reads and writes fastfiles ( all 153 of the install round trip byte for byte ), replaces entities / rawfiles / string tables, takes assets from other zones, builds a zone of rawfiles |
| script editing | Script Workbench, rcon reload | unchanged |

Compiled fastfiles are unsigned ( `IWffu100` ); the signed form needs a key the client does not have. Per the notes
in the install, the retail client only loads unsigned zones from disk with the `Kernel33.dll` patch; the dedicated
server reads both. Maps built from scratch and edited shipped maps have both been loaded on the dedicated server and
in the client - on one machine, over loopback, with a test account, the lobby emulator of the install and a
dedicated server ( `codol_dedrun` ); nobody has played them over a network. `codol_bsp/README.md` and
`codol_mapc/README.md` list what was checked where, with the pictures, and what is still open.

### 3D editing

Brushes and patches are made and shaped in the 3D view as well as in the 2D views. The left mouse button belongs to
the tool that is on ( second row of the view's toolbar ); right drag looks around and flies ( W A S D ), middle drag
pans, the wheel moves forward and back - as before. Every drag is one undo step with a name ( Move, Resize, Move
face, Move vertices, Move edges, Clip, Create box .. ), snaps to the grid ( `[` `]`, the Snap box ), keeps the
texture lock ( Shift+T ) and shows in the 2D views while it happens. Esc during a drag takes it back. The two or
three lines at the bottom of the view say what the mouse and the keys do in the tool that is on.

A camera drag of the 3D view ( look / fly, pan, drive ) and of the model and effect previews ( turn ) holds the
mouse: the cursor is hidden, kept inside the view and put back to its middle every frame, so the motion never stops
at the edge of the view or the screen, and when the button is let go the cursor is back where the drag started.
Escape, Alt-Tab or anything else that takes the editor out of the foreground lets the cursor go at once. Input fed
with `ui_mouse` ( unattended runs ) never moves the real cursor.

| Tool | Key | Left button |
|---|---|---|
| **Select** | Esc | Click selects ( Ctrl or Shift adds / removes ). **Drag on the selection**: it moves, on the world plane that faces the camera most ( look down: on the floor; look along: up and sideways ). **Drag next to a selected brush**: the faces on that side follow the mouse - Radiant's resize; they are outlined while the mouse is there, and a drag that starts past a corner takes both faces. **Squares on the selection's box**: that side of every selected brush. **Ctrl+Shift click** selects a face ( again: deselects it ); **drag a selected face**: it moves along its normal, with every other selected face. **Shift drag**: everything in the rectangle is added to the selection. **Alt click**: the object behind the selected one under the mouse ( Alt+Shift click adds it ) - through walls and into stacks. With nothing selected a left drag drives the camera, as before. |
| **Vertices** | V | Handles on the corners of the selected brushes ( green ) and the control points of the selected patches ( pink ). Click a handle: select it; Ctrl or Shift click adds / removes; Shift drag: the handles in the rectangle. Drag a handle: the selected handles move with it, on the plane that faces the camera most ( with **Alt** along the third axis instead ); the handle that is dragged lands on the grid, also when it was off it. The arrows of the gizmo on the selected handles move them along one axis. A corner that brushes share moves in all of them; sides that are no longer flat are split into triangles ( the brush is rebuilt as the hull of its corners ). **Control points of patches** are the Patch menu's vertex selection ( below ): Alt click selects the column of a point, Alt + Shift click its row ( Ctrl adds ); locked points are red and stay; Delete removes a whole selected row / column; the rotate / scale gizmo ( the view's toolbar, R ) turns / scales a set of them about its centre. |
| **Edges** | E, while the handles are shown | Handles on the middle of the edges ( blue ); an edge moves as a whole, the same way. E again or V: back to the vertices. |
| **Clipper** | C | Click on a surface: a clip point; two or three, drag to move them. Two points cut straight through the surface the first was put on, three define the plane. The cut is drawn on every selected brush, the arrow shows the side that stays. Enter clips, Ctrl+Enter keeps the other side, Shift+Enter splits; Esc removes the points, then leaves. Points placed in a 2D view and in the 3D view are the same points. |
| **Create** | B | Drag a rectangle on the surface under the mouse - the ground, a wall, the top of a brush, the floor of an empty map: a brush stands on it, as high as the last selection was ( 64 when there was none ). It is selected: drag the squares on its box to size it. The list next to the button picks the shape: box, cylinder and cone ( with a number of sides ), wedge, terrain patch. |

The keys that were there stay: W / E / R pick the gizmo ( move / rotate / scale ) - E is the edge mode only while the
vertex handles are shown, where nothing is rotated -, arrows and PageUp / PageDown nudge the selection, Space clones,
Delete deletes, F goes to the selection. New, for the faces that are selected ( or every face of the selected brushes,
as in Radiant ): **Shift + arrows** shift the texture by the grid size, **Ctrl + arrows** scale it by 5 %,
**Shift + PageUp / PageDown** rotate it by the angle snap.

**Materials**: drag a material out of the Materials window and drop it on a face in the 3D view; with Shift held at
the drop it goes on every face of that brush; on a patch it replaces the patch's material. The selection does not
change.

What a drag works on is the document: in a shipped map the world is the zone's render geometry, which no tool
changes - the tools see its surfaces ( to put a clip point or a new brush on ) and the collision brushes
( `Brushes: all` shows them ).

For unattended runs the tools have a console command that calls what the mouse calls, at pixels of the view or at
the pixel a world point projects to: `ed3d tool <object|vertex|edge|clip|create>`, `ed3d primitive <box|cylinder|cone|wedge|terrain> [sides]`,
`ed3d press <px> <py> [shift] [ctrl] [alt]` / `drag <px> <py>` / `release` / `click ..`, the same with a world point
( `wpress x y z`, `wdrag`, `wclick` ), `ed3d drop <material> <px> <py> [brush]` / `wdrop`, `ed3d handles` ( the handles
with their pixels ), `ed3d faces` ( the faces of the selection ), `ed3d mouse <px> <py>` / `wmouse x y z` ( the injected
mouse of `ui_mouse`, on a pixel of the view ). `tests\ed3d\run.ps1 -Exe <CoDOLRadiant.exe> -Script tests\ed3d\<name>.cmds`
runs a command list and checks its `expect` lines against the console log: `object` ( Select tool ), `handles`
( vertices, edges, clipper, create, material ), `shipped` ( in a map of the install ) go through `ed3d press`;
`mouse` and `matdrop` feed the mouse and the keyboard themselves ( `ui_mouse`, `ui_key` ) and so go through the
gui, the gizmo and the shortcuts. `tests\ed3d\run_all.ps1 -Exe <CoDOLRadiant.exe>` runs them all.

**Opening another map** is part of those runs since the editor crashed on it ( 2026-11-12, "from mp_abandon_sh_mk to
mp_hunger" ): a map opened in the middle of a frame - by "Discard" in the unsaved changes question, by a command
typed into the Console window, by the Assets window registering a zone again - released the picture of the 3D view
and the thumbnails while the frame that was being built still drew them. The renderer now keeps the textures it
handed out until the next frame starts ( `map_renderer::release_retired`, called by `scene::sync` ). `switch_discard`
is the crash as it happened, `switch_click` the double click in the Maps window, `switch_stress` ( written by
`make_switch_stress.py [rounds]` ) 33 switches over four shipped maps and a map of your own through the console
window, the Maps window, the question, `map_new`, `map_load` and `map_close`, with a selection, the Materials and the
Preview window open; after each it prints `ed_mem` ( working set, private bytes, free texture memory of the
device ).

### A map of your own

From an empty editor to a map you walk around in. Every step is a menu entry and a console command; what was seen
of each step in the game, and where, is said with it - the details and the pictures are in `codol_bsp/README.md`.

**0. Once: settings.** File > Settings: the CoDOL install folder ( the one with `main\` and `client_shipRetail\` ).
The script folder ( `<game>\main\sh1x_rawfiles` ) is set and created by itself ( [The game's scripts](#the-games-scripts) ). The first build scans the
install's zones ( about 16 s ) and the first asset build its materials ( 20 s ); both are kept.

**1. File > New map**, a name like `mp_mymap` ( `a-z 0-9 _` ). It is saved as `map_source\mp_mymap.map`; its
asset project is `mod\mp_mymap\` ( [Your own assets](#your-own-assets) ).

**2. Draw the map.** Brushes, curve and terrain patches ( the **Brush** and **Patch** menus:
[Brushes and patches](#brushes-and-patches-the-brush-and-patch-menus) ), `misc_model`s ( static models ),
`reflection_probe`s.

* The map has to be **closed** ( no way from an entity to the void ) or stand in a sky box; the build reports a
  leak with its position.
* **Players do not collide with static models** ( the engine's rule, bullets do ): put `clip` brushes around models
  one should not walk through.
* Doorways and windows between rooms become portals by themselves ( planar openings in a wall ); `portal` brushes
  of your own switch that search off.
* Things that repeat can be **prefabs** ( `misc_prefab` entities that stand for another `.map` ), a large map can be
  worked on through a **region**, and hidden faces get caulk with Selection > CSG > Auto Caulk:
  [Selecting, regions, filters, prefabs](#selecting-regions-filters-prefabs).
* *The 3D view* draws a map of your own in a *stage*: an empty world that `codol_bsp` compiles from the light keys
  of the worldspawn ( a second or two, kept in `compiled\cache\stage\` ). Brushes with their materials, models
  where their entities stand, lit by the sun, sky and ambient light the map will have. **It is not the light
  bake**: light entities are drawn as shapes, their light and all shadows of brushes are what File > Build map
  makes.

**3. Light.** Nobody has to type keys: with nothing selected the **Entity** window shows *Light of the map* ( sun,
its colour and direction with a small compass, sky light, ambient light; a field whose key is not set shows the
compiler's default, the `x` behind a field takes the key away ), with a light selected its **Light** section
( colour, intensity, radius, kind, falloff, cone, "scriptable" ). "Show in 3D" builds the stage again when the
light keys changed ( `ed_restage` ). While a light is selected the views draw where it reaches: circles of its
radius, or the cone of a spot as the compiler reads it. Two keys have no field and are typed: `radiosityscale` of
the worldspawn and the cone of an omni light ( `ed_key <key> <value>`; it goes to the selected entity, or to the
worldspawn when nothing is selected ).

| what | keys | in the game |
|---|---|---|
| the map's light | worldspawn `sundirection`, `suncolor`, `sunlight`, `ambient` + `_color`, `skylight` + `skycolor`, `radiosityscale`. Without them: a sun of 1.3 from `-50 40 0`, ambient 0.1, sky 0.4, bounce 1 | sun with run time shadows of players and models, sky light, bounced light in the colour of the materials it comes off |
| a baked light | `light` with `radius` ( 600 ), `intensity` ( 1 ), `_color`; it is down to a quarter at half its radius | in the lightmaps and the light grid; nothing that moves throws a shadow in it |
| a primary spot | `spawnflags 2`, `fov_outer` / `fov_inner` / `exponent`, `angles` or a `target` | lights world and models at run time; players and script models throw shadows in it ( seen ) |
| a primary omni light | `spawnflags 1` | lights all around at run time. Players and models throw shadows **only inside its cone** - straight down, 120 degrees, full strength inside 60, as in the game's own maps; `fov_inner` / `fov_outer` / `angles` change the cone ( seen and measured ) |
| a light a script changes | "scriptable" ( `spawnflags 4` ), a `targetname`; `maxmove` / `maxturn` when the script moves or turns it | the script gets it with `getEnt` and calls `setLightIntensity`, `setLightColor`, `setLightFov`, sets `.origin` ( seen ). What the light could reach within `maxmove` / `maxturn` is lit by it at run time where no other light is |
| the falloff | `def`: `light_point_linear` ( default ), `light_point_quadratic`, `tungsten_lamp` .. - any definition of the install | three of them seen |

Two things a mapper runs into: a surface is lit at run time by **one** primary light - floors and walls are cut
where the reach of two of them meets, along a grid of 32 units; and on the test server a script moves a light only
by setting `.origin` ( `moveTo` and `linkTo` on a light are script errors there, though the game's own scripts use
them ). All keys with their defaults: `codol_bsp/README.md`, "Light keys".

**4. Your own textures, models, animations, sounds, effects** - optional: [Your own assets](#your-own-assets).
Import them, press **Build** in the Assets window, use them. A level script of your own
( Assets > **Level script** ) is where animations, sounds and effects are played from.

**5. What the game's scripts ask of every map.** Spawn points ( `mp_dm_spawn`, `mp_tdm_spawn_*` .. ), one
`mp_global_intermission`, `node_pathnode`s for bots. **Edit > Add gameplay entities** ( `ed_gameplay` ) adds what
is missing besides: the care package with its collision ( without it `_airdrop.gsc` throws a script error at every
map start ) and a helicopter path ( `heli_start`, `heli_loop_start` .. ). The build names what is missing.

**6. File > Build map** ( `map_build`; `map_build fast` while you work on the layout ). The compile, the light bake
and the packaging follow each other; the result is a package in `compiled\<map>\`:

| | |
|---|---|
| `zone\<map>.ff`, `zone\<map>_load.ff` | the map zone ( with the minimap material `compass_map_<map>` ) and its load zone; a picture `map_source\<map>_load.png` becomes the loading screen |
| `scripts\maps\mp\<map>.gsc` ( + `_fx`, `createfx` ) | the level script for the dedicated server's script folder: generated, or your own |
| `main\zz_<project>_sounds.iwd` | only when the project has streamed sounds: their files, as a plain archive |
| `presence\` | the row, arena entry and name for the create-room list, and `INSTALL.txt`: where every file goes |
| `work\<map>.report.txt`, `work\scripts\` | what the compiler did: counts, timings, warnings; the scripts as generated |

From the asset project the zone gets the materials and models the map uses, every animation, sound alias, effect
and model of the project's zone, the animtrees and the animlist, and the files of the project's `scripts\` as
rawfiles ( [In the built map](#in-the-built-map) ). Read the report's warnings: a leak, a material no zone has
( its faces are not drawn ), a light definition that was replaced.

**7. File > Install the built map ..** ( `map_install` ). It asks for the game folder, the server's script
folder and whether the map goes into the create-room list. Three things have to end up in the right place:

| from `compiled\<map>\` | to | why there |
|---|---|---|
| `zone\<map>.ff`, `<map>_load.ff` | **`<game>\client_shipRetail\chinese\`** - give it as the zone folder: `map_install game=<game> zones=<game>\client_shipRetail\chinese`. Without a zone folder they go to `<game>\usermaps\<map>\` | client and server can read both places, but the **lobby** starts a room's map only when `<map>.ff` is in *its* zone folder - `client_shipRetail\chinese` of the install, or the folder `CODOL_ZONE_DIR` names; it never looks into `usermaps\`. Every run in the game described here had the zones in `client_shipRetail\chinese`. A dedicated server with another base path needs the zone under *its* `client_shipRetail\chinese` too |
| `scripts\..` | the server's script folder | the dedicated server reads level scripts from its script folder, never from a zone. Without the level script the map still runs, but nothing of your script does |
| `main\zz_*_sounds.iwd` | `<game>\main\iw_sound_NN.iwd` ( written again: encrypted, every entry deflated, under the first free number from 23 down ) - **in `main\` of the client and of the server** | the game opens only archives named like its own, and plays a streamed file only out of an archive the *server* has as well ( the server tells the client which archives are "pure"; a client with an archive the server lacks ends with "Impure client detected" ). Client and server of one install share `main\`; **a server that runs from another folder needs a copy of the archive in its own `main\`**. By hand: `codol_assetc --install <game> --iwd <game>\main\iw_sound_23.iwd --iwd-add zz_<project>_sounds.iwd` |
| `presence\comaplist_row.csv`, `basemaps_arena_entry.txt`, `localize.txt` | the create-room list of `<game>`, with "Add to the create-room list" ( `list` ) | below |

* A file that is in the way and differs is kept once as `<file>.pre-<map>` before it is replaced; a file that is
  the same already is left alone - installing twice changes nothing.
* Everything written is noted in `compiled\<map>\work\installed.txt`. **Uninstall** ( the same window, or
  `map_uninstall` ) undoes exactly that: new files are deleted, replaced files get their backup back, the map
  comes out of the list, `usermaps\<map>\` goes when it is empty.
* The client has to be started as `codoMP_client_shipRetail.exe` with the install's `Kernel33.dll`: compiled zones
  are unsigned, and per the install's notes only that start loads unsigned zones from disk. No change to
  Kernel33, lobby or server is needed for a new map name.

**8. Play.**

* **Without the list**: start the lobby with `CODOL_FORCE_MAP=<map>` ( and `CODOL_FORCE_GAMETYPE` ), then
  Multiplayer > Start > Search > Ready in the client. This is how every map of these pages was entered.
* **From the create-room list**: install with "Add to the create-room list". The list is three things ( the
  install's `usermaps\build_usermaps_patch.py` describes how the client reads them ): a row of the string table
  `mp/comaplist.csv`, an entry of the rawfile `mp/basemaps.arena` and a localize entry for the name. The client
  keeps the first asset of a name it links and links the zones of `client_shipRetail\patch\` first, so the list is
  whichever zone of that folder holds `mp/comaplist.csv` - in the install of this machine `codol_common.ff`. The
  editor reads that zone, adds the row, the arena entry and the name, writes it again, reads it back and only then
  puts it in place ( without such a zone it makes `usermaps_patch.ff` from the stock list ). `map_list [game
  folder]` prints which zone is the list and its last rows. **Written and loaded by the client, not seen**: the
  client runs its menus with the list zone the editor wrote ( 97 rows ), but its *Create Room* is locked below
  player level 5 and the test account is level 1, so the row was not looked at and no map was started from it.
  Whatever build makes `codol_common.ff` is outside the install and the lobby's folder; when it runs again the row
  is gone with the file - `presence\` is the input for that build's map table.

**What is still open for a map of your own** ( the whole list: [What does not exist yet](#what-does-not-exist-yet) ):
animations of your own have not been seen *moving* in the game ( the test server does not send script model
animations to the client, shipped ones included ); the create-room row was not seen; primary lights cut surfaces
along a coarse grid and an omni light shadows only inside its cone; static models have no player collision; a
script moves a light only through `.origin` on the test server.

Console: `map_new <name>`, `ed_gameplay`, `map_build [fast]`, `map_install [zone folder] [script folder]` ( the
folders of File > Settings when none is given ), `map_install game=<folder> [scripts=<folder>] [zones=<folder>]
[list]`, `map_install window`, `map_uninstall [map]`, `map_list [game folder]`.
The radiant plugin ( `iw3r.dll` inside CoD4 radiant ) described below still builds, but the standalone editor is
where the work goes.

### Brushes and patches: the Brush and Patch menus

What CoD4 radiant's **Brush** and **Patch** menus do, written for the editor's own document ( `codol_edit`,
`patch_ops.hpp` ); nothing of it calls into CoD4 radiant. Every item is one undo step, is greyed with a tooltip
that says why when it does not apply, and has a console command - so a whole map can be scripted
( `CODOL_STARTUP_CMDS` ).

**What a new shape fills.** The box of the selected brushes; with no brush selected, the last box that was
selected or drawn in a 2D view; in a command, `at x1 y1 z1 x2 y2 z2`. A brush primitive *replaces* the selected
brushes ( the first keeps its id, entity, layer, contents and the material of its first face ). A patch made from
a selected brush deletes it ( "Delete Original Selection", `keep` in a command ) and takes its material.

**Brush**

| Menu | Command | |
|---|---|---|
| Arbitrary Sided Cylinder .. | `brush_cylinder [sides] [x\|y\|z]`, `brush_sided ..` | 3 - 64 sides around an axis; the faces touch the box, as radiant's do ( 4 sides are the box itself ) |
| N Sided > 3 .. 9 | `brush_sided <n>` | |
| Cone .. | `brush_cone [sides] [x\|y\|z]` | the same base, a point at the far end of the axis |
| Sphere .. | `brush_sphere [sides] [x\|y\|z]` | 4 - 16 slices, half as many bands; inside the box |
| Box, Cylinder, Wedge | `brush_box`, `brush_cylinder 8`, `brush_wedge [x\|y]` | |
| From Points .. | `brush_points x y z x y z ..` | the convex hull of four or more points |
| ( the dialog ) | `brush_dialog [box\|wedge\|cylinder\|cone\|sphere]` | |

**Patch.** A patch is a grid of control points, *columns x rows*. A **curve** ( odd numbers of both ) is a
quadratic bezier surface: every second control point lies on it. A **terrain** patch ( `mesh` in the file ) is
the grid itself, two triangles per cell.

| Menu | Command | |
|---|---|---|
| Simple Curve Patch .. ( Shift+P ) | `patch_curve [columns rows] [x\|y\|z] [keep]` | flat, on the far side of the box across its thinnest axis |
| Simple Terrain Patch .. | `patch_terrain [columns rows] [x\|y\|z] [keep]` | 2 - 16 each way |
| Curve To Terrain | `patch_curve_to_terrain` | the vertices the curve is drawn with become a terrain mesh |
| Faces To Terrain | `patch_faces_to_terrain [delete]` | one 2 x 2 terrain patch per selected face ( or per face of the selected brushes ) with 3 or 4 corners; keeps the texture alignment |
| Primitives > Bevel, Square Bevel, End Cap, Square End Cap, Cylinder, Dense Cylinder, Very Dense Cylinder, Square Cylinder, Cone | `patch_prim <bevel\|square_bevel\|endcap\|square_endcap\|cylinder\|dense_cylinder\|very_dense_cylinder\|square_cylinder\|cone> [x\|y\|z] [keep]` | 3x3, 5x3, 5x3, 7x3, 9x3, 9x5, 9x9, 9x3, 9x3; the axis is chosen in the submenu |
| Vert Edit Dialog | `patch_dialog vertex`, `patch_color r g b [a]`, `patch_noise x y z [seed]` | vertex colour and noise for the selected vertices, else the whole patch |
| Advanced Edit Dialog ( Shift+S ) | `patch_dialog advanced`, `patch_vsel <none\|all\|row N\|column N\|i j ..> [add]`, `patch_move dx dy dz` | the control points as a table: click a cell, a row or a column header; position, texture and lightmap coordinates of a vertex; subdivision; lock |
| Insert > Insert 2 Columns / Rows | `patch_insert <columns\|rows> [index]` | at the start, or in the segment of the selected vertices. A curve keeps its shape exactly ( the segment is split ) |
| Insert > Add 2 Columns / Rows ( Ctrl+Shift+Num + / Ctrl+Num + ) | `patch_add <columns\|rows>` | at the end |
| Insert > Add Terrain Row / Column | `patch_terrain_add [column\|row] [after]` | one line after the row / column of the selected vertices |
| Insert > Extrude Terrain Row | `patch_extrude [column\|row] [start\|end]` | one more line beyond the edge |
| Delete > First / Last 2 Columns / Rows ( Ctrl+Shift+Num - / Ctrl+Num - ) | `patch_delete <first\|last> <columns\|rows>` | |
| Delete > Remove Terrain Row / Column | `patch_terrain_remove [column\|row] [index]` | |
| Matrix > Transpose ( Ctrl+Shift+M ) | `patch_transpose` | |
| Matrix > Re-disperse Vertices / Columns / Rows | `patch_redisperse <vertices\|columns\|rows>` | vertices: even spacing along the lines the control points form; columns / rows: evenly on the straight line between the first and the last |
| Weld > Connect Selection / Weld | `patch_weld [tolerance]` | vertices of the selected patches closer than 0.1 units ( Tolerant Weld: one grid step ) become one place |
| Weld > Weld to Brush Vertices | `patch_weld_brushes [tolerance]` | onto brush corners |
| Weld > Weld to Model / Prefab Vertices | `patch_weld_models` | the selected vertices ( else all of the selected patches ) onto the nearest vertex of a model ( its mesh as the 3D view draws it ) or of a prefab's contents within the tolerance |
| Weld > Snap To Brush Vertices / Snap To Model / Prefab Vertices While Dragging | `patch_snap_brushes [0\|1]`, `patch_snap_models [0\|1]` | a dragged patch vertex ( 2D or 3D, the one the mouse holds ) lands on such a point within one grid step; the others keep their distance to it |
| Weld > Tolerant Weld, Weld Equal Patch Points During Moves | `patch_tolerant [0\|1]`, `patch_weld_moves [0\|1]` | the second: a vertex dragged in a 2D view or the 3D view ( also by the rotate / scale gizmo ) takes along every vertex of the selected patches in the same place ( the seam of a cylinder ) |
| Inc / Dec Subdivision | `patch_subdiv <+n\|-n>` | curves: the subdivision number goes down ( finer ) / up until the tessellation changes, as in radiant; 1 - 16 quads per segment, in the views and in the compiled map ( below: the size line ) |
| Split | `patch_split [column\|row] [index]` | two patches, at the line of the selected vertices or in the middle |
| Invert ( Ctrl+I ) | `patch_invert` | faces the other way; a terrain keeps its triangles |
| Thicken .. ( Ctrl+T ) | `patch_thicken <units> [seams 0\|1]` | a second surface behind the patch, facing away, and strips that close the edges |
| Cap .. ( Shift+C ) | `patch_cap [auto\|bevel\|endcap\|inverted_bevel\|inverted_endcap\|cylinder] [first\|last\|both]` | closes the open ends ( first / last row ) of a curve; the caps face away from it and get a flat texture |
| Turn Terrain Edges | `patch_turn_edges`, `patch_turn_edge <column> <row>` | the diagonal of the quads the selected vertices start, else of every quad |
| Lock / Unlock Vertices | `patch_lock`, `patch_unlock [all]` | locked vertices ( red in the 2D views ) do not move when vertices are dragged |
| Cycle Cap Texture ( Ctrl+Shift+P ), Cap Texture | `patch_cycle_cap`, `patch_cap_texture [x\|y\|z]` | flat projection along a world axis |
| Natural Texture, Fit Texture | `patch_natural [units per texel]`, `patch_fit [x y]` | |
| Show Patches As Wireframe | `patch_show [0\|1]` | |
| Drill Down Vertices | `patch_drill [0\|1]` | off: a grab in a 2D view takes only the vertex nearest to the viewer |
| Cycle Terrain Edge Mode | `patch_edge_mode [0\|1]` | a click on a terrain quad in the 3D view turns its diagonal |
| | `patch_info` | the selected patches: size, subdivision, bounds, turned edges |

**Patch vertices.** One selection of control points for every view - the vertices of one selected patch, what
"the selected vertices" above means ( `ui::patch_vertex_selection` ). In *vertex mode* ( V ) of a 2D view a patch
shows its control points; grabbing one selects it ( yellow; Shift adds ) together with the ones behind it, and
dragging moves them. The Advanced Edit dialog selects by row and column ( `patch_vsel` ). In the **3D view**'s vertex
tool the control points are handles: click selects one, Ctrl / Shift click adds / removes, Shift drag takes those in
the rectangle, **Alt click its column, Alt + Shift click its row** ( Ctrl adds ); with Drill Down Vertices a click
also takes the points under it on the screen. A drag moves the selection on the plane that faces the camera most,
the held point on the grid, with Weld Equal Patch Points During Moves and the snapping of the Weld menu; **locked**
points ( Lock Vertices ) are red in every view and do not move. The gizmo moves the selection; with the view's
Rotate / Scale gizmo it turns / scales the selected points about their centre ( `patch_vtransform rotate <deg>
[x|y|z] | scale <f>` does the same from the console ). **Delete** with points selected ( 3D or 2D vertex mode ):
a whole row / column of a terrain is removed ( `patch_terrain_remove` ), the first / last row or column of a curve
takes its pair along ( `patch_delete` ); anything else deletes nothing and says why - Esc drops the points, then
Delete deletes the patch. The selection made in a 2D view or the dialog is drawn in the 3D view, as handles in the
vertex tool and as marks in the others. `patch_vsel_info` prints it with positions ( `L`: locked ).
`tests\ed3d\patch_vertices.cmds` runs all of this with pictures.

**In the file** a curve is a `curve` block and a terrain a `mesh` block, as CoD4 radiant writes them: material,
`lightmap_gray`, an optional `smoothing` line, `columns rows 16 8` and per vertex `v x y z [c r g b a] t s t ls lt
[f 1]` ( texture coordinates in texels, lightmap coordinates, `f 1` = turned edge of the quad the vertex starts ).
Unedited patches are written back byte for byte.

**The size line is `columns rows <lightmap sample size> <subdivision>`.** Established from the CoD4 mod tools'
binaries and maps ( read only, 2026-11-12 ):

* `CoD4Radiant.exe` writes it with `"%i %i %g %i"` ( 0x445AEB ): the third number is a float of the patch
  ( `patchMesh_t` + 0x5048, written as 0 unless the flag at + 0x5043 is set ), the fourth an int ( + 0x14 ). Its
  reader ( 0x444CC0 ) takes the third with `atof` and sets the flag when it is not 0, the fourth with `atoi`,
  clamped to at least 1. A new patch has 0 and 8. The function that makes a patch's lightmap coordinates
  ( 0x4397B0 ) computes them from a sample size and stores it in + 0x5048: **the third number is the lightmap
  sample size** ( units per luxel; 16 as radiant's default ).
* Inc Subdivision ( 0x44CDE0 ) passes -1 to the fourth number, Dec +1, and repeats until the patch's vertex count
  changes, never going below 1; terrain is excluded. So **the fourth number is the subdivision of a curve, and
  smaller is finer**; that it is an error bound in world units ( the largest distance a quad may keep from the
  curve, as the quake family's patches have it ) is this editor's reading - the tessellation code of radiant and
  of `cod4map.exe` was not traced. `cod4map.exe` ( ParsePatch 0x428914 ) reads four ints, drops the third and
  keeps the fourth.
* 406 `.map` prefabs of the stock CoD4 mod tools ( 481,000 patch blocks ): curves have sample size 16 ( 2,859 ),
  0, 4, 512, 8, 32, 1024 and subdivisions 8 ( 2,306 ), 1, 3, 2, 5, 4, 12, 7, 34, 23 .. 209; terrain meshes always
  8 ( 20 blocks: 1 ). Neither number follows the smoothing, contents or tool flags lines. Two open-source
  OBJ-to-CoD-map converters write `2 2 16 8` without naming the numbers; no documentation names them.

So here: `patch::lightmap_sample_size` and `patch::subdivision` ( `codol_edit` `model.hpp` ), both kept exactly as
read. A curve is drawn and compiled with n quads per segment, the smallest n with | P0 - 2 P1 + P2 | / ( 4 n² ) <=
the subdivision for every segment of three control points ( the most a quadratic bezier leaves its chord ), 1 - 16,
one n for the whole patch. Inc / Dec Subdivision step the number as radiant does: down ( up ) until n changes. The
lightmap sample size sets the luxel size of the patch in `codol_bsp` ( [Textures](#keys-preferences-toolbar-the-textures-renderer-effects-and-tools-menus) ) and the world
lightmap coordinates of an unlocked patch.

**In the built map** ( File > Build map ) every patch - curves at their subdivision, terrain as it is - becomes
drawn triangles with a lightmap and collision triangles of the clip map's terrain. Triangles without area ( the
fan of a cap around its centre ) are dropped. Patches of a brush entity are not compiled ( the build says so ).
`codol_bsp_patch_tests` compiles a room with every patch kind and brush primitive.

**Not there:** `physics_box` / `physics_cylinder` blocks ( what CoD4 radiant's Brush > Box / Cylinder make for
physics presets ) - here Box and Cylinder make brushes; a third "show patches as" mode.

**In File > Compile map** ( an edited shipped map, `codol_mapc` ) the patches of the worldspawn are drawn: their
tessellation ( the triangles `codol_bsp` makes ) becomes a render surface of the patch's material that takes, per
vertex, the nearest texel of the shipped light bake that faces its way - the way new brush faces are lit there.
**Their collision is not compiled** ( the report says so ): put clip brushes where players and bullets have to stop
on them. Why, and what it would take: `codol_mapc/README.md`, "Patches". `codol_mapc_tests --only patches`; not seen in
the game. Patches of brush entities are left out in both compilers.
### Selecting, regions, filters, prefabs

What CoD4 radiant has in its File, Edit, View, Selection, Grid, Misc and Region menus, written again for this
editor ( nothing of it calls CoD4 radiant ). Every entry is a menu item, a console command and - where radiant
has one and the key is free here - a key; every change of the map is one undo step. The document side is
`codol_edit` ( `src/select_ops.cpp`, `src/prefab.cpp`, tests: `codol_edit_select_tests` ), the menus are
`editor/src/menu_selection.cpp`, `filters.cpp` and `prefabs.cpp`.

**File**

| entry | command | what it does |
|---|---|---|
| Import .map into the map .. | `map_import <file.map>` | brushes of the file's worldspawn join this worldspawn, its entities are added; the new objects are selected |
| Generate File > Save Selected .. | `map_save_selected <file.map>` | the selection as a complete `.map` with the worldspawn's keys and the layer table |
| Generate File > Save Region .. | `map_save_region <file.map>` | everything that touches the region ( the whole map without one ) |
| Generate File > Pointfile | `leak_load [report]` | reads the leak of the last File > Build map from its report ( `compiled\<map>\work\<map>.report.txt` ) and draws the line from the entity to the outside in the 2D and 3D views. `leak_next` / `leak_prev` ( Misc, Ctrl+Alt+K / Ctrl+Alt+L ) put the camera on its points, `leak_clear` takes it away |
| Generate File > Errorfile | `map_errors` | collects the brushes that are no closed convex volume ( the compiler leaves them out ); next / previous leak spot selects them one by one |
| Generate File > Createfx | `map_createfx` | says how many `script_fx` / `script_sound` entities the map has and where the build writes their script. There is no export of its own: `codol_bsp` generates `maps\createfx\<map>_fx.gsc` at every build |
| Build region | `map_build_region [fast]` | File > Build map of what the region shows: it is written with six walls of caulk around it to `compiled\region\<map>.map` and compiled under the map's name |

**Edit**

| entry | key | command | |
|---|---|---|---|
| Copy / Cut | Ctrl+C / Ctrl+X | `ed_copy`, `ed_cut` | the selection goes to the Windows clipboard as `.map` text: paste works in the same map, another map and another CoDOL Radiant |
| Paste | Ctrl+V | `ed_paste` | where the objects were |
| Paste At Cursor / Camera | Ctrl+Shift+V | `ed_paste here`, `ed_paste <x y z>` | under the mouse in a 2D view, else on what the camera looks at |
| Map Info | Ctrl+M | `ed_mapinfo [window]` | brushes, patches, entities, models, lights, prefabs, bounds, entities by class ( a click selects the class ), materials |
| Entity Info | | `ed_entinfo [text] [window]` | every entity with class, targetname, target / model and origin; a filter over all four; a click selects the entity and brings the views to it |
| Enter Prefab / Leave Prefab | | `prefab_enter`, `prefab_leave` | below |

**View**: Filter ( F, `ed_filter` ), Center 2D On Camera ( `view2d_camera` ), Center Camera On Selection
( `ed_focus` ), Up Floor / Down Floor ( Ctrl+PageUp / Ctrl+PageDown, `ed_floor up|down`: eye height above the next
brush top above / below the camera ), Hide Unselected ( Ctrl+H, `ed_hide_unselected` ), Show Last Hidden
( Ctrl+Shift+H, `ed_show_last` ), Show Grid Blocks ( `ed_gridblocks` ), Show Prefab Contents ( `prefab_show` ).

**Selection**

| entry | key | command |
|---|---|---|
| Select > Targetname / Classname | | `sel_targetname [pattern]`, `sel_classname [pattern]`: every entity with the value of the selected one; asks when nothing is selected |
| Select > By Key / Value .. | Ctrl+Shift+F | `sel_key <key> [pattern] [add]`: `*` is any text, no pattern: the key exists |
| Select > Connected | Ctrl+Shift+C | `sel_connected`: everything linked by `target` / `targetname`, in both directions, over any number of steps |
| Select > Complete Tall / Partial Tall / Touching / Inside | | `sel_tall`, `sel_partial_tall`, `sel_touching`, `sel_inside` ( `sel_volume <kind> [x|y|z]` ). The selected brush marks the volume; as in radiant it is deleted by all but Touching ( the undo step brings it back ), and the tall kinds ignore the axis of the view the mouse is over |
| Deselect, Clone, Invert, Delete | Esc, Space, I, Backspace | as before |
| Flip X / Y / Z | | `ed_flip <x|y|z>` |
| Rotate X / Y / Z, Arbitrary Rotation .. | Shift+D ( Z ) | `ed_rotate <axis> <degrees>`, `ed_rotate3 <x> <y> <z>` |
| Free Rotation, Free Scaling | R | the rotate / scale handles of the 3D view ( `ed_gizmo` ) |
| Scale > Lock X / Y / Z, Arbitrary Scale .. | | `ed_scale_lock <letters|none>`, `ed_scale3 <fx> [fy] [fz]` |
| CSG > Hollow, Subtract | | `ed_hollow`, `ed_subtract` |
| CSG > Merge | Ctrl+U | `ed_merge`: one brush out of the selected ones when together they are convex; faces keep their plane points and textures |
| CSG > Auto Caulk ( the selection / the whole map ) | Alt+C | `ed_autocaulk [all]`: a face that other opaque world brushes cover completely gets `caulk`. Clip, trigger, portal and hint brushes hide nothing, nor do materials named glass, fence, grate, foliage, decal, water. "The whole map" leaves brush entities alone |
| Clipper > Toggle, Clip, Split, Clip Other Side | C, Enter, Shift+Enter, Ctrl+Enter | `ed_tool clip`, `ed_clip_apply front|both|back` |
| Prefabs > Enter, Leave, Stamp, Convert Selection To Prefab .., Prefab Browser | | below |
| Connect Entities | Ctrl+K | `ed_connect` |
| Link Entities | | `ed_link`: the selected brushes join the brush entity that has a brush in the selection |
| Ungroup Entity | | `ed_worldspawn` |
| Cycle Group Selection | Tab, Shift+Tab | `sel_group [prev]`: one brush of the brush entity after the other |
| Make Detail / Structural | Ctrl+D / Ctrl+Shift+D | `ed_detail 1|0` |
| Make Weapon Clip | Ctrl+Shift+W | `ed_contents weaponClip [0|1]` ( again: off ) |
| Make Non-Colliding | Ctrl+Shift+- | `ed_contents nonColliding [0|1]` |
| Make Split Coplanar Geo / Don't Split | | `ed_toolflag splitGeo 1|0` |
| Don't Select > Curves, Entities, Sky, Models, World Brushes | | `ed_noselect <group> [0|1]` |

How the flags are in the `.map` and what the compiler does with them:

| in the brush block | set by | `codol_bsp` |
|---|---|---|
| `contents detail;` | Make Detail | does not split cells, hides no faces of other brushes |
| `contents weaponClip;` | Make Weapon Clip | the brush is drawn with its materials; its collision gets the contents of the tool material `clip_weap` ( `0x20C0` in the shipped maps ): bullets and grenades stop, players and AI walk through. Checked by a trace through the compiled clip map with a player mask and a bullet mask ( `codol_bsp_prefab_tests` ) |
| `contents nonColliding;` | Make Non-Colliding | drawn, no collision brush at all ( the same test ) |
| `toolFlags splitGeo;` | Make Split Coplanar Geo | **kept in the file, not used**: `codol_bsp` never merges coplanar faces of different brushes, so there is nothing to switch |

Several words stand on one line ( `contents detail weaponClip;` ), as radiant writes them. The caulk of this
game is the material `caulk`: every shipped map has it on its collision brushes ( contents solid, never drawn );
new brushes start with it and Auto Caulk uses it.

**Grid**: 0.25, 0.5, 1 .. 512 ( `ed_grid <size>`; the number row sets 1 2 4 8 16 32 64 256 512 and `` ` `` 0.5 as
in radiant ), Smaller / Larger ( [ ] ), Snap To Grid, Snap Selection To Grid ( Ctrl+G ), Show Grid Blocks ( the
1024 unit blocks in the 2D views ).

**Misc**

| entry | key | command |
|---|---|---|
| Find Brush .. | | `ed_find <entity number> [brush number]`: counted from 0 in file order, as `// entity N` / `// brush M` say |
| Go To Position .. | | `ed_goto <x> <y> <z>`: the camera there, the 2D views centred on it |
| -> / <- Leak Spot / Error | Ctrl+Alt+K / Ctrl+Alt+L | `leak_next`, `leak_prev` |
| Entity Color .. | K | `ed_color <r> <g> <b>`: `_color` of the selected entities |
| Models Replace .. | | `ed_model_replace <from> <to> [selected]`: `*` in `<from>` is any text |
| Cycle Preview Models | | `ed_models_cycle`: models shown / hidden ( the Models switch of the filter ) |
| Drop Selected To Floor, Orient To Floor When Dropping | | `ed_drop [orient]`: the selected point entities fall onto the brush or terrain below them; models end with their origin on it, other classes with the bottom of their box; with "orient" pitch and roll follow the surface |
| Plant New Models | | `ed_plant [0|1] [orient 0|1]`: a model that is placed falls to the floor by itself |
| Script Group, Vehicle Group, Dyn Entity dialogs, Delete Exportables | | **not applicable** ( greyed, the tooltip says why ): no shipped 1.1.1.13 map has a `script_group` or `script_vehiclegroup` key, `dyn_brushmodel` / `dyn_model` do not occur ( only 64 `dyn_hingeconstraint` ), and "exportables" belong to CoD4 radiant's Maya export |

**Region**: Region Off, Set XY ( what the top view shows, any height ), Set Tall Brush, Set Brush ( the brush that
marks the region is deleted, as in radiant ), Set Selected Brush ( it stays ) - `region <off | xy | tall | brush |
selected | x1 y1 z1 x2 y2 z2>`. A region is view state like hidden objects: only what touches its box is drawn,
picked and selected by "select all"; a yellow box and "region" in the menu bar show that one is set. Save Region
writes what touches it, Build Region compiles it inside a box of caulk.

**The Filter window** ( View > Filter, F ): a tick means shown. Brushes by what they are - structural, detail, clip,
weapon clip, non-colliding, caulk ( all faces ), triggers, portals, hint / skip, sky -, curves, terrain, all of
the world; entities by group - all, models, lights, path nodes, spawns, brush entities, script origins / structs,
info, prefabs, other - and any classname ( `*` allowed ). Below them the "don't select" switches: curves /
terrain, entities, sky, models, world brushes - shown, but a click, a drag rectangle and "select all" pass them
by. The filter is part of the document's visibility ( `codol_edit` `view_filter`, `select_filter` ): what is
filtered is drawn by no view and cannot be picked. `ed_filter` prints what is hidden, `ed_filter <group> [0|1]`,
`ed_filter class <pattern> [0|1]`, `ed_filter none`.

**Prefabs.** A `misc_prefab` entity names another map - `"model" "prefabs/crate.map"`, relative to `map_source` -
and stands for its contents at its `origin` and `angles`. The files are plain `.map` files of this editor.

* **In the views** the contents of every prefab are drawn where the entity stands: brushes and models solid in
  the 3D view, everything as green lines in all views ( yellow while selected ). The box of the entity is the box
  of its contents, so a click on it picks the prefab. Prefabs inside prefabs are drawn too. A prefab whose file
  is missing is a red cross. A file that changes on disk is read again.
* **Prefab Browser** ( View, `prefab_browser` ): the files of `map_source\prefabs\`, with their size and counts;
  a double click places one where the camera looks ( `prefab_place <prefabs/x.map> [x y z] [yaw]` ). No
  thumbnails.
* **Enter Prefab** ( with a prefab selected ) saves the map and opens the prefab's file as the map; the menu bar
  shows `prefab: <map> > prefabs/x.map` and a Leave button. **Leave Prefab** saves the prefab and opens the map
  again where it was, with the prefab selected. Prefabs inside can be entered in turn.
* **Stamp Prefab** ( `prefab_stamp` ) replaces the selected prefab by its brushes and entities ( one level: the
  prefabs inside it stay prefabs ). **Convert Selection To Prefab ..** ( `prefab_convert <name> [overwrite]` )
  writes the selection as `map_source\prefabs\<name>.map` around the middle of its floor and puts a
  `misc_prefab` in its place.
* **Compiling**: `codol_bsp` ( File > Build map ) and `codol_mapc` ( File > Compile map ) replace every prefab,
  and the prefabs inside prefabs, by its contents right after they read the map
  ( `codol_edit` `expand_prefabs_from_disk` ). The file is looked for in the folder of the `.map`, its parent
  folders and a `map_source` folder next to them; `codol_bsp --prefabs <folder>` adds a place. The report says
  `prefabs: N expanded into ..`; a prefab that is missing, or that contains itself, is a warning and compiles
  to nothing. Tests: `codol_edit_select_tests` ( expansion, turned and nested prefabs, cycles, stamp, convert ),
  `codol_bsp_prefab_tests` ( a compiled map with a turned and a nested prefab, traced ).
* `prefab_info` lists the prefabs of the map with what each one holds, `prefab_list [text]` the files.

Default keys that differ from radiant, because the key is taken here: Hide Unselected is Ctrl+H ( Alt+H, radiant's,
is its second key ), Select Connected is Ctrl+Shift+C ( Ctrl+Alt+E selects whole entities ), next / previous leak
spot are Ctrl+Alt+K / Ctrl+Alt+L ( second keys: radiant's Ctrl+Shift+K / L ), the floors are Ctrl+PageUp /
Ctrl+PageDown ( PageUp / PageDown nudge the selection ), the clipper is C. Every key can be changed: Edit > Edit
Hotkeys.

### Keys, preferences, toolbar; the Textures, Renderer, Effects and Tools menus

What the top bar of CoD4 Radiant has beyond the menus above, natively ( `editor/src/hotkeys.cpp`,
`action_table.cpp`, `preferences.cpp`, `toolbar.cpp`, `menu_textures.cpp`, `menu_renderer.cpp`, `effects.cpp`,
`tools.cpp`; shared declarations in `ext.hpp` ).

**Actions and keys.** Everything the editor can be told to do is an *action*: an id ( `edit.clone` ), a label, a
default key, a function. Menu entries, toolbar buttons, keys and the console all run the same action:
`act <id>` runs one, `actions [text]` lists them with their keys. A new shortcut is one line -
`actions::add( "edit.clone", "Clone", "Space", [] { clone_selection(); }, actions::views );` in the file that owns
the function, or `command_action( "patch.invert", "Invert patch", "Ctrl+I", "patch_invert" );` in
`action_table.cpp` for something that has a console command. The last argument says where the mouse has to be for
the key to count: `anywhere`, `views` ( a 3D or 2D view: plain keys must not act while a list has the mouse ),
`view_3d`, `view_2d`. When two actions have the same key, the one that looks at the smaller place runs ( V is the
vertex mode in any view and "back to vertices" in the 3D view's edge mode ).

* **Edit > Edit Hotkeys** ( `ed_hotkeys [text]` ): search, click a key and press the new one ( Esc cancels ), a
  second key per action, `x` for none, `default` per row, "Reset all to defaults". A key that two actions share
  where both can hear it is shown in red with the other action's name. Console: `bind <id> <key> [2]`,
  `unbind <id>`, `bind_reset`. Saved to `codol_editor_hotkeys.cfg` next to the settings, one line per action
  ( `edit.clone "Space" ""` ); a line for an action that does not exist ( yet ) is kept.
* Defaults are Radiant's where the key is free: the grid sizes on `~` and the digits, `T` materials, `U` materials
  in use, `L` layers, `P` preferences, `J` / `Shift+J` outline / tint selected, `Shift+L` cycle the edit layer,
  `Shift+T` / `Shift+R` texture lock of moves / rotations, `Shift+X` crosshair of the 2D views, `Shift+W` patches
  as wireframes, `Ctrl+F` fit texture, `Ctrl+[` / `Ctrl+]` cubic clipping nearer / farther, `F1` help. The texture
  keys are one set for the 3D view and the 2D views: **Shift + arrows** shift by the grid size, **Ctrl + arrows**
  scale by 5 %, **Shift + PageUp / PageDown** rotate by the angle snap; a key that is held is one undo step. Hide Unselected has Alt+H as its second key, the leak spots Ctrl+Shift+K / L.
* **Help** ( `F1`, `ed_help` ): the mouse in the 3D view and the 2D views, and the list of keys made from the
  registry. Every key text the editor shows - the menus, the tooltips of the toolbar and the view buttons, the
  hint lines of the 3D view, Help - is the action's key as it is now ( `menu::key( "<id>" )`,
  `actions::shortcut( "<id>" )` ): a key changed in Edit Hotkeys shows everywhere at once.
* **`actions_check`** walks every menu of the menu bar once ( run off the screen, every submenu "open" ) and prints
  each entry whose key text is not the key of its action, a key text that comes from no action, and an entry
  that shows no key while the action of the same name has one; `actions_menus [text]` lists what it saw ( entry,
  key, action id ). `tests\ed3d\keys.cmds` runs it, changes five keys, runs it again, opens the Edit menu for a
  picture and resets the keys.

**Preferences** ( Edit > Preferences, File > Settings, `P`, `ed_prefs [group]` ) replace the raw list of settings:
General ( text size `ed_ui_scale`, toolbar, undo levels `ed_undo_levels` ), Paths, Camera ( `ed_cam_fov`,
`ed_cam_speed`, `ed_cam_invert_y`, `ed_wire_distance`, cubic clipping ), Grid and snapping ( `ed_grid`,
`ed_snap`, `ed_snap_angle`, what the 2D views show ), Autosave, Selection, Textures ( `ed_texlock_moves`,
`ed_texlock_rotations` ), Game and scripts ( the remaining settings ). Each is a line of `codol_editor.cfg` and
can be set on the console by its name. The camera popup of the 3D view and the grid of the views change the same
values.

Every setting and dialog field that holds a path has a **Browse `..`** button next to its text box: folders ( the CoDOL
install folder, the script folders, the working folder of the pre-reload check, the game and script folders of
*Install the built map*, the sound and image folders of the Assets windows ) open Explorer's folder picker, files
( Start command, Password file, the xanim list, the asset tool, the files of the Assets windows ) its open dialog
with a filter for the kind of file. The picker starts at the path in the box; the box stays editable, the picker
only fills it in. A command with arguments ( Pre-reload check ) gets the program from the picker, the arguments are
typed after it.

### The game's scripts

The offline release ships no loose scripts: the server's GSC is packed in `<game>\main\sh1x_svr_scripts.iwd`
( a plain zip; `scripts/maps/mp/<map>.gsc`, `scripts/maps/createfx/..`, waypoints, `mp/botclasstable.csv` ). A loose
`<game>\main\sh1x_rawfiles\<path>` overrides the packed file of the same path ( the server's script overlay ), so that
is where edited scripts and the level scripts of built maps go. Nothing has to be set up by hand:

* **Script folder.** While `codol_script_root` is empty and the install folder is set, the editor sets it to
  `<game>\main\sh1x_rawfiles` and creates the folder. *Install the built map* and `map_install` use it when no
  `scripts=` is given, the Map Entities export writes `maps\mp\<map>.d3dbsp.ents` there.
* **Reference copy.** At start ( and when the install folder changes ) every script of the archive is unpacked to
  `packed_scripts\` next to the editor, again only when the archive changed ( `packed_scripts\.packed_from` ). It is
  read only: a file there changes nothing in the game. While `codol_script_ref_root` is empty it is the reference
  tree of the Script Workbench. **File > Unpack the game's scripts again** and `scripts_extract [folder]` make it
  again or unpack into another folder ( never into the script folder: every file there would override the archive ).
* **Reading.** The createfx placements of the 3D view ( `maps\createfx\<map>_fx.gsc` ) and the level script the
  Assets window starts from are looked up in the script folder, the reference folder of the settings, then the
  archive.
* **Editing.** A script opened from the reference tree is saved to the script folder as a file of its own ( only that
  file ), so the edit becomes the override; a copy the script folder has already is never overwritten.
  `scripts_unpack <maps/mp/<map>.gsc>` copies one packed script into the script folder to edit it elsewhere.

* **Autosave**: every `ed_autosave_minutes` ( 5 ) a changed map is written to
  `map_source\autosave\<map>.autosave.1.map`; the older copies move to `.2`, `.3` .. up to `ed_autosave_keep`
  ( 5 ), the oldest goes. The map itself and its "modified" mark are not touched. `autosave_now` does it at once.
* **File > Open Recent**: the last ten maps, whether opened as a `.map` or as a zone of the game
  ( `ed_recent`, `ed_recent <n>`, `ed_recent clear` ).
* **Edit > Edit Colors** ( `ed_colors [preset]` ): background, grid lines, axes, coordinates and blocks of the 2D
  views, the selection, brushes ( new, of the zone, detail, tool ), patches, entities without a class colour,
  target lines, the camera, the background of the 3D view. Presets: CoDOL dark, Radiant classic, Night blue,
  High contrast. Settings `ed_color_<name>` ( `#RRGGBBAA` ).

**Toolbar.** Under the menu bar, wrapping to as many rows as the window needs. A button is an action; its picture
is drawn by the editor from lines ( no image files; an action without a picture shows its letters ). **Edit >
Edit Toolbar** ( right click on the bar, `ed_toolbar` ): the bar on the left ( drag a row to move it, Up / Down /
Remove / Add a gap ), every action on the right ( double click adds ), Reset. `toolbar_add <id> [position]`,
`toolbar_remove <id>`, `toolbar_reset`, `toolbar_list`. Saved to `codol_editor_toolbar.cfg` as ids; an id whose
action does not exist keeps its place and shows up when it does.

**Textures menu.**

* **Surface inspector** ( `S` ): below the fields it had - the **edit layer**, steps for shift and scale per axis,
  rotate, flip X / Y, rotate 90, **Natural**, the two texture locks. A face line of a `.map` has two layers: the
  material with its alignment and the lightmap with its own ( `lightmap_gray 16384 16384 0 0 0 0` ); a patch has a
  material, a lightmap material, two sets of texture coordinates and a smoothing word. Shift, scale, rotate, fit,
  flip and natural change the layer that is picked ( `tex_layer material|lightmap|smoothing|cycle`, `Shift+L` );
  in the lightmap layer the material and the size of the layer can be typed ( `tex_lightmap <material> [size]` ),
  in the smoothing layer the word of the selected patches ( `tex_smoothing <word>` ). The map compiler bakes
  lightmaps from the geometry, at the density the lightmap layer asks for: `codol_bsp` makes the luxels of a face
  as much larger as the size of its lightmap layer is over radiant's 16384, per axis ( 8192: twice as many luxels
  each way; clamped to 1/16 .. 16 ), and the luxels of a patch as much larger as its lightmap sample size ( the
  third number of its size line ) is over 16 - `codol_bsp_patch_tests` compiles a room with every face at 4096 and
  finds more than four times the luxels. The projection itself ( shift, rotation ) of the layer is not used: the
  compiler lays its own luxel grid on each face. **Patches** are
  targets of all of these: their texture coordinates move ( scale and rotate about the middle, fit over the whole
  patch, natural from the vertex positions ). Natural is the alignment of a new face - the format has one
  projection, along the world axis closest to the face; there is no separate "face" projection to reset to.
* **Find / Replace**: material A becomes B in the whole map, the selection, or the selected faces only
  ( `tex_replace <a> <b> [map|selection|faces]` ); "Select the faces using Find" ( `tex_select <material>` ). One
  undo step; a name with `wc/` in front and one without are the same material.
* **Texture Lock > Moves / Rotations** ( `tex_lock moves|rotations [0|1]` ): apart. **Texture Lock > Lightmaps**
  ( `tex_lock lightmaps [0|1]`, action `texture.lock_lightmaps`, setting `ed_texlock_lightmaps`, a box in the
  Surface inspector; off by default ): on, the lightmap layer goes with the geometry in every transform - moves,
  rotations, scales, flips ( the gizmo, 2D drags, nudges, the numeric fields, the Selection menu ), clones with an
  offset, a face dragged along its normal in the 3D view or resized - with the same maths as the material's lock
  ( `codol_edit` `lock_layers` ); patches keep the lightmap coordinates of their vertices. Off, as radiant: faces keep
  their lightmap projection in the world, patches get their lightmap coordinates again from where their vertices
  are ( along the axis the patch faces most, a luxel per lightmap sample size ). Paste puts objects where they were,
  so nothing changes there; stamping and making prefabs always keep both layers. A scale that is not the same
  along the axes of a turned projection shears it, which the format cannot hold ( as for the material ). Tests:
  `codol_edit_tests` ( `test_lightmap_lock` ).
* Flip X / Y, rotate 90, fit, natural, "select all faces with the material of the selection" as actions
  ( `tex_flip x|y`, `tex_natural` ).
* **Materials** window: filters for usage, locale and surface type ( also in the menu ), the lists *in use*
  ( `U` ), *favourites* and *recently used*, the size of the pictures; right click on a material: apply, select
  all faces using it, favourite, find / replace with it, preview. Usage and locale are properties of a material's
  GDT entry, which a zone does not have: here the **usage** is read off words of the name ( wall, floor, roof,
  metal .. ), the **locale** is the prefix of the name when at least a dozen materials share it ( `me_`, `ch_` .. ),
  the **surface type** is the material's surface type bits ( read while the filter is on ). Favourites and the
  recent list: `codol_editor_materials.cfg`. `ed_material_browser all|inuse|favourites|recent | usage <n> |
  locale <prefix> | surface <name> | thumb <px> | favourite <material>`.
* **Reload the project's images and materials**: `asset_rescan` + `asset_build`.

**Renderer menu.** Settings `ed_r_*`; `r_status` says what the 3D view does.

| | |
|---|---|
| General | Outline Selected ( `J` ), Tint Selected ( `Shift+J` ), Show Connections, Show Angles ( an arrow where an entity looks ), Draw Model Origins |
| Grid Window | names of entities, grid blocks ( the flag of the Grid menu ), coordinates at the edges and of the blocks, a cross under the mouse with its position |
| Render Method | `r_mode game|fullbright|fake_normal|fake_view|depth`, Wireframe. **Fullbright** is the game's own `unlit` technique of each technique set: the colour maps without lightmap, sun or specular. The **fake lights** shade the fullbright picture by the way a surface faces / by its angle to the camera. **Case Textures** ( `r_mode case` ): what CoD4 radiant's mode of that name is - its technique "case texture" ( `techniques/case_texture.tech` of the mod tools ) samples a code texture that `CoD4Radiant.exe` picks per material ( the function at 0x53B630 compares the width and height of the material's colour map with those of the images listed in `bin/case_textures.txt`: `case256`, `case1024x128`, `case512x64red` .. ). So every surface shows a generic grid of the size of its texture: how many texels it has and how it is stretched. Here the grids are generated ( `codol_view/src/renderer_case.cpp`, nothing of the mod tools is read ): one per colour map size, a checker of 64 texel cells with a line every 256 texels and the size written in each block, coloured by the larger side ( blue 64 and less, cyan 128, green 256, grey 512, orange 1024, red 2048 ); unlit, as fullbright. A legend in the top right corner of the 3D view lists the sizes the frame drew. It is not a surface type view - CoD4 radiant's "case" is the texture size. Alpha tested materials ( fences, foliage ) are drawn solid, as the grid has no holes. |
| Draw Water | off: nothing whose technique set is one of water |
| Texture Filter | as the materials say, point, bilinear, trilinear, anisotropic ( material textures; lightmaps keep theirs ) |
| Texture Resolution | maximum .. an eighth: the mip level every material texture is read at. One setting for colour, specular and normal maps. |
| Specular / Normal maps | off: the game's own stand-ins ( black, flat ) are bound in their place |
| Show Patches As | textured, wireframe, textured + wireframe ( `r_patches`, `Shift+W`; the Patch menu's entry is the same setting ) |
| Show Entities As | CoD4 radiant's six: Bounding Box, Wireframe ( the triangles of each model's mesh as lines, front and back, in its materials ), Selected Wireframe and Selected Skinned ( the models of the selected entities wire / drawn, every other entity its box ), Skinned, Skinned And Boxed. `r_entities <models\|boxes\|both\|wireframe\|selected_wireframe\|selected_models>`, actions `render.entities_<word>` and `render.entities_cycle` ( the toolbar button, in the order of the menu ), setting `ed_r_entities` 0 - 5. The renderer asks the editor per model and frame how to draw it ( `frame_options::model_style` ); models that belong to no entity of the document stay as they are. `tests\ed3d\entities_as.cmds` takes a picture of each. |
| Light / Sun Preview | **Light preview**: the light of the selected `light` entities, and of the ones kept in the preview list, falls on what the 3D view shows - radius, colour, intensity, the cone of a spot; up to 8, the nearest. Sun shadows on / off. For a map of your own "Sun Preview" builds the stage again with the worldspawn's light. |
| Filmtweaks | the film values of a `.vision` file of the game's zones ( `r_vision <name>`, `r_vision list`, the window lists and filters them ), or typed ( `r_film contrast brightness desaturation [light rgb] [dark rgb]` ) |
| Cubic Clipping | only what touches a cube around the camera is drawn ( `r_cubic <distance|off>`, `Ctrl+\`, `Ctrl+[`, `Ctrl+]` ) |

The fake lights, the depth view, the light preview and the film values are one **post process** over the picture
of the renderer: a pixel shader compiled at run time with the system's `d3dcompiler_47.dll` ( nothing is linked
against it; without it these modes say so and the view stays as it is ). Position and normal of a pixel come from
the depth pre-pass of the game's shaders ( `map_renderer::depth_texture()` ), so they exist for what that pass
draws - the opaque world, models and meshes - and not for blended surfaces. The light preview is an
approximation ( a point light that falls off in a line to its radius, no shadows ), not the bake and not the
game's primary light techniques: those light a surface with one primary light, which the previewed light would
have to replace. The vision files of 1.1.1.13 name their settings by number; 520 .. 528 are the film values
( measured on the shipped `mp_crash.vision` ), the glow values 511 .. 519 are not shown.

**Effects menu.** An effect plays in the 3D view where an entity has an `fx` key ( the name: `fx\<name>.efx` of
the project, else the effect of the map's zone or a common zone ) and where the createfx script of the map puts
one ( `<script root>\maps\createfx\<map>_fx.gsc` with the names of `maps\mp\<map>_fx.gsc`; read only ). **Effects
Browser**: the project's and the zones' effects; double click places a `script_origin` with the `fx` key where the
camera looks ( `fx place <name>` ). Play / Pause / Stop / Repeat, **Edit Current Effect** ( the Preview window,
and the project's `.efx` in the program that opens it ), Reload, **Effect Settings** ( timescale, repeat delay,
draw distance, debug tris ). "Write the createfx script" turns the `fx` entities of the map into
`map_source\<map>_createfx.gsc` for the level script to call. `fx play|pause|stop|repeat|time|timescale|delay|
tris|place|list|zone [text]|status|reload|createfx`. The particles are the editor's simulation, as in the
Preview window: sprites, tails and lines, drawn over the picture without depth; entity angles are not applied.

**Tools menu.**

* **Rope Generator** ( `tool_rope x1 y1 z1 x2 y2 z2 [slack] [radius] [segments] [sides] [material]` ): a cable
  between two points that hangs by its slack, as terrain meshes ( a tube; more than 15 segments is a chain of
  meshes that share a ring ), or as a chain of models along the curve ( `tool_rope chain <model> .. [slack]
  [spacing]` ). The points: typed, the camera's target, two selected entities, or the ends of the selection's box.
* **Noise Generator** ( `tool_noise [amplitude] [wave length] [seed] [octaves] [x|y|z] [edges 0|1]` ): hills on
  the selected terrain meshes; the same numbers give the same hills, meshes that touch keep touching. "Next seed
  + apply" flips through hills. `tool_noise terrain [size] [points]` makes a mesh to start with.
* **Mesh Painter** ( the toolbar button, `tool_paint on|off` ): left drag in the 3D view scatters models onto what
  the mouse is over - models ( a list ), radius, density per step, spacing of the steps, random yaw, scale range,
  align to the surface, sink. A stroke is one undo step; the right button still flies the camera.
  `tool_paint models .. | radius | density | spacing | scale | align | yaw | at <px> <py>`.
* PhysX Movement: not in this editor ( it is the PhysX runtime of the mod tools ).

### Your own assets

Every map has a project folder next to the editor ( the project is named after the map; `asset_project <name>`
picks another one ):

```
mod\<project>\images\*.png | *.tga | *.dds        source images; the image asset is named after the file
mod\<project>\materials\*.mtl                    material definitions ( codol_assets/README.md, ".mtl" )
mod\<project>\models\<name>.model                a model: LOD files with distances, collision LOD, material names
mod\<project>\models\*.xmodel_export             LOD files; one that no .model names is a model of one LOD
                                                 ( <name>_lod0 .. _lod3.xmodel_export belong together )
mod\<project>\anims\<name>.anim                  an animation: its xanim_export, model, type, loop, animtree
mod\<project>\anims\*.xanim_export               one that no .anim names is an animation of that name, no options
mod\<project>\soundaliases\*.csv                 sound aliases ( columns of the CoD4 mod tools ); *.vfcurve, *.spkrmap
mod\<project>\sound\<file>                       what the file column of the aliases names ( .wav, streamed: .mp3 / .wav )
mod\<project>\fx\<folder>\<name>.efx             an effect; its name is <folder>/<name>, what loadfx takes
mod\<project>\scripts\..                         your own scripts: maps\mp\<map>.gsc, animtrees\<tree>.atr ..; not
                                                 compiled, the built map takes them as they are
mod\<project>\vision\<map>.vision               the map's vision file: glow, film tweaks ( r_glow "1", r_filmContrast
                                                 "1.2" .. ); without one the build writes a neutral default
mod\<project>\sun\<map>.sun                     the sun's sprite and flare ( r_sunsprite_size, r_sunflare_max_size .. );
                                                 without one a map with a sun and a sky gets a default flare
mod\<project>\generated\animtrees\<tree>.atr     written by the build: one per animtree the animations name
mod\<project>\generated\animlists\map.animlist   written by the build: becomes animlists/<map>.animlist of the map
mod\<project>\zz_<project>_sounds.iwd            written by the build: the streamed sound files
mod\<project>\<project>_assets.ff                 the zone built from all of it
mod\<project>\build_state.txt                     what the last build made of each source
mod\codol_matlib.txt                             the material library: one scan of the install ( 20 s, once )
mod\codol_fxatlas.txt                            the texture atlases of the install's materials ( 30 s, once, with
                                                 the first effect )
```

**View > Assets** lists every source with its status - new, changed, built, failed with the compiler's message,
unused ( an image no material names ), as is ( a script file ) - and has **Build**. A build compiles everything
again on a worker thread ( `codol_assets`: `compile_image`, `compile_material`, `compile_model`, `compile_anim`,
`build_sound_aliases`, `compile_fx`, `AssetLinker` ) into a new zone; the editor stays usable. The renderer then
takes the zone without loading the map again ( `map_renderer::register_zone` ), and every mesh and placed model is
made again from it. A click on a line shows the asset in the **Preview** window ( below ).

| what | where | command |
|---|---|---|
| material from images | Materials > **New material ..**, Assets window, or drop image files on the editor window ( `_n` / `_nml` / `_normal` and `_s` / `_spc` / `_spec` at the end of a name fill the normal and specular map ) | `asset_new_material <name> <kind> <colour map> [normal=<file>] [specular=<file>] [surface=<type>] [overwrite]` |
| model | Models > **Import model ..**, or drop `.xmodel_export` files | `asset_import_model <name> <lod0>[@dist] [<lod1>[@dist] ..] [collision=<lod>] [mtl:<export material>=<material>] [overwrite]` |
| materials of a CoD4 `.gdt` | Materials > **GDT ..**, or drop a `.gdt` | `asset_gdt <file> [root=<folder the image paths start from>] [overwrite] [all \| <material> ..]` ( without a choice: the list ) |
| animation | Assets > **Import animation ..** ( file, name, model, type, looping, animtree ), or drop an `.xanim_export` | `asset_import_anim <file.xanim_export> [name=<name>] [model=<model of the project \| file.xmodel_export>] [type=relative\|delta\|delta3d] [loop] [tree=<animtree>] [overwrite]` |
| sound aliases | Assets > **Sounds ..** ( the csv, the folder that holds `sound\` and `soundaliases\`, overwrite ), or drop a soundaliases `.csv`: the csv is copied with the sound files, curves and speaker maps it names ( from `sound\` and `soundaliases\` around it ) | `asset_import_sounds <file.csv> [root=<folder with sound\ and soundaliases\>] [overwrite]` |
| effect | Assets > **Effect ..** ( the file, a name of your own, overwrite ), or drop an `.efx`; it is named by its place below an `fx` folder ( `..\fx\misc\x.efx`: `misc/x` ), else `<project>/<file>` | `asset_import_fx <file.efx> [name=<folder/name>] [overwrite]` |
| level script | Assets > **Level script**: makes `scripts\maps\mp\<map>.gsc` with what the compiler would generate, to start from | `asset_level_script [<file.gsc>]` ( with a file: a copy of it ) |
| build, list | Assets window | `asset_build`, `asset_list`, `asset_rescan`, `asset_status`, `asset_project [name]` |
| check the zone | | `asset_verify` ( `codol_assetc --verify`: the game's loaders read the zone, it is written again and compared ) |
| look at it, listen to it | a click in the Assets, Models or Materials window | `asset_preview model <xmodel> \| material <name> \| image <file> \| anim <name> [model] \| sounds <csv> \| fx <name> [zone \| file.ff] \| time <seconds> \| frame <n> \| play \| pause \| orbit <yaw> <pitch> [zoom] \| status \| close`, `asset_play <alias> [zone \| file.ff]`, `asset_play stop` |
| the windows, for unattended runs | | `asset_dialog material \| model \| anim [file] \| sounds [file] \| fx [file] \| gdt [file] \| close`, `asset_drop <file> ..` ( what dropping the files does ), `ed_windows a` ( `p`: the Preview window ) |

Material kinds ( the technique set follows from the kind and the maps given, as `codol_assets` `techset_for` has it ):
`world`, `world_alpha` ( alpha tested ), `world_blend`, `decal`, `world_unlit` become `wc/<name>`; `model`,
`model_alpha`, `model_blend`, `model_unlit` become `mc/<name>`; `2d`. The material is the material of new brushes
right away and is applied like any other ( browser, `tex_set` ). A model is placed from the Models browser or with
`ed_place`; `ed_model_class misc_model` makes new model entities static models of the compiled map.

#### The Preview window

**View > Preview**, or a click: on a model in the Models window, a right click on a material in the Materials
window ( a left click too while the window is open ), on any line of the Assets window. A double click still uses
the asset. The pictures are drawn by the renderer of the open map into a target of their own; the camera turns
with the left mouse button and zooms with the wheel.

| what | what is shown |
|---|---|
| model ( of the game or of the project ) | the model as the game draws it, its size in units, **Place in the map** |
| material | a cube drawn with the material's own technique set and shaders, lit by the sun of the map; its colour map with its size; technique set, zone; **Use for new brushes**, **Apply to the selection** |
| image ( a source file of the project ) | the picture as `codol_assets` decodes it ( PNG, TGA, DDS ), size, alpha channel, whether its size is one the compiler takes; **New material from it ..** |
| animation | the animation on its model, with a **timeline**: Play / Pause, the frame ( drag it ), speed, Loop, length and frame rate, the note tracks as marks on the timeline and as a list; another model of the project can stand in. After a build the picture is made from the zone: the `XAnimParts` and the `XModel` are read back out of `<project>_assets.ff` ( `codol_zone`, `read_raw_anim`, `decode_anim`, `extract_model_lod` ), so what plays is the animation as the game gets it, compression included. Before the first build it is made from the `xanim_export` and the `xmodel_export`, and the window says which of the two it shows. The mesh is skinned in the editor ( every vertex by its bone weights ) and drawn with the model's materials. |
| sound aliases ( a csv of the project ) | every row with its file; **Play** plays the `.wav` of the row ( WinMM; an `.mp3` through MCI ), a row whose file is not in the project is a sound of the game. **Play from zone**: an alias name and a zone - empty for the project's zone, a zone of the game ( `mp_rust` .. ) or a `.ff` file -: the alias and its loaded sound are read out of the fastfile, IMA ADPCM is decoded by `codol_assets`, and that is played. |

| effect ( `fx\<name>.efx` of the project, or an effect of the game ) | the effect's particles over time, with a **timeline**: Play / Pause, the moment ( drag it ), speed, Loop, the number of particles. A project's effect is its `.efx`; one of the game ( `asset_preview fx fire/firelp_small_pm`, or with a zone / a `.ff` behind it ) is read out of the zone and turned back into curves ( `codol_assets` `read_fx`, `extract_fx` ). Both are simulated by the editor ( `codol_assets` `simulate_fx` ) and drawn as flat quads with the colour map of their material: sprites, tails, lines, clouds. **It is not the game's effect system**: no depth, no lighting, no fog, all particles blended - added where the material's name says light ( spark, fire, flare, glow .. ) -, a cloud is one particle, and trails, spark fountains, models, lights, sounds, decals and the effects an element starts are named under the picture and not drawn. The effect stands with its forward axis up, as `playfx( fx, origin )` plays it. |

Checked ( pictures in `build-editor\shots\` of the test run ): a shipped model and a project model, a shipped
world material and a project model material on the cube, a PNG, the two-bone test animation at frames 0, 20 and 30
( the lid moves ), the csv's rows; `asset_play` of a project alias from its `.wav` and from the built zone, and of
three aliases of shipped `mp_rust.ff` ( 32 and 44.1 kHz IMA ADPCM, decoded ); sixth round, the effect preview
( `build-editor\bsptest\shots\c6fx_sheet.png` ): the project's `codol_test/spark_burst` at 0.3, 0.9 and 1.8 s - its
sparks, then its smoke puffs growing -, shipped `fire/firelp_small_pm` out of `common_mp.ff` - flame, sparks, the
black and the grey smoke above it. Not there: streamed sounds
of the game ( files of its archives ); animations of the game on models of the game ( `asset_preview anim` takes
animations of the project ); a model with more than one LOD shows LOD 0 in the animation. The test machine has no
audio device to listen with: that Windows accepted the wave data is what was seen.

`codol_view`: a mesh drawn with the technique set of a model material ( `mc_..` ) uses packed model vertices now.
With world vertices such a mesh came out in one flat colour - a brush with a model material did too.

**In the compiled map** ( File > Compile map ). The project's zone is passed to `codol_mapc` with `--assets`, as
File > Build map carries it into a map of one's own: every animation, sound alias ( with its loaded sounds and
curves ), effect and model of the project goes into the edited map's zone; the project's `generated\animlists\map.animlist`
is put behind the map's own animlist, its animtrees and the files of `scripts\` become rawfiles. The project's
scripts are also copied to `compiled\<map>.scripts\` - the dedicated server reads level scripts from its script
folder, so a `scripts\maps\mp\<map>.gsc` of the project *replaces* the shipped map's level script there and has
to hold what that one does ( Assets > level script starts from the shipped one, out of `sh1x_svr_scripts.iwd` ). Streamed sounds need their
archive as for a built map; the console prints the command.

Loaded ( sixth round, `codol_bsp/README.md` ): shipped mp_rust, opened in the editor, with a project of one
material, one model, sound aliases, one effect and a level script; the model placed once as a static model and
once as a `script_model`; File > Compile map. The dedicated server takes the zone and runs the project's level
script; the client enters the map, draws the three crates ( static, entity, spawned by script ), the effect's
puffs above them, and plays the project's loop sound. **Animations** ( seventh round, on the server, with the
command line: `codol_mapc --assets` with the project's animlist appended and its animtree as a rawfile ): the
server reads `animlists/mp_rust.animlist` with the project's two entries, mounts `animtrees/<tree>.atr`, gives the
tree a client id, `getAnimLength` answers and `scriptModelPlayAnim` runs without a script error - the same lines
as for a map built from scratch. Whether they move in the client is the open question it is everywhere.

* A model of the project that an entity uses - a `misc_model` ( static ) or any other class ( `script_model` .. ) -
  is copied into the map zone with its materials and images ( "entity models / static models: xmodel .. taken from
  zone .." in the report ).
* **New world materials are not drawn in the game by Compile map** ( File > Build map draws them: the material, its
  images and technique set go into the built zone ). The incremental compiler draws new faces only with materials
  that the world of the base map already uses; faces with any other material collide but are invisible. The Assets
  window says so next to every such material, and the compile lists the faces - before it runs and with the
  compiler's report - in the console, in a toast and in the Assets window ( "Last compile: not in the game" ).
* Images are stored inline in the zone ( DXT ), which no shipped material texture does; see "Not done, taken on
  trust" in `codol_assets/README.md`.
* The fastfile of an edited shipped map has to keep the length of the shipped file ( the client checks it ): what a
  project adds has to fit into what the better compression leaves. mp_rust with the test project ( 0.3 MB ) fits.

**Animations, sounds, effects.** What each source is and how it is compiled: `codol_assets/README.md`,
"Animations", "Sounds", "Effects".

* An animation needs the model it is for ( the `xanim_export` has no bone hierarchy ): a model of the project by
  its name, or an `.xmodel_export` file. Without one every part is taken as a root - right for a model of one bone.
  Every animation belongs to an animtree, by default one named after the project; the build writes
  `generated\animtrees\<tree>.atr` ( one name per line ) and `generated\animlists\map.animlist`.
* A soundaliases `.csv` is one line of the Assets window with everything it names. Loaded sounds go into the zone,
  streamed files ( `.wav`, `.mp3`: the map's ambient track, long loops ) into `zz_<project>_sounds.iwd`, which the
  install turns into an archive of the game. A file that is not under `sound\` is referenced by name - a sound of
  the game.
* An effect names materials of the project or of the game; effects it emits are read from `fx\<name>.efx` of the
  project.

#### In the built map

File > Build map ( `tools\package_map.ps1 -Assets <project zone> -Project <project folder>` ) puts the project into
the zone of the map, so that a level script can use it:

| from the project | in `compiled\<map>\` |
|---|---|
| the zone's animations, sound alias lists, loaded sounds, curves, effects and models, each with what it points at ( materials, images, technique sets ) | assets of `zone\<map>.ff` ( `codol_assetc --carry`; what the compiler already took stays as it is ) |
| `generated\animtrees\<tree>.atr` | rawfile `animtrees/<tree>.atr` |
| `generated\animlists\map.animlist` | rawfile `animlists/<map>.animlist` ( the compiler's empty one is replaced ) |
| `scripts\<path>` | rawfile `<path>` and `scripts\<path>` |
| `zz_<project>_sounds.iwd` | `main\zz_<project>_sounds.iwd` |

**The rule for scripts:** a file of `mod\<project>\scripts\` goes in front of a generated file of the same path.
`scripts\maps\mp\<map>.gsc` is the level script instead of the generated one, `scripts\animtrees\<tree>.atr`
replaces the generated animtree ( it then has to list the project's animations of that tree; the Assets window says
when it does not ), `scripts\animlists\<map>.animlist` the generated animlist. What the compiler generated is kept in
`compiled\<map>\work\scripts\`. Files that an earlier build put into `scripts\` or `main\` of the package and that
are not made any more are removed; nothing else is deleted. A level script of your own has to make the calls of
the generated one - Assets > **Level script** writes them:

```
#include maps\mp\_utility;
#using_animtree( "mp_mymap" );                                    // the animtree of the project's animations

main()
{
	level._effect[ "sparks" ] = loadfx( "misc/my_sparks" );       // fx\misc\my_sparks.efx
	precacheModel( "my_lidbox" );                                 // a model no entity of the map uses
	maps\mp\mp_mymap_fx::main();                                  // generated: the script_fx / script_sound entities
	maps\mp\_load::main();
	maps\mp\_compass::setupMiniMap( "compass_map_mp_mymap" );
	game[ "attackers" ] = "allies";
	game[ "defenders" ] = "axis";
	level thread show();
}

show()
{
	wait 5;
	box = spawn( "script_model", ( 0, 0, 64 ) );
	box setModel( "my_lidbox" );
	box setAnim( %my_lid_open, 1, 0, 1 );                         // anims\my_lid_open.anim, model my_lidbox
	box playSound( "my_beep" );                                   // an alias of soundaliases\*.csv
	playfx( level._effect[ "sparks" ], box.origin );
}
```

Checked on the dedicated server ( loopback, no client; `mp_edassets2`, a room with one model, two animations, one
alias file and one effect imported and built in the editor, File > Build map, `map_install` into a link based copy
of the install ): the server mounts the map zone, reads `animlists/mp_edassets2.animlist` ( 2 entries ) and
`animtrees/mp_edassets2.atr` from it, runs the project's level script with 0 script errors, `getAnimLength` gives 2
and 1.33 s, `setAnim` binds the script model to the tree, `loadfx` / `playfx` and `playSound` / `playLoopSound` /
`ambientPlay` run; the server's own decoders take the animations and the model. Controls: the same script with an
alias no zone has stops with "unknown sound alias", the same package without the carried assets with "xanim .. is
not in any mounted zone".

**In the game client** ( loopback, the test user; `codol_bsp/README.md`, "Fourth round" ): a packaged map with a
project of one effect, two animated models and three sound aliases loads; the **effect** placed with a `script_fx`
entity is drawn, its smoke puffs coming again and again; the two **models** stand where the level script spawns
them but their **animations do not move** - on this test server no script model animation reaches the client, the
hanging clothes of shipped mp_crash_tropical stand still as well ( `codol_bsp/README.md`, "Script model
animations": what the map side provides is there, the rest is the server's ); **sounds**: a project's loaded sounds
play - measured at the client's audio session, with their distance falloff ( "Fifth round" ) -, and so do its
streamed ones ( sixth round: the ambient track of a map from a `.wav` and from an `.mp3`, an `.mp3` on an entity
with its distance falloff - measured the same way, not listened to ). The effects a map places are made by the client
from the text of `maps\mp\<map>_fx.gsc` and `maps\createfx\<map>_fx.gsc`: leave those two to the build, or keep
their exact form.

Not there: deleting or renaming an asset from the editor ( delete the files, Rescan ), editing a material's
definition in a window ( edit the `.mtl`, Build ), a preview inside the import windows ( import, then click the
line ), streamed images ( `.iwi` in an `.iwd` ), per image options ( format, `flipy` ), the less
common animation options ( `deltapart`, `assettype`, tolerances: `codol_assetc --xanim` has them ), copying the
effects an effect emits on import ( put them into `fx\` by hand ). The shipped animtrees ( `animated_props` .. ) are
not extended: give your animations a tree of their own. An edited shipped map has no install step: its fastfile,
`compiled\<map>.scripts\` and the sound archive are copied by hand.
## What works

This table and the sections below it are about the **radiant plugin** ( `iw3r.dll` inside CoD4 radiant ), the first
form of this work; what the standalone editor does is above.

| Area | Status |
|---|---|
| Script editing | Browse and edit the game's script tree (`.gsc`, animtrees, `mp/playeranim.script`, string tables, ...) inside radiant, with syntax highlighting, find / go to line, GBK and CRLF preserved on save. |
| Player animations | Structured view of `mp/playeranim.script` (defines, states, movement types, conditions, events) with jump-to-line, checked against `animtrees/multiplayer.atr`. |
| Scripted animations | Per-script tables of `level.scr_anim[..]` / `scr_model` / `scr_animtree` entries and `anim_*` calls, the governing `#using_animtree` for the cursor line, and an animtree browser. |
| Validation | A `%anim` that is not in its animtree is an error (the server would append it and shift every later index: T-pose / wrong clips). Optional: animations the game does not ship are warnings. |
| Map import | One click turns a shipped map into an editable radiant map: every collision brush with the material of each side, brush entities with their real brushes, every static model as a `misc_model`, and all entities. `mp_killhouse_sh`: 27,032 world brushes, 640 brushes in 280 brush entities, 16,335 static models, 1,967 entities. |
| Native viewport | **CoDOL > Viewport ( native )** draws a map the way the game does: render world, static models, entity models and brush models read straight from the 1.1.1.13 fastfiles and drawn with the game's own materials, technique sets and compiled shaders (`codol_zone`, `codol_render`, `codol_view`). Lightmaps, reflection probes, light-grid model lighting, sun shadows, cell/portal culling. Frames are bit-identical to CODOL-AssetExplorer's renderer on 8 of 9 test views. Console: `codol_viewport <map>`. |
| Map entities | Edited entities are exported as a server-side override (`<scripts>/maps/mp/<map>.d3dbsp.ents`). Unedited entities export byte-identical to the zone. |
| Hot reload | rcon to the local dedicated server: reload the level when a script is saved, load a map after an entity export. Optional pre-check command. |
| Game window | The window of the running game client can be docked inside radiant (CoDOL > Game Window) and handed back. |
| Asset layouts | `src/codol/c1_layouts.hpp`: the 1.1.1.13 zone layouts (XModel, XSurface, Material, techset, GfxImage, XAnimParts, clipMap, GfxWorld, MapEnts, ...) as C++ structs, every size and named offset checked at compile time. |

## What does not exist yet

Be aware of these before planning work around this branch:

- **Top bar of CoD4 Radiant: what is not there.** Separate texture resolutions for specular and normal maps ( one mip bias for all material textures ), a "face"
  texture projection ( the format has the axial one ), PhysX movement. The light preview and the fake lights are a
  post process from the depth pre-pass, not the game's light techniques; effects in the map view are the editor's
  particle simulation without depth.
- **Maps of your own: lighting is simpler than the game's own maps.** Surfaces are cut by the reach of the
  primary lights along a grid of 32 units, not as finely as the game's compiler does, and a surface is lit at run
  time by one primary light. Bounced light has the mean colour of a material, not its pattern. An omni light
  shadows players and models only inside its cone ( so do the game's own - it is what the shaders read ). A light a
  script moves keeps the shadows of where it was compiled and lights at run time only what no other light claims.
  See "Limits" in `codol_bsp/README.md`.
- **Moving a light by script: `.origin` only, on the test server.** The game's own scripts also use `moveTo` and
  `linkTo` on lights; `codol_dedrun` answers both with a script error for the classes `light_spot` / `light_omni`.
- **Static models have no player collision of their own.** As in the shipped maps, bullets hit a `misc_model`,
  players do not: put clip brushes around models.
- **Nobody has seen an animation of your own move in the game.** The editor plays the compiled animation ( the
  Preview window reads it back from the project's zone ), the Assets window builds it and File > Build map puts it
  into the map with its animtree, animlist and your level script; a dedicated server runs a level script that uses
  it without a script error - for a map built from scratch and for an edited shipped map -, and the client loads
  such a map and draws its effect ( [In the built map](#in-the-built-map) ). But script model animations do not
  move in the client on the test server - shipped ones do not either; what a map has to bring is there
  ( `codol_bsp/README.md`, "Script model animations" ), the rest is the server's.
- **Sounds were measured, not heard.** Loaded and streamed sounds of your own play in the client - measured at
  its audio session on a machine without anybody listening; the editor's Preview window plays the loaded ones.
- **Nothing was played over a network.** Every run in the game was one client, one dedicated server and the lobby
  emulator on one machine, with a test account of level 1.
- **Editing a shipped map** ( File > Compile map ) cannot draw new world materials, does not remove render surfaces
  of deleted brushes and does not re-bake light. For that, build the map from scratch. It has no install step
  either: its fastfile ( which replaces the shipped one and has to keep its length ), `compiled\<map>.scripts\`
  and the sound archive are copied by hand.
- **Imported maps hold collision brushes only.** Brush sides get the right material but a default projection;
  terrain, patches and decals of a shipped map exist only as render surfaces and are not in the imported `.map`.
- **The create-room list: written, loaded by the client, not seen.** File > Install the built map .. adds the map to
  the list zone of a game folder and takes it out again ( [A map of your own](#a-map-of-your-own), step 5 ). The
  client runs with the zone; its Create Room screen is locked below player level 5 and the test account is level 1,
  so the row was not looked at and a map was not started from it. `CODOL_FORCE_MAP=<map>` on the lobby plays a map
  without the list.
- **3D editing**: the selection rectangle is a rectangle ( no lasso ); a drag next to a brush moves faces, it does
  not shear them ( Radiant's Ctrl drag ); the handles of the vertex and edge tools are drawn through walls and have
  a move gizmo only for brush corners ( a set of patch control points turns and scales; brush corners do not ); a brush is always the hull of its corners, so a
  corner pushed inside the others disappears. The texture keys act on every face of a selected brush, as in
  Radiant - deselect the brush ( Esc ) and Ctrl+Shift click faces to work on single ones.
- **The 3D view of a map of your own is not the light bake.** It shows the sun, sky and ambient light of the
  worldspawn on brushes and models; light entities are shown as shapes, their light is in the built map.
- **Patches**: no `physics_box` / `physics_cylinder` blocks. The patch vertex selection is one patch at a time ( a
  click on another patch's point starts a new one ); a vertex of a model is snapped to as the renderer gives its triangles
  for picking. The subdivision number of a curve is radiant's
  ( smaller is finer, verified in its binary ), but how many triangles a number gives is this editor's reading ( an
  error bound in units ) - radiant's and cod4map's tessellation were not traced
  ( [Brushes and patches](#brushes-and-patches-the-brush-and-patch-menus) ). File > Compile map ( an edited
  shipped map ) draws the patches of the worldspawn but compiles **no collision** for them ( clip brushes instead;
  `codol_mapc/README.md`, "Patches" ); neither compiler takes patches of brush entities. Patches compiled into a
  shipped map were not seen in the game.
- **Assets**: no deleting or renaming from the editor, no editing of a material in a window, no streamed images,
  the shipped animtrees are not extended ( the list under [In the built map](#in-the-built-map) ).
- **Menus of CoD4 radiant that are not here** ( of File, Edit, View, Selection, Grid, Misc, Region ): the Script
  Group, Vehicle Group and Dyn Entity dialogs and Delete Exportables have nothing to work on in this game's maps;
  Createfx is not an export of its own ( the build writes the script ); `toolFlags splitGeo` is kept in the `.map`
  but the compiler has no use for it; the Prefab Browser has no thumbnails; there is no Maya export. A click in
  the 3D view on a model or brush that the "don't select" switches lock does nothing ( it does not pick what is
  behind it ). The list with every entry: [Selecting, regions, filters, prefabs](#selecting-regions-filters-prefabs).
- **Entity overrides are mp only.** The server applies `<map>.d3dbsp.ents` in mp matches; co-op and
  zombies compose their own entities.
- The sections below this one describe the radiant plugin ( `iw3r.dll` in CoD4 radiant ), which the standalone
  editor replaces; the Script Workbench and the game bridge are the same code in both.

## Setup

Radiant needs a CoD4 mod tools root (the folder that contains `bin/`, `raw/`, `main/`, `map_source/`).
The DLL hooks `bin/IW3xRadiant.exe` (a patched `CoD4Radiant.exe`, in `assets/bin`).

1. Build `iw3r.dll` (see [build](#code-layout-build-and-tests)) and copy it with the PhysX dlls into
   `<root>/bin`. `cmake -DRADIANT_ROOT=<root>` does that after every build.
2. Copy `assets/bin` and `assets/map_source` into the root (`map_source/prefabs` has to exist,
   radiant aborts at startup without it).
3. Start `bin/IW3xRadiant.exe`, open **CoDOL > Script Workbench > Setup** and fill in:

| Setting | dvar | Meaning |
|---|---|---|
| Script folder | `codol_script_root` | The tree you edit: the folder the server loads loose scripts from. |
| Reference script folder | `codol_script_ref_root` | Optional second tree. Files of the script folder shadow it, animtrees of both are merged. |
| Game xanim list | `codol_xanim_list` | Optional text file, one animation name per line (first word of each line). |
| CoDOL install folder | `codol_install_path` | Contains `main\` and `client_shipRetail\`. Used to list map zones and to find the rcon password file. |
| Start command | `codol_launch_cmd` | Script or exe that starts the client (Launch button of the Game window). |
| Client process names | `codol_client_exe` | Used to find the game window. Default `codoMP_client_shipRetail.exe;C1.exe`. |
| Port | `codol_rcon_port` | UDP port of the local server. Default 17733. |
| Password file | `codol_rcon_password_file` | File with the rcon password. If empty: `<install>\main\k33_rcon.txt`, `<install>\p2p\state\native_server_rcon.txt`, `<install>\CODOL\1.1.1.13_server\alpha0022\native_server_rcon.txt`, `%CODOL_STATE_DIR%\native_server_rcon.txt`. |
| Pre-reload check | `codol_reload_precheck_cmd` / `_cwd` | Optional command that has to exit with 0 before the game is reloaded. |

Settings are saved in `<root>/bin/IW3xRadiant/dvars.cfg`. Without a script folder the workbench
falls back to `%CODOL_SCRIPTS%`, then to `<root>/raw`.

## Script Workbench

**CoDOL > Script Workbench** (console: `codol_workbench`).

- **Files**: the script tree with a path filter. Dot-folders (`.git`) are skipped.
- **Editor**: tabs, Ctrl+S, find (Enter / F3 / Shift+F3), go to line. A dot in the tab marks unsaved
  changes; a file changed outside of radiant is reloaded, or offered for reload if it has unsaved
  edits. Files that are not valid UTF-8 are treated as GBK (the game's text encoding), edited as
  UTF-8 and written back as GBK. Line endings are kept as they were.
- **Problems in the gutter**: a dot next to the line number, hover for the message.
- **Status bar**: line / column, language, encoding, line endings and the animtree that governs
  the cursor line (the last `#using_animtree` above it).

### Animations tab

- **Player**: `mp/playeranim.script` as a tree. Sections (`ANIMATIONS`, `CANNED_ANIMATIONS`,
  `STATECHANGES`, `EVENTS`), states, movement types, condition blocks and their
  `both / legs / torso / turret <anim>` commands. Click to jump. With a filter it becomes a flat
  list of matching commands; the tooltip shows the full path of the block.
- **Scripted**: for the selected script (it follows the active tab) the scene tables
  (`level.scr_anim[ who ][ scene ][ index ] = value`, also `scr_model`, `scr_animtree`, `scr_sound`,
  ...) and the animation calls with their string arguments.
- **Animtrees**: every `animtrees/*.atr` as a tree, flags (`additive`, `complete`, `loopsync`,
  `nonloopsync`) included.

### Problems tab

| Severity | Finding |
|---|---|
| error | `%anim` is not part of the animtree named by the governing `#using_animtree` |
| error | `%anim` is used before any `#using_animtree` |
| error | syntax errors in an animtree or in `playeranim.script` (unbalanced braces, ...) |
| warning | the animtree named by `#using_animtree` is not in the script roots |
| warning | a player animation is not part of `animtrees/multiplayer.atr` |
| warning | duplicate animation in an animtree (stock trees have these) |
| warning | an animation is used but is not in the game xanim list (only with `codol_xanim_list`) |

The index is rebuilt in the background with **Rescan** and updated for a file when it is saved.

## Map Entities

**CoDOL > Map Entities** (console: `codol_mapents`). This is the way to work on a CoDOL map from inside
CoD4 radiant ( the plugin ); the standalone editor opens a shipped map itself and has File > Compile map.

1. **Import** a map: pick a zone of the install or an override that already exists in the script
   folder, or give a file (`.ff`, a decrypted `.zone`, a `.d3dbsp.ents`, plain entity text).
   The map is written to
   `<root>/map_source/codol/<map>.map` and opened.
   - A `.ff` is read natively: `bin/codol_mapconv.exe` loads the fastfile with the zone reader
     (`codol_zone/`, key taken from the client exe in the install folder) and writes the map: all
     collision brushes with the material of each side, brush entities with the brushes of their sub
     model, static models as `misc_model`s marked `codol_static`, and the entities. Brushes that only
     have tool materials (clip, hint, mantle, ...) are on the layer `codol_tools`.
   - A `.d3dbsp.ents`, a decrypted `.zone` or entity text gives the entities only, as points.
2. **Edit** in radiant.
3. **Save + export override**: writes `<script folder>/maps/mp/<map>.d3dbsp.ents` and, if enabled,
   sends `map <map>` to the local server. A file that was already there is kept once as `.bak`.

How entities are represented:

- Point entities are what they are in the game.
- With geometry, an entity with a brush model (`"model" "*N"`) is a real brush entity with the
  brushes of that model. Its brushes are for looking at: the exported entity still names the
  compiled model.
- A trigger hull (`"model" "?N"`), and any brush model without geometry, becomes a `codol_bmodel`
  point entity; its real class is kept in `codol_classname`, the hull in `codol_model`.
  It can be moved (hull bounds are relative to the origin), re-keyed, deleted or copied. New hulls
  cannot be made. A `?N` that the zone does not have is skipped on export with a warning, because
  the server rejects the whole override otherwise.
- The worldspawn carries `codol_map`, `codol_base_crc32` (CRC-32 of the zone's entity string; the
  server only uses an override made for the zone it has mounted) and `codol_trigger_models`.
- New brush entities drawn in radiant, `misc_model` and `misc_prefab` are not exported (a warning
  says so); the static models of the import are skipped silently.

Radiant rewrites entities when it saves, even untouched ones: origins are rounded to one decimal,
classes it knows from CoD4 get their `model` replaced, point entities gain an `origin`. So right
after the import the map is saved once as a baseline (`<map>.codol_base.map`, next to
`<map>.codol_orig.ents`), and the export compares against it: a value radiant changed on its own
is exported the way the zone had it, only real edits are exported as edited. An unedited map
exports byte-identical to the zone's entity string.

## Game window and hot reload

- **Reload in game** (workbench toolbar, CoDOL menu, `codol_reload`) reloads the level on the local
  server. Modes (`codol_reload_mode`): `fast_restart`, `map_restart`, or `map <current map>`, which
  is the default because it recompiles scripts from disk. With **on save** it happens whenever a
  script is saved.
- **Pre-reload check**: a script the server cannot compile makes the map load fail. If a check
  command is set and exits non-zero, its output goes to the console and the game is not reloaded.
- **CoDOL > Game Window**: *Attach* re-parents the window of the running client into the dock and
  keeps it sized to it; *Detach* or closing the dock restores its style and position. *Launch*
  runs the start command. Radiant menus cannot draw over the embedded window, and if radiant is
  killed while the game is attached the game window goes with it.

The rcon packet is `\xff\xff\xff\xffrcon "<password>" <command>` to `127.0.0.1:<port>`.

## Console commands and dvars

| Command | |
|---|---|
| `codol_workbench`, `codol_mapents`, `codol_gameview` | toggle the windows |
| `codol_open <file> [line]` | open a file in the workbench |
| `codol_rcon <command>` | send an rcon command, print the answer |
| `codol_reload` | reload the level (see above) |
| `codol_map <map>` | `map <map>` on the server |
| `codol_import_ents <file> [map]` | import map entities |
| `codol_export_ents [noreload]` | save + export the override |
| `codol_attach`, `codol_detach` | dock / release the game window |

The standalone editor's `brush_*` and `patch_*` commands: [Brushes and patches](#brushes-and-patches-the-brush-and-patch-menus).

Other dvars: `codol_reload_on_save`, `codol_rcon_log`, `codol_rcon_password` (not saved).

For unattended testing: `CODOL_STARTUP_CMDS="cmd;wait 5;cmd"` runs console commands once the UI is
up ( words split on spaces, `"quoted words"` kept together: `ed_key angles "60 200 0"`; up to 32 k characters ),
`CODOL_AUTOSHOT=<file.png>` saves the rendered UI frame to that file every two seconds ( the standalone
editor also has `ed_shot <file.png>`: the next frame, at a moment the command list picks ), and
`CODOL_NO_MONO_FONT=1` skips loading the monospace editor font.

## Code layout, build and tests

| Path | |
|---|---|
| `src/codol/` | Engine independent code: script lexer, animation index, map entity conversion, 1.1.1.13 layouts. No radiant or imgui dependency. |
| `src/components/modules/codol_game.*` | rcon, reload, game window, settings |
| `src/ggui/script_workbench.*`, `src/ggui/codol_map_ents.*` | the windows |
| `codol_zone/` | Native 1.1.1.13 fastfile reader: container, every asset type, typed views. Own CMake project, tool `codol_zonetool`, verified against all 146 shipped zones. |
| `tools/codol_mapconv` | fastfile -> radiant `.map` through `codol_zone` |
| `tests/` | unit tests for `src/codol` |

```
cmake -S . -B build-cmake -G "Visual Studio 17 2022" -A Win32 [-DRADIANT_ROOT=<mod tools root>]
cmake --build build-cmake --config Release
build-cmake\Release\codol_tests.exe [<script tree>] [<map>.d3dbsp.ents] [<decrypted>.zone]
```

The upstream premake build files are untouched (`generate-buildfiles_vs22.bat`) but were not re-run on this branch. The tests take optional
real inputs: a script tree is indexed and summarised, an `.ents` file is round-tripped through a
radiant map, a `.zone` has its `MapEnts` header validated against its entity string.

## Roadmap

What the chain above still lacks, in the order that would help a mapper most:

1. **Animations that move in the client.** The map side is complete; the dedicated server has to send script
   model animations so that the client plays them ( `codol_bsp/README.md`, "Script model animations" ).
2. **The create-room list seen.** Needs an account of level 5; the editor's part is written and loaded.
3. **Light closer to the game's compiler**: surfaces cut along the pieces of a BSP instead of a grid, an omni
   light's cone aimed where the players are, baked shadows of see through brushes, brush models lit where they
   stand.
4. **Player collision for static models** ( clip brushes made by the compiler ).
5. **Editing shipped maps**: new world materials drawn, render surfaces of deleted brushes removed, an install
   step; brush texture alignment, terrain, patches and decals of a shipped map in the imported `.map`.
6. **Assets**: delete / rename, a material editor, streamed images.
