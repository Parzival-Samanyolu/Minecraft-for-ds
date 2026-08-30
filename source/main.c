#include <nds.h>
#include <math.h>
#include <stdio.h>

#include "world.h"
#include "player.h"
#include "raycast.h"
#include "render_nds.h"

#define DEG2RAD 0.0174532925f
#define TURN_SPEED 2.2f    /* degrees/frame */
#define REACH 5.0f         /* blocks, break/place range */
#define DT (1.0f / 60.0f)

static const char *blockName(BlockId b)
{
	switch (b) {
	case BLOCK_AIR: return "air";
	case BLOCK_GRASS: return "grass";
	case BLOCK_DIRT: return "dirt";
	case BLOCK_STONE: return "stone";
	case BLOCK_SAND: return "sand";
	case BLOCK_WATER: return "water";
	case BLOCK_WOOD: return "wood";
	case BLOCK_LEAVES: return "leaves";
	case BLOCK_COAL_ORE: return "coal ore";
	default: return "?";
	}
}

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
	videoSetModeSub(MODE_0_2D);
	vramSetBankC(VRAM_C_SUB_BG);

	consoleDemoInit();

	glInit();
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_ANTIALIAS);

	glClearColor(10, 14, 20, 31);
	glClearPolyID(63);
	glClearDepth(GL_MAX_DEPTH);
	glViewport(0, 0, 255, 191);

	glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(70, 256.0 / 192.0, 0.05f, 40.0f);

	renderInit();
}

int main(void)
{
	initGraphics();

	World world;
	worldInit(&world, 1337);

	Player pl;
	playerInit(&pl, 8.5f, 30.0f, 8.5f);
	worldUpdate(&world, (int32_t)pl.x, (int32_t)pl.z);

	int frame = 0;
	int lastBuilt = 0;

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
		lastBuilt = worldUpdate(&world, (int32_t)pl.x, (int32_t)pl.z);

		float eyeX = pl.x, eyeY = pl.y + PLAYER_EYE_HEIGHT, eyeZ = pl.z;
		RaycastHit target = raycastBlocks(&world, eyeX, eyeY, eyeZ, fx, fy, fz, REACH);

		if ((down & KEY_Y) && target.hit)
			worldSetBlock(&world, target.bx, target.by, target.bz, BLOCK_AIR);
		if ((down & KEY_SELECT) && target.hit)
			worldSetBlock(&world, target.px, target.py, target.pz, BLOCK_STONE);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(eyeX * BLOCK_UNIT, eyeY * BLOCK_UNIT, eyeZ * BLOCK_UNIT,
		          (eyeX + fx) * BLOCK_UNIT, (eyeY + fy) * BLOCK_UNIT, (eyeZ + fz) * BLOCK_UNIT,
		          0.0, 1.0, 0.0);

		renderWorld(&world);
		glFlush(0);

		if (frame % 15 == 0)
		{
			consoleClear();
			iprintf("Minecraft-DS  player+blocks\n\n");
			iprintf("pos    %d,%d,%d %s\n", (int)pl.x, (int)pl.y, (int)pl.z,
			        pl.onGround ? "(ground)" : "(air)");
			iprintf("chunk  %d,%d  built %d\n", world.centerCx, world.centerCz, lastBuilt);
			if (target.hit)
				iprintf("target %d,%d,%d %s\n", (int)target.bx, (int)target.by,
				        (int)target.bz, blockName(worldGetBlock(&world, target.bx, target.by, target.bz)));
			else
				iprintf("target none\n");
			iprintf("\nD-pad look, A/B fwd/back\nL/R strafe, X jump\nY break, SELECT place\n");
		}

		swiWaitForVBlank();
		frame++;

		if (held & KEY_START)
			break;
	}

	return 0;
}
