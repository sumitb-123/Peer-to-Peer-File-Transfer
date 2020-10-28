#include <sys/types.h>
#include <bits/stdc++.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include<unistd.h>
#include <string.h>
#include <iostream>
#include<pthread.h>
#include<fstream>
#define BUFF_SIZE 1024
#define def_ip "127.0.0.1"
#define def_port 11123
#define trd 100
using namespace std;
int port = def_port;
int fl = 0;
string ip = def_ip;

struct fileinfo{
	string username;
	string filename;
	string sha;
	string path;
	int gid;
};
//file information
vector<struct fileinfo> finfo;
//map for saving in which groups user is
map<string, vector<int>> usergroup;
//map for saving user & password
map<string, string> users;
//user and active or not
map<string, int> active;
//group id and users list who has requested for joining the group
map<int, vector<string>> requests;
//group id and owner
map<int, string> groups;
//members of the group
map<int,vector<string>> members;
//logged in peers ip and port 
map<string, pair<string, int>> peerinfo;

//load data structures with data from files
/*void loadFiles(){
	int gid;
	string un;
	ifstream file("groups");
	while(!file.eof()){
		if(file.tellg() != -1)
			break;
		file >> gid;
		file >> un;
		groups[gid] = un;
	}
	file.close();
	ifstream file("requests");
    while(!file.eof()){
        if(file.tellg() != -1)
            break;
        file >> gid;
        file >> un;
        groups[gid] = un;
    }
    file.close();
}*/

//load fileinfo from the file
void loadFinfo(){
	cout<<"inside load info"<<endl;
	ifstream file("finfo");
	//struct fileinfo *temp = (struct fileinfo *)malloc(sizeof(struct fileinfo));
	string fn;
	string us;
	string fp;
	string sh;
	int g;
	while(!file.eof()){
		if(file.tellg() == -1)
			 break;
		fileinfo temp;// = (struct fileinfo *)malloc(sizeof(struct fileinfo));
		file>>g>>us>>fn>>fp>>sh;
		//cout<<g<<" "<<us<<" "<<fn<<" "<<fp<<" "<<sh<<endl;
		temp.filename = fn;
		temp.path = fp;
		temp.username = us;
		temp.sha = sh;
		temp.gid = g;
		cout<<"gid in lf "<<temp.gid<<endl;
		finfo.push_back(temp);
		
	}
	if(finfo.size() > 1){
		finfo.pop_back();
	}
	file.close();
	cout<<"leaving load info"<<endl;

}

//method to parse the info file and set ip and port 
void parseFile(char *filename, int tno){
	
	char fname[trd];
	ifstream rfile;
	int i,item=0;
	string temp="",temp1,temp2;
	rfile.open(filename);
	while(tno--){
		rfile>>temp1>>temp2;
		//cout<<"from the file "<<temp1<<" "<<temp2<<endl;
	}
	i = 0;
	//extracting ip and port of tracker from info file
	ip = temp1;
	port = stoi(temp2);
	while(fname[i]){
		if(fname[i] != ' ')
			temp+=fname[i];
		else{
			item++;
			if(item == 1){
				cout<<"temp "<<temp<<endl;
			}
			else if(item == 2){
				cout<<"temp "<<temp<<endl;
			}
			temp = "";
		}
		i++;
	}
}

//read user password from file
bool readUserU(string user, string pass){
    string uname, passwd;
	if(users.find(user) != users.end())
		return true;
	else return false;
    /*ifstream file("users");
    while(!file.eof()){
        file >> uname;
        file >> passwd;
        if(!user.compare(uname)){
            return true;
        }
    }*/
    return false;
}

//update the request file
void updateRequests(){
	ofstream file("requests");
    for(auto it: requests){
       file << it.first;
       for(auto e: it.second){
	       file <<" "<<e;
       }
       file<<endl;
    }
	file.close();
}

//update the members
void updateMembers(){
    ofstream file("members");
    for(auto it: members){
       file << it.first;
       for(auto e: it.second){
           file <<" "<<e;
       }
       file<<endl;
    }
    file.close();
}

//writing to the usergroup file
void updateUsergroupFile(){
	ofstream file("usergroup");
    for(auto it: usergroup){
       file << it.first;
       for(auto e: it.second){
           file <<" "<<e;
       }
       file<<endl;
    }
    file.close();
}


//read user password from file
bool readUser(string user, string pass){
	//string uname, passwd;
	cout<<user<<" "<<pass<<endl;
	cout<<users[user] <<endl;
	if(users.find(user) != users.end() && users[user] == pass)
		return true;
	else return false;
	/*ifstream file("users");
	while(!file.eof()){
		file >> uname;
		file >> passwd;
		if(!user.compare(uname) && !pass.compare(passwd)){

			return true;
		}
	}
	return false;*/
}

