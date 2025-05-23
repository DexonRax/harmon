#include "game.h"
#include <raylib.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <algorithm>

Game::Game(int width, int height){
    m_speedMultiplier = 1.0;

    m_approachTime = 650; //ms
    m_missTime = 200; //ms
    m_mapPlaying = false;
    m_screenWidth = width;
    m_screenHeight = height;

    m_mapListIndex = 0;

    m_noteHeight = 30;
    m_judgementY = m_screenWidth/12+m_noteHeight;

    m_keys[0] = KEY_A;
    m_keys[1] = KEY_S;
    m_keys[2] = KEY_K;
    m_keys[3] = KEY_L;

    m_startTime = 0;
    InitWindow(m_screenWidth, m_screenHeight, "Harmon");
    SetTargetFPS(240);
    SetExitKey(0);
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

bool Game::LoadMapFile(std::string filename){
    std::ifstream file(filename);
    m_map.clear();
    if (file.is_open()) {
        int a, b;
        while (file >> a >> b) {
            m_map.push_back({a, b, 1});
        }
        file.close();
    } else {
        std::cerr << "Couldn't open the map file!\n";
        return false;
    }
    return true;
}

void Game::GenerateMap(int length){
    int delay = 220;
    m_map.clear();
    for(int i = 0; i < length; i++){
        int row = rand() % 4;
        if(rand()%3==0){
            m_map.push_back({(row+rand()%3)%4, 3000+i*delay, 1});
            std::cout<<(row+rand()%3)%4<<" "<<3000+i*delay<<"\n";
        }
        m_map.push_back({row, 3000+i*delay, 1});
        std::cout<<row<<" "<<3000+i*delay<<"\n";
    }
}

void Game::PlayMap(){
    if(!LoadMapFile(m_mapList[m_mapListIndex])){
        StopMap();
    }else{
        m_mapPlaying = true;
        StartTimer();
    }
    
}

void Game::StopMap(){
    m_mapPlaying = false;
}


void Game::HandleMenu(){
    if(IsKeyPressed(KEY_ENTER)){
        PlayMap();
    }

    if(IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_DOWN)){
        if(m_mapListIndex != m_mapList.size()-1)
            m_mapListIndex++;
    }else if(IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_UP)){
        if(m_mapListIndex != 0)
            m_mapListIndex--;
    }

    BeginDrawing();
    ClearBackground(BLACK);

    if(m_mapList.empty()){
        try {
            for (const auto& entry : std::filesystem::directory_iterator("maps")) {
                if (std::filesystem::is_regular_file(entry.status())) {
                    std::cout << entry.path() << '\n';
                    m_mapList.push_back(entry.path());
                } 
            }
            std::sort(m_mapList.begin(), m_mapList.end());
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Error: " << e.what() << '\n';
        }
    }else{
        for(int i = 0; i < m_mapList.size(); i++){
            if(i == m_mapListIndex){
                DrawText(TextFormat(m_mapList[i].c_str(), GetFPS()), m_screenWidth-200, 20+20*i, 20, RED);
            }else{
                DrawText(TextFormat(m_mapList[i].c_str(), GetFPS()), m_screenWidth-200, 20+20*i, 20, GREEN);
            }   
        }
    }

    DrawText(TextFormat("FPS: %d", GetFPS()), 10, 10, 20, GREEN);
    DrawText(TextFormat("Menu"), m_screenWidth/2, m_screenHeight/2, 20, GREEN);
    DrawText(TextFormat("[Enter]"), m_screenWidth/2, m_screenHeight/2+30, 20, GREEN);

    EndDrawing();

}

void Game::HandleGameplay(){

    if(IsKeyPressed(KEY_ESCAPE)){
        StopMap();
    }

    BeginDrawing();
    ClearBackground(BLACK);

    DrawText(TextFormat("FPS: %d", GetFPS()), 10, 10, 20, GREEN);
    DrawText(TextFormat("Timer: %i", GetElapsedTime()), 10, m_screenHeight-20, 20, GREEN);
    DrawText(TextFormat("%s", m_mapList[m_mapListIndex].c_str()), m_screenWidth-200, m_screenHeight-20, 20, GREEN);

    int screenCenterX = m_screenWidth/2;
    int colWidth = m_screenWidth/12;

    for(int i = 0; i < 4; i++){
        int red = 40 + 10 * (i%2);
        int green = 40 + 10 * (i%2);
        int blue = 40 + 10 * (i%2);
        Color color = { (unsigned char)red, (unsigned char)green, (unsigned char)blue, 255 };
        DrawRectangle(screenCenterX+(i-2)*colWidth, 0, colWidth, m_screenHeight, color);
    }

    DrawRectangle(screenCenterX-colWidth*2, m_screenHeight-m_judgementY, colWidth*4, m_noteHeight, GRAY);

    int currentTime = GetElapsedTime()*m_speedMultiplier;

    for(int i = 0; i < m_map.size(); i++){
        if(m_map[i][2] == 1){
            if(m_map[i][1] - m_approachTime < currentTime && m_map[i][1] + m_missTime > currentTime){
                double tt = m_map[i][1] - currentTime;
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
    if(currentTime-2000 > m_map[m_map.size()-1][1]){
        StopMap();
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