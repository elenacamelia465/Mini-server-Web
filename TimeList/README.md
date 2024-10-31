# WebServer Timeout Management

Acest proiect implementează o listă de timeout (`TimeList`) pentru a gestiona conexiunile inactive pe server. Dacă un utilizator nu trimite date pentru o perioadă lungă de timp după conectarea la server, descriptorul de fișiere asociat rămâne ocupat, ducând la risipa resurselor de conexiune. Pentru a rezolva această problemă, proiectul utilizează un cronometru care eliberează conexiunile inactive și închide descriptorii de fișiere, optimizând utilizarea resurselor serverului.

## Structura Proiectului

- **TimeListNode**: Reprezintă nodurile listei, fiecare nod având un timp de expirare și o conexiune HTTP (`HttpConnection`). Nodurile sunt conectate bidirecțional, facilitând adăugarea și eliminarea eficientă din listă.
- **TimeList**: Implementează lista dublu-încheiată pentru gestionarea conexiunilor HTTP și legarea acestora la un cronometru. La intervale regulate, serverul verifică lista și închide conexiunile care au expirat, eliberând resursele utilizate.

## Funcționalități Principale

1. **Adăugare de Conexiuni**: `attachTimer` asociază o conexiune HTTP unui nod din listă, setând un timp de expirare pe baza unui timeout specificat.
2. **Eliminarea Conexiunilor Expirate**: `tick` parcurge lista și închide conexiunile inactive (acelea pentru care timpul de expirare a trecut).
3. **Reînnoirea Timpului de Expirare**: `updateTimer` actualizează timpul de expirare pentru o conexiune activă, menținând astfel conexiunea în lista de verificare.

