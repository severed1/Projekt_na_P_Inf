// kompilacja
// g++ main_file.cpp -o gra.exe -I C:/raylib/include -L C:/raylib/lib -lraylib -lopengl32 -lgdi32 -lwinmm

// td
//  1. reset

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
#include "hitboxes.cpp"
#include <algorithm>

// wartosci dla okna
#define wysokosc_okna 720
#define szerokosc_okna 1280
#define nazwa_gry_wyswietlana_na_oknie "Dungeon Run" // to zmienimy jak ustalimy fabule
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
#define skalowanie_obrazu_laseru 1.0f
#define skalowanie_borazu_fireball 0.1f
#define skalowanie_obrazu_kuszy 0.15f
#define skalowanie_obrazu_bolt 0.15f
#define skalowanie_obrazu_serca_bossa 0.16f

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

#define offsetXlasera ((float)czaszka_boss.polozenie.x - 1300)
#define offsetYlasera ((float)czaszka_boss.polozenie.y + 450)

#define offsetXfireball ((float)fireball_przeszkoda.polozenie.x + 35)
#define offsetYfireball ((float)fireball_przeszkoda.polozenie.y + 30)

#define offsetXkusza ((float)kusza_item.polozenie.x + 30)
#define offsetYkusza ((float)kusza_item.polozenie.y + 20)

// prock boss_tekstura fighta
#define boss_fight_distance 2000 // 4000
#define co_ile_laser 2000
#define wind_up_laseru 300
#define czas_trwania_laseru 400
#define dlugosc_trzensienia_ziemi 500
#define moc_trznsienia_ziemi 2
#define dlugosc_wysuwania 500
#define dlugosc_czekania_az_boss_zaczie_ruszac 300
#define dlugosc_trwania_znaku 800
#define dystans_do_zatrzymania_przeszkud_gdy_boss 900
#define speed_fireballa 5
#define speed_fireballa_krencenie 0.05f
#define speed_fireballa_przod 70.0f
#define radiusX_fireball 80
#define radiusY_fireball 60
#define jak_czensto_kusza 3000 // 8000

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

struct item
{
    Texture2D tekstura;
    string rodzaj_itemu;
    Vector2 polozenie;
    bool czy_podniesiona;
};

