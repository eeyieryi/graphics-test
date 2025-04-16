#define NOB_IMPLEMENTATION
#include "nob.h"

#define PLATFORM_OS LINUX

void add_raylib(Nob_Cmd *cmd) {
#define RAYLIB_SRC_PATH "../../pkgs/raylib/raylib-5.5/src/"
    nob_cmd_append(cmd, "-I"RAYLIB_SRC_PATH);
#if PLATFORM_OS == LINUX
#define RAYLIB_LIBRARY_PATH RAYLIB_SRC_PATH
    nob_cmd_append(cmd, "-L"RAYLIB_LIBRARY_PATH);
    nob_cmd_append(cmd, "-l:libraylib.a");
#elif PLATFORM_OS == OSX
    nob_cmd_append(cmd, "-lraylib");
    nob_cmd_append(cmd, "-framework", "IOKit", "-framework", "Cocoa", "-framework", "OpenGL");
#endif
    nob_cmd_append(cmd, "-lm", "-ldl", "-lpthread");
}

int main(int argc, char **argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);
    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, "cc", "-Wall", "-Wextra", "-ggdb");
    nob_cmd_append(&cmd, "-I.");
    nob_cmd_append(&cmd, "-o", "main");
    nob_cmd_append(&cmd, "graphics.c");
    add_raylib(&cmd);

    if (!nob_cmd_run_sync(cmd)) return 1;
    return 0;
}
