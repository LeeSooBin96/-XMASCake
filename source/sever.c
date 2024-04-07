#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>

#define MENU_NUM 7 //�޴� ���� 
#define BUF_SIZE 1024
void ErrorHandling(char* message);

typedef struct
{
	char menu[50];
	int price;
}Menu; //�޴���

int main(int argc, char* argv[])
{
	argc = 2, argv[1] = "19016"; //��Ʈ��ȣ ����
	WSADATA wsaData; //���� ���̺귯�� ����ü
	SOCKET hServSock, hClntSock; //���� �ڵ�(��ũ����)
	SOCKADDR_IN servAddr, clntAddr; //�ּ� ����

	int szClntAddr; //�ּ� ũ�� ������ ����
	int strLen, recLen, i; //�޽��� �� ���̿� ���ŵ� �޽��� ���� ������ ����
	
	//�޴���
	Menu cake[MENU_NUM] = { {"��ũ������ũ 1ȣ",24000},{"��������ũ 1ȣ",25000},{"��������ũ 1ȣ",26000},{"��ũ������ũ 2ȣ",30000},{"��������ũ 2ȣ",31000},{"��������ũ 2ȣ",32000},{"�������ũ 3ȣ",45000} };
	int listNum, menuIdx;//��ٱ��� ����Ʈ ��, �޴� �ε��� ����


	if (argc != 2)
	{
		printf("Usage: %s <port>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) //���� ���̺귯�� �ʱ�ȭ
		ErrorHandling("WSAStartup() error!");

	hServSock = socket(PF_INET, SOCK_STREAM, 0); //���� ���� ����
	if (hServSock == INVALID_SOCKET) ErrorHandling("socket() error");

	memset(&servAddr, 0, sizeof(servAddr)); //�ּ����� ����ü �ʱ�ȭ
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(atoi(argv[1]));

	if (bind(hServSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR) //IP, ��Ʈ��ȣ �Ҵ�
		ErrorHandling("bind() error");

	if (listen(hServSock, 10) == SOCKET_ERROR) //���� ��� ���� ����
		ErrorHandling("listen() error");

	while (1) // ����ؼ� Ŭ���̾�Ʈ ����
	{
		//�� Ŭ���̾�Ʈ ���� �ʱ�ȭ �Ǿ�� �� ������
		char buf[BUF_SIZE] = { 0 }; //���ſ� ���� ����
		char SndMsg[BUF_SIZE] = { 0 }; //�۽ſ� ���� ���ڿ�
		int ordernum = 0; //�ֹ� ��ȣ
		int totalPrice = 0; //�� ���� ����
		//�ֹ����� ������ ���� ����
		char line[BUF_SIZE] = { 0 };
		FILE* fp = fopen("orderList.txt", "a+");
		while (1) //�ֹ� ��ȣ ������Ű��
		{
			fgets(line, BUF_SIZE, fp);
			if (feof(fp)) break;
			if (strstr(line, "�ֹ� ��ȣ") != NULL) ordernum++;
		}

		szClntAddr = sizeof(clntAddr);
		hClntSock = accept(hServSock, (SOCKADDR*)&clntAddr, &szClntAddr); // Ŭ���̾�Ʈ ����
		if (hClntSock == INVALID_SOCKET) ErrorHandling("accept() error");

		//�޴� ���� �����ؾ� ��
		for ( i = 0;i < MENU_NUM;i++)
		{
			//���̸� ���� ������ ���� ���� ���� ���� �����ϰ� �׸�ŭ �о���̰�!
			sprintf(SndMsg, "%zd", strlen(cake[i].menu));
			send(hClntSock, SndMsg, strlen(SndMsg), 0); //���� ����
			recv(hClntSock, buf, BUF_SIZE, 0); //Ȯ�θ޽��� ����
			send(hClntSock, cake[i].menu, strlen(cake[i].menu), 0); //�޴� �̸� ����
			recv(hClntSock, buf, BUF_SIZE, 0); //Ȯ�θ޽��� ����
			sprintf(SndMsg, "%d", cake[i].price); //������ �ƽ�Ű �ڵ尪�� �Ѿ�� ���ڿ��� �־
			send(hClntSock, SndMsg, strlen(SndMsg), 0);
			recv(hClntSock, buf, BUF_SIZE, 0); //Ȯ�θ޽��� ����
		} memset(buf, 0, BUF_SIZE); memset(SndMsg, 0, BUF_SIZE);

		
		//�ֹ��� ���� ����
		if (recv(hClntSock, buf, BUF_SIZE, 0) > 0) //�ֹ��� �̸� ���� ����
		{
			ordernum++; //�ֹ��� ���� ���� ������ �ֹ� ��ȣ ����
			fprintf(fp, "�ֹ� ��ȣ: %d \n", ordernum); printf("�ֹ� ��ȣ: %d \n", ordernum);
			send(hClntSock, (char*)&ordernum, sizeof(int), 0);//�ֹ� ��ȣ ����
			strLen = atoi(buf); recLen = 0; fputs("�ֹ��� �̸�: ",fp);fputs("�ֹ��� �̸�: ", stdout);
			while (strLen != recLen) //�ֹ��� �̸� ����
			{
				recv(hClntSock, buf, 1, 0); recLen++;
				fputc(buf[0], fp); //�ѱ��ھ� �Է�
				fputc(buf[0], stdout); //ȭ�鿡�� ���
			}
			send(hClntSock, "fin", 4, 0); //���� Ȯ�� �޽���

			memset(buf, 0, BUF_SIZE);
			recv(hClntSock, buf, BUF_SIZE, 0); //��ȭ��ȣ ���� ����
			send(hClntSock, "fin", 4, 0); //���� Ȯ�� �޽���
			strLen = atoi(buf); recLen = 0; fputs("�ֹ��� ��ȭ��ȣ: ", fp);fputs("�ֹ��� ��ȭ��ȣ: ", stdout);
			while (strLen != recLen) //�ֹ��� ��ȭ��ȣ ����
			{
				recv(hClntSock, buf, 1, 0); recLen++;
				fputc(buf[0], fp); //�ѱ��ھ� �Է�
				fputc(buf[0], stdout); //ȭ�鿡�� ���
			}
			send(hClntSock, "fin", 4, 0); //���� Ȯ�� �޽���

			//�ֹ����� ����
			recv(hClntSock, (char*)&listNum, sizeof(int), 0); //��ٱ��� ����Ʈ ���� ����
			send(hClntSock, "fin", 4, 0); //���� Ȯ�� �޽���
			fputs("�ֹ� ����>\n", fp);fputs("�ֹ� ����>\n", stdout);
			for ( i = 0;i < listNum;i++) //�ֹ����� ����
			{
				recv(hClntSock, (char*)&menuIdx, sizeof(int), 0); //�޴� �ε��� ����
				fprintf(fp, "%d) %17s : %6d��\n", i + 1, cake[menuIdx].menu, cake[menuIdx].price);
				printf("%d) %17s : %6d��\n", i + 1, cake[menuIdx].menu, cake[menuIdx].price);
				totalPrice += cake[menuIdx].price;
				send(hClntSock, "fin", 4, 0); //���� Ȯ�� �޽���
			}
			fprintf(fp, "\t     �� ���� : % 6d�� \n", totalPrice);printf("\t     �� ���� : % 6d�� \n", totalPrice);
			fputs("--------------------------------------\n", fp);
			puts("--------------------------------------");
			fclose(fp);

			closesocket(hClntSock);
		}
	}
	closesocket(hServSock);
	WSACleanup(); //���� ���̺귯�� ����
	return 0;
}
void ErrorHandling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}