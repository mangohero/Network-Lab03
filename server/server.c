#include "header.h"
int online,user,couple;
struct Account userList[LISTENQ];
struct OnlineAccount onlineUser[LISTENQ];
struct Battle battle[LISTENQ];
int listenfd, n;
socklen_t clilen;
struct Package sendpkg,recvpkg;
struct sockaddr_in cliaddr, servaddr;
char username[10];
char password[20];
int k;
struct mythread childthread[LISTENQ];
pthread_t heartthread;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char **argv){
    user = 0;
    online = 0;
    couple = 0;
    k = 0;
    
    if((listenfd = socket(AF_INET,SOCK_STREAM,0)) < 0){
        perror("Problem in creating the socket\n");
        exit(1);
    }
    
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);
    
    bind (listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    listen(listenfd, LISTENQ);
    printf("Server running ... Waiting for connections.\n");
    /*
    int ret = pthread_create(&heartthread,NULL,(void*)heartBeatThread,NULL);
    if (ret != 0){
        printf("Create thread error!\r\n");
        exit(1);
    }*/
    
    while(1){
        clilen = sizeof(cliaddr);
        k = findValid();//处理并发,k!=-1,有空线程可用
        childthread[k].connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen);
        printf("Received request ...\n");
        if(k != -1){
            childthread[k].used = 1;
            int ret = pthread_create(&(childthread[k].handlethread),NULL,(void*)handleThread,(void *)k);
            if (ret != 0){
                printf("Create thread error!\r\n");
                exit(1);
            }
            else{
                printf("Succeed to create a thread!\r\n");
            }
        }
        else{
            printf("Sorry, the server is busy!!!\n");
            exit(1);
        }
    }
    //pthread_join(heartthread,NULL);
    return 0;
}

void handleThread(void * k){
    int index = (int)k;
    int connfd = childthread[index].connfd;
    while((n = recv(connfd, (void *)&childthread[index].recvpkg, MAXLINE, 0) > 0)){
        switch(childthread[index].recvpkg.service){
            case(REGIST):	{handleRegist(connfd,index);break;}
            case(LOGON):	{handleLogon(connfd,index);break;}
            case(LOGOFF):	{handleLogoff(connfd,index);break;}
         //   case(HEARTBEAT):{handleHeartbeat(connfd,index);	break;}
            case(BATTLE):	{handleBattle(connfd,index);break;};
            case(MESSAGE):	{handleMessage(connfd,index);break;}
            case(INFORM):	{handleInform(connfd,index);break;}
            case(ONLINE):	{handleOnline(connfd,index);break;}
            case(WATCH):	{handleWatch(connfd,index);break;}
            default:		{printf("Unknown package!!!\n");}
                fflush(stdout);
        }
        memset(&recvpkg, 0 , MAXLINE);
    }
    printf("The thread is dead\n");
    childthread[index].used = 0;
    if(childthread[index].logged){
        char offuser[10];
        strcpy(offuser,childthread[index].username);
        memset(&(childthread[index].username),0,10);
        abnormal_logoff(offuser);
    }
    pthread_exit(NULL);
}

void handleRegist(int connfd,int index){
    strcpy(username,childthread[index].recvpkg.src);
    strcpy(password,childthread[index].recvpkg.message);
    printf("User regist, the username id %s,the password is %s   ",username,password);
    int i;
    for(i = 0 ; i < user ; i++){
        if(strcmp(userList[i].username,username) == 0)
            break;
    }
    if(i == user){//未被注册过
        strcpy(userList[user].username,username);
        strcpy(userList[user].password,password);
        user++;
        memset(&childthread[index].sendpkg, 0 , MAXLINE);
        init_pkg(&childthread[index].sendpkg);
        childthread[index].sendpkg.service = REGIST;
        strcpy(childthread[index].sendpkg.src, username);
        childthread[index].sendpkg.status = ACCEPT;
        send(connfd, (void *)&childthread[index].sendpkg, HEADLINE, 0);
        printf("Regist successfully\n");
    }
    else{//注册过
        memset(&childthread[index].sendpkg, 0 , MAXLINE);
        init_pkg(&childthread[index].sendpkg);
        childthread[index].sendpkg.service = REGIST;
        strcpy(childthread[index].sendpkg.src, username);
        childthread[index].sendpkg.status = REFUSE;
        send(connfd, (void *)&childthread[index].sendpkg, HEADLINE, 0);
        printf("Regist failed\n");
    }
}

