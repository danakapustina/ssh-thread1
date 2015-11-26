#include<stdio.h>
#include<string.h> //strlen
#include<stdlib.h> //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h> //write
#include<pthread.h> //for threading , link with lpthread
#include <crypt.h>



#define MAX_CONNECTIONS 3

//the thread function
void *connection_handler(void *);
//void *execute_command(const char *command);
int work_con = 0;
int op = 1; 

int main(int argc , char *argv[])
{
    int socket_desc , client_sock , c , *new_sock;
    struct sockaddr_in server , client;
     
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");

    setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &op, sizeof(int));
     
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 8889 );
     
    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");
     
    //Listen
    listen(socket_desc , 3);

    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    while(client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)   )
    {

        if (work_con < MAX_CONNECTIONS)
        {
            puts("Connection accepted");
            work_con++;
            pthread_t sniffer_thread;
            new_sock = malloc(1);
            *new_sock = client_sock;
             
            if(pthread_create( &sniffer_thread , NULL , connection_handler , (void*) new_sock) < 0)
            {
                perror("Could not create thread");
                return 1;
            }
             
            puts("Handler assigned");
        }
        else puts("Too mach connectinons");
    }
     
    if (client_sock < 0)
    {
        perror("Accept failed");
        return 1;
    }
     
    return 0;
}

void execute_command(const char *command, int sock)
{
    FILE *fp;
    char output[1035];    

    /* Open the command for reading. */
    fp = popen(command, "r");
    if (fp == NULL) {
        printf("Failed to run command\n" );
        exit(1);
    }
    
    /* Read the output a line at a time - output it. */
    while (fgets(output, sizeof(output), fp) != NULL) {        
        write(sock, output, strlen(output));              
    }
    
    /* close */
    pclose(fp);
  
}


void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int read_size;
    char *message , client_message[2000];


///////////////////////////////////////////////////////////

    int size_m;
    char login[2049], pass[2049], ilogin[50], ipass[100];            
    int flag = 1;
    int attempts = 2;

    FILE *fp = fopen("users.txt","r");

    while(1)
    {

        size_m = recv(sock, login, 2048, 0);
        login[size_m] = '\0';
        size_m = recv(sock, pass, 2048, 0);
        pass[size_m] = '\0';

        attempts -= 1;

        while(!feof(fp))
        {	
            fscanf(fp, "%s%s", ilogin, ipass);         
            if ((strcmp(ipass, pass) == 0) && (strcmp(ilogin, login) == 0))
            {	
                flag = 100;
            }
        }

    	if(flag != 100)
        {            
            if(attempts != 0)
            {      
                write(sock, "Try again\n", 10);          
                continue;
            }
            else
            {
                write(sock, "Access denied\n", 14);                
                fclose(fp);
                close(sock);
                puts("client disconnected");
                return NULL;
            }            
            
        }
        else
        {	 
            write(sock , "Access granted\n" , 15);
	        puts("client connected");
            break;
        }

    }

    fclose(fp);

    

    while( (read_size = recv(sock , client_message , 2000 , 0)) > 0 )
    {
        //Send the message back to client
        // write(sock , client_message , strlen(client_message));
	    execute_command(client_message, sock);
        //write(sock, output, strlen(output));
    }
 
     
    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }
         
    //Free the socket pointer
    free(socket_desc);
    work_con--;
    return NULL;
}
