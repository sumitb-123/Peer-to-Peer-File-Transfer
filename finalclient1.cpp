#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include<unistd.h>
#include <string.h>
#include <iostream>
#include <stdio.h>
#include <openssl/sha.h>
#include<pthread.h>
#define BUFF_SIZE 1024
#define trd 10
#define nm 100
using namespace std;

int port, port1;
int fl = 0;
string ip = "127.0.0.1";
vector<string> tracker_ip;
vector<int> tracker_port;

struct dstruct{
	string pip;
	string filepath;
	string dpath;
	int  port;
};

string shaCalculate(string );

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
    server.sin_addr.s_addr = inet_addr(ip.c_str());
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

	//char *filepath = ((char *)args);
	char filepath[BUFF_SIZE];
	struct dstruct *peerinfo = ((struct dstruct *)args);
	strcpy(filepath,(peerinfo->filepath).c_str());
	cout<<filepath<<endl;
	int svr_socket;
	//creating socket for the communication
	svr_socket = socket(AF_INET, SOCK_STREAM, 0);
	//structure for defining the connection attributes
	struct sockaddr_in server;
	//assigning values to the structure like type, port and address 
	server.sin_family = AF_INET;
	server.sin_port = htons(peerinfo->port);
	server.sin_addr.s_addr = inet_addr(peerinfo->pip.c_str());

	//connecting to the server
	int status = connect(svr_socket, (struct sockaddr*) &server, sizeof(server));
	if(status<0){
		cout<<"Error in connection establishment "<<endl;
	}
	char response[BUFF_SIZE];
	cout<<"starting communication from client thread"<<endl;
    FILE *fp1 = fopen(peerinfo->dpath.c_str(),"wb");
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

//login into the sharing system
bool login(string user, string pass, int svr_socket, int tracker_desc){
    int option = 2;
    char username[nm];
    char passwd[nm];
    char buffer[BUFF_SIZE];
    int status;
	bool ipst;
	int uport = port;
	cout<<"server port for client "<<uport<<endl;
	string uip = ip;
    strcpy(buffer,user.c_str());
    //cout<<"first"<<endl;
    send(svr_socket,&option,sizeof(option),0);
    //cout<<"second"<<endl;
    send(svr_socket,buffer,BUFF_SIZE,0);
    //cout<<"third"<<endl;
    strcpy(buffer,pass.c_str());
    send(svr_socket,buffer,BUFF_SIZE,0);
    //cout<<"fourth"<<endl;
    recv(svr_socket,&status,sizeof(status),0);
    //cout<<"last"<<endl;
    if(status){
		strcpy(buffer,uip.c_str());
	    send(svr_socket,buffer,BUFF_SIZE,0);
	    send(svr_socket,&uport,sizeof(uport),0);
		recv(svr_socket,&ipst,sizeof(ipst),0);
		if(ipst)
			return true;
		else return false;
	}
    else return false;
}

//creating user
bool createUser(string user, string pass, int svr_socket, int tracker_desc){
	int option = 1;
	char username[nm];
	char passwd[nm];
	char buffer[BUFF_SIZE];
	int status;
	strcpy(buffer,user.c_str());
	//cout<<"first"<<endl;
	send(svr_socket,&option,sizeof(option),0);
	//cout<<"second"<<endl;
	send(svr_socket,buffer,BUFF_SIZE,0);
	//cout<<"third"<<endl;
	strcpy(buffer,pass.c_str());
	send(svr_socket,buffer,BUFF_SIZE,0);
	//cout<<"fourth"<<endl;
	recv(svr_socket,&status,sizeof(status),0);
	//cout<<"last"<<endl;
	if(status)
    	return true;
	else return false;
}

//logout from the tracker
bool logout(string user, int svr_socket, int tracker_desc){
	int option = 5;
    char username[nm];
    char passwd[nm];
    char buffer[BUFF_SIZE];
    int status;
    strcpy(buffer,user.c_str());
    send(svr_socket,&option,sizeof(option),0);
    send(svr_socket,buffer,BUFF_SIZE,0);
    recv(svr_socket,&status,sizeof(status),0);
    if(status)
        return true;
    else return false;
}