void handleLogon(int connfd,int index){
    strcpy(username,childthread[index].recvpkg.src);
    strcpy(password,childthread[index].recvpkg.message);
    printf("User logon, the username id %s,the password is %s   ",username,password);
    int i;
    int success = 0;
    for(i = 0 ; i < online ; i++){
        if(strcmp(onlineUser[i].username,username) == 0){
            memset(&childthread[index].sendpkg, 0 , MAXLINE);
            init_pkg(&childthread[index].sendpkg);
            childthread[index].sendpkg.service = LOGON;
            strcpy(childthread[index].sendpkg.src, username);
            childthread[index].sendpkg.status = REPEAT;
            send(connfd, (void *)&childthread[index].sendpkg, HEADLINE, 0);
            return;
        }
    }
    /*if(i < online){
     memset(&childthread[index].sendpkg, 0 , MAXLINE);
     init_pkg(&childthread[index].sendpkg);
     childthread[index].sendpkg.service = LOGON;
     strcpy(childthread[index].sendpkg.src, username);
     childthread[index].sendpkg.status = REPEAT;
     send(connfd, (void *)&childthread[index].sendpkg, HEADLINE, 0);
     }
     else{*/
    for(i = 0 ; i < user ; i++){
        if(strcmp(userList[i].username,username) == 0){
            if(strcmp(userList[i].password,password) == 0){
                pthread_mutex_lock(&mutex);
                strcpy(onlineUser[online].username,username);
                onlineUser[online].connfd = connfd;
                online++;
                pthread_mutex_unlock(&mutex);
                memset(&childthread[index].sendpkg, 0 , MAXLINE);
                init_pkg(&childthread[index].sendpkg);
                childthread[index].sendpkg.service = LOGON;
                strcpy(childthread[index].sendpkg.src, username);
                strcpy(childthread[index].username,username);
                childthread[index].sendpkg.status = ACCEPT;
                send(connfd, (void *)&childthread[index].sendpkg, HEADLINE, 0);
                success = 1;
                childthread[index].logged = 1;
                broadcast(username,1);
                printf("Log on successfully\n");
                return;
            }
        }
    }
    /*if(i < user){
     if(strcmp(userList[i].password,password) == 0){
     pthread_mutex_lock(&mutex);
     strcpy(onlineUser[online].username,username);
     onlineUser[online].connfd = connfd;
     online++;
     pthread_mutex_unlock(&mutex);
     memset(&childthread[index].sendpkg, 0 , MAXLINE);
     init_pkg(&childthread[index].sendpkg);
     childthread[index].sendpkg.service = LOGON;
     strcpy(childthread[index].sendpkg.src, username);
     strcpy(childthread[index].username,username);
     childthread[index].sendpkg.status = ACCEPT;
     send(connfd, (void *)&childthread[index].sendpkg, HEADLINE, 0);
     success = 1;
     childthread[index].logged = 1;
     broadcast(username,1);
     printf("Log on successfully\n");
     }
     }*/
    if(!success){
        memset(&childthread[index].sendpkg, 0 , MAXLINE);
        init_pkg(&childthread[index].sendpkg);
        childthread[index].sendpkg.service = LOGON;
        strcpy(childthread[index].sendpkg.src, username);
        childthread[index].sendpkg.status = REFUSE;
        send(connfd, (void *)&childthread[index].sendpkg, HEADLINE, 0);
        printf("Log on failed\n");
    }
    //}
}

