#include <nds.h>
#include <math.h>
#include <stdio.h>

#include "world.h"
#include "render_nds.h"

#define DEG2RAD 0.0174532925f
#define MOVE_SPEED 0.10f   /* blocks/frame, ~6 blocks/sec at 60fps */
#define TURN_SPEED 2.2f    /* degrees/frame */

typedef struct {
	float x, y, z;
	float yaw, pitch;
} Camera;

static void cameraForward(const Camera *cam, float *fx, float *fy, float *fz)
{
	float yawR = cam->yaw * DEG2RAD;
	float pitchR = cam->pitch * DEG2RAD;
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

	Camera cam = { 8.0f, 22.0f, 8.0f, 0.0f, -15.0f };
	worldUpdate(&world, (int32_t)cam.x, (int32_t)cam.z);

	int frame = 0;
	int lastBuilt = 0;

	while (pmMainLoop())
	{
		scanKeys();
		u16 held = keysHeld();

		if (held & KEY_LEFT)  cam.yaw -= TURN_SPEED;
		if (held & KEY_RIGHT) cam.yaw += TURN_SPEED;
		if (held & KEY_UP)    cam.pitch += TURN_SPEED;
		if (held & KEY_DOWN)  cam.pitch -= TURN_SPEED;
		if (cam.pitch > 89.0f) cam.pitch = 89.0f;
		if (cam.pitch < -89.0f) cam.pitch = -89.0f;

		float fx, fy, fz;
		cameraForward(&cam, &fx, &fy, &fz);
		float rx = -fz, rz = fx; /* right = forward rotated -90 deg about Y */

		if (held & KEY_A) { cam.x += fx * MOVE_SPEED; cam.y += fy * MOVE_SPEED; cam.z += fz * MOVE_SPEED; }
		if (held & KEY_B) { cam.x -= fx * MOVE_SPEED; cam.y -= fy * MOVE_SPEED; cam.z -= fz * MOVE_SPEED; }
		if (held & KEY_X) { cam.y += MOVE_SPEED; }
		if (held & KEY_Y) { cam.y -= MOVE_SPEED; }
		if (held & KEY_L) { cam.x -= rx * MOVE_SPEED; cam.z -= rz * MOVE_SPEED; }
		if (held & KEY_R) { cam.x += rx * MOVE_SPEED; cam.z += rz * MOVE_SPEED; }

		lastBuilt = worldUpdate(&world, (int32_t)cam.x, (int32_t)cam.z);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(cam.x * BLOCK_UNIT, cam.y * BLOCK_UNIT, cam.z * BLOCK_UNIT,
		          (cam.x + fx) * BLOCK_UNIT, (cam.y + fy) * BLOCK_UNIT, (cam.z + fz) * BLOCK_UNIT,
		          0.0, 1.0, 0.0);

		renderWorld(&world);
		glFlush(0);

		if (frame % 15 == 0)
		{
			consoleClear();
			iprintf("Minecraft-DS  chunk streaming\n\n");
			iprintf("pos    %d,%d,%d\n", (int)cam.x, (int)cam.y, (int)cam.z);
			iprintf("yaw/pt %d/%d\n", (int)cam.yaw, (int)cam.pitch);
			iprintf("chunk  %d,%d\n", world.centerCx, world.centerCz);
			iprintf("built  %d last update\n", lastBuilt);
			iprintf("\nD-pad look, A/B fwd/back\nX/Y up/down, L/R strafe\n");
		}

		swiWaitForVBlank();
		frame++;

		if (held & KEY_START)
			break;
	}

	return 0;
}
