#include<bits/stdc++.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include<thread>
#include <cstring>  
#include <sstream>
#include<dirent.h>
#include <unistd.h>
#include <fstream>
#include <iostream>

#include <sys/stat.h>

#include <openssl/md5.h>
using namespace std;

string hash_val(string path){

	ifstream file(path, ifstream::binary);
	MD5_CTX md5Context;
	MD5_Init(&md5Context);
	char buff[1024 * 16];
	while (file.good()) {
    		file.read(buff, sizeof(buff));
    		MD5_Update(&md5Context, buff, file.gcount());
	}
	unsigned char result[MD5_DIGEST_LENGTH];
	MD5_Final(result, &md5Context);
	stringstream md5string;
	md5string << std::hex << std::uppercase << std::setfill('0');
	for (const auto &byte: result)
   		 md5string << std::setw(2) << (int)byte;
        string sl = md5string.str();
        transform(sl.begin(), sl.end(), sl.begin(), ::tolower); 
     	return sl;
        
}; 


bool compare2strings(string s1, string s2){
    if(s1.compare(s2) < 0) return 1;
    else return 0;
}

void listening_for_connections(int socket_server,int unique_id, int n, vector<string> my_files,char* argv2){
    //cout << "inside listening" << endl;
    vector<string> neigh_files;
    int h=n,max_fd = socket_server+1;
    int l = listen(socket_server,20);
    //while(l!=0) cout << "l value not 0";
    if(l == 0){
        //cout << "Hey, I am listening\n";
        fd_set fd_list;
        fd_set fd_active;
        FD_ZERO(&fd_list); FD_ZERO(&fd_active);
        FD_SET(socket_server,&fd_list);
        while(h>0){
            struct timeval timval;
                timval.tv_sec = 1;
                timval.tv_usec = 0;
            fd_active = fd_list;
            select(max_fd, &fd_active, NULL, NULL, &timval);
            for(int i=0;i<=max_fd;i++){
                if(FD_ISSET(i,&fd_active)){
                    if(i == socket_server){
                        struct sockaddr_in their_addr;
                        socklen_t sin_size = sizeof(struct sockaddr_in);
                        int a = accept(socket_server, (struct sockaddr *)&their_addr, &sin_size);
                        //cout << "accepted "<<a << "\n";
                        string s = to_string(unique_id);
                        if (a != -1) {
                            h--;
                            char buffer[2000];
                            for(int k = 0;k<2000;k++){
                                buffer[k] =0;
                            }
                            int sent = send(a,s.c_str(),strlen(s.c_str()),0);
                            int resc_n = recv(a,buffer,2000,0 );
                            string name = string(buffer);
                            //cout << name << endl;
                            neigh_files.push_back(name);
                            FD_SET(a,&fd_list);
                            max_fd = max(max_fd,a+1);
                            // cout << "data sent " << sent << "\n";
                        }
                    }
                   }
                
            }
            
        }
        
        while(1){
            struct timeval timval;
                timval.tv_sec = 1;
                timval.tv_usec = 0;
            fd_active = fd_list;
            select(max_fd, &fd_active, NULL, NULL, &timval);
            for(int i=0;i<=max_fd;i++){
                if(FD_ISSET(i,&fd_active)){
                    if(i != socket_server){
                        char buffer[256];
                        for(int k=0;k<256;k++) buffer[k] = 0;
                        int res= recv(i,buffer,256,0);
                        //if(res == -1 )cout<<"error 111"<<string(buffer)<<endl;
                        //cout<<res <<" received req for "<<string(buffer)<<endl;
                        stringstream file_depth;
                        file_depth << string(buffer);
                        string file1,dep;
                        file_depth >> file1;
                        file_depth >> dep;
                        //cout<<"at 117\n";
                        //cout<< file1<<" "<<dep<<endl;
                        if(dep.compare("1")==0){
                            bool is_present = false;
                            for(int j=0;j<my_files.size();j++){
                                if(file1.compare(my_files[j]) == 0) {is_present=1;break;}
                            }
                            string yes = "1", no = "0";
                            if(is_present){
                                //cout<<"inside 124\n";
                            	int sent = send(i,yes.c_str(),strlen(yes.c_str()),0 );
                                 //cout<<send<< " sent 127"<<argv2+file1<<endl;
                                
                            	string filenamepath= argv2+file1;
                        	    ifstream file(filenamepath, ios::binary);
                               // cout<<"sent 127 "<<filenamepath<<endl;
				                file.seekg(0, ios::end);
				                int size = file.tellg();
				                file.seekg(0, ios::beg);
				                char* file_buffer = new char[size];
				                file.read(file_buffer, size);
				                file.close();
				
				                int* fsize = &size;
				                int send_size= send(i,(char*)fsize,sizeof(int),0);
  				                int send_file = send(i, file_buffer, size, 0);
  				//cout<<"sent file "<<filenamepath<<endl;
                            }
                            else{int sent = send(i,no.c_str(),strlen(no.c_str()),0 );}
                        } 
                        else{
                            //bool is_present=false;
                            int min_unique_id = INT_MAX,validity = INT_MAX,min_port;
                            for(int j=0;j<neigh_files.size();j++){
                                string list_of_files = neigh_files[j];
                                int unique_id_of_particular_neigh,neigh_port,number_of_files;
                                stringstream compile_unique_file;
                                compile_unique_file << list_of_files;
                                compile_unique_file >> unique_id_of_particular_neigh;
                                string name;
                                compile_unique_file >> neigh_port;
                                compile_unique_file >> number_of_files;
                                for(int k=0; k<number_of_files; k++){
                                    compile_unique_file >> name;
                                    if(name.compare(file1) == 0){
                                        if(min_unique_id <= unique_id_of_particular_neigh) min_port=neigh_port;
                                        min_unique_id = min(min_unique_id, unique_id_of_particular_neigh);
                                        break;
                                    }
                                }
                                
                            }
                            //cout << "file : " << file << " at " << min_unique_id;
                            if(min_unique_id!= validity){
                                string yes = "1 " + to_string(min_unique_id)+" "+ to_string(min_port);
                                int sent = send(i,yes.c_str(),strlen(yes.c_str()),0);
                            }
                            else{
                                string no = "0";
                                int sent = send(i,no.c_str(),strlen(no.c_str()),0);
                            }
                        }
                    
                    }
                    else { 
                	    struct sockaddr_in their_addr;
                        socklen_t sin_size = sizeof(struct sockaddr_in);
                	    int c = accept(socket_server, (struct sockaddr *)&their_addr, &sin_size);
                	    char buffer[2000];
                        for(int k = 0;k<2000;k++){
                            buffer[k] =0;
                        }
                        int resc_filename_2 = recv(c,buffer,2000,0 );
                        string filenamepath= argv2+string(buffer);
                        //cout<<filenamepath<<" "<<187<<endl;
                        ifstream file(filenamepath, ios::binary);
			            file.seekg(0, ios::end);
			            int size = file.tellg();
			            file.seekg(0, ios::beg);
			            char* file_buffer = new char[size];
			            file.read(file_buffer, size);
			            file.close();
				
			            int* fsize = &size;
			            int send_size_2= send(c,(char*)fsize,sizeof(int),0);
  			            int send_file_2 = send(c, file_buffer, size, 0);
  			            close(c);
  	                }
                               	
                      
            }
        }
      }
     }
    
    
    //else cout << "not listening" << endl;
    return;

}
void connecting_to_neighbours( int n, string data,int unique_id, int files_num, vector<string> files_data, vector<string> my_files,int port,char* argv2){
    //cout << "Inside connecting thread" << endl;
    string my_files_compilation = to_string(unique_id)+" "+to_string(port)+" "+to_string(my_files.size());
    for(int i=0;i<my_files.size();i++){
        my_files_compilation += " "+my_files[i];
    }
    vector<int> fileExistence[files_num],fileExistence1[files_num],file_ports[files_num];
    vector<char*> filedata[files_num];
    vector<int> filesizes[files_num];
    int socket_client[n], connect_list[n], neighbor_id[n], neighbor_port[n],unique_ids[n];
    bool all_notconnect = true;
    stringstream list_stream;
    list_stream << data;
    for(int i=0; i<n; i++) {
        socket_client[i] = socket(AF_INET, SOCK_STREAM, 0);
        connect_list[i] = -1;
        list_stream >> neighbor_id[i];
        list_stream >> neighbor_port[i];
    }
    while(all_notconnect) {
        all_notconnect = false;
        for(int i=0;i<n;i++){
            //cout << n_id << " " << n_port << "\n";
            //int n_id = data[3*i]-'0', n_port = data[3*i+2]-'0';
            struct sockaddr_in dest_addr; // will hold the destination addr
            dest_addr.sin_family = AF_INET; // host byte order
            dest_addr.sin_port = htons(neighbor_port[i]); // network byte order
            dest_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
            memset(&(dest_addr.sin_zero), '\0', 8); // zero the rest of the struct 
            
            if (connect_list[i] == -1) {
                int a = connect(socket_client[i], (struct sockaddr *)&dest_addr, sizeof(struct sockaddr));
                if (a == -1 ) {
                    all_notconnect = true;
                    continue;
                }
                
                char buffer[256];
                for(int k=0;k<256;k++) buffer[k] = 0;
                connect_list[i] = a;
                int res = recv(socket_client[i], buffer, 256, 0);
                int sent_my_files = send(socket_client[i], my_files_compilation.c_str(), strlen(my_files_compilation.c_str()),0);
                stringstream list_stream1;
                list_stream1 << buffer;
                list_stream1 >> unique_ids[i];
                
            }            
        }
    }
    for(int i=0;i<n;i++){
        cout << "Connected to " << neighbor_id[i]<< " with unique-ID "<< unique_ids[i] << " on port " << neighbor_port[i] << endl;
    }
    
    for(int i=0;i<files_num;i++){
        string name=files_data[i];
        name += " 1";
        for(int j=0;j<n;j++){
                                //cout<<"inside line 267"<<endl;

            int sent = send(socket_client[j],name.c_str(),strlen(name.c_str()),0 );
            //cout<<"asking "<<name<<"  "<<unique_ids[j]<<endl;
            char yes_no[256];
            yes_no[0] = 0;
            yes_no[1] = 0;
            //cout << "276 " << endl;
            int res = recv(socket_client[j],yes_no, 256,0);
            if(res != -1){
                //cout << string(yes_no) << " :yes_no " << endl;
                if(yes_no[0] == '1') {
                    fileExistence[i].push_back(unique_ids[j]);
                    //cout<<"inside line 275"<<endl;
                    int filesize = 0;
		            int rec_size = recv(socket_client[j], (char*)&filesize, sizeof(filesize), 0);
		            char* filebuf = new char[filesize];
		            memset(filebuf,'\0', filesize);
	                int rec = recv(socket_client[j], filebuf, filesize, MSG_WAITALL);
            		//cout<<name<<" "<<endl;
            	    filedata[i].push_back(filebuf);
                    filesizes[i].push_back(filesize); 
            		//cout<<filedata[i].back()<<endl;
                }
            }
           // else cout <<unique_ids[j]<< " rec -1 294 " <<name<<  endl;
        }
    }
    
    for(int i=0;i<files_num;i++){
        if(fileExistence[i].size() == 0){
            string name=files_data[i];
            name += " 2";
            for(int j=0;j<n;j++){
                int sent = send(socket_client[j],name.c_str(),strlen(name.c_str()),0 );
                char yes_no[256];
                for(int k=0;k<256;k++) yes_no[k] = 0;
                int res = recv(socket_client[j],yes_no, 256,0);
                stringstream files_2;
                files_2 << string(yes_no);
                string done_not;
                files_2 >> done_not;
                if(res != -1){
                    if(done_not.compare("1") == 0){ 
                        int unique_neigh;
                        files_2 >> unique_neigh;
                        int port_neigh;
                	    files_2 >> port_neigh;
                        fileExistence1[i].push_back(unique_neigh);
                        file_ports[i].push_back(port_neigh);
                        
                    }
                }
                //else cout << "rec -1" <<  endl;
            }
        }
    }
    int min_id_index=0;
    for(int i=0;i<files_num;i++){
        string name=files_data[i];
        if(fileExistence[i].size() > 0){
        	min_id_index=min_element(fileExistence[i].begin(),fileExistence[i].end()) - fileExistence[i].begin();
        	char* dat = filedata[i][min_id_index];
        	//cout<<dat<<endl;
        	string d="Downloaded/";
        	mkdir((argv2+d).c_str(),0777);
        	ofstream myfile;
        	myfile.open(argv2+d+name, ios::binary);
        	//cout<<argv2+d+name<<endl;
		    myfile.write(dat,filesizes[i][min_id_index]);
            //cout<<argv2+d+name<<" 340\n";
		    delete[] dat;
		    myfile.close();
		    string hash = hash_val(argv2+d+name);// cout <<files_data[i]<< hash <<" "<<*min_element(fileExistence[i].begin(), fileExistence[i].end())<< endl;
        	cout  <<"Found " << files_data[i] << " at " <<  *min_element(fileExistence[i].begin(), fileExistence[i].end()) << " with MD5 "<<hash<<" at depth 1" <<flush<< endl;
        }
        else if(fileExistence1[i].size() > 0){
        	min_id_index=min_element(fileExistence1[i].begin(),fileExistence1[i].end()) - fileExistence1[i].begin();
        	int soc = socket(AF_INET, SOCK_STREAM, 0);
        	struct sockaddr_in neigh_addr; // will hold the destination addr
            	neigh_addr.sin_family = AF_INET; // host byte order
            	neigh_addr.sin_port = htons(file_ports[i][min_id_index]); // network byte order
            	neigh_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
            	memset(&(neigh_addr.sin_zero), '\0', 8);
            	//min_id_index=min_element(fileExistence1[i].begin(),fileExistence1[i].end()) - fileExistence1[i].begin();
                int con = connect(soc, (struct sockaddr *)&neigh_addr, sizeof(struct sockaddr));
        	int sen = send(soc,name.c_str(),strlen(name.c_str()),0);
        	int filesize = 0;
		int rec_size = recv(soc, (char*)&filesize, sizeof(filesize), 0);
		char* filebuf = new char[filesize];
		memset(filebuf,'\0', filesize);
		int rec2 = recv(soc, filebuf, filesize, MSG_WAITALL);
		
        	//char* dat_2 = filebuf;
        	//cout<<dat<<endl;
        	string d="Downloaded/";
        	mkdir((argv2+d).c_str(),0777);
        	ofstream myfile;
        	myfile.open(argv2+d+name, ios::binary);
        	//cout<<argv2+d+name<<endl;
		myfile.write(filebuf,filesize);
        //cout<<argv2+d+name<< " 371\n"<<endl;
        //delete[] filebuf;
		myfile.close();
        
        close(soc);
		string hash= hash_val(argv2+d+name);
		cout  <<"Found " << files_data[i] << " at " <<  *min_element(fileExistence1[i].begin(), fileExistence1[i].end()) << " with MD5 "<<hash<<" at depth 2" <<flush<< endl;
        }
        
        else cout << "Found " << files_data[i] << " at " << 0 << " with MD5 0 at depth 0"<<flush<< endl;
        //cout << endl;
    }
    return;
}

