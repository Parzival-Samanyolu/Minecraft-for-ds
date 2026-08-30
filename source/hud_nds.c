#include <nds.h>
#include <string.h>
#include "hud_nds.h"
#include "hud_layout.h"
#include "texture_bin.h"

#define TEX_TILE_BYTES (16 * 16 * 2)
#define BMP_DIM 256

static u16 *s_gfx;

void hudInit(void)
{
	videoSetModeSub(MODE_5_2D);
	vramSetBankC(VRAM_C_SUB_BG);

	int bg = bgInitSub(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
	s_gfx = bgGetGfxPtr(bg);
	memset(s_gfx, 0, (size_t)BMP_DIM * BMP_DIM * sizeof(u16));
}

static void putPixel(int x, int y, u16 color)
{
	if (x < 0 || x >= BMP_DIM || y < 0 || y >= 192)
		return;
	s_gfx[y * BMP_DIM + x] = color;
}

static void fillRect(int x, int y, int w, int h, u16 color)
{
	for (int yy = 0; yy < h; yy++)
		for (int xx = 0; xx < w; xx++)
			putPixel(x + xx, y + yy, color);
}

static void blitIcon(int x, int y, int texIndex)
{
	const u16 *src = (const u16 *)((const u8 *)texture_bin + texIndex * TEX_TILE_BYTES);
	for (int ty = 0; ty < 16; ty++)
		for (int tx = 0; tx < 16; tx++)
			putPixel(x + tx, y + ty, src[ty * 16 + tx] | BIT(15));
}

static void drawSlotFrame(int x, int y, int highlighted)
{
	u16 border = highlighted ? ARGB16(1, 31, 31, 4) : ARGB16(1, 9, 9, 11);
	u16 inner = ARGB16(1, 4, 4, 5);
	fillRect(x, y, HUD_SLOT_SIZE, HUD_SLOT_SIZE, border);
	fillRect(x + 1, y + 1, HUD_SLOT_SIZE - 2, HUD_SLOT_SIZE - 2, inner);
}

void hudRender(const Inventory *inv, int hasTarget, BlockId targetBlock)
{
	for (int i = 0; i < INV_TOTAL_SLOTS; i++) {
		HudRect r = hudSlotPosition(i);
		drawSlotFrame(r.x, r.y, i == inv->selectedHotbar);
		BlockId item = inv->slots[i].id;
		if (item != BLOCK_AIR)
			blitIcon(r.x + 2, r.y + 2, blockFaceTexture(item, 1, 1));
	}

	drawSlotFrame(HUD_TARGET_X, HUD_TARGET_Y, 0);
	if (hasTarget && targetBlock != BLOCK_AIR)
		blitIcon(HUD_TARGET_X + 2, HUD_TARGET_Y + 2, blockFaceTexture(targetBlock, 1, 1));
}
