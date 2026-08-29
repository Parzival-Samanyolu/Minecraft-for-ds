#include <nds.h>
#include <stdio.h>

#include "texture_bin.h"

static int blockTex;

static void initGraphics(void)
{
	videoSetMode(MODE_0_3D);
	videoSetModeSub(MODE_0_2D);
	vramSetBankA(VRAM_A_TEXTURE);
	vramSetBankC(VRAM_C_SUB_BG);

	consoleDemoInit();

	glInit();
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_ANTIALIAS);

	glClearColor(6, 6, 12, 31);
	glClearPolyID(63);
	glClearDepth(GL_MAX_DEPTH);
	glViewport(0, 0, 255, 191);

	glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(70, 256.0 / 192.0, 0.1f, 40.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glGenTextures(1, &blockTex);
	glBindTexture(0, blockTex);
	glTexImage2D(0, 0, GL_RGB, TEXTURE_SIZE_16, TEXTURE_SIZE_16, 0,
	             GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T, (u8 *)texture_bin);
}

static inline void quad(float shade,
                         float x0, float y0, float z0,
                         float x1, float y1, float z1,
                         float x2, float y2, float z2,
                         float x3, float y3, float z3)
{
	glColor3f(shade, shade, shade);
	glTexCoord2f(0, 1); glVertex3f(x0, y0, z0);
	glTexCoord2f(1, 1); glVertex3f(x1, y1, z1);
	glTexCoord2f(1, 0); glVertex3f(x2, y2, z2);
	glTexCoord2f(0, 0); glVertex3f(x3, y3, z3);
}

/* shades match vanilla MC's fixed per-face brightness (1.0/0.8/0.6/0.5)
 * -- cheap stand-in for real lighting, and it's what block meshing will
 * use anyway once chunks exist, so may as well bake it in now */
static void drawBlockCube(void)
{
	glBindTexture(0, blockTex);
	glBegin(GL_QUADS);

	quad(1.0f, -0.5f, 0.5f, -0.5f,  0.5f, 0.5f, -0.5f,  0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f);   /* top */
	quad(0.5f, -0.5f,-0.5f,  0.5f,  0.5f,-0.5f,  0.5f,  0.5f,-0.5f,-0.5f, -0.5f,-0.5f,-0.5f);   /* bottom */
	quad(0.8f, -0.5f,-0.5f, -0.5f,  0.5f,-0.5f, -0.5f,  0.5f, 0.5f,-0.5f, -0.5f, 0.5f,-0.5f);   /* north */
	quad(0.8f,  0.5f,-0.5f,  0.5f, -0.5f,-0.5f,  0.5f, -0.5f, 0.5f, 0.5f,  0.5f, 0.5f, 0.5f);   /* south */
	quad(0.6f, -0.5f,-0.5f,  0.5f, -0.5f,-0.5f, -0.5f, -0.5f, 0.5f,-0.5f, -0.5f, 0.5f, 0.5f);   /* west */
	quad(0.6f,  0.5f,-0.5f, -0.5f,  0.5f,-0.5f,  0.5f,  0.5f, 0.5f, 0.5f,  0.5f, 0.5f,-0.5f);   /* east */

	glEnd();
}

int main(void)
{
	initGraphics();

	float rotX = 0.0f, rotY = 0.0f;
	int frame = 0;

	while (pmMainLoop())
	{
		scanKeys();
		touchPosition touch;
		touchRead(&touch);

		rotY += 1.0f;
		rotX += 0.4f;

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(0.0, 0.0, 2.2, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
		glRotateX(rotX);
		glRotateY(rotY);

		drawBlockCube();
		glFlush(0);

		if (frame % 15 == 0)
		{
			consoleClear();
			iprintf("Minecraft-DS scaffold\n\n");
			iprintf("frame  %d\n", frame);
			iprintf("rotY   %d deg\n", (int)rotY % 360);
			if (keysHeld() & KEY_TOUCH)
				iprintf("touch  %d,%d\n", touch.px, touch.py);
			else
				iprintf("touch  --\n");
		}

		swiWaitForVBlank();
		frame++;

		if (keysHeld() & KEY_START)
			break;
	}

	return 0;
}
