#include "header.h"

bool init()
{
	bool success = true;
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		success = false;
	else
	{
		gWindow = SDL_CreateWindow("MagicShot", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN | SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_FULLSCREEN_DESKTOP);
		if (gWindow == NULL)
			success = false;
		else
			gScreenSurface = SDL_GetWindowSurface(gWindow);
	}

	return success;
}

void close()
{
	SDL_DestroyRenderer(renderer);
	SDL_FreeSurface(gScreenSurface);
	SDL_Quit();
}

PBITMAPINFO CreateBitmapInfoStruct(HBITMAP hBmp)
{
	BITMAP bmp{};
	PBITMAPINFO pbmi;
	WORD    cClrBits = 32;

	if (!GetObject(hBmp, sizeof(BITMAP), (LPSTR)&bmp))
		return NULL;

	pbmi = (PBITMAPINFO)malloc(sizeof(BITMAPINFOHEADER));
	if (!pbmi)
		return NULL;

	pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pbmi->bmiHeader.biWidth = bmp.bmWidth;
	pbmi->bmiHeader.biHeight = bmp.bmHeight;
	pbmi->bmiHeader.biPlanes = bmp.bmPlanes;
	pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel;
	if (cClrBits < 24)
		pbmi->bmiHeader.biClrUsed = (1 << cClrBits);

	pbmi->bmiHeader.biCompression = BI_RGB;

	pbmi->bmiHeader.biSizeImage = ((pbmi->bmiHeader.biWidth * cClrBits + 31) & ~31) / 8 * pbmi->bmiHeader.biHeight;
	pbmi->bmiHeader.biClrImportant = 0;
	return pbmi;
}

void CreateBMPFile(PBITMAPINFO pbi, HBITMAP hBMP, string path)
{
	if (!pbi)
		return;

	BITMAPFILEHEADER hdr{};
	PBITMAPINFOHEADER pbih;
	LPBYTE lpBits;

	pbih = (PBITMAPINFOHEADER)pbi;
	lpBits = (LPBYTE)malloc(pbih->biSizeImage);
	if (!lpBits)
		return;

	HDC hDC = CreateDC(L"DISPLAY", NULL, NULL, NULL);
	if (!hDC)
		return;

	if (!GetDIBits(hDC, hBMP, 0, (WORD)pbih->biHeight, lpBits, pbi, DIB_RGB_COLORS))
		return;

	hdr.bfType = 0x4d42;        // 0x42 = "B" 0x4d = "M"  
	hdr.bfSize = (DWORD)(sizeof(BITMAPFILEHEADER) +
		pbih->biSize + pbih->biClrUsed
		* sizeof(RGBQUAD) + pbih->biSizeImage);
	hdr.bfReserved1 = 0;
	hdr.bfReserved2 = 0;

	hdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) +
		pbih->biSize + pbih->biClrUsed
		* sizeof(RGBQUAD);

	DWORD total = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + pbih->biClrUsed * sizeof(RGBQUAD) + pbih->biSizeImage;
	BYTE* pbResult = (BYTE*)malloc(total);
	if (!pbResult)
		return;

	memcpy(pbResult, &hdr, sizeof(BITMAPFILEHEADER));
	memcpy(pbResult + sizeof(BITMAPFILEHEADER), pbih, sizeof(BITMAPINFOHEADER) + pbih->biClrUsed * sizeof(RGBQUAD));
	memcpy(pbResult + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + pbih->biClrUsed * sizeof(RGBQUAD), lpBits, pbih->biSizeImage);

	FILE* file = fopen(path.data(), "w+");
	fwrite(pbResult, 1, total, file);
	fclose(file);

	free(pbResult);
	free(pbi);
	free(lpBits);
}

void copyAsFile(HBITMAP hBitmap)
{
	string path = filesystem::temp_directory_path().string() + "/Screenshot.bmp";

	CreateBMPFile(CreateBitmapInfoStruct(hBitmap), hBitmap, path);
	
	int size = sizeof(DROPFILES) + ((lstrlenA(path.data()) + 2));
	HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, size);
	if (!hGlobal)
		return;

	DROPFILES* df = (DROPFILES*)GlobalLock(hGlobal);
	if (!df)
		return;

	ZeroMemory(df, size);
	df->pFiles = sizeof(DROPFILES);
	df->fWide = FALSE;
	LPSTR ptr = (LPSTR)(df + 1);
	lstrcpyA(ptr, path.data());
	GlobalUnlock(hGlobal);
	SetClipboardData(CF_HDROP, hGlobal);
}

void copyToClipboard()
{
	HDC hDC = GetDC(NULL);
	if (!hDC)
		return;

	HDC memDC = CreateCompatibleDC(hDC);
	if (!memDC)
		return;

	HBITMAP bm = CreateCompatibleBitmap(hDC, rect.w - 2 * BORDER_WIDTH, rect.h - 2 * BORDER_WIDTH);
	if (!bm)
		return;

	HGDIOBJ oldbm = SelectObject(memDC, bm);
	if (!oldbm)
		return; 

	if (!BitBlt(memDC, - BORDER_WIDTH - rect.x, - BORDER_WIDTH - rect.y, rect.x + rect.w - BORDER_WIDTH, rect.y + rect.h - BORDER_WIDTH, hDC, 0, 0, SRCCOPY))
		return;
	
	OpenClipboard(NULL);
	EmptyClipboard();
	SetClipboardData(CF_BITMAP, bm);

	copyAsFile(bm);

	CloseClipboard();

	DeleteObject(oldbm);
	DeleteObject(bm);
	ReleaseDC(NULL, memDC);
	ReleaseDC(NULL, hDC);
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

	SDL_SetRenderDrawColor(renderer, BORDER_COLOR);
	SDL_RenderFillRect(renderer, &rect);
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
	SDL_RenderFillRect(renderer, &newRect);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);

	SDL_SetRenderDrawColor(renderer, SELECTION_COLOR);
	for (auto const& p : pixels)
		if (SDL_PointInRect(&p, &newRect))
			SDL_RenderDrawPoint(renderer, p.x, p.y);

	SDL_RenderPresent(renderer);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
}

void handleEvent(SDL_Event* e)
{
	if (e->key.keysym.scancode == SDL_SCANCODE_C)
	{
		copyToClipboard();
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

string getAppDir(char* arg)
{
	return filesystem::path(arg).parent_path().string();
}

int main(int argc, char* args[])
{
	if (init())
	{
		appDir = getAppDir(args[0]); 
		
		SDL_Surface* icon = IMG_Load((appDir + WINDOW_ICON).data());
		if (icon)
			SDL_SetWindowIcon(gWindow, icon);
		
		SDL_SetWindowOpacity(gWindow, WINDOW_OPACITY);
		renderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);

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