#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <poll.h>

#undef VERBOSE

#define LEFT_KEY '4'
#define RIGHT_KEY '6'
#define GAME_STARTED '+'
#define GAME_ENDED '#'
#define SCORE_END '|'
#define EXIT_KEY 'q'

const char* file_name = "scores.txt";

// Helper functions for printing to the terminal
void terminalPrintWithLength(char *str, size_t length);
void terminalPrint(char *str);
void exitGame(struct termios *old_settings);

int main(int argc, char *argv[])
{
    // Terminál törlése
    system("clear");

    if (argc < 2) {
        printf("Nem adtál meg sorosportnév-paramétert!\n");

        return -1;
    }

    // === Sorosporti kommunikáció beállítása ===
    struct termios serial;
    memset(&serial, 0, sizeof(struct termios));

    // struktúra kitöltése
    serial.c_cflag = CS8 | CREAD | CLOCAL; // 8-bites keretméret, vétel engedélyezése, modem control tiltása
    serial.c_cc[VMIN] = 1; // karakterenkénti olvasás engedélyezése
    serial.c_cc[VTIME] = 5; // nem-kanonikus olvasás időlimitje tizedmásodpercben
    cfsetospeed(&serial, B115200); // adó sebességének beállítása
    cfsetispeed(&serial, B115200); // vevő sebességének beállítása

    // soros port megnyitása írásra és olvasásra
    int serial_fd = open(argv[1], O_RDWR);
    if (serial_fd == -1) {
        write(STDERR_FILENO, "Sikertelen soros porti kommunikáció!\n", 38);

        return -1;
    }

    // beállítások alkalmazása
    tcsetattr(serial_fd, TCSANOW, &serial); // TCSANOW = "alkalmazás azonnal"

    // === Terminál beállítása (hogy ne kelljen entert nyomni a karakterek elküldéséhez) ===
    struct termios old_settings, new_settings;

    // jelenlegi terminálbeállítások elmentése
    tcgetattr(STDIN_FILENO, &old_settings);
    new_settings = old_settings;

    // terminál beállítása nemkanonikus módra, és hogy ne jelenjenek meg a karakterek a terminálban
    new_settings.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_settings);

    // === Játék ===
    terminalPrint("Gombnyomással indítsd a játékot!\n");

    struct pollfd fds[2];
    fds[0].fd = STDIN_FILENO; // FD 0: terminálbemenet
    fds[0].events = POLLIN;
    fds[1].fd = serial_fd; // FD 1: soros port
    fds[1].events = POLLIN;

    while (1) {
        int pollResult = poll( // Várakozás valamelyik esemény bekövetkezésére
            fds,
            /* number of watched FD-s: */ 2,
            /* infinite timeout: */ -1
            );

        if (pollResult < 0) {
            write(STDERR_FILENO, "Hiba történt az eseményekre várakozáskor.\n", 34);
            exitGame(&old_settings);

            return -1;
        }

        if (pollResult == 0) // nem történt esemény (végtelen timeout esetén ez nem fordulhat elő)
            continue;

        // Terminálbemeneti események kezelése
        if (fds[0].revents & POLLIN) {
            char c_tx = 0;
            if (read(STDIN_FILENO, &c_tx, 1) > 0) {
                write(serial_fd, &c_tx, 1); // karakter elküldése a soros porton

#ifdef VERBOSE
                switch (c_tx)
                {
                case LEFT_KEY:
                    terminalPrint("<"); // balra fordulás jelzése a terminálban
                    break;
                case RIGHT_KEY:
                    terminalPrint(">"); // jobbra fordulás jelzése a terminálban
                    break;
                }
#endif

                // ha a karakter EXIT_KEY, akkor kilépünk a játékból
                if (c_tx == EXIT_KEY)
                    break;
            }
        }

        // Sorosporti események kezelése
        if (fds[1].revents & POLLIN) {
            char c_rx = 0;
            if (read(serial_fd, &c_rx, 1) > 0) {
#ifdef VERBOSE
                terminalPrintWithLength(&c_rx, 1); // karakter kiírása a terminálban
#endif

                // ha a karakter GAME_STARTED, akkor a játék elindult
                if (c_rx == GAME_STARTED) {
                    terminalPrint("Elindult a játék!\n");
                }

                // ha a karakter GAME_ENDED, akkor a játék véget ért és a pontszámot küldi a fejlesztőkártya
                if (c_rx == GAME_ENDED) {
                    // pontszám beolvasása
                    char newScore[3]; // két számjegy + lezáró karakter
                    for (size_t i = 0; i < 2;)
                    {
                        int readResult = read(serial_fd, &c_rx, 1);

                        if (readResult <= 0) // addig olvasunk, amígy nem érkezett meg a teljes pontszám
                            continue;

                        if (c_rx == SCORE_END)
                            break;
                        else {
                            newScore[i] = c_rx;
                            i++;
                        }
                    }

                    int currentScore = atoi(newScore); // jelenleg elért pontszám

                    // pontszám kiírása
                    terminalPrint("Játék vége!\nElért pontszám: ");
                    terminalPrintWithLength(newScore, 2);
                    terminalPrint("\n");

                    // pontszámok fájlba írása
                    FILE *file = fopen(file_name, "a+");
                    if (file == NULL) {
                        write(STDERR_FILENO, "Nem tudtuk elmenteni az eredményedet :(\n\n", 39);
                        continue;
                    }

                    fprintf(file, "%s\n", newScore); // jelenleg elért pontszám fájlba írása

                    // Összes pontszám beolvasása
                    fseek(file, 0, SEEK_SET);

                    int scores[100];
                    int scoresCount;
                    for (scoresCount = 0; scoresCount < 100; scoresCount++)
                    {
                        if (fscanf(file, "%d", &(scores[scoresCount])) == EOF)
                            break;
                    }

                    fclose(file);

                    // pontszámok rendezése csökkenő sorrendbe
                    for (int i = 0; i < scoresCount; i++) {
                        for (int k = i + 1; k < scoresCount; k++) {
                            if (scores[i] < scores[k]) {
                                int temp = scores[i];
                                scores[i] = scores[k];
                                scores[k] = temp;
                            }
                        }
                    }

                    if (currentScore == scores[0]) // Új rekord ellenőrzése
                        terminalPrint("Új rekord!\n\n");
                    else {
                        printf("Eddigi rekord: %d\n", scores[0]);

                        // hány százalékban ért el a felhasználó jobb pontszámot
                        int atLeastAsGoodCount;
                        for (atLeastAsGoodCount = 0; atLeastAsGoodCount < scoresCount; atLeastAsGoodCount++) {
                            if (currentScore > scores[atLeastAsGoodCount])
                                break;
                        }

                        float atLeastAsGoodPercent = (float) atLeastAsGoodCount / scoresCount * 100;

                        printf("A jelenlegi eredményed a felső %.0f%%-ban van benne\n\n", atLeastAsGoodPercent);
                    }
                }
            }
        }
    }

    system("clear");

    exitGame(&old_settings);

    return 0;
}

void terminalPrintWithLength(char *str, size_t length) {
    write(STDOUT_FILENO, str, length);
}
void terminalPrint(char *str) {
    terminalPrintWithLength(str, strlen(str));
}
void exitGame(struct termios *old_settings) {
    // Restore original terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, old_settings);
    
    terminalPrint("\nKilépés...\n");
}
