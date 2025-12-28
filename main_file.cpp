
// biblioteki
#include <iostream>
#include "raylib.h"

// wartosci dla okna
#define wysokosc_okna 720                          // 180
#define szerokosc_okna 1280                        // 480
#define nazwa_gry_wyswietlana_na_oknie "Nazwa Gry" // to zmienimy jak ustalimy fabule
#define ilosc_fps 120

// uzywam do skalowania grafiki gracza by byla mniejsza
#define skalowanie_obrazu_gracza 0.2f // 0.1f
#define skalowanie_borazu_tla 5.0f

// porusznie sie
#define prendkosc 300.0f
#define dlugosc_skoku 20 * 3
#define wysokosc_skoku 60.0f * 3

using namespace std;

int main()
{
    InitWindow(szerokosc_okna, wysokosc_okna, nazwa_gry_wyswietlana_na_oknie); // inicjuje otwarcie okna o podanych wymiarach
    SetTargetFPS(ilosc_fps);                                                   // ustala ilosc docelowa fps w oknie

    Texture2D tekstura_gracza = LoadTexture("assets/gracz_1.png"); // zapisywanie tekstury gracza z katalogu assets i pliku gracz_1

    // wartości dla postaci
    float polozenie_gracza_x = 20;
    float polozenie_gracza_y = GetScreenHeight() - tekstura_gracza.height * skalowanie_obrazu_gracza; // ustawianie go na srodku
    float ziemiaY = polozenie_gracza_y;

    Vector2 polozenie_gracza = {polozenie_gracza_x, polozenie_gracza_y}; // inicjowanie vektora 2 wymiarowego


    //zmienne zamiast int klatka_skoku
    float czas_skoku = 0.0f;
    float max_czas_skoku = 0.5f; //tutaj mozna zmieniac czas trwania skoku (w sekundach)

    // tło
    Texture2D background = LoadTexture("assets/background/background.png"); // wczytywanie tekstury tła

    float szerokosc_tla_przeskalowana = background.width * skalowanie_borazu_tla;
    float scrollingBack = 0.0f;
    float scrollSpeed = 150.00f; // predkosc przesuwania się tła w pikselach na sekunde

    while (!WindowShouldClose()) // utrzymuje okno otwarte i wykonuje polecenie wymagane przy operacji na oknach
    {
        // poruszanie sie
        float dt = GetFrameTime();

        if (IsKeyPressed(KEY_SPACE) && czas_skoku <= 0){
            czas_skoku = 0.001f; //zaczynamy skok
        }

        if (czas_skoku > 0){
            float t = czas_skoku / max_czas_skoku;
            float parabola = 4.0f * t * (1.0f - t);
            polozenie_gracza.y = ziemiaY - wysokosc_skoku * parabola;

            czas_skoku += dt; //dodajemy czas, ktory uplynal (GetFrameTime)

            if (czas_skoku >= max_czas_skoku){
                czas_skoku = 0; //koniec skoku
                polozenie_gracza.y = ziemiaY; //upewniamy sie ze postac stoi na ziemi
            }
        }

        scrollingBack -= scrollSpeed * dt; // aktualizacja pozycji tła
        if (scrollingBack <= -szerokosc_tla_przeskalowana)       //* skalowanie_borazu_tla) // resetowanie pozycji gry tło wyjdzie poza ekran
        {
            scrollingBack += szerokosc_tla_przeskalowana;
        }

        // rysowanie grafiki gracza
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // rysownanie tła w dwóch kopiach by płynnie tło cały czas się wyświetlało bez luk
        // pierwsza kopia tła
        // DrawTextureEx(background, {scrollingBack, 0}, 0.0f, skalowanie_borazu_tla, WHITE);
        // druga kopia doklejona dokładnie za pierwszą
        // DrawTextureEx(background, {scrollingBack + background.width, 0}, 0.0f, skalowanie_borazu_tla, WHITE);

        for (float x = scrollingBack; x < szerokosc_okna; x += szerokosc_tla_przeskalowana)
        {
            DrawTextureEx(background, {x, 0}, 0.0f, skalowanie_borazu_tla, WHITE);
        }

        DrawTextureEx(tekstura_gracza, polozenie_gracza, 0.0f, skalowanie_obrazu_gracza, WHITE); // rysowanie gracza w zdefiniowanej skali

        EndDrawing();
    }

    // zwalnianie pamieci
    UnloadTexture(background);
    UnloadTexture(tekstura_gracza);
    CloseWindow();

    return 0;
}
