# SR Textool

Tool for converting DDS textures to and from the Volition bitmap format.
Supports Saints Row: The Third and Saints Row IV.


## Usage

### Help

Every command has a help flag to print out its usage information. Use this if
you want to know all the available arguments and how to specify them.
```
srtextool --help
srtextool x --help
srtextool a --help
```

### Extract textures

Extract all textures to the current directory.
```
srtextool x professorgenki.cpeg_pc
```

Extract only `professorgenki_sm_n.tga` and `professorgenki_sm_dp.tga`.
```
srtextool x professorgenki.cpeg_pc professorgenki_sm_n.tga professorgenki_sm_dp.tga
```

Extract everything to the folder `extracted` (needs to be created first).
```
srtextool x professorgenki.cpeg_pc -o extracted
```

### Update or add textures

Textures get automatically added it they don't exist. There's no need to
specify which action to use.

Update `professorgenki_sm_n.tga.dds` and add `new_texture.tga.dds`
```
srtextool a professorgenki.cpeg_pc professorgenki_sm_n.tga.dds new_texture.tga.dds
```

Update everything in the current directory
```
srtextool a professorgenki.cpeg_pc -i .
```

Create a new cpeg file
```
srtextool a new.cpeg_pc new_texture.tga.dds
```

Linux only: Update all textures matching `*.dds`
```
srtextool a professorgenki.cpeg_pc *.dds
```

Note: New textures with an alpha channel require the `BM_F_ALPHA` flag to be set. This can be done using the modify command. Existing textures already have it and don't need to be modified.

### Delete textures

Delete `new_texture.tga.dds`
```
srtextool d professorgenki.cpeg_pc new_texture.tga.dds
```

### Modify texture

Change name of `new_texture.tga` to `new_texture2.tga`
```
srtextool m professorgenki.cpeg_pc new_texture.tga -n new_texture2.tga
```

Set alpha flag. A complete list of flags can be found in the [Peg file format] thread.
```
srtextool m professorgenki.cpeg_pc new_texture.tga -f 0x1
```

### List contents

```
srtextool l professorgenki.cpeg_pc
```

### Check file for errors

This command only prints errors. No output means the file is good.
```
srtextool c professorgenki.cpeg_pc
```


## Building

This section only applies if you downloaded the source code. Regular users
can safely ignore it.

### Dependencies

* [CMake]
* Compiler with good C++11 support
* Taywee's [args], which is included in `external` with slight modifications

### Linux

```
mkdir build
cd build
cmake ..
make -j 4
```

If you get an error about which mentions `std::ios_base::failure`, add
`-DGCC_ABI_WORKAROUND=ON` to the cmake command. This applies to all
GCC builds using version 5 or 6 at the time of writing.

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
`cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-win32.cmake`.


## Credits

* \[V\] Knobby for providing the headers of the [Peg file format]
* Scanti for writing the [SR3 Texture Utilities], which were a great starting
  point


## License

[LGPL-3.0+](LICENSE)



[CMake]: https://cmake.org/
[args]: https://github.com/Taywee/args
[Peg file format]: https://www.saintsrowmods.com/forum/threads/peg-file-format.2908/
[SR3 Texture Utilities]: https://www.saintsrowmods.com/forum/threads/sr3-texture-utilities.566/

