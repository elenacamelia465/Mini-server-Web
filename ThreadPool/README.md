# WebServer ThreadPool

Acest proiect implementează un `ThreadPool` care gestionează un grup de threaduri pentru a prelua și procesa cererile HTTP în paralel, ceea ce sporește eficiența aplicației. ThreadPool-ul folosește un model semi-sincron/semi-reactor: thread-ul principal gestionează evenimentele asincron, ascultă socket-urile și adaugă cereri într-o coadă de solicitări. Threadurile de lucru preiau sarcinile din coadă și finalizează procesarea datelor.

## Funcții principale

- **Inițializare**: Constructorul creează un număr specificat de threaduri pentru a gestiona cererile.
- **Adăugare sarcini**: `addTask` introduce cereri HTTP în coadă, notificând un thread de lucru disponibil.
- **Oprire**: `quitLoop` oprește execuția threadurilor printr-un semnal de închidere sincronizat.


