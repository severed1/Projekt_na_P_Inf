
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
#define ilosc_fps 120

// uzywam do skalowania grafiki gracza by byla mniejsza
#define skalowanie_obrazu_gracza 0.23f
#define skalowanie_obrazu_gracza_skoku 0.23f
#define skalowanie_obrazu_szkieleta 0.27f
#define skalowanie_obrazu_duch 0.05f
#define skalowanie_obrazu_tla 1.2 // 1.2 //3
#define skalowanie_obrazu_tla_menu 0.84
#define skalowanie_obrazu_szczura 0.2f
#define skalowanie_obrazu_bat 0.14f
#define skalowanie_obrazu_boss 0.7f

// porusznie sie
#define prendkosc 300.0f
#define dlugosc_skoku (20 * 3)
#define wysokosc_skoku (60.0f * 3)

#define offsetYducha_1 (duch.polozenie.y + 40)

#define offsetYducha_2 (duch.polozenie.y + 50)
#define offsetXducha_2 (duch.polozenie.x + 80)

#define offsetYducha_3 (duch.polozenie.y + 80)
#define offsetXducha_3 (duch.polozenie.x + 140)

#define offsetYducha_4 (duch.polozenie.y + 120)
#define offsetXducha_4 (duch.polozenie.x + 210)

#define offsetYszczura (szczur.polozenie.y + 60)
#define offsetXszczura (szczur.polozenie.x + 70)

#define offsetYbat (bat.polozenie.y + 45)
#define offsetXbat (bat.polozenie.x + 70)

// prock boss_tekstura fighta
#define boss_fight_distance 4000
#define co_ile_laser 2000
#define wind_up_laseru 300
#define czas_trwania_laseru 400

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
    bool czy_jest_zatrzymany;
};

