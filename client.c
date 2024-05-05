#include "commons.h"

// Define the buffer size for the client
#define BUFFER_SIZE 32

// Initialize the client coordinates with specific values
Coordinate clientCoordinates = {-19.926639241000448, -43.94068052574999};

/**
 * Prints the client menu message.
 * 
 * This function prints a message indicating the options the user can take in order to interact with the server.
 * If the driver was not found, an additional message indicating that the driver was not found is printed.
 * 
 * @param hasAdditionalInfo A flag indicating whether there ia an additional message (1) or not (0).
 * @param additionalInfo The additional message to be printed.
 */
void printClientMenu(int hasAdditionalInfo, char *additionalInfo) {
  printf("-------------------------------------\n");
  hasAdditionalInfo ? 
    (printf("| $ %s|\n", additionalInfo)) 
    : 0; 
  printf("| $ 0 - Sair                        |\n");
  printf("| $ 1 - Solicitar Corrida           |\n");
  printf("|                                   |\n");
  printf("-------------------------------------\n");
}

/**
 * @brief Handles the TCP server connection and communication with the server.
 *
 * This function establishes a TCP connection with the server specified by the IP address and port number.
 * It sends the client's coordinates to the server and receives messages from the server.
 * The function continues to run until the user chooses to end the program or the driver arrives.
 *
 * @param ipType The IP address type (IPv4 or IPv6).
 * @param servPort The server port number.
 * @param ipAddress The server IP address.
 */
void handleTCPServer(int ipType, int servPort, char *ipAddress) {
  int endsProgram = -1;
  int additionalInfo = 0;
  while(endsProgram != 0) {
    printClientMenu(additionalInfo, "Não foi encontrado um motorista.");
    scanf("%d", &endsProgram);

    if(endsProgram == 0) break;

    // Create a reliable, stream socket using TCP
    int sock = socket((ipType == IPV4_CODE) ? (AF_INET) : (AF_INET6), SOCK_STREAM, IPPROTO_TCP);
    if(sock == -1) 
      exitWithSystemMessage("socket() failed");

    // Construct the server address structure
    ServerAddress serverAddress;
    socklen_t serverAddressLen;
    void* addrPtr;

    buildServerAddress(ipType, servPort, &serverAddress, &serverAddressLen, &addrPtr);

    // Converts the string representation of the server’s address into a 32-bit binary representation
    int returnValue = inet_pton((ipType == IPV4_CODE) ? (AF_INET) : (AF_INET6), ipAddress, addrPtr); 
    if(returnValue == 0)
      exitWithUserMessage("inet_pton() failed", "invalid address string");
    else if(returnValue < 0)
      exitWithSystemMessage("inet_pton() failed"); 

    // Establish the connection to the server
    if(connect(sock, (struct sockaddr *) &serverAddress, serverAddressLen) == -1)
      exitWithSystemMessage("connect() failed");

    char message[MESSAGE_SIZE]; 
    sprintf(message, "(%lf, %lf)", clientCoordinates.latitude, clientCoordinates.longitude);

    ssize_t numBytes = send(sock, message, sizeof(message), 0);
    if (numBytes < 0)
      exitWithSystemMessage("send() failed");
    else if (numBytes != sizeof(message))
      exitWithUserMessage("send()", "sent unexpected number of bytes");

    int printLine = 1;

    char buffer[MESSAGE_SIZE];
    while(1) {
      memset(buffer, 0, sizeof(buffer));
      numBytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
      if(numBytes < 0)
        exitWithSystemMessage("recv() failed");
      else if(numBytes == 0)
        exitWithUserMessage("recv()", "connection closed prematurely");
        
      buffer[numBytes] = '\0';

      if(strcmp(buffer, "NO_DRIVER_FOUND") == 0) {
        additionalInfo = 1;
        break;
      } else if (strcmp(buffer, "DRIVER_ARRIVED") == 0) {
        printf("| $ O motorista chegou.           |\n");
        printf("| $ <Encerrar programa >          |\n");
        printf("-----------------------------------\n");

        endsProgram = 0;
        break;
      } else {
        if(printLine)  {
          printf("-----------------------------------\n");
          printLine = 0;
        }
        printf("| $ Motorista a %sm              |\n", buffer);
      }
    }
    close(sock);
  }
}

/**
 * @brief The main function of the client program.
 *
 * This function is the entry point of the client program. It takes command line arguments
 * for IP type, IP address, and port number. It then calls the handleTCPServer function to
 * establish a TCP connection with the server.
 *
 * @param argc The number of command line arguments.
 * @param argv An array of strings containing the command line arguments.
 * @return 0 on successful execution.
 */
int main(int argc, char *argv[]) {
  if(argc != 4)
    exitWithSystemMessage("Parameters: <IP_type> <IP_address)> <port>\n");

  int ipType = (strcmp(argv[1], "ipv4") == 0) ? IPV4_CODE : IPV6_CODE;
  char *ipAddress = argv[2];
  int servPort = atoi(argv[3]);

  handleTCPServer(ipType, servPort, ipAddress);
  
  exit(0);
  return 0;
}