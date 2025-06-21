// ---------------------------------------------------------------------------
// Minion Bob (geometry-only) demo – no texture mapping, freeglut + OpenGL fixed-func
// ---------------------------------------------------------------------------

#include <glut.h>
#include <GL/glu.h>
#include <cmath>
#include <iostream>
#include <initializer_list>
#ifndef M_PI                        
#define M_PI 3.14159265358979323846
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

GLuint beachTexture;
GLuint leftEyeTexture, rightEyeTexture;
GLuint denimTexture;

// 텍스처 로딩 함수
void loadBackgroundTexture() {
    int width, height, channels;

    //이미지 수직 뒤집기 설정
    stbi_set_flip_vertically_on_load(true);

    std::string path = "../beach.jpg";
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
    if (!data) {
        std::cerr << "Failed to load beach.jpg\n";
        std::cerr << "Reason: " << stbi_failure_reason() << std::endl;  //실패 이유 출력
        return;
    }

    glGenTextures(1, &beachTexture);
    glBindTexture(GL_TEXTURE_2D, beachTexture);

    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0,
        format, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
}

void loadTextureFromFile(const char* path, GLuint& textureID) {
    int width, height, channels;

    stbi_set_flip_vertically_on_load(true);  // Y축 반전

    unsigned char* data = stbi_load(path, &width, &height, &channels, 0);
    if (!data) {
        std::cerr << "Failed to load image: " << path << std::endl;
        std::cerr << "Reason: " << stbi_failure_reason() << std::endl;
        return;
    }

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0,
        format, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
}

void loadEyeTextures() {
    loadTextureFromFile("../leftEye.png", leftEyeTexture);
    loadTextureFromFile("../rightEye.png", rightEyeTexture);
}

void loadDenimTexture() {
    loadTextureFromFile("../denim.png", denimTexture);
}

// 2D 배경 이미지 그리기
void drawBackground() {
    glDisable(GL_LIGHTING);   // ✨ 조명 끄기

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 1, 0, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, beachTexture);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, 0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 1.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 1.0f);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_LIGHTING);   // ✨ 조명 다시 켜기
}

// 초기화
void init() {
    glEnable(GL_DEPTH_TEST);
    loadBackgroundTexture();
    loadEyeTextures();
    loadDenimTexture();
}


// ────────────────────────────────────────────────
// 전역 상태 변수 (회전/줌/마우스)
// ────────────────────────────────────────────────
static float rotateX = 0.f, rotateY = 0.f;
static float zoom = 1.0f;
static int   lastX = 0, lastY = 0;
static bool  mouseDown = false;


// ── 인사 애니메이션 설정 ----------------------------------
const int kRaise = 14;   // 40 × ⅓ ≈ 13.3 → 14
const int kWave = 40;   // 120 × ⅓ = 40
const int kLower = 14;   // 40 × ⅓ ≈ 13.3 → 14
const int kTotal = kRaise + kWave + kLower;

bool  waving = false;
int   waveStep = 0;        // 0‥kTotal-1


// ------------------------------------------------------------
// lighting setup
// ------------------------------------------------------------
void setupLighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    GLfloat pos[] = { 2,  2,  2, 1 };
    GLfloat amb[] = { 0.3f, 0.3f, 0.3f, 1 };  // ✨ 강도 1/3
    GLfloat diff[] = { 0.6f, 0.6f, 0.6f, 1 }; // ✨ 강도 1/3
    //GLfloat spec[] = { 0.4f, 0.4f, 0.4f, 1.0f };  // ✨ 스페큘러 반사 추가
    glLightfv(GL_LIGHT0, GL_POSITION, pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diff);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    //glMaterialfv(GL_FRONT, GL_SPECULAR, spec);     // ✨ 재질에 스페큘러 추가
    //glMaterialf(GL_FRONT, GL_SHININESS, 30.0f);    // ✨ 반짝임 정도 조절
}


// ────────────────────────────────────────────────
// 애니메이션 타이머 : 30 ms 간격, 200스텝 후 종료(2회)
// ────────────────────────────────────────────────
void timer(int) {
    if (!waving) return;
    if (++waveStep >= kTotal) {          // 한 싸이클 끝
        waving = false;
        waveStep = 0;
    }
    else {
        glutTimerFunc(30, timer, 0);     // 30 ms 간격
    }
    glutPostRedisplay();
}


// ────────────────────────────────────────────────
// 키보드 : a -> 줌인, z -> 줌아웃, r -> 인사, q -> 종료
// ────────────────────────────────────────────────
void keyboard(unsigned char k, int, int) {
    if (k == 'a' || k == 'A') zoom *= 1.1f; if (k == 'z' || k == 'Z') zoom *= 0.9f;
    if (k == 'r' || k == 'R' && !waving) { waving = true; waveStep = 0; glutTimerFunc(0, timer, 0); }
    if (k == 'q' || k == 'Q') {
        exit(0);  // q를 누르면 프로그램 종료
    }
    glutPostRedisplay();
}


// ------------------------------------------------------------
// basic shapes
// ------------------------------------------------------------
void drawCylinder(GLUquadric* q, float r, float h, int sl = 32) {
    gluCylinder(q, r, r, h, sl, 1);
}
void drawDisk(GLUquadric* q, float r, int sl = 32) {
    gluDisk(q, 0.0, r, sl, 1);
}

