#include<stdio.h> //printf
#include<string.h> //strlen
#include<sys/socket.h> //socket
#include<arpa/inet.h> //inet_addr
#include <crypt.h>



int main(int argc , char *argv[])
{
    int sock;
    struct sockaddr_in server;
    char message[1000] , server_reply[2000];
     
    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 8889 );
 
    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }
     
    puts("Connected\n");
    
    char login[2000], pass[2000];

    printf("Login: ");
    scanf("%s",login);

    send(sock, login, strlen(login), 0);

    printf("Password: ");
    scanf("%s",pass);
	
    send(sock, pass, strlen(pass), 0);

    int size_m = recv(sock,server_reply,2000,0);
    server_reply[size_m] = '\0';
    if(strstr(server_reply, "adenied"))
    {
         printf("Incorrect login/password!!!\n");
    }
    else
    {
	printf("Correct login/password.\n");
    }
    
    while(1)
    {
        printf("Enter message : ");
        scanf("%s" , message);
         
        //Send some data
        if( send(sock , message , strlen(message) , 0) < 0)
        {
            puts("Send failed");
            return 1;
        }
         
        //Receive a reply from the server
        if( recv(sock , server_reply , 2000 , 0) < 0)
        {
            puts("recv failed");
            break;
        }
         
        puts("Server reply :");
        puts(server_reply);
    }
     
    close(sock);
    return 0;
}
