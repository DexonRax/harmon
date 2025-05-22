#include <iostream>
#include <vector>
#include <raylib.h>
//#include "raylib/build/raylib/include/raylib.h"
#include <algorithm> //std::clamp
#include <fstream>

class KeyRows {
private:
    int m_height;
    int m_width;
    const int keycount = 4;
    int* m_notes = nullptr;
    double* m_noteTimings = nullptr;
    int m_notesLen = 0;

public:
    KeyRows(int width, int height) {
        m_height = height;
        m_width  = width / 3;
    }

    ~KeyRows() {
        delete[] m_notes;
        delete[] m_noteTimings;
    }

    void GenerateNotes(int num, double delay) {
        m_notesLen       = num;
        m_notes         = new int[m_notesLen];
        m_noteTimings   = new double[m_notesLen];
        for (int i = 0; i < m_notesLen; i++) {
            m_notes[i] = rand() % keycount;
            m_noteTimings[i] = delay * i + 3000.0;
        }
    }

    int GetWidth()              const { return m_width; }
    int GetHeight()             const { return m_height; }
    int GetNotesCount()         const { return m_notesLen; }
    int GetNoteRow(int i)       const { return m_notes[i]; }
    double GetNoteTiming(int i) const { return m_noteTimings[i]; }
};

class Game {
private:
    Color BG_COLOR = GRAY;
    Color ROW_COLOR = BLACK;
    Color NOTE_COLOR = RED;
    Color HITBAR_COLOR = RAYWHITE;

    int  m_screenWidth;
    int  m_screenHeight;
    double m_startTime = 0.0;
    bool   m_playing   = false;

    // rytmiczne parametry:
    double approachTime = 700.0; // ms przed timingiem start nutki
    int judgmentY;       // y linii oceny
    int noteHeight;
    int startY;          // startowa pozycja y (powyżej ekranu)
    int travelDistance;  // dystans do przebycia w pikselach

    int m_nextNoteIndex = 0;     // index następnej nutki do oceny
    int m_score = 0;             // punkty
    std::vector<bool> m_hitFlags; // czy dana nutka została trafiona lub przegapiona

public:
    KeyRows* keyRows = nullptr;

    Game(int width, int height)
        : m_screenWidth(width), m_screenHeight(height)
    {
        approachTime = 700.0;
        std::string line;
        std::ifstream settingsFile("settings.ini");
        while (getline (settingsFile, line)) {
            if(line.find("at=") != std::string::npos){
                //std::cout<<"AAAAAAAAAAAAA "<<line.substr(3, line.length())<<"\n";
                approachTime = std::stod(line.substr(3, line.length()));
            }
        }
        keyRows        = new KeyRows(width, height);
        judgmentY      = m_screenHeight - m_screenHeight / 8;
        noteHeight     = m_screenHeight / 20;
        startY         = -noteHeight;
        travelDistance = judgmentY - startY;
    }

    ~Game() {
        delete keyRows;
    }

    int GetElapsedTime() const {
        return (int)((GetTime() - m_startTime) * 1000.0);
    }

    void StartTimer() {
        m_startTime = GetTime();
    }

    void Play() {
        m_playing       = true;
        keyRows->GenerateNotes(8000, 120.0);
        StartTimer();
        m_score         = 0;
        m_nextNoteIndex = 0;
        m_hitFlags.assign(keyRows->GetNotesCount(), false);
    }

    Rectangle GetKeyRowsShape() const {
        return Rectangle{
            (float)(m_screenWidth/2 - keyRows->GetWidth()/2),
            0.0f,
            (float)keyRows->GetWidth(),
            (float)keyRows->GetHeight()
        };
    }

    Rectangle GetNoteRect(int noteRow, int yPos) const {
        int   noteWidth = keyRows->GetWidth() / 4;
        float xStart   = (float)(m_screenWidth/2 - keyRows->GetWidth()/2);
        return Rectangle{
            xStart + noteRow * noteWidth,
            (float)yPos,
            (float)noteWidth,
            (float)noteHeight
        };
    }

    void Loop() {
        if (!m_playing) return;

        double currentTime = GetElapsedTime();

        ClearBackground(BG_COLOR);
        DrawRectangleRec(GetKeyRowsShape(), ROW_COLOR);
        DrawRectangle(m_screenWidth/3, judgmentY, keyRows->GetWidth(), noteHeight, HITBAR_COLOR);
        
        int totalNotes = keyRows->GetNotesCount();

        // Rysuj wszystkie nutki w locie, które nie zostały trafione ani przegapione
        for (int i = 0; i < totalNotes; i++) {
            if (m_hitFlags[i]) continue;

            double noteTime = keyRows->GetNoteTiming(i);
            double timeToHit = noteTime - currentTime;
            if (timeToHit > approachTime || timeToHit < -200.0) continue;

            double progress = 1.0 - (timeToHit / approachTime);
            progress = std::clamp(progress, 0.0, 1.0);

            int yPos = startY + (int)(progress * travelDistance);
            int row  = keyRows->GetNoteRow(i);
            DrawRectangleRec(GetNoteRect(row, yPos), NOTE_COLOR);
        }

        // Obsługa trafiania lub przegapienia nadchodzącej nutki
        while (m_nextNoteIndex < totalNotes) {
            if (m_hitFlags[m_nextNoteIndex]) {
                m_nextNoteIndex++;
                continue;
            }

            double nt   = keyRows->GetNoteTiming(m_nextNoteIndex);
            double tt   = nt - currentTime;

            //nie trafioned
            if (tt < -200.0) {
                m_hitFlags[m_nextNoteIndex] = true;
                m_nextNoteIndex++;
                continue;
            }

            double prog = 1.0 - (tt / approachTime);
            prog = std::clamp(prog, 0.0, 1.0);

            if (prog >= 0.85) {
                int row = keyRows->GetNoteRow(m_nextNoteIndex);
                bool hit = false;
                if ((row == 0 && IsKeyPressed(KEY_A)) ||
                    (row == 1 && IsKeyPressed(KEY_S)) ||
                    (row == 2 && IsKeyPressed(KEY_K)) ||
                    (row == 3 && IsKeyPressed(KEY_L))) {
                    hit = true;
                }
                if (hit) {
                    m_score++;
                    m_hitFlags[m_nextNoteIndex] = true;
                    //PlaySound(Beep);
                    m_nextNoteIndex++;
                }
            }
            break;
        }

        DrawText(TextFormat("Score: %03d", m_score), 10, 40, 20, DARKBLUE);
    }
};

int main(){
    const int screenWidth  = 900;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "Harmon");
    InitAudioDevice();
    SetTargetFPS(240);

    Game game(screenWidth, screenHeight);
    game.Play();

    while (!WindowShouldClose()) {
        BeginDrawing();
        

        DrawText(TextFormat("Time: %08d", game.GetElapsedTime()), 10, 10, 20, BLACK);
        DrawText(TextFormat("FPS: %d", GetFPS()), 10, 70, 20, GREEN);
        game.Loop();

        EndDrawing();
    }

    CloseAudioDevice();
    CloseWindow();
    return 0;
}
