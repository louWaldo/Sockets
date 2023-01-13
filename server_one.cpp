#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <sys/wait.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>


struct Argument2
{
    char c;
    int dec;
    std::string * compPtr;
    int maxBits;
    std::string code;
    int freq;
};

struct Argument1
{
    char c;
    int dec;
    std::string code;
};

int findMax(Argument1 arr[], int size)
{
    int result = 0;
    for (int i = 0; i < size; i++)
    {
        if (arr[i].dec > result)
        {
            result = arr[i].dec;
        }
    }
    return result;
}

//findBIts takes largest number from input and determines bit length of each compressed character
int findBits(int maxVal)
{
    int result;
    result = ceil(log2(maxVal));
    return result;
}

std::vector<int> toBin(int maxBits, int inVal)
{
    std::vector<int> result;
    for (int i = 0; i < maxBits; i++)
    {
        int temp = inVal % 2;
        result.push_back(temp);
        inVal = inVal / 2;
    }
    reverse(result.begin(), result.end());
    return result;
}

std::vector<std::string> toString(std::vector<std::vector < int>> v, int bits)
{
    std::vector<std::string> result;
    for (int i = 0; i < v.size(); i++)
    {
        std::string temp = "";
        for (int j = 0; j < bits; j++)
        {
            temp += (v[i][j] + '0');
        }
        result.push_back(temp);
    }
    return result;
}

void fireman(int)
{
    //waitpid() --> different from wait in the sense that you can 
    //waitpid(-1 --> any child doesnt matter, status NULL default, WNOHANG --> with no hangout)
    //WNOHANG --> cannot suspend execution
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main(int argc, char *argv[])
{
    /*
    input alphabet information
    */
    int n;
    std::cin >> n;
    std::cin.ignore();  //ignores all whitspace and proceeds to next line
    Argument1 *codes = new Argument1[n];
    for (int i = 0; i < n; i++)
    {
        Argument1 temp;
        std::string tempStr;

        getline(std::cin, tempStr);
        char tchar = tempStr[0];
        std::string sub = "";
        sub += tempStr.substr(2);
        int deci = stoi(sub);
        temp.dec = deci;
        temp.c = tchar;

        codes[i] = temp;
    }

    /*
    calculate number of bits for fixed length codes
    */
    int maxVal = findMax(codes, n);
    int bits = findBits(maxVal);

    std::vector<std::vector < int>> intBin;
    for (int i = 0; i < n; i++)
    {
        std::vector<int> temp = toBin(bits, codes[i].dec);
        intBin.push_back(temp);
    }
    std::vector<std::string> stringBin = toString(intBin, bits);

    //saving information for alphabet
    std::vector<std::pair<std::string, char>> language;
    for (int i = 0; i < n; i++)
    {
        std::pair<std::string, char> temp;
        temp.first = stringBin[i];
        temp.second = codes[i].c;
        language.push_back(temp);
    }

    //two socket file descriptors
    int sockfd;
    int newsockfd;

    int portno;
    int clilen;
    char buffer[256];

    //two sockAdresssing structures instead of one (one for server, one for client)
    struct sockaddr_in serverAdress;
    struct sockaddr_in clientAdress;
    int readWrite;

    //as mentioned above, only sending two arguments
    if (argc < 2)
    {
        printf("no port number was provided\n");
    }

    //creating socket as we did in the clint.cpp
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("error opening the socket\n");
    }
    //initializing server adress with nul characters
    bzero((char*) &serverAdress, sizeof(serverAdress));

    //set port number to argument received
    portno = atoi(argv[1]);

    //same as client
    serverAdress.sin_family = AF_INET;

    //not same as client. INADDR_ANY allows this process to receive requests from ANY adress avaliable to the user
    //now we dont need to have a list of users planning to send requests to this server
    serverAdress.sin_addr.s_addr = INADDR_ANY;

    //same as client
    serverAdress.sin_port = htons(portno);

    //on client side, we used connect()
    //bind() --> attatches port that we're using to this particular process
    //computer now knows this portno is being used by this particular process
    int bindStatus = bind(sockfd, (struct sockaddr *) &serverAdress, sizeof(serverAdress));

    if (bindStatus < 0)
    {
        printf("error in binding, possible that process is currently using this port and youre trying to use it again \n");
    }

    //listen() --> tells you number of concurrent requests that can be answered 
    //5 is arbitrary in this case
    listen(sockfd, 5);
    clilen = sizeof(clientAdress);

    newsockfd = accept(sockfd,
        (struct sockaddr *) &clientAdress,
        (socklen_t*) &clilen);

    if (newsockfd < 0)
    {
        printf("error from accept()\n");
    }
    bzero(buffer, 256);

    int value = bits;

    //server recieving request from client
    if (readWrite = read(newsockfd, &value, sizeof(int)))
    {
        if (readWrite < 0)
        {
            printf("error reading from the socket\n");
        }

        readWrite = write(newsockfd, &bits, sizeof(int));
        if (readWrite < 0)
        {
            printf("ERROR in writing to the socket\n");
        }

        signal(SIGCHLD, fireman);
        while (true)
        {
            newsockfd = accept(sockfd,
                (struct sockaddr *) &clientAdress,
                (socklen_t*) &clilen);

            if (fork() == 0)
            {
                if (newsockfd < 0)
                {
                    printf("error from accept()\n");
                }
                bzero(buffer, 256);

                int value = bits;

                if (readWrite = read(newsockfd, buffer, 256))
                {

                    char decryptChar;
                    std::string buffString = "";
                    for (int i = 0; i < bits; i++)
                    {
                        buffString += buffer[i];
                    }

                    for (int i = 0; i < language.size(); i++)
                    {
                        if (buffString == language[i].first)
                        {
                            decryptChar = language[i].second;
                        }
                    }
                    //std::cout << "I'm sending character: " << decryptChar << std::endl;
                    readWrite = write(newsockfd, &decryptChar, sizeof(char));
                    if (readWrite < 0)
                    {
                        printf("ERROR in writing to the socket\n");
                    }
                }

                //note that we are only closing newsocket in child process
                close(newsockfd);
                _exit(0);
            }
            std::cin.get();
        }
    }
    close(sockfd);
}
