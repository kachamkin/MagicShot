#include "header.h"

wchar_t* a2w(const char* c)
{
	size_t len = strlen(c);
	wchar_t* wc = new wchar_t[len + 1] {L'\0'};
	if (!wc)
		return NULL;

	for (size_t i = 0; i < len; i++)
		wc[i] = (wchar_t)c[i];

	return wc;
}

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;          
	UINT  size = 0;         

	ImageCodecInfo* pImageCodecInfo = NULL;

	GetImageEncodersSize(&num, &size);
	if (!size)
		return -1;  

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if (!pImageCodecInfo)
		return -1;  

	GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (!wcscmp(pImageCodecInfo[j].MimeType, format))
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  
		}
	}

	free(pImageCodecInfo);
	return 0;
}

void copyAsFile(HBITMAP hBitmap, bool png = false)
{
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	Bitmap bm(hBitmap, 0);

	CLSID clsid;
	string fileName;

	if (png)
	{
		GetEncoderClsid(L"image/png", &clsid);
		fileName = filesystem::temp_directory_path().string() + "/Screenshot.png";
	}
	else
	{
		GetEncoderClsid(L"image/bmp", &clsid);
		fileName = filesystem::temp_directory_path().string() + "/Screenshot.bmp";
	}

	wchar_t* fileNameW = a2w(fileName.data());
	bm.Save(fileNameW, &clsid, NULL);
	delete[] fileNameW;
	
	int size = sizeof(DROPFILES) + ((lstrlenA(fileName.data()) + 2));
	HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, size);
	if (!hGlobal)
		return;

	DROPFILES* df = (DROPFILES*)GlobalLock(hGlobal);
	if (!df)
		return;

	ZeroMemory(df, size);
	df->pFiles = sizeof(DROPFILES);
	df->fWide = FALSE;
	lstrcpyA((LPSTR)(df + 1), fileName.data());

	GlobalUnlock(hGlobal);
	SetClipboardData(CF_HDROP, hGlobal);
}

void copyToClipboard(bool isDesktop = true, int x = 0, int y = 0, int w = 0, int h = 0, bool png = false)
{
	if (!w && !h)
	{
		w = GetSystemMetrics(SM_CXVIRTUALSCREEN) - 1;
		h = GetSystemMetrics(SM_CYVIRTUALSCREEN);
	}

	HWND hWnd = NULL;
	if (isDesktop)
	{
		hWnd = GetDesktopWindow();
		if (!hWnd)
			return;
	}

	HDC hDC = GetDC(hWnd);
	if (!hDC)
		return;

	HDC memDC = CreateCompatibleDC(hDC);
	if (!memDC)
		return;

	HBITMAP bm = CreateCompatibleBitmap(hDC, w, h);
	if (!bm)
		return;

	HGDIOBJ oldbm = SelectObject(memDC, bm);
	if (!oldbm)
		return;

	if (!BitBlt(memDC, 0, 0, w, h, hDC, x, y, SRCCOPY | CAPTUREBLT))
		return;

	OpenClipboard(NULL);
	EmptyClipboard();
	SetClipboardData(CF_BITMAP, bm);

	copyAsFile(bm, png);

	CloseClipboard();

	SelectObject(memDC, oldbm);

	DeleteObject(oldbm);
	DeleteObject(bm);
	DeleteDC(memDC);
	ReleaseDC(hWnd, hDC); 

	GdiplusShutdown(gdiplusToken);
}

bool init()
{
	bool success = true;
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		success = false;
	else
	{
		copyToClipboard();
		gWindow = SDL_CreateWindow("MagicShot", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 0, 0, SDL_WINDOW_SHOWN | SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_FULLSCREEN_DESKTOP);
		if (!gWindow)
			success = false;
		else
			gScreenSurface = SDL_GetWindowSurface(gWindow);
	}

	return success;
}

void close()
{
	if (text)
		SDL_DestroyTexture(text);
	if (renderer)
		SDL_DestroyRenderer(renderer);
	if (gScreenSurface)
		SDL_FreeSurface(gScreenSurface);
	SDL_Quit();
}