//-------------------------------------------------------------
// parameters
//-------------------------------------------------------------
const float bodyRadius = 0.5f;
const float bodyHeight = 0.8f;
// pants silhouette radius on X,Z axes
const float pantsRadius = bodyRadius * 1.12f;
// waist cylinder height (around 5% of body)
const float waistHeight = bodyHeight * 0.05f;
// bib face height (we'll scale it by bibScale)
const float bibHeight = bodyHeight * 0.45f;
// bib scale factor (2/3 size)
const float bibScale = 2.0f / 3.0f;
// bib wrap angle (100° scaled by bibScale)
const float bibArcDeg = 100.0f * bibScale;
// strap diameter
const float strapDia = 0.06f;

//-------------------------------------------------------------
// curved bib: 앞/뒤에 붙는 얇은 패널
// 멜빵바지 앞 뒷면
//-------------------------------------------------------------
void drawCurvedBib(float yBase, float zSign) {
    const int slices = 24;
    // inner / outer radius (얇은 두께)
    const float innerR = pantsRadius - 0.06f;
    const float outerR = innerR + 0.04f;
    // 실제 bib 높이 (scaled)
    const float h = bibHeight * bibScale;

    glPushMatrix();
    // bib 중앙을 몸통에 맞춰 이동
    glTranslatef(0, yBase + h / 2.0f, 0);
    // 앞/뒤로 돌리기
    glRotatef(zSign > 0 ? 90.0f : -90.0f, 0, 1, 0);

    // 텍스처 설정
    glDisable(GL_LIGHTING);       // 조명 꺼서 반사 없음
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, denimTexture);
    glColor3f(1.0f, 1.0f, 1.0f);

    // arcDeg 따라 약간만 감싸는 호(가로 길이 축소)
    const float step = bibArcDeg / slices;
    for (int i = 0; i < slices; ++i) {
        float t0 = float(i) / slices;
        float t1 = float(i + 1) / slices;

        float a0 = -bibArcDeg / 2.0f + t0 * bibArcDeg;
        float a1 = -bibArcDeg / 2.0f + t1 * bibArcDeg;

        float c0 = cosf(a0 * M_PI / 180.0f), s0 = sinf(a0 * M_PI / 180.0f);
        float c1 = cosf(a1 * M_PI / 180.0f), s1 = sinf(a1 * M_PI / 180.0f);

        // inner wall
        glBegin(GL_QUADS);
        glTexCoord2f(t0, 0.0f); glVertex3f(innerR * c0, -h / 2, innerR * s0);
        glTexCoord2f(t0, 1.0f); glVertex3f(innerR * c0, h / 2, innerR * s0);
        glTexCoord2f(t1, 1.0f); glVertex3f(innerR * c1, h / 2, innerR * s1);
        glTexCoord2f(t1, 0.0f); glVertex3f(innerR * c1, -h / 2, innerR * s1);
        glEnd();

        // outer wall
        glBegin(GL_QUADS);
        glTexCoord2f(t1, 0.0f); glVertex3f(outerR * c1, -h / 2, outerR * s1);
        glTexCoord2f(t1, 1.0f); glVertex3f(outerR * c1, h / 2, outerR * s1);
        glTexCoord2f(t0, 1.0f); glVertex3f(outerR * c0, h / 2, outerR * s0);
        glTexCoord2f(t0, 0.0f); glVertex3f(outerR * c0, -h / 2, outerR * s0);
        glEnd();

        // top face
        glBegin(GL_QUADS);
        glTexCoord2f(t0, 0.0f); glVertex3f(innerR * c0, h / 2, innerR * s0);
        glTexCoord2f(t0, 1.0f); glVertex3f(outerR * c0, h / 2, outerR * s0);
        glTexCoord2f(t1, 1.0f); glVertex3f(outerR * c1, h / 2, outerR * s1);
        glTexCoord2f(t1, 0.0f); glVertex3f(innerR * c1, h / 2, innerR * s1);
        glEnd();

        // bottom face
        glBegin(GL_QUADS);
        glTexCoord2f(t1, 0.0f); glVertex3f(innerR * c1, -h / 2, innerR * s1);
        glTexCoord2f(t1, 1.0f); glVertex3f(outerR * c1, -h / 2, outerR * s1);
        glTexCoord2f(t0, 1.0f); glVertex3f(outerR * c0, -h / 2, outerR * s0);
        glTexCoord2f(t0, 0.0f); glVertex3f(innerR * c0, -h / 2, innerR * s0);
        glEnd();

        // left face
        glBegin(GL_QUADS);
        glTexCoord2f(t0, 0.0f); glVertex3f(innerR * c0, -h / 2, innerR * s0);
        glTexCoord2f(t0, 1.0f); glVertex3f(innerR * c0, h / 2, innerR * s0);
        glTexCoord2f(t0, 1.0f); glVertex3f(outerR * c0, h / 2, outerR * s0);
        glTexCoord2f(t0, 0.0f); glVertex3f(outerR * c0, -h / 2, outerR * s0);
        glEnd();

        // right face
        glBegin(GL_QUADS);
        glTexCoord2f(t1, 0.0f); glVertex3f(outerR * c1, -h / 2, outerR * s1);
        glTexCoord2f(t1, 1.0f); glVertex3f(outerR * c1, h / 2, outerR * s1);
        glTexCoord2f(t1, 1.0f); glVertex3f(innerR * c1, h / 2, innerR * s1);
        glTexCoord2f(t1, 0.0f); glVertex3f(innerR * c1, -h / 2, innerR * s1);
        glEnd();

    }
    glDisable(GL_TEXTURE_2D);  // 텍스처 끄기
    glEnable(GL_LIGHTING);        // 원래대로 조명 다시 켜줌
    glPopMatrix();
}

