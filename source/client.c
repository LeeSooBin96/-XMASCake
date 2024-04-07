#define  _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>

#define MENU_NUM 7 //메뉴 갯수 
#define BUF_SIZE 1024
void ErrorHandling(char* message);
void ClearBuffer(void);

typedef struct
{
	char menu[50];
	int price;
}Menu; //메뉴판
typedef struct
{
	int count; //메뉴 개수
	int menu[10]; //메뉴 인덱스 저장 (이름->인덱스로 변경)
	int price[10]; //메뉴가격 저장
	int Tprice; //총 가격 저장
	char name[20]; //회원 이름 저장
	char phone[20]; //핸드폰 번호 저장
	char pay[10]; //결제 방법 저장
}Slist; //장바구니

//메뉴판 출력
void PrintMenu(Menu* menu);
//장바구니 출력
void PrintList(Slist list,Menu* menu);
//장바구니에 메뉴 추가(menu의 num번 요소의 정보를 list에 추가)
void AddMenu(Menu* menu, Slist* list,int num);
//장바구니 수정(장바구니 num번째 요소 삭제)
void Modifylist(Slist* list,int num);
//결제 진행
int Pay(Slist* list,Menu* menu);
//주문 정보 전송
void SndList(SOCKET* sock,Slist list);

int main(int argc, char* argv[])
{
	argc = 3;argv[1] = "127.0.0.1";argv[2] = "19016";//서버 아이피, 포트번호 지정
	WSADATA wsaData; //윈속 라이브러리 구조체
	SOCKET hSocket; //소켓 핸들(디스크립터)
	SOCKADDR_IN servAddr; //주소 정보

	if (argc != 3)
	{
		printf("Usage: %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	while (1)
	{
		char buf[BUF_SIZE] = { 0 };//수신에 사용할 버퍼
		int strLen, recLen; //메시지 총 길이, 수신된 메시지 길이 저장할 변수

		Menu menu[MENU_NUM] = { 0 };//메뉴판 내용 저장할 구조체 변수
		Slist shoplist = { 0 };//주문 정보 저장할 구조체 변수
		char input[5] = { 0 }; //입력 인자
		int quit = 0; //while문 탈출 코드

		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) //소켓 라이브러리 생성
			ErrorHandling("WSAStartup() error!");
		
		hSocket = socket(PF_INET, SOCK_STREAM, 0); //통신에 사용할 소켓 생성
		if (hSocket == INVALID_SOCKET) ErrorHandling("socket() error");

		memset(&servAddr, 0, sizeof(servAddr)); //주소 정보 구조체 초기화
		servAddr.sin_family = AF_INET;
		servAddr.sin_addr.s_addr = inet_addr(argv[1]);
		servAddr.sin_port = htons(atoi(argv[2]));

	
		if (connect(hSocket, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR) //서버에 접속
			ErrorHandling("connect() error!");
		else
			puts("안녕하세요! 정영근 베이커리입니다.\n잠시만 기다려주세요...");

		//메뉴 정보 받아와야 함
		for (int i = 0;i < MENU_NUM;i++)
		{
			recv(hSocket, buf, BUF_SIZE, 0); //메뉴 이름 길이 정보 전달 받음
			send(hSocket, "fin", 4, 0); //수신 확인 메시지
			strLen = atoi(buf); recLen = 0;
			while (strLen != recLen)
			{
				recv(hSocket, &menu[i].menu[recLen++], 1, 0); //한글자씩 입력받음
			}
			send(hSocket, "fin", 4, 0); //수신 확인 메시지
			recv(hSocket, buf, BUF_SIZE, 0);//메뉴 가격 받아오기
			menu[i].price = atoi(buf);
			send(hSocket, "fin", 4, 0); //수신 확인 메시지
			memset(buf, 0, BUF_SIZE); //버퍼 초기화
		}
		Sleep(2000);

		while (!quit)
		{
			system("cls");
			PrintMenu(menu);
			PrintList(shoplist,menu);
			puts("===========================================");
			puts("(장바구니 수정은 -1, 결제를 진행하시려면 0을 입력해주세요)");
			printf("메뉴번호를 입력하세요>");
			fgets(input, sizeof(input), stdin);
			if (input[1] == '\n' && input[0] >= '1' && input[0] < '1' + MENU_NUM) //메뉴 선택 했을 시
			{
				if (shoplist.count == 9)
				{
					puts("장바구니가 꽉찼습니다."); continue;
				}
				AddMenu(menu, &shoplist, atoi(input) - 1);
				printf("%s가 장바구니에 담겼습니다. \n", menu[atoi(input) - 1].menu);
				Sleep(1000);
			}
			else if (input[1] == '\n' && input[0] == '0') //결제 선택했을 시
			{
				if (shoplist.count == 0)
				{
					puts("선택한 메뉴가 없습니다.");Sleep(1000);
					continue;
				}
				quit = Pay(&shoplist,menu); //결제 진행
				if (quit == 0) continue;
				SndList(&hSocket, shoplist);
				Sleep(1000);
			}
			else if (input[2] == '\n' && input[0] == '-' && input[1] == '1') //장바구니 수정 선택했을 시
			{
				system("cls");
				PrintList(shoplist,menu);
				puts("===========================================");
				printf("삭제하고 싶은 메뉴번호를 선택하세요(취소는 0)>");
				memset(input, 0, sizeof(input));
				fgets(input, sizeof(input), stdin);
				if (input[1] == '\n' && input[0] >= '1' && input[0] <= '9')
					Modifylist(&shoplist, atoi(input));
				else if (input[1] == '\n' && input[0] == '0') continue;
				else
				{
					puts("잘못된 입력입니다.");
				}
				Sleep(1000);
			}
			else //잘못된 입력
			{
				if (strstr(input, "\n") == NULL && strlen(input) == sizeof(input)) ClearBuffer();
				puts("잘못된 입력입니다.");
				Sleep(1000);
			}
		}
		puts("잠시후 초기화면으로 돌아갑니다.");
		Sleep(2000);
		system("cls");
		closesocket(hSocket); //연결 종료
		WSACleanup(); //윈속 라이브러리 해제
	}
	return 0;
}
void ErrorHandling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
void ClearBuffer(void)
{
	while (getchar() != '\n'); 
}
void PrintMenu(Menu* menu)
{
	puts("   크리스마스 케이크 예약 주문 프로그램");
	puts("===========================================");
	for (int i = 0;i < MENU_NUM;i++)
	{
		printf("%d) %17s : %6d원 \n", i + 1, menu[i].menu, menu[i].price);
	}
}
void PrintList(Slist list,Menu* menu)
{
	puts("===========================================");
	puts("선택하신 메뉴>");
	for (int i = 0;i <list.count;i++)
	{
		printf("%d) %17s : %6d원 \n", i+1, menu[list.menu[i]].menu, list.price[i]);
	}
	puts("-------------------------------------------");
	printf("\t     총 가격 : % 6d원 \n", list.Tprice);
}
void AddMenu(Menu* menu, Slist* list, int num)
{
	list->menu[list->count]=num; //메뉴 추가
	list->price[list->count] = menu[num].price; //가격 추가
	list->Tprice += menu[num].price; //총 가격에 적용
	list->count++; //메뉴 개수 카운트
}
void Modifylist(Slist* list,int num)
{
	list->Tprice -= list->price[num - 1]; //총 가격에서 제외
	for (int i = num;i < list->count;i++)
	{
		list->menu[i - 1] = list->menu[i]; //메뉴 리스트에서 제거
		list->price[i - 1] = list->price[i]; //가격 리스트에서 제거
	}
	list->count--; //개수 감소
	list->menu[list->count]=0;list->price[list->count] = 0; //마지막 요소 초기화
}
int Pay(Slist* list,Menu* menu)
{
	char input[3] = { 0 }; //입력 인자

	system("cls");
	while (1)
	{
		puts("결제를 진행합니다...");
		Sleep(1000);
		PrintList(*list,menu);
		puts("===========================================");
		printf("결제를 계속하시겠습니까?(y/n)");
		fgets(input, sizeof(input), stdin);
		if (input[1] == '\n' && input[0] == 'n' || input[0] == 'N') return 0; //주문화면으로 돌아가기
		else if (input[1] == '\n' && input[0] == 'y' || input[0] == 'Y') break;
		else
		{
			if (strstr(input, "\n") == NULL && strlen(input) == sizeof(input)-1) ClearBuffer();
			puts("잘못된 입력입니다.");
		}Sleep(1000);system("cls");printf("계속해서 ");
	}
	while (1)
	{
		puts("===========================================");
		printf("주문자 성함을 입력하세요>");
		fgets(list->name, sizeof(list->name), stdin);
		printf("입력된 이름: %s \n", list->name);
		printf("연락처를 입력해주세요>");
		fgets(list->phone, sizeof(list->phone), stdin);
		printf("입력된 연락처: %s \n", list->phone);
		puts("===========================================");
		printf("위의 정보가 맞습니까?(y/n)");
		fgets(input, sizeof(input), stdin);
		if (input[1] == '\n' && input[0] == 'y' || input[0] == 'Y') break;
		else if (input[1] == '\n' && input[0] == 'n' || input[0] == 'N');
		else
		{
			if (strstr(input, "\n") == NULL && strlen(input) == sizeof(input) - 1) ClearBuffer();
			puts("잘못된 입력입니다.");
		}Sleep(1000);system("cls"); puts("다시 한번 입력해주세요.");
	}
	puts("===========================================");
	puts("1. 현금(계좌 이체)");
	puts("2. 카드");
	printf("결제 방법을 선택하세요>");
	while (1)
	{
		fgets(input, sizeof(input), stdin);
		if (input[1] == '\n' && input[0] == '1') strcpy(list->pay, "현금");
		else if(input[1] == '\n' && input[0] == '2') strcpy(list->pay, "카드");
		else
		{
			if (strstr(input, "\n") == NULL && strlen(input) == sizeof(input) - 1) ClearBuffer();
			printf("잘못된 입력입니다. 다시 입력해주세요>");
			continue;
		}
		break;
	}
	puts("===========================================");
	printf("%d원이 결제됩니다... \n", list->Tprice);
	Sleep(1000);
	puts("결제가 완료되었습니다.");
	return 1;
}
void SndList(SOCKET* sock, Slist list)
{
	char buf[BUF_SIZE] = { 0 }; //수신 버퍼
	char snd[BUF_SIZE] = { 0 }; //보낼 메시지
	int ordernum;

	//주문자 정보 전송
	sprintf(snd, "%zd", strlen(list.name));
	send(*sock, snd, strlen(snd), 0);//이름의 길이 전송
	recv(*sock, (char*)&ordernum, sizeof(int), 0);//주문 번호 수신
	printf("주문 번호: %d \n", ordernum);
	puts("맛있게 만들어드리겠습니다. 이용해주셔서 감사합니다.");
	send(*sock, list.name, strlen(list.name), 0); //주문자 이름 전송
	recv(*sock, buf, BUF_SIZE, 0);//확인메시지 수신

	sprintf(snd, "%zd", strlen(list.phone));
	send(*sock, snd, strlen(snd), 0);//전화번호의 길이 전송
	recv(*sock, buf, BUF_SIZE, 0);//확인메시지 수신
	send(*sock, list.phone, strlen(list.phone), 0);//전화번호 전송
	recv(*sock, buf, BUF_SIZE, 0);//확인메시지 수신

	send(*sock,(char*) &list.count, sizeof(int), 0); //장바구니 리스트 개수 송신
	recv(*sock, buf, BUF_SIZE, 0);//확인메시지 수신
	for (int i = 0;i < list.count;i++) //주문내역 전송
	{
		send(*sock, (char*)&list.menu[i], sizeof(int), 0);
		recv(*sock, buf, BUF_SIZE, 0);//확인메시지 수신
	}
	Sleep(1000);
}
