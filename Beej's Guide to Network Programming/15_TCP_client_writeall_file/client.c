#include "include.h"
#include "declare.h"
#include "define.h"

int main(int argc, char **argv, char *envp[])
{
    int sockfdClient = -1, nFileSize = 0;
    struct sockaddr_in stSockAddrInClient;
    socklen_t clientLen = 0;
    char strTemp[32];
    FILE *fp = (FILE *)NULL;
    int nRet = -1;

    if (argc < 2) {
        fprintf(stderr, "Please input a file. \n");
        exit(EXIT_FAILURE);
    }

    sockfdClient = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfdClient == -1) {
        herror("socket");
        exit(EXIT_FAILURE);
    }

    bzero((void *)&stSockAddrInClient, sizeof(struct sockaddr_in));
    stSockAddrInClient.sin_family = AF_INET;
    stSockAddrInClient.sin_addr.s_addr = inet_addr(IP_ADDRESS);
    stSockAddrInClient.sin_port = htons(PORT);
    clientLen = sizeof(struct sockaddr_in);
    nRet = connect(sockfdClient, (struct sockaddr *)&stSockAddrInClient, clientLen);
    if (nRet == -1) {
        herror("connect");
        close(sockfdClient);
        exit(EXIT_FAILURE);
    }

    fp = fopen(argv[1], "rb");
    if (ferror(fp) != 0) {
        perror("fopen");
        fclose(fp);
        close(sockfdClient);
        exit(EXIT_FAILURE);
    }

    while (feof(fp) == 0) {
        memset(strTemp, '\0', sizeof(char) * 32);
        nRet = fread((void *)strTemp, sizeof(char), 32, fp);
        if (nRet == 0)
            break;
        nRet = writeall(sockfdClient, (void *)strTemp, nRet);
        nFileSize += nRet;
    }
    fclose(fp);

    printf("Client (%s: %u) writeall %d bytes to server: %s \n", \
        inet_ntoa(stSockAddrInClient.sin_addr), ntohs(stSockAddrInClient.sin_port),  nFileSize, argv[1]);
    sleep(1);
    close(sockfdClient);
    printf("Client done!! \n");

    return 0;
}
