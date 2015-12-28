// udpsender.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include <windows.h>

#pragma comment(lib,"Ws2_32.lib")


int _tmain(int argc, _TCHAR* argv[])
{
	if ( argc != 4 ) {
		fprintf(stderr, "Usage: udpsender.exe <address> <port> <message>\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "  ex) udpsender.exe \"127.0.0.1\" 57777 \"Send Message.\"\n");
		fprintf(stderr, "\n");
		return 1;
	}

	// WinSock2初期化
	WSADATA wsaData;
	int err = ::WSAStartup(MAKEWORD(2, 2), &wsaData);
	if ( err != 0 ) {
		fprintf(stderr, "[ERROR] WSAStartup failed with error: %d\n", err);
		return 1;
	}
	
	int ret = 1;
	int iResult;
	SOCKET sock = INVALID_SOCKET;
	sockaddr_in RecvAddr;
	char target_ip[50] = ""; 
	unsigned short target_port = 0;
	char send_buf[1024] = "";
	char recv_buf[1024] = "";

	strncpy(target_ip, argv[1], sizeof(target_ip) - 1);
	target_port = (unsigned short)atoi(argv[2]);
	strncpy(send_buf, argv[3], sizeof(send_buf) - 1);

	printf("[ ip=%s, port=%u ]\n", target_ip, target_port);
	printf("SEND: %s\n", send_buf);

	// socketオープン
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if ( sock == INVALID_SOCKET ) {
		fprintf(stderr, "[ERROR] socket failed with error: %d\n", ::WSAGetLastError());
		goto EXIT;
	}

	RecvAddr.sin_family = AF_INET;
	RecvAddr.sin_port = htons(target_port);
	RecvAddr.sin_addr.s_addr = inet_addr(target_ip);

	// Send a datagram to the receiver
	iResult = sendto(sock, send_buf, strlen(send_buf), 0, (SOCKADDR *)&RecvAddr, sizeof(RecvAddr));
	if ( iResult == SOCKET_ERROR ) {
		fprintf(stderr, "[ERROR] sendto failed with error: %d\n", ::WSAGetLastError());
		goto EXIT;
	}

	iResult = recv(sock, recv_buf, sizeof(recv_buf) - 1, 0);
	if ( iResult == SOCKET_ERROR ) {
		fprintf(stderr, "[ERROR] recvfrom failed with error: %d\n", ::WSAGetLastError());
		goto EXIT;
	}

	printf("RECV: %s\n", recv_buf);
	ret = 0;

EXIT:
	// socketクローズ
	if ( sock != INVALID_SOCKET ) {
		closesocket(sock);
	}

	// WinSock2終了
	::WSACleanup();

	return ret;
}

