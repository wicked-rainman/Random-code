int createUdpBroadcastSocket(struct sockaddr_in *bcastAddr, char *broadcastip, int port) {
    int UDPsock;
    int broadcastPermission = 1;
    if ((UDPsock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
                syslog(LOG_NOTICE,"Failed to open UDP socket : %d (%s)\n",errno,strerror(errno));
                exit(EXIT_FAILURE);
    }
    if (setsockopt(UDPsock, SOL_SOCKET, SO_BROADCAST, (void *) &broadcastPermission, sizeof(broadcastPermission)) < 0) {
                syslog(LOG_NOTICE,"setsockopt failed : %d (%s)\n",errno,strerror(errno));
                exit(EXIT_FAILURE);
    }
    memset(bcastAddr, 0, sizeof(bcastAddr));
    bcastAddr->sin_family = AF_INET;
    bcastAddr->sin_addr.s_addr = inet_addr(broadcastip);
    bcastAddr->sin_port = htons(port);
    syslog(LOG_NOTICE,"Ready to broadcast UDP on port %d\n",port);
    return UDPsock;
}
