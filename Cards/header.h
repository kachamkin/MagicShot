#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <string>
#include <filesystem>
#include <SDL.h>
#include <Windows.h>
#include <Shlobj.h>
#include <gdiplus.h>

using namespace std;
using namespace Gdiplus;

#define EVENT_TIMEOUT 64
#define BORDER_WIDTH 30
#define BORDER_COLOR 60, 60, 60, 100
#define SELECTION_COLOR 255, 0, 0, 255
#define DARK_FILL_COLOR 10, 10, 10, 100
#define SELECTION_WIDTH 3

SDL_Window* gWindow = NULL;
SDL_Surface* gScreenSurface = NULL;
SDL_Texture* text = NULL;
SDL_Renderer* renderer = NULL;
SDL_Rect rect{0, 0, 0, 0};

int initX = 0, initY = 0, prevX = 0, prevY = 0;

bool quit = false;

vector<SDL_Point> pixels;
ULONG_PTR gdiplusToken;
