#pragma once
#include <vector>
#include <array>

class Game {
public:
    Game(int width, int height);
    ~Game();

    void Run();

private:
    void HandleMenu();
    void HandleGameplay();
    void GenerateMap();
    void StartTimer();
    int GetElapsedTime() const;

    int m_screenWidth, m_screenHeight;
    bool m_mapPlaying;
    int m_startTime;
    int m_approachTime;
    int m_missTime;
    int m_judgementY;
    int m_noteHeight;
    int m_keys[4];
    std::vector<std::array<int, 3>> m_map;
};