//create group
bool createGroup(string user, int group_id, int svr_socket, int tracker_desc){
    int option = 6;
    char buffer[BUFF_SIZE];
    int status;
    strcpy(buffer,user.c_str());
    send(svr_socket,&option,sizeof(option),0);
    send(svr_socket,buffer,BUFF_SIZE,0);
	cout<<group_id<<endl;
    send(svr_socket,&group_id,sizeof(group_id),0);
    recv(svr_socket,&status,sizeof(status),0);
    if(status)
        return true;
    else return false;
}

//list all groups
void listAllGroups(int svr_socket, int tracker_desc){
	int option = 7;
	char buffer[BUFF_SIZE];
    int status=1;
	int group_id;
	int gsize =0;
	send(svr_socket,&option,sizeof(option),0);
	recv(svr_socket,&gsize,sizeof(gsize),0);
	while(gsize){
		recv(svr_socket,&group_id,sizeof(group_id),0);
		cout<<group_id<<" ";
		recv(svr_socket,buffer,BUFF_SIZE,0);
		cout<<buffer<<endl;
		gsize--;
		memset (buffer,'\0',BUFF_SIZE);
	}
}

//join group
void joinGroup(string uname, int gid, int svr_socket, int tracker_desc){
    int option = 8;
    char buffer[BUFF_SIZE];
    bool status;
	//strcpy(buffer,uname.c_str());
    send(svr_socket,&option,sizeof(option),0);
    //send(svr_socket,buffer,BUFF_SIZE,0);
    send(svr_socket,&gid,sizeof(gid),0);
	recv(svr_socket,&status,sizeof(status),0);
	if(status){
		cout<<"requested for joining"<<endl;
	}
	else{
		cout<<"join request failed"<<endl;
	}	
}

//listing all the pending request for a group
void listRequests(int gid, int svr_socket, int tracker_desc){
	int option = 9;
    char buffer[BUFF_SIZE];
    bool status;
	int preq;
    //strcpy(buffer,uname.c_str());
    send(svr_socket,&option,sizeof(option),0);
    //send(svr_socket,buffer,BUFF_SIZE,0);
    send(svr_socket,&gid,sizeof(gid),0);
    recv(svr_socket,&preq,sizeof(preq),0);
	if(!preq){
		cout<<"N0 pending requests"<<endl;
		return;
	}
	while(preq){
		recv(svr_socket,buffer,BUFF_SIZE,0);
		cout<<"requested "<<buffer<<endl;
		preq--;
	}
}
//Accept request
void acceptRequests(int gid, string un, string username, int svr_socket, int tracker_desc){
	int option = 10;
    char buffer[BUFF_SIZE];
    bool status=true;
    strcpy(buffer,un.c_str());
    send(svr_socket,&option,sizeof(option),0);
    send(svr_socket,&gid,sizeof(gid),0);
    send(svr_socket,buffer,BUFF_SIZE,0);
    strcpy(buffer,username.c_str());
    send(svr_socket,buffer,BUFF_SIZE,0);
	recv(svr_socket,&status,sizeof(status),0);
	if(status){
		cout<<"request accepted"<<endl;
	}
	else{
		cout<<"not able to process the request"<<endl;
	}
}

//Leave Group
void leaveGroup(int gid, string un, int svr_socket, int tracker_desc){
	int option=11;
	char buffer[BUFF_SIZE];
    bool status=true;
    strcpy(buffer,un.c_str());
    send(svr_socket,&option,sizeof(option),0);
    send(svr_socket,&gid,sizeof(gid),0);
    send(svr_socket,buffer,BUFF_SIZE,0);
    recv(svr_socket,&status,sizeof(status),0);
    if(status){
        cout<<"left group successfully"<<endl;
    }
    else{
        cout<<"not able to leave group"<<endl;
    }	
}

//extract file path and name
string parseFilepath(string fpath){
	int l;
	l = fpath.size() -1;
	string fname = "";
	while(fpath[l]){
		if(fpath[l] == '/'){
			break;
		}
		fname = fpath[l] + fname;
		l--;
	}
	return fname;
}

