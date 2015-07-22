# forest - a forest fire simulator written in C


`forest` is a small C project that I wrote for a high performance computing class in 2013.
It simulates forest fires using a cellular automata algorithm.

## Features

 * works with arbitrary grid sizes, as long as they are divisible by 8
 * multiple output modes:
 --* PNG imageset (convertible to a movie with a makefile command)
 --* console output (driven by ncurses)
 --* null output (for benchmarking)
 --* checksum output (for debugging and verification)
 * supports multithreading and parallelisation through OpenMP and MPI


## Requirements

### Mandatory

 * GCC and make (*build-essentials* package on ubuntu)

### Optional

 * OpenMP and MPI (if you want to use parallelisation)
 * Latex (*texlive-latex-base* and *texlive-latex-extra* packages on ubuntu) for viewing the Latex documentation
 * ffmpeg (or avconv/libav, depending on your distribution) for rendering PNG imagesets into h264 movies


## Installation and Usage

Compile with `make`, which will create a binary called `forest` in the current directory.
the included makefile has several options:

 * `make omp` - compiles `forest` with OpenMP support
 * `make mpi-moss` - compiles `forest` with MPI support, using the `mpicc` binary.
 * `make mpi-savanna` - compiles `forest` with MPI support, using the `mpiicc` binary.
 * `make movie` - processes the PNG images found in the `out/` folder into a mp4 movie encoded with x264, using either `avconv` or `ffmpeg`, depending on your distribution.
 * `make lossmovie` - as above, but creates a lossless movie file (much larger size, but no loss of information).
 * `make clean` - removes all temporary files, compiled binaries, imagesets, and movie files.

once compiled, run the `forest` binary like so:

`./forest dimensionX dimensionY output simlength`

`dimensionX` and `dimensionY` must both be divisible by 8.

`output` should be one of 'png' 'ncurses' 'null' 'verify' 'log'.

`simlength` should be greater than 1.

## Development Notes and Documentation

Latex documentation is included in the "tex" folder. run `make report-3` to compile.


## License

The MIT License (MIT)

Copyright (c) 2013-2015 Anthony Vaccaro

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

