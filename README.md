# erc
### (Emulator of old, Retro Computers)

---

Erc is software that allows you to emulate computers from the days of yore. (The "days of yore" may be defined as the 1980s.) You may find it interesting if you feel nostalgia for those old machines, or if you want to see how those computers worked. 

Erc is also a sound you might make when feeling slightly frustrated, or if you were a small cat that happens to make funny sounds that aren't quite meows.

## Running

Right now, erc is mostly unusable; large components of it are still being built out. However, if you do run it, you will see a string that reads:

> Hello, world!

In and of itself, this is not so bad, as the software is at least cheerful. If you were having a bad day, seeing "Hello, world!" might make you feel a little bit better.

## Compiling and installing

This software uses CMake to build its makefiles, so if you're familiar with that, you should feel somewhat at home.

If you've never compiled any C code before, you will need to install a few things first. If you have not done so, you should install the excellent [Homebrew](https://brew.sh/) if using a Mac. If you are using Linux, you are probably already acquainted with your local package manager. You will also need to have XCode installed in a Mac environment. You can do so through the App Store.

The following other things you must install are given as Homebrew commands. If you are using Linux, I will leave the appropriate commands you must run as an exercise for the reader.

```
brew install cmake
brew install snaipe/soft/criterion
```

Once all that is accomplished, you can do this:

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
make; ./erc-test
```

This will execute the testing build of the software, which is handled through [Criterion](https://github.com/Snaipe/Criterion).

## Contributing

Right now, I am not accepting pull requests as so much of the design of erc is in flux. I am happy to receive any issues you may file.