// strap parameters ― 몸치수와 연동해서 한 곳에서 조정
const float shoulderRise = bodyHeight * 0.28; // 옆구리에서 ↑ 얼마나 올라갈지

//------------------------------------------------------------------
// shoulder strap: bib front edge → over shoulder → bib back edge
//  yFront : bib 상단 높이
//  left   : 왼쪽(true) / 오른쪽(false)
//------------------------------------------------------------------
void drawShoulderStrap(float yFront, bool left)
{
    const int   slices = 32;
    const float R = pantsRadius;  // your 0.5*1.12
    const float d = strapDia;
    const float rise = shoulderRise;
    const float halfBib = bibArcDeg * 0.5f;

    // pick start/end so it just wraps back on the same side
    float a0Deg, a1Deg;
    if (left) {
        a0Deg = 80.0f + halfBib;   // left-front
        a1Deg = 285.0f - halfBib;   // left-back
    }
    else {
        a0Deg = 100.0f - halfBib;   // right-front
        a1Deg = -105.0f + halfBib;   // right-back
    }

    float arc = a1Deg - a0Deg;
    float step = arc / slices;

    glPushMatrix();
    // scale X,Z so the strap sits right on the curved overalls
    glScalef(1.0f, 1.0f, 1.0f);
    glColor3f(0.05f, 0.10f, 0.60f);

    // 텍스처 설정
    glDisable(GL_LIGHTING);       // 조명 꺼서 반사 없음
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, denimTexture);
    glColor3f(1.0f, 1.0f, 1.0f);

    for (int i = 0; i < slices; ++i) {
        float t0 = float(i) / slices;
        float t1 = float(i + 1) / slices;
        float ang0 = (a0Deg + arc * t0) * M_PI / 180.0f;
        float ang1 = (a0Deg + arc * t1) * M_PI / 180.0f;

        float y0 = yFront + rise * sinf(t0 * M_PI);
        float y1 = yFront + rise * sinf(t1 * M_PI);

        float c0 = cosf(ang0), s0 = sinf(ang0);
        float c1 = cosf(ang1), s1 = sinf(ang1);

        float ix0 = (R - d * 0.5f) * c0, iz0 = (R - d * 0.5f) * s0;
        float ox0 = (R + d * 0.5f) * c0, oz0 = (R + d * 0.5f) * s0;
        float ix1 = (R - d * 0.5f) * c1, iz1 = (R - d * 0.5f) * s1;
        float ox1 = (R + d * 0.5f) * c1, oz1 = (R + d * 0.5f) * s1;

        glBegin(GL_QUADS);

        // inner face
        glTexCoord2f(t0, 0.0f); glVertex3f(ix0, y0, iz0);
        glTexCoord2f(t0, 1.0f); glVertex3f(ix0, y0 - d, iz0);
        glTexCoord2f(t1, 1.0f); glVertex3f(ix1, y1 - d, iz1);
        glTexCoord2f(t1, 0.0f); glVertex3f(ix1, y1, iz1);

        // outer face
        glTexCoord2f(t1, 0.0f); glVertex3f(ox1, y1, oz1);
        glTexCoord2f(t1, 1.0f); glVertex3f(ox1, y1 - d, oz1);
        glTexCoord2f(t0, 1.0f); glVertex3f(ox0, y0 - d, oz0);
        glTexCoord2f(t0, 0.0f); glVertex3f(ox0, y0, oz0);

        // top edge
        glTexCoord2f(t0, 0.0f); glVertex3f(ix0, y0, iz0);
        glTexCoord2f(t0, 1.0f); glVertex3f(ox0, y0, oz0);
        glTexCoord2f(t1, 1.0f); glVertex3f(ox1, y1, oz1);
        glTexCoord2f(t1, 0.0f); glVertex3f(ix1, y1, iz1);

        // bottom edge
        glTexCoord2f(t1, 0.0f); glVertex3f(ix1, y1 - d, iz1);
        glTexCoord2f(t1, 1.0f); glVertex3f(ox1, y1 - d, oz1);
        glTexCoord2f(t0, 1.0f); glVertex3f(ox0, y0 - d, oz0);
        glTexCoord2f(t0, 0.0f); glVertex3f(ix0, y0 - d, iz0);

        // front face (i == 0)
        glTexCoord2f(t0, 0.0f); glVertex3f(ix0, y0, iz0);
        glTexCoord2f(t0, 1.0f); glVertex3f(ox0, y0, oz0);
        glTexCoord2f(t0, 1.0f); glVertex3f(ox0, y0 - d, oz0);
        glTexCoord2f(t0, 0.0f); glVertex3f(ix0, y0 - d, iz0);

        // back face (i == slices - 1)
        glTexCoord2f(t1, 0.0f); glVertex3f(ix1, y1, iz1);
        glTexCoord2f(t1, 1.0f); glVertex3f(ox1, y1, oz1);
        glTexCoord2f(t1, 1.0f); glVertex3f(ox1, y1 - d, oz1);
        glTexCoord2f(t1, 0.0f); glVertex3f(ix1, y1 - d, iz1);

        glEnd();
    }
    glDisable(GL_TEXTURE_2D);  // 텍스처 끄기
    glEnable(GL_LIGHTING);        // 원래대로 조명 다시 켜줌
    glPopMatrix();
}

