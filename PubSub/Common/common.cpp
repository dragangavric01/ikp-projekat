#include "common.h"

void window_setup(char* argv[]) {
    int x = atoi(argv[1]);
    int y = atoi(argv[2]);
    int width = atoi(argv[3]);
    int height = atoi(argv[4]);

    HWND hwnd = GetConsoleWindow();
    MoveWindow(hwnd, x, y, width, height, TRUE);
}