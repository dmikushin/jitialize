# Jitialize: specialize a C/C++ function for a parameter value in runtime

Specialize a C/C++ function for a parameter value in runtime using the LLVM Just-In-Time compiler.

## Prerequisites

Prerequisites are given for Ubuntu 22.04 (Jammy):

```bash
sudo apt install build-essential git unzip cmake llvm-12-dev clang-12 libclang-12-dev libz-dev libxml2-dev python3-pip
sudo pip3 install lit
```

## Building

```bash
mkdir build
cd build
cmake ..
cmake --build .
cmake --build . --target check
```

## Example usage

To build the examples, install the [opencv](https://opencv.org/) library, 
and add the flag `-DJITIALIZE_EXAMPLE=1` to the CMake command:

```bash
sudo apt install libopencv-dev
cd build
cmake -DJITIALIZE_EXAMPLE=ON ..
make
bin/jitialize-example 1 1 2 3 4 s 1 2 3 4 s 1 2 3 4 q
```

## Benchmarking

To enable benchmarking, add the flag `-DJITIALIZE_BENCHMARK=1` to the CMake command:

```bash
cd build
cmake -DJITIALIZE_BENCHMARK=ON ..
make
bin/jitialize-benchmark
```

## Docker

If you want to give only a quick test to the project, everything is provided to use it with docker.
To do this, generate a Dockerfile from the current directory using the scripts in `<path_to_jitialize_src>/misc/docker`, 
then generate your docker instance.

```bash
python3 <path_to_jitialize_src>/misc/docker/GenDockerfile.py  <path_to_jitialize_src>/.travis.yml > Dockerfile
docker build -t jitialize/test -f Dockerfile
docker run -ti jitialize/test /bin/bash
```

## Guidelines

### Compiling my project with Jitialize

Since the Jitialize library relies on assistance from the compiler, its
mandatory to load a compiler plugin in order to use it.
The flag `-Xclang -load -Xclang <path_to_jitialize_build>/bin/JitializePass.so`
loads the plugin.

The included headers require C++14 support, and remember to add the include directories!
Use `--std=c++14 -I<path_to_jitialize_src>/cpplib/include`.

Finaly, the binary must be linked against the Jitialize runtime library, using
`-L<path_to_jitialize_build>/bin -lJitializeRuntime`.

Putting all together we get the command bellow.

```bash
clang++-6.0 --std=c++14 <my_file.cpp> \
  -Xclang -load -Xclang /path/to/jitialize/jit/build/bin/bin/JitializePass.so \
  -I<path_to_jitialize_src>/cpplib/include \
  -L<path_to_jitialize_build>/bin -lJitializeRuntime
```

### Using Jitialize inside my project

Consider the code below from a software that applies image filters on a video stream.
In the following sections we are going to adapt it to use the Jitialize library.
The function to optimize is `kernel`, which applies a mask on the entire image.

The mask, its dimensions and area do not change often, so specializing the function for
these parameters seems reasonable.
Moreover, the image dimensions and number of channels typically remain constant during
the entire execution; however, it is impossible to know their values as they depend on the stream.

```cpp
static void kernel(const char* mask, unsigned mask_size, unsigned mask_area,
                   const unsigned char* in, unsigned char* out,
                   unsigned rows, unsigned cols, unsigned channels) {
  unsigned mask_middle = (mask_size/2+1);
  unsigned middle = (cols+1)*mask_middle;

  for(unsigned i = 0; i != rows-mask_size; ++i) {
    for(unsigned j = 0; j != cols-mask_size; ++j) {
      for(unsigned ch = 0; ch != channels; ++ch) {

        long out_val = 0;
        for(unsigned ii = 0; ii != mask_size; ++ii) {
          for(unsigned jj = 0; jj != mask_size; ++jj) {
            out_val += mask[ii*mask_size+jj] * in[((i+ii)*cols+j+jj)*channels+ch];
          }
        }
        out[(i*cols+j+middle)*channels+ch] = out_val / mask_area;
      }
    }
  }
}

static void apply_filter(const char *mask, unsigned mask_size, unsigned mask_area, cv::Mat &image, cv::Mat *&out) {
  kernel(mask, mask_size, mask_area, image.ptr(0,0), out->ptr(0,0), image.rows, image.cols, image.channels());
}
```

The main header for the library is `jitialize/jit.h`, where the only core function
of the library is exported. This function is called -- guess how? -- `jitialize::jit`.
We add the corresponding include directive them in the top of the file.

```cpp
#include <jitialize/jit.h>
```

With the call to `jitialize::jit`, we specialize the function and obtain a new
one taking only two parameters (the input and the output frame).

```cpp
static void apply_filter(const char *mask, unsigned mask_size, unsigned mask_area, cv::Mat &image, cv::Mat *&out) {
  using namespace std::placeholders;

  auto kernel_opt = jitialize::jit(kernel, mask, mask_size, mask_area, _1, _2, image.rows, image.cols, image.channels());
  kernel_opt(image.ptr(0,0), out->ptr(0,0));
}
```

#### Deducing which functions to expose at runtime

Jitialize embeds the [LLVM bitcode](https://llvm.org/docs/LangRef.html)
representation of the functions to specialize at runtime in the binary code.
To perform this, the library requires access to the implementation of these
functions.
Jitialize does an effort to deduce which functions are specialized at runtime,
still in many cases this is not possible.

In this case, it's possible to use the `JITIALIZE_EXPOSE` macro, as shown in
the following code,

```cpp
void JITIALIZE_EXPOSE kernel() { /* ... */ }
```

or using a regular expression during compilation.
The command bellow exports all functions whose name starts with "^kernel".

```bash
clang++ ... -mllvm -jitialize-export="^kernel.*"  ...
```

#### Caching

In parallel to the `jitialize/jit.h` header, there is `jitialize/code_cache.h` which
provides a code cache to avoid recompilation of functions that already have been
generated.

Bellow we show the code from previous section, but adapted to use a code cache.

```cpp
#include <jitialize/code_cache.h>
```

```cpp
static void apply_filter(const char *mask, unsigned mask_size, unsigned mask_area, cv::Mat &image, cv::Mat *&out) {
  using namespace std::placeholders;

  static jitialize::Cache<> cache;
  auto const &kernel_opt = cache.jit(kernel, mask, mask_size, mask_area, _1, _2, image.rows, image.cols, image.channels());
  kernel_opt(image.ptr(0,0), out->ptr(0,0));
}
```


## Talks 
 
* [Easy::jit at EuroLLVM'18](https://www.youtube.com/watch?v=sFxqI6Z_bhE)
* [Easy::jit at FOSDEM'18](https://www.youtube.com/watch?v=5_rydTiB32I)


## License

See file [LICENSE](LICENSE) at the top-level directory of this project.


## Thanks

Special thanks to Quarkslab for their support on working in personal projects.


## Warriors

Serge Guelton (serge_sans_paille)

Juan Manuel Martinez Caamaño (jmmartinez)

Kavon Farvardin (kavon) author of [atJIT](https://github.com/kavon/atJIT)