//-------------------------------------------------------------
// drawBody: 몸체 + overalls
//-------------------------------------------------------------
void drawBody() {
    GLUquadric* quad = gluNewQuadric();

    // --- Yellow body ---
    //glDisable(GL_LIGHTING);
    glColor3f(1.0f, 0.72f, 0.0f);
    // 몸통 원기둥 + bottom disk
    glPushMatrix();
    glTranslatef(0, -bodyHeight / 2, 0);
    glRotatef(-90, 1, 0, 0);
    drawCylinder(quad, bodyRadius, bodyHeight);
    drawDisk(quad, bodyRadius);
    glPopMatrix();
    // 머리쪽 구
    glPushMatrix();
    glTranslatef(0, bodyHeight / 2, 0);
    glScalef(1, 0.7f, 1);
    glutSolidSphere(bodyRadius, 32, 32);
    glPopMatrix();
    // 하단 납작 구
    glPushMatrix();
    glTranslatef(0, -bodyHeight / 2, 0);
    glScalef(1, 0.32f, 1);
    glutSolidSphere(bodyRadius, 32, 32);
    glPopMatrix();

    // --- Blue overalls silhouette ---
    glDisable(GL_LIGHTING);       // 조명 꺼서 반사 없음
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, denimTexture);
    glColor3f(1.0f, 1.0f, 1.0f);
    gluQuadricTexture(quad, GL_TRUE);

    glPushMatrix();
    glTranslatef(0, -bodyHeight / 2, 0);
    glScalef(1.1f, 0.35f, 1.1f);
    gluSphere(quad, bodyRadius, 32, 32);  // gluSphere로 교체
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);        // 원래대로 조명 다시 켜줌

    // bib panels
    float yBibBottom = -bodyHeight / 2 + waistHeight;
    float yBibTop = yBibBottom + bibHeight * bibScale;

    drawCurvedBib(yBibBottom, 1.0f);   // front
    drawCurvedBib(yBibBottom, -1.0f);  // back

    // NEW shoulder straps
    drawShoulderStrap(yBibTop, true);   // 왼쪽
    drawShoulderStrap(yBibTop, false);  // 오른쪽

    //glEnable(GL_LIGHTING);
    gluDeleteQuadric(quad);
}

void drawFace() {
    GLUquadric* quad = gluNewQuadric();

    // -----------------------------
    // 1. 안경띠 (머리에 밀착된 Band)
    // -----------------------------
    glPushMatrix();
    glColor3f(0.05f, 0.05f, 0.05f);
    // 위치: 머리 윗부분 약간 아래
    glTranslatef(0, 0.46f * bodyHeight, 0);
    // 반지름 기준으로 정렬되도록 X축 회전
    glRotatef(90, 1, 0, 0);
    // 중심 반지름은 머리 둘레 기준, 두께는 얇게
    glutSolidTorus(0.015f, bodyRadius, 32, 64);
    // 머리띠처럼 수직 방향만 눌러줌 → 납작한 도넛
    glScalef(1.0f, 1.0f, 0.2f);  // Z축을 0.2배로 압축
    glPopMatrix();

    // -----------------------------
    // 1-2. 아래쪽 안경띠 (겹쳐서 붙이기)
    // -----------------------------
    glPushMatrix();
    glColor3f(0.05f, 0.05f, 0.05f);
    // 살짝 더 아래로 내림 (0.42 정도)
    glTranslatef(0, 0.43f * bodyHeight, 0);
    glRotatef(90, 1, 0, 0);
    glutSolidTorus(0.015f, bodyRadius, 32, 64);
    glScalef(1.0f, 1.0f, 0.5f);  // Z축을 0.5배로 압축
    glPopMatrix();


    // --------------------------------
    // 2. 안경테 (속이 빈 원통형 프레임)
    // --------------------------------
    for (float ex : {-0.165f, 0.175f}) {
        float outerR = 0.19f;
        float innerR = 0.15f;
        float depth = 0.2f;
        glPushMatrix();
        glTranslatef(ex, bodyHeight * 0.47f, bodyRadius * 1.09f - depth);
        glRotatef(90, 0, 0, 1);
        glColor3f(0.8f, 0.8f, 0.8f);

        gluCylinder(quad, outerR, outerR, depth, 32, 1);

        glPushMatrix();
        glTranslatef(0, 0, 0);
        gluDisk(quad, innerR, outerR, 32, 1);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(0, 0, depth);
        gluDisk(quad, innerR, outerR, 32, 1);
        glPopMatrix();

        glPopMatrix();
    }

    // -------------------------
    // 3. 눈 (흰자 + 홍채)
    // -------------------------
    gluQuadricTexture(quad, GL_TRUE);

    int i = 0;
    for (float ex : {-0.165f, 0.175f}) {
        glPushMatrix();
        glTranslatef(ex, bodyHeight * 0.47f, bodyRadius * 1.04f);

        // 흰자
        glPushMatrix();
        glScalef(1.0f, 1.0f, 0.01f);
        glColor3f(1.0f, 1.0f, 1.0f);
        glutSolidSphere(0.155f, 24, 24);
        glPopMatrix();

        // 홍채 텍스처 선택
        glEnable(GL_TEXTURE_2D);
        if (i == 0)
            glBindTexture(GL_TEXTURE_2D, leftEyeTexture);   // 왼쪽 눈
        else
            glBindTexture(GL_TEXTURE_2D, rightEyeTexture);  // 오른쪽 눈
        glColor3f(1.0f, 1.0f, 1.0f);

        glTranslatef(0, 0, 0.0016f);
        gluDisk(quad, 0.0f, 0.06f, 32, 1);

        glDisable(GL_TEXTURE_2D);
        glPopMatrix();

        i++;  // 왼→오른 판단용
    }

    gluDeleteQuadric(quad);
}

