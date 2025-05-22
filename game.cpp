#include "game.h"
#include <raylib.h>
#include <cstdlib>
#include <iostream>


Game::Game(int width, int height){
    m_approachTime = 650; //ms
    m_missTime = 200; //ms
    m_mapPlaying = false;
    m_screenWidth = width;
    m_screenHeight = height;

    m_noteHeight = 30;
    m_judgementY = m_screenWidth/12+m_noteHeight;

    m_keys[0] = KEY_A;
    m_keys[1] = KEY_S;
    m_keys[2] = KEY_K;
    m_keys[3] = KEY_L;

    m_startTime = 0;
    InitWindow(m_screenWidth, m_screenHeight, "Harmon");
    SetTargetFPS(240);
}

Game::~Game(){
    CloseWindow();
}

int Game::GetElapsedTime() const {
    if(m_mapPlaying){
        return (int)((GetTime() - m_startTime) * 1000.0);
    }
    return 0;
}

void Game::StartTimer() {
    if(m_mapPlaying){
        m_startTime = GetTime();
    }
}

void Game::GenerateMap(){
    int delay = 180;
    m_map.clear();
    for(int i = 0; i < 100; i++){
        int row = rand() % 4;
        m_map.push_back({row, 3000+i*delay, 1});
    }
}

void Game::HandleMenu(){
    BeginDrawing();
    ClearBackground(GRAY);

    if(IsKeyPressed(KEY_ENTER)){
        m_mapPlaying = true;
        GenerateMap();
        StartTimer();
    }

    DrawText(TextFormat("FPS: %d", GetFPS()), 10, 10, 20, GREEN);
    DrawText(TextFormat("Menu"), m_screenWidth/2, m_screenHeight/2, 20, GREEN);
    DrawText(TextFormat("[Enter]"), m_screenWidth/2, m_screenHeight/2+30, 20, GREEN);

    EndDrawing();

}

void Game::HandleGameplay(){
    BeginDrawing();
    ClearBackground(BLACK);

    DrawText(TextFormat("FPS: %d", GetFPS()), 10, 10, 20, GREEN);
    DrawText(TextFormat("Timer: %i", GetElapsedTime()), 10, m_screenHeight-20, 20, GREEN);

    int screenCenterX = m_screenWidth/2;
    int colWidth = m_screenWidth/12;

    for(int i = 0; i < 4; i++){
        int red = 40 + 10 * (i + 1);
        int green = 40;
        int blue = 40;
        Color color = { (unsigned char)red, (unsigned char)green, (unsigned char)blue, 255 };
        DrawRectangle(screenCenterX+(i-2)*colWidth, 0, colWidth, m_screenHeight, color);
    }

    DrawRectangle(screenCenterX-colWidth*2, m_screenHeight-m_judgementY, colWidth*4, m_noteHeight, GRAY);


    for(int i = 0; i < m_map.size(); i++){
        if(m_map[i][2] == 1){
            if(m_map[i][1] - m_approachTime < GetElapsedTime() && m_map[i][1] + m_missTime > GetElapsedTime()){
                double tt = m_map[i][1] - GetElapsedTime();
                double prog = 1.0 - (tt / (double)m_approachTime);

                if(prog > 0.85){
                    if(IsKeyPressed(m_keys[m_map[i][0]])){
                        m_map[i][2] = 0;
                    }
                }

                int yPos = (m_screenHeight - m_judgementY) * prog;
                DrawRectangle(screenCenterX+(m_map[i][0]-2)*colWidth, yPos, colWidth, m_noteHeight, RED);
                //DrawText(TextFormat("%f", prog), screenCenterX-(m_map[i][0]-1)*colWidth, yPos, 20, BLACK);
            }
        }
    }

    //end map after last note
    if(GetElapsedTime()-2000 > m_map[m_map.size()-1][1]){
        m_mapPlaying = false;
    }


    EndDrawing();
}

void Game::Run(){
    while (!WindowShouldClose()) {
        if(!m_mapPlaying){
            HandleMenu();
        }else{
            HandleGameplay();
        }
    }
}