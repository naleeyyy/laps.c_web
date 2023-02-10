#include "../../utils/linear_algebra.h"

#define WIDTH 960
#define HEIGHT 720
#define BACKGROUND_COLOR 0xFF181818

static uint32_t pixels[WIDTH * HEIGHT];

float sqrtf(float x);
float atan2f(float y, float x);
float sinf(float x);
float cosf(float x);
float tanf(float x);

#define PI 3.14159265359
float fTheta = 0;

Laps_Canvas render(float dt)
{
    Laps_Canvas c = laps_canvas(pixels, WIDTH, HEIGHT);

    laps_fill(c, BACKGROUND_COLOR);

    cubeMesh3d meshCube;
    float fNear = 0.1f;
    float fFar = 100.0f;
    float fFov = 50.0f;
    float fAspectRatio = ((float)HEIGHT / (float)WIDTH);
    float fFovRad = 1.0f / tanf(fFov * 0.5f / 180.0f * PI);

    triangle3d tris[12] = {

        // SOUTH
        make_triangle3d(make_vec3d(0.0f, 0.0f, 0.0f), make_vec3d(0.0f, 1.0f, 0.0f), make_vec3d(1.0f, 1.0f, 0.0f)),
        make_triangle3d(make_vec3d(0.0f, 0.0f, 0.0f), make_vec3d(1.0f, 1.0f, 0.0f), make_vec3d(1.0f, 0.0f, 0.0f)),

        // EAST
        make_triangle3d(make_vec3d(1.0f, 0.0f, 0.0f), make_vec3d(1.0f, 1.0f, 0.0f), make_vec3d(1.0f, 1.0f, 1.0f)),
        make_triangle3d(make_vec3d(1.0f, 0.0f, 0.0f), make_vec3d(1.0f, 1.0f, 1.0f), make_vec3d(1.0f, 0.0f, 1.0f)),

        // NORTH
        make_triangle3d(make_vec3d(1.0f, 0.0f, 1.0f), make_vec3d(1.0f, 1.0f, 1.0f), make_vec3d(0.0f, 1.0f, 1.0f)),
        make_triangle3d(make_vec3d(1.0f, 0.0f, 1.0f), make_vec3d(0.0f, 1.0f, 1.0f), make_vec3d(0.0f, 0.0f, 1.0f)),

        // WEST
        make_triangle3d(make_vec3d(0.0f, 0.0f, 1.0f), make_vec3d(0.0f, 1.0f, 1.0f), make_vec3d(0.0f, 1.0f, 0.0f)),
        make_triangle3d(make_vec3d(0.0f, 0.0f, 1.0f), make_vec3d(0.0f, 1.0f, 0.0f), make_vec3d(0.0f, 0.0f, 0.0f)),

        // TOP
        make_triangle3d(make_vec3d(0.0f, 1.0f, 0.0f), make_vec3d(0.0f, 1.0f, 1.0f), make_vec3d(1.0f, 1.0f, 1.0f)),
        make_triangle3d(make_vec3d(0.0f, 1.0f, 0.0f), make_vec3d(1.0f, 1.0f, 1.0f), make_vec3d(1.0f, 1.0f, 0.0f)),

        // BOTTOM
        make_triangle3d(make_vec3d(1.0f, 0.0f, 1.0f), make_vec3d(0.0f, 0.0f, 1.0f), make_vec3d(0.0f, 0.0f, 0.0f)),
        make_triangle3d(make_vec3d(1.0f, 0.0f, 1.0f), make_vec3d(0.0f, 0.0f, 0.0f), make_vec3d(1.0f, 0.0f, 0.0f)),
    };

    for (int i = 0; i < 12; ++i)
    {
        meshCube.tris[i] = tris[i];
    }

    mat4x4 matRotZ = init_mat4x4();
    mat4x4 matRotX = init_mat4x4();
    fTheta += 1.0f * dt;

    // Rotation Z
    matRotZ.m[0][0] = cosf(fTheta);
    matRotZ.m[0][1] = sinf(fTheta);
    matRotZ.m[1][0] = -sinf(fTheta);
    matRotZ.m[1][1] = cosf(fTheta);
    matRotZ.m[2][2] = 1;
    matRotZ.m[3][3] = 1;

    // Rotation X
    matRotX.m[0][0] = 1;
    matRotX.m[1][1] = cosf(fTheta * 0.5f);
    matRotX.m[1][2] = sinf(fTheta * 0.5f);
    matRotX.m[2][1] = -sinf(fTheta * 0.5f);
    matRotX.m[2][2] = cosf(fTheta * 0.5f);
    matRotX.m[3][3] = 1;

    mat4x4 matProj = init_mat4x4();

    matProj.m[0][0] = fAspectRatio * fFovRad;
    matProj.m[1][1] = fFovRad;
    matProj.m[2][2] = fFar / (fFar - fNear);
    matProj.m[3][2] = (-fFar * fNear) / (fFar - fNear);
    matProj.m[2][3] = 1.0f;
    matProj.m[3][3] = 0.0f;

    for (int i = 0; i < 12; ++i)
    {
        triangle3d triProjected, triTranslated, triRotatedZ, triRotatedZX;

        // Rotate in Z-Axis
        MultiplyMatrixVector(&meshCube.tris[i].p[0], &triRotatedZ.p[0], &matRotZ);
        MultiplyMatrixVector(&meshCube.tris[i].p[1], &triRotatedZ.p[1], &matRotZ);
        MultiplyMatrixVector(&meshCube.tris[i].p[2], &triRotatedZ.p[2], &matRotZ);

        // Rotate in X-Axis
        MultiplyMatrixVector(&triRotatedZ.p[0], &triRotatedZX.p[0], &matRotX);
        MultiplyMatrixVector(&triRotatedZ.p[1], &triRotatedZX.p[1], &matRotX);
        MultiplyMatrixVector(&triRotatedZ.p[2], &triRotatedZX.p[2], &matRotX);

        // Offset into the screen
        triTranslated = make_triangle3d(triRotatedZX.p[0], triRotatedZX.p[1], triRotatedZX.p[2]);
        triTranslated.p[0].z = triRotatedZX.p[0].z + 3.0f;
        triTranslated.p[1].z = triRotatedZX.p[1].z + 3.0f;
        triTranslated.p[2].z = triRotatedZX.p[2].z + 3.0f;

        // Project triangles from 3D --> 2D
        MultiplyMatrixVector(&triTranslated.p[0], &triProjected.p[0], &matProj);
        MultiplyMatrixVector(&triTranslated.p[1], &triProjected.p[1], &matProj);
        MultiplyMatrixVector(&triTranslated.p[2], &triProjected.p[2], &matProj);

        // Scale into view
        triProjected.p[0].x += 1.0f;
        triProjected.p[0].y += 1.0f;
        triProjected.p[1].x += 1.0f;
        triProjected.p[1].y += 1.0f;
        triProjected.p[2].x += 1.0f;
        triProjected.p[2].y += 1.0f;
        triProjected.p[0].x *= 0.5f * (float)WIDTH;
        triProjected.p[0].y *= 0.5f * (float)HEIGHT;
        triProjected.p[1].x *= 0.5f * (float)WIDTH;
        triProjected.p[1].y *= 0.5f * (float)HEIGHT;
        triProjected.p[2].x *= 0.5f * (float)WIDTH;
        triProjected.p[2].y *= 0.5f * (float)HEIGHT;

        // Rasterize triangle
        laps_hollow_triangle(c, (uint16_t)triProjected.p[0].x, (uint16_t)triProjected.p[0].y,
                             (uint16_t)triProjected.p[1].x, (uint16_t)triProjected.p[1].y,
                             (uint16_t)triProjected.p[2].x, (uint16_t)triProjected.p[2].y,
                             0xFFFFFFFF);
    }
    return c;
}
