
`` 

#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

// 게임 상태 정의
typedef enum { STATE_START, STATE_PLAYING, STATE_GAME_OVER, STATE_CLEAR } GameState;

// 패들 구조체
typedef struct {
    Rectangle rect;
    float speed;
} Paddle;

// 공 구조체
typedef struct {
    Vector2 pos;
    Vector2 speed;
    float radius;
    bool active;
} Ball;

// 블록 구조체
typedef struct {
    Rectangle rect;
    Color color;
    bool destroyed;
} Block;

#define ROWS 7
#define COLS 10

int main(void) {
    // 윈도우 초기화
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Rainbow Breakout - Raylib");
    SetTargetFPS(60);

    GameState state = STATE_START;

    // 패들 초기화
    Paddle paddle = {
        { SCREEN_WIDTH / 2.0f - 50.0f, SCREEN_HEIGHT - 40.0f, 100.0f, 15.0f },
        8.0f
    };

    // 공 초기화
    Ball ball = {
        { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT - 60.0f },
        { 5.0f, -5.0f },
        8.0f,
        false
    };

    // 무지개 색상 배열 (빨, 주, 노, 초, 파, 남, 보)
    Color rainbowColors[ROWS] = {
        RED,
        ORANGE,
        YELLOW,
        GREEN,
        BLUE,
        DARKBLUE,
        PURPLE
    };

    // 블록 초기화
    Block blocks[ROWS][COLS];
    float blockWidth = 70.0f;
    float blockHeight = 20.0f;
    float offsetX = 35.0f;
    float offsetY = 60.0f;
    float padding = 10.0f;

    // 블록 배치 함수 (재설정용)
    void ResetBlocks() {
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLS; j++) {
                blocks[i][j].rect = (Rectangle){
                    offsetX + j * (blockWidth + padding),
                    offsetY + i * (blockHeight + padding),
                    blockWidth,
                    blockHeight
                };
                blocks[i][j].color = rainbowColors[i];
                blocks[i][j].destroyed = false;
            }
        }
    }

    ResetBlocks();
    int score = 0;
    int lives = 3;

    while (!WindowShouldClose()) {
        // --- 1. 업데이트(Update) ---
        switch (state) {
            case STATE_START:
                if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                    state = STATE_PLAYING;
                    ball.active = true;
                }
                break;

            case STATE_PLAYING:
                // 패들 이동 (좌우 방향키 또는 A/D)
                if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) paddle.rect.x -= paddle.speed;
                if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) paddle.rect.x += paddle.speed;

                // 패들 화면 이탈 방지
                if (paddle.rect.x < 0) paddle.rect.x = 0;
                if (paddle.rect.x + paddle.rect.width > SCREEN_WIDTH) 
                    paddle.rect.x = SCREEN_WIDTH - paddle.rect.width;

                // 공 움직이기
                if (ball.active) {
                    ball.pos.x += ball.speed.x;
                    ball.pos.y += ball.speed.y;
                } else {
                    ball.pos.x = paddle.rect.x + paddle.rect.width / 2.0f;
                    ball.pos.y = paddle.rect.y - ball.radius - 2.0f;
                    if (IsKeyPressed(KEY_SPACE)) {
                        ball.active = true;
                    }
                }

                // 벽 충돌 (좌/우/상)
                if (ball.pos.x - ball.radius < 0) { ball.pos.x = ball.radius; ball.speed.x *= -1; }
                if (ball.pos.x + ball.radius > SCREEN_WIDTH) { ball.pos.x = SCREEN_WIDTH - ball.radius; ball.speed.x *= -1; }
                if (ball.pos.y - ball.radius < 0) { ball.pos.y = ball.radius; ball.speed.y *= -1; }

                // 바닥 충돌 (목숨 감소)
                if (ball.pos.y + ball.radius > SCREEN_HEIGHT) {
                    lives--;
                    ball.active = false;
                    if (lives <= 0) {
                        state = STATE_GAME_OVER;
                    }
                }

                // 패들과 공 충돌
                Rectangle ballRect = { ball.pos.x - ball.radius, ball.pos.y - ball.radius, ball.radius * 2, ball.radius * 2 };
                if (CheckCollisionRecs(ballRect, paddle.rect)) {
                    ball.speed.y *= -1;
                    // 패들의 맞은 위치에 따라 X 반사각 조절
                    float hitFactor = (ball.pos.x - (paddle.rect.x + paddle.rect.width / 2.0f)) / (paddle.rect.width / 2.0f);
                    ball.speed.x = hitFactor * 6.0f;
                }

                // 블록과 공 충돌 체크
                bool allCleared = true;
                for (int i = 0; i < ROWS; i++) {
                    for (int j = 0; j < COLS; j++) {
                        if (!blocks[i][j].destroyed) {
                            allCleared = false;
                            if (CheckCollisionCircleRec(ball.pos, ball.radius, blocks[i][j].rect)) {
                                blocks[i][j].destroyed = true;
                                ball.speed.y *= -1;
                                score += (ROWS - i) * 10; // 위쪽 블록일수록 점수 높음
                            }
                        }
                    }
                }

                if (allCleared) {
                    state = STATE_CLEAR;
                }
                break;

            case STATE_GAME_OVER:
            case STATE_CLEAR:
                if (IsKeyPressed(KEY_R)) {
                    ResetBlocks();
                    score = 0;
                    lives = 3;
                    ball.active = false;
                    state = STATE_PLAYING;
                }
                break;
        }

        // --- 2. 그리기(Draw) ---
        BeginDrawing();
        ClearBackground(RAYWHITE);

        switch (state) {
            case STATE_START:
                DrawText("RAINBOW BREAKOUT", SCREEN_WIDTH / 2 - MeasureText("RAINBOW BREAKOUT", 40) / 2, SCREEN_HEIGHT / 2 - 60, 40, DARKGRAY);
                DrawText("Press ENTER or SPACE to Start", SCREEN_WIDTH / 2 - MeasureText("Press ENTER or SPACE to Start", 20) / 2, SCREEN_HEIGHT / 2 + 10, 20, GRAY);
                DrawText("Move: Arrow Keys or A / D", SCREEN_WIDTH / 2 - MeasureText("Move: Arrow Keys or A / D", 16) / 2, SCREEN_HEIGHT / 2 + 60, 16, LIGHTGRAY);
                break;

            case STATE_PLAYING:
                // 블록 그리기
                for (int i = 0; i < ROWS; i++) {
                    for (int j = 0; j < COLS; j++) {
                        if (!blocks[i][j].destroyed) {
                            DrawRectangleRec(blocks[i][j].rect, blocks[i][j].color);
                            DrawRectangleLines((int)blocks[i][j].rect.x, (int)blocks[i][j].rect.y, (int)blocks[i][j].rect.width, (int)blocks[i][j].rect.height, Fade(BLACK, 0.2f));
                        }
                    }
                }

                // 패들 및 공 그리기
                DrawRectangleRec(paddle.rect, DARKGRAY);
                DrawCircleV(ball.pos, ball.radius, MAROON);

                // UI (점수 및 생명)
                DrawText(TextFormat("SCORE: %d", score), 20, 20, 20, DARKGRAY);
                DrawText(TextFormat("LIVES: %d", lives), SCREEN_WIDTH - 120, 20, 20, DARKGRAY);
                
                if (!ball.active) {
                    DrawText("Press SPACE to Launch", SCREEN_WIDTH / 2 - MeasureText("Press SPACE to Launch", 20) / 2, SCREEN_HEIGHT / 2 + 100, 20, GRAY);
                }
                break;

            case STATE_GAME_OVER:
                DrawText("GAME OVER", SCREEN_WIDTH / 2 - MeasureText("GAME OVER", 40) / 2, SCREEN_HEIGHT / 2 - 40, 40, RED);
                DrawText(TextFormat("Final Score: %d", score), SCREEN_WIDTH / 2 - MeasureText(TextFormat("Final Score: %d", score), 20) / 2, SCREEN_HEIGHT / 2 + 10, 20, DARKGRAY);
                DrawText("Press 'R' to Restart", SCREEN_WIDTH / 2 - MeasureText("Press 'R' to Restart", 20) / 2, SCREEN_HEIGHT / 2 + 50, 20, GRAY);
                break;

            case STATE_CLEAR:
                DrawText("VICTORY!", SCREEN_WIDTH / 2 - MeasureText("VICTORY!", 40) / 2, SCREEN_HEIGHT / 2 - 40, 40, GREEN);
                DrawText(TextFormat("Final Score: %d", score), SCREEN_WIDTH / 2 - MeasureText(TextFormat("Final Score: %d", score), 20) / 2, SCREEN_HEIGHT / 2 + 10, 20, DARKGRAY);
                DrawText("Press 'R' to Restart", SCREEN_WIDTH / 2 - MeasureText("Press 'R' to Restart", 20) / 2, SCREEN_HEIGHT / 2 + 50, 20, GRAY);
                break;
        }

        EndDrawing();
    }

    // 자원 해제
    CloseWindow();
    return 0;
}

`` 

