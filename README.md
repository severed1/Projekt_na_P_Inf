# Dungeon Run
Projekt zawiera grę pt "Dungeon Run" typu endless runner. Wykorzystuje bibliotekę raylib 5.5, dlatego należy mieć ją zainstalowaną na urządzeniu.

Instalacja raylib na Windows:
Należy pobrać odpowiedni zip z poniższej strony:

https://github.com/raysan5/raylib/releases/tag/5.5

Wersja pobierana zależy od kompilatora, my korzystaliśmy z g++. Następnie należy umieścić folder raylib w wybranym miejscu i zapisać do niego ścieżkę w pliku c_cpp_properies.json w miejscach:

"includePath": [
                "${workspaceFolder}/**",
                "C:/raylib/include"
            ],
"browse": {
                "path": [
                    "C:/raylib/include"
                ]
            }
Program powinien wtedy widzieć raylib, gdy go includujemy w pliku gry.

Instalacja raylib na ubuntu:
`sudo apt update`
`sudo apt install libraylib-dev`

Kompilacja gry:
`g++ main_file.cpp -o game -no-pie $(pkg-config --cflags --libs raylib)`
W przypadku, gdy program nie widzi raylib.h można użyć komendy:
`g++ main_file.cpp -o 'nazwa_gry' -I 'sciezka_do_raylib_include' -L 'sciezka_do_raylib_lib' -lraylib -lopengl32 -lgdi32 -lwinmm`

Po kompilacji uruchamiamy gre plikiem .exe, w naszym przypadku gra.exe.

Komendy do gita: 

`git status` - sprawdzanie zmian 

`git commit -a -m "wiadomosc"` - commituje zmiany na lokalnego brancha

`git push` - wrzuca zmiany na server 

`git pull` - zciąga zmiany na lokalne urządzenie 

`git add .` - Pozwala nam upewnić się że wszystkie chciane zmiany zostaną dołączone do naszego commita
