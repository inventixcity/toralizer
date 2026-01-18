#include "toralize.h"

// ./toralize 1.2.3.4 80
// ./toralize ip port


Req *request(struct sockaddr_in* sock2){

    Req *req;
    req = malloc(reqsize);
    memset(req, 0, reqsize);
    req->vn = 4; //SOCKS4
    req->cd = 1; //establish a TCP/IP stream connection
    req->dstport = sock2->sin_port;
    req->dstip = sock2->sin_addr.s_addr;
    strncpy((char *)req->userid, USERNAME, 7);
    return req;
}

#ifdef _WIN32
int main(int argc, char *argv[]) {
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

    WSADATA wsaData;
    int wsaerr = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (wsaerr != 0) {
        fprintf(stderr, "WSAStartup failed: %d\n", wsaerr);
        return -1;
    }
    
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET) {
        fprintf(stderr, "socket failed: %ld\n", WSAGetLastError());
        WSACleanup();
        return -1;
    }

    sock.sin_family = AF_INET;
    sock.sin_port = htons(PROXYPORT);
    sock.sin_addr.s_addr = inet_addr(PROXY);

    if(connect(s, (struct sockaddr *)&sock, sizeof(sock)) == SOCKET_ERROR) {
        fprintf(stderr, "connect failed: %ld\n", WSAGetLastError());
        closesocket(s);
        WSACleanup();
        return -1;
    }

    printf("Connected to proxy %s:%d\n", PROXY, PROXYPORT);
    struct sockaddr_in target;
    target.sin_family = AF_INET;
    target.sin_port = htons(port);
    target.sin_addr.s_addr = inet_addr(host);

    req = request(&target);
    send(s, (const char*)req, reqsize, 0);

    memset(buf, 0, ressize);
    if(recv(s, buf, ressize, 0) < 1){
        fprintf(stderr, "Failed to read response from proxy\n");
        free(req);
        closesocket(s);
        return -1;
    }

    res = (Res *)buf;
    success = (res->cd == 90) ? 1 : 0;
    if(!success){
        fprintf(stderr, "Proxy connection failed with code: %d\n", res->cd);
        closesocket(s);
        free(req);
        return -1;
    }
    printf("Proxy connection established to %s:%d\n", host, port);

    // Simple HTTP test
    memset(tmp, 0, 512);
    snprintf(tmp, 511, "HEAD / HTTP/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n", host);
    send(s, tmp, strlen(tmp), 0);

    memset(tmp, 0, 512);
    recv(s, tmp, 511, 0);
    printf("Received data:\n%s\n", tmp);

    closesocket(s);
    WSACleanup();
    free(req);
    return 0;
}
#else
int connect(int s2, const struct sockaddr *sock2, socklen_t addrlen){

    int s;
    struct sockaddr_in sock;
    Req *req;
    Res *res;
    char buf[ressize];
    int success;
    char tmp[512];
    int (*p)(int, const struct sockaddr*, socklen_t);

    
    p = dlsym(RTLD_NEXT, "connect");

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        perror("socket");
        return -1;
    }
    

    sock.sin_family = AF_INET;
    sock.sin_port = htons(PROXYPORT);
    sock.sin_addr.s_addr = inet_addr(PROXY);

    if(p(s, (struct sockaddr *)&sock, sizeof(sock))) {
        perror("connect");
        close(s);
        return -1;
    }

    printf("Connected to proxy %s:%d\n", PROXY, PROXYPORT);
    req = request((struct sockaddr_in*)sock2);
    write(s, (const char*)req, reqsize);

    memset(buf, 0, ressize);
    if(read(s, buf, ressize) < 1){
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
    } 
    printf("Proxy connection established\n");

    
    dup2(s, s2);
    free(req);
    return 0;
}
#endif