//write user password into the file
void writeUser(string user, string pass){
	ofstream file;
	file.open("users");
	users[user] = pass;
	for(auto u: users){
		file << u.first << " " << u.second << endl;
	}
}

//maintaining the active status
void writeActive(string us){
	ofstream file("active");
	for(auto it: active){
		file << it.first <<" "<<it.second<<endl;
	}
}

//create group
int createGroup(string user, int group_id){
	ifstream file("groups");
	int grp_id;
	string uname;
    /*while(!file.eof()){
        file >> grp_id;
        file >> uname;
        if(grp_id == group_id){
            return 0;
        }
    }*/
	if(groups.find(group_id) == groups.end()){
		groups[group_id] = user;
		ofstream ofile("groups");
		for(auto g: groups){
			ofile <<g.first<<" "<<g.second<<endl;
		}
		return 1;
	}
	else return 0;
	
}

//request for joining group
bool joinGroupRequest(string us, int gid){
	if(groups.find(gid) != groups.end()){	
		requests[gid].push_back(us);
		ofstream file("requests");
		for(auto it: requests){
			file << it.first;
			for(auto e: it.second){
				file <<" "<<e;
			}
			file<<endl;
		}
		return true;
	}
	else{
		return false;
	}
	
}

//update the group usr belongs
void updateUserGroup(int gid, string us){
	if(usergroup.find(us) != usergroup.end()){
		vector<int> :: iterator it;
		it = find(usergroup[us].begin(),usergroup[us].end(),gid);
		if(it == usergroup[us].end()){
			usergroup[us].push_back(gid);
		}
	}
	else{
		usergroup[us].push_back(gid);
	}
	updateUsergroupFile();

}

//remove the gid from user list
void removeUserGroup(int gid, string us){
	if(usergroup.find(us) != usergroup.end()){
		vector<int> :: iterator it;
        it = find(usergroup[us].begin(),usergroup[us].end(),gid);
        if(it != usergroup[us].end()){
            usergroup[us].erase(it);
            if(usergroup[us].size() == 0)
                usergroup.erase(us);
        }
		updateUsergroupFile();
	}
}


//accept requests
bool acceptRequests(int gid, string uname, string us){
	if(requests.find(gid) != requests.end() && groups[gid] == us){
		vector<string> :: iterator it;
		it = find(requests[gid].begin(),requests[gid].end(),uname);
		if(it != requests[gid].end()){
			requests[gid].erase(it);
			if(requests[gid].size() == 0)
				requests.erase(gid);
		}
		//updating the requests file
		updateRequests();
		members[gid].push_back(uname);
		updateMembers();
		updateUserGroup(gid, uname);
		return true;
	}
	else return false;
}

//leave group
bool leaveGroup(int gid, string uname){
	if(members.find(gid) != members.end()){
		vector<string> :: iterator it;
        it = find(members[gid].begin(),members[gid].end(),uname);
        if(it != members[gid].end()){
            members[gid].erase(it);
            if(members[gid].size() == 0)
                members.erase(gid);
        }
        //updating the requests file
        updateMembers();
		removeUserGroup(gid,uname);
		return true;
	}
	else return false;
}

//save users ip port
bool saveIpPort(string us, string uip, int port){
	pair<string, int> temp;
	temp.first = uip;
	temp.second = port;
	peerinfo[us] = temp;
	ofstream file("peerinfo");
	for(auto e: peerinfo){
		file <<e.first <<" "<<e.second.first<<" "<<e.second.second<<endl;
	}
	file.close();
	return true;
}

//remove user ip and port
void removeIpPort(string us){
	if(peerinfo.find(us) != peerinfo.end()){
		peerinfo.erase(us);
	}
	ofstream file("peerinfo");
    for(auto e: peerinfo){
        file <<e.first <<" "<<e.second.first<<" "<<e.second.second<<endl;
    }
    file.close();
}

//getting all the filepath for a gid
vector<vector<string>> getPath(int gid, string fname){

	map<string,bool> mems;
	cout<<"inside get path"<<endl;
	vector<vector<string>> peers;
	for(auto mem: members){
		cout<<"groups user "<<mem.first<<endl;
	}
	if(members.find(gid) != members.end()){
		for(auto g: members[gid]){
			mems[g] = true;
		}
		for(auto e: finfo){
			if(mems.find(e.username) != mems.end() && active[e.username] == 1 && e.filename == fname){
				vector<string> onepeer;
				onepeer.push_back(e.path);
				onepeer.push_back(peerinfo[e.username].first);
				onepeer.push_back(to_string(peerinfo[e.username].second));
				peers.push_back(onepeer);
			}
			
    	}
	cout<<"size of peers "<<peers.size()<<endl;
	return peers;
	}
	cout<<"outside get path peers"<<endl;
	return peers;
}

