#include "head.h"

int sockfd;
struct sockaddr_in servaddr;
struct Package sendpkg,recvpkg;
char username[10];
char password[20];
int validuser;
char func[10];
int loged;
int onlineUser;
char userBattleyou[10];
char userList[100][10];
char usertochat[10];
char usertoBattle[10];
char SERV_IP[15] = "127.0.0.1";
//全局变量，标志有没有人向自己发动对战
int battle = 0;

pthread_mutex_t sendmutex = PTHREAD_MUTEX_INITIALIZER;
//观看对战
void watch(){
	//memset(&number,0,strlen(number));
	//scanf("%s",number);
	memset(&sendpkg, 0 , MAXLINE);
	init_pkg(&sendpkg);
	sendpkg.service = WATCH;
	sendpkg.status = REQUEST;
	strcpy(sendpkg.src, username);
	send(sockfd, (void *)&sendpkg,HEADLINE, 0);
	if(recv(sockfd, (void *)&recvpkg, MAXLINE, 0) == 0){	//receive服务器返回的数据	
		perror("Receive error\n");
		exit(1);
	}
	//拒绝观看
	if(recvpkg.status == REFUSE)
		printf("The person you want to battle with refuse your request!\n");
	//接收观看
	else if(recvpkg.status == ACCEPT){
		//先列出所有的人在对战状态///////////////////////////////
		//listfri();
		printf("Please choose the number you want to watch:");
		char number[10];
		memset(&number,0,strlen(number));
		scanf("%s",number);
		memset(&sendpkg, 0 , MAXLINE);
		init_pkg(&sendpkg);
		sendpkg.service = WATCH;
		sendpkg.status = CHOOSE;
		strcpy(sendpkg.src, username);
		strcpy(sendpkg.message, number);
		sendpkg.length = strlen(number);
		send(sockfd, (void *)&sendpkg,sendpkg.length + HEADLINE, 0);
		//从服务器端接收的信息
		if(recv(sockfd, (void *)&recvpkg, MAXLINE, 0) == 0){	//receive服务器返回的数据	
			perror("Receive error\n");
			exit(1);
		}
		//解析从服务器接收的报文信息，即对战的即时消息////////////////////////////////////////
	}	
	//从服务器端接收的信息
	if(recv(sockfd, (void *)&recvpkg, MAXLINE, 0) == 0){	//receive服务器返回的数据	
		perror("Receive error\n");
		exit(1);
	}
}
//请求对战
void battle(){
	//先列出所有的人在对战状态
	listfri();
	printf("Please choose someone to battle with:");
	memset(&usertoBattle,0,strlen(usertoBattle));
	scanf("%s",usertoBattle);
	memset(&sendpkg, 0 , MAXLINE);
	init_pkg(&sendpkg);
	sendpkg.service = BATTLE;
	sendpkg.status = PREPARE;
	strcpy(sendpkg.src, username);
	strcpy(sendpkg.dst, usertoBattle); 
	send(sockfd, (void *)&sendpkg,HEADLINE, 0);	
	//从服务器端接收的信息
	if(recv(sockfd, (void *)&recvpkg, MAXLINE, 0) == 0){	//receive服务器返回的数据	
		perror("Receive error\n");
		exit(1);
	}
	//拒绝对战
	if(recvpkg.status == REFUSE)
		printf("The person you want to battle with refuse your request!\n");
	//接收对战
	else if(recvpkg.status == ACCEPT){
		printf("start battle!");
		printf("Round 1:Please input your gesture:\n\t1.stone;\n\t2.scissor\n\t3.paper\n");
		int round = 2;
		char some[10];
		scanf("%s",some);
		int sign = 0;
		while(sign ==0 ){
			if(strcmp(some,"0")!= 0 && strcmp(some,"1")!= 0 && strcmp(some,"2")!= 0)
				printf("Input error,please try again\n");
			else
				sign = 1;
		}
		memset(&sendpkg, 0 , MAXLINE);
		init_pkg(&sendpkg);
		sendpkg.service = BATTLE;
		sendpkg.status = BATTLE;
		strcpy(sendpkg.src, username);
		strcpy(sendpkg.dst, usertoBattle); 
		strcpy(sendpkg.message, some); 
		sendpkg.length = strlen(some);
		send(sockfd, (void *)&sendpkg, sendpkg.length + HEADLINE, 0);	//将用户名和密码信息send到服务器
		memset(&recvpkg, 0 , MAXLINE);
		if(recv(sockfd, (void *)&recvpkg, MAXLINE, 0) == 0){	//receive服务器返回的数据	
			perror("Receive error\n");
			exit(1);
		}
		while(recvpkg.status != END){
			char winner[10];
			int i;
			for(i = 0;i<9;i++ ){
				winner[i] = recvpkg.message[i+1];
			}
			char status1[10];
			switch(recvpkg.message[10]){
				case 0:strcpy(status1,"stone");break;
				case 1:strcpy(status1,"scissor");break;
				case 2:strcpy(status1,"paper");break;
			}
			char status2[10];
			switch(recvpkg.message[12]){
				case 0:strcpy(status2,"stone");break;
				case 1:strcpy(status2,"scissor");break;
				case 2:strcpy(status2,"paper");break;
			}
			printf("Round %d	Winner:%s\n",round,winner);
			printf("%s:%s   Blood:%d\n",username,status1,recvpkg.message[11]);
			printf("%s:%s   Blood:%d\n",usertoBattle,status2,recvpkg.message[13]);

			printf("Round %d:Please input your gesture:\n\t1.stone;\n\t2.scissor\n\t3.paper\n4.exit",round);
			round++;
			char some1[10];
			scanf("%s",some1);
			int sign = 0;
			while(sign ==0 ){
				//按4之后退出对战
				if(strcmp(some1,"4") == 0){
					//首先发送退出的消息给服务器
					memset(&sendpkg, 0 , MAXLINE);
					init_pkg(&sendpkg);
					sendpkg.service = BATTLE;
					sendpkg.status = END;  ////
					strcpy(sendpkg.src, username);
					strcpy(sendpkg.dst, usertoBattle); 
					send(sockfd, (void *)&sendpkg, HEADLINE, 0);	//将用户名和密码信息send到服务器s
					mainThread();
				}
				else if(strcmp(some1,"1")!= 0 && strcmp(some1,"2")!= 0 && strcmp(some1,"3")!= 0)
					printf("Input error,please try again\n");
				else
					sign = 1;
			}
			memset(&sendpkg, 0 , MAXLINE);
			init_pkg(&sendpkg);
			sendpkg.service = BATTLE;
			sendpkg.status = BATTLE;
			strcpy(sendpkg.src, username);
			strcpy(sendpkg.dst, usertoBattle); 
			strcpy(sendpkg.message, some1); 
			sendpkg.length = strlen(some1);
			send(sockfd, (void *)&sendpkg, sendpkg.length + HEADLINE, 0);	//将用户名和密码信息send到服务器
			memset(&recvpkg, 0 , MAXLINE);
			if(recv(sockfd, (void *)&recvpkg, MAXLINE, 0) == 0){	//receive服务器返回的数据	
				perror("Receive error\n");
				exit(1);
			}
		}
	}
	//对方正在对战，忙碌状态
	else if(recvpkg.status == WAIT){
		printf("The person you want to battle with is busy,\nplease try later or choose another person to battle with\n");
	}
	else{
		printf("Error Package !!!\n");
	}
	memset(&recvpkg, 0 , MAXLINE);
}

