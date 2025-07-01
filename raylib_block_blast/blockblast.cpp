#define _CRT_SECURE_NO_WARNINGS
#include "raylib.h"
#include <string.h>
#include <stdio.h>

// Global tan�mlamalar ve yap� tan�mlar� burada
#define MAX_SIZE 4
#define GRID_SIZE 8
#define CELL_SIZE 40
#define PIECE_COUNT 25
#define HIGH_SCORE_FILE "highscore.txt"

// Ses dosyas�
Sound explosionSound;
Sound gameOverSound;
Sound placedsound;
bool isAnimating = false;
int animationFrames = 0;
const int animationDuration = 30; // Patlama animasyonu s�resi (30 frame)

bool explosionGrid[GRID_SIZE][GRID_SIZE] = { false }; // Patlama animasyonunun yap�laca�� h�creler
bool gameOverSoundPlayed = false; // Oyun bitti�inde sesin bir kez �al�n�p �al�nmad���n� kontrol eder

int LoadHighScore();
void SaveHighScore(int score);
int highScore = 0;

typedef struct {
    int sizeX, sizeY;
    int shape[MAX_SIZE][MAX_SIZE];
    Color color;
    bool used; // Par�an�n kullan�l�p kullan�lmad���n� belirten bayrak
} Piece;

typedef struct {
    int pieceIndex;
    Color color;
} GridCell;

typedef enum GameScreen { LOGO = 0, TITLE, GAMEPLAY, ENDING } GameScreen;

Color SARI = { 250,235,135,255 };
Color YESIL = { 147,209,82,255 };
Color MAVI = { 73,217,227,255 };
Color MOR = { 190,142,245,255 };
Color PEMBE = { 250,147,243,255 };
Color KIRMIZI = { 250,147,176,255 };
Color TURUNCU = { 255,179,112,255 };
Color LIGHT = { 227,228,230,255 };

// Par�alar� tan�mlayal�m
Piece pieces[PIECE_COUNT] = {
    {2, 3, {{1, 0 }, {1, 0 }, {1, 1}}, SARI, false},     // Sar� L
    {2, 3, {{0, 1}, {0, 1}, {1, 1}}, YESIL, false},      // Ye�il L
    {3, 2, {{0, 1, 0}, {1, 1, 1}}, MAVI, false},         // Mavi T
    {3, 2, {{1, 1, 1}, {0, 1, 0}}, MOR, false},          // Mor T
    {2, 2, {{1, 1}, {1, 1}}, PEMBE, false},              // Pembe Kare
    {3, 1, {{1, 1, 1}}, KIRMIZI, false},                 // K�rm�z� �ubuk
    {4, 1, {{1, 1, 1, 1}}, SARI, false},                 // Sar� �ubuk
    {3, 2, {{1, 1, 1}, {1, 0, 0}},YESIL, false},         // Ye�il J
    {3, 2, {{1, 1, 1}, {0, 0, 1}}, MAVI, false},         // Mavi J Ters
    {3, 2, {{1, 1, 0}, {0, 1, 1}}, MOR, false},          // Mor Z
    {3, 2, {{0, 1, 1}, {1, 1, 0}}, PEMBE, false},        // Pembe S
    {2, 3, {{0, 1}, {1, 1}, {0, 1}}, KIRMIZI, false},    // K�rm�z� T Ters
    {1, 3, {{1}, {1}, {1}}, SARI, false},                // Sar� �ubuk
    {1, 4, {{1}, {1}, {1}, {1}}, YESIL, false},          // Ye�il �ubuk
    {1, 2, {{1}, {1}}, MAVI, false},                     // Mavi �ubuk
    {2, 1, {{1, 1}}, MOR, false},                        // Mor �ift
    {2, 2, {{1, 0}, {1, 1}}, PEMBE, false},              // Pembe K��e
    {3, 2, {{1, 0, 0}, {1, 1, 1}}, KIRMIZI, false},      // K�rm�z� L
    {2, 3, {{1, 1}, {1, 0}, {1, 0}}, SARI, false},       // Sar� Ters L
    {2, 2, {{1, 1}, {1, 0}}, YESIL, false},              // Ye�il K��e
    {2, 3, {{1, 1}, {0, 1}, {0, 1}}, MAVI, false},       // Mavi Ters L
    {2, 2, {{1, 1}, {0, 1}}, MOR, false},                // Mor K��e
    {2, 2, {{0, 1}, {1, 1}}, PEMBE, false},              // Pembe K��e
    {3, 2, {{0, 0, 1}, {1, 1, 1}}, KIRMIZI, false},      // K�rm�z� Ters L
    {2, 3, {{1, 0}, {1, 1}, {1, 0}}, TURUNCU , false}    // Turuncu Ters T
};

