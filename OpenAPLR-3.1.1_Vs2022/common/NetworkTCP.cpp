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

#pragma comment (lib, "../x64/Release/aes256.lib")

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

  ULONG NonBlock = 1;
  if (ioctlsocket(TcpListenPort->ListenFd, FIONBIO, &NonBlock) == SOCKET_ERROR)
  {
      perror("ioctlsocket() failed with error");
      return(NULL);
  }
  else
      printf("ioctlsocket() is OK!\n");
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

 ULONG NonBlock = 1;
  if (ioctlsocket(TcpConnectedPort->ConnectedFd, FIONBIO, &NonBlock) == SOCKET_ERROR)
 {
     printf("ioctlsocket(FIONBIO) failed with error %d\n", WSAGetLastError());
     CloseTcpConnectedPort(&TcpConnectedPort);
     return(NULL);
 }
 else
     printf("ioctlsocket(FIONBIO) is OK!\n");
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
#if 1
ssize_t ReadDataTcp(TTcpConnectedPort *TcpConnectedPort,unsigned char *data, size_t length)
{
 ssize_t bytes;
 ByteArray aesKey(AES_KEY_SIZE);
 //ByteArray plainData(length);
 //ByteArray encryptData;;
 ByteArray descryptData;

 int encryptedLength = (length / AES_BLOCK_SIZE + 1) * AES_BLOCK_SIZE + 1;
 unsigned char* receiveData = new unsigned char(encryptedLength);

 for (int i = 0; i < AES_KEY_SIZE; i++) {
     aesKey[i] = aesPrivateKey[i];
 }
#
 //printf("length[%d]\n", length);

 if (length == sizeof(unsigned short))
 {
     for (size_t i = 0; i < encryptedLength; i += bytes)
     {
         if ((bytes = recv(TcpConnectedPort->ConnectedFd, (char*)(receiveData + i), (int)(encryptedLength - i), 0)) == -1)
         {
             return (-1);
         }
     }
    // printf("1[%02x][%02x]\n", receiveData[0], receiveData[1]);
#if 1
     Aes256::decrypt(aesKey, receiveData, encryptedLength, descryptData);

     //printf("2[%d]\n", descryptData.size());
     data[0] = descryptData[0];
     data[1] = descryptData[1];
#endif
#if 0
     for (int i = 0; i < length; i++)
     {
         printf("%02x", descryptData[i]);
     }
#endif
 }
 else
 {
     for (size_t i = 0; i < encryptedLength; i += bytes)
     {
         if ((bytes = recv(TcpConnectedPort->ConnectedFd, (char*)(receiveData + i), (int)(encryptedLength - i), 0)) == -1)
         {
             return (-1);
         }
     }

     Aes256::decrypt(aesKey, receiveData, encryptedLength, descryptData);

     for (int i = 0; i < encryptedLength; i++)
     {
         data[i] = descryptData[i];
     }
 }

 delete receiveData;

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
    printf("data[%02x][%02x]\n", data[0], data[1]);

    return(length);
}
#endif
//-----------------------------------------------------------------
// END ReadDataTcp
//-----------------------------------------------------------------
//-----------------------------------------------------------------
// WriteDataTcp - Writes the specified amount TCP data 
//-----------------------------------------------------------------
#if 1
ssize_t WriteDataTcp(TTcpConnectedPort *TcpConnectedPort,unsigned char *data, size_t length)
{
  ssize_t total_bytes_written = 0;
  ssize_t bytes_written;

  ByteArray aesKey(AES_KEY_SIZE);
  //ByteArray plainData(length);
  ByteArray encryptedData;
  //ByteArray decryptedData;

  int encryptedLength = (length / AES_BLOCK_SIZE + 1) * AES_BLOCK_SIZE + 1;
  unsigned char* sendData = new unsigned char(encryptedLength);

  memset(sendData, 0x00, encryptedLength);
 
  for (int i = 0; i < AES_KEY_SIZE; i++) {
      aesKey[i] = aesPrivateKey[i];
  }

  Aes256::encrypt(aesKey, data, length, encryptedData);
  
  printf("plain[%d][%02x][%02x]\n", length, data[0], data[1]);
  printf("encrypt[%d][%02x][%02x]\n", encryptedLength, encryptedData[0], encryptedData[1]);

  for (int i = 0; i < encryptedLength; i++)
  {
      sendData[i] = encryptedData[i];
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

    printf("total_bytes_written[%d][%02x][%02x]\n", total_bytes_written, sendData[0], sendData[1]);

    //Aes256::decrypt(aesKey, sendData, encryptedLength, decryptedData);

    //printf("total[%d][%02x][%02x]\n", decryptedData.size(), decryptedData[0], decryptedData[1]);

    delete sendData;

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

    printf("data[%02x][%02x]\n", data[0], data[1]);

    return(total_bytes_written);
}
#endif
//-----------------------------------------------------------------
// END WriteDataTcp
//-----------------------------------------------------------------
//-----------------------------------------------------------------
// END of File
//-----------------------------------------------------------------


