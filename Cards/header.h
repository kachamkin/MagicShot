#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <string>
#include <filesystem>
#include <SDL.h>
#include <SDL_image.h>
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
#define SELECTION_BORDER 5
#define SDL_DELAY 20
#define BUTTON_PATH "/rect.png"
#define PRESSED_PATH "/pressed.png"

bool pressed = false;
bool selectionExists = false;

SDL_Window* gWindow = NULL;
SDL_Surface* gScreenSurface = NULL;
SDL_Texture* text = NULL;
SDL_Texture* button = NULL;
SDL_Texture* pressedButton = NULL;
SDL_Renderer* renderer = NULL;
SDL_Rect rect{0, 0, 0, 0};
SDL_Rect buttonRect{ 0, 0, 32, 32 };

int initX = 0, initY = 0, prevX = 0, prevY = 0;

string appPath;
vector<SDL_Point> pixels;

GdiplusStartupInput gdiplusStartupInput;
ULONG_PTR gdiplusToken;
Rect redRect;

HDC hdc = NULL;
Gdiplus::Graphics* graphics = NULL;
