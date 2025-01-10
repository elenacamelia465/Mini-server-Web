## Clasa HttpConnection

Clasa `HttpConnection` este responsabilă pentru gestionarea fiecărei conexiuni HTTP individuale. Aceasta oferă:
- **Gestionarea conexiunilor asincrone**: Utilizează `epoll` pentru a monitoriza evenimentele I/O și a optimiza gestionarea cererilor multiple.
- **Procesarea cererilor HTTP**:
  - Citește cererile primite prin metoda `handleRead`.
  - Trimite răspunsurile prin metoda `handleWrite`.
- **Suport pentru conexiuni persistente**: Prin opțiunea `isKeepAlive`, permite menținerea conexiunii deschise pentru mai multe cereri de la același client.
- **Tratarea metodelor HTTP**: Suportă metode precum `GET`, `POST`, `PUT`, `DELETE` și `HEAD`.
- **Servirea fișierelor**: Găzduiește fișiere statice (HTML, CSS, JS, imagini) și procesează fișiere PHP prin CGI.
- **Suport SSL/TLS**: Permite conexiuni securizate utilizând un obiect SSL.
 **SSID-Cookie**: Prezinta cookie pentru ficare conectare, acesta fiind securizat si criptat in functie de timestamp + o functie de generare nr random totul xor<s>.


## Clasa HttpRequest

Clasa `HttpRequest` este utilizată pentru parsarea cererilor HTTP primite. Aceasta include:
- **Identificarea metodei HTTP**: Recunoaște tipul cererii (`GET`, `POST`, etc.).
- **Parsarea URI-ului și anteturilor**: Extrage ruta solicitată și informațiile relevante din anteturi.
- **Gestionarea corpului cererii**: Stochează datele transmise pentru metodele `POST` și `PUT`.

## Scop

Aceste clase lucrează împreună pentru a permite gestionarea eficientă a cererilor HTTP, asigurând suport pentru cereri simple și complexe, inclusiv încărcări de fișiere, ștergeri și procesarea conținutului dinamic.
