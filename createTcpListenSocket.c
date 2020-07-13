int createTcpListenSocket(int port) {
        int sockfd;
        struct sockaddr_in server_address;
        sockfd=socket(AF_INET , SOCK_STREAM , 0);
        if (sockfd == -1) {
                syslog(LOG_NOTICE,"Could not create listen socket: %d (%s)\n",errno,strerror(errno));
                exit(EXIT_FAILURE);
        }
        server_address.sin_family = AF_INET;
        server_address.sin_addr.s_addr = INADDR_ANY;
        server_address.sin_port = htons( port );
        if( bind(sockfd,(struct sockaddr *) &server_address , sizeof(server_address)) < 0) {
                syslog(LOG_NOTICE,"Bind failed : %d (%s)\n",errno,strerror(errno));
                exit(EXIT_FAILURE);
        }
        syslog(LOG_NOTICE,"Listening for TCP connections on port %d\n",port);
        listen(sockfd,3);
        return sockfd;
}
