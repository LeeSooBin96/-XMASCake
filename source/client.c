#define  _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>

#define MENU_NUM 7 //�޴� ���� 
#define BUF_SIZE 1024
void ErrorHandling(char* message);
void ClearBuffer(void);

typedef struct
{
	char menu[50];
	int price;
}Menu; //�޴���
typedef struct
{
	int count; //�޴� ����
	int menu[10]; //�޴� �ε��� ���� (�̸�->�ε����� ����)
	int price[10]; //�޴����� ����
	int Tprice; //�� ���� ����
	char name[20]; //ȸ�� �̸� ����
	char phone[20]; //�ڵ��� ��ȣ ����
	char pay[10]; //���� ��� ����
}Slist; //��ٱ���

//�޴��� ���
void PrintMenu(Menu* menu);
//��ٱ��� ���
void PrintList(Slist list,Menu* menu);
//��ٱ��Ͽ� �޴� �߰�(menu�� num�� ����� ������ list�� �߰�)
void AddMenu(Menu* menu, Slist* list,int num);
//��ٱ��� ����(��ٱ��� num��° ��� ����)
void Modifylist(Slist* list,int num);
//���� ����
int Pay(Slist* list,Menu* menu);
//�ֹ� ���� ����
void SndList(SOCKET* sock,Slist list);

