# IPC-Publish-Subscribe
Projekt zaliczeniowy z przedmiotu programowanie systemowe i współbieżne.

## Instrukcja kompilacji
Zalecamy kompilację projektu używając kompilatora gcc oraz za pomocą następujących komend.
```
gcc -o server server.c -Wall
gcc -o client client.c ui.c -Wall
```

## Instrukcja uruchomienia
Aby uruchomić program należy:
1. uruchomić program server
2. uruchomić wybraną liczbę klientów
3. wybrać unikalną nazwę użytkownika dla każdego z klientów (niepowodzenie zakończy się błędem)


## Instrukcja użytkowania
Po zalogowaniu program umożliwia pisanie wiadomości na wybrany temat, wysyłanie ich do wszystkich użytkowników subskrybujących dany temat.
Domyślny temat jest określony w pliku parameters.h jako DEFAULT_TOPIC i jest do niego zapisywany każdy nowy użytkownik. Treść pisanej
wiadomości jest wyświetlana w ostatniej linijce. Wysłanie wiadomości następuje po naciśnięciu klawisza ENTER. W polu wiadomości można
również wywoływać komendy, które rozpoczynają się od znaku "/".\
Dostępne komendy:

|Komenda                                               | Opis                                                |
|:-----------------------------------------------------|:----------------------------------------------------|
| `/topic [nazwa_tematu]`                              | zmiana tematu pisanych wiadomości                   |
| `/newtopic [nazwa_tematu]`                           | stworzenie nowego tematu                            |
| `/topiclist`                                         | wyświetlenie listy aktywnych tematów                |
| `/sub [nazwa_tematu] [czas_trwania_sub_przejściowej]`| zasubskrybowanie tematu                             |
| `/unsub [nazwa_tematu]`                              | odsubskrybowanie tematu                             |
| `/mute [nazwa_użytkownika]`                          | przełączenie wyciszenia użytkownika o podanej nazwie|
| `/quit`                                              | opuszczenie programu                                |


## Opis plików:
client.c - implementacja całej logiki działania klienta\
ui.c - implementacja interfeksu użytkownika klienta\
server.c - implementacja całej logiki działania serwera