void handleLogoff(int connfd,int index){
    strcpy(username,childthread[index].recvpkg.src);
    printf("User log off, the username id %s   ",username);
    memset(&(childthread[index].username),0,10);
    int i,j;
    pthread_mutex_lock(&mutex);
    for(i = 0 ; i < online ; i++){
        if(strcmp(userList[i].username, username) == 0)
            break;
    }
    
    for(j = i ; i < online-2 ; j++){
        strcpy(onlineUser[j].username,onlineUser[j+1].username);
        onlineUser[j].connfd = onlineUser[j+1].connfd;
    }
    online--;
    close(childthread[index].connfd);
    childthread[index].logged = 0;
    pthread_mutex_unlock(&mutex);
    broadcast(username,0);
    printf("Log off successfully\n");
}

void abnormal_logoff(char *username){
    printf("User abnormal disconnection, the username id %s   ",username);
    int i,j;
    pthread_mutex_lock(&mutex);
    for(i = 0 ; i < online ; i++){
        if(strcmp(userList[i].username, username) == 0)
            break;
    }
    for(j = i ; i < online-2 ; j++){
        strcpy(onlineUser[j].username,onlineUser[j+1].username);
        onlineUser[j].connfd = onlineUser[j+1].connfd;
    }
    online--;
    pthread_mutex_unlock(&mutex);
    broadcast(username,0);
}

