
//biblioteki
#include <iostream> 
#include "raylib.h"

//wartosci dla okna
#define wysokosc_okna 720
#define szerokosc_okna 1280
#define nazwa_gry_wyswietlana_na_oknie "Nazwa Gry" //to zmienimy jak ustalimy fabule
#define ilosc_fps 60

//uzywam do skalowania grafiki gracza by byla mniejsza 
#define skalowanie_obrazu 0.25f

//porusznie sie
#define prendkosc 300.0f
void skakanie();

using namespace std;

int main()
{
    InitWindow(szerokosc_okna, wysokosc_okna, nazwa_gry_wyswietlana_na_oknie); //inicjuje otwarcie okna o podanych wymiarach
    SetTargetFPS(ilosc_fps); // ustala ilosc docelowa fps w oknie 

    Texture2D tekstura_gracza = LoadTexture("assets/gracz_1.png"); //zapisywanie tekstury gracza z katalogu assets i pliku gracz_1
    

    // wartości dla postaci
    float polozenie_gracza_x = 20;
    float polozenie_gracza_y = (GetScreenHeight() - tekstura_gracza.height * skalowanie_obrazu) / 2.0f; // ustawianie go na srodku 
    
    Vector2 polozenie_gracza = {polozenie_gracza_x, polozenie_gracza_y}; //inicjowanie vektora 2 wymiarowego 

    while (!WindowShouldClose()) // utrzymuje okno otwarte i wykonuje polecenie wymagane przy operacji na oknach
    {
        //poruszanie sie
        float dt = GetFrameTime();

        // RUCH: góra/dół
        if (IsKeyDown(KEY_W))
        {
            polozenie_gracza.y -= prendkosc * dt;
        }  
        if (IsKeyDown(KEY_S))
        {
            polozenie_gracza.y += prendkosc * dt;
        }
        // OGRANICZENIE DO OKNA
        float playerH = tekstura_gracza.height * skalowanie_obrazu;
        if (polozenie_gracza.y < 0)
        {
            polozenie_gracza.y = 0;
        }
        if (polozenie_gracza.y > GetScreenHeight() - playerH)
        {
          polozenie_gracza.y = GetScreenHeight() - playerH;  
        }

        //rysowanie grafiki gracza 
        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawTextureEx(tekstura_gracza, polozenie_gracza, 0.0f, skalowanie_obrazu, WHITE); //rysowanie gracza w zdefiniowanej skali

        EndDrawing();
    }

    //zwalnianie pamieci 
    UnloadTexture(tekstura_gracza);
    CloseWindow();

    return 0;
}

void skakanie()
{
    return;
}