void drawMouth() {
    const int segments = 32;
    const float innerRadius = 0.10f;
    const float outerRadius = 0.11f;
    const float mouthDepth = 0.09f;
    const float centerX = 0.0f;
    const float centerY = 0.13f * bodyHeight;
    const float centerZ = bodyRadius + 0.0001f;

    // --- 입 안쪽 (빨강)
    glPushMatrix();
    glTranslatef(centerX, centerY, centerZ);
    glScalef(1.6f, 1.0f, 1.0f);
    glColor3f(0.4f, 0.0f, 0.0f);
    glBegin(GL_POLYGON);
    for (int i = 0; i <= segments; ++i) {
        float angle = M_PI + (M_PI * i / segments);
        float x = innerRadius * cosf(angle);
        float y = innerRadius * sinf(angle);
        glVertex3f(x, y, 0);
    }
    glEnd();
    glPopMatrix();

    // --- 입 외곽 노란 띠 (속 비어있고 위쪽 덮개 있음)
    GLUquadric* quad = gluNewQuadric();
    //glDisable(GL_LIGHTING); // 노란 띠 조명 영향 제거
    glPushMatrix();
    glTranslatef(centerX, centerY, centerZ - mouthDepth / 1.005f);
    glScalef(1.6f, 1.0f, 1.0f);
    glColor3f(1.0f, 0.72f, 0.0f);

    for (int i = 0; i < segments; ++i) {
        float theta1 = M_PI + (M_PI * i / segments);
        float theta2 = M_PI + (M_PI * (i + 1) / segments);

        float x1o = outerRadius * cosf(theta1), y1o = outerRadius * sinf(theta1);
        float x2o = outerRadius * cosf(theta2), y2o = outerRadius * sinf(theta2);
        float x1i = innerRadius * cosf(theta1), y1i = innerRadius * sinf(theta1);
        float x2i = innerRadius * cosf(theta2), y2i = innerRadius * sinf(theta2);

        // 겉벽
        glBegin(GL_QUADS);
        glVertex3f(x1o, y1o, 0);
        glVertex3f(x1o, y1o, mouthDepth);
        glVertex3f(x2o, y2o, mouthDepth);
        glVertex3f(x2o, y2o, 0);
        glEnd();

        // 속벽
        glBegin(GL_QUADS);
        glVertex3f(x2i, y2i, 0);
        glVertex3f(x2i, y2i, mouthDepth);
        glVertex3f(x1i, y1i, mouthDepth);
        glVertex3f(x1i, y1i, 0);
        glEnd();

        // 위쪽 덮개
        glBegin(GL_QUADS);
        glVertex3f(x1i, y1i, 0);
        glVertex3f(x1o, y1o, 0);
        glVertex3f(x2o, y2o, 0);
        glVertex3f(x2i, y2i, 0);
        glEnd();

        // 아래쪽 내부 (뒷면)
        glBegin(GL_QUADS);
        glVertex3f(x2i, y2i, mouthDepth);
        glVertex3f(x2o, y2o, mouthDepth);
        glVertex3f(x1o, y1o, mouthDepth);
        glVertex3f(x1i, y1i, mouthDepth);
        glEnd();
    }
    glPopMatrix();

    // 3. 위쪽 입 주변 띠 (평평한 직사각형 형태)
    glPushMatrix();
    glTranslatef(centerX, centerY, centerZ - mouthDepth / 1.005f);  // 살짝 위로
    glScalef(1.6f, 0.2f, 1.0f);  // 가로 길이 넓게, 세로 얇게
    glColor3f(1.0f, 0.72f, 0.0f); // 몸 색깔

    glBegin(GL_QUADS);
    // 앞면
    glVertex3f(-outerRadius, 0, 0);
    glVertex3f(outerRadius, 0, 0);
    glVertex3f(outerRadius, 0.03f, 0);
    glVertex3f(-outerRadius, 0.03f, 0);

    // 뒷면
    glVertex3f(-outerRadius, 0, mouthDepth);
    glVertex3f(outerRadius, 0, mouthDepth);
    glVertex3f(outerRadius, 0.03f, mouthDepth);
    glVertex3f(-outerRadius, 0.03f, mouthDepth);

    // 위면
    glVertex3f(-outerRadius, 0.03f, 0);
    glVertex3f(outerRadius, 0.03f, 0);
    glVertex3f(outerRadius, 0.03f, mouthDepth);
    glVertex3f(-outerRadius, 0.03f, mouthDepth);

    // 아래면
    glVertex3f(-outerRadius, 0, 0);
    glVertex3f(outerRadius, 0, 0);
    glVertex3f(outerRadius, 0, mouthDepth);
    glVertex3f(-outerRadius, 0, mouthDepth);

    // 좌우면
    glVertex3f(-outerRadius, 0, 0);
    glVertex3f(-outerRadius, 0.03f, 0);
    glVertex3f(-outerRadius, 0.03f, mouthDepth);
    glVertex3f(-outerRadius, 0, mouthDepth);

    glVertex3f(outerRadius, 0, 0);
    glVertex3f(outerRadius, 0.03f, 0);
    glVertex3f(outerRadius, 0.03f, mouthDepth);
    glVertex3f(outerRadius, 0, mouthDepth);
    glEnd();
    glPopMatrix();

    // --- 윗니 4개 (하단 모서리만 곡선)
    // --- 윗니 하단 둥근 부분 6개 (넓고 아래로 내려감)
    glPushMatrix();
    glTranslatef(centerX, centerY + 0.001f, centerZ + 0.001f);  // 위치 아래로 조정
    //glDisable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 1.0f);  // 흰색 이빨 하단 곡선

    float arcSpacing = 0.041f;  // 이빨 간격 유지
    float arcWidth = 0.045f;    // 좌우 길이 증가
    float arcHeight = 0.012f;   // 상하 높이 그대로 유지
    int arcSegments = 14;

    for (int i = 0; i < 6; ++i) {
        float cx = (i - 2.5f) * arcSpacing;  // 중심 위치

        glPushMatrix();
        glTranslatef(cx, 0.0f, 0.0f);
        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(0.0f, 0.0f, 0.0f);  // 중심점
        for (int j = 0; j <= arcSegments; ++j) {
            float theta = M_PI + (M_PI * j / arcSegments);
            float dx = arcWidth * 0.5f * cosf(theta);
            float dy = arcHeight * sinf(theta);
            glVertex3f(dx, dy, 0.0f);
        }
        glEnd();
        glPopMatrix();
    }
    glPopMatrix();

    // --- 아랫니 4개 (입 안쪽 곡면 기준, 위쪽 향하도록 그리기)
    glPushMatrix();
    //glDisable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 1.0f);  // 흰색

    const int lowerToothCount = 4;
    const float lowerArcWidth = 0.04f;
    const float lowerArcHeight = 0.008f;
    const float lowerSpacing = 0.042f;
    const int lowerArcSegments = 16;

    float baseY = centerY - 0.1f;  // 입 안쪽 아래 기준으로 약간 위로 조정
    float baseZ = centerZ + 0.0001f;  // 입 표면에 약간 튀어나오게

    for (int i = 0; i < lowerToothCount; ++i) {
        float cx = (i - (lowerToothCount - 1) / 2.0f) * lowerSpacing;

        // 양 끝 이빨만 작게
        float width = lowerArcWidth;
        float height = lowerArcHeight;
        if (i == 0 || i == lowerToothCount - 1) {
            width *= 0.5f;
            height *= 1.1f;
        }

        glPushMatrix();
        glTranslatef(cx, baseY, baseZ);
        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(0.0f, 0.0f, 0.0f);
        for (int j = 0; j <= lowerArcSegments; ++j) {
            float theta = 0 + (M_PI * j / lowerArcSegments);
            float dx = width * 0.5f * cosf(theta);
            float dy = height * sinf(theta);
            glVertex3f(dx, dy, 0.0f);
        }
        glEnd();
        glPopMatrix();
    }

    glPopMatrix();

    //glEnable(GL_LIGHTING); // 조명 다시 켜주기
    gluDeleteQuadric(quad);
}