struct boss
{
    Texture2D tekstury[4];
    Vector2 polozenie;
    int ilosc_hp;
    bool laser_on;
    bool laser_rozgrzewanie;
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
    // Vector2 myszka = GetMousePosition();
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

int zycie = 3; // ilość serc/gracza
bool trafiony = false;
int poprzednieZycie = 3;
bool IsDead = false;
int distance = 0;

float animacjaSerceTimer = 0.0f;
bool animacjaSerceAktywna = false;

const float CZAS_ANIMACJI_SERCA = 0.3f;

int szerokoscSerca = 65;
int wysokoscSerca = 65;
int odstep = 0;  // odstęp między sercami
int startX = 10; // lewy margines od ekranu
int startY = 10;

bool gra_wystartowala = false;

void StartNowejGry()
{
    trafiony = false;
    zycie = 3;
    poprzednieZycie = 3;
    animacjaSerceAktywna = false;
    animacjaSerceTimer = 0.0f;
    IsDead = false;
    gra_wystartowala = false;
}

void GraczTrafiony()
{
    if (!animacjaSerceAktywna)
    {
        zycie--;
        animacjaSerceAktywna = true;
        animacjaSerceTimer = 0.0f;
        if (zycie <= 0)
        {
            aktualnyStan = GameOver;
            IsDead = true;
        }
    }
}

int main()
{
    InitWindow(szerokosc_okna, wysokosc_okna, nazwa_gry_wyswietlana_na_oknie); // inicjuje otwarcie okna o podanych wymiarach
    SetExitKey(KEY_NULL);                                                      // wylaczamy bazowe zachowanie klawisza esc
    SetTargetFPS(ilosc_fps);                                                   // ustala ilosc docelowa fps w oknie

    Font font = LoadFontEx("assets/font/font.otf", 64, 0, 0);

    random_device rd;
    mt19937 gen(rd());

    // TŁA I ZMIENNE DO ZMIANY TŁA
    Texture2D backgroundLochy = LoadTexture("assets/background/lochy.png"); // wczytywanie tekstury tła
    Texture2D backgroundLas = LoadTexture("assets/background/las.png");
    Texture2D backgroundBridge = LoadTexture("assets/background/bridge.png");

    Texture2D *tloAktualne = &backgroundLochy;
    Texture2D *tloNastepne = &backgroundLochy;

    int fazaSwiata = 0; // 0 - lochy, 1 - przejscie do lasu, 2 - las, 3- przejscie do lochow
    bool transition = false;

    // BOHATER
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

    Texture2D szczur_tekstura = LoadTexture("assets/przeszkody/szczur.png");

    Texture2D bat_tekstura = LoadTexture("assets/przeszkody/bat.png");
    Texture2D bat_tekstura_2 = LoadTexture("assets/przeszkody/bat_2.png");
    Texture2D bat_tekstura_3 = LoadTexture("assets/przeszkody/bat_3.png");

    Texture2D boss_tekstura = LoadTexture("assets/Boss/boss.png");
    Texture2D boss_tekstura_laser_1 = LoadTexture("assets/Boss/boss_laser_1.png");
    Texture2D boss_tekstura_laser_2 = LoadTexture("assets/Boss/boss_laser_2.png");
    Texture2D boss_tekstura_2 = LoadTexture("assets/Boss/boss_2.png");
    Texture2D napis_o_bossie = LoadTexture("assets/background/napis.png");
    Texture2D laser = LoadTexture("assets/Boss/laser_2.png");
    Texture2D kusza = LoadTexture("assets/Boss/kusza.png");
    Texture2D serce_bossa = LoadTexture("assets/Boss/serce_bossa.png");

    Texture2D fireball = LoadTexture("assets/Boss/fireball.png");

    Texture2D serceTexture = LoadTexture("assets/background/serce.png"); // życie gracza

    // tekstury przycisków
    Texture2D startText = LoadTexture("assets/przyciski/start.png");
    Texture2D menuText = LoadTexture("assets/przyciski/menu.png");
    Texture2D exitText = LoadTexture("assets/przyciski/exit.png");
    Texture2D wznowText = LoadTexture("assets/przyciski/wznow.png");
    Texture2D retryText = LoadTexture("assets/przyciski/retry.png");
    Texture2D bolt = LoadTexture("assets/Boss/bolt.png");

    int animacja = 0; // nie istotne uzyte do próby animacji
    // generowanie losowej pozycji dla kaktusow

    float rozstrzal_przy_losowaniu = 9000;
    float rozstrzal_przy_losowaniu_czestych = 5000;
    // upewnianie sie ze nie wygeneruja sie zablisko
    float minDist = 500;
    uniform_int_distribution<int> pozycjaX(szerokosc_okna + szkielet_tekstura.width * skalowanie_obrazu_szkieleta, rozstrzal_przy_losowaniu);
    uniform_int_distribution<int> pozycjaX_wysoki(szerokosc_okna + duch_tekstura.width * skalowanie_obrazu_duch, rozstrzal_przy_losowaniu);
    uniform_int_distribution<int> pozycjay_wysoki(200, (float)wysokosc_okna - szkielet_tekstura.height * skalowanie_obrazu_szkieleta + 90);
    uniform_int_distribution<int> pozycja_czenste(szerokosc_okna + szkielet_tekstura.width * skalowanie_obrazu_szkieleta, rozstrzal_przy_losowaniu_czestych);

    // losowanie dla fire balla
    uniform_int_distribution<int> r_fireball_x(50, 110);
    uniform_int_distribution<int> r_fireball_y(50, 110);
    uniform_real_distribution<float> krencenie_fireball(0.05f, 0.07f);
    uniform_int_distribution<int> speed_fireball(60, 80);

    uniform_int_distribution<int> pozycjaX_kuszy(szerokosc_okna + szkielet_tekstura.width * skalowanie_obrazu_szkieleta, jak_czensto_kusza);

    // sprawdza czy w trakcie boss_tekstura fightu
    // zmienne dla bossa
    bool w_trakcie_bossa = false;
    bool krencenie_sie = false;
    bool czy_powinna_leciec = false;
    bool po_bossie = false;
    int licznik_klatek_wysuwanie = 0;
    int klatka = 0;
    int distance_dla_fireballa = 0;
    int animacja_na_7 = 0;
    Vector2 pozycja_po_wysunieciu;
    bool ucieczka = false;
    int animacja_smierci_czaszki = 0;

    float speed_fireballa_krencenie_z = speed_fireballa_krencenie;
    float speed_fireballa_przod_z = speed_fireballa_przod;
    float radiusX_fireball_z = radiusX_fireball;
    float radiusY_fireball_z = radiusY_fireball;

    // bolt zmienne

    float boltT = 0.0f;
    float boltStep = 0.005f;
    float boltHeight = 200.0f;
    float boltAngle = 0.0f;
    float endT = 0.75f;
    float startX = 250.0f;
    float endX = 1200.0f;

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
    czaszka_boss.tekstury[3] = boss_tekstura_2;
    czaszka_boss.ilosc_hp = 3;
    czaszka_boss.polozenie = {szerokosc_okna / 2 + 500, wysokosc_okna / 2 - 500};
    czaszka_boss.laser_on = false;

    przeszkoda laser_przeszkoda;
    laser_przeszkoda.tekstura = laser;
    laser_przeszkoda.rodzaj_typu_przeszkody = 5;
    laser_przeszkoda.polozenie = {(float)czaszka_boss.polozenie.x - 1300, (float)czaszka_boss.polozenie.y};
    Vector2 polozenie_startowe_laseru = laser_przeszkoda.polozenie;

    przeszkoda fireball_przeszkoda;
    fireball_przeszkoda.tekstura = fireball;
    fireball_przeszkoda.rodzaj_typu_przeszkody = 6;
    fireball_przeszkoda.polozenie = {(float)czaszka_boss.polozenie.x - 700, (float)czaszka_boss.polozenie.y + 350}; // problem
    Vector2 polozenie_startowe_fireballa = fireball_przeszkoda.polozenie;
    Vector2 polozenie_startowe_fireballa_prawdziwe = polozenie_startowe_fireballa;

    item kusza_item;
    kusza_item.tekstura = kusza;
    kusza_item.rodzaj_itemu = "kusza";
    kusza_item.polozenie = {(float)pozycjaX_kuszy(gen), wysokosc_okna / 2 + 145};
    kusza_item.czy_podniesiona = false;

    Vector2 czaszka_boss_pozycja_poczontokowa = {czaszka_boss.polozenie.x, czaszka_boss.polozenie.y};

    Vector2 polozenie_napisu = {szerokosc_okna / 2 - 350, (float)-napis_o_bossie.height};
    Vector2 poczontkowe_polozenie_napisu = {szerokosc_okna / 2 - 350, (float)-napis_o_bossie.height};

    int pozycja_tla_y = -200;

    int distance = 0; // zmienna bedzie rosla w trakcie gry jak postac biegnie i przykladowo jak dojdzie do jakiejs wartosci to pojawia sie boss_tekstura

    bool startLevelOne = true; // dodanie tektu przy pierwszej zmianie predkosci
    float timer_tekst = 0.0f;
    std::string tekst_fabularny = "Wyruszamy ku przygodzie!"; //  tekst fabularny

    //---ZMIENNE DO TABELI WYNIKOW I NICKU GRACZA
    char nickname[16] = "\0"; // tablica na nick (15znakow)
    int charCount = 0;
    // int highScore = 0;

    // wartości dla postaci
    float polozenie_gracza_x = 20;
    float polozenie_gracza_y = GetScreenHeight() - tekstura_gracza_1.height * skalowanie_obrazu_gracza - 60; // ustawianie go na dole
    float ziemiaY = polozenie_gracza_y;

    Vector2 polozenie_gracza = {polozenie_gracza_x, polozenie_gracza_y}; // inicjowanie vektora 2 wymiarowego

    // zmienne zamiast int klatka_skoku
    float czas_skoku = 0.0f;
    float max_czas_skoku = 1.0f; // tutaj mozna zmieniac czas trwania skoku (w sekundach)

    float szerokosc_tla_przeskalowana = backgroundLochy.width * skalowanie_obrazu_tla;
    float scrollingBack = 0.0f;
    float scrollSpeed = 150.00f; // predkosc przesuwania się tła w pikselach na sekunde

    // tworzenie hit boxów

    Rectangle Hit_box_gracza = {polozenie_gracza.x + 520 * skalowanie_obrazu_gracza, polozenie_gracza.y + 100 * skalowanie_obrazu_gracza, tekstura_gracza_1.width * skalowanie_obrazu_gracza - 250, tekstura_gracza_1.height * skalowanie_obrazu_gracza - 50};
    Rectangle Hit_box_gracza_w_skoku = {polozenie_gracza.x + 350 * skalowanie_obrazu_gracza, polozenie_gracza.y - 10, tekstura_gracza_1.width * skalowanie_obrazu_gracza - 230, tekstura_gracza_1.height * skalowanie_obrazu_gracza - 100};
    Rectangle Hit_box_gracza_slizg = {polozenie_gracza.x + 150 * skalowanie_obrazu_gracza, polozenie_gracza.y + 600 * skalowanie_obrazu_gracza, slizg.width * skalowanie_obrazu_gracza - 100, slizg.height * skalowanie_obrazu_gracza - 140};

    Rectangle Hit_box_szkieleta = {szkielet.polozenie.x + 30, szkielet.polozenie.y + 48, szkielet.tekstura.width * skalowanie_obrazu_szkieleta - 360, szkielet.tekstura.height * skalowanie_obrazu_szkieleta - 300};
    // Rectangle Hit_box_szkieleta_nogi = {szkielet.polozenie.x + 30, szkielet.polozenie.y + 48, szkielet.tekstura.width * skalowanie_obrazu_szkieleta - 320, szkielet.tekstura.height * skalowanie_obrazu_szkieleta - 300};

    Rectangle Hit_box_ducha_1 = {duch.polozenie.x + 30, offsetYducha_1, duch.tekstura.height * skalowanie_obrazu_duch, duch.tekstura.width * skalowanie_obrazu_duch - 60};
    Rectangle Hit_box_ducha_2 = {offsetXducha_2, offsetYducha_2, duch.tekstura.height * skalowanie_obrazu_duch + 30, duch.tekstura.width * skalowanie_obrazu_duch - 35};
    Rectangle Hit_box_ducha_3 = {offsetXducha_3, offsetYducha_3, duch.tekstura.height * skalowanie_obrazu_duch + 80, duch.tekstura.width * skalowanie_obrazu_duch - 25};
    Rectangle Hit_box_ducha_4 = {offsetXducha_4, offsetYducha_4, duch.tekstura.height * skalowanie_obrazu_duch + 70, duch.tekstura.width * skalowanie_obrazu_duch - 40};

    Rectangle Hit_box_bat = {offsetXbat, offsetYbat, bat.tekstura.width * skalowanie_obrazu_duch, bat.tekstura.height * skalowanie_obrazu_duch};

    Rectangle Hit_box_szczura = {offsetXszczura, offsetYszczura, szczur.tekstura.height * skalowanie_obrazu_szczura - 40, (szczur.tekstura.width * skalowanie_obrazu_szczura) - 250};

    Rectangle Hit_box_lasera = {offsetXlasera, offsetYlasera, szerokosc_okna + 300, (laser_przeszkoda.tekstura.width * skalowanie_obrazu_szczura) - 220};

    Rectangle Hit_box_fireball = {offsetXfireball, offsetYfireball, fireball_przeszkoda.tekstura.width * skalowanie_borazu_fireball - 70, fireball_przeszkoda.tekstura.height * skalowanie_borazu_fireball - 50};

    Rectangle Hit_box_kusza = {offsetXkusza, offsetYkusza, kusza_item.tekstura.width * skalowanie_obrazu_kuszy - 70, kusza_item.tekstura.height * skalowanie_obrazu_kuszy - 70};

    // potrzebne zeby sie nie ruszal po smierci

    double game_speed_hamowanie[4] = {5 + log(1 + distance / 300)}; // zwiekszanie sie game speedu logarytmicznie zeby nie zaszybko sie zmieniało w predkosc swiatla

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

    Vector2 Polozenie_poczatkowe_hitbox_laseru = {Hit_box_lasera.x, Hit_box_lasera.y};

    // Vector2 Polozenie_poczatkowe_hitbox_fireballa = {Hit_box_fireball.x, Hit_box_fireball.y};

    float przesuniecie = 30.0f;
    Vector2 Polozenie_bolt = {100, wysokosc_okna / 2 + 100};
    Vector2 Polozenie_serca_1 = {szerokosc_okna / 2 + 610 - przesuniecie, wysokosc_okna / 2 - 200 - przesuniecie};
    Vector2 Polozenie_serca_2 = {szerokosc_okna / 2 + 680 - przesuniecie, wysokosc_okna / 2 - 270 - przesuniecie};
    Vector2 Polozenie_serca_3 = {szerokosc_okna / 2 + 750 - przesuniecie, wysokosc_okna / 2 - 200 - przesuniecie};

    Vector2 Polozenie_poczatkowe_serca_1 = Polozenie_serca_1;
    Vector2 Polozenie_poczatkowe_serca_2 = Polozenie_serca_2;
    Vector2 Polozenie_poczatkowe_serca_3 = Polozenie_serca_3;

    //--- PRZYCISKI DLA MENU I PAUZY
    // przyciski menu glownego
    Rectangle btnStart = {szerokosc_okna / 2.0f - 155, wysokosc_okna - 195, 320, 90};
    Rectangle btnExit = {szerokosc_okna / 2.0f - 155, wysokosc_okna - 95, 320, 90};

    // przyciski pauzy
    Rectangle btnWznow = {szerokosc_okna / 2.0f - 100, 250, 200, 50};
    Rectangle btnPowrotMenu = {szerokosc_okna / 2.0f - 100, 320, 200, 50};
    Rectangle btnPauzaExit = {szerokosc_okna / 2.0f - 100, 390, 200, 50};

    bool na_dole = false;

    bool trafiony = false;

    while (!WindowShouldClose()) // utrzymuje okno otwarte i wykonuje polecenie wymagane przy operacji na oknach
    {
        //-- IF DLA POSZCZEGOLNYCH STANOW GRY
        if (aktualnyStan == MENU)
        {
            //--- KOD MENU ---
            if (CzyKliknietoPrzycisk(btnStart) || IsKeyPressed(KEY_ENTER))
            {
                StartNowejGry();
                aktualnyStan = LEVELONE;
                gra_wystartowala = true;
            }

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
        else if (aktualnyStan == LEVELONE || aktualnyStan == LEVELTWO)
        // --- KOD GRY ---
        {
            animacja++;
            distance++;

            // --- ustawienie tekstu fabularnego tylko raz ---
            if (startLevelOne)
            {

                timer_tekst = 2.0f;
                tekst_fabularny = "Wyruszamy ku przygodzie!"; // tekst fabularny
                startLevelOne = false;
            }

            // przechodzimy do pauzy gdy kliknie ktos escape
            if (IsKeyPressed(KEY_ESCAPE))
                aktualnyStan = PAUZA;

            // --- LOGIKA GRY ---

            // sprawdzanie kolizji kolizje*
            trafiony = false;

            if (czas_skoku == 0)
            {
                if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))
                {
                    if (CheckCollisionRecs(Hit_box_gracza_slizg, Hit_box_szkieleta) || CheckCollisionRecs(Hit_box_gracza_slizg, Hit_box_fireball) || CheckCollisionRecs(Hit_box_gracza_slizg, Hit_box_szczura) || CheckCollisionRecs(Hit_box_gracza_slizg, Hit_box_bat) || CheckCollisionRecs(Hit_box_gracza_slizg, Hit_box_ducha_1) || CheckCollisionRecs(Hit_box_gracza_slizg, Hit_box_ducha_2) || CheckCollisionRecs(Hit_box_gracza_slizg, Hit_box_ducha_3) || CheckCollisionRecs(Hit_box_gracza_slizg, Hit_box_ducha_4))
                    {
                        trafiony = true;
                    }
                }
                else
                {

                    if (CheckCollisionRecs(Hit_box_gracza, Hit_box_szkieleta) || CheckCollisionRecs(Hit_box_gracza, Hit_box_fireball) || CheckCollisionRecs(Hit_box_gracza, Hit_box_szczura) || CheckCollisionRecs(Hit_box_gracza, Hit_box_bat) || CheckCollisionRecs(Hit_box_gracza, Hit_box_ducha_1) || CheckCollisionRecs(Hit_box_gracza, Hit_box_ducha_2) || CheckCollisionRecs(Hit_box_gracza, Hit_box_ducha_3) || CheckCollisionRecs(Hit_box_gracza, Hit_box_ducha_4))
                    {
                        trafiony = true;
                    }
                }
            }
            else
            {
                if (CheckCollisionRecs(Hit_box_gracza_w_skoku, Hit_box_szkieleta) || CheckCollisionRecs(Hit_box_gracza_w_skoku, Hit_box_fireball) || CheckCollisionRecs(Hit_box_gracza_w_skoku, Hit_box_szczura) || CheckCollisionRecs(Hit_box_gracza_w_skoku, Hit_box_bat) || CheckCollisionRecs(Hit_box_gracza_w_skoku, Hit_box_ducha_1) || CheckCollisionRecs(Hit_box_gracza_w_skoku, Hit_box_ducha_2) || CheckCollisionRecs(Hit_box_gracza_w_skoku, Hit_box_ducha_3) || CheckCollisionRecs(Hit_box_gracza_w_skoku, Hit_box_ducha_4))
                {
                    trafiony = true;
                }
            }

            if (czaszka_boss.laser_on)
            {
                if (czas_skoku == 0)
                {
                    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))
                    {
                        if (CheckCollisionRecs(Hit_box_gracza_slizg, Hit_box_lasera))
                        {
                            trafiony = true;
                        }
                    }
                    else
                    {
                        if (CheckCollisionRecs(Hit_box_gracza, Hit_box_lasera))
                        {
                            trafiony = true;
                        }
                    }
                }
                else
                {
                    if (CheckCollisionRecs(Hit_box_gracza_w_skoku, Hit_box_lasera))
                    {
                        trafiony = true;
                    }
                }
            }