void mainThread(){
	while(battle == 1){
		battle = 0;//首先将battle的状态改变，以便于下一次的对战请求
		printf("%s want to battle with you,if agree(please enter y),or disagree(please enter n)",userBattleyou);
		char enter[10];
		scanf("%s",enter);
		int sign = 0;
		while(sign = 0){
			if(strcmp(enter,"y") == 0){
				sign = 1;
				//发送接收对战请求报文
				memset(&sendpkg, 0 , MAXLINE);
				init_pkg(&sendpkg);
				sendpkg.service = BATTLE;
				sendpkg.status = ACCEPT;
				strcpy(sendpkg.src, username);
				strcpy(sendpkg.dst, userBattleyou); 
				//sendpkg.length = strlen(password);
				send(sockfd, (void *)&sendpkg,HEADLINE, 0);	
			}
			else if(strcmp(enter,"y") == 0){
				sign = 1;
				//发送拒绝对战请求报文
				memset(&sendpkg, 0 , MAXLINE);
				init_pkg(&sendpkg);
				sendpkg.service = BATTLE;
				sendpkg.status = REFUSE;
				strcpy(sendpkg.src, username);
				strcpy(sendpkg.dst, userBattleyou); 
				//sendpkg.length = strlen(password);
				send(sockfd, (void *)&sendpkg,HEADLINE, 0);	//将用户名和密码信息send到服务器
			}
			else 
				printf("Input error,please try again!");
		}
	}
	printf("Please choose the function:\n");
	printf("1--List online friends; \n");
	printf("2--Send messages to one online friend; \n");
	printf("3--Send message to all people online:  \n");
	printf("4--Start a Battle");
	printf("5--Watch Battle");
	printf("6--Log off and exit): \n");
	int sign =0;
	scanf("%s", func);
	while(sign == 0){
		if(strcmp(func,"1") == 0){
			sign = 1;
			listfri();
		}
		else if(strcmp(func,"2") == 0){
			sign = 1;
			chat();
		}
		else if(strcmp(func,"3") == 0){
			sign = 1;
			inform();
		}
		else if(strcmp(func,"4") == 0){
			sign = 1;
			battle();
		}
		else if(strcmp(func,"5") == 0){
			sign = 1;
			watch();
		}
		else if(strcmp(func,"6") == 0){
			sign = 1;
			logoff();
		}
		else 
			printf("Input error!");
	}
}

