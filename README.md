# Rogue Squadron 64 Recompiled mod template

A starting point for code mods (`.nrm`) for [Rogue Squadron 64 Recompiled](https://github.com/MikeSemicolonD/RogueSquadron64Recomp). The `syms` submodule ([RogueSquadron64RecompSyms](https://github.com/MikeSemicolonD/RogueSquadron64RecompSyms)) holds the game's symbols and the SDK headers, so the game itself isn't needed to build a mod.

## Getting started

1. Use this repository as a template on GitHub, or clone it with its submodule:

   ```
   git clone --recursive https://github.com/MikeSemicolonD/RogueSquadron64RecompModTemplate.git my-mod
   ```

   Run `git submodule update --init` if you cloned without `--recursive`.

2. Install the tools:
   - **MIPS GCC** (`mips64-elf-gcc` and `mips64-elf-ld`). Windows builds are at [n64-tools/gcc-toolchain-mips64](https://github.com/n64-tools/gcc-toolchain-mips64/releases).
   - **RecompModTool**, built from [N64ModernRuntime](https://github.com/MikeSemicolonD/N64ModernRuntime) (`N64Recomp/`, target `RecompModTool`).
   - **CMake** 3.20 or later.

3. Build:

   ```
   cmake -S . -B build -DMIPS_TOOLCHAIN_DIR=<toolchain root> -DRECOMP_MOD_TOOL=<path to RecompModTool>
   cmake --build build
   ```

   Either tool can be on `PATH` instead. The result is `build/my_mod.nrm`.

4. Copy the `.nrm` into the game's `mods/` folder and enable it in the Mods panel (F1) or `mods.json`. The example prints a line to the game's console when a mission starts and when it ends.

## Making it yours

- **Name:** change `id`, `display_name` and `mod_filename` in `mod.toml`. `id` must be a C identifier.
- **Code:** goes in `src/` (every `.c` file there is built). `src/mod_main.c` shows an event callback; its closing comment shows hooks and patches.
- **Headers:** `rs64_api.h` (mod macros and the game's exports), `recomputils.h` (`recomp_alloc`, `recomp_free`, `recomp_printf`), `recompconfig.h` (config options) and `recompdata.h` (hashmaps, hashsets, slotmaps), all in `syms/include/`.

The [syms README](https://github.com/MikeSemicolonD/RogueSquadron64RecompSyms#readme) covers what mods can call and hook, including the game functions the base recomp patches. The game's `docs/modding-host-api.md` lists every `rs64_on_*` event.

## Updating

The game's function names change as its decomp names more functions. Before releasing, update the symbols and rebuild:

```
git submodule update --remote syms
cmake --build build
```

A renamed function shows up as a RecompModTool error, not as a broken mod.
