## RetroMap: A tilemap editor

![Screenshot of the main window](/home/loxo/Nextcloud/Projects/Qt/Mapped/.readme/screen1.png)

RetroMap is a tilemap editor that is specifically aimed at GameBoy Advance development, altough I want it to have a bigger scope in the future.  
It was born out of the need to have a tool that could handle 4bpp tilesets and different maps that use the same graphics data.  
**RetroMap does not have any way of editing the tileset graphics (for now)**, for that you need to use an external image editor that can export as 8 bit indexed bitmaps.
For that you can use [GIMP](https://www.gimp.org/) or [Usenti](https://www.coranac.com/projects/usenti/).

### Download
- You can get the latest binaries of RetroMap from Github Actions in the Artifacts section (you need to be logged in to Github)  
or  
- Check out the [Releases](https://github.com/LoxoSoftware/RetroMap/releases)

### Building

#### Debian based GNU/Linux distributions (Ubuntu, Mint, ...)
1. Install dependencies  
	`sudo apt install gcc g++ make git cmake qt6-base-dev`
2. Clone this repository and go to it  
	`git clone https://github.com/LoxoSoftware/RetroMap.git; cd RetroMap`
3. Configure the project with CMake  
	`cmake -S . -B build -G "Unix Makefiles"`
4. Build the project  
	`cmake --build build --target all --parallel`  
And now you should have the executable in the "build" folder, to run it:  
	`./RetroMap`

### Notice!

This program is still under heavy developement, and thus things may break and give unexpected results.  
It is strongly reccomended that you **create backup copies of your assets**!  
