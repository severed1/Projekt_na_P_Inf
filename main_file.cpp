
// biblioteki
#include <iostream>
#include "raylib.h"

// wartosci dla okna
#define wysokosc_okna 180
#define szerokosc_okna 480
#define nazwa_gry_wyswietlana_na_oknie "Nazwa Gry" // to zmienimy jak ustalimy fabule
#define ilosc_fps 60

// uzywam do skalowania grafiki gracza by byla mniejsza
#define skalowanie_obrazu 0.1f

// porusznie sie
#define prendkosc 300.0f
#define wysokosc_skoku 60

// prototypy funkcji
float skakanie(float polozenie, int liczba_klatek, float dt);

using namespace std;

int main()
{
    InitWindow(szerokosc_okna, wysokosc_okna, nazwa_gry_wyswietlana_na_oknie); // inicjuje otwarcie okna o podanych wymiarach
    SetTargetFPS(ilosc_fps);                                                   // ustala ilosc docelowa fps w oknie

    Texture2D tekstura_gracza = LoadTexture("assets/gracz_1.png"); // zapisywanie tekstury gracza z katalogu assets i pliku gracz_1

    // wartości dla postaci
    float polozenie_gracza_x = 20;
    float polozenie_gracza_y = (GetScreenHeight() - tekstura_gracza.height * skalowanie_obrazu) / 2.0f; // ustawianie go na srodku

    Vector2 polozenie_gracza = {polozenie_gracza_x, polozenie_gracza_y}; // inicjowanie vektora 2 wymiarowego

    int klatka_skoku = -1;

    // tło
    Texture2D background = LoadTexture("assets/background/background.png"); // wczytywanie tekstury tła

    float scrollingBack = 0.0f;
    float scrollSpeed = 150.00f; // predkosc przesuwania się tła w pikselach na sekunde

    while (!WindowShouldClose()) // utrzymuje okno otwarte i wykonuje polecenie wymagane przy operacji na oknach
    {
        // poruszanie sie
        float dt = GetFrameTime();

        scrollingBack -= scrollSpeed * GetFrameTime(); // aktualizacja pozycji tła
        if (scrollingBack <= -background.width)        // resetowanie pozycji gry tło wyjdzie poza ekran
        {
            scrollingBack = 0;
        }

        if (klatka_skoku >= wysokosc_skoku)
        {
            klatka_skoku = -1;
        }

        if (IsKeyPressed(KEY_SPACE) && klatka_skoku == -1)
        {
            klatka_skoku = 0;
        }

        if (klatka_skoku != -1)
        {
            polozenie_gracza.y = skakanie(polozenie_gracza.y, klatka_skoku, dt);
            klatka_skoku++;
        }

        // rysowanie grafiki gracza
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // rysownanie tła w dwóch kopiach by płynnie tło cały czas się wyświetlało bez luk
        // pierwsza kopia tła
        DrawTextureEx(background, {scrollingBack, 0}, 0.0f, 1.0f, WHITE);
        // druga kopia doklejona dokładnie za pierwszą
        DrawTextureEx(background, {scrollingBack + background.width, 0}, 0.0f, 1.0f, WHITE);

        DrawTextureEx(tekstura_gracza, polozenie_gracza, 0.0f, skalowanie_obrazu, WHITE); // rysowanie gracza w zdefiniowanej skali

        EndDrawing();
    }

    // zwalnianie pamieci
    UnloadTexture(tekstura_gracza);
    CloseWindow();

    return 0;
}

float skakanie(float polozenie, int liczba_klatek, float dt)
{
    float polozenie_koncowe = polozenie;

    if (liczba_klatek <= (wysokosc_skoku / 2))
    {
        polozenie_koncowe -= prendkosc * dt;
    }
    else if (liczba_klatek > (wysokosc_skoku / 2))
    {
        polozenie_koncowe += prendkosc * dt;
    }
    return polozenie_koncowe;
}
