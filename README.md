Indirect fork of PalSchema with some changes.

# Installation

1. Install UE4SS in `steamapps/common/The Blood of Dawnwalker/Dawnwalker/Binaries/Win64`
2. Install DawnSchema in `steamapps/common/The Blood of Dawnwalker/Dawnwalker/Binaries/Win64/ue4ss/Mods`

DawnSchema mods go in `steamapps/common/The Blood of Dawnwalker/Dawnwalker/Binaries/Win64/ue4ss/Mods/DawnSchema/mods`

# Documentation

Functionally DawnSchema is slightly different than PalSchema, but you can still use the original docs as a reference for types and raw tables: https://okaetsu.github.io/PalSchema/docs/gettingstarted

The [examples](https://github.com/Okaetsu/DawnSchema/tree/main/assets/examples) folder has some mods you can use as a reference as well.

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