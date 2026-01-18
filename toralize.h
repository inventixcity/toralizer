#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")
#define close(s) closesocket(s)
#else
#include <dlfcn.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#endif
#define PROXY "127.0.0.1"
#define PROXYPORT 9050
#define USERNAME "toraliz"
#define reqsize sizeof(struct proxy_request)
#define ressize sizeof(struct proxy_response)

typedef unsigned char int8;
typedef unsigned short int16;
typedef unsigned int int32;

/*

        Read from here: https://www.openssh.org/txt/socks4.protocol

                +----+----+----+----+----+----+----+----+----+----+....+----+
		| VN | CD | DSTPORT |      DSTIP        | USERID       |NULL|
		+----+----+----+----+----+----+----+----+----+----+....+----+

        VN      SOCKS protocol version number
                The version number is 4.

        CD      SOCKS command code:
                1 = establish a TCP/IP stream connection
                2 = establish a TCP/IP port binding

        DSTPORT Destination port, in network byte order

        DSTIP   Destination IP address, in network byte order
                If the client is using a domain name service, it should set the
                first three bytes of DSTIP to null (0x00 0x00 0x00) and put the
                domain name into the USERID field.

        USERID  The user ID string, variable length, terminated with a null (0x00)

        NULL    The byte 0x00 to indicate the end of the USERID field
*/

#pragma pack(push, 1)
struct proxy_request {
    int8 vn;
    int8 cd;
    int16 dstport;
    int32 dstip;
    unsigned char userid[8];

};
#pragma pack(pop)
typedef struct proxy_request Req;

/*

                +----+----+----+----+----+----+----+----+
		| VN | CD | DSTPORT |      DSTIP        |
		+----+----+----+----+----+----+----+----+

        VN      SOCKS protocol version number
                The version number is 4.

        CD      Result code:
                90 = request granted
                91 = request rejected or failed
                92 = request rejected because SOCKS server cannot connect to identd on the client
                93 = request rejected because the client program and identd report different user IDs

        DSTPORT Server bound port number, in network byte order

        DSTIP   Server bound IP address, in network byte order

*/


#pragma pack(push, 1)
struct proxy_response {
    int8 vn;
    int8 cd;
    int16 _; //not used
    int32 __; //not used
};
#pragma pack(pop)
typedef struct proxy_response Res;

Req *request(struct sockaddr_in*);
int connect(int, const struct sockaddr*, socklen_t);