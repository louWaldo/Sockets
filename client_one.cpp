#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <string>
#include <netdb.h>
#include <string>
#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>

struct outData
{
    std::string chunk;
    char decomped;
    int portnum;
    struct sockaddr_in srvrAddr;
    struct hostent * servr;
    int testing;
};

void *giveChar(void *v_ptr)
{
    outData *dataPtr = (outData*) v_ptr;
    //create socket to communicate with server
    int sockfd;
    int portno;
    int n;

    struct sockaddr_in serverAdress = dataPtr->srvrAddr;    //using sockaddr structure for internet protocol 
    struct hostent *server = dataPtr->servr;    //when you request website, you dont ask for IP adress, you ask for name. hostent lets you translate name into an IP adress we can use.

    std::string chunkBuf = dataPtr->chunk;
    int chunkSize = chunkBuf.length();
    char buffer[256];

    //setting portnumber
    portno = (dataPtr->portnum);

    //socket(family of internet protocol, socket type -> stream, deafult parameter)
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        printf("error opening the socket");
    }

    if (server == NULL)
    {
        //like trying to use google but mispelling gogle
        printf("error-> hostname we received from user is invalid");
    }

    //bcopy (contents here, copied to here, based on this length)
    bcopy((char*) server->h_addr,
        (char*) &serverAdress.sin_addr.s_addr,
        server->h_length);

    //setting portumber
    serverAdress.sin_port = htons(portno);

    int connectStatus = connect(sockfd, (struct sockaddr *) &serverAdress, sizeof(serverAdress));
    if (connectStatus < 0)
    {
        //if connectStatus > 0, then we have connected the socket
        printf("fucked up the connection\n");
    }
    bzero(buffer, 256);

    //translating strings to character arrays
    for (int i = 0; i < chunkSize; i++)
    {
        buffer[i] = chunkBuf[i];
    }
    //send binary chunk of symbol to the server
    n = write(sockfd, buffer, 256);
    if (n < 0)
    {
        printf("error writing to the socket\n");
    }

    //wait for decompressed character returned from server
    char result;
    n = read(sockfd, &result, sizeof(result));

    //write this info somewhere main thread can acces
    dataPtr->decomped = result;

    //finsh execution
    close(sockfd);
    return (void*) dataPtr;
}

int main(int argc, char *argv[])
{
    /*
        input compressed message
    */
    std::string compressed;
    std::cin >> compressed;

    /*
        receive number of bits per character from server
    */

    int sockfd;
    int portno;
    int n;

    struct sockaddr_in serverAdress;    //using sockaddr structure for internet protocol 
    struct hostent * server;    //when you request website, you dont ask for IP adress, you ask for name. hostent lets you translate name into an IP adress we can use.

    char buffer[256];

    //setting port number to be the second command line argument
    portno = atoi(argv[2]);

    //socket(family of internet protocol, socket type -> stream, deafult parameter)
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        printf("error opening the socket");
    }
    //gethostbyname translates server name into an adress and storing that info into pointer hosten
    server = gethostbyname(argv[1]);

    if (server == NULL)
    {
        //like trying to use google but mispelling gogle
        printf("error-> hostname we received from user is invalid");
    }
    //bzero initializes everything as NULL characters
    //before using character array, we need to initialize all of them as null charcacters
    bzero((char*) &serverAdress, sizeof(serverAdress));

    //setting the family to be the same as the socket (internet family of protocols)
    serverAdress.sin_family = AF_INET;

    //bcopy (contents here, copied to here, based on this length)
    bcopy((char*) server->h_addr,
        (char*) &serverAdress.sin_addr.s_addr,
        server->h_length);

    //assigniming port number of server adress
    //htons makes sure we have a similar language when representing our integers
    //garantees that different machines on different platforms understand what they're saying through this intermediary language provided by htons
    serverAdress.sin_port = htons(portno);

    int connectStatus = connect(sockfd, (struct sockaddr *) &serverAdress, sizeof(serverAdress));
    if (connectStatus < 0)
    {
        //if connectStatus > 0, then we have connected the socket
        printf("connection failed\n");
    }
    int value = 0;

    //write(sockfd, buffer, sizeof(buffer))
    n = write(sockfd, &value, sizeof(int));
    if (n < 0)
    {
        printf("error writing to the socket\n");
    }

    //read has essentilly same parameters as write
    bzero(buffer, 256);
    n = read(sockfd, &value, sizeof(int));  //note only reading 255 bytes
    if (n < 0)
    {
        printf("error reading from the socket");
    }

    //printf("bits received from server: %d\n", value);
    int bits = value;

    //splitting the compressed message into bitSize "chunks" that eventually get sent to the server 
    std::vector<std::string > chunks;
    for (int i = 0; i < compressed.size(); i += bits)
    {
        std::string chunk = compressed.substr(i, bits);
        chunks.push_back(chunk);
    }
    int totalChar = chunks.size();

    /*
        create (m) threads that each return a character, m = #characters in decompressed (using giveChar)
    */
    
    //creating the structures that will bec p[assed into the threads]
    outData *outArr = new outData[totalChar];
    for (int i = 0; i < totalChar; i++)
    {
        outArr[i].chunk = chunks[i];
        outArr[i].portnum = portno;
        outArr[i].srvrAddr = serverAdress;
        outArr[i].servr = server;
    }

    pthread_t *threads = new pthread_t[totalChar];
    for (int i = 0; i < totalChar; i++)
    {
        outData *temp = new outData;
        *temp = outArr[i];
        pthread_create(&threads[i], NULL, giveChar, (void*) temp);
    }
    
    //last vec will be the vector of structures containing the final message
    std::vector<outData> lastVec;
    for (int i = 0; i < totalChar; i++)
    {
        outData *temp = new outData;
        pthread_join(threads[i], (void **) &temp);
        lastVec.push_back(*temp);
        delete temp;
    }


    /*
        print decompressed message
    */
    std::cout << "Decompressed message: ";
    for (int i = 0; i < totalChar; i++)
    {
        std::cout << lastVec[i].decomped;
    }

    close(sockfd);
    return 0;
}
