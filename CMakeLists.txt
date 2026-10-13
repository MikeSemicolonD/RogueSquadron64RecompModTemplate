# Builds the mod in src/ into build/<mod_filename>.nrm:
#
#   cmake -S . -B build -DMIPS_TOOLCHAIN_DIR=<mips64-elf toolchain root> -DRECOMP_MOD_TOOL=<path to RecompModTool>
#   cmake --build build
#
# Both tools are also found on PATH. The .nrm name comes from mod_filename in mod.toml.
cmake_minimum_required(VERSION 3.20)
project(rs64_mod NONE)

set(MIPS_TOOLCHAIN_DIR "" CACHE PATH "MIPS GCC toolchain root (holds bin/mips64-elf-gcc)")
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

file(STRINGS "${CMAKE_SOURCE_DIR}/mod.toml" mod_filename_line REGEX "^mod_filename *=")
string(REGEX REPLACE "^mod_filename *= *\"([^\"]+)\".*" "\\1" MOD_FILENAME "${mod_filename_line}")

set(MOD_OUT "${CMAKE_SOURCE_DIR}/build")
file(GLOB_RECURSE MOD_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.c")
file(GLOB_RECURSE MOD_HEADERS CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.h" "${CMAKE_SOURCE_DIR}/syms/include/*.h")

# The game's code-mod flags. -fno-optimize-sibling-calls keeps calls as jal: a tail jump into a game function or import fails LiveRecomp.
set(MOD_CFLAGS -mips2 -mabi=32 -O2 -G0 -mno-abicalls -fomit-frame-pointer -fno-builtin -fno-optimize-sibling-calls
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

# Game functions stay undefined so RecompModTool resolves them by name; ld flags each call into the game as out of range, which --noinhibit-exec lets through.
add_custom_command(OUTPUT "${MOD_OUT}/mod.elf"
    COMMAND "${MIPS_LD}" -nostdlib -T "${CMAKE_SOURCE_DIR}/mod.ld" -Map "${MOD_OUT}/mod.map"
        --unresolved-symbols=ignore-all --emit-relocs --noinhibit-exec -e 0 ${objs} -o "${MOD_OUT}/mod.elf"
    DEPENDS ${objs} "${CMAKE_SOURCE_DIR}/mod.ld"
    VERBATIM)

add_custom_command(OUTPUT "${MOD_OUT}/${MOD_FILENAME}.nrm"
    COMMAND "${RECOMP_MOD_TOOL}" "${CMAKE_SOURCE_DIR}/mod.toml" "${MOD_OUT}"
    DEPENDS "${MOD_OUT}/mod.elf" "${CMAKE_SOURCE_DIR}/mod.toml"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    COMMENT "Packing ${MOD_FILENAME}.nrm"
    VERBATIM)

add_custom_target(mod ALL DEPENDS "${MOD_OUT}/${MOD_FILENAME}.nrm")
