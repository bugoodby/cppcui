// tcpsender.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include <windows.h>

#pragma comment(lib,"Ws2_32.lib")


BOOL ReadFileWithAlloc( LPCTSTR szFileName, LPDWORD pdwSize, LPBYTE *ppBytes )
{
	HANDLE		hFile = INVALID_HANDLE_VALUE;
	DWORD		dwBytes = 0;
	BOOL		bSuccess = FALSE;

	// Validate pointer parameters
	if ( ( pdwSize == NULL ) || ( ppBytes == NULL ) ) {
		fprintf(stderr, "[ERROR] invalid argument\n");
		goto EXIT;
	}
	
	// Open the file for reading
	hFile = ::CreateFile( szFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hFile == INVALID_HANDLE_VALUE ) {
		fprintf(stderr, "[ERROR] CreateFile failed (err=0x%x)\n", ::GetLastError());
		goto EXIT;
	}
	
	// How big is the file?
	*pdwSize = ::GetFileSize( hFile, NULL );
	if( *pdwSize == (DWORD)-1 ) {
		fprintf(stderr, "[ERROR] GetFileSize failed (err=0x%x)\n", ::GetLastError());
		goto EXIT;
	}

	// Allocate the memory
	*ppBytes = (LPBYTE)::GlobalAlloc( GPTR, *pdwSize );
	if ( *ppBytes == NULL ) {
		fprintf(stderr, "[ERROR] GlobalAlloc failed (err=0x%x)\n", ::GetLastError());
		goto EXIT;
	}
	
	// Read the file into the newly allocated memory
	bSuccess = ::ReadFile( hFile, *ppBytes, *pdwSize, &dwBytes, NULL );
	if( !bSuccess ) {
		fprintf(stderr, "[ERROR] ReadFile failed (err=0x%x)\n", ::GetLastError());
		goto EXIT;
	}

EXIT:
	// Clean up
	if ( hFile != INVALID_HANDLE_VALUE ) ::CloseHandle( hFile );
	return bSuccess;
}


int _tmain(int argc, _TCHAR* argv[])
{
	if ( argc != 4 ) {
		fprintf(stderr, "Usage: tcpsender.exe <address> <port> <file>\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "  ex) udpsender.exe \"127.0.0.1\" 9100 \"test.bin\"\n");
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

	// 送信ファイル読み込み
	LPBYTE pBytes = NULL;
	DWORD dwSize = 0;
	if ( !ReadFileWithAlloc(argv[3], &dwSize, &pBytes) ) {
		fprintf(stderr, "[ERROR] ReadFileWithAlloc failed.\n");
		return 1;
	}
	
	int ret = 1; 
	int iResult;
	SOCKET sock = INVALID_SOCKET;
	sockaddr_in sockAddr = {0};
	char target_ip[50] = ""; 
	unsigned short target_port = 0;

	strncpy(target_ip, argv[1], sizeof(target_ip) - 1);
	target_port = (unsigned short)atoi(argv[2]);

	printf("[ ip=%s, port=%u ]\n", target_ip, target_port);

	// socketオープン
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if ( sock == INVALID_SOCKET ) {
		fprintf(stderr, "[ERROR] socket failed with error: %d\n", ::WSAGetLastError());
		goto EXIT;
	}

	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons(target_port);
	sockAddr.sin_addr.s_addr = inet_addr(target_ip);

	iResult = connect(sock, (struct sockaddr*)&sockAddr, sizeof(sockAddr));
	if ( iResult == SOCKET_ERROR ) {
		fprintf(stderr, "[ERROR] connect failed with error: %d\n", ::WSAGetLastError());
		goto EXIT;
	}

	// Send a datagram to the receiver
	iResult = send(sock, (const char *)pBytes, dwSize, 0);
	if ( iResult == SOCKET_ERROR ) {
		fprintf(stderr, "[ERROR] sendto failed with error: %d\n", ::WSAGetLastError());
		goto EXIT;
	}

	ret = 0;

EXIT:
	// socketクローズ
	if ( sock != INVALID_SOCKET ) {
		closesocket(sock);
	}
	if ( pBytes ) {
		::GlobalFree( (HGLOBAL)pBytes );
	}

	// WinSock2終了
	::WSACleanup();

	return ret;
}