int main(int argc, char* argv[]){
    vector<string> my_files;
    struct dirent *d;
    DIR *dr;
    dr = opendir(argv[2]);
    if(dr!=NULL)
    {
        //cout<<"List of Files and Folders:-\n";
        while((d=readdir(dr))!=NULL){
            string k = "/";
            string name = argv[2] + k + d->d_name;
            DIR *dr1;
            dr1 = opendir(name.c_str());
            if(dr1 == NULL){cout<<d->d_name<<endl;my_files.push_back(d->d_name);}
        }
        closedir(dr);
    }
    else
        cout<<"\nError Occurred!";
    //cout<<endl;
    fstream configfile;
    stringstream list_stream;
    int client_id,port,unique_id,n,socket_server;
    struct sockaddr_in my_addr;
    my_addr.sin_family = AF_INET;
    configfile.open(argv[1],ios::in);

    if(configfile.is_open()){
        string data;
        getline(configfile, data);
        list_stream << data;
        list_stream >> client_id;
        list_stream >> port;
        list_stream >> unique_id;  
        //cout << client_id <<" "<< port << " " << unique_id << "\n";
        //client_id = data[0]-'0'; port = data[2]-'0'; unique_id = data[4]-'0';
        socket_server = socket(AF_INET, SOCK_STREAM, 0);
        //socket_client = socket(AF_INET, SOCK_STREAM, 0);
        //cout << "server " << socket_server << " client " << socket_client << "\n";
        //set up server part of client here and start listenjing to connections and set up client socket...
        my_addr.sin_port = htons(port);
        my_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        memset(&(my_addr.sin_zero),'\0',8);
        int t = bind(socket_server, (struct sockaddr*)&my_addr, sizeof(struct sockaddr));
        if(t == -1 )  {
        	perror("bind");exit(1);//close(socket_server);socket_server = socket(AF_INET, SOCK_STREAM, 0);t = bind(socket_server, (struct sockaddr*)&my_addr, sizeof(struct sockaddr));
        };
        //cout << "bind " << t << "\n";
        //thread th1(listening_for_connections, socket_server, unique_id);
        string data2;
        getline(configfile,data2);
        n = stoi(data2);

        string data4;
        getline(configfile,data4);
        

        int files_num;
        string data5; // 4th line contains num of files to be searched
        getline(configfile,data5);
        stringstream list_stream5;
        list_stream5 << data5;
        list_stream5 >> files_num;
        //files_num = atoi(data5);
        vector<string> files_data;
        for(int i=0;i<files_num;i++){    
            string file_name;  // 5rd line contains info on neigh
            getline(configfile,file_name);
            files_data.push_back(file_name);
        }
        sort(files_data.begin(), files_data.end(),compare2strings);
        configfile.close();
        //cout << "going into threads" << endl;
        thread th2(connecting_to_neighbours,n , data4, unique_id, files_num, files_data, my_files,port,argv[2]);
        thread th1(listening_for_connections, socket_server, unique_id,n, my_files,argv[2]);
        th1.join();
        th2.join();
    }
    
}
