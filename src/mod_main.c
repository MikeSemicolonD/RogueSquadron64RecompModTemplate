// Example mod: prints once when a mission starts and counts mission frames. Replace it with your own code.
#include "rs64_api.h"
#include "recomputils.h"

static unsigned int mission_frames;

// Runs every mission frame, after the game reads the controllers (see the game's docs/modding-host-api.md for every rs64_on_* event).
RECOMP_CALLBACK("*", rs64_on_mission_frame_pads) void my_mod_on_mission_frame(void) {
    if (mission_frames++ == 0) {
        recomp_printf("[my_mod] mission started (draw distance x%.2f)\n", rs64_draw_dist());
    }
}

// Runs at the end of every mission.
RECOMP_CALLBACK("*", rs64_on_mission_end) void my_mod_on_mission_end(void) {
    recomp_printf("[my_mod] mission ended after %u frames\n", mission_frames);
    mission_frames = 0;
}

// More ways in, by game function name from syms/rogue_squadron.syms.toml:
//   RECOMP_HOOK("runInMissionFrame") void before(void) { ... }        runs before the function
//   RECOMP_HOOK_RETURN("runInMissionFrame") void after(void) { ... }  runs after it returns
//   RECOMP_PATCH unsigned int getSecondaryWeaponCount(unsigned int craft, unsigned int weapon) { return 0xFF; }  replaces it