SDL_Rect getInnerRect()
{
	SDL_Rect newRect = rect;
	newRect.x = rect.x + BORDER_WIDTH;
	newRect.w = rect.w - 2 * BORDER_WIDTH;
	newRect.y = rect.y + BORDER_WIDTH;
	newRect.h = rect.h - 2 * BORDER_WIDTH;

	return newRect;
}

bool pointAtVerticalBorder(int x)
{
	return x < rect.x + rect.w && x > rect.x + rect.w - BORDER_WIDTH || x < rect.x + BORDER_WIDTH && x > rect.x;
}

bool pointAtHorizontalBorder(int y)
{
	return y < rect.y + rect.h && y > rect.y + rect.h - BORDER_WIDTH || y < rect.y + BORDER_WIDTH && y > rect.y;
}

void setCursor(int x, int y)
{
	if (!rect.w)
	{
		SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW));
		return;
	}

	bool atV = pointAtVerticalBorder(x);
	bool atH = pointAtHorizontalBorder(y);

	if (atV && atH)
	{
		if (x < rect.x + BORDER_WIDTH && x > rect.x && y < rect.y + BORDER_WIDTH && y > rect.y || x < rect.x + rect.w && x > rect.x + rect.w - BORDER_WIDTH && y < rect.y + rect.h && y > rect.y + rect.h - BORDER_WIDTH)
			SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE));
		else if (x < rect.x + rect.w && x > rect.x + rect.w - BORDER_WIDTH && y < rect.y + BORDER_WIDTH && y > rect.y || x < rect.x + BORDER_WIDTH && x > rect.x && y < rect.y + rect.h && y > rect.y + rect.h - BORDER_WIDTH)
			SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENESW));
		else
			SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW));
		return;
	}

	if (atV || atH)
	{
		if (atV)
			SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE));
		if (atH)
			SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS));
	}
	else
		SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW));
}

void drawRectangle()
{
	SDL_Rect newRect = getInnerRect();

	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, text, NULL, NULL);
	SDL_SetRenderDrawColor(renderer, DARK_FILL_COLOR);

	SDL_Rect rectToFill;

	rectToFill.x = 0;
	rectToFill.y = 0;
	rectToFill.w = gScreenSurface->w;
	rectToFill.h = rect.y;
	SDL_RenderFillRect(renderer, &rectToFill);

	rectToFill.x = 0;
	rectToFill.y = rect.y + rect.h;
	rectToFill.w = gScreenSurface->w;
	rectToFill.h = gScreenSurface->h - rect.y - rect.h;
	SDL_RenderFillRect(renderer, &rectToFill);

	rectToFill.x = 0;
	rectToFill.y = rect.y;
	rectToFill.w = rect.x;
	rectToFill.h = rect.h;
	SDL_RenderFillRect(renderer, &rectToFill);

	rectToFill.x = rect.x + rect.w;
	rectToFill.y = rect.y;
	rectToFill.w = gScreenSurface->w - rect.x - rect.w;
	rectToFill.h = rect.h;
	SDL_RenderFillRect(renderer, &rectToFill);

	SDL_SetRenderDrawColor(renderer, BORDER_COLOR);

	rectToFill.x = rect.x;
	rectToFill.y = rect.y;
	rectToFill.w = rect.w;
	rectToFill.h = BORDER_WIDTH;
	SDL_RenderFillRect(renderer, &rectToFill);

	rectToFill.x = rect.x;
	rectToFill.y = rect.y + rect.h - BORDER_WIDTH;
	rectToFill.w = rect.w;
	rectToFill.h = BORDER_WIDTH;
	SDL_RenderFillRect(renderer, &rectToFill);

	rectToFill.x = rect.x;
	rectToFill.y = rect.y + BORDER_WIDTH;
	rectToFill.w = BORDER_WIDTH;
	rectToFill.h = rect.h - 2 * BORDER_WIDTH;
	SDL_RenderFillRect(renderer, &rectToFill);

	rectToFill.x = rect.x + rect.w - BORDER_WIDTH;
	rectToFill.y = rect.y + BORDER_WIDTH;
	rectToFill.w = BORDER_WIDTH;
	rectToFill.h = rect.h - 2 * BORDER_WIDTH;
	SDL_RenderFillRect(renderer, &rectToFill);

	SDL_SetRenderDrawColor(renderer, SELECTION_COLOR);
	for (auto const& p : pixels)
		if (SDL_PointInRect(&p, &newRect))
			SDL_RenderDrawPoint(renderer, p.x, p.y);

	SDL_RenderPresent(renderer);
}

