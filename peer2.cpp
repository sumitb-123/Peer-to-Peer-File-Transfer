#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include<unistd.h>
#include <string.h>
#include <iostream>
#include<pthread.h>
#define BUFF_SIZE 1024
#define trd 10
#define ip "127.0.0.1"
#define nm 100
using namespace std;

int port, port1;
int fl = 0;

void *fileRead(void *sock){

	cout<<"inside the thread"<<endl;
	int client_socket = *((int *)sock);
	string filename = "rfile"+to_string(++fl);
	FILE *fp = fopen(filename.c_str(), "wb");
    char Buffer[BUFF_SIZE];
    int file_size,n;
    recv(client_socket, &file_size, sizeof(file_size), 0);
    cout<<"file size received "<<file_size<<endl;
    while ((n=recv(client_socket,Buffer,BUFF_SIZE,0))>0 && file_size>0){
        fwrite(Buffer, sizeof(char), n, fp);
        file_size = file_size - n;
    }
    cout<<"file is closed"<<endl;
    fclose(fp);
	close(client_socket);
	pthread_exit(NULL);

}

void *fileSend(void *sock){
	
	char Buffer[BUFF_SIZE];
    int file_size,n,size;
	cout<<"inside sending thread"<<endl;
	int client_socket = *((int *)sock);
    recv(client_socket, Buffer, BUFF_SIZE, 0);
	FILE *fp = fopen(Buffer,"rb");
    fseek (fp,0,SEEK_END);
    size = ftell(fp);
    rewind (fp);
	send (client_socket,&size,sizeof(size), 0);
	cout<<Buffer<<endl;
	memset(Buffer,'\0', BUFF_SIZE);

	while ((n= fread(Buffer,sizeof(char),BUFF_SIZE,fp)) > 0 && size > 0 ){
        send (client_socket,Buffer, n, 0);
        size = size - n ;
    }

    cout<<"closing the file in sending theread"<<endl;
    fclose(fp);
	close(client_socket);
	pthread_exit(NULL);

}

void *peerServer(void *sock){

	int svr_socket;
    pthread_t thread[trd];
    char response[BUFF_SIZE];
    //creating socket for the communication
    svr_socket = socket(AF_INET, SOCK_STREAM, 0);
    //structure for defining the connection attributes
    struct sockaddr_in server;
    //assigning type, port and address 
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);
    int addrlen = sizeof(sockaddr);
    //binding the address
    bind(svr_socket, (struct sockaddr*) &server, sizeof(server));
    //listening to respond
    listen(svr_socket,trd);
    int temp,i=0,sts;
    string filename;
    while(1){
        //accepting request from clients
        cout<<"server is listening"<<endl;
        int client_socket = accept(svr_socket, (struct sockaddr *)&server, (socklen_t*)&addrlen);
        sts = pthread_create(&thread[i], NULL, fileSend, &client_socket);
        i++;
    }
    //closing the socket
    close(svr_socket);
	pthread_exit(NULL);

}

void *peerClient(void *args){

	char *filepath = ((char *)args);
	cout<<filepath<<endl;
	int svr_socket;
	//creating socket for the communication
	svr_socket = socket(AF_INET, SOCK_STREAM, 0);
	//structure for defining the connection attributes
	struct sockaddr_in server;
	//assigning values to the structure like type, port and address 
	server.sin_family = AF_INET;
	server.sin_port = htons(port1);
	server.sin_addr.s_addr = inet_addr(ip);
	//connecting to the server
	int status = connect(svr_socket, (struct sockaddr*) &server, sizeof(server));
	if(status<0){
		cout<<"Error in connection establishment "<<endl;
	}
	char response[BUFF_SIZE];
	cout<<"starting communication from client thread"<<endl;
    FILE *fp1 = fopen("received","wb");
    send (svr_socket,filepath,sizeof(filepath), 0);

    char Buffer[BUFF_SIZE] ;
    int n;
	int file_size;
	recv(svr_socket, &file_size, sizeof(file_size), 0);
	while ((n=recv(svr_socket,Buffer,BUFF_SIZE,0))>0 && file_size>0){
        fwrite(Buffer, sizeof(char), n, fp1);
        file_size = file_size - n;
    }
    cout<<"closing the file"<<endl;
    fclose(fp1);
	pthread_exit(NULL);

}

//int socket(int domain, int type, int protocol);
int main(int argc, char* argv[]){
	
	port = stoi(argv[1]);
	port1 = stoi(argv[2]);
	string st;
	pthread_t td1,td2;
	char filepath[nm];
	int status = pthread_create(&td1, NULL, peerServer,NULL);
	while(1){
		cout<<"Enter download "<<endl;
		cin>>st;
		if(st.compare("download") == 0){
			cout<<"Enter filepath "<<endl;
			cin>>filepath;
			int cstatus = pthread_create(&td1, NULL, peerClient,&filepath);
		}		
	}
	return 0;

}
