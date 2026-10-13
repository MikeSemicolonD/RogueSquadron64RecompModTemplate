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

4. Copy the `.nrm` into the game's `mods/` folder and enable it in the Mods panel (F1) or `mods.json`. The example prints a line to the game's console when a mission starts and when it ends, and adds a MY MOD page to the main menu: ANNOUNCE switches its option, REPORT prints its state.

## Making it yours

- **Name:** change `id`, `display_name` and `mod_filename` in `mod.toml`. `id` must be a C identifier.
- **Code:** goes in `src/` (every `.c` file there is built). `src/mod_main.c` shows event callbacks and reading an option; its closing comment shows hooks and patches.
- **Options:** declare them in `mod.toml` (`[[manifest.config_options]]`; the example has an On/Off "Announce missions"). They appear under the mod in the Mods panel (F1), and the code reads them with `recomp_get_config_u32` / `_double` / `_string`.
- **Menu:** `roguesq_menu.json` adds entries to the game's own menus (main menu, options, pause, and pages of your own), packed into the `.nrm` through `additional_files` in `mod.toml`. `"toggle": "config:<option id>"` shows an `Enum` option as a toggle and `"slider": "config:<option id>"` a `Number` option as a slider; `"action": "@name"` calls one of your `RECOMP_EXPORT` functions (`"toggle": "@name"` calls `name` to flip and `name_get` to read). Labels use the menu font: capitals, digits and spaces. The game's [docs/adding-menus-and-buttons.md](https://github.com/MikeSemicolonD/RogueSquadron64Recomp/blob/main/docs/adding-menus-and-buttons.md) has the full schema. Delete the file and its `additional_files` line if you don't want a menu.
- **Instruction patches:** to change only a few of the game's instructions (a constant, a limit), don't copy the function: add `[[patches.instruction]]` entries to `mod.toml` (there's a commented example at the end). Each gives the function name and the instruction's address from `syms/rogue_squadron.syms.toml`, the game's original instruction as `expect_asm` and the new one as `asm`. The build assembles them with your MIPS GCC and packs them into the `.nrm`, and the game checks the original when it loads the mod. With `-DGAME_ROM=<rom.z64>` the build checks it too. Branches and jumps can't be assembled on their own: give those as `value = 0x...`. A mod can be only patches: leave `src/` empty and drop `elf_path` from `mod.toml`. See the [syms README](https://github.com/MikeSemicolonD/RogueSquadron64RecompSyms#instruction-patches).
- **Headers:** `rs64_api.h` (mod macros and the game's exports), `recomputils.h` (`recomp_alloc`, `recomp_free`, `recomp_printf`), `recompconfig.h` (config options) and `recompdata.h` (hashmaps, hashsets, slotmaps), all in `syms/include/`.
- **Linker script:** the build uses `syms/include/mod.ld`. Put a `mod.ld` in the repository root only if you need a different layout; it must not merge the `.recomp_hook.*` sections into one output section.

The [syms README](https://github.com/MikeSemicolonD/RogueSquadron64RecompSyms#readme) covers what mods can call and hook, including the game functions the base recomp patches. [syms/EVENTS.md](https://github.com/MikeSemicolonD/RogueSquadron64RecompSyms/blob/main/EVENTS.md) lists every `rs64_on_*` event and what a callback can read, change or return there.

## Updating

The game's function names change as its decomp names more functions. Before releasing, update the symbols and rebuild:

```
git submodule update --remote syms
cmake --build build
```

A renamed function shows up as a RecompModTool error, not as a broken mod.
