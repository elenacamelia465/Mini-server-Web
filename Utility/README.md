# Utility - Server Utility Functions

Acest proiect definește o clasă `Utility` care include funcții de utilitate pentru configurarea și gestionarea conexiunilor serverului. Codul folosește `epoll` pentru monitorizarea evenimentelor pe descriptorii de fișiere, setează socket-uri non-blocante, gestionează semnalele și implementează un mecanism de gestionare a conexiunilor bazat pe semnale, pentru a eficientiza performanța serverului.

## Structura Proiectului

- **Monitorizarea conexiunilor**: `addFdToEpoll` adaugă un descriptor de fișier (`fd`) în structura `epoll`, cu opțiunea `oneShot` pentru a controla repetarea evenimentelor.
- **Crearea socket-urilor de ascultare**: `getListenFd` inițializează un socket TCP de ascultare la un anumit port și setează opțiunea de reutilizare a adresei (`SO_REUSEADDR`) pentru a evita probleme de reconectare. În mod implicit, socket-ul ascultă pe toate adresele IP (`INADDR_ANY`), dar poate fi schimbat pentru a asculta pe o adresă IP specifică.
- **Setarea socket-urilor ca non-blocante**: `setFdNonBlock` face ca descriptorul de fișier (`fd`) să nu blocheze, permițând serverului să gestioneze multiple conexiuni simultan.

## Funcționalități Principale

1. **Configurare semnale**: `setSignal` setează handler-ul pentru un semnal specific și maschează alte semnale, permițând serverului să proceseze evenimente pe bază de semnal.
2. **Gestionare periodică a conexiunilor**: `signalAlrmHandler` folosește un semnal pentru a declanșa periodic verificări asupra timeout-urilor de conexiune, scriind în conductă și notificând serverul să închidă conexiunile inactive.
3. **Oprire server**: `signalSigintHandler` scrie un semnal de întrerupere (`SIGINT`) în conductă, semnalizând oprirea sigură a serverului.
