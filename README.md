# SDCafiine Plugin

## What is SDCafiine
The main feature of this application is the **on-the-fly replacing of files**, which can be used used to loaded modified content from external media (**SD**). It hooks into the file system functions of the WiiU. Whenever a file is accessed, SDCafiine checks if a (modified) version of it present on the SD device, and redirect the file operations if needed.

## Dependecies
Requires the [RPXLoadingModule](https://github.com/wiiu-env/RPXLoadingModule) to be loaded.

## Installation of the modules
(`[ENVIRONMENT]` is a placeholder for the actual environment name.)

1. Copy the file `sdcafiine.wps` into `sd:/wiiu/environments/[ENVIRONMENT]/plugins`.  
2. Requires the [WiiUPluginLoaderBackend](https://github.com/wiiu-env/WiiUPluginLoaderBackend) in `sd:/wiiu/environments/[ENVIRONMENT]/modules`.
3. Requires the [RPXLoadingModule](https://github.com/wiiu-env/RPXLoadingModule) in `sd:/wiiu/environments/[ENVIRONMENT]/modules`.

### Installation of the mods
Before the mods can be loaded, they need to be copied to a SD device. 
**In the following "root:/" is corresponding to the root of your SD device**. The basic filepath structure is this:

```
root:/sdcafiine/[TITLEID]/[MODPACK]/content/  <-- for game files. Maps to /vol/content/
```
Replace the following:
- "[TITLEID]" need to be replaced the TitleID of the games that should be modded. A list of can be found [here](http://wiiubrew.org/w/index.php?title=Title_database#00050000:_eShop_and_disc_titles) (without the "-"). Example for SSBU "0005000010145000". Make sure to use the ID of the fullgame and not the update title ID. 
- "[MODPACK]" name of the modpack. This folder name can be everything but "content" or "aoc".

Example path for the EUR version of SuperSmashBros for Wii U:
```
root:/sdcafiine/0005000010145000/SpecialChars/content/  <-- for game files. Maps to /vol/content/
```

For replacing the file /vol/content/movie/intro.mp4, put a modified file into:
```
root:/sdcafiine/0005000010145000/SpecialChars/content/movie/intro.mp4
```

*NOTES: paths like "root:/sdcafiine/0005000010145000/content/" are still supported for compatibility, but **not recommended** *

### Handling multiple mod packs
SDCafiine supports multiple different mods for a single game on the same SDCard. Each mod has an own subfolder.
Example:
```
sd:/sdcafiine/0005000010145000/ModPack1/content/
sd:/sdcafiine/0005000010145000/ModPack2/content/
```

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
