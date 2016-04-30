#include <DxLib.h>

double px = 0, py = 0, ppx = 0, ppy = 0;
double pvx = 0, pvy = 0;
const double pra = 0.5, pfa = 0.2, pjv = 8;
const int pjcMax = 20;
bool pg = false;
int pjc = 0;
const double PvxMax = 6, PvyMax = 8;
const double GravitationalAcceleration = 0.6;

int graphic, playerGraphic;
int key = 0, pkey = 0, tkey = 0;

const int ChipWidth = 32, ChipHeight = 32;
const int MapWidth = 640 / ChipWidth, MapHeight = 480 / ChipHeight;
const int pcw = 24, pch = 30, pcx = (ChipWidth - pcw) / 2, pcy = ChipHeight - pch;
int map[MapWidth][MapHeight];

const int ScreenWidth = 640, ScreenHeight = 480;

void loadMap(const char *file)
{
	int h = FileRead_open(file);

	for (int y = 0; y < MapHeight; ++y) {
		for (int x = 0; x < MapWidth; ++x) {
			map[x][y] = FileRead_getc(h) == '@' ? 1 : 0;
		}
		FileRead_getc(h);
		FileRead_getc(h);
	}

	FileRead_close(h);
}

bool CollidesParallelogramLine(
	double x1, double y1, double x2, double y2, double pw, double lx, double ly, double lw)
{
	return y1 <= ly && ly <= y2 &&
		(y2-y1)*(lx-x1-pw) < (x2-x1)*(ly-y1) && (x2-x1)*(ly-y1) <= (y2-y1)*(lx-x1+lw);
}

