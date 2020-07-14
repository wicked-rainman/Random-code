//----------------------------------------------------------------
// Program: UdpReBroadcast
// Reason:  This program accepts incomming UDP packets on port -i
//          and sends them out as UDP broadcasts on port -o
//          where the broadcast address is -a
//          Maximum payload is MAXRECVSTRING.
//          Broadcast output port is SO_REUSEPORT
//
//Usage:    -i <input port> -o<output port> -b <broadcast addr>
//
//Author:   wr
//----------------------------------------------------------------


#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <errno.h>
#include <time.h>
#define MAXRECVSTRING 10000
int createUdpBroadcastSocket(struct sockaddr_in *,char *, int);

//----------------------------------------------------------------
// Main
//----------------------------------------------------------------
int main(int argc, char *argv[]) {
        int UdpTxSock=-1;                       //Socket FD for UDP broadcast port
        int UdpRxSock=-1;                       //Socket FD for UDP receive port
        struct sockaddr_in broadcastAddr;       //UDP broadcast socket
        struct sockaddr_in recAddr;             //UDP receive socket
        char broadcastIP[16];                   //Broadcast address string
        unsigned short RxPort;                  //Receive port number
        unsigned short TxPort;                  //Transmit port number
        unsigned int recvStringLen;             //Length of received payload
        char recvString[MAXRECVSTRING+1];       //UDP payload
        int x,c;                                //Some old crap
        char *cvalue=NULL;

        //-----------------------------------------------------------
        //Process command line arguments
        //-----------------------------------------------------------
        if(argc !=7) {
                fprintf(stderr,
                "Usage: %s -i <input port> -o <output port> -b <broadcast address>\n\n",argv[0]);
                exit(EXIT_FAILURE);
        }
        while((c=getopt(argc,argv,"i:o:b:")) !=-1) {
                switch(c) {
                        case 'b' : {
                                strncpy(broadcastIP,optarg,15);
                                break;
                        }
                        case 'o' : {
                                TxPort = atoi(optarg);
                                break;
                        }
                        case 'i' : {
                                RxPort= atoi(optarg);
                                break;
                        }
                        default : {
                                fprintf(stderr,
                                "Usage: %s -i <input port> -o <output port> -b <broadcast address>\n\n",argv[0]);
                                exit(EXIT_FAILURE);
                        }
                }
        }

        //-----------------------------------------------------------
        //Open UDP broadcast socket
        //-----------------------------------------------------------
        UdpTxSock = createUdpBroadcastSocket(&broadcastAddr, broadcastIP, TxPort);

        //-----------------------------------------------------------
        //Open UDP receive socket
        //-----------------------------------------------------------
        if ((UdpRxSock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
                syslog(LOG_NOTICE,"Could not open UDP receive socket\n");
                exit(EXIT_FAILURE);
        }
        memset(&recAddr, 0, sizeof(recAddr));   /* Zero out structure */
        recAddr.sin_family = AF_INET;                 /* Internet address family */
        recAddr.sin_addr.s_addr = htonl(INADDR_ANY);  /* Any incoming interface */
        recAddr.sin_port = htons(RxPort);      /* Broadcast port */
        if (bind(UdpRxSock, (struct sockaddr *) &recAddr, sizeof(recAddr)) < 0) {
                syslog(LOG_NOTICE,"Could not bind UDP receive port\n");
                exit(EXIT_FAILURE);
        }

        syslog(LOG_NOTICE,"Reading port %d [UDP] and broadcasting on %s:%d [UDP]\n",RxPort,broadcastIP,TxPort);
        //-----------------------------------------------------------
        //Read incomming UDP packets and send them back out
        //-----------------------------------------------------------
        while(1) {
                recvStringLen=-1;
                /* Receive a single datagram from the server */
                if((recvStringLen=read(UdpRxSock,recvString,MAXRECVSTRING))<0) {
                //if ((recvStringLen = recvfrom(UdpRxSock, recvString, MAXRECVSTRING, 0, NULL, 0)) < 0) {
                        syslog(LOG_NOTICE,"Error reading UDP receive port\n");
                        exit(EXIT_FAILURE);
                }
                else {
                        recvString[recvStringLen] = '\0';
                        //Debug printf("Received: %s",recvString);
                        x=sendto(UdpTxSock, recvString, recvStringLen, 0, (struct sockaddr *) &broadcastAddr, sizeof(broadcastAddr));
                        if(x!=recvStringLen) {
                                syslog(LOG_NOTICE,"sendto(UDP) sent a different number of bytes than expected\n");
                                exit(EXIT_FAILURE);
                        }
                }
        }
}

//-----------------------------------------
//
//Function create UDP broadcast socket
//Re-use socket if it is already open
//
//------------------------------------------
int createUdpBroadcastSocket(struct sockaddr_in *bcastAddr, char *broadcastip, int port) {
    int UDPsock;
    int broadcastPermission = 1;
    if ((UDPsock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
                syslog(LOG_NOTICE,"Failed to open UDP socket : %d (%s)\n",errno,strerror(errno));
                exit(EXIT_FAILURE);
    }
    if (setsockopt(UDPsock, SOL_SOCKET, SO_BROADCAST, (void *) &broadcastPermission, sizeof(broadcastPermission)) < 0) {
                syslog(LOG_NOTICE,"setsockopt SO_BROADCAST failed : %d (%s)\n",errno,strerror(errno));
                exit(EXIT_FAILURE);
    }
    if (setsockopt(UDPsock, SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR) , (void *) &broadcastPermission, sizeof(broadcastPermission)) < 0) {
                syslog(LOG_NOTICE,"setsockopt SO_REUSEADDR failed : %d (%s)\n",errno,strerror(errno));
                exit(EXIT_FAILURE);
    }
    memset(bcastAddr, 0, sizeof(bcastAddr));
    bcastAddr->sin_family = AF_INET;
    bcastAddr->sin_addr.s_addr = inet_addr(broadcastip);
    bcastAddr->sin_port = htons(port);
    return UDPsock;
}
                                                    