//接收信息的线程
void receiveMsg(){
	while(1){
		if(recv(sockfd, (void *)&recvpkg, MAXLINE, 0) == 0){	//
			perror("Receive error\n");
			exit(1);
		}
		switch (recvpkg.service){
			case(HEARTBEAT):	{sendheart();	break;}
			case(MESSAGE):	case(INFORM):		{printf("%s : %s\n", recvpkg.src, recvpkg.message);	break;}
			case(UPDATE):	{updatelist();	break;}
			case(ONLINE):	{showlist(); break;}
			case(BATTLE):	{battle = 1;  strcpy(userBattleyou,recvpkg.src);}
			default:{printf("Unknown package");}
		}
		memset(&recvpkg, 0 , MAXLINE);
	}
}

void showlist(){
	onlineUser = recvpkg.status - 0x00;
	int i;
	for(i = 0 ; i < onlineUser ; i++){
		sscanf(&recvpkg.message[10*i],"%s",userList[i]); 
	}
	printf("There are %d friends online, their username follows:\n", onlineUser);
	for(i = 0 ; i < onlineUser ; i++){
		//???????????????????????????????
		if(recvpkg.message[i*10]== 1){//在对战中
			printf("%d.%s     On battle\n",i+1,userList[i]);
		}
		else{//不在对战中
			printf("%d.%s     Not on battle\n",i+1,userList[i]);
		}
	}
	printf("\n");
}


void init_pkg(struct Package *pkg){
	strcpy(pkg->proname, "ZRQP");
	pkg->length = 0;
	pkg->service = INIT;
}

//用户注册
void regist(){
	system("clear");
	printf("Please enter your username and password you want to regist.\n");
	while(!validuser){
		printf("username(less than 10 char):");
		scanf("%s", username);
		printf("password(less than 20 char):");
		scanf("%s", password);


		//pthread_mutex_lock (&sendmutex);  //
		memset(&sendpkg, 0 , MAXLINE);
		init_pkg(&sendpkg);
		sendpkg.service = REGIST;
		strcpy(sendpkg.src, username);
		strcpy(sendpkg.message, password); 
		sendpkg.length = strlen(password);
		send(sockfd, (void *)&sendpkg, sendpkg.length + HEADLINE, 0);	//将用户名和密码信息send到服务器
		//pthread_mutex_unlock (&sendmutex);  
 
		if(recv(sockfd, (void *)&recvpkg, MAXLINE, 0) == 0){	//receive服务器返回的数据	
			perror("Receive error\n");
			exit(1);
		}
		if(recvpkg.status == REFUSE)
			printf("The username you entered has been registed, please try another one.\n");
		else if(recvpkg.status == ACCEPT){
			validuser = 1;
			printf("Register successfully !!!\n");
		}
		else{
			printf("Error Package !!!\n");
		}
		memset(&recvpkg, 0 , MAXLINE);
	}
}