bool f(double x, double y, double px, double py, double )
{
	double x1, x2, y1, y2;
	double hw = pcw, bw = ChipWidth, hh = pch, bh = ChipHeight;

	if(y > py) {
		x1 = px; y1 = py; x2 = x; y2 = y;
	} else {
		x1 = x; y1 = y; x2 = px; y2 = py;
	}
	int mx = static_cast<int>(x/ChipWidth), my = static_cast<int>(y/ChipHeight);
	double bx = mx * ChipWidth, by = my * ChipHeight;
	pg = false;
	for (int i = 0; i < 3; ++i) {
		if (map[mx][my] == 1 && CollidesParallelogramLine(x1, y1, x2, y2, hw, bx, by, bw)) {
			return true;
		}
		++mx;
		bx += ChipWidth;
	}
	return false;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// 画面モードのセット
	SetGraphMode(ScreenWidth, ScreenHeight, 32);
	ChangeWindowMode(TRUE);

	if (DxLib_Init() == -1) return -1;

	// 描画先画面を裏画面にセット
	SetDrawScreen(DX_SCREEN_BACK);

	loadMap("map.txt");

	graphic = LoadGraph("graphic.png");
	playerGraphic = DerivationGraph(0, 0, 32, 32, graphic);

	// ループ
	while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0) {
		ClearDrawScreen();
		pkey = key;
		key = GetJoypadInputState(DX_INPUT_KEY_PAD1);
		tkey = ~pkey & key;

		ppx = px; ppy = py;
		pvy += GravitationalAcceleration;
		if (pvy > PvyMax) pvy = PvyMax;

		if(pg && tkey & PAD_INPUT_1) {
			pvy = -pjv;
			pjc = pjcMax;
		} else if(pjc > 0 && key & PAD_INPUT_1) {
			pvy -= 0.4;
			--pjc;
		} else if(pjc > 0) {
			pjc = 0;
		}

		if (key & PAD_INPUT_LEFT) {
			pvx -= pra;
			if (pvx < -PvxMax) pvx = -PvxMax;
		}
		if (key & PAD_INPUT_RIGHT) {
			pvx += pra;
			if (pvx > PvxMax) pvx = PvxMax;
		}

		if (pvx < -pfa) pvx += pfa;
		else if (pvx <= pfa) pvx = 0;
		else pvx -= pfa;

		px += pvx; py += pvy;
		if (py > ScreenHeight - ChipHeight) py = ScreenHeight - ChipHeight;
		
		{
			double hx = px+pcx, hy = py+pcy, phx = ppx+pcx, phy = ppy+pcy,
				hw = pcw, bw = ChipWidth, hh = pch, bh = ChipHeight;
			if(hy > phy) {
				double x1 = phx, y1 = phy+hh, x2 = hx, y2 = hy+hh;
				int mx = static_cast<int>(x1/ChipWidth), my = static_cast<int>(y2/ChipHeight);
				double bx = mx * ChipWidth, by = my * ChipHeight;
				pg = false;
				for (int i = 0; i < 3; ++i) {
					if(map[mx][my] == 1 && CollidesParallelogramLine(x1, y1, x2, y2, hw, bx, by, bw)) {
						hy = by - hh;
						pg = true;
						break;
					}
					++mx;
					bx += ChipWidth;
				}
			} else if (hy < phy) {
				double x1 = hx, y1 = hy, x2 = phx, y2 = phy;
				int mx = static_cast<int>(x1/ChipWidth), my = static_cast<int>(y1/ChipHeight);
				double bx = mx * ChipWidth, by = (my+1) * ChipHeight;
				for (int i = 0; i < 3; ++i) {
					if (map[mx][my] == 1 && CollidesParallelogramLine(x1, y1, x2, y2, hw, bx, by, bw)) {
						hy = by;
						break;
					}
					++mx;
					bx += ChipWidth;
				}
			}

			if (hx > phx) {
				double x1 = phx+hw, y1 = phy, x2 = hx+hw, y2 = hy;
				int mx = static_cast<int>(x2/ChipWidth), my = static_cast<int>(y1/ChipHeight);
				double bx = mx * ChipWidth, by = my * ChipHeight;
				for (int i = 0; i < 3; ++i) {
					if (map[mx][my] == 1 && CollidesParallelogramLine(y1, x1, y2, x2, hh, by, bx, bh)) {
						hx = bx - hw;
						break;
					}
					++my;
					by += ChipHeight;
				}
			} else if (hx < phx) {
				double x1 = hx, y1 = hy, x2 = phx, y2 = phy;
				int mx = static_cast<int>(x1/ChipWidth), my = static_cast<int>(y1/ChipHeight);
				double bx = (mx+1) * ChipWidth, by = my * ChipHeight;
				for (int i = 0; i < 3; ++i) {
					if (map[mx][my] == 1 && CollidesParallelogramLine(y1, x1, y2, x2, hh, by, bx, bh)) {
						hx = bx;
						break;
					}
					++my;
					by += ChipHeight;
				}
			}

			if (hy != py + pcy) {
				py = hy - pcy; pvy = 0;
			}
			if(hx != px + pcx) {
				px = hx - pcx; pvx = 0;
			}

		}


		if(py >= ScreenHeight - ChipHeight) {
			px = ppx = 0; py = ppy = 0;
			pvx = 0; pvy = 0;
		}

		for (int x = 0; x < MapWidth; x++) {
			for (int y = 0; y < MapHeight; y++) {
				unsigned c = map[x][y] == 1 ? GetColor(192, 128, 48) : GetColor(128, 128, 255);
				DrawBox(x * ChipWidth, y * ChipHeight, (x+1) * ChipWidth - 1, (y+1) * ChipHeight - 1, c, TRUE);
			}
		}

		DrawGraph(static_cast<int>(px), static_cast<int>(py), playerGraphic, TRUE);
		DrawBox(static_cast<int>(px+pcx), static_cast<int>(py+pcy),
			static_cast<int>(px+pcx+pcw), static_cast<int>(py+pcy+pch),
			GetColor(0, 192, 0), FALSE);

		ScreenFlip();
	}

	DxLib_End();

	return 0;
}