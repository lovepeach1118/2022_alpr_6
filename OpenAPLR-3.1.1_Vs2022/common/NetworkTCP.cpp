//------------------------------------------------------------------------------------------------
// File: NetworkTCP.cpp
// Project: LG Exec Ed Program
// Versions:
// 1.0 April 2017 - initial version
// Provides the ability to send and recvive TCP byte streams for both Window platforms
//------------------------------------------------------------------------------------------------
#include <iostream>
#include <new>
#include <stdio.h>
#include <string.h>
#include "NetworkTCP.h"

#include "../aes256/include/aes256.hpp"
#include <mstcpip.h>

using namespace std;

#pragma comment (lib, "../x64/Release/aes256.lib")

using namespace std;
//-----------------------------------------------------------------
// OpenTCPListenPort - Creates a Listen TCP port to accept
// connection requests
//-----------------------------------------------------------------
TTcpListenPort *OpenTcpListenPort(short localport)
{
  TTcpListenPort *TcpListenPort;
  struct sockaddr_in myaddr;

  TcpListenPort= new (std::nothrow) TTcpListenPort;  
  
  if (TcpListenPort==NULL)
     {
      fprintf(stderr, "TUdpPort memory allocation failed\n");
      return(NULL);
     }
  TcpListenPort->ListenFd=BAD_SOCKET_FD;

  WSADATA wsaData;
  int     iResult;
  iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (iResult != 0) 
    {
     delete TcpListenPort;
     printf("WSAStartup failed: %d\n", iResult);
     return(NULL);
    }

  // create a socket
  if ((TcpListenPort->ListenFd= socket(AF_INET, SOCK_STREAM, 0)) == BAD_SOCKET_FD)
     {
      CloseTcpListenPort(&TcpListenPort);
      perror("socket failed");
      return(NULL);  
     }
  int option = 1; 

   if(setsockopt(TcpListenPort->ListenFd,SOL_SOCKET,SO_REUSEADDR,(char*)&option,sizeof(option)) < 0)
     {
      CloseTcpListenPort(&TcpListenPort);
      perror("setsockopt failed");
      return(NULL);
     }

  // bind it to all local addresses and pick any port number
  memset((char *)&myaddr, 0, sizeof(myaddr));
  myaddr.sin_family = AF_INET;
  myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  myaddr.sin_port = htons(localport);

  if (bind(TcpListenPort->ListenFd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0)
    {
      CloseTcpListenPort(&TcpListenPort);
      perror("bind failed");
      return(NULL); 
    }
   
 
  if (listen(TcpListenPort->ListenFd,5)< 0)
  {
      CloseTcpListenPort(&TcpListenPort);
      perror("bind failed");
      return(NULL);	  
  }
#if 0
  ULONG NonBlock = 1;
  if (ioctlsocket(TcpListenPort->ListenFd, FIONBIO, &NonBlock) == SOCKET_ERROR)
  {
      perror("ioctlsocket() failed with error");
      return(NULL);
  }
#endif
  
  return(TcpListenPort);
}
//-----------------------------------------------------------------
// END OpenTCPListenPort
//-----------------------------------------------------------------
//-----------------------------------------------------------------
// CloseTcpListenPort - Closes the specified TCP listen port
//-----------------------------------------------------------------
void CloseTcpListenPort(TTcpListenPort **TcpListenPort)
{
  if ((*TcpListenPort)==NULL) return;
  if ((*TcpListenPort)->ListenFd!=BAD_SOCKET_FD)  
     {
      CLOSE_SOCKET((*TcpListenPort)->ListenFd);
      (*TcpListenPort)->ListenFd=BAD_SOCKET_FD;
     }
   delete (*TcpListenPort);
  (*TcpListenPort)=NULL;

   WSACleanup();

}
//-----------------------------------------------------------------
// END CloseTcpListenPort
//-----------------------------------------------------------------
//-----------------------------------------------------------------
// AcceptTcpConnection -Accepts a TCP Connection request from a 
// Listening port
//-----------------------------------------------------------------
TTcpConnectedPort *AcceptTcpConnection(TTcpListenPort *TcpListenPort, 
                       struct sockaddr_in *cli_addr,socklen_t *clilen)
{
  TTcpConnectedPort *TcpConnectedPort;

  TcpConnectedPort= new (std::nothrow) TTcpConnectedPort;  
  
  if (TcpConnectedPort==NULL)
     {
      fprintf(stderr, "TUdpPort memory allocation failed\n");
      return(NULL);
     }
  TcpConnectedPort->ConnectedFd= accept(TcpListenPort->ListenFd,
                      (struct sockaddr *) cli_addr,clilen);
					  
  if (TcpConnectedPort->ConnectedFd== BAD_SOCKET_FD) 
  {
	perror("ERROR on accept");
	delete TcpConnectedPort;
	return NULL;
  }
#if 0
 int bufsize = 200 * 1024;
 if (setsockopt(TcpConnectedPort->ConnectedFd, SOL_SOCKET, 
                 SO_RCVBUF, (char *)&bufsize, sizeof(bufsize)) == -1)
	{
         CloseTcpConnectedPort(&TcpConnectedPort);
         perror("setsockopt SO_SNDBUF failed");
         return(NULL);
	}
 if (setsockopt(TcpConnectedPort->ConnectedFd, SOL_SOCKET, 
                 SO_SNDBUF, (char *)&bufsize, sizeof(bufsize)) == -1)
	{
         CloseTcpConnectedPort(&TcpConnectedPort);
         perror("setsockopt SO_SNDBUF failed");
         return(NULL);
	}
#endif

#if 0
 ULONG NonBlock = 1;
  if (ioctlsocket(TcpConnectedPort->ConnectedFd, FIONBIO, &NonBlock) == SOCKET_ERROR)
 {
     printf("ioctlsocket(FIONBIO) failed with error %d\n", WSAGetLastError());
     CloseTcpConnectedPort(&TcpConnectedPort);
     return(NULL);
 }
#endif
 else
     printf("ioctlsocket(FIONBIO) is OK!\n");

#if (defined(TCP_KEEPALIVE) && (TCP_KEEPALIVE == TRUE))
  DWORD dwError = 0L;
  tcp_keepalive sKA_Settings = { 0 }, sReturned = { 0 };
  sKA_Settings.onoff = 1;
  sKA_Settings.keepalivetime = 50;
  sKA_Settings.keepaliveinterval = 50;
  DWORD dwBytes;

  if (WSAIoctl(TcpConnectedPort->ConnectedFd, SIO_KEEPALIVE_VALS, &sKA_Settings, sizeof(sKA_Settings),
      &sReturned, sizeof(sReturned), &dwBytes, NULL, NULL) != 0)
  {
      dwError = WSAGetLastError();
      printf("SIO_KEEPALIVE_VALS result : %dn", dwError);
  }
#endif
 return TcpConnectedPort;
}
//-----------------------------------------------------------------
// END AcceptTcpConnection
//-----------------------------------------------------------------
//-----------------------------------------------------------------
// OpenTCPConnection - Creates a TCP Connection to a TCP port
// accepting connection requests
//-----------------------------------------------------------------
TTcpConnectedPort *OpenTcpConnection(const char *remotehostname, const char * remoteportno)
{
  TTcpConnectedPort *TcpConnectedPort;
  int                s;
  struct addrinfo   hints;
  struct addrinfo   *result = NULL;

  TcpConnectedPort= new (std::nothrow) TTcpConnectedPort;  
  
  if (TcpConnectedPort==NULL)
     {
      fprintf(stderr, "TUdpPort memory allocation failed\n");
      return(NULL);
     }
  TcpConnectedPort->ConnectedFd=BAD_SOCKET_FD;

  WSADATA wsaData;
  int     iResult;
  iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (iResult != 0) 
    {
     delete TcpConnectedPort;
     printf("WSAStartup failed: %d\n", iResult);
     return(NULL);
    }

  // create a socket
   memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  
  s = getaddrinfo(remotehostname, remoteportno, &hints, &result);
  if (s != 0) 
    {
	  delete TcpConnectedPort;
      fprintf(stderr, "getaddrinfo: Failed\n");
      return(NULL);
    }
  if ( result==NULL)
    {
	  delete TcpConnectedPort;
      fprintf(stderr, "getaddrinfo: Failed\n");
      return(NULL);
    }
  if ((TcpConnectedPort->ConnectedFd= socket(AF_INET, SOCK_STREAM, 0)) == BAD_SOCKET_FD)
     {
      CloseTcpConnectedPort(&TcpConnectedPort);
	  freeaddrinfo(result);
      perror("socket failed");
      return(NULL);  
     }

  int bufsize = 200 * 1024;
  if (setsockopt(TcpConnectedPort->ConnectedFd, SOL_SOCKET,
                 SO_SNDBUF, (char *)&bufsize, sizeof(bufsize)) == -1)
	{
         CloseTcpConnectedPort(&TcpConnectedPort);
         perror("setsockopt SO_SNDBUF failed");
         return(NULL);
	}
  if (setsockopt(TcpConnectedPort->ConnectedFd, SOL_SOCKET, 
                 SO_RCVBUF, (char *)&bufsize, sizeof(bufsize)) == -1)
	{
         CloseTcpConnectedPort(&TcpConnectedPort);
         perror("setsockopt SO_SNDBUF failed");
         return(NULL);
	}
  
	
  if (connect(TcpConnectedPort->ConnectedFd,result->ai_addr,(int)result->ai_addrlen) < 0) 
          {
	    CloseTcpConnectedPort(&TcpConnectedPort);
	    freeaddrinfo(result);
            perror("connect failed");
            return(NULL);
	  }
  freeaddrinfo(result);	 

#if (defined(TCP_KEEPALIVE) && (TCP_KEEPALIVE == TRUE))
  DWORD dwError = 0L;
  tcp_keepalive sKA_Settings = { 0 }, sReturned = { 0 };
  sKA_Settings.onoff = 1;
  sKA_Settings.keepalivetime = 50;
  sKA_Settings.keepaliveinterval = 50;
  DWORD dwBytes;

  if (WSAIoctl(TcpConnectedPort->ConnectedFd, SIO_KEEPALIVE_VALS, &sKA_Settings, sizeof(sKA_Settings),
      &sReturned, sizeof(sReturned), &dwBytes, NULL, NULL) != 0)
  {
      dwError = WSAGetLastError();
      printf("SIO_KEEPALIVE_VALS result : %dn", dwError);
  }
#endif
  return(TcpConnectedPort);
}
//-----------------------------------------------------------------
// END OpenTcpConnection
//-----------------------------------------------------------------
//-----------------------------------------------------------------
// CloseTcpConnectedPort - Closes the specified TCP connected port
//-----------------------------------------------------------------
void CloseTcpConnectedPort(TTcpConnectedPort **TcpConnectedPort)
{
  if ((*TcpConnectedPort)==NULL) return;
  if ((*TcpConnectedPort)->ConnectedFd!=BAD_SOCKET_FD)  
     {
      CLOSE_SOCKET((*TcpConnectedPort)->ConnectedFd);
      (*TcpConnectedPort)->ConnectedFd=BAD_SOCKET_FD;
     }
   delete (*TcpConnectedPort);
  (*TcpConnectedPort)=NULL;

   WSACleanup();

}
//-----------------------------------------------------------------
// END CloseTcpListenPort
//-----------------------------------------------------------------
//-----------------------------------------------------------------
// BytesAvailableTcp - Reads the bytes available 
//-----------------------------------------------------------------
ssize_t BytesAvailableTcp(TTcpConnectedPort* TcpConnectedPort)
{
    unsigned long n = -1;;
    
    if (ioctlsocket(TcpConnectedPort->ConnectedFd, FIONREAD, &n) <0)
    {
        printf("BytesAvailableTcp: error %d\n", WSAGetLastError());
        return -1;
    }
  
    return((ssize_t)n);
}
//-----------------------------------------------------------------
// END BytesAvailableTcp 
//-----------------------------------------------------------------
//-----------------------------------------------------------------
// ReadDataTcp - Reads the specified amount TCP data 
//-----------------------------------------------------------------
#if (defined(AES_ENCRYPTION) && (AES_ENCRYPTION == TRUE))
ssize_t ReadDataTcp(TTcpConnectedPort *TcpConnectedPort,unsigned char *data, size_t length)
{
    ssize_t bytes;
    ByteArray aesKey(AES_KEY_SIZE);
    ByteArray descryptData;

    int encryptedLength = (length % AES_BLOCK_SIZE == 0) 
                            ? ((length / AES_BLOCK_SIZE) * AES_BLOCK_SIZE + 1) 
                            : ((length / AES_BLOCK_SIZE + 1) * AES_BLOCK_SIZE + 1);
    unsigned char receiveData[RECEIVE_DATA_SIZE] = { 0, };
    
    for (int i = 0; i < AES_KEY_SIZE; i++) 
    {
        aesKey[i] = aesPrivateKey[i];
    }
    //cout << " length : " << length << " encryp : " << encryptedLength << endl;

    for (size_t i = 0; i < encryptedLength; i += bytes)
    {
        bytes = recv(TcpConnectedPort->ConnectedFd, (char*)(receiveData + i), (int)(encryptedLength - i), 0);
        if (bytes == -1) {
            //char buf[256];
            //strerror_s(buf, sizeof(buf), errno);
            //cout << "Recv failed : " << bytes << " error : " << buf << endl;
            return -1;
        }
    }
    
    Aes256::decrypt(aesKey, receiveData, encryptedLength, descryptData);
    
    for (int i = 0; i < descryptData.size(); i++)
    {
        data[i] = descryptData.at(i);
    }
    data[descryptData.size()] = 0;
    return(length);
}
#else
ssize_t ReadDataTcp(TTcpConnectedPort* TcpConnectedPort, unsigned char* data, size_t length)
{
    ssize_t bytes;

    for (size_t i = 0; i < length; i += bytes)
    {
        if ((bytes = recv(TcpConnectedPort->ConnectedFd, (char*)(data + i), (int)(length - i), 0)) == -1)
        {
            return (-1);
        }
    }

    return(length);
}
#endif
//-----------------------------------------------------------------
// END ReadDataTcp
//-----------------------------------------------------------------
//-----------------------------------------------------------------
// WriteDataTcp - Writes the specified amount TCP data 
//-----------------------------------------------------------------
#if (defined(AES_ENCRYPTION) && (AES_ENCRYPTION == TRUE))
ssize_t WriteDataTcp(TTcpConnectedPort *TcpConnectedPort,const unsigned char *data, size_t length)
{
    ssize_t total_bytes_written = 0;
    ssize_t bytes_written;

    ByteArray aesKey(AES_KEY_SIZE);
    ByteArray encryptedData;

    int encryptedLength = (length % AES_BLOCK_SIZE == 0) 
                            ? ((length / AES_BLOCK_SIZE) * AES_BLOCK_SIZE + 1) 
                            : ((length / AES_BLOCK_SIZE + 1) * AES_BLOCK_SIZE + 1);
    unsigned char sendData[SEND_DATA_SIZE] = { 0, };
    //cout << "Write Data : " << length << endl;
 
    for (int i = 0; i < AES_KEY_SIZE; i++) 
    {
        aesKey[i] = aesPrivateKey[i];
    }

    Aes256::encrypt(aesKey, data, length, encryptedData);
  
    for (int i = 0; i < encryptedLength; i++)
    {
        sendData[i] = encryptedData.at(i);
    }
  
    while (total_bytes_written != encryptedLength)
    {
        bytes_written = send(TcpConnectedPort->ConnectedFd,
	                            (char *)(sendData + total_bytes_written),
                                (int)(encryptedLength - total_bytes_written), 0);
        if (bytes_written == -1)
        {
            return(-1);
        }
        
        total_bytes_written += bytes_written;
    }
    
    return(length);
}
#else
ssize_t WriteDataTcp(TTcpConnectedPort* TcpConnectedPort, unsigned char* data, size_t length)
{
    ssize_t total_bytes_written = 0;
    ssize_t bytes_written;
    while (total_bytes_written != length)
    {
        bytes_written = send(TcpConnectedPort->ConnectedFd, 
            (char*)(data + total_bytes_written),
            (int)(length - total_bytes_written), 0);
        if (bytes_written == -1)
        {
            return(-1);
        }
        total_bytes_written += bytes_written;
    }

    return(total_bytes_written);
}
#endif
//-----------------------------------------------------------------
// END WriteDataTcp
//-----------------------------------------------------------------
//-----------------------------------------------------------------
// END of File
//-----------------------------------------------------------------