// Fonksiyon prototipleri
void DrawGameGrid(GridCell grid[GRID_SIZE][GRID_SIZE], int offsetX, int offsetY);
void DrawPiece(Piece p, int posX, int posY, int cellSize, bool isDragging);
void DrawPiecePool(Piece pool[3], int offsetX, int offsetY, int draggedPiece);
void InitializePiecePool(Piece pool[3]);
void UpdatePiecePool(Piece pool[3], int* piecesUsedCount);
void ClearFullLines(GridCell grid[GRID_SIZE][GRID_SIZE], int* score);
void PlacePieceIfPossible(GridCell grid[GRID_SIZE][GRID_SIZE], Piece piecePool[3], int* piecesUsedCount, int selectedPieceIndex, int mouseX, int mouseY, int* score);
bool CanPlacePiece(GridCell grid[GRID_SIZE][GRID_SIZE], Piece* p, int startX, int startY);
bool IsGameOver(GridCell grid[GRID_SIZE][GRID_SIZE], Piece piecePool[3]);
bool DrawButton(int posX, int posY, int width, int height, const char* text);
void FillRandomCells(GridCell grid[GRID_SIZE][GRID_SIZE], int count, Color color);
void DrawExplosion(int x, int y);

// grid h�crelerini �izer
void DrawGameGrid(GridCell grid[GRID_SIZE][GRID_SIZE], int offsetX, int offsetY) {
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            Color color = (grid[i][j].pieceIndex == 0) ? LIGHT : grid[i][j].color;
            DrawRectangle(offsetX + j * CELL_SIZE, offsetY + i * CELL_SIZE, CELL_SIZE - 2, CELL_SIZE - 2, color);
        }
    }
}

//tek bir par�ay� verilen konumda ve boyda �iziyor
void DrawPiece(Piece p, int posX, int posY, int cellSize, bool isDragging) {
    if (!p.used) {
        for (int i = 0; i < p.sizeY; i++) {
            for (int j = 0; j < p.sizeX; j++) {
                if (p.shape[i][j] == 1) {
                    DrawRectangle(posX + j * cellSize, posY + i * cellSize, cellSize - 2, cellSize - 2, isDragging ? Fade(p.color, 0.5f) : p.color);
                }
            }
        }
    }
}

//3 adet kullan�lacak par�a �izer
void DrawPiecePool(Piece pool[3], int offsetX, int offsetY, int draggedPiece) {
    for (int i = 0; i < 3; i++) {
        if (i != draggedPiece) {
            DrawPiece(pool[i], offsetX, offsetY + i * (MAX_SIZE * CELL_SIZE + 10), CELL_SIZE, false);
        }
    }
}

//rastgele par�a se�er
void InitializePiecePool(Piece pool[3]) {
    for (int i = 0; i < 3; i++) {
        int randIndex = GetRandomValue(0, PIECE_COUNT - 1);
        pool[i] = pieces[randIndex];
        pool[i].used = false;
    }
}

// E�er �� par�a da kullan�ld�ysa, yeni par�a havuzu olu�turur
void UpdatePiecePool(Piece pool[3], int* piecesUsedCount) {
    if (*piecesUsedCount == 3) {
        InitializePiecePool(pool);
        *piecesUsedCount = 0;
    }
}