            if (trafiony && !animacjaSerceAktywna)
            {
                zycie--;                     // odejmujemy życie
                animacjaSerceAktywna = true; // uruchamiamy animację "znikającego serca"
                animacjaSerceTimer = 0.0f;   // reset timera animacji
                trafiony = false;            // reset flagi kolizji
                if (zycie <= 0)
                { // jeśli brak życia, GameOver
                    aktualnyStan = GameOver;
                    IsDead = true;
                }
            }

            // sprawdzanie hit boxu kuszy
            if (w_trakcie_bossa)
            {
                // sprawdzanie kolizji hit boxu kuszy

                if (czas_skoku == 0)
                {
                    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))
                    {
                        if (CheckCollisionRecs(Hit_box_gracza_slizg, Hit_box_kusza))
                        {
                            kusza_item.czy_podniesiona = true;
                            czy_powinna_leciec = true;
                        }
                    }
                    else
                    {

                        if (CheckCollisionRecs(Hit_box_gracza, Hit_box_kusza))
                        {
                            kusza_item.czy_podniesiona = true;
                            czy_powinna_leciec = true;
                        }
                    }
                }
                else
                {
                    if (CheckCollisionRecs(Hit_box_gracza_w_skoku, Hit_box_kusza))
                    {
                        kusza_item.czy_podniesiona = true;
                        czy_powinna_leciec = true;
                    }
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
                if (licznik_klatek_wysuwanie < dlugosc_wysuwania)
                {
                    czaszka_boss.polozenie.x -= 0.5;
                    laser_przeszkoda.polozenie.x -= 0.5;
                    Polozenie_serca_1.x -= 0.5;
                    Polozenie_serca_2.x -= 0.5;
                    Polozenie_serca_2.x -= 0.5;

                    licznik_klatek_wysuwanie++;
                }
                else if (licznik_klatek_wysuwanie <= dlugosc_wysuwania + dlugosc_czekania_az_boss_zaczie_ruszac)
                {
                    licznik_klatek_wysuwanie++;
                    pozycja_po_wysunieciu = {czaszka_boss.polozenie.x, czaszka_boss.polozenie.y};
                }
                else if (licznik_klatek_wysuwanie > dlugosc_wysuwania + dlugosc_czekania_az_boss_zaczie_ruszac)
                {
                    krencenie_sie = true;
                }
            }
            if (czaszka_boss.ilosc_hp == 0) // smierc bossa
            {

                po_bossie = true;

                ucieczka = true;

                // reset pozycji przeszkud
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

                bat.polozenie = polozenie_startowe_bat;
                Hit_box_bat.x = Polozenie_poczatkowe_hitbox_bat.x;
                Hit_box_bat.y = Polozenie_poczatkowe_hitbox_bat.y;

                szczur.polozenie = polozenie_startowe_szczura;
                Hit_box_szczura.x = Polozenie_poczatkowe_hitbox_szczura.x;
                Hit_box_szczura.y = Polozenie_poczatkowe_hitbox_szczura.y;

                szkielet.czy_jest_zatrzymany = false;
                duch.czy_jest_zatrzymany = false;
                bat.czy_jest_zatrzymany = false;
                szczur.czy_jest_zatrzymany = false;

                // reset napisu
                polozenie_napisu = poczontkowe_polozenie_napisu;
                na_dole = false;

                // reset laseru
                czaszka_boss.laser_on = false;
                Hit_box_lasera.x = Polozenie_poczatkowe_hitbox_laseru.x;
                Hit_box_lasera.y = Polozenie_poczatkowe_hitbox_laseru.y;

                krencenie_sie = false;

                distance_dla_fireballa = 0;

                fireball_przeszkoda.polozenie.x = polozenie_startowe_fireballa.x;
                fireball_przeszkoda.polozenie.y = polozenie_startowe_fireballa.y;

                Hit_box_fireball.x = offsetXfireball;
                Hit_box_fireball.y = offsetYfireball;

                czaszka_boss.ilosc_hp = 3;

                // reset kuszy
                kusza_item.polozenie.x = pozycjaX_kuszy(gen);
                Hit_box_kusza.x = offsetXkusza;

                w_trakcie_bossa = false;
            }
            if (ucieczka)
            {
                if (czaszka_boss.polozenie.x <= czaszka_boss_pozycja_poczontokowa.x)
                {
                    int animacja_smierci_czaszkai_12 = (animacja_smierci_czaszki + 1) % 12;
                    czaszka_boss.polozenie.x += 1.2;
                    if (animacja_smierci_czaszkai_12 < 3)
                    {
                        czaszka_boss.polozenie.y += 0.6;
                    }
                    else if (animacja_smierci_czaszkai_12 < 6)
                    {
                        czaszka_boss.polozenie.y += 0.6;
                    }
                    else if (animacja_smierci_czaszkai_12 < 9)
                    {
                        czaszka_boss.polozenie.y -= 0.6;
                    }
                    else
                    {
                        czaszka_boss.polozenie.y -= 0.6;
                    }
                    animacja_smierci_czaszki++;
                }
                if (czaszka_boss.polozenie.x > 1280)
                {
                    ucieczka = false;
                    transition = true;
                    po_bossie = false;
                    czaszka_boss.polozenie = czaszka_boss_pozycja_poczontokowa;
                    if (fazaSwiata == 0)
                        aktualnyStan = LEVELTWO;
                    else
                        aktualnyStan = LEVELONE;
                }
            }

            // mechanika działania fireballa
            static float angle = 0.0f;
            static Vector2 lastCirclePos = {0.0f, 0.0f};

            Vector2 center;

            if (w_trakcie_bossa && !czaszka_boss.laser_on && krencenie_sie)
            {
                angle += speed_fireballa_krencenie_z;

                center.x = polozenie_startowe_fireballa.x - (distance_dla_fireballa / 20.0f) * speed_fireballa_przod_z;
                center.y = polozenie_startowe_fireballa.y;

                Vector2 newCirclePos;
                newCirclePos.x = center.x + cos(angle) * radiusX_fireball_z;
                newCirclePos.y = center.y + sin(angle) * radiusY_fireball_z;

                Vector2 delta;
                delta.x = newCirclePos.x - lastCirclePos.x;
                delta.y = newCirclePos.y - lastCirclePos.y;

                fireball_przeszkoda.polozenie.x += delta.x;
                fireball_przeszkoda.polozenie.y += delta.y;

                Hit_box_fireball.x += delta.x;
                Hit_box_fireball.y += delta.y;

                lastCirclePos = newCirclePos;

                distance_dla_fireballa++;

                if (fireball_przeszkoda.polozenie.x < -fireball_przeszkoda.tekstura.width * skalowanie_borazu_fireball)
                {
                    if (!czaszka_boss.laser_rozgrzewanie)
                    {
                        distance_dla_fireballa = 0;
                        speed_fireballa_krencenie_z = krencenie_fireball(gen);
                        speed_fireballa_przod_z = speed_fireball(gen);
                        radiusX_fireball_z = r_fireball_x(gen);
                        radiusY_fireball_z = r_fireball_y(gen);

                        // do naprawy
                    }
                }
            }

            float t = distance * speed;

            if (w_trakcie_bossa && krencenie_sie)
            {

                czaszka_boss.polozenie.x = (czaszka_boss_pozycja_poczontokowa.x - dlugosc_wysuwania + 100) + cos(t) * radiusX;
                czaszka_boss.polozenie.y = czaszka_boss_pozycja_poczontokowa.y + sin(t) * radiusY;

                laser_przeszkoda.polozenie.x = (polozenie_startowe_laseru.x - dlugosc_wysuwania + 100) + cos(t) * radiusX;
                laser_przeszkoda.polozenie.y = polozenie_startowe_laseru.y + sin(t) * radiusY;

                Polozenie_serca_1.x = (Polozenie_poczatkowe_serca_1.x - dlugosc_wysuwania + 100) + cos(t) * radiusX;
                Polozenie_serca_1.y = Polozenie_poczatkowe_serca_1.y + sin(t) * radiusY;

                Polozenie_serca_2.x = (Polozenie_poczatkowe_serca_2.x - dlugosc_wysuwania + 100) + cos(t) * radiusX;
                Polozenie_serca_2.y = Polozenie_poczatkowe_serca_2.y + sin(t) * radiusY;

                Polozenie_serca_3.x = (Polozenie_poczatkowe_serca_3.x - dlugosc_wysuwania + 100) + cos(t) * radiusX;
                Polozenie_serca_3.y = Polozenie_poczatkowe_serca_3.y + sin(t) * radiusY;

                Hit_box_lasera.y = Polozenie_poczatkowe_hitbox_laseru.y + sin(t) * radiusY;
            }
            // przemieszczanie się napisu
            float speed_napisu = 2.0f;
            if (polozenie_napisu.y <= 0 && !na_dole && distance >= boss_fight_distance - dlugosc_trwania_znaku && !po_bossie)
            {
                if (polozenie_napisu.y == 0)
                {
                    na_dole = true;
                }

                polozenie_napisu.y += speed_napisu;
            }
            if (na_dole && polozenie_napisu.y >= -napis_o_bossie.height && distance >= boss_fight_distance)
            {
                polozenie_napisu.y -= speed_napisu;
            }

            // trzęsienie się ziemi
            int klatki_trzensienie_ziemi = (animacja + 1) % 24;
            if (distance >= boss_fight_distance - dlugosc_trzensienia_ziemi && (!szkielet.czy_jest_zatrzymany || !duch.czy_jest_zatrzymany || !szczur.czy_jest_zatrzymany || !bat.czy_jest_zatrzymany) && !po_bossie)
            {
                if (klatki_trzensienie_ziemi < 6)
                {
                    pozycja_tla_y -= moc_trznsienia_ziemi;
                }
                else if (klatki_trzensienie_ziemi < 12)
                {
                    pozycja_tla_y -= moc_trznsienia_ziemi;
                }
                else if (klatki_trzensienie_ziemi < 18)
                {
                    pozycja_tla_y += moc_trznsienia_ziemi;
                }
                else
                {
                    pozycja_tla_y += moc_trznsienia_ziemi;
                }
            }

            // skakanie
            if ((IsKeyDown(KEY_SPACE) && czas_skoku <= 0) || (IsKeyDown(KEY_W) && czas_skoku <= 0) || (IsKeyDown(KEY_UP) && czas_skoku <= 0))
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

                // logika podmiany tekstur
                tloAktualne = tloNastepne; // przesuwamy kolejke tła

                if (transition)
                {
                    tloNastepne = &backgroundBridge;
                    transition = false;
                }
                else if (tloAktualne == &backgroundBridge)
                {
                    // jesli nie ma przejscia to rysujemy dalej to samo
                    if (fazaSwiata == 0)
                    {
                        tloNastepne = &backgroundLas;
                        fazaSwiata = 3;
                    }
                    else
                    {
                        tloNastepne = &backgroundLochy;
                        fazaSwiata = 1;
                    }
                }
            }

            float game_speed = 5 + log(1 + distance / 300);

            rozstrzal_przy_losowaniu += game_speed / 100;
            minDist += game_speed / 100;

            scrollSpeed += game_speed / 400;

            uniform_int_distribution<int> pozycjaX(szerokosc_okna + szkielet_tekstura.width * skalowanie_obrazu_szkieleta, rozstrzal_przy_losowaniu);
            uniform_int_distribution<int> pozycjaX_wysoki(szerokosc_okna + duch_tekstura.width * skalowanie_obrazu_duch, rozstrzal_przy_losowaniu);

            double gs = 5.0 + log(1.0 + distance / 300.0);
            for (int i = 0; i < 4; i++)
            {
                game_speed_hamowanie[i] = gs;
            }
            // latanie ducha
            duch.polozenie.y += 2 * sin(distance / 20);
            Hit_box_ducha_1.y += 2 * sin(distance / 20);
            Hit_box_ducha_2.y += 2 * sin(distance / 20);
            Hit_box_ducha_3.y += 2 * sin(distance / 20);
            Hit_box_ducha_4.y += 2 * sin(distance / 20);

            int dodatek = 400;

            // poruszanie się boltu
            if (czy_powinna_leciec && Polozenie_bolt.x <= czaszka_boss.polozenie.x + dodatek)
            {
                boltT += boltStep;
                if (boltT > 1.0f)
                {
                    boltT = endT;
                }

                // X 0 → czaszka_boss.polozenie.x
                Polozenie_bolt.x = startX + (endX - startX) * boltT;

                // Parabola Y
                float baseY = 500.0f;
                float parabola = 4.0f * boltHeight * boltT * (1.0f - boltT);
                Polozenie_bolt.y = baseY - parabola;

                // Rotation
                float dx = czaszka_boss.polozenie.x + dodatek;
                float dy = -4.0f * boltHeight * (1.0f - 2.0f * boltT);
                boltAngle = atan2f(dy, dx) * 180.0f / PI;
            }
            if (boltT >= endT)
            {
                czy_powinna_leciec = false;
                Polozenie_bolt.x = 0;
                boltT = 0.0f;
                czaszka_boss.ilosc_hp--;
            }

            // poruszanie się kuszy
            if (krencenie_sie)
            {
                kusza_item.polozenie.x -= game_speed / 2;
                Hit_box_kusza.x -= game_speed / 2;
            }
            if (kusza_item.polozenie.x < -kusza_item.tekstura.width * skalowanie_obrazu_kuszy)
            {
                kusza_item.polozenie.x = pozycjaX_kuszy(gen);
                Hit_box_kusza.x = offsetXkusza;

                kusza_item.czy_podniesiona = false;
            }

            if (!szkielet.czy_jest_zatrzymany)
            {
                szkielet.polozenie.x -= game_speed_hamowanie[0];
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
            // reset loops
            if ((distance < boss_fight_distance - dystans_do_zatrzymania_przeszkud_gdy_boss) || po_bossie)
            {
                constexpr int max_ilosc_prob = 200;

                if ((distance < boss_fight_distance - dystans_do_zatrzymania_przeszkud_gdy_boss) || po_bossie)
                {
                    if (szkielet.polozenie.x < -szkielet_tekstura.width * skalowanie_obrazu_szkieleta)
                    {
                        int x = 0;
                        bool znaleziono = false;

                        for (int pruby = 0; pruby < max_ilosc_prob; ++pruby)
                        {
                            if (distance >= boss_fight_distance - 1000 && distance <= boss_fight_distance)
                                x = pozycja_czenste(gen);
                            else
                                x = pozycjaX(gen);

                            if (!TooCloseForSzkielet(x, minDist, duch.polozenie, bat.polozenie, szczur.polozenie))
                            {
                                znaleziono = true;
                                break;
                            }
                        }

                        if (!znaleziono)
                        {
                            float relaxed = minDist * 0.5f;
                            for (int pruby = 0; pruby < 50; ++pruby)
                            {
                                if (distance >= boss_fight_distance - 1000 && distance <= boss_fight_distance)
                                    x = pozycja_czenste(gen);
                                else
                                    x = pozycjaX(gen);

                                if (!TooCloseForSzkielet(x, relaxed, duch.polozenie, bat.polozenie, szczur.polozenie))
                                    break;
                            }
                        }

                        szkielet.polozenie.x = (float)x;
                        Hit_box_szkieleta.x = szkielet.polozenie.x + 30;
                    }

                    if (duch.polozenie.x < -duch_tekstura.width * skalowanie_obrazu_szkieleta)
                    {
                        int x = 0;
                        bool znaleziono = false;

                        for (int pruby = 0; pruby < max_ilosc_prob; ++pruby)
                        {
                            if (distance >= boss_fight_distance - 1000 && distance <= boss_fight_distance)
                                x = pozycja_czenste(gen);
                            else
                                x = pozycjaX(gen);

                            if (!TooCloseForDuch(x, minDist, szkielet.polozenie, bat.polozenie, szczur.polozenie))
                            {
                                znaleziono = true;
                                break;
                            }
                        }

                        if (!znaleziono)
                        {
                            float relaxed = minDist * 0.5f;
                            for (int pruby = 0; pruby < 50; ++pruby)
                            {
                                if (distance >= boss_fight_distance - 1000 && distance <= boss_fight_distance)
                                    x = pozycja_czenste(gen);
                                else
                                    x = pozycjaX(gen);

                                if (!TooCloseForDuch(x, relaxed, szkielet.polozenie, bat.polozenie, szczur.polozenie))
                                    break;
                            }
                        }

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
                        int x = 0;
                        bool znaleziono = false;

                        for (int pruby = 0; pruby < max_ilosc_prob; ++pruby)
                        {
                            x = pozycja_czenste(gen);

                            if (!TooCloseForBat(x, minDist, duch.polozenie, szkielet.polozenie, szczur.polozenie))
                            {
                                znaleziono = true;
                                break;
                            }
                        }

                        if (!znaleziono)
                        {
                            float relaxed = minDist * 0.5f;
                            for (int pruby = 0; pruby < 50; ++pruby)
                            {
                                x = pozycja_czenste(gen);
                                if (!TooCloseForBat(x, relaxed, duch.polozenie, szkielet.polozenie, szczur.polozenie))
                                    break;
                            }
                        }

                        bat.polozenie.x = (float)x;

                        Hit_box_bat.x = offsetXbat;

                        bat.polozenie.y = pozycjay_wysoki(gen);

                        Hit_box_bat.y = offsetYbat;
                    }

                    if (szczur.polozenie.x < -szczur_tekstura.width * skalowanie_obrazu_szczura)
                    {
                        int x = 0;
                        bool znaleziono = false;

                        for (int pruby = 0; pruby < max_ilosc_prob; ++pruby)
                        {
                            x = pozycjaX(gen);

                            if (!TooCloseForSzczur(x, minDist, duch.polozenie, szkielet.polozenie, bat.polozenie))
                            {
                                znaleziono = true;
                                break;
                            }
                        }

                        if (!znaleziono)
                        {
                            float relaxed = minDist * 0.5f;
                            for (int pruby = 0; pruby < 50; ++pruby)
                            {
                                x = pozycjaX(gen);
                                if (!TooCloseForSzczur(x, relaxed, duch.polozenie, szkielet.polozenie, bat.polozenie))
                                    break;
                            }
                        }

                        szczur.polozenie.x = (float)x;

                        Hit_box_szczura.x = offsetXszczura; // dopasuj offset
                        Hit_box_szczura.y = offsetYszczura;
                    }
                }
            }
            else if (!po_bossie)
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
            {
                if (fazaSwiata == 0) // bylo jedno i kompilator sie buntowal
                    aktualnyStan = LEVELONE;
                if (fazaSwiata == 2) // bylo jedno = i kompilator sie buntowal
                    aktualnyStan = LEVELTWO;
            }
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
        else if (aktualnyStan == GameOver) // smierc + reset
        {
            animacja++;
            if (CzyKliknietoPrzycisk(btnPowrotMenu) || CzyKliknietoPrzycisk(btnWznow)) //
            {
                UpdateTop10(nickname, distance);
                // 1. STATUS I PODSTAWY
                IsDead = false;
                distance = 0;
                scrollSpeed = 150.0f;
                timer_tekst = 0;
                startLevelOne = true;
                aktualnyStan = CzyKliknietoPrzycisk(btnPowrotMenu) ? MENU : LEVELONE;
                StartNowejGry();

                // 2. RESET GRACZA
                polozenie_gracza.y = ziemiaY;
                czas_skoku = 0;
                max_czas_skoku = 1.0f;
                Hit_box_gracza.y = Polozenie_poczatkowe_hitbox_gracza.y;
                Hit_box_gracza_w_skoku.y = Polozenie_poczatkowe_hitbox_gracza_w_skoku.y;

                // 3. RESET STANDARDOWYCH PRZESZKÓD
                szkielet.polozenie = polozenie_startowe_szkieleta;
                szkielet.czy_jest_zatrzymany = false;
                Hit_box_szkieleta.x = Polozenie_poczatkowe_hitbox_szkieleta.x;

                duch.polozenie = polozenie_startowe_ducha;
                duch.czy_jest_zatrzymany = false;
                Hit_box_ducha_1.x = Polozenie_poczatkowe_hitbox_ducha_1.x;
                Hit_box_ducha_1.y = Polozenie_poczatkowe_hitbox_ducha_1.y;
                Hit_box_ducha_2.x = Polozenie_poczatkowe_hitbox_ducha_2.x;
                Hit_box_ducha_2.y = Polozenie_poczatkowe_hitbox_ducha_2.y;
                Hit_box_ducha_3.x = Polozenie_poczatkowe_hitbox_ducha_3.x;
                Hit_box_ducha_3.y = Polozenie_poczatkowe_hitbox_ducha_3.y;
                Hit_box_ducha_4.x = Polozenie_poczatkowe_hitbox_ducha_4.x;
                Hit_box_ducha_4.y = Polozenie_poczatkowe_hitbox_ducha_4.y;

                bat.polozenie = polozenie_startowe_bat;
                bat.czy_jest_zatrzymany = false;
                Hit_box_bat.x = Polozenie_poczatkowe_hitbox_bat.x;
                Hit_box_bat.y = Polozenie_poczatkowe_hitbox_bat.y;

                szczur.polozenie = polozenie_startowe_szczura;
                szczur.czy_jest_zatrzymany = false;
                Hit_box_szczura.x = Polozenie_poczatkowe_hitbox_szczura.x;
                Hit_box_szczura.y = Polozenie_poczatkowe_hitbox_szczura.y;

                // 4. RESET BOSSA I JEGO ATAKÓW (KLUCZOWE!)
                w_trakcie_bossa = false;
                po_bossie = false;
                ucieczka = false;
                krencenie_sie = false;
                czaszka_boss.ilosc_hp = 3;
                czaszka_boss.laser_on = false;
                czaszka_boss.laser_rozgrzewanie = false;
                czaszka_boss.polozenie = czaszka_boss_pozycja_poczontokowa;

                licznik_klatek_wysuwanie = 0;
                klatka = 0;
                animacja_smierci_czaszki = 0;

                // 5. RESET FIREBALLA I MECHANIKI KOŁA
                distance_dla_fireballa = 0;
                fireball_przeszkoda.polozenie = polozenie_startowe_fireballa_prawdziwe;
                Hit_box_fireball.x = offsetXfireball;
                Hit_box_fireball.y = offsetYfireball;

                // Ważne: zresetuj parametry losowe fireballa do bazowych
                speed_fireballa_krencenie_z = speed_fireballa_krencenie;
                speed_fireballa_przod_z = speed_fireballa_przod;
                radiusX_fireball_z = radiusX_fireball;
                radiusY_fireball_z = radiusY_fireball;

                // 6. RESET PRZEDMIOTÓW I NAPISÓW
                kusza_item.czy_podniesiona = false;
                czy_powinna_leciec = false;
                boltT = 0; // reset lotu bełtu
                kusza_item.polozenie.x = (float)pozycjaX_kuszy(gen);

                polozenie_napisu = poczontkowe_polozenie_napisu;
                na_dole = false;
                pozycja_tla_y = -200; // powrót tła na miejsce po trzęsieniu ziemi

                // 7. PONOWNE LOSOWANIE SEEDU
                gen.seed(rd());
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
        // Color kolor1btnStart = SKYBLUE;
        // Color kolor2btnStart = DARKBLUE;
        // Color kolor1btnMenu = GRAY;
        // Color kolor2btnMenu = DARKGRAY;
        // Color kolor1btnExit = RED;
        // Color kolor2btnExit = DARKBROWN;

        if (aktualnyStan == MENU)
        {

            // // rysujemy tlo menu
            DrawTextureEx(background_menu, {0, 0}, 0.0f, skalowanie_obrazu_tla_menu, WHITE);
            // // rysujemy na tle gry zeby przyciemnic
            DrawRectangle(0, 0, szerokosc_okna, wysokosc_okna, Fade(BLACK, 0.3f));

            // Pole na wpisywanie nicku
            DrawRectangle(szerokosc_okna / 2 - 140, wysokosc_okna / 2 + 40, 290, 48, GRAY); // pole tekstowe
            DrawTextEx(font, nickname, {szerokosc_okna / 2 - 125, wysokosc_okna / 2 + 45}, 40, 3, BLACK);

            // rysowanie przycisku START
            Color tintStart = CzyMyszkaNadPrzyciskiem(btnStart) ? GRAY : LIGHTGRAY;
            DrawTexturePro(startText, {0, 0, (float)startText.width, (float)startText.height}, btnStart, {0, 0}, 0.0f, tintStart);

            // rysowanie przycisku WYJDZ
            Color tintExit = CzyMyszkaNadPrzyciskiem(btnExit) ? GRAY : LIGHTGRAY;
            DrawTexturePro(exitText, {0, 0, (float)exitText.width, (float)exitText.height}, btnExit, {0, 0}, 0.0f, tintExit);
            //--RYSOWANIE LEADERBOARD
            ifstream plikPokaz("top10.txt");
            string n;
            int d;
            int yOffset = 0;

            while (plikPokaz >> n >> d && yOffset < 10)
            {
                DrawTextEx(font, TextFormat("%d. %s - %d", yOffset + 1, n.c_str(), d), {50, (float)120 + (yOffset * 25)}, 25, 3, DARKGRAY);
                yOffset++;
            }
            plikPokaz.close();
        }
        else if (aktualnyStan == LEVELONE || aktualnyStan == PAUZA || aktualnyStan == GameOver) // gre rysujemy nawet jak jestesmy w pauzie
        {
            int frameCount = (animacja + 1) % 60;

            // for (float x = scrollingBack; x < szerokosc_okna; x += szerokosc_tla_przeskalowana)
            //{
            DrawTextureEx(*tloAktualne, {scrollingBack, (float)pozycja_tla_y}, 0.0f, skalowanie_obrazu_tla, WHITE);
            DrawTextureEx(*tloNastepne, {scrollingBack + szerokosc_tla_przeskalowana, (float)pozycja_tla_y}, 0.0f, skalowanie_obrazu_tla, WHITE);
            //}

            // rysowanie przeciwników w zależności od tego czy jest boss_tekstura fight czy go niema

            if (IsDead == false)
            {
                // rysowanie bossa

                DrawTextureEx(napis_o_bossie, polozenie_napisu, 0.0f, 0.45, WHITE);

                // rysowanie kuszy
                if (w_trakcie_bossa && !kusza_item.czy_podniesiona)
                {
                    DrawTextureEx(kusza_item.tekstura, kusza_item.polozenie, 0.5f, skalowanie_obrazu_kuszy, GRAY);
                }

                // rysowanie bossa w trakcie ucieczki
                if (ucieczka)
                {
                    DrawTextureEx(czaszka_boss.tekstury[3], czaszka_boss.polozenie, 0.0f, skalowanie_obrazu_boss, GRAY);
                }

                if (szkielet.czy_jest_zatrzymany && duch.czy_jest_zatrzymany && bat.czy_jest_zatrzymany && szczur.czy_jest_zatrzymany)
                {
                    int Animacja_lasera = (klatka + 1) % co_ile_laser;
                    int Animacja_jedzenia = (klatka + 1) % 60;
                    int Animacja_migania_laseru = (klatka + 1) % 9;
                    w_trakcie_bossa = true;

                    if (co_ile_laser / 2 <= Animacja_lasera && Animacja_lasera <= co_ile_laser / 2 + wind_up_laseru)
                    {
                        DrawTextureEx(czaszka_boss.tekstury[1], czaszka_boss.polozenie, 0.0f, skalowanie_obrazu_boss, GRAY);
                        czaszka_boss.laser_rozgrzewanie = true;
                    }
                    else if (co_ile_laser / 2 + wind_up_laseru <= Animacja_lasera && Animacja_lasera <= co_ile_laser / 2 + wind_up_laseru + czas_trwania_laseru)
                    {
                        // rysowanie bolta

                        if (czy_powinna_leciec)
                        {
                            Vector2 pozycja_rysowania_bolta;
                            pozycja_rysowania_bolta.x = Polozenie_bolt.x - (bolt.width * skalowanie_obrazu_bolt) * 0.5f;
                            pozycja_rysowania_bolta.y = Polozenie_bolt.y - (bolt.height * skalowanie_obrazu_bolt) * 0.5f;

                            DrawTextureEx(bolt, pozycja_rysowania_bolta, boltAngle, skalowanie_obrazu_bolt, BLACK);
                        }
                        czaszka_boss.laser_rozgrzewanie = false;
                        czaszka_boss.laser_on = true;
                        DrawTextureEx(czaszka_boss.tekstury[2], czaszka_boss.polozenie, 0.0f, skalowanie_obrazu_boss, GRAY);
                        if (Animacja_migania_laseru < 3)
                        {
                            DrawTextureEx(laser_przeszkoda.tekstura, laser_przeszkoda.polozenie, 0.0f, skalowanie_obrazu_laseru, YELLOW);
                        }
                        else if (Animacja_migania_laseru < 6)
                        {
                            DrawTextureEx(laser_przeszkoda.tekstura, laser_przeszkoda.polozenie, 0.0f, skalowanie_obrazu_laseru, RED);
                        }
                        else
                        {
                            DrawTextureEx(laser_przeszkoda.tekstura, laser_przeszkoda.polozenie, 0.0f, skalowanie_obrazu_laseru, WHITE);
                        }
                    }
                    else
                    {
                        if (Animacja_jedzenia < 30)
                        {
                            czaszka_boss.laser_on = false;
                            DrawTextureEx(czaszka_boss.tekstury[3], czaszka_boss.polozenie, 0.0f, skalowanie_obrazu_boss, GRAY);
                        }
                        else
                        {
                            DrawTextureEx(czaszka_boss.tekstury[0], czaszka_boss.polozenie, 0.0f, skalowanie_obrazu_boss, GRAY);
                        }
                    }
                    // rysowanie serc bossa
                    if (krencenie_sie)
                    {
                        int animacja_na_7_serce = (animacja_na_7 % 70);

                        if (czaszka_boss.ilosc_hp == 3)
                        {
                            if (animacja_na_7_serce < 10)
                            {
                                DrawTextureEx(serce_bossa, Polozenie_serca_1, 0.0f, skalowanie_obrazu_serca_bossa * 0.9, WHITE);
                                DrawTextureEx(serce_bossa, Polozenie_serca_2, 0.0f, skalowanie_obrazu_serca_bossa * 0.9, WHITE);
                                DrawTextureEx(serce_bossa, Polozenie_serca_3, 0.0f, skalowanie_obrazu_serca_bossa * 0.9, WHITE);
                            }
                            else if (animacja_na_7_serce < 20)
                            {
                                DrawTextureEx(serce_bossa, Polozenie_serca_1, 0.0f, skalowanie_obrazu_serca_bossa, WHITE);
                                DrawTextureEx(serce_bossa, Polozenie_serca_2, 0.0f, skalowanie_obrazu_serca_bossa, WHITE);
                                DrawTextureEx(serce_bossa, Polozenie_serca_3, 0.0f, skalowanie_obrazu_serca_bossa, WHITE);
                            }
                            else if (animacja_na_7_serce < 30)
                            {
                                DrawTextureEx(serce_bossa, Polozenie_serca_1, 0.0f, skalowanie_obrazu_serca_bossa * 1.1, WHITE);
                                DrawTextureEx(serce_bossa, Polozenie_serca_2, 0.0f, skalowanie_obrazu_serca_bossa * 1.1, WHITE);
                                DrawTextureEx(serce_bossa, Polozenie_serca_3, 0.0f, skalowanie_obrazu_serca_bossa * 1.1, WHITE);
                            }
                            else if (animacja_na_7_serce < 40)
                            {
                                DrawTextureEx(serce_bossa, Polozenie_serca_1, 0.0f, skalowanie_obrazu_serca_bossa * 1.2, WHITE);
                                DrawTextureEx(serce_bossa, Polozenie_serca_2, 0.0f, skalowanie_obrazu_serca_bossa * 1.2, WHITE);
                                DrawTextureEx(serce_bossa, Polozenie_serca_3, 0.0f, skalowanie_obrazu_serca_bossa * 1.2, WHITE);
                            }
                            else if (animacja_na_7_serce < 50)
                            {
                                DrawTextureEx(serce_bossa, Polozenie_serca_1, 0.0f, skalowanie_obrazu_serca_bossa * 1.1, WHITE);
                                DrawTextureEx(serce_bossa, Polozenie_serca_2, 0.0f, skalowanie_obrazu_serca_bossa * 1.1, WHITE);
                                DrawTextureEx(serce_bossa, Polozenie_serca_3, 0.0f, skalowanie_obrazu_serca_bossa * 1.1, WHITE);
                            }
                            else
                            {
                                DrawTextureEx(serce_bossa, Polozenie_serca_1, 0.0f, skalowanie_obrazu_serca_bossa, WHITE);
                                DrawTextureEx(serce_bossa, Polozenie_serca_2, 0.0f, skalowanie_obrazu_serca_bossa, WHITE);
                                DrawTextureEx(serce_bossa, Polozenie_serca_3, 0.0f, skalowanie_obrazu_serca_bossa, WHITE);
                            }
                            animacja_na_7++;
                        }
                        else if (czaszka_boss.ilosc_hp == 2)
                        {
                            if (animacja_na_7_serce < 10)
                            {
                                DrawTextureEx(serce_bossa, Polozenie_serca_1, 0.0f, skalowanie_obrazu_serca_bossa * 0.9, WHITE);
                                DrawTextureEx(serce_bossa, Polozenie_serca_2, 0.0f, skalowanie_obrazu_serca_bossa * 0.9, WHITE);
                            }
                            else if (animacja_na_7_serce < 20)
                            {
                                DrawTextureEx(serce_bossa, Polozenie_serca_1, 0.0f, skalowanie_obrazu_serca_bossa, WHITE);
                                DrawTextureEx(serce_bossa, Polozenie_serca_2, 0.0f, skalowanie_obrazu_serca_bossa, WHITE);
                            }
                            else if (animacja_na_7_serce < 30)
                            {
                                DrawTextureEx(serce_bossa, Polozenie_serca_1, 0.0f, skalowanie_obrazu_serca_bossa * 1.1, WHITE);
                                DrawTextureEx(serce_bossa, Polozenie_serca_2, 0.0f, skalowanie_obrazu_serca_bossa * 1.1, WHITE);
                            }
                            else if (animacja_na_7_serce < 40)
                            {
                                DrawTextureEx(serce_bossa, Polozenie_serca_1, 0.0f, skalowanie_obrazu_serca_bossa * 1.2, WHITE);
                                DrawTextureEx(serce_bossa, Polozenie_serca_2, 0.0f, skalowanie_obrazu_serca_bossa * 1.2, WHITE);
                            }
                            else if (animacja_na_7_serce < 50)
                            {
                                DrawTextureEx(serce_bossa, Polozenie_serca_1, 0.0f, skalowanie_obrazu_serca_bossa * 1.1, WHITE);
                                DrawTextureEx(serce_bossa, Polozenie_serca_2, 0.0f, skalowanie_obrazu_serca_bossa * 1.1, WHITE);
                            }
                            else
                            {
                                DrawTextureEx(serce_bossa, Polozenie_serca_1, 0.0f, skalowanie_obrazu_serca_bossa, WHITE);
                                DrawTextureEx(serce_bossa, Polozenie_serca_2, 0.0f, skalowanie_obrazu_serca_bossa, WHITE);
                            }
                            animacja_na_7++;
                        }
                        else if (czaszka_boss.ilosc_hp == 1)
                        {
                            if (animacja_na_7_serce < 10)
                            {

                                DrawTextureEx(serce_bossa, Polozenie_serca_2, 0.0f, skalowanie_obrazu_serca_bossa * 0.9, WHITE);
                            }
                            else if (animacja_na_7_serce < 20)
                            {

                                DrawTextureEx(serce_bossa, Polozenie_serca_2, 0.0f, skalowanie_obrazu_serca_bossa, WHITE);
                            }
                            else if (animacja_na_7_serce < 30)
                            {

                                DrawTextureEx(serce_bossa, Polozenie_serca_2, 0.0f, skalowanie_obrazu_serca_bossa * 1.1, WHITE);
                            }
                            else if (animacja_na_7_serce < 40)
                            {

                                DrawTextureEx(serce_bossa, Polozenie_serca_2, 0.0f, skalowanie_obrazu_serca_bossa * 1.2, WHITE);
                            }
                            else if (animacja_na_7_serce < 50)
                            {

                                DrawTextureEx(serce_bossa, Polozenie_serca_2, 0.0f, skalowanie_obrazu_serca_bossa * 1.1, WHITE);
                            }
                            else
                            {

                                DrawTextureEx(serce_bossa, Polozenie_serca_2, 0.0f, skalowanie_obrazu_serca_bossa, WHITE);
                            }
                            animacja_na_7++;
                        }
                    }

                    if (aktualnyStan != PAUZA)
                    {
                        if (w_trakcie_bossa)
                        {
                            klatka++;
                        }
                    }
                    // rysowanie fireballa
                    if (krencenie_sie)
                    {
                        DrawTextureEx(fireball_przeszkoda.tekstura, fireball_przeszkoda.polozenie, 0.0f, skalowanie_borazu_fireball, WHITE);
                    }
                    // rysowanie bolta

                    if (czy_powinna_leciec)
                    {
                        Vector2 pozycja_rysowania_bolta;
                        pozycja_rysowania_bolta.x = Polozenie_bolt.x - (bolt.width * skalowanie_obrazu_bolt) * 0.5f;
                        pozycja_rysowania_bolta.y = Polozenie_bolt.y - (bolt.height * skalowanie_obrazu_bolt) * 0.5f;

                        DrawTextureEx(bolt, pozycja_rysowania_bolta, boltAngle, skalowanie_obrazu_bolt, WHITE);
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
            if (czas_skoku == 0)
            {
                if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))
                {
                    rysowanie_hit_box(Hit_box_gracza_slizg, RED);
                }
                else
                    rysowanie_hit_box(Hit_box_gracza, RED);
            }
            else
            {
                rysowanie_hit_box(Hit_box_gracza_w_skoku, RED);
            }

            rysowanie_hit_box(Hit_box_szkieleta, GREEN);
            rysowanie_hit_box(Hit_box_ducha_1, GREEN);

            rysowanie_hit_box(Hit_box_ducha_2, GREEN);
            rysowanie_hit_box(Hit_box_ducha_3, GREEN);
            rysowanie_hit_box(Hit_box_ducha_4, GREEN);

            rysowanie_hit_box(Hit_box_bat, GREEN);

            rysowanie_hit_box(Hit_box_szczura, GREEN);
            rysowanie_hit_box(Hit_box_lasera, GREEN);

            rysowanie_hit_box(Hit_box_fireball, GREEN);

            rysowanie_hit_box(Hit_box_kusza, GREEN);

            // istotne prosze nie usuwać !!!!

            // animacja szkieleta

            // Wyświetlanie fabularnego tekstu
            if (timer_tekst > 0.0f)
            {
                int szerokoscTekstu = MeasureText(tekst_fabularny.c_str(), 24);

                // --- nowy dodany prostokąt pod tekstem ---
                // DrawRectangle(
                //     szerokosc_okna / 2 - szerokoscTekstu / 2 - 10, // x
                //     100 - 5,                                       // y
                //     szerokoscTekstu + 20,                          // szerokość
                //     34,                                            // wysokość (wysokość tekstu + padding)
                //     Fade(BLACK, 0.5f)                              // kolor półprzezroczysty
                // );
                DrawTextEx(font, tekst_fabularny.c_str(), {(float)szerokosc_okna / 2 - szerokoscTekstu / 2}, 50, 3, RED);
                timer_tekst -= GetFrameTime();
            }

            DrawTextEx(font, "Dungeons", {10, 10}, 30, 3, GRAY);
            DrawTextEx(font, TextFormat("DYSTANS: %05d", distance), {szerokosc_okna - 200, 10}, 30, 3, GRAY);

            if (aktualnyStan == PAUZA)
            {
                // Nakładamy lekki przyciemniający filtr na ekran gry
                DrawRectangle(0, 0, szerokosc_okna, wysokosc_okna, Fade(BLACK, 0.5f));
                DrawTextEx(font, "PAUZA", {szerokosc_okna / 2 - 50, 150}, 40, 3, WHITE);

                Color tintWznow = CzyMyszkaNadPrzyciskiem(btnWznow) ? GRAY : LIGHTGRAY;
                DrawTexturePro(wznowText, {0, 0, (float)wznowText.width, (float)wznowText.height}, btnWznow, {0, 0}, 0.0f, tintWznow);

                Color tintPowrotMenu = CzyMyszkaNadPrzyciskiem(btnPowrotMenu) ? GRAY : LIGHTGRAY;
                DrawTexturePro(menuText, {0, 0, (float)menuText.width, (float)menuText.height}, btnPowrotMenu, {0, 0}, 0.0f, tintPowrotMenu);

                Color tintPauzaExit = CzyMyszkaNadPrzyciskiem(btnPauzaExit) ? GRAY : LIGHTGRAY;
                DrawTexturePro(exitText, {0, 0, (float)exitText.width, (float)exitText.height}, btnPauzaExit, {0, 0}, 0.0f, tintPauzaExit);
            }
            else if (aktualnyStan == GameOver)
            {

                DrawRectangle(0, 0, szerokosc_okna, wysokosc_okna, Fade(BLACK, 0.5f));
                DrawTextEx(font, "GAME OVER", {szerokosc_okna / 2 - 130, 100}, 50, 3, GRAY);

                int frameCounter = (animacja + 1) % 30;

                if (frameCounter < 15)
                {
                    DrawTextureEx(czaszka, {szerokosc_okna / 2 - 105, 420}, 0.0f, 0.2, GRAY);
                }
                else
                {
                    DrawTextureEx(czaszka_2, {szerokosc_okna / 2 - 105, 420}, 0.0f, 0.2, GRAY);
                }

                DrawTextEx(font, TextFormat("KONCOWY DYSTANS: %05d", distance), {szerokosc_okna / 2 - 280, 160}, 50, 3, GRAY);

                Color tintRetry = CzyMyszkaNadPrzyciskiem(btnWznow) ? GRAY : LIGHTGRAY;
                DrawTexturePro(retryText, {0, 0, (float)retryText.width, (float)retryText.height}, btnWznow, {0, 0}, 0.0f, tintRetry);

                Color tintPowrotMenu = CzyMyszkaNadPrzyciskiem(btnPowrotMenu) ? GRAY : LIGHTGRAY;
                DrawTexturePro(menuText, {0, 0, (float)menuText.width, (float)menuText.height}, btnPowrotMenu, {0, 0}, 0.0f, tintPowrotMenu);

                Color tintPauzaExit = CzyMyszkaNadPrzyciskiem(btnPauzaExit) ? GRAY : LIGHTGRAY;
                DrawTexturePro(exitText, {0, 0, (float)exitText.width, (float)exitText.height}, btnPauzaExit, {0, 0}, 0.0f, tintPauzaExit);
            }

            for (int i = 0; i < zycie; i++)
            {
                float alpha = 1.0f; // pełna widoczność

                if (animacjaSerceAktywna)
                {
                    float czasNaSerce = animacjaSerceTimer - i * 0.1f;
                    if (czasNaSerce > 0)
                    {
                        alpha = 1.0f - (czasNaSerce / 0.5f);
                        if (alpha < 0.0f)
                            alpha = 0.0f;
                    }
                }

                DrawTexturePro(
                    serceTexture,
                    (Rectangle){0, 0, (float)serceTexture.width, (float)serceTexture.height},
                    (Rectangle){
                        (float)(startX + i * (szerokoscSerca + odstep)),
                        (float)startY,
                        (float)szerokoscSerca,
                        (float)wysokoscSerca},
                    (Vector2){0, 0},
                    0.0f,
                    Fade(WHITE, alpha));
            }

            // Aktualizacja timera animacji
            if (animacjaSerceAktywna)
            {
                animacjaSerceTimer += GetFrameTime();
                if (animacjaSerceTimer >= 0.5f + zycie * 0.1f)
                {
                    animacjaSerceAktywna = false;
                    animacjaSerceTimer = 0.0f;
                }
            }
        }
        EndDrawing();
    }

    UnloadTexture(czaszka_2);
    UnloadTexture(czaszka);
    UnloadTexture(slizg);
    UnloadTexture(background_menu);
    UnloadTexture(tekstura_skoku);
    UnloadTexture(backgroundLochy);
    UnloadTexture(backgroundLas);
    UnloadTexture(backgroundBridge);
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