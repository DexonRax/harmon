#pragma once
#include <vector>
#include <array>
#include <string>
#include <raylib.h>

class Game {
public:
    Game(int width, int height);
    ~Game();

    void Run();

private:
    void HandleMenu();
    void HandleGameplay();
    void GenerateMap(double, double, double, std::string);
    void StartTimer();
    int GetElapsedTime();
    void PlayMap();
    void StopMap();
    bool LoadMapFile(std::string);
    void ReloadMapList();

    int m_screenWidth, m_screenHeight;
    bool m_mapPlaying;
    int m_startTime;
    int m_approachTime;
    int m_missTime;
    int m_judgementY;
    int m_noteHeight;
    int m_startDelay;
    int m_keys[4];
    double m_speedMultiplier;
    std::vector<int> m_noteJudgements;
    std::vector<std::array<int, 3>> m_map;
    std::vector<std::string> m_mapList;
    int m_mapListIndex;
    bool m_exitWindowRequested;
    Music m_musicPlayer;
};
