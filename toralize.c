#include "toralize.h"

// ./toralize 1.2.3.4 80
// ./toralize ip port


Req *request(const char *dstip, const int dstport){

    Req *req;
    req = malloc(reqsize);
    memset(req, 0, reqsize);
    req->vn = 4; //SOCKS4
    req->cd = 1; //establish a TCP/IP stream connection
    req->dstport = htons(dstport);
    req->dstip = inet_addr(dstip);
    strncpy((char *)req->userid, USERNAME, 7);
    return req;
}

int main(int argc, char *argv[])
{
    char *host;
    int port, s;
    struct sockaddr_in sock;
    Req *req;
    Res *res;
    char buf[ressize];
    int success;
    char tmp[512];
    

    if (argc < 3) {
        fprintf(stderr, "Usage: %s <host> <port>\n", argv[0]);
        return -1;
    }

    host = argv[1];
    port = atoi(argv[2]);

  //  printf("[DEBUG] Host: %s, Port: %d\n", host, port);

#ifdef _WIN32
    WSADATA wsaData;
    int wsaerr = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (wsaerr != 0) {
        fprintf(stderr, "WSAStartup failed: %d\n", wsaerr);
        return -1;
    }
#endif

    s = socket(AF_INET, SOCK_STREAM, 0);
#ifdef _WIN32
    if (s == INVALID_SOCKET) {
        fprintf(stderr, "socket failed: %ld\n", WSAGetLastError());
        WSACleanup();
        return -1;
    }
#else
    if (s < 0) {
        perror("socket");
        return -1;
    }
#endif
    

    sock.sin_family = AF_INET;
    sock.sin_port = htons(PROXYPORT);
    sock.sin_addr.s_addr = inet_addr(PROXY);

   // printf("[DEBUG] Connecting to proxy %s:%d...\n", PROXY, PROXYPORT);
#ifdef _WIN32
    if(connect(s, (struct sockaddr *)&sock, sizeof(sock)) == SOCKET_ERROR) {
        fprintf(stderr, "connect failed: %ld\n", WSAGetLastError());
        closesocket(s);
        WSACleanup();
        return -1;
    }
#else
    if(connect(s, (struct sockaddr *)&sock, sizeof(sock))) {
        perror("connect");
        close(s);
        return -1;
    }
#endif

    printf("Connected to proxy %s:%d\n", PROXY, PROXYPORT);
    req = request(host, port);
    send(s, (const char*)req, reqsize, 0);

    memset(buf, 0, ressize);
    if(recv(s, buf, ressize, 0) < 1){
        fprintf(stderr, "Failed to read response from proxy\n");
        free(req);
        close(s);
        return -1;
    }

    res = (Res *)buf;
    success = (res->cd == 90) ? 1 : 0;
    if(!success){
        fprintf(stderr, "Proxy connection failed with code: %d\n", res->cd);
        close(s);
        free(req);
        return -1;
    } else {
        printf("Proxy connection established to %s:%d\n", host, port);

    }

    memset(tmp, 0, 512);
    snprintf(tmp, 511, "HEAD / HTTP/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n", host);
    send(s, tmp, strlen(tmp), 0);

    memset(tmp, 0, 512);
    recv(s, tmp, 511, 0);
    printf("Received data:\n%s\n", tmp);



    close(s);
    free(req);
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;

}