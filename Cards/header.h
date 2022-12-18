#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <string>
#include <filesystem>
#include <SDL.h>
#include <SDL_Image.h>
#include <Windows.h>
#include <Shlobj.h>

using namespace std;

#define EVENT_TIMEOUT 64
#define BORDER_WIDTH 20
#define BORDER_COLOR 0, 0, 255, 0
#define SELECTION_COLOR 255, 0, 0, 0
#define SELECTION_WIDTH 3
#define WINDOW_OPACITY 0.3
#define WINDOW_ICON "/screenshot_icon.png"

SDL_Window* gWindow = NULL;
SDL_Surface* gScreenSurface = NULL;
SDL_Renderer* renderer = NULL;
SDL_Rect rect;

int initX = 0, initY = 0, prevX = 0, prevY = 0;

bool quit = false;

string appDir;

vector<SDL_Point> pixels;