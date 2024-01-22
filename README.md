# IPC-Publish-Subscribe
Projekt zaliczeniowy z przedmiotu programowanie systemowe i współbieżne.
Zalecamy kompilację projektu używając kompilatora gcc oraz za pomocą następujących komend.
```
gcc -o server server.c -Wall
gcc -o client client.c ui.c -Wall
```
Opis plików:
- client.c - implementacja całej logiki działania klienta
- ui.c - implementacja interfejsu użytkownika klienta
- server.c - implementacja całej logiki działania serwera
