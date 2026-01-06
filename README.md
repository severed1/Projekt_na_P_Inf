# Projekt_na_P_Inf
Jak bedziecie chceli to kompilowac to musicie uzyc takiej komedy 
`g++ main_file.cpp -o game -no-pie $(pkg-config --cflags --libs raylib)`
i do tego musicie miec zainstalowaną ta biblioteke raylib.h na ubuntu robi się to tak 
`sudo apt update`
`sudo apt install libraylib-dev`

Komendy do gita: 

`git status` - sprawdzanie zmian 

`git commit -a -m "wiadomosc"` - commituje zmiany na lokalnego brancha

`git push` - wrzuca zmiany na server 

`git pull` - zciąga zmiany na lokalne urządzenie 

Pozycja (0, 0) z tego co rozumiem jest w lewym górnym rogu okna

W folderze background jest przykładowe tło (wymiary ma 480szer/180wys wiec mozna znalezc jakies troche wieksze do testow), jak już ustalimy co i jak to poszukamy takiego jakie bedzie pasowało do gry, dla osobnych leveli możemy dawać inne tła przykładowo.
Tło działa w taki sposób że jest grafika tak narysowana ze jakby ciągnie się w nieskończonośc jeżeli ustawimy kolejne kopie koło siebie (to sie nazywa obraz parallax bodajże). W grze możemy po porstu operować na dwóch tłach - jedno dojdzie do konca to od razu zaczyna się drugie a to pierwsze zostaje przeniesione za drugie i tak w nieskończoność, to daje wrażenie że postać biegnie a tak naprwde stoi w miejscu i tylko tło sie rusza.

Menu jest nie dopasowane bo musiałem je zmienić z powodu zmiany tła gry i nie miałem czasu dzisiaj już go naprawić 

Czasami gra może się zawieszać/nie odpowiedać popracuje nad tym jutro wszystko inne powinno być ok jak macie jakieś problemy do zgłoszenia to możecie też to robić na githubie w sekcji Issues

pousuwałem stare grafiki ale jeśli z jakiegoś powodu będziecie ich potrzebowali to może je sciągnąć ze starych commitów 

jak dla mnie to baza poziomu 1 jest już gotowa teraz trzebasię zabrać za 1 boss fight (mozna by lekko poprawić niektóre hit boxy ale mi się narazie nie chce)


Plik zrobił się duzy wiec polecam korzystac z `CTRL + f czylil` wyszukiwania słów kluczowych (mozna sobie dodwac w komentazu i odrazu znajdowac) i `CTRL + left mouse` czyli przesuwasz sie do definicji a `CTRL + ALT + "-"` wraca tam gdzie byliscie wczesniej

moze niedziałac restart bo smierci, czasami przeszkody moga sie pojawiac za blisko 