//upload filepath 
void uploadFile(int g_id,string un, string ufile, int svr_socket, int tracker_desc){
	int option = 3;
	string fpath = ufile;
	string sha;
	sha = shaCalculate(fpath);
	char buffer[BUFF_SIZE];
	string fname = parseFilepath(ufile);
	cout<<"fpath "<<fpath<<endl;
	cout<<"fname "<<fname<<endl;
	bool status;
	//sending the option
	send(svr_socket,&option,sizeof(option),0);
    //sending the group id of the requesting client
    send(svr_socket,&g_id,sizeof(g_id),0);
	//sending the filepath
    strcpy(buffer,fpath.c_str());
    send(svr_socket,buffer,BUFF_SIZE,0);
    //sending the filename
    strcpy(buffer,fname.c_str());
    send(svr_socket,buffer,BUFF_SIZE,0);
	//sending the sha of the file
	strcpy(buffer,sha.c_str());
    send(svr_socket,buffer,BUFF_SIZE,0);
	recv(svr_socket,&status,sizeof(status),0);
    if(status){
        cout<<"file uploaded successfully"<<endl;
    }
    else{
        cout<<"not able to upload the file"<<endl;
    }

}


//download file from peer
bool downloadFile(int g_id,string un, string fname, string dstpath, int svr_socket, int tracker_desc){
	pthread_t td1;
	int option = 4;
	char buffer[BUFF_SIZE];
	//char fname[BUFF_SIZE];
	char filepath[BUFF_SIZE];
	int nops;
	//strcpy(fname,fpath);
	string peerip;
	int peerport;
	//sending the option 
	send(svr_socket,&option,sizeof(option),0);
	//sending the user name of the requesting client
	send(svr_socket,&g_id,sizeof(g_id),0);
	strcpy(buffer,un.c_str());
    send(svr_socket,buffer,BUFF_SIZE,0);
	//sending the filename to be downloaded
	strcpy(buffer,fname.c_str());
    send(svr_socket,buffer,BUFF_SIZE,0);
	//receiving the no of peers
	recv(svr_socket,&nops,sizeof(nops),0);
	while(nops){
		recv(svr_socket,buffer,BUFF_SIZE,0);
    	strcpy(filepath,buffer);
    	//receiving the peer ip
    	recv(svr_socket,buffer,BUFF_SIZE,0);
    	peerip = buffer;
    	//receiving the peer port
	    recv(svr_socket,&peerport,sizeof(peerport),0);
		nops--;
	}
	cout<<"file path "<<filepath<<" peer ip "<<peerip<<" peer port "<<peerport<<endl;
	struct dstruct *temp = (struct dstruct*)malloc(sizeof(dstruct));
	temp->pip = peerip;
	temp->filepath = filepath;
	temp->dpath = dstpath;
	temp->port = peerport;
	int cstatus = pthread_create(&td1, NULL, peerClient,(void*)temp);	
	
}

//calculate the sha of a file
string shaCalculate(string filepath){
	string sha = "";
	unsigned char digest[SHA_DIGEST_LENGTH];
    char mdString[SHA_DIGEST_LENGTH*2+1];
    char fl[BUFF_SIZE];
    int i = 0;
    ifstream file(filepath);
    while(!file.eof() && i < BUFF_SIZE)
    {
        file.get(fl[i]); //reading single character from file to array
        i++;
    }
	//finding the SHA1 of the file
    SHA1((unsigned char*)&fl, strlen(fl), (unsigned char*)&digest);

    for(int i = 0; i < SHA_DIGEST_LENGTH; i++)
         sprintf(&mdString[i*2], "%02x", (unsigned int)digest[i]);
    cout<<mdString<<endl;
	sha = mdString;

	return sha;
}

void listFiles(int group_id,int  svr_socket, int tracker_status){
	ifstream file("finfo");
	int gid;
	string us, fpath, fname,sha;
	while(true){
		file>>gid >>us>>fname>>fpath>>sha;
		if(file.eof())
			break;
		if(group_id == gid){
			cout<<gid<<" "<<us<<" "<<fname<<" "<<fpath<<" "<<sha<<endl;
		}
	}
}