void handleEvent(SDL_Event* e)
{
	if (e->key.keysym.scancode == SDL_SCANCODE_C)
	{
		copyToClipboard(false, rect.x + BORDER_WIDTH, rect.y + BORDER_WIDTH, rect.w - 2 * BORDER_WIDTH, rect.h - 2 * BORDER_WIDTH, true);
		quit = true;
	}

	if (e->key.keysym.scancode == SDL_SCANCODE_ESCAPE)
		quit = true;
	
	int x, y;
	Uint32 state = SDL_GetMouseState(&x, &y);
	SDL_Point p(x, y);
	setCursor(x, y);

	if (e->type == SDL_MOUSEBUTTONUP)
	{
		initX = 0;
		initY = 0;
	}

	if (e->type == SDL_MOUSEBUTTONDOWN && !SDL_PointInRect(&p, &rect))
		pixels.clear();

	if (e->type == SDL_MOUSEMOTION)
	{
		if (state & SDL_BUTTON_LMASK)
		{
			if (!initX && !initY)
			{
				initX = x;
				initY = y;
			}

			bool atV = pointAtVerticalBorder(x);
			bool atH = pointAtHorizontalBorder(y);

			if (atV || atH)
			{
				if (atV && x > rect.x + rect.w - BORDER_WIDTH)
					rect.w += x - prevX; 
				if (atV && x < rect.x + BORDER_WIDTH)
				{
					rect.w += prevX - x; 
					rect.x += x - prevX;
				}
				if (atH && y > rect.y + rect.h - BORDER_WIDTH)
					rect.h += y - prevY;
				if (atH && y < rect.y + BORDER_WIDTH)
				{
					rect.h += prevY - y; 
					rect.y += y - prevY;
				}
				drawRectangle();
			}

			SDL_Rect innerRect = getInnerRect();
			if (SDL_PointInRect(&p, &rect))
			{
				SDL_SetRenderDrawColor(renderer, SELECTION_COLOR);
				for (int i = 0; i < gScreenSurface->w; i++)
				{
					for (int j = 0; j < gScreenSurface->h; j++)
					{
						SDL_Point pp(i, j);
						if ((i - x) * (i - x) + (j - y) * (j - y) <= SELECTION_WIDTH * SELECTION_WIDTH && SDL_PointInRect(&pp, &innerRect))
						{
							SDL_RenderDrawPoint(renderer, i, j);
							SDL_RenderPresent(renderer);
							pixels.push_back(SDL_Point(i, j));
						}
					}
				}
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
			}
			else
			{
				rect.x = initX;
				rect.y = initY;
				rect.w = x > initX ? x - initX : initX - x;
				rect.h = y > initY ? y - initY : initY - y;
				drawRectangle();
			}
		}
		prevX = x;
		prevY = y;
	}
}

int main(int argc, char* args[])
{
	if (init())
	{
		renderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

		SDL_Surface* deskTop = SDL_LoadBMP((filesystem::temp_directory_path().string() + "/Screenshot.bmp").data());
		if (renderer && deskTop)
		{
			text = SDL_CreateTextureFromSurface(renderer, deskTop);
			SDL_RenderCopy(renderer, text, NULL, NULL);

			SDL_SetRenderDrawColor(renderer, DARK_FILL_COLOR);
			SDL_RenderFillRect(renderer, NULL);
			SDL_RenderPresent(renderer);
		}
		else
			quit = true;

		SDL_Event e;
		while (!quit)
		{
			SDL_WaitEventTimeout(NULL, EVENT_TIMEOUT);
			while (SDL_PollEvent(&e))
			{
				if (e.type == SDL_QUIT)
					quit = true;
				handleEvent(&e);
			}
		}
	}

	close();

	return 0;
}