// ────────────────────────────────────────────────
// 수정된 drawGlove: isLeft에 따라 엄지·검지-중지 사이·약지-새끼 사이 손가락 3개 배치
// ────────────────────────────────────────────────
void drawGlove(bool isLeft = true) {
    glPushMatrix();
    glColor3f(0.05f, 0.05f, 0.05f);

    // 1) 손 전체 구 (손바닥 부분)
    glPushMatrix();
    // 기존에 손 구를 Z축 앞으로 살짝 띄웠던 위치로 이동
    glTranslatef(0, 0.0f, 0.055f);
    if (isLeft) glRotatef(-30.f, 0, 0, 1);
    else glRotatef(30.f, 0, 0, 1);
    // 기존의 스케일과 구 그리기
    glScalef(1.2f, 0.8f, 1.2f);
    glutSolidSphere(0.07f, 16, 16);
    glPopMatrix();

    // 2) 손목 밴드 (도넛형 입체감)
    glPushMatrix();
    glTranslatef(0, 0.0f, -0.01f);
    glutSolidTorus(0.022f, 0.045f, 16, 32);
    glPopMatrix();

    glPopMatrix();
}

// ────────────────────────────────────────────────
// drawArmSegment: 팔/팔뚝(원기둥) + 양끝 원판 + 조건부로 drawGlove 호출
// ────────────────────────────────────────────────
void drawArmSegment(float length, float radius, bool drawGloves = false, bool isLeft = true) {
    GLUquadric* quad = gluNewQuadric();
    gluCylinder(quad, radius, radius, length, 20, 1);
    // 아래쪽 원판 (어깨/팔꿈치 쪽)
    glPushMatrix();
    glRotatef(180, 1, 0, 0);
    drawDisk(quad, radius);
    glPopMatrix();
    // 위쪽 원판 (팔꿈치/손목 쪽)
    glPushMatrix();
    glTranslatef(0, 0, length);
    drawDisk(quad, radius);
    // isLeft에 따라 drawGlove 호출
    if (drawGloves) drawGlove(isLeft);
    glPopMatrix();
    gluDeleteQuadric(quad);
}