void handleBattle(int connfd,int index){
    int i = 0;
    int m = 0;
    int fd = 0;
    int num = 0;
    int blood1 = 0;
    int blood2 = 0;
    switch(childthread[index].recvpkg.status){
        case(REQUEST):	{
            char dst[10];
            strcpy(username,childthread[index].recvpkg.src);
            strcpy(dst,childthread[index].recvpkg.dst);
            printf("%s want to battle with %s", username,dst);
            memset(&childthread[index].sendpkg, 0 , MAXLINE);
            init_pkg(&childthread[index].sendpkg);
            childthread[index].sendpkg.service = BATTLE;
            for(i = 0 ; i < online ; i++){//查找目标用户的connfd
                if(strcmp(dst,onlineUser[i].username)==0)
                    break;
            }
            if(onlineUser[i].battling != 0){//对方正在对战,通知等待
                childthread[index].sendpkg.status = WAIT;
                strcpy(childthread[index].sendpkg.src, username);
                send(connfd, (void *)&childthread[index].sendpkg, HEADLINE, 0);
                printf("send battle wait successfully\n");
                break;
            }
            else{//向对方发送对战请求
                fd = onlineUser[i].connfd;
                childthread[index].sendpkg.status = REQUEST;
                strcpy(childthread[index].sendpkg.src, username);
                strcpy(childthread[index].sendpkg.dst, dst);
                //pthread_mutex_lock(&mutex);
                /*for(i = 0 ; i < online ; i++){
                 if(strcmp(dst,onlineUser[i].username)==0){
                 fd = onlineUser[i].connfd;*/
                send(fd, (void *)&childthread[index].sendpkg, HEADLINE, 0);
                printf("send battle request successfully\n");
                break;
            }
            //pthread_mutex_unlock(&mutex);
            break;
        }
        case(ACCEPT):{
            char dst[10];
            strcpy(username,childthread[index].recvpkg.src);
            strcpy(dst,childthread[index].recvpkg.dst);
            printf("%s accept to battle with %s", username,dst);
            memset(&childthread[index].sendpkg, 0 , MAXLINE);
            init_pkg(&childthread[index].sendpkg);
            childthread[index].sendpkg.service = BATTLE;
            childthread[index].sendpkg.status = ACCEPT;
            strcpy(childthread[index].sendpkg.src, username);
            strcpy(childthread[index].sendpkg.dst, dst);
            pthread_mutex_lock(&mutex);
            for (m = 0; m < couple; m++) {//取得申请者的fd
                if(strcmp(dst,onlineUser[m].username)==0){
                    fd = onlineUser[m].connfd;
                    break;
                }
            }
            for (i = 0; i < couple; i++) {//前面有空battle位，i等于该空位，否则，i等于couple
                if (battle[i].user1[0] == 0)
                    break;
            }
            battle[i].user1[0] = connfd;
            battle[i].user1[1] = 10;
            battle[i].user2[0] = fd;
            battle[i].user2[1] = 10;
            int j;
            for (j = 0; j < 10; j++)
                battle[i].watch_connfd[j] = 0;
            
            for(j = 0 ; j < online ; j++){
                if(strcmp(username,onlineUser[j].username)==0)
                    onlineUser[j].battling = i;
                }
            onlineUser[m].battling = i;
            send(fd, (void *)&childthread[index].sendpkg, HEADLINE, 0);
            printf("send battle accept successfully\n");
            pthread_mutex_unlock(&mutex);
            break;
        }
        case(REFUSE):{
            char dst[10];
            strcpy(username,childthread[index].recvpkg.src);
            strcpy(dst,childthread[index].recvpkg.dst);
            printf("%s refuse to battle with %s", username,dst);
            memset(&childthread[index].sendpkg, 0 , MAXLINE);
            init_pkg(&childthread[index].sendpkg);
            childthread[index].sendpkg.service = BATTLE;
            childthread[index].sendpkg.status = REFUSE;
            strcpy(childthread[index].sendpkg.src, username);
            strcpy(childthread[index].sendpkg.dst, dst);
            pthread_mutex_lock(&mutex);
            for(i = 0 ; i < online ; i++){
                if(strcmp(dst,onlineUser[i].username)==0){
                    fd = onlineUser[i].connfd;
                    send(fd, (void *)&childthread[index].sendpkg, HEADLINE, 0);
                    printf("send battle refuse successfully\n");
                    break;
                }
            }
            pthread_mutex_unlock(&mutex);
            break;
        }
        case(BATTLE):{
            char dst[10];
            strcpy(username,childthread[index].recvpkg.src);
            strcpy(dst,childthread[index].recvpkg.dst);
            for(i = 0 ; i < online ; i++)
                if(strcmp(username,onlineUser[i].username)==0){
                   // fd = onlineUser[i].connfd;
                    num = onlineUser[i].battling;
                    break;
                }
            if (battle[num].user1[0] == connfd) {//connfd is user1
                battle[num].user1[2]++;//round1++
                battle[num].user1[3] = (int)&childthread[index].recvpkg.message[0];//get gesture
                if (battle[num].user1[2] == battle[num].user2[2]) {//both have gestured, the same round
                    pthread_mutex_lock(&mutex);
                    int result = compare_gesture(battle[num].user1[3],battle[num].user2[3]);
                    battle[num].user1[1] += result;//blood1
                    battle[num].user2[1] -= result;//blood2
                    memset(&childthread[index].sendpkg, 0 , MAXLINE);
                    init_pkg(&childthread[index].sendpkg);
                    strcpy(childthread[index].sendpkg.src, username);
                    strcpy(childthread[index].sendpkg.dst, dst);
                    childthread[index].sendpkg.service = BATTLE;
                    
                    childthread[index].sendpkg.message[0] = battle[num].user1[2];//round
                    
                    childthread[index].sendpkg.message[1] = 1 + result;
                    childthread[index].sendpkg.message[2] = battle[num].user1[3];//user gesture
                    childthread[index].sendpkg.message[3] = battle[num].user1[1];//user blood
                    childthread[index].sendpkg.message[4] = battle[num].user2[3];
                    childthread[index].sendpkg.message[5] = battle[num].user2[1];
                    
                    if (battle[num].user1[1] == 0 || battle[num].user2[1] == 0)
                        childthread[index].sendpkg.status = END;
                    else
                        childthread[index].sendpkg.status = BATTLE;
                    childthread[index].sendpkg.length = strlen(childthread[index].sendpkg.message);
                    send(connfd, (void *)&childthread[index].sendpkg, HEADLINE+childthread[index].sendpkg.length, 0);
                    
                    memset(&childthread[index].sendpkg.src, 0 , 10);
                    memset(&childthread[index].sendpkg.dst, 0 , 10);
                    strcpy(childthread[index].sendpkg.src, dst);
                    strcpy(childthread[index].sendpkg.dst, username);
                    childthread[index].sendpkg.message[1] = 1 - result;
                    childthread[index].sendpkg.message[2] = battle[num].user2[3];//user gesture
                    childthread[index].sendpkg.message[3] = battle[num].user2[1];//user blood
                    childthread[index].sendpkg.message[4] = battle[num].user1[3];
                    childthread[index].sendpkg.message[5] = battle[num].user1[1];
                    send(fd, (void *)&childthread[index].sendpkg, HEADLINE+childthread[index].sendpkg.length, 0);
                    
                    childthread[index].sendpkg.service = WATCH;
                    if (battle[num].user1[1] == 0 || battle[num].user2[1] == 0)
                        childthread[index].sendpkg.status = END;
                    else
                        childthread[index].sendpkg.status = BATTLE;
                    for(i = 0; i < 10; i++)
                        if (battle[num].watch_connfd[i] != 0) {
                            send(battle[num].watch_connfd[i], (void *)&childthread[index].sendpkg, HEADLINE+childthread[index].sendpkg.length, 0);
                        }
                    pthread_mutex_unlock(&mutex);
                }
            }
            else if(battle[num].user2[0] == connfd) {
                battle[num].user2[2]++;//round1++
                battle[num].user2[3] = (int)&childthread[index].recvpkg.message[0];//get gesture
                if (battle[num].user1[2] == battle[num].user2[2]) {//both have gestured, the same round
                    pthread_mutex_lock(&mutex);
                    int result = compare_gesture(battle[num].user2[3],battle[num].user1[3]);
                    battle[num].user2[1] += result;//blood1
                    battle[num].user1[1] -= result;//blood2
                    memset(&childthread[index].sendpkg, 0 , MAXLINE);
                    init_pkg(&childthread[index].sendpkg);
                    strcpy(childthread[index].sendpkg.src, username);
                    strcpy(childthread[index].sendpkg.dst, dst);
                    childthread[index].sendpkg.service = BATTLE;
                    
                    childthread[index].sendpkg.message[0] = battle[num].user2[2];//round
                    
                    childthread[index].sendpkg.message[1] = 1 + result;
                    childthread[index].sendpkg.message[2] = battle[num].user2[3];//user gesture
                    childthread[index].sendpkg.message[3] = battle[num].user2[1];//user blood
                    childthread[index].sendpkg.message[4] = battle[num].user1[3];
                    childthread[index].sendpkg.message[5] = battle[num].user1[1];
                    
                    if (battle[num].user1[1] == 0 || battle[num].user2[1] == 0)
                        childthread[index].sendpkg.status = END;
                    else
                        childthread[index].sendpkg.status = BATTLE;
                    childthread[index].sendpkg.length = strlen(childthread[index].sendpkg.message);
                    send(connfd, (void *)&childthread[index].sendpkg, HEADLINE+childthread[index].sendpkg.length, 0);
                    
                    memset(&childthread[index].sendpkg.src, 0 , 10);
                    memset(&childthread[index].sendpkg.dst, 0 , 10);
                    strcpy(childthread[index].sendpkg.src, dst);
                    strcpy(childthread[index].sendpkg.dst, username);
                    childthread[index].sendpkg.message[1] = 1 - result;
                    childthread[index].sendpkg.message[2] = battle[num].user1[3];//user gesture
                    childthread[index].sendpkg.message[3] = battle[num].user1[1];//user blood
                    childthread[index].sendpkg.message[4] = battle[num].user2[3];
                    childthread[index].sendpkg.message[5] = battle[num].user2[1];
                    send(fd, (void *)&childthread[index].sendpkg, HEADLINE+childthread[index].sendpkg.length, 0);
                    childthread[index].sendpkg.service = WATCH;
                    for(i = 0; i < 10; i++)
                        if (battle[num].watch_connfd[i] != 0) {
                            send(battle[num].watch_connfd[i], (void *)&childthread[index].sendpkg, HEADLINE+childthread[index].sendpkg.length, 0);
                        }
                    pthread_mutex_unlock(&mutex);
                }
            }
            memset(battle[num].watch_connfd, 0, 10);
            battle[num].user1[0] = 0;
            battle[num].user1[2] = 0;
            battle[num].user1[3] = 0;
            battle[num].user2[0] = 0;
            battle[num].user2[2] = 0;
            battle[num].user2[3] = 0;
            break;
        }
        case(END):{
            char dst[10];
            strcpy(username,childthread[index].recvpkg.src);
            strcpy(dst,childthread[index].recvpkg.dst);
            printf("%s quit the battle with %s\n", username,dst);
            memset(&childthread[index].sendpkg, 0 , MAXLINE);
            init_pkg(&childthread[index].sendpkg);
            childthread[index].sendpkg.service = BATTLE;
            childthread[index].sendpkg.status = END;
            strcpy(childthread[index].sendpkg.src, dst);
            strcpy(childthread[index].sendpkg.dst, username);
            for(i = 0 ; i < online ; i++){
                if(strcmp(dst,onlineUser[i].username)==0){
                    fd = onlineUser[i].connfd;
                    num = onlineUser[i].battling;
                    onlineUser[i].battling = 0;//对方恢复空闲状态
                    }
                if (strcmp(username, onlineUser[i].username)==0)
                    onlineUser[i].battling = 0;//恢复空闲状态
            }
            
            //告诉对方游戏结束，对方赢
            childthread[index].sendpkg.message[0] = battle[num].user1[2];//round
            if (battle[num].user1[0] == connfd) {
                childthread[index].sendpkg.message[1] = 0;
                childthread[index].sendpkg.message[2] = battle[num].user1[3];//user gesture
                childthread[index].sendpkg.message[3] = battle[num].user1[1];//user blood
                childthread[index].sendpkg.message[4] = battle[num].user2[3];
                childthread[index].sendpkg.message[5] = battle[num].user2[1];
                childthread[index].sendpkg.length = strlen(childthread[index].sendpkg.message);
                pthread_mutex_lock(&mutex);
                send(connfd, (void *)&childthread[index].sendpkg, HEADLINE+childthread[index].sendpkg.length, 0);
                pthread_mutex_unlock(&mutex);
                childthread[index].sendpkg.message[1] = 2;
                childthread[index].sendpkg.message[2] = battle[num].user2[3];//user gesture
                childthread[index].sendpkg.message[3] = battle[num].user2[1];//user blood
                childthread[index].sendpkg.message[4] = battle[num].user1[3];
                childthread[index].sendpkg.message[5] = battle[num].user1[1];
                strcpy(childthread[index].sendpkg.src, username);
                strcpy(childthread[index].sendpkg.dst, dst);
                pthread_mutex_lock(&mutex);
                send(fd, (void *)&childthread[index].sendpkg, HEADLINE+childthread[index].sendpkg.length, 0);
                pthread_mutex_unlock(&mutex);
            }
            else if (battle[num].user2[0] == connfd) {
                childthread[index].sendpkg.message[1] = 0;
                childthread[index].sendpkg.message[2] = battle[num].user2[3];//user gesture
                childthread[index].sendpkg.message[3] = battle[num].user2[1];//user blood
                childthread[index].sendpkg.message[4] = battle[num].user1[3];
                childthread[index].sendpkg.message[5] = battle[num].user1[1];
                childthread[index].sendpkg.length = strlen(childthread[index].sendpkg.message);
                pthread_mutex_lock(&mutex);
                send(connfd, (void *)&childthread[index].sendpkg, HEADLINE+childthread[index].sendpkg.length, 0);
                pthread_mutex_unlock(&mutex);
                childthread[index].sendpkg.message[1] = 2;
                childthread[index].sendpkg.message[2] = battle[num].user1[3];//user gesture
                childthread[index].sendpkg.message[3] = battle[num].user1[1];//user blood
                childthread[index].sendpkg.message[4] = battle[num].user2[3];
                childthread[index].sendpkg.message[5] = battle[num].user2[1];
                strcpy(childthread[index].sendpkg.src, username);
                strcpy(childthread[index].sendpkg.dst, dst);
                pthread_mutex_lock(&mutex);
                send(fd, (void *)&childthread[index].sendpkg, HEADLINE+childthread[index].sendpkg.length, 0);
                pthread_mutex_unlock(&mutex);
            }
            childthread[index].sendpkg.service = WATCH;
            strcpy(childthread[index].sendpkg.src, dst);
            strcpy(childthread[index].sendpkg.dst, username);
            pthread_mutex_lock(&mutex);
            for(i = 0; i < 10; i++)
                if (battle[num].watch_connfd[i] != 0) {
                    send(battle[num].watch_connfd[i], (void *)&childthread[index].sendpkg, HEADLINE+childthread[index].sendpkg.length, 0);
                }
            pthread_mutex_unlock(&mutex);
            //清空本次battle纪录
            memset(battle[num].watch_connfd, 0, 10);
            battle[num].user1[0] = 0;
            battle[num].user1[2] = 0;
            battle[num].user1[3] = 0;
            battle[num].user2[0] = 0;
            battle[num].user2[2] = 0;
            battle[num].user2[3] = 0;
            break;
        }
        default:break;
        }
    }

