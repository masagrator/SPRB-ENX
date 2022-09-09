# SPRB-ENX
Summer Pockets Reflection Blue translation mod for Nintendo Switch

Translation mod is based on mod for PC release from Alka Translations that you can find here:<br>
https://alkatranslations.com/downloads/

# Installation

Mod is compatible only with version 1.0.1 of game. It's not known if it works with Summer Pockets + Reflection Blue DLC. If not, I don't plan to port plugin to it which is required for textures and many texts to be translated.

Emulators are not supported by me, but it doesn't mean it won't work on them.

1. Download mod from Releases
2. copy `atmosphere` folder to root of your sdcard
3. Run game

# Making mod manually

Requirements:
- Python 3.10+ (and library: `numpy`)

For plugin compilation you need:
- devkitpro (with `switch-dev` package installed)

To compile plugin, just run `make` inside Plugin folder.<br>
To compile script use this command inside root of repo:
```cmd
python script_assemble.py ENG SCRIPT2 SCRIPT2.PAK
```

Copy subsdk9 and main.npdm from Plugin folder to
```
atmosphere/contents/0100273013ECA000/exefs
```
Copy SCRIPT2.PAK from root of repo folder to 
```
atmosphere/contents/0100273013ECA000/romfs
```

Assets are available only from Release page.

# Notes
- included differences in script between PC and Switch in SCRIPT2/notes.txt
- Plugin is incomparible with DVR patches

# Thanks to
- `Alka Translation` for making translation mod (not involved with this project)
- `wetor` for making LuckSystem (not involved with this project) that allowed me to convert images to native compressed formats (it's slightly bugged, I needed to write my own script to fix header data after converting each image)
https://github.com/wetor/LuckSystem/issues/4
- `Crispy` for translating Shimapong Fight tutorial