struct boss
{
    Texture2D tekstury[3];
    Vector2 polozenie;
    bool laser_on;
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

static bool TooCloseX(int x, float otherX, int minDist)
{
    return std::abs(x - (int)otherX) < minDist;
}

static bool TooCloseForSzkielet(int x, int minDist,
                                const Vector2 &duchPos,
                                const Vector2 &batPos,
                                const Vector2 &szczurPos)
{
    return TooCloseX(x, duchPos.x, minDist) ||
           TooCloseX(x, batPos.x, minDist) ||
           TooCloseX(x, szczurPos.x, minDist);
}

static bool TooCloseForDuch(int x, int minDist,
                            const Vector2 &szkieletPos,
                            const Vector2 &batPos,
                            const Vector2 &szczurPos)
{
    return TooCloseX(x, szkieletPos.x, minDist) ||
           TooCloseX(x, batPos.x, minDist) ||
           TooCloseX(x, szczurPos.x, minDist);
}

static bool TooCloseForBat(int x, int minDist,
                           const Vector2 &duchPos,
                           const Vector2 &szkieletPos,
                           const Vector2 &szczurPos)
{
    return TooCloseX(x, duchPos.x, minDist) ||
           TooCloseX(x, szkieletPos.x, minDist) ||
           TooCloseX(x, szczurPos.x, minDist);
}

static bool TooCloseForSzczur(int x, int minDist,
                              const Vector2 &duchPos,
                              const Vector2 &szkieletPos,
                              const Vector2 &batPos)
{
    return TooCloseX(x, duchPos.x, minDist) ||
           TooCloseX(x, szkieletPos.x, minDist) ||
           TooCloseX(x, batPos.x, minDist);
}

int main()
{
    InitWindow(szerokosc_okna, wysokosc_okna, nazwa_gry_wyswietlana_na_oknie); // inicjuje otwarcie okna o podanych wymiarach
    SetExitKey(KEY_NULL);                                                      // wylaczamy bazowe zachowanie klawisza esc
    SetTargetFPS(ilosc_fps);                                                   // ustala ilosc docelowa fps w oknie

    random_device rd;
    mt19937 gen(rd());

    Texture2D background = LoadTexture("assets/background/background_3.png"); // wczytywanie tekstury tła

    Texture2D tekstura_skoku = LoadTexture("assets/bohater/skok.png");

    Texture2D tekstura_gracza_1 = LoadTexture("assets/bohater/rycez_1.png"); // zapisywanie tekstury gracza z katalogu assets i pliku gracz_1
    Texture2D tekstura_gracza_2 = LoadTexture("assets/bohater/rycez_2.png");
    Texture2D tekstura_gracza_3 = LoadTexture("assets/bohater/rycez_3.png");

    Texture2D szkielet_tekstura = LoadTexture("assets/przeszkody/szkielet.png"); // wczytywanie tekstury szkieleta
    Texture2D szkielet_tekstura_2 = LoadTexture("assets/przeszkody/skeleton_2.png");
    Texture2D szkielet_tekstura_3 = LoadTexture("assets/przeszkody/szkielet_3.png");

    Texture2D duch_tekstura = LoadTexture("assets/przeszkody/duch.png");

    Texture2D czaszka = LoadTexture("assets/background/death.png");

    Texture2D czaszka_2 = LoadTexture("assets/background/death_2.png");

    Texture2D slizg = LoadTexture("assets/bohater/slizg.png");

    Texture2D background_menu = LoadTexture("assets/background/background_menu.png");

    Texture2D background_las = LoadTexture("assets/backgournd/las.png");

    Texture2D szczur_tekstura = LoadTexture("assets/przeszkody/szczur.png");

    Texture2D bat_tekstura = LoadTexture("assets/przeszkody/bat.png");
    Texture2D bat_tekstura_2 = LoadTexture("assets/przeszkody/bat_2.png");
    Texture2D bat_tekstura_3 = LoadTexture("assets/przeszkody/bat_3.png");

    Texture2D boss_tekstura = LoadTexture("assets/Boss/boss.png");
    Texture2D boss_tekstura_laser_1 = LoadTexture("assets/Boss/boss_laser_1.png");
    Texture2D boss_tekstura_laser_2 = LoadTexture("assets/Boss/boss_laser_2.png");

    Texture2D napis_o_bossie = LoadTexture("assets/background/napis.png");

    int animacja = 0; // nie istotne uzyte do próby animacji
    // generowanie losowej pozycji dla kaktusow

    float rozstrzal_przy_losowaniu = 9000;
    float rozstrzal_przy_losowaniu_czestych = 5000;
    // upewnianie sie ze nie wygeneruja sie zablisko
    float minDist = 500;
    uniform_int_distribution<int> pozycjaX(szerokosc_okna + szkielet_tekstura.width * skalowanie_obrazu_szkieleta, rozstrzal_przy_losowaniu);
    uniform_int_distribution<int> pozycjaX_wysoki(szerokosc_okna + duch_tekstura.width * skalowanie_obrazu_duch, rozstrzal_przy_losowaniu);
    uniform_int_distribution<int> pozycjay_wysoki(300, (float)wysokosc_okna - szkielet_tekstura.height * skalowanie_obrazu_szkieleta + 120);
    uniform_int_distribution<int> pozycja_czenste(szerokosc_okna + szkielet_tekstura.width * skalowanie_obrazu_szkieleta, rozstrzal_przy_losowaniu_czestych);

    // sprawdza czy w trakcie boss_tekstura fightu
    bool w_trakcie_bossa = false;

    int losowa_szkielet;
    int losowa_duch;
    int losowa_szczura;
    int losowa_bat;
    do
    {
        losowa_bat = pozycja_czenste(gen);
        losowa_szczura = pozycja_czenste(gen);
        losowa_szkielet = pozycjaX(gen);
        losowa_duch = pozycjaX_wysoki(gen);
    } while (abs(losowa_szkielet - losowa_duch) < minDist || abs(losowa_szkielet - losowa_szczura) < minDist || abs(losowa_szkielet - losowa_bat) < minDist || abs(losowa_bat - losowa_szczura) < minDist || abs(losowa_duch - losowa_bat) < minDist || abs(losowa_duch - losowa_szczura) < minDist);

    // inicjowanie kaktusa niskiego
    przeszkoda szkielet;
    szkielet.tekstura = szkielet_tekstura;
    szkielet.rodzaj_typu_przeszkody = 1;
    szkielet.czy_jest_zatrzymany = false;
    szkielet.polozenie = {(float)losowa_szkielet, (float)wysokosc_okna - szkielet_tekstura.height * skalowanie_obrazu_szkieleta + 40};
    Vector2 polozenie_startowe_szkieleta = szkielet.polozenie;

    // inicjowanie kaktusa wysokiego
    przeszkoda duch;
    duch.tekstura = duch_tekstura;
    duch.rodzaj_typu_przeszkody = 2;
    duch.czy_jest_zatrzymany = false;
    duch.polozenie = {(float)losowa_duch, (float)pozycjay_wysoki(gen)};
    Vector2 polozenie_startowe_ducha = duch.polozenie;

    przeszkoda szczur;
    szczur.tekstura = szczur_tekstura;
    szczur.rodzaj_typu_przeszkody = 3;
    szczur.polozenie = {(float)losowa_szczura, (float)wysokosc_okna - szczur_tekstura.height * skalowanie_obrazu_szczura};
    Vector2 polozenie_startowe_szczura = szczur.polozenie;

    przeszkoda bat;
    bat.tekstura = bat_tekstura;
    bat.rodzaj_typu_przeszkody = 4;
    bat.polozenie = {(float)losowa_bat, (float)pozycjay_wysoki(gen)};
    Vector2 polozenie_startowe_bat = bat.polozenie;

    boss czaszka_boss;
    czaszka_boss.tekstury[0] = boss_tekstura;
    czaszka_boss.tekstury[1] = boss_tekstura_laser_1;
    czaszka_boss.tekstury[2] = boss_tekstura_laser_2;
    czaszka_boss.polozenie = {szerokosc_okna / 2 + 200, wysokosc_okna / 2 - 500};
    czaszka_boss.laser_on = false;

    Vector2 czaszka_boss_pozycja_poczontokowa = {czaszka_boss.polozenie.x, czaszka_boss.polozenie.y};

    Vector2 polozenie_napisu = {szerokosc_okna/2- 350, (float)-napis_o_bossie.height};

    int distance = 0; // zmienna bedzie rosla w trakcie gry jak postac biegnie i przykladowo jak dojdzie do jakiejs wartosci to pojawia sie boss_tekstura

    //---ZMIENNE DO TABELI WYNIKOW I NICKU GRACZA
    char nickname[16] = "\0"; // tablica na nick (15znakow)
    int charCount = 0;
    int highScore = 0;

    // wartości dla postaci
    float polozenie_gracza_x = 20;
    float polozenie_gracza_y = GetScreenHeight() - tekstura_gracza_1.height * skalowanie_obrazu_gracza - 60; // ustawianie go na dole
    float ziemiaY = polozenie_gracza_y;

    Vector2 polozenie_gracza = {polozenie_gracza_x, polozenie_gracza_y}; // inicjowanie vektora 2 wymiarowego

    // zmienne zamiast int klatka_skoku
    float czas_skoku = 0.0f;
    float max_czas_skoku = 1.0f; // tutaj mozna zmieniac czas trwania skoku (w sekundach)

    float szerokosc_tla_przeskalowana = background.width * skalowanie_obrazu_tla;
    float scrollingBack = 0.0f;
    float scrollSpeed = 150.00f; // predkosc przesuwania się tła w pikselach na sekunde

    // tworzenie hit boxów

    Rectangle Hit_box_gracza = {polozenie_gracza.x + 520 * skalowanie_obrazu_gracza, polozenie_gracza.y + 100 * skalowanie_obrazu_gracza, tekstura_gracza_1.width * skalowanie_obrazu_gracza - 250, tekstura_gracza_1.height * skalowanie_obrazu_gracza - 50};
    Rectangle Hit_box_gracza_w_skoku = {polozenie_gracza.x + 350 * skalowanie_obrazu_gracza, polozenie_gracza.y - 10, tekstura_gracza_1.width * skalowanie_obrazu_gracza - 230, tekstura_gracza_1.height * skalowanie_obrazu_gracza - 100};
    Rectangle Hit_box_gracza_slizg = {polozenie_gracza.x + 150 * skalowanie_obrazu_gracza, polozenie_gracza.y + 600 * skalowanie_obrazu_gracza, slizg.width * skalowanie_obrazu_gracza - 100, slizg.height * skalowanie_obrazu_gracza - 140};

    Rectangle Hit_box_szkieleta = {szkielet.polozenie.x + 30, szkielet.polozenie.y + 48, szkielet.tekstura.width * skalowanie_obrazu_szkieleta - 360, szkielet.tekstura.height * skalowanie_obrazu_szkieleta - 300};
    Rectangle Hit_box_szkieleta_nogi = {szkielet.polozenie.x + 30, szkielet.polozenie.y + 48, szkielet.tekstura.width * skalowanie_obrazu_szkieleta - 320, szkielet.tekstura.height * skalowanie_obrazu_szkieleta - 300};

    Rectangle Hit_box_ducha_1 = {duch.polozenie.x + 30, offsetYducha_1, duch.tekstura.height * skalowanie_obrazu_duch, duch.tekstura.width * skalowanie_obrazu_duch - 60};
    Rectangle Hit_box_ducha_2 = {offsetXducha_2, offsetYducha_2, duch.tekstura.height * skalowanie_obrazu_duch + 30, duch.tekstura.width * skalowanie_obrazu_duch - 35};
    Rectangle Hit_box_ducha_3 = {offsetXducha_3, offsetYducha_3, duch.tekstura.height * skalowanie_obrazu_duch + 80, duch.tekstura.width * skalowanie_obrazu_duch - 25};
    Rectangle Hit_box_ducha_4 = {offsetXducha_4, offsetYducha_4, duch.tekstura.height * skalowanie_obrazu_duch + 70, duch.tekstura.width * skalowanie_obrazu_duch - 40};

    Rectangle Hit_box_bat = {offsetXbat, offsetYbat, bat.tekstura.width * skalowanie_obrazu_duch, bat.tekstura.height * skalowanie_obrazu_duch};

    Rectangle Hit_box_szczura = {offsetXszczura, offsetYszczura, szczur.tekstura.height * skalowanie_obrazu_szczura - 40, (szczur.tekstura.width * skalowanie_obrazu_szczura) - 250};

    // potrzebne zeby sie nie ruszal po smierci
    bool IsDead = false;
    double game_speed_hamowanie[3] = {5 + log(1 + distance / 300)}; // zwiekszanie sie game speedu logarytmicznie zeby nie zaszybko sie zmieniało w predkosc swiatla

    // zapisywanie poczatkowych polozeni do resetu po skuciu
    Vector2 Polozenie_poczatkowe_hitbox_gracza = {Hit_box_gracza.x, Hit_box_gracza.y};
    Vector2 Polozenie_poczatkowe_hitbox_gracza_w_skoku = {Hit_box_gracza_w_skoku.x, Hit_box_gracza_w_skoku.y};
    Vector2 Polozenie_poczatkowe_hitbox_szkieleta = {Hit_box_szkieleta.x, Hit_box_szkieleta.y};
    Vector2 Polozenie_poczatkowe_hitbox_ducha_1 = {Hit_box_ducha_1.x, Hit_box_ducha_1.y};
    Vector2 Polozenie_poczatkowe_hitbox_ducha_2 = {Hit_box_ducha_2.x, Hit_box_ducha_2.y};
    Vector2 Polozenie_poczatkowe_hitbox_ducha_3 = {Hit_box_ducha_3.x, Hit_box_ducha_3.y};
    Vector2 Polozenie_poczatkowe_hitbox_ducha_4 = {Hit_box_ducha_4.x, Hit_box_ducha_4.y};

    Vector2 Polozenie_poczatkowe_hitbox_bat = {Hit_box_bat.x, Hit_box_bat.y};
    Vector2 Polozenie_poczatkowe_hitbox_szczura = {Hit_box_szczura.x, Hit_box_szczura.y};

    //--- PRZYCISKI DLA MENU I PAUZY
    // przyciski menu glownego
    Rectangle btnStart = {szerokosc_okna / 2.0f - 100, 300, 200, 50};
    Rectangle btnExit = {szerokosc_okna / 2.0f - 100, 370, 200, 50};

    // przyciski pauzy
    Rectangle btnWznow = {szerokosc_okna / 2.0f - 100, 250, 200, 50};
    Rectangle btnPowrotMenu = {szerokosc_okna / 2.0f - 100, 320, 200, 50};
    Rectangle btnPauzaExit = {szerokosc_okna / 2.0f - 100, 390, 200, 50};

    bool na_dole = false;

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
            IsDead = false;
        }
        else if (aktualnyStan == LEVELONE)
        // --- KOD GRY ---
        {
            animacja++;
            distance++;
            // przechodzimy do pauzy gdy kliknie ktos escape
            if (IsKeyPressed(KEY_ESCAPE))
                aktualnyStan = PAUZA;

            // --- LOGIKA GRY ---

            // sprawdzanie kolizji kolizje*
             if (czas_skoku == 0)
             {
                 if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))
                 {
                     if (CheckCollisionRecs(Hit_box_gracza_slizg, Hit_box_szkieleta) || CheckCollisionRecs(Hit_box_gracza_slizg, Hit_box_szczura) || CheckCollisionRecs(Hit_box_gracza_slizg, Hit_box_bat) || CheckCollisionRecs(Hit_box_gracza_slizg, Hit_box_ducha_1) || CheckCollisionRecs(Hit_box_gracza_slizg, Hit_box_ducha_2) || CheckCollisionRecs(Hit_box_gracza_slizg, Hit_box_ducha_3) || CheckCollisionRecs(Hit_box_gracza_slizg, Hit_box_ducha_4))
                     {
                         aktualnyStan = GameOver;
                         IsDead = true;
                     }
                 }
                 else
                 {

                    if (CheckCollisionRecs(Hit_box_gracza, Hit_box_szkieleta) || CheckCollisionRecs(Hit_box_gracza, Hit_box_szczura) || CheckCollisionRecs(Hit_box_gracza, Hit_box_bat) || CheckCollisionRecs(Hit_box_gracza, Hit_box_ducha_1) || CheckCollisionRecs(Hit_box_gracza, Hit_box_ducha_2) || CheckCollisionRecs(Hit_box_gracza, Hit_box_ducha_3) || CheckCollisionRecs(Hit_box_gracza, Hit_box_ducha_4))
                    {
                        aktualnyStan = GameOver;
                        IsDead = true;
                    }
                }
            }
            else
            {
                if (CheckCollisionRecs(Hit_box_gracza_w_skoku, Hit_box_szkieleta) || CheckCollisionRecs(Hit_box_gracza_w_skoku, Hit_box_szczura) || CheckCollisionRecs(Hit_box_gracza_w_skoku, Hit_box_bat) || CheckCollisionRecs(Hit_box_gracza_w_skoku, Hit_box_ducha_1) || CheckCollisionRecs(Hit_box_gracza_w_skoku, Hit_box_ducha_2) || CheckCollisionRecs(Hit_box_gracza_w_skoku, Hit_box_ducha_3) || CheckCollisionRecs(Hit_box_gracza_w_skoku, Hit_box_ducha_4))
                {
                    aktualnyStan = GameOver;
                    IsDead = true;
                }
            }

            // poruszanie sie
            float dt = GetFrameTime();
            float speed = 0.01f; 

            float radiusX = 100; 
            float radiusY = 80;
            

            // przemieszczanie się bossa
            if (w_trakcie_bossa)
            {
                float t = distance * speed;

               czaszka_boss.polozenie.x = czaszka_boss_pozycja_poczontokowa.x + cos(t) * radiusX;
               czaszka_boss.polozenie.y = czaszka_boss_pozycja_poczontokowa.y + sin(t) * radiusY;
            }
            //przemieszczanie się napisu 
            float speed_napisu = 2.0f;
            if(polozenie_napisu.y <= 0 && !na_dole && distance >= boss_fight_distance - 1000)
            {
                if(polozenie_napisu.y == 0)
                {
                    na_dole = true;
                }

                polozenie_napisu.y += speed_napisu;
            }
            if(na_dole && polozenie_napisu.y >= -napis_o_bossie.height && distance >= boss_fight_distance)
            {
                polozenie_napisu.y -= speed_napisu;
            }
            // skakanie
            if (IsKeyDown(KEY_SPACE) && czas_skoku <= 0 || IsKeyDown(KEY_W) && czas_skoku <= 0 || IsKeyDown(KEY_UP) && czas_skoku <= 0)
            {
                czas_skoku = 0.001f; // zaczynamy skok
            }

            if (czas_skoku > 0)
            {
                float t = czas_skoku / max_czas_skoku;
                float parabola = 6.0f * t * (1.0f - t);
                polozenie_gracza.y = ziemiaY - wysokosc_skoku * parabola;
                Hit_box_gracza_w_skoku.y = polozenie_gracza.y - 10;
                czas_skoku += dt; // dodajemy czas, ktory uplynal (GetFrameTime)

                if (czas_skoku >= max_czas_skoku)
                {
                    czas_skoku = 0;               // koniec skoku
                    polozenie_gracza.y = ziemiaY; // upewniamy sie ze postac stoi na ziemi
                    Hit_box_gracza.y = Polozenie_poczatkowe_hitbox_gracza.y;
                    Hit_box_gracza_w_skoku.y = Polozenie_poczatkowe_hitbox_gracza_w_skoku.y;
                }
            }

            scrollingBack -= scrollSpeed * dt;                 // aktualizacja pozycji tła
            if (scrollingBack <= -szerokosc_tla_przeskalowana) //* skalowanie_obrazu_tla) // resetowanie pozycji gry tło wyjdzie poza ekran
            {
                scrollingBack += szerokosc_tla_przeskalowana;
            }

            float game_speed = 5 + log(1 + distance / 300);

            rozstrzal_przy_losowaniu += game_speed / 100;
            minDist += game_speed / 100;

            scrollSpeed += game_speed / 400;

            uniform_int_distribution<int> pozycjaX(szerokosc_okna + szkielet_tekstura.width * skalowanie_obrazu_szkieleta, rozstrzal_przy_losowaniu);
            uniform_int_distribution<int> pozycjaX_wysoki(szerokosc_okna + duch_tekstura.width * skalowanie_obrazu_duch, rozstrzal_przy_losowaniu);

            for (int i; i < 4; i++)
            {
                game_speed_hamowanie[i] = {5 + log(1 + distance / 300)};
            }
            // latanie ducha
            duch.polozenie.y += 2 * sin(distance / 20);
            Hit_box_ducha_1.y += 2 * sin(distance / 20);
            Hit_box_ducha_2.y += 2 * sin(distance / 20);
            Hit_box_ducha_3.y += 2 * sin(distance / 20);
            Hit_box_ducha_4.y += 2 * sin(distance / 20);

            if (!szkielet.czy_jest_zatrzymany)
            {
                szkielet.polozenie.x -= game_speed_hamowanie[0]; // przyspieszanie przeszkody w zaleznosci od game speedu
                Hit_box_szkieleta.x -= game_speed_hamowanie[0];
            }

            if (!bat.czy_jest_zatrzymany)
            {
                bat.polozenie.x -= game_speed_hamowanie[2];
                Hit_box_bat.x -= game_speed_hamowanie[2];
            }

            if (!szczur.czy_jest_zatrzymany)
            {
                szczur.polozenie.x -= game_speed_hamowanie[3];
                Hit_box_szczura.x -= game_speed_hamowanie[3];
            }

            if (!duch.czy_jest_zatrzymany)
            {
                duch.polozenie.x -= game_speed_hamowanie[1];
                Hit_box_ducha_1.x -= game_speed_hamowanie[1];
                Hit_box_ducha_2.x -= game_speed_hamowanie[1];
                Hit_box_ducha_3.x -= game_speed_hamowanie[1];
                Hit_box_ducha_4.x -= game_speed_hamowanie[1];
            }

            // upewnianie sie ze nie wygeneruja sie zablisko
            if (distance < boss_fight_distance - 500)
            {

                if (szkielet.polozenie.x < -szkielet_tekstura.width * skalowanie_obrazu_szkieleta)
                {
                    int x;
                    do
                    {
                        x = pozycjaX(gen);
                    } while (TooCloseForSzkielet(x, minDist, duch.polozenie, bat.polozenie, szczur.polozenie));

                    szkielet.polozenie.x = (float)x;
                    Hit_box_szkieleta.x = szkielet.polozenie.x + 30;
                }

                if (duch.polozenie.x < -duch_tekstura.width * skalowanie_obrazu_szkieleta)
                {
                    int x;
                    do
                    {
                        x = pozycjaX_wysoki(gen);
                    } while (TooCloseForDuch(x, minDist, szkielet.polozenie, bat.polozenie, szczur.polozenie));

                    duch.polozenie.x = (float)x;

                    Hit_box_ducha_1.x = duch.polozenie.x + 30;
                    Hit_box_ducha_2.x = offsetXducha_2;
                    Hit_box_ducha_3.x = offsetXducha_3;
                    Hit_box_ducha_4.x = offsetXducha_4;

                    duch.polozenie.y = pozycjay_wysoki(gen);

                    Hit_box_ducha_1.y = offsetYducha_1;
                    Hit_box_ducha_2.y = offsetYducha_2;
                    Hit_box_ducha_3.y = offsetYducha_3;
                    Hit_box_ducha_4.y = offsetYducha_4;
                }

                if (bat.polozenie.x < -bat_tekstura.width * skalowanie_obrazu_bat)
                {
                    int x;
                    do
                    {
                        x = pozycja_czenste(gen);
                    } while (TooCloseForBat(x, minDist, duch.polozenie, szkielet.polozenie, szczur.polozenie));

                    bat.polozenie.x = (float)x;

                    Hit_box_bat.x = offsetXbat;

                    bat.polozenie.y = pozycjay_wysoki(gen);

                    Hit_box_bat.y = offsetYbat;
                }
                if (szczur.polozenie.x < -szczur_tekstura.width * skalowanie_obrazu_szczura)
                {
                    int x;
                    do
                    {
                        x = pozycjaX(gen);
                    } while (TooCloseForSzczur(x, minDist, duch.polozenie, szkielet.polozenie, bat.polozenie));

                    szczur.polozenie.x = (float)x;

                    Hit_box_szczura.x = offsetXszczura; // dopasuj offset
                    Hit_box_szczura.y = offsetYszczura;
                }
            }
            else
            {
                if (szkielet.polozenie.x < -szkielet_tekstura.width * skalowanie_obrazu_szkieleta)
                {
                    szkielet.polozenie = polozenie_startowe_szkieleta;
                    Hit_box_szkieleta.x = Polozenie_poczatkowe_hitbox_szkieleta.x;
                    szkielet.czy_jest_zatrzymany = true;
                }

                if (duch.polozenie.x < -duch_tekstura.width * skalowanie_obrazu_szkieleta)
                {
                    duch.polozenie = polozenie_startowe_ducha;
                    Hit_box_ducha_1.x = Polozenie_poczatkowe_hitbox_ducha_1.x;
                    Hit_box_ducha_1.y = Polozenie_poczatkowe_hitbox_ducha_1.y;

                    Hit_box_ducha_2.x = Polozenie_poczatkowe_hitbox_ducha_2.x;
                    Hit_box_ducha_2.y = Polozenie_poczatkowe_hitbox_ducha_2.y;

                    Hit_box_ducha_3.x = Polozenie_poczatkowe_hitbox_ducha_3.x;
                    Hit_box_ducha_3.y = Polozenie_poczatkowe_hitbox_ducha_3.y;

                    Hit_box_ducha_4.x = Polozenie_poczatkowe_hitbox_ducha_4.x;
                    Hit_box_ducha_4.y = Polozenie_poczatkowe_hitbox_ducha_4.y;

                    duch.czy_jest_zatrzymany = true;
                }

                if (bat.polozenie.x < -bat_tekstura.width * skalowanie_obrazu_bat)
                {
                    bat.polozenie = polozenie_startowe_bat;
                    Hit_box_bat.x = Polozenie_poczatkowe_hitbox_bat.x;
                    Hit_box_bat.y = Polozenie_poczatkowe_hitbox_bat.y;
                    bat.czy_jest_zatrzymany = true;
                }
                if (szczur.polozenie.x < -szczur_tekstura.width * skalowanie_obrazu_szczura)
                {
                    szczur.polozenie = polozenie_startowe_szczura;
                    Hit_box_szczura.x = Polozenie_poczatkowe_hitbox_szczura.x;
                    Hit_box_szczura.y = Polozenie_poczatkowe_hitbox_szczura.y;
                    szczur.czy_jest_zatrzymany = true;
                }
            }

            max_czas_skoku -= log(2 + distance / 1000) / 100000; // w teori powinno uniknąc to problemów przy zaduzej predkosci przeszkud ale moze to usune potem
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
            animacja++;
            if (CzyKliknietoPrzycisk(btnPowrotMenu))
            {

                UpdateTop10(nickname, distance);
                distance = 0;
                szkielet.polozenie = polozenie_startowe_szkieleta;
                Hit_box_szkieleta.x = Polozenie_poczatkowe_hitbox_szkieleta.x;

                duch.polozenie = polozenie_startowe_ducha;
                Hit_box_ducha_1.x = Polozenie_poczatkowe_hitbox_ducha_1.x;
                Hit_box_ducha_1.y = Polozenie_poczatkowe_hitbox_ducha_1.y;

                Hit_box_ducha_2.x = Polozenie_poczatkowe_hitbox_ducha_2.x;
                Hit_box_ducha_2.y = Polozenie_poczatkowe_hitbox_ducha_2.y;

                Hit_box_ducha_3.x = Polozenie_poczatkowe_hitbox_ducha_3.x;
                Hit_box_ducha_3.y = Polozenie_poczatkowe_hitbox_ducha_3.y;

                Hit_box_ducha_4.x = Polozenie_poczatkowe_hitbox_ducha_4.x;
                Hit_box_ducha_4.y = Polozenie_poczatkowe_hitbox_ducha_4.y;

                polozenie_gracza.y = ziemiaY;
                Hit_box_gracza.y = Polozenie_poczatkowe_hitbox_gracza.y;

                bat.polozenie = polozenie_startowe_bat;
                Hit_box_bat.x = Polozenie_poczatkowe_hitbox_bat.x;
                Hit_box_bat.y = Polozenie_poczatkowe_hitbox_bat.y;

                szczur.polozenie = polozenie_startowe_szczura;
                Hit_box_szczura.x = Polozenie_poczatkowe_hitbox_szczura.x;
                Hit_box_szczura.y = Polozenie_poczatkowe_hitbox_szczura.y;

                max_czas_skoku = 1.0f;
                aktualnyStan = MENU;
                scrollSpeed = 150.0f;

                gen.seed(random_device{}());

                szkielet.czy_jest_zatrzymany = false;
                duch.czy_jest_zatrzymany = false;
                bat.czy_jest_zatrzymany = false;
                szczur.czy_jest_zatrzymany = false;
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

            // // rysujemy tlo menu
            DrawTextureEx(background_menu, {0, 0}, 0.0f, skalowanie_obrazu_tla_menu, WHITE);
            // // rysujemy na tle gry zeby przyciemnic
            DrawRectangle(0, 0, szerokosc_okna, wysokosc_okna, Fade(BLACK, 0.3f));

            // napis w menu
            // DrawText("TYTUL GRY", szerokosc_okna / 2 - 150, 100, 40, DARKGRAY);

            // Pole na wpisywanie nicku
            // DrawText("WPISZ SWOJ NICK: ", szerokosc_okna / 2 - 100, 200, 20, DARKGRAY);
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

            // DrawText("TABELA TOP 10: ", 50, 50, 20, GOLD);

            while (plikPokaz >> n >> d && yOffset < 10)
            {
                DrawText(TextFormat("%d. %s - %d", yOffset + 1, n.c_str(), d), 50, 120 + (yOffset * 25), 18, DARKGRAY);
                yOffset++;
            }
            plikPokaz.close();
        }
        else if (aktualnyStan == LEVELONE || aktualnyStan == PAUZA || aktualnyStan == GameOver) // gre rysujemy nawet jak jestesmy w pauzie
        {
            int frameCount = (animacja + 1) % 60;

            for (float x = scrollingBack; x < szerokosc_okna; x += szerokosc_tla_przeskalowana)
            {
                DrawTextureEx(background, {x, -200}, 0.0f, skalowanie_obrazu_tla, WHITE);
            }

            // rysowanie przeciwników w zależności od tego czy jest boss_tekstura fight czy go niema

            if (IsDead == false)
            {
                // rysowanie bossa
                
                DrawTextureEx(napis_o_bossie, polozenie_napisu, 0.0f, 0.45, WHITE);

                if (szkielet.czy_jest_zatrzymany && duch.czy_jest_zatrzymany && bat.czy_jest_zatrzymany && szczur.czy_jest_zatrzymany)
                {
                    int Animacja_wychodzenia = (distance + 1) % co_ile_laser;
                    w_trakcie_bossa = true;
                    if (co_ile_laser / 2 <= Animacja_wychodzenia && Animacja_wychodzenia <= co_ile_laser / 2 + wind_up_laseru)
                    {
                        DrawTextureEx(czaszka_boss.tekstury[1], czaszka_boss.polozenie, 0.0f, skalowanie_obrazu_boss, GRAY);
                    }
                    else if (co_ile_laser / 2 + wind_up_laseru <= Animacja_wychodzenia && Animacja_wychodzenia <= co_ile_laser / 2 + wind_up_laseru + czas_trwania_laseru)
                    {
                        DrawTextureEx(czaszka_boss.tekstury[2], czaszka_boss.polozenie, 0.0f, skalowanie_obrazu_boss, GRAY);
                    }
                    else
                    {
                        DrawTextureEx(czaszka_boss.tekstury[0], czaszka_boss.polozenie, 0.0f, skalowanie_obrazu_boss, GRAY);
                    }
                }
                DrawTextureEx(duch.tekstura, duch.polozenie, 0.25f, skalowanie_obrazu_szkieleta, WHITE);

                // rysowanie nietobeza i szczura

                if (frameCount <= 15)
                {
                    DrawTextureEx(szczur_tekstura, szczur.polozenie, 0.0f, skalowanie_obrazu_szczura, WHITE);
                }
                else if (frameCount <= 30)
                {
                    DrawTextureEx(szczur_tekstura, szczur.polozenie, 0.0f, skalowanie_obrazu_szczura * 0.9, WHITE);
                }
                else if (frameCount <= 45)
                {
                    DrawTextureEx(szczur_tekstura, szczur.polozenie, 0.0f, skalowanie_obrazu_szczura * 0.85, WHITE);
                }
                else
                {
                    DrawTextureEx(szczur_tekstura, szczur.polozenie, 0.0f, skalowanie_obrazu_szczura * 0.9, WHITE);
                }

                if (frameCount <= 15)
                {
                    DrawTextureEx(bat_tekstura, bat.polozenie, 0.0f, skalowanie_obrazu_bat, WHITE);
                }
                else if (frameCount <= 30)
                {
                    DrawTextureEx(bat_tekstura_2, bat.polozenie, 0.0f, skalowanie_obrazu_bat, WHITE);
                }
                else if (frameCount <= 45)
                {
                    DrawTextureEx(bat_tekstura, bat.polozenie, 0.0f, skalowanie_obrazu_bat, WHITE);
                }
                else
                {
                    DrawTextureEx(bat_tekstura_3, bat.polozenie, 0.0f, skalowanie_obrazu_bat, WHITE);
                }

                if (frameCount <= 15)
                {
                    DrawTextureEx(szkielet_tekstura_2, szkielet.polozenie, 0.0f, skalowanie_obrazu_szkieleta, WHITE);
                }
                else if (frameCount <= 30)
                {
                    DrawTextureEx(szkielet_tekstura_3, szkielet.polozenie, 0.0f, skalowanie_obrazu_szkieleta, WHITE);
                }
                else if (frameCount <= 45)
                {
                    DrawTextureEx(szkielet_tekstura, szkielet.polozenie, 0.0f, skalowanie_obrazu_szkieleta * 0.7, WHITE);
                }
                else
                {
                    DrawTextureEx(szkielet_tekstura_3, szkielet.polozenie, 0.0f, skalowanie_obrazu_szkieleta, WHITE);
                }

                if (czas_skoku == 0)
                {
                    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))
                    {
                        DrawTextureEx(slizg, {polozenie_gracza.x, polozenie_gracza.y + 50}, 0.0f, skalowanie_obrazu_gracza, WHITE);
                    }
                    else
                    {
                        int frameCount = (animacja + 1) % 60;
                        if (frameCount <= 15)
                        {
                            DrawTextureEx(tekstura_gracza_1, polozenie_gracza, 0.0f, skalowanie_obrazu_gracza, WHITE);
                        }
                        else if (frameCount <= 30)
                        {
                            DrawTextureEx(tekstura_gracza_3, polozenie_gracza, 0.0f, skalowanie_obrazu_gracza, WHITE);
                        }
                        else if (frameCount <= 45)
                        {
                            DrawTextureEx(tekstura_gracza_2, polozenie_gracza, 0.0f, skalowanie_obrazu_gracza, WHITE);
                        }
                        else
                        {
                            DrawTextureEx(tekstura_gracza_3, polozenie_gracza, 0.0f, skalowanie_obrazu_gracza, WHITE);
                        }
                    }
                }
                else
                {
                    DrawTextureEx(tekstura_skoku, {polozenie_gracza.x - 40, polozenie_gracza.y - 30}, 0.0f, skalowanie_obrazu_gracza_skoku, WHITE);
                }
            }

            // rysowanie hit box do debugowania i dostoswywania
            // if (czas_skoku == 0)
            // {
            //     if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))
            //     {
            //         rysowanie_hit_box(Hit_box_gracza_slizg, RED);
            //     }
            //     else
            //         rysowanie_hit_box(Hit_box_gracza, RED);
            // }
            // else
            // {
            //     rysowanie_hit_box(Hit_box_gracza_w_skoku, RED);
            // }

            // rysowanie_hit_box(Hit_box_szkieleta, GREEN);
            // rysowanie_hit_box(Hit_box_ducha_1, GREEN);

            // rysowanie_hit_box(Hit_box_ducha_2, GREEN);
            // rysowanie_hit_box(Hit_box_ducha_3, GREEN);
            // rysowanie_hit_box(Hit_box_ducha_4, GREEN);

            // rysowanie_hit_box(Hit_box_bat, GREEN);

            // rysowanie_hit_box(Hit_box_szczura, GREEN);
            // istotne prosze nie usuwać !!!!

            // animacja szkieleta

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
                DrawText("GAME OVER", szerokosc_okna / 2 - 150, 200, 50, GRAY);

                int frameCounter = (animacja + 1) % 30;

                if (frameCounter < 15)
                {
                    DrawTextureEx(czaszka, {szerokosc_okna / 2 - 105, 420}, 0.0f, 0.2, GRAY);
                }
                else
                {
                    DrawTextureEx(czaszka_2, {szerokosc_okna / 2 - 105, 420}, 0.0f, 0.2, GRAY);
                }

                DrawText(TextFormat("KONCOWY DYSTANS: %05d", distance), szerokosc_okna / 2 - 300, 260, 50, GRAY);

                DrawRectangleRec(btnPowrotMenu, CzyMyszkaNadPrzyciskiem(btnPowrotMenu) ? kolor1btnMenu : kolor2btnMenu);
                DrawText("MENU", btnPowrotMenu.x + 65, btnPowrotMenu.y + 15, 20, WHITE);

                DrawRectangleRec(btnPauzaExit, CzyMyszkaNadPrzyciskiem(btnPauzaExit) ? kolor1btnExit : kolor2btnExit);
                DrawText("WYJDZ", btnPauzaExit.x + 60, btnPauzaExit.y + 15, 20, WHITE);
            }
        }
        EndDrawing();
    }

    UnloadTexture(czaszka_2);
    UnloadTexture(czaszka);
    UnloadTexture(slizg);
    UnloadTexture(background_menu);
    UnloadTexture(background_las);
    UnloadTexture(tekstura_skoku);
    UnloadTexture(background);
    UnloadTexture(tekstura_gracza_1);
    UnloadTexture(tekstura_gracza_2);
    UnloadTexture(tekstura_gracza_3);
    UnloadTexture(szkielet_tekstura);
    UnloadTexture(szkielet_tekstura_2);
    UnloadTexture(szkielet_tekstura_3);
    UnloadTexture(boss_tekstura_laser_2);
    
    CloseWindow();

    return 0;
}
