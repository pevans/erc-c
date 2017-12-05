## Compiling and installing

This software uses CMake to build its makefiles, so if you're familiar with that, you should feel somewhat at home. Do this:

```
cd build
cmake ..
make
```

This should produce an executable of the emulator, which you can install wherever you wish.

## Testing

If you're feeling a bit nosy, you can run unit tests by doing the following:

```
cd tests/build
cmake ..
make; ./emp-test
```

This will execute the testing build of the software, which is handled through [Criterion](https://github.com/Snaipe/Criterion). Note you must have already installed Criterion for this to work.