void handleMessage(int connfd,int index){
    char dst[10];
    strcpy(username,childthread[index].recvpkg.src);
    strcpy(dst,childthread[index].recvpkg.dst);
    printf("%s send message to %s : %s   ",username,dst,childthread[index].recvpkg.message);
    memset(&childthread[index].sendpkg, 0 , MAXLINE);
    init_pkg(&childthread[index].sendpkg);
    childthread[index].sendpkg.service = MESSAGE;
    strcpy(childthread[index].sendpkg.src, username);
    strcpy(childthread[index].sendpkg.dst, dst);
    strcpy(childthread[index].sendpkg.message, childthread[index].recvpkg.message);
    childthread[index].sendpkg.length = strlen(childthread[index].sendpkg.message);
    int fd,i;
    pthread_mutex_lock(&mutex);
    for(i = 0 ; i < online ; i++){
        if(strcmp(dst,onlineUser[i].username)==0){
            fd = onlineUser[i].connfd;
            send(fd, (void *)&childthread[index].sendpkg, childthread[index].sendpkg.length + HEADLINE, 0);
            printf("send successfully\n");
            break;
        }
    }
    pthread_mutex_unlock(&mutex);
}

void handleInform(int connfd,int index){
    int i;
    memset(&sendpkg, 0 , MAXLINE);
    init_pkg(&sendpkg);
    sendpkg.service = INFORM;
    strcpy(sendpkg.src, childthread[index].recvpkg.src);
    strcpy(sendpkg.message, childthread[index].recvpkg.message);
    sendpkg.length = childthread[index].recvpkg.length;
    pthread_mutex_lock(&mutex);
    for(i = 0 ; i < online ; i++){
        if(strcmp(onlineUser[i].username,childthread[index].recvpkg.src)!=0){
            send(onlineUser[i].connfd, (void *)&sendpkg, sendpkg.length + HEADLINE, 0);
        }
    }
    pthread_mutex_unlock(&mutex);
}


