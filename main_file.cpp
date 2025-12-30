
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
#define skalowanie_obrazu_tla 5.0f

// porusznie sie
#define prendkosc 300.0f
#define dlugosc_skoku 20 * 3
#define wysokosc_skoku 60.0f * 3

using namespace std;

//stany gry
enum StanGry {MENU, GRA, PAUZA};
StanGry aktualnyStan = MENU;

//uniwersalna funkcja przycisku by nie pisac za kazdym razem tego samego
bool CzyKliknietoPrzycisk(Rectangle obszar){
    Vector2 myszka = GetMousePosition();
    if (CheckCollisionPointRec(myszka, obszar)){
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) return true;
    }
    return false;
}
//uniwersalna funkcja sprawdzajaca czy nad przyciskienm znajduje sie myszka
bool CzyMyszkaNadPrzyciskiem(Rectangle obszar){
    Vector2 myszka = GetMousePosition();
    return CheckCollisionPointRec(GetMousePosition(), obszar);
}

int main()
{
    InitWindow(szerokosc_okna, wysokosc_okna, nazwa_gry_wyswietlana_na_oknie); // inicjuje otwarcie okna o podanych wymiarach
    SetExitKey(KEY_NULL); //wylaczamy bazowe zachowanie klawisza esc
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

    float szerokosc_tla_przeskalowana = background.width * skalowanie_obrazu_tla;
    float scrollingBack = 0.0f;
    float scrollSpeed = 150.00f; // predkosc przesuwania się tła w pikselach na sekunde

    //--- PRZYCISKI DLA MENU I PAUZY
    //przyciski menu glownego
    Rectangle btnStart = { szerokosc_okna / 2.0f - 100, 300, 200, 50};
    Rectangle btnExit = {szerokosc_okna / 2.0f - 100, 370, 200, 50};

    //przyciski pauzy
    Rectangle btnWznow = {szerokosc_okna / 2.0f - 100, 250, 200, 50};
    Rectangle btnPowrotMenu = {szerokosc_okna / 2.0f - 100, 320, 200, 50};
    Rectangle btnPauzaExit = {szerokosc_okna / 2.0f - 100, 390, 200, 50};
    
    while (!WindowShouldClose()) // utrzymuje okno otwarte i wykonuje polecenie wymagane przy operacji na oknach
    {
        //-- IF DLA POSZCZEGOLNYCH STANOW GRY
        if (aktualnyStan == MENU){
            //--- KOD MENU ---
            if (CzyKliknietoPrzycisk(btnStart)) aktualnyStan = GRA;
            if (CzyKliknietoPrzycisk(btnExit)) break; //zamykanie programu
        } 
        else if (aktualnyStan == GRA) 
        // --- KOD GRY ---
        {
            //przechodzimy do pauzy gdy kliknie ktos escape
            if (IsKeyPressed(KEY_ESCAPE)) aktualnyStan = PAUZA;

            // --- LOGIKA GRY ---

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
            if (scrollingBack <= -szerokosc_tla_przeskalowana)       //* skalowanie_obrazu_tla) // resetowanie pozycji gry tło wyjdzie poza ekran
            {
                scrollingBack += szerokosc_tla_przeskalowana;
            }
        } else if (aktualnyStan == PAUZA) {
            if (IsKeyPressed(KEY_ESCAPE)) aktualnyStan = GRA;
            if (CzyKliknietoPrzycisk(btnWznow)) aktualnyStan = GRA;
            if (CzyKliknietoPrzycisk(btnPowrotMenu)){
                aktualnyStan = MENU;
            }
            if (CzyKliknietoPrzycisk(btnPauzaExit)) break;
        }
        

        //--- RYSOWANIE DLA MENU I DLA GRY
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // -- KOLORY PRZYCISKOW
        Color kolor1btnStart = SKYBLUE;
        Color kolor2btnStart = DARKBLUE;
        Color kolor1btnMenu = GRAY;
        Color kolor2btnMenu = DARKGRAY;
        Color kolor1btnExit = RED;
        Color kolor2btnExit = DARKBROWN;

        if (aktualnyStan == MENU){
            
            //rysujemy tlo menu
            DrawTextureEx(background, {0,0}, 0.0f, skalowanie_obrazu_tla, WHITE);
            //rysujemy na tle gry zeby przyciemnic
            DrawRectangle(0, 0, szerokosc_okna, wysokosc_okna, Fade(BLACK, 0.3f));

            //napis w menu
            DrawText("TYTUL GRY", szerokosc_okna / 2-150, 100, 40, DARKGRAY);

            //rysowanie przycisku (zmienia kolor po najechaniu myszką)
            DrawRectangleRec(btnStart, CzyMyszkaNadPrzyciskiem(btnStart) ? kolor1btnStart : kolor2btnStart);
            DrawText("START", btnStart.x + 60, btnStart.y + 15, 20, WHITE);
            
            DrawRectangleRec(btnExit, CzyMyszkaNadPrzyciskiem(btnExit) ? kolor1btnExit : kolor2btnExit);
            DrawText("WYJDZ", btnExit.x + 60, btnExit.y + 15, 20, WHITE);
        } 
        else if(aktualnyStan == GRA || aktualnyStan == PAUZA) // gre rysujemy nawet jak jestesmy w pauzie
        {
            // rysownanie tła w dwóch kopiach by płynnie tło cały czas się wyświetlało bez luk
            // pierwsza kopia tła
            // DrawTextureEx(background, {scrollingBack, 0}, 0.0f, skalowanie_obrazu_tla, WHITE);
            // druga kopia doklejona dokładnie za pierwszą
            // DrawTextureEx(background, {scrollingBack + background.width, 0}, 0.0f, skalowanie_obrazu_tla, WHITE);

            for (float x = scrollingBack; x < szerokosc_okna; x += szerokosc_tla_przeskalowana)
            {
                DrawTextureEx(background, {x, 0}, 0.0f, skalowanie_obrazu_tla, WHITE);
            }

            DrawTextureEx(tekstura_gracza, polozenie_gracza, 0.0f, skalowanie_obrazu_gracza, WHITE); // rysowanie gracza w zdefiniowanej skali
            if (aktualnyStan == PAUZA){
                // Nakładamy lekki przyciemniający filtr na ekran gry
                DrawRectangle(0, 0, szerokosc_okna, wysokosc_okna, Fade(BLACK, 0.5f));
                DrawText("PAUZA", szerokosc_okna / 2 - 50, 150, 40, WHITE);

                DrawRectangleRec(btnWznow, CzyMyszkaNadPrzyciskiem(btnWznow) ? kolor1btnStart : kolor2btnStart);
                DrawText("WZNOW", btnWznow.x + 55, btnWznow.y + 15, 20, WHITE);
                
                DrawRectangleRec(btnPowrotMenu, CzyMyszkaNadPrzyciskiem(btnPowrotMenu) ? kolor1btnMenu : kolor2btnMenu);
                DrawText("MENU", btnPowrotMenu.x + 65, btnPowrotMenu.y + 15, 20, WHITE);

                DrawRectangleRec(btnPauzaExit, CzyMyszkaNadPrzyciskiem(btnPauzaExit) ? kolor1btnExit : kolor2btnExit);
                DrawText("WYJDZ", btnPauzaExit.x + 60, btnPauzaExit.y + 15, 20, WHITE);
            }
        }

        EndDrawing();
    }

    // zwalnianie pamieci
    UnloadTexture(background);
    UnloadTexture(tekstura_gracza);
    CloseWindow();

    return 0;
}
