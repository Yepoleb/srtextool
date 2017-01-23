# SR Textool

Tool for converting DDS textures to and from the Volition bitmap format. Supports Saints Row: The Third and Saints Row IV


## Usage

### Help

```
srtextool --help
```

### Extract Textures

```
srtextool x professorgenki.cpeg_pc
srtextool x professorgenki.cpeg_pc professorgenki_sm_n.tga professorgenki_sm_dp.tga
srtextool x professorgenki.cpeg_pc -o extracted
```

### Update or add Textures

```
srtextool a professorgenki.cpeg_pc professorgenki_sm_n.tga.dds new_texture.tga.dds
```

Linux only:

```
srtextool a professorgenki.cpeg_pc *.dds
```

### Delete Textures

```
srtextool d professorgenki.cpeg_pc new_texture.tga.dds
```

### List Contents

```
srtextool l professorgenki.cpeg_pc
```

### Check file for errors

```
srtextool c professorgenki.cpeg_pc
```


## Dependencies

* [CMake]
* Compiler with good C++11 support
* Taywee's [args], which is included in `external` with slight modifications


## Building

### Linux

```
mkdir build
cd build
cmake ..
make -j 4
```

### Windows

Using the CMake GUI:

* Select `...\srtextool` as the source location
* Select `...\srtextool\build` as the binary location
* Run "Configure" and "Generate"
* Open the project file in `build` in your IDE or run make, ninja, whatever
  you used as the generator target

### Cross compiling for Windows

First make sure the paths in `cmake/toolchain-win32.cmake` are correct. Then
do the same as for [Linux](#linux), but replace `cmake ..` with
`cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-win32.cmake`


## Credits

* \[V\] Knobby for providing the headers of the [peg file format]
* Scanti for writing the [SR3 Texture Utilities], which were a great starting
  point


## License

[LGPL-3.0+](LICENSE)



[CMake]: https://cmake.org/
[args]: https://github.com/Taywee/args
[peg file format]: https://www.saintsrowmods.com/forum/threads/peg-file-format.2908/
[SR3 Texture Utilities]: https://www.saintsrowmods.com/forum/threads/sr3-texture-utilities.566/

