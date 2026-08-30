#include <nds.h>
#include <math.h>

#include "world.h"
#include "player.h"
#include "raycast.h"
#include "render_nds.h"
#include "inventory.h"
#include "craft.h"
#include "hud_layout.h"
#include "hud_nds.h"
#include "daynight.h"

#define DEG2RAD 0.0174532925f
#define TURN_SPEED 2.2f    /* degrees/frame */
#define REACH 5.0f         /* blocks, break/place range */
#define DT (1.0f / 60.0f)

static void cameraForward(float yaw, float pitch, float *fx, float *fy, float *fz)
{
	float yawR = yaw * DEG2RAD;
	float pitchR = pitch * DEG2RAD;
	*fx = sinf(yawR) * cosf(pitchR);
	*fy = sinf(pitchR);
	*fz = -cosf(yawR) * cosf(pitchR);
}

static void initGraphics(void)
{
	videoSetMode(MODE_0_3D);

	glInit();
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_ANTIALIAS);

	glClearPolyID(63);
	glClearDepth(GL_MAX_DEPTH);
	glViewport(0, 0, 255, 191);

	glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(70, 256.0 / 192.0, 0.05f, 40.0f);

	renderInit();
	hudInit();
}

int main(void)
{
	initGraphics();

	World world;
	worldInit(&world, 1337);

	Player pl;
	playerInit(&pl, 8.5f, 30.0f, 8.5f);
	worldUpdate(&world, (int32_t)pl.x, (int32_t)pl.z);

	Inventory inv;
	inventoryInit(&inv);

	CraftGrid grid;
	craftGridInit(&grid);
	int craftMode = 0;

	DayNight dn;
	dayNightInit(&dn);

	while (pmMainLoop())
	{
		scanKeys();
		u16 held = keysHeld();
		u16 down = keysDown();

		if (held & KEY_LEFT)  pl.yaw -= TURN_SPEED;
		if (held & KEY_RIGHT) pl.yaw += TURN_SPEED;
		if (held & KEY_UP)    pl.pitch += TURN_SPEED;
		if (held & KEY_DOWN)  pl.pitch -= TURN_SPEED;
		if (pl.pitch > 89.0f) pl.pitch = 89.0f;
		if (pl.pitch < -89.0f) pl.pitch = -89.0f;

		float fx, fy, fz;
		cameraForward(pl.yaw, pl.pitch, &fx, &fy, &fz);
		float rx = -fz, rz = fx;

		float moveX = 0.0f, moveZ = 0.0f;
		if (held & KEY_A) { moveX += fx; moveZ += fz; }
		if (held & KEY_B) { moveX -= fx; moveZ -= fz; }
		if (held & KEY_R) { moveX += rx; moveZ += rz; }
		if (held & KEY_L) { moveX -= rx; moveZ -= rz; }
		moveX *= PLAYER_WALK_SPEED * DT;
		moveZ *= PLAYER_WALK_SPEED * DT;

		playerStep(&pl, &world, moveX, moveZ, held & KEY_X, DT);
		worldUpdate(&world, (int32_t)pl.x, (int32_t)pl.z);

		float eyeX = pl.x, eyeY = pl.y + PLAYER_EYE_HEIGHT, eyeZ = pl.z;
		RaycastHit target = raycastBlocks(&world, eyeX, eyeY, eyeZ, fx, fy, fz, REACH);

		if ((down & KEY_Y) && target.hit) {
			BlockId broken = worldGetBlock(&world, target.bx, target.by, target.bz);
			worldSetBlock(&world, target.bx, target.by, target.bz, BLOCK_AIR);
			inventoryAddItem(&inv, broken, 1);
		}
		if ((down & KEY_SELECT) && target.hit) {
			BlockId toPlace = inventorySelectedBlock(&inv);
			if (toPlace != BLOCK_AIR) {
				worldSetBlock(&world, target.px, target.py, target.pz, toPlace);
				inventoryTakeSelected(&inv);
			}
		}

		if (down & KEY_TOUCH) {
			touchPosition touch;
			touchRead(&touch);
			HudHit hit = hudHitTest(touch.px, touch.py);

			switch (hit.kind) {
			case HUD_HIT_INV_SLOT:
				if (craftMode)
					craftPlaceFromInventory(&grid, &inv, hit.index);
				else if (hit.index < INV_HOTBAR_SLOTS)
					inv.selectedHotbar = hit.index;
				else
					inventorySwap(&inv, inv.selectedHotbar, hit.index);
				break;
			case HUD_HIT_CRAFT_CELL:
				craftReturnCell(&grid, &inv, hit.index);
				break;
			case HUD_HIT_CRAFT_OUTPUT:
				craftCollect(&grid, &inv);
				break;
			case HUD_HIT_CRAFT_TOGGLE:
				craftMode = !craftMode;
				break;
			default:
				break;
			}
		}

		dayNightStep(&dn, DT);
		int skyR, skyG, skyB;
		dayNightSkyColor(&dn, &skyR, &skyG, &skyB);
		glClearColor(skyR, skyG, skyB, 31);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(eyeX * BLOCK_UNIT, eyeY * BLOCK_UNIT, eyeZ * BLOCK_UNIT,
		          (eyeX + fx) * BLOCK_UNIT, (eyeY + fy) * BLOCK_UNIT, (eyeZ + fz) * BLOCK_UNIT,
		          0.0, 1.0, 0.0);

		renderWorld(&world, dayNightLightLevel(&dn));
		glFlush(0);

		hudRender(&inv, &grid, craftMode, target.hit,
		          target.hit ? worldGetBlock(&world, target.bx, target.by, target.bz) : BLOCK_AIR);

		swiWaitForVBlank();

		if (held & KEY_START)
			break;
	}

	return 0;
}
