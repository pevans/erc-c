# erc
### (Emulator of old, Retro Computers)

---

Erc is software that allows you to emulate computers from the days of yore. (The "days of yore" may be defined as the 1980s.) You may find it interesting if you feel nostalgia for those old machines, or if you want to see how those computers worked. 

Erc is also a sound you might make when feeling slightly frustrated, or if you were a small cat that happens to make funny sounds that aren't quite meows.

## Goals

I've long had an interest in retro computing, collecting books on the 6502 processor in particular, and this program has been written and rewritten in fits and starts over several years (and in several languages!). Being able to emulate the Apple II, which was my childhood computer, has been a dream of mine for some time!

Erc is written in C, largely because I have long loved that language, though I have not had much need to use it in my professional career. C is a somewhat cranky language, and not one that lends itself to modern engineering practices, so erc was partly written as a challenge to myself to try and elevate the language.

In particular, it's a goal of mine to ensure that erc:

* is written for other developers to read, especially as a reference for those who are interested in emulation in general and the platforms emulated here in particular;
* is modular, allowing as much code reuse within the application as is practical;
* is unit-testable, to the extent that C allows, and maintains a high level of code coverage.

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
