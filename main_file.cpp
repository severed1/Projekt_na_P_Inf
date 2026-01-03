
// biblioteki
#include <iostream>
#include "raylib.h"
#include <vector>
#include <algorithm>
#include <fstream>
#include <string>
#include <cmath>
#include <random>
#include <cstdlib>

// wartosci dla okna
#define wysokosc_okna 720
#define szerokosc_okna 1280
#define nazwa_gry_wyswietlana_na_oknie "Nazwa Gry" // to zmienimy jak ustalimy fabule
#define ilosc_fps 90

// uzywam do skalowania grafiki gracza by byla mniejsza
#define skalowanie_obrazu_gracza 0.2f
#define skalowanie_obrazu_gracza_skoku 0.18f
#define skalowanie_obrazu_kaktus 0.1302f
#define skalowanie_obrazu_kaktus_wysoki 0.1302f
#define skalowanie_obrazu_tla 5.0f

// porusznie sie
#define prendkosc 300.0f
#define dlugosc_skoku 20 * 3
#define wysokosc_skoku 60.0f * 3

using namespace std;

// struktura do zapisu nicku z highscorem
struct Rekord
{
    std::string nick;
    int dystans;
};

// struktura do uporzątkowania pojawiających się przeszkod
struct przeszkoda
{
    Texture2D tekstura;
    int rodzaj_typu_przeszkody;
    Vector2 polozenie;
    bool czy_dotyka_gracza;
};

// stany gry
enum StanGry
{
    MENU,
    LEVELONE,
    LEVELTWO,
    GameOver,
    PAUZA
};
StanGry aktualnyStan = MENU;

// uniwersalna funkcja przycisku by nie pisac za kazdym razem tego samego
bool CzyKliknietoPrzycisk(Rectangle obszar)
{
    Vector2 myszka = GetMousePosition();
    if (CheckCollisionPointRec(myszka, obszar))
    {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            return true;
    }
    return false;
}
// uniwersalna funkcja sprawdzajaca czy nad przyciskienm znajduje sie myszka
bool CzyMyszkaNadPrzyciskiem(Rectangle obszar)
{
    Vector2 myszka = GetMousePosition();
    return CheckCollisionPointRec(GetMousePosition(), obszar);
}

// funkcja zapisujaca do pliku w momencie przegranej lub wyjscia do menu
void UpdateTop10(string nick, int wynik)
{
    if (nick == "")
        nick = "Sigma"; // jkak ktos nie poda nicku to ustawiamy bazowo nazwe

    vector<Rekord> tabela;

    // wczytujemy instniejace rekordy z pliku
    ifstream plikWe("top10.txt");
    Rekord r;
    while (plikWe >> r.nick >> r.dystans)
    { // przy kazdej linijce zczytujemy nick i wynik
        tabela.push_back(r);
    }
    plikWe.close();

    // dodajemy aktualny wynik gracza
    Rekord nowy;
    nowy.nick = nick;
    nowy.dystans = wynik;
    tabela.push_back(nowy);

    // sortujemy tabele od najwiekszego dystansu
    sort(tabela.begin(), tabela.end(), [](const Rekord &a, const Rekord &b)
         { return a.dystans > b.dystans; });

    // zachowujemy tylko 10 pierwszych wynikow
    if (tabela.size() > 10)
    {
        tabela.resize(10);
    }

    // zapisujemy posortowana tablice spowrotem do pliku
    ofstream plikWy("top10.txt");
    for (const auto &r : tabela)
    {
        plikWy << r.nick << " " << r.dystans << "\n";
    }
    plikWy.close();
}

// Rectangle MakeHitbox(Vector2 pos, float scale,
//                      float offX, float offY,
//                      float w, float h)
// {
//     return {
//         pos.x + offX * scale,
//         pos.y + offY * scale,
//         w * scale,
//         h * scale
//     };
// }

void rysowanie_hit_box(Rectangle hitbox, Color kolor)
{
    DrawRectangleLines( // rysowanie hit boxów bo je trzeba lekko poprawiać
        (int)hitbox.x,
        (int)hitbox.y,
        (int)hitbox.width,
        (int)hitbox.height,
        kolor);
}

