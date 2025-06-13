#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

void find_tsp_files(const char *root_dir, void (*callback)(const char *, void *), void *userdata) {
    char search_path[MAX_PATH];
    snprintf(search_path, sizeof(search_path), "%s\\*", root_dir);
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(search_path, &fd);
    if (hFind == INVALID_HANDLE_VALUE) return;
    do {
        if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0) continue;
        char path[MAX_PATH];
        snprintf(path, sizeof(path), "%s\\%s", root_dir, fd.cFileName);
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            find_tsp_files(path, callback, userdata);
        } else {
            const char *ext = strrchr(fd.cFileName, '.');
            if (ext && _stricmp(ext, ".tsp") == 0) {
                callback(path, userdata);
            }
        }
    } while (FindNextFileA(hFind, &fd));
    FindClose(hFind);
}

struct RunArgs {
    const char *main_exe;
    const char *method;
    const char *log_file;
    int extra_argc;
    char **extra_argv;
};

void run_main_on_tsp(const char *tsp_file, void *userdata) {
    struct RunArgs *args = (struct RunArgs *)userdata;
    char cmd[4096] = {0};
    snprintf(cmd, sizeof(cmd), "\"%s\" -m %s --log \"%s\" \"%s\"", args->main_exe, args->method, args->log_file, tsp_file);
    for (int i = 0; i < args->extra_argc; ++i) {
        strncat(cmd, " ", sizeof(cmd) - strlen(cmd) - 1);
        strncat(cmd, args->extra_argv[i], sizeof(cmd) - strlen(cmd) - 1);
    }
    printf("Running: %s\n", cmd);
    system(cmd);
}

int main(int argc, char *argv[]) {
    if (argc < 5) {
        printf("Usage: %s main_exe root_dir method log_file [extra args ...]\n", argv[0]);
        return 1;
    }
    struct RunArgs args;
    args.main_exe = argv[1];
    const char *root_dir = argv[2];
    args.method = argv[3];
    args.log_file = argv[4];
    args.extra_argc = argc - 5;
    args.extra_argv = argv + 5;
    printf("Searching for .tsp files in %s...\n", root_dir);
    find_tsp_files(root_dir, run_main_on_tsp, &args);
    return 0;
}