void ClearFullLines(GridCell grid[GRID_SIZE][GRID_SIZE], int* score) {
    bool fullRows[GRID_SIZE] = { false };
    bool fullCols[GRID_SIZE] = { false };

    // Hangi sat�r ve s�tunlar tamamen dolu i�aretler
    for (int i = 0; i < GRID_SIZE; i++) {
        bool rowFull = true;
        bool colFull = true;

        for (int j = 0; j < GRID_SIZE; j++) {
            if (grid[i][j].pieceIndex == 0) rowFull = false;
            if (grid[j][i].pieceIndex == 0) colFull = false;
        }

        fullRows[i] = rowFull;
        fullCols[i] = colFull;
    }

    // ��aretlenen sat�r ve s�tunlar� temizler
    for (int i = 0; i < GRID_SIZE; i++) {
        if (fullRows[i]) {
            for (int j = 0; j < GRID_SIZE; j++) {
                grid[i][j].pieceIndex = 0;
                grid[i][j].color = LIGHT;
                explosionGrid[i][j] = true;
            }
            *score += 100;
            PlaySound(explosionSound);
            isAnimating = true;
            animationFrames = 0;
        }

        if (fullCols[i]) {
            for (int j = 0; j < GRID_SIZE; j++) {
                grid[j][i].pieceIndex = 0;
                grid[j][i].color = LIGHT;
                explosionGrid[j][i] = true;
            }
            *score += 100;
            PlaySound(explosionSound);
            isAnimating = true;
            animationFrames = 0;
        }
    }
}

void PlacePieceIfPossible(GridCell grid[GRID_SIZE][GRID_SIZE], Piece piecePool[3], int* piecesUsedCount, int selectedPieceIndex, int mouseX, int mouseY, int* score) {
    Piece* p = &piecePool[selectedPieceIndex];

    // Par�an�n dolu h�crelerinin ba�lang�c�n� bulur 
    int firstX = -1, firstY = -1;
    for (int y = 0; y < p->sizeY; y++) {
        for (int x = 0; x < p->sizeX; x++) {
            if (p->shape[y][x] == 1) {
                if (firstX == -1 || x < firstX) firstX = x;
                if (firstY == -1 || y < firstY) firstY = y;
            }
        }
    }

    // Mouse pozisyonundan grid �zerindeki ba�lang�� h�cresini hesaplar
    int startX = (mouseX - 80) / CELL_SIZE - firstX;
    int startY = (mouseY - 80) / CELL_SIZE - firstY;

    // Par�an�n grid �zerinde yerle�tirilip yerle�tirilemeyece�ini kontrol ediyor
    bool canPlace = true;
    for (int y = 0; y < p->sizeY; y++) {
        for (int x = 0; x < p->sizeX; x++) {
            if (p->shape[y][x] == 1) {
                int gridX = startX + x;
                int gridY = startY + y;

                // Grid s�n�rlar� i�inde ve h�cre bo� mu kontrol eder
                if (gridX < 0 || gridX >= GRID_SIZE || gridY < 0 || gridY >= GRID_SIZE || grid[gridY][gridX].pieceIndex != 0) {
                    canPlace = false;
                    break;
                }
            }
        }
        if (!canPlace) break;
    }

    // Par�ay� yerle�tirir
    if (canPlace) {
        for (int y = 0; y < p->sizeY; y++) {
            for (int x = 0; x < p->sizeX; x++) {
                if (p->shape[y][x] == 1) {
                    int gridX = startX + x;
                    int gridY = startY + y;
                    grid[gridY][gridX].pieceIndex = selectedPieceIndex + 1;
                    grid[gridY][gridX].color = p->color;
                    PlaySound(placedsound);
                }
            }
        }

        // Par�a yerle�tirildi�inde puan ekle
        int pieceScore = 0;
        for (int y = 0; y < p->sizeY; y++) {
            for (int x = 0; x < p->sizeX; x++) {
                if (p->shape[y][x] == 1) {
                    pieceScore += 10;  // Her dolu h�cre i�in 10 puan ekle
                }
            }
        }
        *score += pieceScore;

        ClearFullLines(grid, score); // Dolmu� sat�r ve s�tunlar� temizle ve skoru g�ncelle

        // Par�an�n kullan�ld���n� i�aretle
        p->used = true;

        // Kullan�lan par�a say�s�n� art�r ve gerekirse par�a havuzunu yenile
        (*piecesUsedCount)++;
        UpdatePiecePool(piecePool, piecesUsedCount);
    }
}

