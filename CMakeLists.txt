# Builds the mod in src/ into build/<mod_filename>.nrm:
#
#   cmake -S . -B build -DMIPS_TOOLCHAIN_DIR=<mips64-elf toolchain root> -DRECOMP_MOD_TOOL=<path to RecompModTool> [-DGAME_ROM=<rom.z64>]
#   cmake --build build
#
# Both tools are also found on PATH. The .nrm name comes from mod_filename in mod.toml.
cmake_minimum_required(VERSION 3.20)
project(rs64_mod NONE)

set(MIPS_TOOLCHAIN_DIR "" CACHE PATH "MIPS GCC toolchain root (holds bin/mips64-elf-gcc)")
set(GAME_ROM "" CACHE FILEPATH "Optional: the game ROM (big-endian .z64), to check [[patches.instruction]] entries against it at build time")
find_program(MIPS_GCC NAMES mips64-elf-gcc HINTS "${MIPS_TOOLCHAIN_DIR}/bin")
find_program(MIPS_LD NAMES mips64-elf-ld HINTS "${MIPS_TOOLCHAIN_DIR}/bin")
find_program(RECOMP_MOD_TOOL NAMES RecompModTool)
foreach(tool MIPS_GCC MIPS_LD RECOMP_MOD_TOOL)
    if (NOT ${tool})
        message(FATAL_ERROR "${tool} not found: set MIPS_TOOLCHAIN_DIR / RECOMP_MOD_TOOL, or put the tool on PATH.")
    endif()
endforeach()
if (NOT EXISTS "${CMAKE_SOURCE_DIR}/syms/include/rs64_api.h")
    message(FATAL_ERROR "syms/ is empty: run git submodule update --init")
endif()
if (NOT EXISTS "${CMAKE_SOURCE_DIR}/mod.ld" AND NOT EXISTS "${CMAKE_SOURCE_DIR}/syms/include/mod.ld")
    message(FATAL_ERROR "syms/ predates the shared linker script: run git submodule update --remote syms")
endif()

file(STRINGS "${CMAKE_SOURCE_DIR}/mod.toml" mod_filename_line REGEX "^mod_filename *=")
string(REGEX REPLACE "^mod_filename *= *\"([^\"]+)\".*" "\\1" MOD_FILENAME "${mod_filename_line}")

set(MOD_OUT "${CMAKE_SOURCE_DIR}/build")
file(GLOB_RECURSE MOD_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.c")
file(GLOB_RECURSE MOD_HEADERS CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.h" "${CMAKE_SOURCE_DIR}/syms/include/*.h")

# The game's code-mod flags. -fno-optimize-sibling-calls keeps calls as jal: a tail jump into a game function or import fails LiveRecomp; -fno-stack-protector because distro MIPS GCCs enable it and the game has no guard symbol.
set(MOD_CFLAGS -mips2 -mabi=32 -O2 -G0 -mno-abicalls -fomit-frame-pointer -fno-builtin -fno-optimize-sibling-calls -fno-stack-protector
    -nostdinc -I "${CMAKE_SOURCE_DIR}/syms/include" -I "${CMAKE_SOURCE_DIR}/src" -Wall -Wextra -Wno-unused-parameter)

set(objs "")
foreach(src ${MOD_SOURCES})
    file(RELATIVE_PATH rel "${CMAKE_SOURCE_DIR}/src" "${src}")
    string(REPLACE "/" "_" obj_name "${rel}")
    set(obj "${MOD_OUT}/obj/${obj_name}.o")
    add_custom_command(OUTPUT "${obj}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${MOD_OUT}/obj"
        COMMAND "${MIPS_GCC}" ${MOD_CFLAGS} -c "${src}" -o "${obj}"
        DEPENDS "${src}" ${MOD_HEADERS}
        VERBATIM)
    list(APPEND objs "${obj}")
endforeach()

# The SDK's linker script, unless the mod brings its own mod.ld.
if (EXISTS "${CMAKE_SOURCE_DIR}/mod.ld")
    set(MOD_LD "${CMAKE_SOURCE_DIR}/mod.ld")
else()
    set(MOD_LD "${CMAKE_SOURCE_DIR}/syms/include/mod.ld")
endif()

# Game functions stay undefined so RecompModTool resolves them by name; ld flags each call into the game as out of range, which --noinhibit-exec lets through.
# No sources: a mod of only [[patches.instruction]] entries (mod.toml has no elf_path).
set(MOD_ELF_DEP "")
if (objs)
    add_custom_command(OUTPUT "${MOD_OUT}/mod.elf"
        COMMAND "${MIPS_LD}" -nostdlib -T "${MOD_LD}" -Map "${MOD_OUT}/mod.map"
            --unresolved-symbols=ignore-all --emit-relocs --noinhibit-exec -e 0 ${objs} -o "${MOD_OUT}/mod.elf"
        DEPENDS ${objs} "${MOD_LD}"
        VERBATIM)
    set(MOD_ELF_DEP "${MOD_OUT}/mod.elf")
endif()

# --as assembles mod.toml's [[patches.instruction]] entries; --rom checks them against the game.
set(MOD_TOOL_ARGS --as "${MIPS_GCC}")
if (GAME_ROM)
    list(APPEND MOD_TOOL_ARGS --rom "${GAME_ROM}")
endif()

# Files mod.toml packs with additional_files repack the .nrm when they change.
file(GLOB MOD_EXTRA_FILES "${CMAKE_SOURCE_DIR}/roguesq_menu.json")
add_custom_command(OUTPUT "${MOD_OUT}/${MOD_FILENAME}.nrm"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${MOD_OUT}"
    COMMAND "${RECOMP_MOD_TOOL}" "${CMAKE_SOURCE_DIR}/mod.toml" "${MOD_OUT}" ${MOD_TOOL_ARGS}
    DEPENDS ${MOD_ELF_DEP} "${CMAKE_SOURCE_DIR}/mod.toml" ${MOD_EXTRA_FILES}
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    COMMENT "Packing ${MOD_FILENAME}.nrm"
    VERBATIM)

add_custom_target(mod ALL DEPENDS "${MOD_OUT}/${MOD_FILENAME}.nrm")
