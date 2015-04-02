#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
//service
#define INIT 0x00
#define REGIST 0x01
#define LOGON 0x02
#define LOGOFF 0x03
//#define HEARTBEAT 0x04
#define MESSAGE 0x05
#define ONLINE 0x06
#define UPDATE 0x07
#define INFORM 0x08
#define BATTLE 0x09
#define WATCH 0x10

//status
#define REFUSE 0x00
#define ACCEPT 0x01
#define REQUEST 0x03
#define REPLY 0x04
#define REPEAT 0x05
#define WAIT 0x06
#define PREPARE 0x07
#define START 0x08
#define ROUND 0x09
#define END 0x10
#define CHOOSE 0x11

#define SERV_PORT 3000
#define MAXLINE 1024
#define HEADLINE 28
#define LISTENQ 100

struct Package{
    char proname[4];
    short length;
    char service;
    char status;
    char src[10];
    char dst[10];
    char message[996];
};

struct Account{
    char username[10];
    char password[20];
};

struct OnlineAccount{
    char username[10];
    int battling;//正在对战:组号+1;空闲:0
    int connfd;
};

struct Battle{
    int user1[4];//user1[0]connfd,user1[1]blood1,user1[2]time1,user1[3]gesture1
    int user2[4];
    //char username1[2][10];
    //char username2[2][10];
   /* int blood1;//初始化:10
     int blood2;
    int time1;//出拳次数
    int time2;
    int gesture1;
    int gesture2;*/
    //char winnername[10];
    int watch_connfd[10];//观看者的编号
};

struct mythread{
    char username[10];
    int connfd;
    pthread_t handlethread;
    struct Package sendpkg;
    struct Package recvpkg;
    int used;
    int logged;
};

void regist();
void logon();
void listfri();
void chat();
void inform();
void logoff();
void sendheart();
void updatelist();
void receiveMsg();
void init_pkg(struct Package *pkg);
void handleThread(void* l);
void handleRegist(int connfd,int index);
void handleLogon(int connfd,int index);
void handleLogoff(int connfd,int index);
void handleBattle(int connfd,int index);
void handleMessage(int connfd,int index);
void handleInform(int connfd,int index);
void handleOnline(int connfd,int index);
void handleWatch(int connfd,int index);
void broadcast(char *username, int type);
//void handleHeartbeat(int connfd,int index);
void mainThread();
void showlist();
/*
void checkheartbeat();
void sendheartbeat();
void heartBeatThread();
*/
int findValid();
void abnormal_logoff(char *username);
int compare_gesture(int i, int j);