bool CanPlacePiece(GridCell grid[GRID_SIZE][GRID_SIZE], Piece* p, int startX, int startY) {
    for (int y = 0; y < p->sizeY; y++) {
        for (int x = 0; x < p->sizeX; x++) {
            if (p->shape[y][x] == 1) {
                int gridX = startX + x;
                int gridY = startY + y;

                // Grid s�n�rlar� i�inde ve h�cre bo�sa kontrol et
                if (gridX < 0 || gridX >= GRID_SIZE || gridY < 0 || gridY >= GRID_SIZE || grid[gridY][gridX].pieceIndex != 0) {
                    return false;
                }
            }
        }
    }
    return true;
}

bool IsGameOver(GridCell grid[GRID_SIZE][GRID_SIZE], Piece piecePool[3]) {
    for (int i = 0; i < 3; i++) {
        Piece* p = &piecePool[i];
        if (!p->used) {
            for (int y = 0; y < GRID_SIZE; y++) {
                for (int x = 0; x < GRID_SIZE; x++) {
                    if (CanPlacePiece(grid, p, x, y)) {
                        return false; // Yerle�tirilebilir bir yer varsa oyun devam ediyor
                    }
                }
            }
        }
    }
    return true; // Hi�bir par�a yerle�tirilemiyorsa oyun bitti
}

bool DrawButton(int posX, int posY, int width, int height, const char* text) {
    int mouseX = GetMouseX();
    int mouseY = GetMouseY();
    bool isHovered = mouseX >= posX && mouseX <= posX + width && mouseY >= posY && mouseY <= posY + height;
    bool isClicked = isHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    Color buttonColor = isHovered ? DARKGRAY : GRAY;

    DrawRectangle(posX, posY, width, height, buttonColor);
    DrawText(text, posX + width / 2 - MeasureText(text, 20) / 2, posY + height / 2 - 10, 20, RAYWHITE);

    return isClicked;
}

void FillRandomCells(GridCell grid[GRID_SIZE][GRID_SIZE], int count, Color color) {
    int filled = 0;
    while (filled < count) {
        int x = GetRandomValue(0, GRID_SIZE - 1);
        int y = GetRandomValue(0, GRID_SIZE - 1);

        if (grid[y][x].pieceIndex == 0) { // H�cre bo�sa doldur
            grid[y][x].pieceIndex = 1; // Rastgele bir index veriyoruz
            grid[y][x].color = color;
            filled++;
        }
    }
}

void DrawExplosion(int x, int y, int frame) {
    // Patlama efektini animasyon karelerine g�re de�i�tir
    float alpha = 1.0f - (frame / 10.0f);     // Saydaml�k giderek azal�r
    int padding = frame * 2;                 // Patlama her karede geni�ler

    // H�crenin merkezini hesapla
    int centerX = x + CELL_SIZE / 2;
    int centerY = y + CELL_SIZE / 2;
    int radius = CELL_SIZE / 2 - padding;

    if (radius > 0) {
        Color explosionColor = Fade(TURUNCU, alpha);
        DrawCircle(centerX, centerY, radius, explosionColor);           // Dolgu daire
        DrawCircleLines(centerX, centerY, radius, Fade(KIRMIZI, alpha));    // Kenarl�k
    }
}

int LoadHighScore() {
    FILE* file = fopen(HIGH_SCORE_FILE, "r");
    int highScore = 0;
    if (file != NULL) {
        fscanf(file, "%d", &highScore);
        fclose(file);
    }
    return highScore;
}

void SaveHighScore(int score) {
    FILE* file = fopen(HIGH_SCORE_FILE, "w");
    if (file != NULL) {
        fprintf(file, "%d", score);
        fclose(file);
    }
}

