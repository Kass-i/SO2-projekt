# Systemy operacyjne 2 - projekt

Celem zajęć jest wykonanie aplikacji wykorzystującej wielowątkowość oraz wykorzystanie mechanizmów zabezpieczających sekcje krytyczne, czyli mutexów, semaforów.

## Instrukcja uruchomienia projektów

Każdy projekt znajduje się w osobnym folderze, w który należy wejść. Znajdują się w nich pliki Makefile.
1. Sprawdź obecność narzędzia `make` poprzez wpisanie w terminalu:
    ```bash
    make --version
    ```
    Jeśli zobaczysz numer wersji, to znaczy, że make jest zainstalowany. Jeśli `make` nie jest zainstalowane:
  * Na Ubuntu:
    ```bash
    sudo apt update
    sudo apt install build-essential
    ```
  * Na Windowsie:
    * Zainstaluj [MSYS2](https://www.msys2.org/).
    * Otwórz MSYS2 MinGW i wpisz:
      ```bash
      pacman -S make
      ```
2. Zbuduj projekt za pomocą:
   ```bash
   make
   ```
3. Uruchom program za pomocą:
    ```bash
    ./nazwa_programu {liczba_filozofów}
    ```
4. Wyczyść pliki za pomocą:
    ```bash
    make clean
    ```

## Projekt 1 - Problem jedzących filozofów

### Opis problemu

Przy okrągłym stole zasiada N filozofów. Każdy z nich może albo jeść, albo rozmyślać. Pomiędzy każdą parą filozofów znajduje się jeden widelec. 
Podczas jedzenia filozof musi skorzystać z dwóch widelców - lewego i prawego oraz nie ma możliwości skorzystania z widelca, który nie leży bezpośrednio przy nim.
Filozofowie nie rozmawiają ze sobą, więc może pojawić się sytuacja zakleszczenia (deadlock), czyli każdy z nich weźmie lewy widelec i będzie czekał w nieskończoność na prawy.

### Wątki

Każdy wątek reprezentuje jednego filozofa. Filozof myśli, podnosi widelce, je lub odkłada widelce. Wątki przechowywane są w wektorze `vector<thread> philosophersThreads`.

### Sekcje krytyczne

Sekcja krytyczna jest fragmentem kodu, w którym wątki (lub procesy) odwołują się do wspólnego zasobu. Wiążą się z tym pojęciem różne zagrożenia, takie jak:
* jednoczesna modyfikacja zasobu - nieprzewidywalny, niepoprawny wynik,
* zakleszczenia (deadlock) - blokada wątku w nieskończoność,
* zagłodzenie (starvation) - jeden z wątków nigdy nie uzyska dostępu do sekcji krytycznej - inne wątki zawsze ją blokują.

Stosując odpowiednie mechanizmy synchronizacji (jak mutexy, semafory), można bezpiecznie zarządzać dostępem do sekcji krytycznych w programach wielowątkowych.

W problemie jedzących filozofów można wyróżnić dwie sekcje krytyczne związane z:
1. Wypisywaniem stanów filozofów - `std::cout` nie zabezpiecza wyjścia przed jednoczesnym zapisem spowodowanym przez wątki - tekst może się nałożyć lub być wybrakowany.
   Dlatego dzięki `lock_guard<mutex> coutLock(outputMtx);` wątek blokuje pozostałym wątkom możliwość wypisywania statusu i odblokowuje ją po własnym wypisaniu tekstu. Gwarantuje to czytelność i poprawność wyjścia na ekran.
2. Ruchem widelców - nieakceptowalna jest sytuacja, w której dwóch filozofów jednocześnie podnosi ten sam widelec.
   Aby zapobiec anomaliom związanym ze stanem widelców, użyto  `lock_guard<mutex> forkLock{forksMtx};`, przez co podczas odkładania oraz próby podniesienia widelców tylko jeden filozof może ingerować w zmianę ich pozycji.

### Rozwiązanie problemu trwałego zablokowania wątków

Istnieją różne rozwiązania w celu eliminacji `deadlock`'ów w problemie jedzących filozofów. Jednym z nich jest [Rozwiązanie Dijkstry](https://en.wikipedia.org/wiki/Dining_philosophers_problem#Dijkstra's_solution).
Polega ono na tym, że filozof nie może trzymać w ręce jednego widelca - podnosi jednocześnie oba lub czeka do momentu zwolnienia potrzebnych widelców. 
Implementacja polega na przypisaniu każdemu filozofowi stanu (Myśli, Głoduje, Je) oraz na zdefiniowaniu semafora binarnego, który kontroluje dostęp do obu widelców naraz.

### Działanie semaforów binarnych

* Gdy filozof jest głodny próbuje podnieść oba widelce w `takeForks()`.
  * Sprawdza w `checkPhilosopher()` czy jego sąsiedzi nie jedzą, czyli czy są wolne oba widelce. Jeśli tak, to semafor zostaje otwarty poprzez `release()`, w przeciwnym wypadku nic się w tej funkcji nie dzieje.
  * Następnie poprzez `acquire()` następuje próba zdobycia widelców - jeśli w poprzednim kroku semafor został zwolniony (czyli możliwe jest podniesienie widelców) to przejmuje zasób, a w przeciwnym wypadku czeka, aż zasób będzie zwolniony.
* Gdy filozof kończy jedzenie to w `putDownForks()` następuje sprawdzenie sąsiadów poprzez `checkPhilosopher()`, czyli czy są głodni i mają zwolniony drugi widelec.
  Dzięki temu sąsiedzi od razu po zwolnieniu widelców mogą zabrać się za jedzenie, dzięki czemu pozbyto się problemu zagłodzenia.
