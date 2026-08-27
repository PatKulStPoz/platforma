# Platforma Jeżdząca
Dokumentacja systemu kontroli.

## Wspierane połączenie.
### UART po USB:
Platforma wspiera podawanie komend po przez UART po połączeniu USB.
Dodatkowo na nim wyświetlać mogą sie dodatkowe informacje o stanie platformy.
Odpowiedzi do komend są prefixowane po przez `[CMD] `.

### Serwer WWW pod portem `80`:
Dostępny jest serwer WWW z prostym interfacem wyświetlającym informacje,
udostępniającym przyciski dla wybranych prostych komend oraz oknem terminala z możliwością wysłania dowolnej komendy.

### Serwer tekstowy pod portem `2000`:
Jest to prosty serwer, który używa surowego protokołu tekstowego.
Tzn jako informacje wysyłasz komendy jako surowy tekst, 
a otrzymujesz tekst będący odpowiedzą na to.

Można podłączyć się do niego za pomocą clienta Telnet (nie jest to jednak kompatybilny z właściwym protokołem telnet).
Wysłana sekwencja nowej lini jest równa `\r\n`, a komenda jest wywołana tylko po otrzymaniu znaku `\n`. Inne znaki kontrolne są ignorowane.
W celu programowym można wykorzystać zwykły socket.

## Komendy:
- `help` - Wbudowany opis komend.
- `start` - Startuje koła/silniki platformy.
- `stop` - Zatrzymuje koła/silniki.
- `setlevel [<0;100>]` - Ustawia poziom mocy kół w procentach (od 0 do 100%).
- `setdirection [forward/backward]` - Ustawia kierunek koła (jeśli wspierane).
- `setbrake [true/false]` - Zaciąga / puszcza hamulce (jeśli wspierane).
- `behavior ...` - Ustawia zachowanie kół.
  - `behavior default` - Ustawia zachowanie kół na domyślne / niezależne.
  - `behavior sync` - Ustawia zachowanie kół na zsynchronizowane / zależne.
  - `behavior clear` - Czyści zachowanie (tak samo jak default).
- `rotate [liczba całkowita, kąt]` - Obraca koło o podany kąt (w limicie dokładności zależnym od l. ticków czujnika halla).
- `wait [czas, w sekundach]` - Czeka X sekund zanim wykona kolejne zadanie.
- `waitfor` - Czeka aż zachowanie zostanie wykonane (np. obrót o 360 stopni).
- `state` - Wyświetla status koła.
- `reset` - Resetuje ustawienia koła.
- `cleartask` - Czyści wszystkie oczekujące polecenia.
- `esp32.reboot` - Restartuje mikrokontroler esp32 (płytkę sterującą).
- `uart.echo [true/false]` - Przełącza ustawienia echa po połączeniu uart.
- `battery.status` - Wyświetla stan bateri.
- `config ...` - Konfiguruje płytkę.
  - `config print` - Wyświetla aktualny stan konfiguracji.
  - `config save` - Zapisuje aktualne ustawienia (w innym wypadku będą wyczyszczone przy starcie).
  - `config load` - Wczytuje wcześniej zapisane ustawienia.
  - `config set ...` - Ustawia wartość...
    - `config set hall_ticks [ilość]` - Ustawia liczbę ticków z czujnika halla, która wskazuje pełen obrót.
    - `config set driver_ticks [ilość]` - Ustawia liczbę ticków z sterownika koła, która wskazuje pełen obrót.
    - `config set has_brake [true/false]` - Ustawia, czy wspierany jest hamulec.
    - `config set allow_backwards [true/false]` - Ustawia, czy wspierane jest kręceniem kołem do tyłu.
    - `config set scale_level [<0;100>]` - Ustawia mnożnik poziomu mocy kół, w procentach.
    - `config set wifi_ssid [ssid]` - Ustawia ssid sieci wifi do której ma się połączyć / hotspota. Puste, aby wyłączyć.
    - `config set wifi_password [hasło]` - Ustawia hasło sieci wifi do której ma się połączyć / hotspota.
    - `config set wifi_ap [true/false]` - Ustawia czy płytka powinna tworzyć własną sieć wifi.
    - `config set hostname [nazwa]` - Hostname, na jaki reklamuje się płytka. (Tzn nazwa domeny hostname.local do której da się podłączyć).

W celu wybrania koła, należy dodać prefix `l:` dla lewego oraz `r:` dla prawego, przed podaną komendą (np. `l:setbrake true`). 
Komendy mogą być rozdzielone znakiem `;`, jednak wykonują się tylko po otrzymaniu znaku nowej linii.