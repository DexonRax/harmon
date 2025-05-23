#pragma once
#include <vector>
#include <array>
#include <string>

class Game {
public:
    Game(int width, int height);
    ~Game();

    void Run();

private:
    void HandleMenu();
    void HandleGameplay();
    void GenerateMap(int);
    void StartTimer();
    int GetElapsedTime() const;
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
    int m_keys[4];
    double m_speedMultiplier;
    std::vector<std::array<int, 3>> m_map;
    std::vector<std::string> m_mapList;
    int m_mapListIndex;
};
