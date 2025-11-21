# Proponowane zadania jakościowe

## Poprawka literówki
- **Problem:** W pliku `xmlviewer.readme` lista zmian zawiera wpis "Cliboard support" z błędnie zapisaną nazwą schowka systemowego.
- **Zadanie:** Zamienić "Cliboard" na "Clipboard" w sekcji NEWS, aby opis funkcji był poprawny językowo.

## Usunięcie błędu
- **Problem:** W funkcji `default_hndl` (`xmlviewerexpat.c`) w przypadku niepowodzenia alokacji bufora zwalniana jest tylko lista atrybutów, ale nie sama struktura `xml_data`, co powoduje wyciek pamięci.
- **Zadanie:** Dodać zwolnienie `tmp` w ścieżce błędu, gdy `AllocVec` dla `buffer` zwraca `NULL`.

## Korekta komentarza/dokumentacji
- **Problem:** Plik `xmlviewerabout.c` wyświetla wersję "version 0.9 (Amigaos)", podczas gdy `VERSION_TAG` w `xmlviewer.c` wskazuje wersję 0.17, a `xmlviewer.readme` opisuje wersję 0.16 — informacje o wersji są niespójne.
- **Zadanie:** Ujednolicić komunikaty o wersji (np. w oknie About i README) z bieżącym `VERSION_TAG` aplikacji.

## Ulepszenie testu
- **Problem:** W repozytorium brakuje jakichkolwiek zadań testowych lub celu `test` w `makefile`, co utrudnia automatyczne wykrywanie regresji w parserze XML i widoku drzewka.
- **Zadanie:** Dodać prosty test (np. cel `test` uruchamiający minimalny binarny test lub skrypt) weryfikujący parsowanie przykładowego pliku XML i poprawne odzwierciedlenie struktury w `xmlviewerexpat`/`xmlviewertree`.
