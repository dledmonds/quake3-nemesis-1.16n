# quake3-nemesis-1.16n
Quake 3 1.16n Nemesis client mod

## Download
As the main website is no longer up, the original binaries are available to download from here. Download Nemesis-Client-v2.0.14_462.zip, follow the installation instructions, but use the latest client pk3, zzz-nemesis-client-2-0-14_529.pk3

There is no documentation for the superHUD as it was stored on the forum - while not lost, it's a pain to recover. Hopefully, anyone downloading Nemesis client after all these years already knows how to configure the superHUD themselves (or use one of the default configurations provided in `hud` directory which include documentation).

## Development
It's easier if you checkout the repository into `\quake3\nemesis` as some paths are hardcoded to use that location - note it will work in other locations, but you will need to update the configuration yourself.

The code lives in the `code` directory, cgame is client side, ui is user interface and game is server side. Each of these directories contains a bat file used to compile (apart from game as server side is not included). You will need to add the `bin` directory to your PATH for this to work. If you checked out into another directory, update the q3asm files as the first line is currently set to `-o "\quake3\nemesis\vm\..."

If the code built successfully, you'll find a `dist-pk3.bat` file in the main directory to package the client and models pk3 files. It expects to find a `vm` folder in the main directory that contains the generated qvm files from compilation. If compilation succeeded, but the `vm` directory doesn't exist, or is empty, check the output folder setting for the q3asm stage. If it all worked, `zzz-nemesis-client-2-0-X.pk3` and `zzz-nemesis-models.pk3` will be generated in the main directory. `dist-zip.bat` can be used to create the final package that includes documentation.

You can also setup the code in visual studio and use DLLs for testing, but you'll need to setup the solution and configure it.