//download information send to peer
void downloadFile(int client_socket,int gid, string us, string fname){
	int peerport=11123;
	char buffer[BUFF_SIZE];
	int no_of_matches;
	string peerip = "127.0.0.1";
	string filepath = "test";
	cout<<"inside download file"<<endl;
	loadFinfo();
	vector<vector<string>> peers = getPath(gid,fname);
	no_of_matches = peers.size();
	send(client_socket, &no_of_matches, sizeof(no_of_matches),0);
	for(auto e : peers){
		cout<<"peers info array "<<e[0]<<" "<<e[1]<<" "<<e[2]<<endl;
		strcpy(buffer,e[0].c_str());
    	send(client_socket, buffer, BUFF_SIZE,0);
    	strcpy(buffer,e[1].c_str());
    	send(client_socket, buffer,BUFF_SIZE,0);
		peerport = stoi(e[2]);
    	send(client_socket, &peerport, sizeof(peerport),0);
	}
	/*strcpy(buffer,filepath.c_str());
	send(client_socket, buffer, BUFF_SIZE,0);
	strcpy(buffer,peerip.c_str());
	send(client_socket, buffer,BUFF_SIZE,0);
	send(client_socket, &peerport, sizeof(peerport),0);*/

}

bool uploadFile(int gid, string fpath, string fname, string us, string sha){
	ofstream file("finfo", ios::app);
	file<<gid<<" "<<us<<" "<<fname<<" "<<fpath<<" "<<sha<<endl;
	file.close();
	return true;
}

