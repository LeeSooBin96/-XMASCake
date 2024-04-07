#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>

#define MENU_NUM 7 //메뉴 갯수 
#define BUF_SIZE 1024
void ErrorHandling(char* message);

typedef struct
{
	char menu[50];
	int price;
}Menu; //메뉴판

int main(int argc, char* argv[])
{
	argc = 2, argv[1] = "19016"; //포트번호 지정
	WSADATA wsaData; //윈속 라이브러리 구조체
	SOCKET hServSock, hClntSock; //소켓 핸들(디스크립터)
	SOCKADDR_IN servAddr, clntAddr; //주소 정보

	int szClntAddr; //주소 크기 저장할 변수
	int strLen, recLen, i; //메시지 총 길이와 수신된 메시지 길이 저장할 변수
	
	//메뉴판
	Menu cake[MENU_NUM] = { {"생크림케이크 1호",24000},{"초코케이크 1호",25000},{"고구마케이크 1호",26000},{"생크림케이크 2호",30000},{"초코케이크 2호",31000},{"고구마케이크 2호",32000},{"모듬케이크 3호",45000} };
	int listNum, menuIdx;//장바구니 리스트 수, 메뉴 인덱스 저장


	if (argc != 2)
	{
		printf("Usage: %s <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) //소켓 라이브러리 초기화
		ErrorHandling("WSAStartup() error!");

	hServSock = socket(PF_INET, SOCK_STREAM, 0); //서버 소켓 생성
	if (hServSock == INVALID_SOCKET) ErrorHandling("socket() error");

	memset(&servAddr, 0, sizeof(servAddr)); //주소정보 구조체 초기화
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(atoi(argv[1]));

	if (bind(hServSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR) //IP, 포트번호 할당
		ErrorHandling("bind() error");

	if (listen(hServSock, 10) == SOCKET_ERROR) //연결 대기 상태 진입
		ErrorHandling("listen() error");

	while (1) // 계속해서 클라이언트 받음
	{
		//매 클라이언트 마다 초기화 되어야 할 정보들
		char buf[BUF_SIZE] = { 0 }; //수신에 사용될 버퍼
		char SndMsg[BUF_SIZE] = { 0 }; //송신에 사용될 문자열
		int ordernum = 0; //주문 번호
		int totalPrice = 0; //총 가격 저장
		//주문정보 저장할 파일 열기
		char line[BUF_SIZE] = { 0 };
		FILE* fp = fopen("orderList.txt", "a+");
		while (1) //주문 번호 누적시키기
		{
			fgets(line, BUF_SIZE, fp);
			if (feof(fp)) break;
			if (strstr(line, "주문 번호") != NULL) ordernum++;
		}

		szClntAddr = sizeof(clntAddr);
		hClntSock = accept(hServSock, (SOCKADDR*)&clntAddr, &szClntAddr); // 클라이언트 연결
		if (hClntSock == INVALID_SOCKET) ErrorHandling("accept() error");

		//메뉴 정보 전송해야 함
		for ( i = 0;i < MENU_NUM;i++)
		{
			//길이를 같이 보내려 하지 말고 길이 먼저 전송하고 그만큼 읽어들이게!
			sprintf(SndMsg, "%zd", strlen(cake[i].menu));
			send(hClntSock, SndMsg, strlen(SndMsg), 0); //길이 전송
			recv(hClntSock, buf, BUF_SIZE, 0); //확인메시지 수신
			send(hClntSock, cake[i].menu, strlen(cake[i].menu), 0); //메뉴 이름 전송
			recv(hClntSock, buf, BUF_SIZE, 0); //확인메시지 수신
			sprintf(SndMsg, "%d", cake[i].price); //가격은 아스키 코드값을 넘어가니 문자열에 넣어서
			send(hClntSock, SndMsg, strlen(SndMsg), 0);
			recv(hClntSock, buf, BUF_SIZE, 0); //확인메시지 수신
		} memset(buf, 0, BUF_SIZE); memset(SndMsg, 0, BUF_SIZE);

		
		//주문자 정보 저장
		if (recv(hClntSock, buf, BUF_SIZE, 0) > 0) //주문자 이름 길이 수신
		{
			ordernum++; //주문자 정보 전송 받으면 주문 번호 갱신
			fprintf(fp, "주문 번호: %d \n", ordernum); printf("주문 번호: %d \n", ordernum);
			send(hClntSock, (char*)&ordernum, sizeof(int), 0);//주문 번호 전송
			strLen = atoi(buf); recLen = 0; fputs("주문자 이름: ",fp);fputs("주문자 이름: ", stdout);
			while (strLen != recLen) //주문자 이름 수신
			{
				recv(hClntSock, buf, 1, 0); recLen++;
				fputc(buf[0], fp); //한글자씩 입력
				fputc(buf[0], stdout); //화면에도 출력
			}
			send(hClntSock, "fin", 4, 0); //수신 확인 메시지

			memset(buf, 0, BUF_SIZE);
			recv(hClntSock, buf, BUF_SIZE, 0); //전화번호 길이 수신
			send(hClntSock, "fin", 4, 0); //수신 확인 메시지
			strLen = atoi(buf); recLen = 0; fputs("주문자 전화번호: ", fp);fputs("주문자 전화번호: ", stdout);
			while (strLen != recLen) //주문자 전화번호 수신
			{
				recv(hClntSock, buf, 1, 0); recLen++;
				fputc(buf[0], fp); //한글자씩 입력
				fputc(buf[0], stdout); //화면에도 출력
			}
			send(hClntSock, "fin", 4, 0); //수신 확인 메시지

			//주문정보 수신
			recv(hClntSock, (char*)&listNum, sizeof(int), 0); //장바구니 리스트 개수 수신
			send(hClntSock, "fin", 4, 0); //수신 확인 메시지
			fputs("주문 내역>\n", fp);fputs("주문 내역>\n", stdout);
			for ( i = 0;i < listNum;i++) //주문내역 수신
			{
				recv(hClntSock, (char*)&menuIdx, sizeof(int), 0); //메뉴 인덱스 수신
				fprintf(fp, "%d) %17s : %6d원\n", i + 1, cake[menuIdx].menu, cake[menuIdx].price);
				printf("%d) %17s : %6d원\n", i + 1, cake[menuIdx].menu, cake[menuIdx].price);
				totalPrice += cake[menuIdx].price;
				send(hClntSock, "fin", 4, 0); //수신 확인 메시지
			}
			fprintf(fp, "\t     총 가격 : % 6d원 \n", totalPrice);printf("\t     총 가격 : % 6d원 \n", totalPrice);
			fputs("--------------------------------------\n", fp);
			puts("--------------------------------------");
			fclose(fp);

			closesocket(hClntSock);
		}
	}
	closesocket(hServSock);
	WSACleanup(); //소켓 라이브러리 해제
	return 0;
}
void ErrorHandling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}