int main()
{
    InitWindow(szerokosc_okna, wysokosc_okna, nazwa_gry_wyswietlana_na_oknie); // inicjuje otwarcie okna o podanych wymiarach
    SetExitKey(KEY_NULL);                                                      // wylaczamy bazowe zachowanie klawisza esc
    SetTargetFPS(ilosc_fps);                                                   // ustala ilosc docelowa fps w oknie

    random_device rd; // seed (true randomness if available)
    mt19937 gen(rd());

    Texture2D background = LoadTexture("assets/background/background.png"); // wczytywanie tekstury tła

    Texture2D tekstura_skoku = LoadTexture("assets/bohater/skok.png");

    Texture2D tekstura_gracza = LoadTexture("assets/bohater/gracz_1.png"); // zapisywanie tekstury gracza z katalogu assets i pliku gracz_1

    Texture2D texKaktus = LoadTexture("assets/przeszkody/kaktus_1.png"); // wczytywanie tekstury kaktusa

    Texture2D texKaktus_wysoki = LoadTexture("assets/przeszkody/kaktus_2.png");

    Texture2D czaszka = LoadTexture("assets/background/death.png");

    Texture2D czaszka_2 = LoadTexture("assets/background/death_2.png");

    int animacja; // nie istotne uzyte do próby animacji 
    // generowanie losowej pozycji dla kaktusow

    float rozstrzal_przy_losowaniu = 3000;
    uniform_int_distribution<int> pozycjaX(szerokosc_okna + texKaktus.width * skalowanie_obrazu_kaktus, rozstrzal_przy_losowaniu);
    uniform_int_distribution<int> pozycjaX_wysoki(szerokosc_okna + texKaktus_wysoki.width * skalowanie_obrazu_kaktus_wysoki, rozstrzal_przy_losowaniu);

    // upewnianie sie ze nie wygeneruja sie zablisko
    float minDist = texKaktus.width * skalowanie_obrazu_kaktus + tekstura_gracza.width * skalowanie_obrazu_gracza + texKaktus_wysoki.width * skalowanie_obrazu_kaktus_wysoki;

    int losowa_kaktus;
    int losowa_kaktus_wysoki;
    do
    {
        losowa_kaktus = pozycjaX(gen);
        losowa_kaktus_wysoki = pozycjaX_wysoki(gen);
    } while (abs(losowa_kaktus - losowa_kaktus_wysoki) < minDist);

    // inicjowanie kaktusa niskiego
    przeszkoda kaktus_1;
    kaktus_1.tekstura = texKaktus;
    kaktus_1.rodzaj_typu_przeszkody = 1;
    kaktus_1.czy_dotyka_gracza = false;
    kaktus_1.polozenie = {(float)losowa_kaktus, (float)wysokosc_okna - texKaktus.height * skalowanie_obrazu_kaktus + 60}; // nie mniejsze niz
    Vector2 polozenie_startowe_kaktusa = kaktus_1.polozenie;

    // inicjowanie kaktusa wysokiego
    przeszkoda kaktus_2;
    kaktus_2.tekstura = texKaktus_wysoki;
    kaktus_2.rodzaj_typu_przeszkody = 2;
    kaktus_2.czy_dotyka_gracza = false;
    kaktus_2.polozenie = {(float)losowa_kaktus_wysoki, (float)wysokosc_okna - texKaktus_wysoki.height * skalowanie_obrazu_kaktus_wysoki + 32};
    Vector2 polozenie_startowe_kaktusa_2 = kaktus_2.polozenie;

    int distance = 0; // zmienna bedzie rosla w trakcie gry jak postac biegnie i przykladowo jak dojdzie do jakiejs wartosci to pojawia sie boss

    //---ZMIENNE DO TABELI WYNIKOW I NICKU GRACZA
    char nickname[16] = "\0"; // tablica na nick (15znakow)
    int charCount = 0;
    int highScore = 0;

    // wartości dla postaci
    float polozenie_gracza_x = 20;
    float polozenie_gracza_y = GetScreenHeight() - tekstura_gracza.height * skalowanie_obrazu_gracza; // ustawianie go na dole
    float ziemiaY = polozenie_gracza_y;

    Vector2 polozenie_gracza = {polozenie_gracza_x, polozenie_gracza_y}; // inicjowanie vektora 2 wymiarowego

    // zmienne zamiast int klatka_skoku
    float czas_skoku = 0.0f;
    float max_czas_skoku = 1.0f; // tutaj mozna zmieniac czas trwania skoku (w sekundach)

    float szerokosc_tla_przeskalowana = background.width * skalowanie_obrazu_tla;
    float scrollingBack = 0.0f;
    float scrollSpeed = 150.00f; // predkosc przesuwania się tła w pikselach na sekunde

    // tworzenie hit boxów

    Rectangle Hit_box_gracza = {polozenie_gracza.x + 10 * skalowanie_obrazu_gracza, polozenie_gracza.y + 5 * skalowanie_obrazu_gracza, tekstura_gracza.width * skalowanie_obrazu_gracza, tekstura_gracza.height * skalowanie_obrazu_gracza};
    Rectangle Hit_box_gracza_w_skoku = {polozenie_gracza.x + 10 * skalowanie_obrazu_gracza, polozenie_gracza.y + 5 * skalowanie_obrazu_gracza, tekstura_gracza.width * skalowanie_obrazu_gracza - 25, tekstura_gracza.height * skalowanie_obrazu_gracza};
    Rectangle Hit_box_kaktusa = {kaktus_1.polozenie.x + 30, kaktus_1.polozenie.y + 48, kaktus_1.tekstura.width * skalowanie_obrazu_kaktus - 52, kaktus_1.tekstura.height * skalowanie_obrazu_kaktus - 50};
    Rectangle Hit_box_kaktusa_wysokiego = {kaktus_2.polozenie.x + 30, kaktus_2.polozenie.y + 48, kaktus_2.tekstura.width * skalowanie_obrazu_kaktus_wysoki - 70, kaktus_2.tekstura.height * skalowanie_obrazu_kaktus_wysoki - 50};

    // zapisywanie poczatkowych polozeni do resetu po skuciu
    Vector2 Polozenie_poczatkowe_hitbox_gracza = {Hit_box_gracza.x, Hit_box_gracza.y};
    Vector2 Polozenie_poczatkowe_hitbox_kaktusa = {Hit_box_kaktusa.x, Hit_box_kaktusa.y};
    Vector2 Polozenie_poczatkowe_hitbox_kaktusa_wysokiego = {Hit_box_kaktusa_wysokiego.x, Hit_box_kaktusa_wysokiego.y};

    //--- PRZYCISKI DLA MENU I PAUZY
    // przyciski menu glownego
    Rectangle btnStart = {szerokosc_okna / 2.0f - 100, 300, 200, 50};
    Rectangle btnExit = {szerokosc_okna / 2.0f - 100, 370, 200, 50};

    // przyciski pauzy
    Rectangle btnWznow = {szerokosc_okna / 2.0f - 100, 250, 200, 50};
    Rectangle btnPowrotMenu = {szerokosc_okna / 2.0f - 100, 320, 200, 50};
    Rectangle btnPauzaExit = {szerokosc_okna / 2.0f - 100, 390, 200, 50};

    while (!WindowShouldClose()) // utrzymuje okno otwarte i wykonuje polecenie wymagane przy operacji na oknach
    {
        //-- IF DLA POSZCZEGOLNYCH STANOW GRY
        if (aktualnyStan == MENU)
        {
            //--- KOD MENU ---
            if (CzyKliknietoPrzycisk(btnStart) || IsKeyPressed(KEY_ENTER))
                aktualnyStan = LEVELONE;
            if (CzyKliknietoPrzycisk(btnExit))
                break; // zamykanie programu

            // wczytywanie wpisywanego nicku
            int klawisz = GetCharPressed();
            while (klawisz > 0)
            {
                if ((klawisz >= 32) && (klawisz <= 125) && (charCount < 15))
                {
                    nickname[charCount] = (char)klawisz;
                    nickname[charCount + 1] = '\0';
                    charCount++;
                }
                klawisz = GetCharPressed();
            }

            // usuwanie znaku przy kliknieciu backspace
            if (IsKeyPressed(KEY_BACKSPACE) && charCount > 0)
            {
                charCount--;
                nickname[charCount] = '\0';
            }
        }
        else if (aktualnyStan == LEVELONE)
        // --- KOD GRY ---
        {
            distance++;
            // przechodzimy do pauzy gdy kliknie ktos escape
            if (IsKeyPressed(KEY_ESCAPE))
                aktualnyStan = PAUZA;

            // --- LOGIKA GRY ---

            // poruszanie sie
            float dt = GetFrameTime();

            if (IsKeyDown(KEY_SPACE) && czas_skoku <= 0 || IsKeyDown(KEY_W) && czas_skoku <= 0 || IsKeyDown(KEY_UP) && czas_skoku <= 0)
            {
                czas_skoku = 0.001f; // zaczynamy skok
            }

            if (czas_skoku > 0)
            {
                float t = czas_skoku / max_czas_skoku;
                float parabola = 6.0f * t * (1.0f - t);
                polozenie_gracza.y = ziemiaY - wysokosc_skoku * parabola;
                Hit_box_gracza.y = ziemiaY - wysokosc_skoku * parabola;
                Hit_box_gracza_w_skoku.y = ziemiaY - wysokosc_skoku * parabola;
                czas_skoku += dt; // dodajemy czas, ktory uplynal (GetFrameTime)

                if (czas_skoku >= max_czas_skoku)
                {
                    czas_skoku = 0;               // koniec skoku
                    polozenie_gracza.y = ziemiaY; // upewniamy sie ze postac stoi na ziemi
                    Hit_box_gracza.y = ziemiaY;
                    Hit_box_gracza_w_skoku.y = ziemiaY - wysokosc_skoku * parabola;
                }
            }

            scrollingBack -= scrollSpeed * dt;                 // aktualizacja pozycji tła
            if (scrollingBack <= -szerokosc_tla_przeskalowana) //* skalowanie_obrazu_tla) // resetowanie pozycji gry tło wyjdzie poza ekran
            {
                scrollingBack += szerokosc_tla_przeskalowana;
            }

            float game_speed = 5 + log(1 + distance / 300); // zwiekszanie sie game speedu logarytmicznie zeby nie zaszybko sie zmieniało w predkosc swiatla

            kaktus_1.polozenie.x -= game_speed; // przyspieszanie przeszkody w zaleznosci od game speedu
            Hit_box_kaktusa.x -= game_speed;

            kaktus_2.polozenie.x -= game_speed;
            Hit_box_kaktusa_wysokiego.x -= game_speed;

            rozstrzal_przy_losowaniu += game_speed / 100;
            minDist += game_speed / 100;

            uniform_int_distribution<int> pozycjaX(szerokosc_okna + texKaktus.width * skalowanie_obrazu_kaktus, rozstrzal_przy_losowaniu);
            uniform_int_distribution<int> pozycjaX_wysoki(szerokosc_okna + texKaktus_wysoki.width * skalowanie_obrazu_kaktus_wysoki, rozstrzal_przy_losowaniu);

            // upewnianie sie ze nie wygeneruja sie zablisko
            if (kaktus_1.polozenie.x < -texKaktus.width * skalowanie_obrazu_kaktus)
            {
                int x;
                do
                {
                    x = pozycjaX(gen);
                } while (abs(x - (int)kaktus_2.polozenie.x) < minDist);

                kaktus_1.polozenie.x = (float)x;
                Hit_box_kaktusa.x = kaktus_1.polozenie.x + 30;
            }

            if (kaktus_2.polozenie.x < -texKaktus_wysoki.width * skalowanie_obrazu_kaktus)
            {
                int x;
                do
                {
                    x = pozycjaX_wysoki(gen);
                } while (abs(x - (int)kaktus_1.polozenie.x) < minDist);

                kaktus_2.polozenie.x = (float)x;
                Hit_box_kaktusa_wysokiego.x = kaktus_2.polozenie.x + 30;
            }

            max_czas_skoku -= log(2 + distance / 1000) / 100000; // w teori powinno uniknąc to problemów przy zaduzej predkosci przeszkud ale moze to usune potem

            if (czas_skoku == 0)
            {
                if (CheckCollisionRecs(Hit_box_gracza, Hit_box_kaktusa) || CheckCollisionRecs(Hit_box_gracza, Hit_box_kaktusa_wysokiego))
                {
                    aktualnyStan = GameOver;
                }
            }
            else 
            {
                if (CheckCollisionRecs(Hit_box_gracza_w_skoku, Hit_box_kaktusa) || CheckCollisionRecs(Hit_box_gracza, Hit_box_kaktusa_wysokiego))
                {
                    aktualnyStan = GameOver;
                }
            }
        }
        else if (aktualnyStan == PAUZA)
        {
            if (IsKeyPressed(KEY_ESCAPE))
                aktualnyStan = LEVELONE;
            if (CzyKliknietoPrzycisk(btnWznow))
                aktualnyStan = LEVELONE;
            if (CzyKliknietoPrzycisk(btnPowrotMenu))
            {
                UpdateTop10(nickname, distance);
                distance = 0;
                aktualnyStan = MENU;
            }
            if (CzyKliknietoPrzycisk(btnPauzaExit))
            {
                UpdateTop10(nickname, distance);
                break;
            }
        }
        else if (aktualnyStan == GameOver)
        {
            animacja ++;
            if (CzyKliknietoPrzycisk(btnPowrotMenu))
            {
                
                UpdateTop10(nickname, distance);
                distance = 0;
                kaktus_1.polozenie = polozenie_startowe_kaktusa;
                Hit_box_kaktusa.x = Polozenie_poczatkowe_hitbox_kaktusa.x;

                kaktus_2.polozenie = polozenie_startowe_kaktusa_2;
                Hit_box_kaktusa_wysokiego.x = Polozenie_poczatkowe_hitbox_kaktusa_wysokiego.x;

                polozenie_gracza.y = ziemiaY;
                Hit_box_gracza.y = Polozenie_poczatkowe_hitbox_gracza.y;
                aktualnyStan = MENU;
            }
            if (CzyKliknietoPrzycisk(btnPauzaExit))
            {
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

        if (aktualnyStan == MENU)
        {

            // rysujemy tlo menu
            DrawTextureEx(background, {0, 0}, 0.0f, skalowanie_obrazu_tla, WHITE);
            // rysujemy na tle gry zeby przyciemnic
            DrawRectangle(0, 0, szerokosc_okna, wysokosc_okna, Fade(BLACK, 0.3f));

            // napis w menu
            DrawText("TYTUL GRY", szerokosc_okna / 2 - 150, 100, 40, DARKGRAY);

            // Pole na wpisywanie nicku
            DrawText("WPISZ SWOJ NICK: ", szerokosc_okna / 2 - 100, 200, 20, DARKGRAY);
            DrawRectangle(szerokosc_okna / 2 - 105, 230, 210, 40, LIGHTGRAY); // pole tekstowe
            DrawText(nickname, szerokosc_okna / 2 - 100, 240, 20, BLACK);

            // rysowanie przycisku (zmienia kolor po najechaniu myszką)
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

            while (plikPokaz >> n >> d && yOffset < 10)
            {
                DrawText(TextFormat("%d. %s - %d", yOffset + 1, n.c_str(), d), 50, 80 + (yOffset * 25), 18, DARKGRAY);
                yOffset++;
            }
            plikPokaz.close();
        }
        else if (aktualnyStan == LEVELONE || aktualnyStan == PAUZA || aktualnyStan == GameOver) // gre rysujemy nawet jak jestesmy w pauzie
        {

            for (float x = scrollingBack; x < szerokosc_okna; x += szerokosc_tla_przeskalowana)
            {
                DrawTextureEx(background, {x, 0}, 0.0f, skalowanie_obrazu_tla, WHITE);
            }

            // if (czas_skoku == 0)
            // {
            //     rysowanie_hit_box(Hit_box_gracza, RED);
            // }
            // else
            // {
            //     rysowanie_hit_box(Hit_box_gracza_w_skoku, RED);
            // }
            // rysowanie_hit_box(Hit_box_kaktusa, GREEN);
            // rysowanie_hit_box(Hit_box_kaktusa_wysokiego, GREEN);
            //
            //istotne prosze nie usuwać !!!!

            DrawTextureEx(kaktus_2.tekstura, kaktus_2.polozenie, 0.25f, skalowanie_obrazu_kaktus, WHITE);
            DrawTextureEx(kaktus_1.tekstura, kaktus_1.polozenie, 0.0f, skalowanie_obrazu_kaktus, WHITE); // rysowanie przeszkody w zdefiniowanej skali
            if (czas_skoku == 0)
            {
                DrawTextureEx(tekstura_gracza, polozenie_gracza, 0.0f, skalowanie_obrazu_gracza, WHITE);
            }
            else
            {
                DrawTextureEx(tekstura_skoku, {polozenie_gracza.x - 40, polozenie_gracza.y - 60}, 0.0f, skalowanie_obrazu_gracza_skoku, WHITE);
            }

            DrawText("POZIOM 1", 10, 10, 20, GRAY);
            DrawText(TextFormat("DYSTANS: %05d", distance), szerokosc_okna - 200, 10, 20, GRAY);
            if (aktualnyStan == PAUZA)
            {
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
            else if (aktualnyStan == GameOver)
            {

                DrawRectangle(0, 0, szerokosc_okna, wysokosc_okna, Fade(BLACK, 0.5f));
                DrawText("GAME OVER", szerokosc_okna / 2 - 150, 200, 50, BLACK);

                int frameCounter = (animacja + 1) %30;

                if (frameCounter < 15)
                {
                    DrawTextureEx(czaszka, {szerokosc_okna/2 - 105, 420}, 0.0f, 0.2, GRAY);
                }
                else
                {
                     DrawTextureEx(czaszka_2, {szerokosc_okna/2 - 105, 420}, 0.0f, 0.2, GRAY);
                }
                
                
                DrawText(TextFormat("KONCOWY DYSTANS: %05d", distance), szerokosc_okna / 2 - 300, 260, 50, BLACK);

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
    UnloadTexture(texKaktus);
    CloseWindow();

    return 0;
}