void handleOnline(int connfd,int index){
    strcpy(username,childthread[index].recvpkg.src);
    printf("%s want to know online friends    ",username);
    memset(&childthread[index].sendpkg, 0 , MAXLINE);
    init_pkg(&childthread[index].sendpkg);
    childthread[index].sendpkg.service = ONLINE;
    strcpy(childthread[index].sendpkg.src, username);
    childthread[index].sendpkg.status = 0x00 + online;
    int i;
    pthread_mutex_lock(&mutex);
    for(i = 0 ; i < online ; i++){
        strcpy(&childthread[index].sendpkg.message[10*i],onlineUser[i].username);
    }
    send(connfd, (void *)&childthread[index].sendpkg, HEADLINE + 10*online , 0);
    pthread_mutex_unlock(&mutex);	
    printf("success \n");
}

void handleWatch(int connfd, int index){
    int i = 0;
    switch (childthread[index].recvpkg.status) {
        case REQUEST:
            strcpy(username,childthread[index].recvpkg.src);
            printf("%s want to battle ",username);
            memset(&childthread[index].sendpkg, 0 , MAXLINE);
            init_pkg(&childthread[index].sendpkg);
            childthread[index].sendpkg.service = WATCH;
            childthread[index].sendpkg.status = CHOOSE;
            strcpy(childthread[index].sendpkg.src, username);
            int number,j;
            
            for (i = 0; i < couple; i++) {
                if (battle[i].user1[0] != 0){
                    for (j = 0; j < online; j++) {
                        if(onlineUser[j].battling == i){
                            if(number == 0)
                                strcpy(&childthread[index].sendpkg.message[1],onlineUser[j].username);
                            else
                                strcpy(&childthread[index].sendpkg.message[20*number],onlineUser[j].username);
                            break;
                        }
                    }
                    for (j = 0; j < online; j++) {
                        if(onlineUser[j].battling == i){
                            strcpy(&childthread[index].sendpkg.message[20*number+10],onlineUser[j].username);
                            break;
                        }
                    }
                    number++;
                }
            }
            childthread[index].sendpkg.message[0] = number;
            childthread[index].sendpkg.length = strlen(childthread[index].sendpkg.message);
            send(connfd, (void *)&childthread[index].sendpkg, HEADLINE + childthread[index].sendpkg.length, 0);
            break;
        case CHOOSE:
            strcpy(username,childthread[index].recvpkg.src);
            printf("%s want to watch battle ",username);
            char name[10];
            strcpy(name, &childthread[index].sendpkg.message[0]);
            printf("%s",name);
            int battlenum = 0;
            for (i = 0; i < online; i++) {
                if(strcmp(name, onlineUser[i].username) == 0){//find his battling number
                    battlenum = onlineUser[i].battling;
                        for (j = 0; j < 10; j++) {//add the audience to the battle's watching group
                            if (battle[battlenum].watch_connfd[j] == 0) {
                                battle[battlenum].watch_connfd[j] = connfd;
                                break;
                            }
                        }
                    break;
                }
            }
            break;
        default:
            break;
    }
}
/*
void handleHeartbeat(int connfd,int index){
    printf("Receive heartbeat from %s \n",childthread[index].recvpkg.src);
    if(childthread[index].recvpkg.status == REPLY){
        pthread_mutex_lock(&mutex);
        strcpy(onlineUser[online].username,childthread[index].recvpkg.src);
        onlineUser[online].connfd = connfd;
        online++;
        printf("online %d\n",online);
        pthread_mutex_unlock(&mutex);
    }
}*/