// ---------------------------------------------------------------------------
// 인사 동작 구현함수
// ---------------------------------------------------------------------------
void drawArmWithJoint(float bx, float by, float bz, bool isLeft)
{
    const float armLen = 0.23f, foreLen = 0.20f, R = 0.045f;

    // ── 기본 각도 (몸 옆에 붙어 있음) ─────────────────────────
    float shX = 60.0f;
    float shY = isLeft ? 18.0f : -18.0f;
    float shZ = isLeft ? 80.0f : -80.0f;
    float elbow = -75.0f, wristZ = isLeft ? -85.0f : 85.0f, wristX = -60.0f;

    // ── 올린 자세 목표값 ────────────────────────────────────
    const float shX_DN = 60.0f, shX_UP = 90.0f;
    const float shY_DN = (isLeft ? 18.0f : -18.0f),  // 시작 Y
        shY_UP = -115.0f;                     // 끝나는 Y
    const float shZ_DN = -80.0f, shZ_UP = 70.0f; // 앞서 70→130으로 수정
    const float elb_DN = -75.0f, elb_UP = -45.0f;
    const float wristZ_DN = 85.0f, wristZ_UP = 90.0f;
    const float wristX_DN = -60.0f, wristX_UP = 0.0f;

    if (!isLeft && waving) {
        if (waveStep < kRaise) {           // ① 올리기 (0→1)
            float t = waveStep / float(kRaise);
            shX = shX_DN + (shX_UP - shX_DN) * t;
            shY = shY_DN + (shY_UP - shY_DN) * t;    
            shZ = shZ_DN + (shZ_UP - shZ_DN) * t;
            elbow = elb_DN + (elb_UP - elb_DN) * t;
            wristZ = wristZ_DN + (wristZ_UP - wristZ_DN) * t;
            wristX = wristX_DN + (wristX_UP - wristX_DN) * t;
        }
        else if (waveStep < kRaise + kWave) { // ② 흔들기
            float t = (waveStep - kRaise) / float(kWave);
            float ph = sinf(t * 4 * M_PI);
            shX = shX_UP + 16.0f * ph;
            shY = shY_UP;                       
            shZ = shZ_UP;
            elbow = elb_UP + 40.0f * ph;
            wristZ = wristZ_UP;
            wristX = wristX_UP;
        }
        else {                              // ③ 내리기 (0→1)
            float t = (waveStep - kRaise - kWave) / float(kLower);
            shX = shX_UP + (shX_DN - shX_UP) * t;
            shY = shY_UP + (shY_DN - shY_UP) * t;  
            shZ = shZ_UP + (shZ_DN - shZ_UP) * t;
            elbow = elb_UP + (elb_DN - elb_UP) * t;
            wristZ = wristZ_UP + (wristZ_DN - wristZ_UP) * t;
            wristX = wristX_UP + (wristX_DN - wristX_UP) * t;
        }
    }

    // ── 실제 그리기(기존 코드) ───────────────────────────────
    glPushMatrix(); glTranslatef(bx, by, bz);
    glRotatef(shX, 1, 0, 0); glRotatef(shY, 0, 1, 0); glRotatef(shZ, 0, 0, 1);
    glColor3f(1, 0.72f, 0);  glutSolidSphere(R, 20, 20);
    drawArmSegment(armLen, R, false, isLeft);

    glTranslatef(0, 0, armLen);  glutSolidSphere(R, 20, 20);
    glRotatef(elbow, 1, 0, 0);   glRotatef(wristZ, 0, 0, 1); glRotatef(wristX, 1, 0, 0);
    drawArmSegment(foreLen, R, true, isLeft);
    glPopMatrix();
}



