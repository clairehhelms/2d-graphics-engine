## This 2D Graphics Engine
was built during a semester-long course at UNC Chapel Hill based off the Skia graphics engine and under the instruction of Mike Reed.

Files in the sub-directories `/apps`, `/expected`, `/include` and `/src` and the `Makefile` are part of the starter code. Otherwise, the files are my own.

To test the engine against a set of images, run:
```
make image
mkdir diff
./image -e expected -d diff
open diff/index.html
```

You can also run unit tests:
```
make tests
./tests -v
```

Or speed-test the engine running
```
make bench
./bench
```

If you want to get creative, you can modify the `my_draw_something.cpp` file to use this engine to draw what you desire!

There is a lot of room to still improve on this engine, but I was very happy with what I accomplished and learned while building this!