void broadcast(char *username, int type){		//通知所有在线用户，某用户上线或下线，上线1，下线0
    int i;
    memset(&sendpkg, 0 , MAXLINE);
    init_pkg(&sendpkg);
    sendpkg.service = UPDATE;
    if(type)	sendpkg.status = LOGON;
    else sendpkg.status = LOGOFF;
    strcpy(sendpkg.src, username);
    pthread_mutex_lock(&mutex);
    for(i = 0 ; i < online ; i++){
        if(strcmp(onlineUser[i].username,username)!=0){
            send(onlineUser[i].connfd, (void *)&sendpkg, HEADLINE, 0);
        }
    }
    pthread_mutex_unlock(&mutex);
}
/*
void heartBeatThread(){
    signal(SIGALRM,sendheartbeat);
    alarm(30);
}

void sendheartbeat(){
    int i;
    pthread_mutex_lock(&mutex);
    //online = 0;
    for(i = online-1 ; i >= 0 ; i--){
        printf("Send heartbeat to %s \n",onlineUser[i].username);
        strcpy(sendpkg.src, onlineUser[i].username);
        sendpkg.service = HEARTBEAT;
        sendpkg.status = REQUEST;
        send(onlineUser[i].connfd, (void *)&sendpkg, HEADLINE, 0);
        online -- ;
    }
    pthread_mutex_unlock(&mutex);
    //sleep(5);
    checkheartbeat();
    alarm(30);
}

void checkheartbeat(){
}
*/
void init_pkg(struct Package *pkg){
    strcpy(pkg->proname, "ZRQP");
    pkg->length = 0;
    pkg->service = INIT;
}

int findValid(){
    int i;
    for( i = 0 ; i < LISTENQ ; i++){
        if(childthread[i].used != 1)
            return i;
    }
    return -1;
}

int compare_gesture(int i, int j){
    switch (i-j) {
        case 0:return 0;break;
        case 1:case -2:return -1;break;
        case -1:case 2:return 1;break;
        default:return 0;break;
    }
            }