//int socket(int domain, int type, int protocol);
int main(int argc, char* argv[]){
	
	//command line arguments
	ip = argv[1];
	port = stoi(argv[2]);
	//cout<<"port "<<port<<endl;
	//port1 = stoi(argv[3]);
	string conf_file = argv[3];
	
	//declarations
	string st;
    pthread_t td1,td2;
    char filepath[nm];
	string tfname,username,passwd;
	int tport;
	int option;
	int userFlag = 0;
	int loginFlag = 0;
	bool status;
	int log_option;
	//vector<string> tracker_ip;
	//vector<int> tracker_port;
	ifstream file(conf_file);
	

	for(int i=0;i<2;i++){
		file >>tfname;
		tracker_ip.push_back(tfname);
		file >> tport;
		tracker_port.push_back(tport);
	}
	
	int statust = pthread_create(&td1, NULL, peerServer,NULL);

	//connecting to the tracker
	int svr_socket;
    //creating socket for the communication
    svr_socket = socket(AF_INET, SOCK_STREAM, 0);
    //structure for defining the connection attributes
    struct sockaddr_in server;
    //assigning values to the structure like type, port and address 
    server.sin_family = AF_INET;
    server.sin_port = htons(tracker_port[0]);
    server.sin_addr.s_addr = inet_addr(ip.c_str());
    
	int tracker_status = connect(svr_socket, (struct sockaddr*) &server, sizeof(server));

	if(tracker_status <= 0){
		server.sin_port = htons(tracker_port[1]);
		tracker_status = connect(svr_socket, (struct sockaddr*) &server, sizeof(server));
	}
	
	while(1){
		cout<<"1.Create User "<<endl;
		cout<<"2.Login "<<endl;
		cin>>option;
		if(option == 1){
			cout<<"Enter username"<<endl;
			cin>>username;
			cout<<"Enter password"<<endl;
			cin>>passwd;
			status = createUser(username, passwd, svr_socket, tracker_status);
			if(status == true){
				cout<<"user create successful"<<endl;
				userFlag = 1;
			}
			else{
				cout<<"user create failed"<<endl;
            	continue;
			}
		}
		else if(option == 2){
			cout<<"Enter username"<<endl;
            cin>>username;
            cout<<"Enter password"<<endl;
            cin>>passwd;
            status = login(username, passwd, svr_socket, tracker_status);
            if(status == true){
                cout<<"login successful"<<endl;
                loginFlag = 1;
			}
			else{
				cout<<"login failed"<<endl;
                continue;
			}
		}
		if(loginFlag == 1){
			while(1){
			cout<<"3.Upload"<<endl;
			cout<<"4.Download"<<endl;
			cout<<"5.Logout"<<endl;
			cout<<"6.Create Group"<<endl;
			cout<<"7.List Groups"<<endl;
			cout<<"8.Join Group"<<endl;
			cout<<"9.List request"<<endl;
			cout<<"10.Accept Request"<<endl;
			cout<<"11.Leave Group"<<endl;
			cout<<"12.List Files"<<endl;
			/*cout<<""<<endl;*/
			/*cin>>st;
			if(){
				cout<<"Enter filepath "<<endl;
				cin>>filepath;
				int cstatus = pthread_create(&td1, NULL, peerClient,&filepath);
			}*/		
			cin>>log_option;
			if(log_option == 3){
                int g_id;
                string ufpath;
                cin>>ufpath>>g_id;
                uploadFile(g_id,username, ufpath, svr_socket, tracker_status);
                //int cstatus = pthread_create(&td1, NULL, peerClient,&filepath);
            }
			else if(log_option == 4){
				int g_id;
				string dfpath,dstpath;
				cin>>g_id>>dfpath>>dstpath;
				downloadFile(g_id,username, dfpath, dstpath, svr_socket, tracker_status);
				//int cstatus = pthread_create(&td1, NULL, peerClient,&filepath);
            }
			else if(log_option == 5){
				status = logout(username, svr_socket, tracker_status);
				if(status == 1){
					cout<<"logged out successfully"<<endl;
					break;
				}
			}
			else if(log_option == 6){
                int group_id;
                cin>>group_id;
                status = createGroup(username, group_id, svr_socket, tracker_status);
                if(status){
                    cout<<"group created"<<endl;
                }
                else{
                    cout<<"error in group creation"<<endl;
                }
            }
			else if(log_option == 7){
				listAllGroups(svr_socket, tracker_status);
			}
			else if(log_option == 8){
				int group_id;
				cin>>group_id;
				joinGroup(username, group_id, svr_socket, tracker_status);
			}
			else if(log_option == 9){
				int group_id;
				cin>>group_id;
				listRequests(group_id, svr_socket, tracker_status);
			}
			else if(log_option == 10){
                int group_id;
				string un;
                cin>>group_id>>un;
                acceptRequests(group_id, un, username, svr_socket, tracker_status);
            }
			else if(log_option == 11){
                int group_id;
                cin>>group_id;
                leaveGroup(group_id, username, svr_socket, tracker_status);
            }
			else if(log_option == 12){
                int group_id;
                cin>>group_id;
                listFiles(group_id, svr_socket, tracker_status);
            }
			}
		}
	}
	return 0;

}
