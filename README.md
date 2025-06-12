# Systemy operacyjne 2 - projekt

Celem zajęć jest wykonanie aplikacji wykorzystującej wielowątkowość oraz wykorzystanie mechanizmów zabezpieczających sekcje krytyczne, czyli mutexów, semaforów.

## Instrukcja uruchomienia projektów

Każdy projekt znajduje się w osobnym folderze, w który należy wejść.

### Projekt 1
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

### Projekt 2
1. Sprawdź obecność pythona poprzez wpisanie w terminalu:
    ```bash
    which python
    ```
    Jeśli zobaczysz ścieżkę, to znaczy, że python jest zainstalowany. Jeśli nie to zainstaluj z [oficjalnej strony](https://www.python.org/downloads).
2. Utwórz środowisko wirtualne `.venv`:
  * Na Ubuntu:
    ```bash
    python3 -m venv .venv
    . venv/bin/activate
    ```
  * Na Windowsie:
      ```bash
      python -m venv .venv
      .venv\Scripts\activate
      ```
3. Uruchom program za pomocą:
    ```bash
    python ./server.py
    python ./client.py
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


## Projekt 2 - Wielowątkowy serwer czatu

### Opis projektu

Należało zaimplementować Wielowątkowy serwer czatu, gdzie:
* Serwer tworzy osobny wątek dla każdego połączenia od klienta
* Serwer dba o synchronizację wiadomości od klientów
* Klient widzi wiadomości w chacie
* Klient ma możliwość wysyłania wiadomości

### Wątki

Serwer (`server.py`) tworzy osobny wątek dla każdego nowego klienta. Dzięki temu wielu klientów może być obsługiwanych równolegle bez blokowania głównego wątku `accept()`.
Klient (`client.py`) korzysta z dwóch wątków:
- główny wątek: pobiera dane od użytkownika (`input()`),
- drugi wątek: w tle odbiera wiadomości z serwera i je wyświetla.

### Sekcje krytyczne

Lista `clients[]` w serwerze zawiera aktywne połączenia. Ponieważ wiele wątków może modyfikować tę listę jednocześnie (klienci mogą dodać siebie do listy (`append`), usunąć siebie (`remove`) lub wysyłać wiadomość innym — czyli czytać listę (`for client in clients`)), 
jej obsługa wymaga synchronizacji. Dlatego w tych fragmentach skorzystano z `with clients_lock:` dla `clients_lock = threading.Lock()`. Lock zapewnia, że tylko jeden wątek naraz ma dostęp do chronionej sekcji kodu. Konstrukcja `with clients_lock:` działa w ten sposób, że wątek, który pierwszy wejdzie w sekcję krytyczną, blokuje dostęp innym wątkom, aż zakończy wykonywanie tej części kodu.
