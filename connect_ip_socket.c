//-----------------------------------------------------------------
// connect_ip_socket
// Connect to a port on the given IP address (no DNS lookups)
// Requires sys/socket.h and arpa/inet.h
//-----------------------------------------------------------------
int connect_ip_socket(char *host_ip, int port) {
        struct sockaddr_in server_address;
        int sockfd;
                sockfd = socket(AF_INET, SOCK_STREAM, 0);
                if(sockfd<0) {
                        fprintf(stderr,"\nCould not create new socket!\n");
                        exit(EXIT_FAILURE) ;
                }
                server_address.sin_family = AF_INET;
                server_address.sin_port = htons((uint16_t)port);
                if(inet_pton(AF_INET,host_ip,&server_address.sin_addr)<=0) {
                        fprintf(stderr, "\nCould not convert %s to binary\n",host_ip);
                        exit(EXIT_FAILURE) ;
                }
                if(connect(sockfd,(struct sockaddr *) &server_address,(socklen_t) sizeof(server_address))<0) {
                        fprintf(stderr,"\nCould not connect to %s:%d",host_ip,port);
                        exit(EXIT_FAILURE) ;
                }
                return (sockfd);
}
