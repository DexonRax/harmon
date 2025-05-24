#include "game.h"
#include <raylib.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <fstream>

Game::Game(int width, int height){
    srand(time(NULL));
    m_speedMultiplier = 1.0;

    m_exitWindowRequested = false;

    m_approachTime = 750; //ms
    m_missTime = 200; //ms
    m_mapPlaying = false;
    m_screenWidth = width;
    m_screenHeight = height;
    m_startDelay = 1500;

    m_mapListIndex = 0;

    m_noteHeight = 30;
    m_judgementY = m_screenWidth/12+m_noteHeight;

    m_keys[0] = KEY_A;
    m_keys[1] = KEY_S;
    m_keys[2] = KEY_K;
    m_keys[3] = KEY_L;

    m_startTime = 0;
    InitWindow(m_screenWidth, m_screenHeight, "Harmon");
    SetTargetFPS(1000);
    SetExitKey(0);
    InitAudioDevice();
}

Game::~Game(){
    UnloadMusicStream(m_musicPlayer);
    CloseAudioDevice();
    CloseWindow();
}

int Game::GetElapsedTime(){
    if(m_mapPlaying){
        return (int)((GetTime()*1000 - m_startTime));
    }
    return 0;
}

void Game::StartTimer() {
    if(m_mapPlaying){
        m_startTime = GetTime()*1000;
    }
}

bool Game::LoadMapFile(std::string filename){

    std::string mapPath = "maps/"+filename+"/notes.txt";
    std::string audioPath = "maps/"+filename+"/audio.mp3";
    
    std::cout<<mapPath<<"\n";

    m_map.clear();

    std::ifstream file(mapPath);
    if (file.is_open()) {
        int col, time;
        while (file >> col >> time) {
            m_map.push_back({col, time, 1});
        }
        file.close();
        m_musicPlayer = LoadMusicStream(audioPath.c_str());
    } else {
        std::cerr << "Couldn't open the map file!\n";
        return false;
    }
    return true;

}

void Game::ReloadMapList(){

    m_mapList.clear();

    std::string path = "maps";

    try {
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            if (entry.is_directory()) {
                //std::cout << entry.path().filename() << '\n';
                m_mapList.push_back(entry.path().filename());
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Couldn't load maps!\n";
    }
    
}

void Game::GenerateMap(double startDelay, double endTime, double delay, std::string path){
    std::ofstream map(path);

    int i = 0;
    while(startDelay + i*delay < endTime){
        int col = rand() % 4;
        double timing = startDelay+(double)i*delay;
        map<<col<<" "<<(int)timing<<"\n";
        i++;
    }

    std::cout<<"Map generated!\n";
    ReloadMapList();
}

void Game::PlayMap(){
    m_noteJudgements.clear();
    if(!LoadMapFile(m_mapList[m_mapListIndex])){
        StopMap();
    }else{
        m_mapPlaying = true;
        StartTimer();
        PlayMusicStream(m_musicPlayer);
        SetMusicPitch(m_musicPlayer, m_speedMultiplier);
    }
    
}

void Game::StopMap(){
    StopMusicStream(m_musicPlayer);
    m_mapPlaying = false;
}

void Game::HandleMenu(){

    if(IsKeyPressed(KEY_ESCAPE)){
        m_exitWindowRequested = true;
    }

    if(IsKeyPressed(KEY_G)){
        GenerateMap(0.0, 120000.0, 250.0, "maps/240bpm/notes.txt");
    }

    if(IsKeyPressed(KEY_ENTER)){
        PlayMap();
    }

    if(IsKeyPressed(KEY_F5)){
        ReloadMapList();
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
        ReloadMapList();
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
    

    int notesHit = 0;
    int notesMissed = 0;
    for(int i = 0; i < m_noteJudgements.size(); i++){
        if(m_noteJudgements[i] == 1){
            notesHit++;
        }else{
            notesMissed++;
        }
    }

    double accuracy = 100.0;
    if(m_noteJudgements.size()>0){
        accuracy = ((double)m_noteJudgements.size()-(double)notesMissed)/(double)m_noteJudgements.size();
        accuracy = std::clamp(accuracy, 0.0, 1.0);
        accuracy *= 100;
    }

    DrawText(TextFormat("Hits: %i/%i", notesHit, m_noteJudgements.size()), 10, 40, 20, GREEN);
    DrawText(TextFormat("Acc: %.2f", accuracy), 10, 70, 20, GREEN);

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


    int currentTime = GetElapsedTime()*m_speedMultiplier-m_startDelay;

    DrawText(TextFormat("Timer: %i", currentTime), 10, m_screenHeight-20, 20, GREEN);

    if(0 > currentTime){
        if(IsMusicStreamPlaying(m_musicPlayer)){
            PauseMusicStream(m_musicPlayer);
        }
    }else{
        if(!IsMusicStreamPlaying(m_musicPlayer)){
            ResumeMusicStream(m_musicPlayer);
        }
    }

    UpdateMusicStream(m_musicPlayer);

    for(int i = 0; i < m_map.size(); i++){
        if(m_map[i][2] == 1){
            if(m_map[i][1] - m_approachTime - 100.0 < currentTime && m_map[i][1] + m_missTime + 100.0 > currentTime){
                double tt = m_map[i][1] - currentTime;
                double prog = 1.0 - (tt / (double)m_approachTime);

                if(prog > 0.85){
                    if(IsKeyPressed(m_keys[m_map[i][0]])){
                        m_map[i][2] = 0;
                        m_noteJudgements.push_back(1);
                    }
                }

                if(m_map[i][1] + m_missTime < currentTime){
                    m_map[i][2] = 0;
                    m_noteJudgements.push_back(0);
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
    while (!WindowShouldClose() && !m_exitWindowRequested) {
        if(!m_mapPlaying){
            HandleMenu();
        }else{
            HandleGameplay();
        }
    }
}