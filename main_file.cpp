
// biblioteki
#include <iostream>
#include "raylib.h"
#include <vector>
#include <algorithm>
#include <fstream>
#include <string>

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

//struktura do zapisu nicku z highscorem
struct Rekord {
    std::string nick;
    int dystans;
};

//stany gry
enum StanGry {MENU, LEVELONE, LEVELTWO ,PAUZA};
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

//funkcja zapisujaca do pliku w momencie przegranej lub wyjscia do menu
void UpdateTop10(string nick, int wynik){
    if (nick == "") nick = "Sigma"; //jkak ktos nie poda nicku to ustawiamy bazowo nazwe

    vector<Rekord> tabela;

    //wczytujemy instniejace rekordy z pliku 
    ifstream plikWe("top10.txt");
    Rekord r;
    while (plikWe >> r.nick >> r.dystans){ //przy kazdej linijce zczytujemy nick i wynik
        tabela.push_back(r);
    }
    plikWe.close();

    //dodajemy aktualny wynik gracza
    Rekord nowy;
    nowy.nick = nick;
    nowy.dystans = wynik;
    tabela.push_back(nowy);
    
    //sortujemy tabele od najwiekszego dystansu
    sort(tabela.begin(), tabela.end(), [](const Rekord& a, const Rekord& b){
        return a.dystans > b.dystans;
    });

    //zachowujemy tylko 10 pierwszych wynikow
    if(tabela.size() >10){
        tabela.resize(10);
    }

    //zapisujemy posortowana tablice spowrotem do pliku
    ofstream plikWy("top10.txt");
    for(const auto& r: tabela){
        plikWy << r.nick << " " << r.dystans << "\n";
    }
    plikWy.close();
}

int main()
{
    InitWindow(szerokosc_okna, wysokosc_okna, nazwa_gry_wyswietlana_na_oknie); // inicjuje otwarcie okna o podanych wymiarach
    SetExitKey(KEY_NULL); //wylaczamy bazowe zachowanie klawisza esc
    SetTargetFPS(ilosc_fps);                                                   // ustala ilosc docelowa fps w oknie

    Texture2D tekstura_gracza = LoadTexture("assets/gracz_1.png"); // zapisywanie tekstury gracza z katalogu assets i pliku gracz_1

    int distance; // zmienna bedzie rosla w trakcie gry jak postac biegnie i przykladowo jak dojdzie do jakiejs wartosci to pojawia sie boss

    //---ZMIENNE DO TABELI WYNIKOW I NICKU GRACZA
    char nickname[16] = "\0"; // tablica na nick (15znakow)
    int charCount = 0; 
    int highScore = 0;

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
            if (CzyKliknietoPrzycisk(btnStart) || IsKeyPressed(KEY_ENTER)) aktualnyStan = LEVELONE;
            if (CzyKliknietoPrzycisk(btnExit)) break; //zamykanie programu

            //wczytywanie wpisywanego nicku
            int klawisz = GetCharPressed();
            while (klawisz > 0){
                if ((klawisz >= 32) && (klawisz <=125) && (charCount < 15)){
                    nickname[charCount] = (char)klawisz;
                    nickname[charCount + 1] = '\0';
                    charCount++;
                }
                klawisz = GetCharPressed();
            }

            //usuwanie znaku przy kliknieciu backspace
            if(IsKeyPressed(KEY_BACKSPACE) && charCount > 0){
                charCount--;
                nickname[charCount] = '\0';
            }
        } 
        else if (aktualnyStan == LEVELONE) 
        // --- KOD GRY ---
        {
            distance++;
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
            if (IsKeyPressed(KEY_ESCAPE)) aktualnyStan = LEVELONE;
            if (CzyKliknietoPrzycisk(btnWznow)) aktualnyStan = LEVELONE;
            if (CzyKliknietoPrzycisk(btnPowrotMenu)){
                UpdateTop10(nickname, distance);
                distance = 0;
                aktualnyStan = MENU;
            }
            if (CzyKliknietoPrzycisk(btnPauzaExit)){
                UpdateTop10(nickname, distance);
                break;
            }
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

            //Pole na wpisywanie nicku
            DrawText("WPISZ SWOJ NICK: ", szerokosc_okna/2-100, 200, 20, DARKGRAY);
            DrawRectangle(szerokosc_okna/2-105, 230, 210, 40, LIGHTGRAY); //pole tekstowe
            DrawText(nickname, szerokosc_okna/2-100, 240, 20, BLACK);

            //rysowanie przycisku (zmienia kolor po najechaniu myszką)
            DrawRectangleRec(btnStart, CzyMyszkaNadPrzyciskiem(btnStart) ? kolor1btnStart : kolor2btnStart);
            DrawText("START", btnStart.x + 60, btnStart.y + 15, 20, WHITE);
            
            DrawRectangleRec(btnExit, CzyMyszkaNadPrzyciskiem(btnExit) ? kolor1btnExit : kolor2btnExit);
            DrawText("WYJDZ", btnExit.x + 60, btnExit.y + 15, 20, WHITE);

            //--RYSOWANIE LEADERBOARD
            ifstream plikPokaz("top10.txt");
            string n;
            int d;
            int yOffset = 0;

            DrawText("TABELA TOP 10: ", 50, 50, 20, GOLD);

            while (plikPokaz >> n >> d && yOffset < 10){
                DrawText(TextFormat("%d. %s - %d", yOffset + 1, n.c_str(), d), 50, 80 + (yOffset * 25), 18, DARKGRAY);
                yOffset++;
            }
            plikPokaz.close();
        } 
        else if(aktualnyStan == LEVELONE || aktualnyStan == PAUZA) // gre rysujemy nawet jak jestesmy w pauzie
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
            DrawText("POZIOM 1", 10, 10, 20, GRAY);
            DrawText(TextFormat("DYSTANS: %05d", distance), szerokosc_okna-200, 10, 20, GRAY);
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
