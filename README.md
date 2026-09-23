Indirect fork of PalSchema

# Installation

In-depth installation guide can be found [here](https://okaetsu.github.io/PalSchema/docs/installation)

# Documentation

Documentation for modders can be found [here](https://okaetsu.github.io/PalSchema/docs/gettingstarted)

# Building from Source

1. You must complete the **Build requirements** over at [UE4SS Docs](https://docs.ue4ss.com/#build-requirements) and make sure your GitHub account is linked to Epic Games for Unreal Engine source access.

2. Create your own fork of DawnSchema and clone it.

3. Execute this command: `git submodule update --init --recursive`

4. Execute the following:

Choose either MSVC or Ninja

MSVC (multi-configuration, slower, allows switching configs without reconfiguring)
```
cmake -B build -G "Visual Studio 17 2022"
```

or with Ninja (single-configuration, faster)
```
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Game__Shipping__Win64
```