void drawTaperedLeg(float height, float topRadius, float bottomRadius, int slices = 32) {
    GLUquadric* quad = gluNewQuadric();
    gluQuadricTexture(quad, GL_TRUE);  // 텍스처 좌표 자동 생성

    glPushMatrix();
    glRotatef(90, 1, 0, 0);  

    // 원기둥 (Z축 → Y축으로 회전됨)
    gluCylinder(quad, topRadius, bottomRadius, height, slices, 1);

    // 상단 캡
    gluDisk(quad, 0.0, topRadius, slices, 1);

    // 하단 캡
    glTranslatef(0, 0, height);
    gluDisk(quad, 0.0, bottomRadius, slices, 1);

    glPopMatrix();

    gluDeleteQuadric(quad);
}

void drawHeel() {
    GLUquadric* quad = gluNewQuadric();

    // 상단 구형 부분 (볼륨 있는 뒤꿈치)
    glPushMatrix();
    glColor3f(0.05f, 0.05f, 0.05f);  // 검정색
    glRotatef(90, 1, 0, 0);
    glScalef(1.0f, 1.0f, 0.4f);    

    glutSolidSphere(0.09f, 20, 20);
    glPopMatrix();

    // 하단 원기둥 부분 (발바닥 쪽)
    glPushMatrix();
    glRotatef(90, 1, 0, 0);       
    gluCylinder(quad, 0.075f, 0.085f, 0.08f, 20, 1);
    glPopMatrix();

    // 하단 밑창
    glPushMatrix();
    glColor3f(0.05f, 0.05f, 0.05f);  // 검정색
    glRotatef(90, 1, 0, 0);
    glTranslatef(0.0f, 0.0f, 0.08f); // 앞으로 내밀기
    glScalef(1.0f, 1.0f, 0.2f);      // Z축 찌그러뜨림

    glutSolidSphere(0.085f, 20, 20);
    glPopMatrix();

    gluDeleteQuadric(quad);
}

void drawToeCap(float angle) {
    // 앞코 (납작한 구)
    glPushMatrix();
    glRotatef(90, 1, 0, 0);
    glRotatef(angle, 0, 0, 1);
    glColor3f(0.05f, 0.05f, 0.05f);  // 검정색
    glTranslatef(0.0f, 0.12f, 0.04f); // 앞으로 내밀기
    glScalef(1.0f, 1.0f, 0.6f);      // 납작한 앞코 형태
    glutSolidSphere(0.09f, 20, 20);
    glPopMatrix();
}



void drawLeg(float x, float y, float z, bool isLeft) {
    float toecapAngle = isLeft ? -18.0f : 16.0f;
    glPushMatrix();
    glTranslatef(x, y, z);

    // 텍스처 적용 시작
    glDisable(GL_LIGHTING);       // 조명 꺼서 반사 없음
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, denimTexture);
    glColor3f(1.0f, 1.0f, 1.0f);

    // 텍스처가 입혀진 다리
    drawTaperedLeg(0.18f, 0.1f, 0.08f);

    glDisable(GL_TEXTURE_2D);  // 텍스처 영향 제거
    glEnable(GL_LIGHTING);        // 원래대로 조명 다시 켜줌


    // 신발 추가
    glTranslatef(0, -0.18f, 0.0f);  // 다리 아래로 살짝
    // 신발 뒷코
    drawHeel();
    // 신발 앞코
    drawToeCap(toecapAngle);

    glPopMatrix();
}




void drawLimbs() {
    // 왼팔
    drawArmWithJoint(+0.49f, -0.05f, 0.0f, true);
    // 오른팔
    drawArmWithJoint(-0.49f, -0.05f, 0.0f, false);
    // 두 다리
    drawLeg(+0.1f, -0.5f, 0.0f, true);
    drawLeg(-0.1f, -0.5f, 0.0f, false);
}

//-------------------------------------------------------------
// GLUT callbacks
//-------------------------------------------------------------
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawBackground();

    glLoadIdentity();
    gluLookAt(0, 0, 4.f, 0, 0, 0, 0, 1, 0);
    glScalef(zoom, zoom, zoom);

    // 회전 중심점을 모델 위치에 맞게 아래로 이동
    glTranslatef(0.0f, -0.7f, 0.0f);  // 기준점 내림
    glRotatef(rotateY, 1, 0, 0);
    glRotatef(rotateX, 0, 1, 0);

    drawBody();
    drawFace();
    drawMouth();
    drawLimbs();

    glutSwapBuffers();
}


void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, (double)w / h, 0.1, 100);
    glMatrixMode(GL_MODELVIEW);
}
void mouse(int b, int s, int x, int y) {
    if (b == GLUT_LEFT_BUTTON) { mouseDown = (s == GLUT_DOWN); lastX = x; lastY = y; }
    if (b == 3) zoom *= 1.1f; if (b == 4) zoom *= 0.9f;
    glutPostRedisplay();
}
void motion(int x, int y) {
    if (!mouseDown) return;
    rotateX += (x - lastX);
    rotateY += (y - lastY);
    lastX = x;
    lastY = y;
    glutPostRedisplay();
}
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Minions Bob");
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.2f, 0.2f, 0.2f, 1);
    setupLighting();
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutMainLoop();
    return 0;
}
