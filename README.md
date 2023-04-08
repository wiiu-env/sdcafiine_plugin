[![CI-Release](https://github.com/wiiu-env/sdcafiine_plugin/actions/workflows/ci.yml/badge.svg)](https://github.com/wiiu-env/sdcafiine_plugin/actions/workflows/ci.yml)

# SDCafiine Plugin

## What is SDCafiine
The main feature of this plugin is the **on-the-fly replacing of files**, which can be used to load modified content from external media (**SD**). It hooks into the file system functions of the Wii U. Whenever a file is accessed, SDCafiine checks if a (modified) version is present on the SD card, and redirect the file operations if needed.

## Dependencies
Requires the [ContentRedirectionModule](https://github.com/wiiu-env/ContentRedirectionModule) to be loaded.

## Installation of the plugin
(`[ENVIRONMENT]` is a placeholder for the actual environment name.)

1. Copy the file `sdcafiine.wps` into `sd:/wiiu/environments/[ENVIRONMENT]/plugins`.  
2. Requires the [WiiUPluginLoaderBackend](https://github.com/wiiu-env/WiiUPluginLoaderBackend) in `sd:/wiiu/environments/[ENVIRONMENT]/modules`.
3. Requires the [ContentRedirectionModule](https://github.com/wiiu-env/ContentRedirectionModule) in `sd:/wiiu/environments/[ENVIRONMENT]/modules`.

## Usage
**The path of SDCafiine has changed to `sd:/wiiu/sdcafiine`**

Via the plugin config menu (press L, DPAD Down and Minus on the gamepad) you can configure the plugin. The available options are the following:
- **Settings**: 
  - Enable SDCafiine:
    - With this option you can globally enable/disable SDCafiine. If you're currently running a game you need to restart it before this option has an effect.
- **Advanced settings**:
  - Auto apply the modpack if only one modpack exists:
    - Skip the modpack selection screen if the current title only has one modpack to choose from. To boot the game without mods, you need to press X while "Preparing modpack" is shown on the screen.
  - Skip "Preparing modpack..."-screen
    - Skips the "Preparing modpack..."-screen which appears when auto booting into a single modpack for a title is activated. To run the game without mods you need to disable this option.

### Installation of the mods
Before the mods can be loaded, they need to be copied to a SD card. 
**In the following "root:/" is corresponding to the root of your SD card**. The basic filepath structure is this:

```
root:/wiiu/sdcafiine/[TITLEID]/[MODPACK]/content/  <-- for game files. Maps to /vol/content/
root:/wiiu/sdcafiine/[TITLEID]/[MODPACK]/aoc/  <-- for DLC files. Maps to /vol/aoc/
```
Replace the following:
- "[TITLEID]" need to be replaced the TitleID of the games that should be modded. A list of title ids can be found [here](http://wiiubrew.org/w/index.php?title=Title_database#00050000:_eShop_and_disc_titles) (without the "-"). Example for SSBU "0005000010145000". Make sure to use the ID of the fullgame and not the update title ID. 
- "[MODPACK]" name of the modpack.

Example path for the EUR version of SuperSmashBros for Wii U:
```
root:/wiiu/sdcafiine/0005000010145000/SpecialChars/content/  <-- for game files. Maps to /vol/content/
root:/wiiu/sdcafiine/0005000010145000/SpecialChars/aoc/  <-- for DLC files. Maps to /vol/aoc/
```

For replacing the file /vol/content/movie/intro.mp4, put a modified file into:
```
root:/wiiu/sdcafiine/0005000010145000/SpecialChars/content/movie/intro.mp4
```

### Handling multiple mod packs
SDCafiine supports multiple different mods for a single game on the same SD card. Each modpack has its own subdirectory.
Example:
```
sd:/wiiu/sdcafiine/0005000010145000/ModPack1/content/
sd:/wiiu/sdcafiine/0005000010145000/ModPack2/content/
```
### "Delete" files via SDCafiine
If a game should not see or access a file anymore, it's possible to "delete"/"hide" it from the game without actually deleting it. 
The process is similar to redirecting a file. But instead of creating a replacement file with the same name, you create an empty file with the prefix `.deleted_`.

If you want to stop a game from accessing `/vol/content/assets/tree.bin` you need to create this file in your modpack.
```
root:/wiiu/sdcafiine/[TITLEID]/[MODPACK]/content/assets/.deleted_tree.bin
```

## Buildflags

### Logging
Building via `make` only logs errors (via OSReport). To enable logging via the [LoggingModule](https://github.com/wiiu-env/LoggingModule) set `DEBUG` to `1` or `VERBOSE`.

`make` Logs errors only (via OSReport).  
`make DEBUG=1` Enables information and error logging via [LoggingModule](https://github.com/wiiu-env/LoggingModule).  
`make DEBUG=VERBOSE` Enables verbose information and error logging via [LoggingModule](https://github.com/wiiu-env/LoggingModule).  

If the [LoggingModule](https://github.com/wiiu-env/LoggingModule) is not present, it'll fallback to UDP (Port 4405) and [CafeOS](https://github.com/wiiu-env/USBSerialLoggingModule) logging.

## Building using the Dockerfile

It's possible to use a docker image for building. This way you don't need anything installed on your host system.

```
# Build docker image (only needed once)
docker build . -t sdcafiineplugin-builder

# make 
docker run -it --rm -v ${PWD}:/project sdcafiineplugin-builder make

# make clean
docker run -it --rm -v ${PWD}:/project sdcafiineplugin-builder make clean
```

## Format the code via docker

`docker run --rm -v ${PWD}:/src ghcr.io/wiiu-env/clang-format:13.0.0-2 -r ./src -i`