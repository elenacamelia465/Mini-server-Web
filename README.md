# Mini-server-Web 
 - Std. Sg. Negoescu Elena - Camelia
 - Std. Sg. Rusu Petru - Calin


Acest proiect oferă implementarea unui **mini server web** în C++, destinat să ofere funcționalități fundamentale pentru servirea conținutului web într-un mod performant, modular și extensibil. Serverul integrează concepte avansate din programarea în rețea și este ideal pentru începători sau pentru cei care doresc să aprofundeze noțiuni de concurență, arhitectură web și procesare a cererilor HTTP/HTTPS.

## Funcționalități principale

1. **Model de concurență avansat**:
   - Utilizarea unui **thread pool** pentru gestionarea eficientă a firelor de execuție, reducând timpul de creare și distrugere a firelor.
   - **Socket neblocant** pentru manipularea simultană a mai multor conexiuni.
   - **Epoll** cu suport atât pentru modurile **Edge Triggered (ET)**, cât și **Level Triggered (LT)** pentru monitorizarea eficientă a evenimentelor.
   - Procesare de evenimente utilizând modelele **Reactor** și **Proactor** (simulat), oferind flexibilitate și performanță ridicată.

2. **Analiza cererilor HTTP/HTTPS**:
   - Implementarea unei **mașini de stare finite** pentru parsarea și interpretarea mesajelor HTTP/HTTPS.
   - Suport pentru cereri de tip **GET** și **POST**, inclusiv parsarea și procesarea lor.

3. **Integrarea unei baze de date**:
   - Accesarea bazei de date pentru implementarea funcțiilor de **înregistrare** și **autentificare** a utilizatorilor.
   - Servirea conținutului multimedia, cum ar fi imagini și fișiere video, stocate pe server.

4. **Sistem de logare**:
   - Înregistrarea stării de funcționare a serverului în timp real.

5. **Compresia și decompresia cererilor HTTP**:
   - Procesarea **head-url** și a **body-ului** cererilor pentru optimizarea transmisiei datelor și reducerea latenței.

6. **Performanță ridicată**:
   - Serverul suportă zeci de mii de conexiuni simultane, fiind potrivit pentru scenarii de testare și simulări reale.

## Caracteristici suplimentare

- **Configurabilitate ridicată**: Parametrii precum portul, numărul de fire din thread pool și modul de logare pot fi personalizați printr-un fișier de configurare (`config`).
- **Extensibilitate**: Structura modulară permite integrarea rapidă a noilor funcționalități, cum ar fi suport pentru protocoale suplimentare sau autentificare avansată.
- **Securitate**: Posibilitatea de a integra suport **SSL/TLS** pentru conexiuni sigure.

## Structura proiectului

- **`Config`**: Gestionarea și citirea setărilor serverului.
- **`HttpConnection`**: Tratarea cererilor HTTP și gestionarea răspunsurilor.
- **`ThreadPool`**: Implementarea unui mecanism eficient de gestionare a firelor.
- **`Utility`**: Funcții ajutătoare pentru parsarea datelor, logare și gestionarea conexiunilor.
- **`Main`**: Punctul de intrare al aplicației, responsabil pentru inițializarea serverului și rularea principală.

## Cerințe

- **Librării necesare**:
  - **Pthreads** pentru gestionarea firelor.
  - **OpenSSL** (opțional) pentru conexiuni securizate.
  - **nlohmann-json** pentru compresia și decompresia datelor.

- **Platformă de testare**:
  - **Ubuntu 24.04** pentru rularea serverului.
  - Browsere compatibile: **Chrome** și **Firefox** pe Linux și Windows.

- **Dependențe**:
  ```bash
  sudo apt update
  sudo apt install libssl-dev
  sudo apt install zlib1g-dev
  sudo apt-get install libcurl4-openssl-dev
  sudo apt-get install nlohmann-json3-dev
  sudo apt install php-cgi


## Instalare și utilizare

1. **Clonați proiectul**:
   ```bash
   git clone <URL-repo>
   cd mini-web-server
2. **Compilați proiectul**:
    ```bash
    make

3. **Operare personalizata**:
    ```bash
    ./server [-p port] [-n numWorker] [-s] [-t timeout]

    -p: Setează portul pentru server (implicit: 8086).
    -n: Setează numărul de fire din thread pool (implicit: 5).
    -s: Activează modul HTTPS (TLS 1.2/1.3).
    -t: Setează timeout-ul conexiunii în secunde (implicit: 20).
## Testare:

   Testarea conexiunilor HTTP cu Telnet: Pentru a testa funcționalitatea de 
   timeout a serverului, folosiți comanda:

    
    telnet localhost 8086

- După conectare, nu trimiteți nicio cerere. Veți observa cum conexiunea este închisă automat după perioada de timeout setată.

