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

void copyToClipboard()
{
	HDC hDC = GetDC(NULL);
	if (!hDC)
		return;

	HDC memDC = CreateCompatibleDC(hDC);
	if (!memDC)
		return;

	HBITMAP bm = CreateCompatibleBitmap(hDC, rect.w, rect.h);
	if (!bm)
		return;

	HGDIOBJ oldbm = SelectObject(memDC, bm);
	if (!oldbm)
		return; 

	if (!BitBlt(memDC, -rect.x, -rect.y, rect.x + rect.w, rect.y + rect.h, hDC, 0, 0, SRCCOPY))
		return;
	
	OpenClipboard(NULL);
	EmptyClipboard();
	SetClipboardData(CF_BITMAP, bm);
	CloseClipboard();

	DeleteObject(oldbm);
	DeleteObject(bm);
	ReleaseDC(NULL, memDC);
	ReleaseDC(NULL, hDC);
}

void handleEvent(SDL_Event* e)
{
	if (e->type == SDL_MOUSEBUTTONUP)
	{
		initX = 0;
		initY = 0;
	}

	if (e->type == SDL_MOUSEMOTION)
	{
		SDL_FillRect(gScreenSurface, NULL, SDL_MapRGB(gScreenSurface->format, 0, 0, 0));
		int x, y;
		Uint32 state = SDL_GetMouseState(&x, &y);
		if (state & SDL_BUTTON_LMASK)
		{
			if (!initX && !initY)
			{
				initX = x;
				initY = y;
			}

			SDL_Point p(x, y);
			if (SDL_PointInRect(&p, &rect))
			{
				SDL_SetRenderDrawColor(renderer, SELECTION_COLOR);
				for (int i = 0; i < gScreenSurface->w; i++)
				{
					for (int j = 0; j < gScreenSurface->h; j++)
					{
						if ((i - x) * (i - x) + (j - y) * (j - y) <= SELECTION_WIDTH * SELECTION_WIDTH)
						{
							SDL_RenderDrawPoint(renderer, i, j);
						}
					}
				}
				SDL_RenderPresent(renderer); 
			}
			else
			{
				rect.x = initX;
				rect.y = initY;
				rect.w = x > initX ? x - initX : initX - x;
				rect.h = y > initY ? y - initY : initY - y;

				SDL_Rect newRect = rect;
				newRect.x = rect.x + BORDER_WIDTH;
				newRect.w = rect.w - 2 * BORDER_WIDTH;
				newRect.y = rect.y + BORDER_WIDTH;
				newRect.h = rect.h - 2 * BORDER_WIDTH;

				SDL_RenderClear(renderer);

				SDL_SetRenderDrawColor(renderer, BORDER_COLOR);
				SDL_RenderFillRect(renderer, &rect);
				SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
				SDL_RenderFillRect(renderer, &newRect);
				SDL_RenderPresent(renderer);
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			}
		}
	}

	if (e->key.keysym.scancode == SDL_SCANCODE_C && (e->key.keysym.mod == KMOD_LCTRL || e->key.keysym.mod == KMOD_RCTRL))
	{
		copyToClipboard();
		quit = true;
	}

	if (e->key.keysym.scancode == SDL_SCANCODE_ESCAPE)
		quit = true;
}

int main(int argc, char* args[])
{
	if (init())
	{
		SDL_SetWindowOpacity(gWindow, 0.3);
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