void logon(){
	system("clear");
	printf("Please enter your username and password to login on.\n");
	while(!validuser){
		printf("username(less than 10 char):");
		scanf("%s", username);
		printf("password(less than 20 char):");
		scanf("%s", password);

		//pthread_mutex_lock (&sendmutex);  
		memset(&sendpkg, 0 , MAXLINE);
		init_pkg(&sendpkg);
		sendpkg.service = LOGON;
		strcpy(sendpkg.src, username);
		strcpy(sendpkg.message, password); 
		sendpkg.length = strlen(password);
		send(sockfd, (void *)&sendpkg, sendpkg.length + HEADLINE, 0);	//将用户名和密码信息send到服务器
		//pthread_mutex_unlock (&sendmutex);  
		
		if(recv(sockfd, (void *)&recvpkg, MAXLINE, 0) == 0){	//receive服务器返回的数据	
			perror("Receive error\n");
			exit(1);
		}
		if(recvpkg.service == LOGON && recvpkg.status == REFUSE)
			printf("Username or password is wrong, please try again.\n");
		else if(recvpkg.service == LOGON && recvpkg.status == ACCEPT){ 
			validuser = 1;
			printf("Login on successfully !!!\n");
			loged = 1;
		}
		else if(recvpkg.service == LOGON && recvpkg.status == REPEAT){
			printf("The user has loged on, please wait a minute or try another username\n");
		}
		else{
			printf("Error Package !!!\n");
		}
		memset(&recvpkg, 0 , MAXLINE);
	}
}
//列出所有在线用户
void listfri(){
	pthread_mutex_lock (&sendmutex);
	memset(&sendpkg, 0 , MAXLINE);
	init_pkg(&sendpkg);  
	sendpkg.service = ONLINE;
	strcpy(sendpkg.src, username);
	sendpkg.status = REQUEST;
	send(sockfd, (void *)&sendpkg, HEADLINE, 0);	//向服务器请求在线好友列表
	pthread_mutex_unlock (&sendmutex);  

}

void chat(){
	printf("Please choose a friend to chat with(Input the username):\n");
	scanf("%s",usertochat);
	int i;
	for(i = 0 ; i < onlineUser ; i++){
		if(strcmp(usertochat, userList[i]) == 0)
			break;
	}
	if( i == onlineUser)
		printf("The friend you entered is offline, you can't send message to him(her)\n");
	else{
		char message[900];
		printf("Please enter the message you want to sent to him(her):\n");
		scanf("%s", message);
		pthread_mutex_lock (&sendmutex);  
		memset(&sendpkg, 0 , MAXLINE);
		init_pkg(&sendpkg);
		strcpy(sendpkg.message , message);
		sendpkg.service = MESSAGE;
		strcpy(sendpkg.src, username);
		strcpy(sendpkg.dst, usertochat);
		sendpkg.length = strlen(message);
		send(sockfd, (void *)&sendpkg, sendpkg.length + HEADLINE, 0);	//发送消息至服务器
		pthread_mutex_unlock (&sendmutex);  
		printf("sent successfully\n");
		
	}
}
//发送信息给所有在线用户
void inform(){
	char message[900];
	printf("Please enter the inform you want to sent:\n");
	scanf("%s", message);
 
	pthread_mutex_lock (&sendmutex);  
	memset(&sendpkg, 0 , MAXLINE);
	init_pkg(&sendpkg);
	strcpy(sendpkg.message , message);
	sendpkg.service = INFORM;
	strcpy(sendpkg.src, username);
	sendpkg.length = strlen(message);
	send(sockfd, (void *)&sendpkg, sendpkg.length + HEADLINE, 0);	//发送消息至服务器
	pthread_mutex_unlock (&sendmutex);  
	printf("sent inform successfully\n");
}
//下线
void logoff(){
	pthread_mutex_lock (&sendmutex);  
	memset(&sendpkg, 0 , MAXLINE);
	init_pkg(&sendpkg);
	sendpkg.service = LOGOFF;
	strcpy(sendpkg.src, username);
	send(sockfd, (void *)&sendpkg, HEADLINE, 0);	//向服务器通知下线
	pthread_mutex_unlock (&sendmutex);  
	loged = 0;
	system("clear");
	printf("Log off successfully!!!\n");
	//close(sockfd);
	exit(0);
}