int main(void) {
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "BLOCK BLAST");

    InitAudioDevice(); // Ses cihaz�n� ba�lat
    explosionSound = LoadSound("ses2.wav"); // Ses dosyas�n� y�kle
    gameOverSound = LoadSound("gameover.wav"); // Game over ses dosyas�n� y�kle
    placedsound = LoadSound("ses.wav");


    GameScreen currentScreen = LOGO;

    int framesCounter = 0;
    int piecesUsedCount = 0;
    GridCell grid[GRID_SIZE][GRID_SIZE] = { 0 };
    Piece piecePool[3];
    InitializePiecePool(piecePool);

    FillRandomCells(grid, 15, TURUNCU); // Grid �zerine rastgele 20 h�creyi doldur

    bool isDragging = false;
    int draggedPiece = -1;
    int dragOffsetX = 0;
    int dragOffsetY = 0;

    int score = 0;
    highScore = LoadHighScore();

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        // === 1. G�NCELLEME FAZI ===
        switch (currentScreen)
        {
        case LOGO:
            if (IsKeyPressed(KEY_ENTER) || IsGestureDetected(GESTURE_TAP)) {
                currentScreen = TITLE;
            }
            break;

        case TITLE:
            if (IsKeyPressed(KEY_ENTER) || IsGestureDetected(GESTURE_TAP)) {
                currentScreen = GAMEPLAY;
                framesCounter = 0;
                memset(grid, 0, sizeof(grid));
                memset(explosionGrid, 0, sizeof(explosionGrid));
                InitializePiecePool(piecePool);
                FillRandomCells(grid, 15, TURUNCU);
                score = 0;
                piecesUsedCount = 0;
                gameOverSoundPlayed = false;
            }

            if (DrawButton(screenWidth - 120, screenHeight - 60, 100, 40, "Close")) {
                CloseWindow();
                return 0;
            }
            break;

        case GAMEPLAY:
        {
            int mouseX = GetMouseX();
            int mouseY = GetMouseY();

            const int pieceStartX = 500;
            const int pieceStartY = 100;
            const int pieceSpacing = MAX_SIZE * CELL_SIZE + 10;

            // --- Par�a se�me ---
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                for (int i = 0; i < 3; i++) {
                    int pieceX = pieceStartX;
                    int pieceY = pieceStartY + i * pieceSpacing;

                    int pieceWidth = piecePool[i].sizeX * CELL_SIZE;
                    int pieceHeight = piecePool[i].sizeY * CELL_SIZE;

                    Rectangle pieceRect = { pieceX, pieceY, pieceWidth, pieceHeight };

                    if (CheckCollisionPointRec(GetMousePosition(), pieceRect)) {
                        int localX = (mouseX - pieceX) / CELL_SIZE;
                        int localY = (mouseY - pieceY) / CELL_SIZE;

                        //s�n�r kontrol�
                        if (localX >= 0 && localX < piecePool[i].sizeX &&
                            localY >= 0 && localY < piecePool[i].sizeY &&
                            piecePool[i].shape[localY][localX] == 1)
                        {
                            isDragging = true;
                            draggedPiece = i;
                            dragOffsetX = mouseX - pieceX;
                            dragOffsetY = mouseY - pieceY;
                            break;
                        }
                    }
                }
            }

            //s�r�klenen par�ay� b�rakma
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && isDragging) {
                PlacePieceIfPossible(grid, piecePool, &piecesUsedCount, draggedPiece, mouseX - dragOffsetX, mouseY - dragOffsetY, &score);
                isDragging = false;
                draggedPiece = -1;
            }

            //game over kontrol�
            if (IsGameOver(grid, piecePool)) {
                currentScreen = ENDING;

                if (score > highScore) {
                    highScore = score;
                    SaveHighScore(highScore);
                }
            }
            if (DrawButton(screenWidth - 100, screenHeight - 50, 90, 40, "Replay")) {
                currentScreen = GAMEPLAY;
                framesCounter = 0;
                memset(grid, 0, sizeof(grid));
                memset(explosionGrid, 0, sizeof(explosionGrid));
                InitializePiecePool(piecePool);
                FillRandomCells(grid, 15, TURUNCU);
                score = 0;
                piecesUsedCount = 0;
                gameOverSoundPlayed = false;
            }

            if (DrawButton(screenWidth - 200, screenHeight - 50, 90, 40, "Exit")) {
                currentScreen = TITLE;
                framesCounter = 0;
            }
        }
        break;

        case ENDING:
            if (!gameOverSoundPlayed) {
                PlaySound(gameOverSound);
                gameOverSoundPlayed = true;
            }

            if (IsKeyPressed(KEY_ENTER) || IsGestureDetected(GESTURE_TAP)) {
                currentScreen = TITLE;
                framesCounter = 0;
            }
            break;
        }

        // === 2. ��Z�M FAZI ===
        BeginDrawing();
        ClearBackground(RAYWHITE);

        switch (currentScreen)
        {
        case LOGO:
            DrawText("START SCREEN", 30, 30, 20, TURUNCU);
            DrawText("WELCOME", 245, 130, 65, KIRMIZI);
            DrawText("How to Play", 50, 240, 30, TURUNCU);
            DrawText("Place blocks on the 8x8 grid to complete rows or columns.", 50, 280, 20, GRAY);
            DrawText("3 pieces appear each round drag and drop them onto the grid.", 50, 310, 20, GRAY);
            DrawText("Cleared lines earn points.", 50, 340, 20, GRAY);
            DrawText("Game ends when no pieces can be placed.", 50, 370, 20, GRAY);
            DrawText("Tip: Leave room for tricky pieces, use space wisely!", 50, 400, 20, GRAY);
            DrawText("Press ENTER or TAP to continue...", 200, 500, 20, YESIL);
            break;

        case TITLE:
        {
            DrawRectangle(0, 0, screenWidth, screenHeight, RAYWHITE);
            DrawText("BLOCK BLAST", (screenWidth - MeasureText("BLOCK BLAST", 80)) / 2, 150, 80, RED);
            DrawText("PRESS ENTER or TAP to START GAME", 220, 500, 15, ORANGE);
            DrawText(TextFormat("HIGH SCORE: %d", highScore), 245, 245, 30, RED);

            if (DrawButton(screenWidth - 120, screenHeight - 60, 100, 40, "Close")) {
                currentScreen = TITLE;
            }

            Piece demo1 = { 3, 3, {{1,0,0},{1,1,0},{1,0,0}}, RED, false };
            Piece demo2 = { 2, 3, {{1,1},{0,1},{0,1}}, PURPLE, false };
            Piece demo3 = { 2, 2, {{0,1},{1,1}}, GREEN, false };
            Piece demo4 = { 1, 4, {{1},{1},{1},{1}}, ORANGE, false };

            DrawPiece(demo1, 300, 300, CELL_SIZE, false);
            DrawPiece(demo2, 340, 300, CELL_SIZE, false);
            DrawPiece(demo3, 300, 380, CELL_SIZE, false);
            DrawPiece(demo4, 420, 300, CELL_SIZE, false);


        }
        break;

        case GAMEPLAY:
            DrawGameGrid(grid, 100, 100);

            if (isDragging && draggedPiece != -1) {
                int mouseX = GetMouseX();
                int mouseY = GetMouseY();
                DrawPiecePool(piecePool, 500, 100, draggedPiece);
                DrawPiece(piecePool[draggedPiece], mouseX - dragOffsetX, mouseY - dragOffsetY, CELL_SIZE, true);
            }
            else {
                DrawPiecePool(piecePool, 500, 100, -1);
            }

            if (isAnimating) {
                for (int i = 0; i < GRID_SIZE; i++) {
                    for (int j = 0; j < GRID_SIZE; j++) {
                        if (explosionGrid[i][j] && animationFrames < animationDuration) {
                            DrawExplosion(100 + j * CELL_SIZE, 100 + i * CELL_SIZE, animationFrames);
                        }
                    }
                }
                animationFrames++;
                if (animationFrames >= animationDuration) {
                    isAnimating = false;
                    memset(explosionGrid, 0, sizeof(explosionGrid));
                }
            }

            DrawText(TextFormat("SCORE: %d", score), 200, 50, 20, YESIL);
            DrawText(TextFormat("HIGH SCORE: %d", highScore), 380, 50, 20, TURUNCU);
            break;

        case ENDING:
            DrawRectangle(0, 0, screenWidth, screenHeight, RAYWHITE);
            DrawText("GAME OVER", 180, 155, 70, RED);
            DrawText(TextFormat("FINAL SCORE: %d", score), 250, 275, 30, YESIL);
            DrawText(TextFormat("HIGH SCORE: %d", highScore), 250, 315, 30, YESIL);
            DrawText("PRESS ENTER or TAP to RETURN to TITLE SCREEN", 120, 400, 20, DARKGRAY);
            break;
        }

        EndDrawing();
    }

    UnloadSound(explosionSound); // Ses dosyas�n� serbest b�rak
    UnloadSound(gameOverSound); // Game over ses dosyas�n� serbest b�rak
    UnloadSound(placedsound);
    CloseAudioDevice(); // Ses cihaz�n� kapat
    CloseWindow();

    return 0;
}