//thread for handling the client request
void *clientHandler(void *arg){
	
	char Buffer[BUFF_SIZE];
	string us;
	string pass;
	int option;
	int size;
	int status = 1;
    int client_socket = *((int *)arg);
	cout<<"connected to client handler"<<endl;
	while(1){
		//cout<<"first "<<endl;
    	recv(client_socket, &option, sizeof(int), 0);
		if(option == 1){
		 	recv(client_socket, Buffer, BUFF_SIZE, 0);
			us = Buffer;
		 	recv(client_socket, Buffer, BUFF_SIZE, 0);
			pass = Buffer;
			int check = readUserU(us,pass);
			if(!check){
				writeUser(us,pass);
				status = 1;
                send(client_socket, &status, sizeof(status),0);
			}
			else{
				status = 0;
                send(client_socket, &status, sizeof(status),0);
			}
			/*if(users.find(us) == users.end()){
				users[us] = pass;
				status = 1;
	            send(client_socket, &status, sizeof(status),0);
			}
			else{
				status = 0;
				send(client_socket, &status, sizeof(status),0);
			}*/
		}
		else if(option == 2){
            recv(client_socket, Buffer, BUFF_SIZE, 0);
            us = Buffer;
            recv(client_socket, Buffer, BUFF_SIZE, 0);
            pass = Buffer;
			bool check = readUser(us,pass);
			string uip;
			int uport;
			bool ss;
		    if(check){
                //writeUser(us,pass);
                status = 1;
				active[us] = 1;
				writeActive(us);
                send(client_socket, &status, sizeof(status),0);
				recv(client_socket, Buffer, BUFF_SIZE, 0);
				uip = Buffer;
				recv(client_socket, &uport, sizeof(uport), 0);
				ss = saveIpPort(us,uip,uport);
				send(client_socket, &ss, sizeof(ss),0);
				
            }
            else{
                status = 0;
                send(client_socket, &status, sizeof(status),0);
            }

            /*if(users.find(us) != users.end() && users[us] == pass){
                status = 1;
				active[us] = 1;
                send(client_socket, &status, sizeof(status),0);
            }
            else{
                status = 0;
                send(client_socket, &status, sizeof(status),0);
            }*/
        }
		else if(option == 3){
			int grp_id;
			string ufpath;
			string ufname;
			string sha;
			char buffer[BUFF_SIZE];
			bool ustat;
			recv(client_socket, &grp_id, sizeof(grp_id), 0);
            recv(client_socket, buffer, BUFF_SIZE, 0);
            ufpath = buffer;
            recv(client_socket, buffer, BUFF_SIZE, 0);
            ufname = buffer;
			recv(client_socket, buffer, BUFF_SIZE, 0);
            sha = buffer;
            ustat = uploadFile(grp_id,ufpath, ufname, us,sha);
			send(client_socket, &ustat, sizeof(ustat),0);
		}

		else if( option == 4){
			int grp_i;
			string reqfile;
			recv(client_socket, &grp_i, sizeof(grp_i), 0);
			recv(client_socket, Buffer, BUFF_SIZE, 0);
			us = Buffer;
			recv(client_socket, Buffer, BUFF_SIZE, 0);
			reqfile = Buffer;
			downloadFile(client_socket,grp_i,us,reqfile);	
		}
		else if( option == 5){
			recv(client_socket, Buffer, BUFF_SIZE, 0);
            us = Buffer;
			//recv(client_socket, Buffer, BUFF_SIZE, 0);
			//pass = Buffer;
			//writeUser(us,pass);
            active[us] = 0;
			writeActive(us);
			removeIpPort(us);
            status = 1;
            send(client_socket, &status, sizeof(status),0);
        }
		else if(option == 6){
			int grp;
			recv(client_socket, Buffer, BUFF_SIZE, 0);
            us = Buffer;
			recv(client_socket, &grp, sizeof(grp), 0);
			//cout<<"group"<<grp<<endl;
			status = createGroup(us, grp);
			send(client_socket, &status, sizeof(status),0);
		}
		else if(option == 7){
			int more_grp = 1;
			//ifstream file("groups");
    		int grp_id;
    		string uname;
			char user[BUFF_SIZE];
			int gsize = 0;
			if(groups.size()>0){
				gsize = groups.size();
			}
			send(client_socket, &gsize, sizeof(gsize),0);
			for(auto u : groups){
				grp_id=u.first;
				strcpy(user, u.second.c_str());
                send(client_socket, &grp_id, sizeof(grp_id),0);
                send(client_socket, user, BUFF_SIZE,0);
			}
    		/*while(!file.eof()){
        		file >> grp_id;
       		 	file >> uname;
				cout<<"grp "<<grp_id<<" un "<<uname<<endl;
				//if(file.tellg() == -1) break;
				strcpy(user, uname.c_str());
				send(client_socket, &more_grp, sizeof(more_grp),0);
				send(client_socket, &grp_id, sizeof(grp_id),0);
				send(client_socket, user, sizeof(user),0);
    		}
			file.close();*/
			
		}
		else if(option == 8){
			int gid;
			bool st;
			recv(client_socket, &gid, sizeof(gid), 0);			
			st = joinGroupRequest(us,gid);
			send(client_socket, &st, sizeof(st),0);
		}
		else if(option == 9){
			int gid;
			int pdr = 0;
            recv(client_socket, &gid, sizeof(gid), 0);
			if(requests.find(gid) != requests.end()){
            	pdr = requests[gid].size();
				send(client_socket, &pdr, sizeof(pdr),0);
				int ru = 0;
				char rur[50];
				while(pdr){
					strcpy(rur,requests[gid][ru].c_str());
					send(client_socket,rur,sizeof(rur),0);
					pdr--;
				}
			}
			else{
				send(client_socket, &pdr, sizeof(pdr),0);
			}
		}
		else if(option == 10){
			int gid;
			string uname;
			string username;
			char buffer[BUFF_SIZE];
			bool sts;
			recv(client_socket, &gid, sizeof(gid), 0);
			recv(client_socket, buffer, BUFF_SIZE, 0);
			uname = buffer;
			recv(client_socket, buffer, BUFF_SIZE, 0);
			username = buffer;
			sts = acceptRequests(gid,uname,username);
			send(client_socket, &sts, sizeof(sts),0);
		}
		else if(option == 11){
            int gid;
            string uname;
            char buffer[BUFF_SIZE];
            bool sts;
            recv(client_socket, &gid, sizeof(gid), 0);
            recv(client_socket, buffer, BUFF_SIZE, 0);
            uname = buffer;
            sts = leaveGroup(gid,uname);
            send(client_socket, &sts, sizeof(sts),0);
        }
	}
    close(client_socket);
    pthread_exit(NULL);	
}

void *trackerServer(void *sock){
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
        sts = pthread_create(&thread[i], NULL, clientHandler, &client_socket);
        i++;
    }
    //closing the socket
    close(svr_socket);
    pthread_exit(NULL);

}


int main(int argc, char *argv[]){
	
	if(argc != 3){
		cout<<"check the arguments"<<endl;
		exit(0);
	}

	int tracker_no = stoi(argv[2]);
	string quit;
	parseFile(argv[1], tracker_no);
	cout<<"port "<<port<<" ip "<<ip<<endl;
    pthread_t td1;
    int status = pthread_create(&td1, NULL, trackerServer,NULL);
	//pthread_join(td1,NULL);
	while(1){
		//cout<<"quit to close the tracker"<<endl;
		cin>>quit;
		if(!quit.compare("quit")){
			cout<<"Bye !"<<endl;
			break;
		}
	}
	return 0;
}
