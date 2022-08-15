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

using namespace std;

bool compare2strings(string s1, string s2){
    if(s1.compare(s2) < 0) return 1;
    else return 0;
}

void listening_for_connections(int socket_server,int unique_id, int n, vector<string> my_files){
    //cout << "inside listening" << endl;
    int h=n,max_fd = socket_server+1;
    int l = listen(socket_server,20);
    //while(l!=0) cout << "l value not 0";
    if(l == 0){
        //cout << "Hey, I am listening\n";
        fd_set fd_list;
        fd_set fd_active;
        FD_ZERO(&fd_list); FD_ZERO(&fd_active);
        FD_SET(socket_server,&fd_list);
        while(1){
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
                            int sent = send(a,s.c_str(),strlen(s.c_str()),0);
                            FD_SET(a,&fd_list);
                            max_fd = max(max_fd,a+1);
                            // cout << "data sent " << sent << "\n";
                        }
                    }
                    else{
                        char buffer[256];
                        for(int k=0;k<256;k++) buffer[k] = 0;
                        int res= recv(i,buffer,256,0);
                        //stringstream filenames;
                        //filenames << buffer;
                        string name = string(buffer);
                        //filenames >> name;
                        bool is_present = false;
                        for(int j=0;j<my_files.size();j++){
                            if(name.compare(my_files[j]) == 0) {is_present=1;break;}
                        }
                        string yes = "1", no = "0";
                        if(is_present){int sent = send(i,yes.c_str(),strlen(yes.c_str()),0 );}
                        else{int sent = send(i,no.c_str(),strlen(no.c_str()),0 );}
                    }
                }
                
            }
            
        }
    }
    //else cout << "not listening" << endl;
    return;
}
void connecting_to_neighbours( int n, string data,int unique_id, int files_num, vector<string> files_data){
    //cout << "Inside connecting thread" << endl;
    vector<int> fileExistence[files_num];
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
                //cout << "connection to client_id " << n_id << " with socket " << a  << " on port " << n_port << "\n"; 
                char buffer[256];
                for(int k=0;k<256;k++) buffer[k] = 0;
                connect_list[i] = a;
                int res = recv(socket_client[i], buffer, 256, 0);
                stringstream list_stream1;
                list_stream1 << buffer;
                list_stream1 >> unique_ids[i];
                //while(res == -1) res = recv(a, buffer, 256, 0);
                //cout << "data recv on client side " << res << "\n";
                //cout << "Connected to " << neighbor_id[i]<< " with unique-ID "<< buffer << " on port " << neighbor_port[i] << "\n";
                //cout << buffer << " ";
                //close(socket_client[i]);
            }            
        }
    }
    for(int i=0;i<n;i++){
        cout << "Connected to " << neighbor_id[i]<< " with unique-ID "<< unique_ids[i] << " on port " << neighbor_port[i] << endl;
    }
    //cout << files_num << endl;
    for(int i=0;i<files_num;i++){
        string name=files_data[i];
        for(int j=0;j<n;j++){
            int sent = send(socket_client[j],name.c_str(),strlen(name.c_str()),0 );
            char yes_no[256];
            yes_no[0] = 0;
            yes_no[1] = 0;
            int res = recv(socket_client[j],yes_no, 256,0);
            if(res != -1){if(yes_no[0] == '1') fileExistence[i].push_back(unique_ids[j]);}
            else cout << "rec -1" <<  endl;
        }
    }
    for(int i=0;i<files_num;i++){
        //cout << files_data[i] << ": ";
        /*for(int j=0;j<fileExistence[i].size();j++){
            cout << unique_ids[fileExistence[i][j]] << " ";
        }*/
        if(fileExistence[i].size() > 0){/*for(int k=0;k<fileExistence[i].size();k++) cout << fileExistence[i][k] << " ";*/cout  <<"Found " << files_data[i] << " at " <<  *min_element(fileExistence[i].begin(), fileExistence[i].end()) << " with MD5 0 at depth 1";}
        else cout << "Found " << files_data[i] << " at " <<  0 << " with MD5 0 at depth 0";;
        cout << endl;
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
        if(t == -1 )  {perror("bind");exit(1);//close(socket_server);socket_server = socket(AF_INET, SOCK_STREAM, 0);t = bind(socket_server, (struct sockaddr*)&my_addr, sizeof(struct sockaddr));
        };
        //if(t == -1 )  {close(socket_server);socket_server = socket(AF_INET, SOCK_STREAM, 0);t = bind(socket_server, (struct sockaddr*)&my_addr, sizeof(struct sockaddr));};
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
        thread th2(connecting_to_neighbours,n , data4, unique_id, files_num, files_data);
        thread th1(listening_for_connections, socket_server, unique_id,n, my_files);
        th1.join();
        th2.join();
    }
    
}