void sendheart(){
	printf("HeartBeat\n");
	pthread_mutex_lock (&sendmutex);  
	memset(&sendpkg, 0 , MAXLINE);
	init_pkg(&sendpkg);
	sendpkg.service = HEARTBEAT;
	sendpkg.status = REPLY;
	strcpy(sendpkg.src, username);
	send(sockfd, (void *)&sendpkg, HEADLINE, 0);	//向服务器发送心跳响应包
	pthread_mutex_unlock (&sendmutex);  
}

void updatelist(){
	if(recvpkg.status == LOGON){
		strcpy(userList[onlineUser],recvpkg.src);
		printf("User %s is online:\n", userList[onlineUser]);
		onlineUser++;
		printf("\n");
	}
	else if(recvpkg.status == LOGOFF){
		char temp[10];
		strcpy(temp,recvpkg.src);
		printf("User %s is offline:\n", temp);
		int i ;
		for(i = 0 ; i < onlineUser ; i++){
			if(strcmp(userList[i], temp) == 0)
				break;
		}
		int j;
		for(j = i ; i < onlineUser-2 ; j++){
			strcpy(userList[j],userList[j+1]);
		}
		onlineUser--;
	}
}

int main(int argc, char **argv){

/*-----------------------------------------------------------------------------------------------------------------------*/
//创建socket，并连接到服务器上

	if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ){		//创建socket
		perror("Socket created error\n");
		exit(1);
	}

	memset(&servaddr, 0, sizeof(servaddr));			//设置服务器套接字地址
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(SERV_IP);
	servaddr.sin_port = htons(SERV_PORT);

	if( connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 ){	//连接服务器
		perror("Connect error\n");
		exit(1);
	}

	printf("Connect to the server successfully!!!\n");

/*---------------------------------------------------------------------------------------------------------------------*/
//用户注册帐号，或者登录过程，成功登录则进行下一步操作

	while(!loged){
		printf("Please choose the function  (1--Regist; 2--Logon ;3--exit): \n");
		scanf("%s", func);
		validuser = 0;
		int sign = 0;
		while(sign == 0){
			if(strcmp(func,"1") == 0){
				sign = 1;
				regist();
			}
			else if(strcmp(func,"2") == 0){
				sign = 1;
				logon();
			}
			else if(strcmp(func,"3") == 0){
				sign = 1;
				close(sockfd);
				exit(0);
			}
			else 
				printf("Input error!");
		}
	}

/*---------------------------------------------------------------------------------------------------------------------*/
//登录成功的用户选择查看在线好友，以及聊天功能

	system("clear");
	printf("You are online,and your username is %s\n", username);

	pthread_t recvthread;
	int ret1 = pthread_create(&recvthread,NULL,(void*)receiveMsg,NULL);
	if (ret1 != 0){
		printf("Create thread error!\r\n");
		exit(1);
	}

	pthread_t mainthread;
	int ret2 = pthread_create(&mainthread,NULL,(void*)mainThread,NULL);
	if (ret2 != 0){
		printf("Create thread error!\r\n");
		exit(1);
	}

	pthread_join(recvthread,NULL);
	pthread_join(mainthread,NULL);
	return 0;
}
