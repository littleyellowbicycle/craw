#include<arpa/inet.h>
#include<netdb.h>
#include<iostream>
#include<stdio.h>
using namespace std;
int main(int argc,char**argv){
    //const char *hostname="baidu.com";
    const char* hostname;
    const char* defname="baidu.com";
    if(argc=2){
        hostname=argv[1];
    }else{
        hostname=defname;
    }
    cout<<"input name is:"<<hostname<<endl;
    struct hostent*hct=gethostbyname(hostname);
    if(!hct){
        perror("hostname is error");
    }
    cout<<"official name:"<<endl;
    cout<<hct->h_name<<endl;
    cout<<"alias:"<<endl;
    char**aliasp=hct->h_aliases;
    while(*aliasp){
        cout<<*aliasp++<<endl;
    }
    cout<<"addr :"<<endl;
    struct in_addr** addrp=(struct in_addr**)hct->h_addr_list;
    while(*addrp){
       cout<< inet_ntoa(**addrp)<<endl;
       addrp++;
    }


    return 0;
}