int main(int argc, char* argv[])
{
	argc = 3;argv[1] = "127.0.0.1";argv[2] = "19016";//���� ������, ��Ʈ��ȣ ����
	WSADATA wsaData; //���� ���̺귯�� ����ü
	SOCKET hSocket; //���� �ڵ�(��ũ����)
	SOCKADDR_IN servAddr; //�ּ� ����

	if (argc != 3)
	{
		printf("Usage: %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	while (1)
	{
		char buf[BUF_SIZE] = { 0 };//���ſ� ����� ����
		int strLen, recLen; //�޽��� �� ����, ���ŵ� �޽��� ���� ������ ����

		Menu menu[MENU_NUM] = { 0 };//�޴��� ���� ������ ����ü ����
		Slist shoplist = { 0 };//�ֹ� ���� ������ ����ü ����
		char input[5] = { 0 }; //�Է� ����
		int quit = 0; //while�� Ż�� �ڵ�

		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) //���� ���̺귯�� ����
			ErrorHandling("WSAStartup() error!");
		
		hSocket = socket(PF_INET, SOCK_STREAM, 0); //��ſ� ����� ���� ����
		if (hSocket == INVALID_SOCKET) ErrorHandling("socket() error");

		memset(&servAddr, 0, sizeof(servAddr)); //�ּ� ���� ����ü �ʱ�ȭ
		servAddr.sin_family = AF_INET;
		servAddr.sin_addr.s_addr = inet_addr(argv[1]);
		servAddr.sin_port = htons(atoi(argv[2]));

	
		if (connect(hSocket, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR) //������ ����
			ErrorHandling("connect() error!");
		else
			puts("�ȳ��ϼ���! ������ ����Ŀ���Դϴ�.\n��ø� ��ٷ��ּ���...");

		//�޴� ���� �޾ƿ;� ��
		for (int i = 0;i < MENU_NUM;i++)
		{
			recv(hSocket, buf, BUF_SIZE, 0); //�޴� �̸� ���� ���� ���� ����
			send(hSocket, "fin", 4, 0); //���� Ȯ�� �޽���
			strLen = atoi(buf); recLen = 0;
			while (strLen != recLen)
			{
				recv(hSocket, &menu[i].menu[recLen++], 1, 0); //�ѱ��ھ� �Է¹���
			}
			send(hSocket, "fin", 4, 0); //���� Ȯ�� �޽���
			recv(hSocket, buf, BUF_SIZE, 0);//�޴� ���� �޾ƿ���
			menu[i].price = atoi(buf);
			send(hSocket, "fin", 4, 0); //���� Ȯ�� �޽���
			memset(buf, 0, BUF_SIZE); //���� �ʱ�ȭ
		}
		Sleep(2000);

		while (!quit)
		{
			system("cls");
			PrintMenu(menu);
			PrintList(shoplist,menu);
			puts("===========================================");
			puts("(��ٱ��� ������ -1, ������ �����Ͻ÷��� 0�� �Է����ּ���)");
			printf("�޴���ȣ�� �Է��ϼ���>");
			fgets(input, sizeof(input), stdin);
			if (input[1] == '\n' && input[0] >= '1' && input[0] < '1' + MENU_NUM) //�޴� ���� ���� ��
			{
				if (shoplist.count == 9)
				{
					puts("��ٱ��ϰ� ��á���ϴ�."); continue;
				}
				AddMenu(menu, &shoplist, atoi(input) - 1);
				printf("%s�� ��ٱ��Ͽ� �����ϴ�. \n", menu[atoi(input) - 1].menu);
				Sleep(1000);
			}
			else if (input[1] == '\n' && input[0] == '0') //���� �������� ��
			{
				if (shoplist.count == 0)
				{
					puts("������ �޴��� �����ϴ�.");Sleep(1000);
					continue;
				}
				quit = Pay(&shoplist,menu); //���� ����
				if (quit == 0) continue;
				SndList(&hSocket, shoplist);
				Sleep(1000);
			}
			else if (input[2] == '\n' && input[0] == '-' && input[1] == '1') //��ٱ��� ���� �������� ��
			{
				system("cls");
				PrintList(shoplist,menu);
				puts("===========================================");
				printf("�����ϰ� ���� �޴���ȣ�� �����ϼ���(��Ҵ� 0)>");
				memset(input, 0, sizeof(input));
				fgets(input, sizeof(input), stdin);
				if (input[1] == '\n' && input[0] >= '1' && input[0] <= '9')
					Modifylist(&shoplist, atoi(input));
				else if (input[1] == '\n' && input[0] == '0') continue;
				else
				{
					puts("�߸��� �Է��Դϴ�.");
				}
				Sleep(1000);
			}
			else //�߸��� �Է�
			{
				if (strstr(input, "\n") == NULL && strlen(input) == sizeof(input)) ClearBuffer();
				puts("�߸��� �Է��Դϴ�.");
				Sleep(1000);
			}
		}
		puts("����� �ʱ�ȭ������ ���ư��ϴ�.");
		Sleep(2000);
		system("cls");
		closesocket(hSocket); //���� ����
		WSACleanup(); //���� ���̺귯�� ����
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
	puts("   ũ�������� ����ũ ���� �ֹ� ���α׷�");
	puts("===========================================");
	for (int i = 0;i < MENU_NUM;i++)
	{
		printf("%d) %17s : %6d�� \n", i + 1, menu[i].menu, menu[i].price);
	}
}
void PrintList(Slist list,Menu* menu)
{
	puts("===========================================");
	puts("�����Ͻ� �޴�>");
	for (int i = 0;i <list.count;i++)
	{
		printf("%d) %17s : %6d�� \n", i+1, menu[list.menu[i]].menu, list.price[i]);
	}
	puts("-------------------------------------------");
	printf("\t     �� ���� : % 6d�� \n", list.Tprice);
}
void AddMenu(Menu* menu, Slist* list, int num)
{
	list->menu[list->count]=num; //�޴� �߰�
	list->price[list->count] = menu[num].price; //���� �߰�
	list->Tprice += menu[num].price; //�� ���ݿ� ����
	list->count++; //�޴� ���� ī��Ʈ
}
void Modifylist(Slist* list,int num)
{
	list->Tprice -= list->price[num - 1]; //�� ���ݿ��� ����
	for (int i = num;i < list->count;i++)
	{
		list->menu[i - 1] = list->menu[i]; //�޴� ����Ʈ���� ����
		list->price[i - 1] = list->price[i]; //���� ����Ʈ���� ����
	}
	list->count--; //���� ����
	list->menu[list->count]=0;list->price[list->count] = 0; //������ ��� �ʱ�ȭ
}
int Pay(Slist* list,Menu* menu)
{
	char input[3] = { 0 }; //�Է� ����

	system("cls");
	while (1)
	{
		puts("������ �����մϴ�...");
		Sleep(1000);
		PrintList(*list,menu);
		puts("===========================================");
		printf("������ ����Ͻðڽ��ϱ�?(y/n)");
		fgets(input, sizeof(input), stdin);
		if (input[1] == '\n' && input[0] == 'n' || input[0] == 'N') return 0; //�ֹ�ȭ������ ���ư���
		else if (input[1] == '\n' && input[0] == 'y' || input[0] == 'Y') break;
		else
		{
			if (strstr(input, "\n") == NULL && strlen(input) == sizeof(input)-1) ClearBuffer();
			puts("�߸��� �Է��Դϴ�.");
		}Sleep(1000);system("cls");printf("����ؼ� ");
	}
	while (1)
	{
		puts("===========================================");
		printf("�ֹ��� ������ �Է��ϼ���>");
		fgets(list->name, sizeof(list->name), stdin);
		printf("�Էµ� �̸�: %s \n", list->name);
		printf("����ó�� �Է����ּ���>");
		fgets(list->phone, sizeof(list->phone), stdin);
		printf("�Էµ� ����ó: %s \n", list->phone);
		puts("===========================================");
		printf("���� ������ �½��ϱ�?(y/n)");
		fgets(input, sizeof(input), stdin);
		if (input[1] == '\n' && input[0] == 'y' || input[0] == 'Y') break;
		else if (input[1] == '\n' && input[0] == 'n' || input[0] == 'N');
		else
		{
			if (strstr(input, "\n") == NULL && strlen(input) == sizeof(input) - 1) ClearBuffer();
			puts("�߸��� �Է��Դϴ�.");
		}Sleep(1000);system("cls"); puts("�ٽ� �ѹ� �Է����ּ���.");
	}
	puts("===========================================");
	puts("1. ����(���� ��ü)");
	puts("2. ī��");
	printf("���� ����� �����ϼ���>");
	while (1)
	{
		fgets(input, sizeof(input), stdin);
		if (input[1] == '\n' && input[0] == '1') strcpy(list->pay, "����");
		else if(input[1] == '\n' && input[0] == '2') strcpy(list->pay, "ī��");
		else
		{
			if (strstr(input, "\n") == NULL && strlen(input) == sizeof(input) - 1) ClearBuffer();
			printf("�߸��� �Է��Դϴ�. �ٽ� �Է����ּ���>");
			continue;
		}
		break;
	}
	puts("===========================================");
	printf("%d���� �����˴ϴ�... \n", list->Tprice);
	Sleep(1000);
	puts("������ �Ϸ�Ǿ����ϴ�.");
	return 1;
}
void SndList(SOCKET* sock, Slist list)
{
	char buf[BUF_SIZE] = { 0 }; //���� ����
	char snd[BUF_SIZE] = { 0 }; //���� �޽���
	int ordernum;

	//�ֹ��� ���� ����
	sprintf(snd, "%zd", strlen(list.name));
	send(*sock, snd, strlen(snd), 0);//�̸��� ���� ����
	recv(*sock, (char*)&ordernum, sizeof(int), 0);//�ֹ� ��ȣ ����
	printf("�ֹ� ��ȣ: %d \n", ordernum);
	puts("���ְ� �����帮�ڽ��ϴ�. �̿����ּż� �����մϴ�.");
	send(*sock, list.name, strlen(list.name), 0); //�ֹ��� �̸� ����
	recv(*sock, buf, BUF_SIZE, 0);//Ȯ�θ޽��� ����

	sprintf(snd, "%zd", strlen(list.phone));
	send(*sock, snd, strlen(snd), 0);//��ȭ��ȣ�� ���� ����
	recv(*sock, buf, BUF_SIZE, 0);//Ȯ�θ޽��� ����
	send(*sock, list.phone, strlen(list.phone), 0);//��ȭ��ȣ ����
	recv(*sock, buf, BUF_SIZE, 0);//Ȯ�θ޽��� ����

	send(*sock,(char*) &list.count, sizeof(int), 0); //��ٱ��� ����Ʈ ���� �۽�
	recv(*sock, buf, BUF_SIZE, 0);//Ȯ�θ޽��� ����
	for (int i = 0;i < list.count;i++) //�ֹ����� ����
	{
		send(*sock, (char*)&list.menu[i], sizeof(int), 0);
		recv(*sock, buf, BUF_SIZE, 0);//Ȯ�θ޽��� ����
	}
	Sleep(1000);
}
