#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <string>
#include <filesystem>
#include <SDL.h>
#include <Windows.h>
#include <Shlobj.h>

using namespace std;

#define EVENT_TIMEOUT 64
#define BORDER_WIDTH 8
#define BORDER_COLOR 0, 0, 255, 255
#define SELECTION_COLOR 255, 0, 0, 255
#define SELECTION_WIDTH 5

SDL_Window* gWindow = NULL;
SDL_Surface* gScreenSurface = NULL;
SDL_Renderer* renderer = NULL;
SDL_Rect rect;

int initX = 0, initY = 0;

bool quit = false;