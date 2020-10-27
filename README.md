# ascii-conv
Simple converter png-images -> text for windows.
____
### Build and usage instructions

For now project supports only gcc with common structure of bin/ obj/ directories.
You can run these commands in your MinGW from project's root directory (build_flags are changeable):
```bash
# Debug example.
mingw32-make --directory=./obj/Debug --makefile=../../makefile build_flags="-O0 -g3 -D_DEBUG" build_dir=Debug
# Release example.
mingw32-make --directory=./obj/Release --makefile=../../makefile build_flags="-O2 -march=native -fomit-frame-pointer" build_dir=Release
```
*note: libpng and zlib are needed!*

conv.exe searches for images in ../../arts/ relative to it's own directory (some sort of $PATH with customizeable list of art directories will be added in the future). Just copy your **image.png** into arts/ and run
```bash
conv image
```
____
### Parameters (you can use UNIX-style "-h" or Windows-style "/?"):
```
Usage: conv image_name [-p <number>] [-s <number>] [-w]
```
1. **image_name**. If you want to convert an image with other name than input.png, you should put it's name as a first parameter (strictly), without extension. You can use a paths into nested directories arts/\*, which names must be without spaces.

2. **/p**. Population size for genetic algorhytm which is finding the most suitable palette for each image. Recommended range is 400 .. 1600.

3. **/s**. Sensitivity for colour's comparations. The lower the value — the stricter the selection of colors. For example, if sensitivity = 16, colours #ffaa00 and #ffaa0e will be treated as equal. Feel free to experiment with this one! Recommended range is 10 .. 30.

4. **/w**. Creates file output.png with source image, but coloured in finded 4-bit palette.

5. **/h** or **/?**. Shows help.
____

![Alt-текст](https://cdn.everypony.ru/storage/04/45/32/2020/10/25/147b54fce8.png "Lyra Heartstrings")

![Alt-текст](https://cdn.everypony.ru/storage/04/45/32/2020/10/25/2f14c6a009.png "Sweetie Belle")

![Alt-текст](https://cdn.everypony.ru/storage/04/45/32/2020/10/25/c523b0aa4f.png "Yona")

![Alt-текст](https://cdn.everypony.ru/storage/04/45/32/2020/10/25/96b482e569.png "Some building")
