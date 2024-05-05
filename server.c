#include "commons.h"

// Maximum number of pending connections
#define MAXPENDING 10

// Value of pi
#define M_PI 3.14159265358979323846

// Distance in meters traveled
#define METERS_TRAVELED 400

// Number of seconds to wait
#define SECONDS_WAIT 2

// Structure to store server coordinates
Coordinate coordServ = {-19.9227,-43.9451};

/**
 * Prints the server-client menu options.
 * The menu displays the available options for the server-client interaction.
 */
void printServerClient() {
  printf("-----------------------------------\n");
  printf("| $ Corrida disponível            |\n");
  printf("| $ 0 - Recusar                   |\n");
  printf("| $ 1 - Aceitar                   |\n");
  printf("| $                               |\n");
  printf("-----------------------------------\n");
}

/**
 * Prints the server waiting message.
 * 
 * This function prints a message indicating that the server is waiting for a request.
 * If the driver has arrived, an additional message indicating that the driver has arrived is printed.
 * 
 * @param driverArrived A flag indicating whether the driver has arrived (1) or not (0).
 */
void printServerWaiting(int driverArrived) {
  printf("-----------------------------------\n");
  driverArrived ? 
    (printf("| $ O motorista chegou!           |\n")) 
    : 0; 
  printf("| $ Aguardando solicitação.       |\n");
  printf("| $                               |\n");
  printf("-----------------------------------\n");
}

/**
 * Calculates the number of places in a given integer.
 * A place is defined as a digit in the integer.
 *
 * @param n The integer for which the number of places needs to be calculated.
 * @return The number of places in the given integer.
 */
int numPlaces (int n) {
  int r = 1;
  while (n > 9) {
    n /= 10;
    r++;
  }
  return r;
}

/**
 * Calculates the distance between two points on the Earth's surface using the Haversine formula.
 *
 * @param lat1 The latitude of the first point in degrees.
 * @param lon1 The longitude of the first point in degrees.
 * @param lat2 The latitude of the second point in degrees.
 * @param lon2 The longitude of the second point in degrees.
 * @return The distance between the two points in kilometers.
 */
double haversine(double lat1, double lon1, double lat2, double lon2) {
  // distance between latitudes
  // and longitudes
  double dLat = (lat2 - lat1) * M_PI / 180.0;
  double dLon = (lon2 - lon1) * M_PI / 180.0;

  // convert to radians
  lat1 = (lat1) * M_PI / 180.0;
  lat2 = (lat2) * M_PI / 180.0;

  // apply formulae
  double a = pow(sin(dLat / 2), 2) + pow(sin(dLon / 2), 2) * cos(lat1) * cos(lat2);
  double rad = 6371;
  double c = 2 * asin(sqrt(a));
  return rad * c;
}

/**
 * Handles the TCP client connection.
 *
 * @param clientSocket The socket descriptor for the client connection.
 * @return Returns 1 if the driver arrived, 0 otherwise.
 */
int handleTCPClient(int clientSocket) {
  int driverArrived = 0;
  Coordinate clientCoordinates;

  // Receive message from client
  char message[MESSAGE_SIZE];
  ssize_t numBytesReceived = recv(clientSocket, message, sizeof(message), 0);
  sscanf(message, "(%lf, %lf)", &clientCoordinates.latitude, &clientCoordinates.longitude);
  if (numBytesReceived < 0) {
    exitWithSystemMessage("recv() failed");
  }

  
  int acceptRide = 0;
  printServerClient();
  scanf("%d", &acceptRide);
  if(acceptRide == 0) {
    ssize_t numBytesSent = send(clientSocket, "NO_DRIVER_FOUND", sizeof("NO_DRIVER_FOUND"), 0);
        
    if (numBytesSent < 0)
      exitWithSystemMessage("send() failed");
  } else {
    int dist = round(haversine(clientCoordinates.latitude, clientCoordinates.longitude, coordServ.latitude, coordServ.longitude) * 1000);
    
    // Send distance updates to client until driver arrives
    while (1) { // 0 indicates end of stream
      char distanceMessage[numPlaces(dist) + 1];
      sprintf(distanceMessage, "%d", dist);

      // Send message to client
      ssize_t numBytesSent = -1;
      if (dist > 0) {
        numBytesSent = send(clientSocket, distanceMessage, sizeof(distanceMessage), 0);
      } else {
        numBytesSent = send(clientSocket, "DRIVER_ARRIVED", sizeof("DRIVER_ARRIVED"), 0);
        driverArrived = 1;
        break;
      }

      if (numBytesSent < 0)
        exitWithSystemMessage("send() failed");
    
      dist -= METERS_TRAVELED;
      sleep(SECONDS_WAIT);
    }
  }

  close(clientSocket); // Close client socket
  return driverArrived;
}

/**
 * Main function of the server program.
 * 
 * @param argc The number of command-line arguments.
 * @param argv An array of strings containing the command-line arguments.
 *             The first argument should be either "ipv4" or "ipv6" to specify the IP type.
 *             The second argument should be the server port number.
 * @return Returns 0 on successful execution.
 */
int main (int argc, char *argv[]) {
  if (argc != 3) {
    exitWithUserMessage("Parameter(s)", "<Server Port>");
  }

  int ipType = (strcmp(argv[1], "ipv4") == 0) ? IPV4_CODE : IPV6_CODE;
  in_port_t servPort = atoi(argv[2]); // First arg: local port

  // Construct local address structure
  ServerAddress serverAddress;
  socklen_t serverAddressLen;
  void* addrPtr;

  buildServerAddress(ipType, servPort, &serverAddress, &serverAddressLen, &addrPtr);

  // Create socket for incoming connections
  int serverSock; // Socket descriptor for server
  if ((serverSock = socket((ipType == IPV4_CODE) ? (AF_INET) : (AF_INET6), SOCK_STREAM, IPPROTO_TCP)) < 0) {
    exitWithSystemMessage("socket() failed");
  }
  

  // Bind to the local address
  if (bind(serverSock, (struct sockaddr *) &serverAddress, serverAddressLen) < 0) {
    exitWithSystemMessage("bind() failed");
  }

  // Mark the socket so it will listen for incoming connections
  if (listen(serverSock, MAXPENDING) < 0) {
    exitWithSystemMessage("listen() failed");
  }

  printServerWaiting(0);
  while (1) {
    union {
      struct sockaddr_in6 IPV6; // IPv6 address
      struct sockaddr_in IPV4; // IPv4 address
    } clientAddress;
    socklen_t clientAddressLen = sizeof(clientAddress); // Set length of client address structure

    // Wait for a client to connect
    int clientSock = accept(serverSock, (struct sockaddr *) &clientAddress, &clientAddressLen);
    if (clientSock < 0) {
      exitWithSystemMessage("accept() failed");
    }

    // Call function to handle client
    int driverArrived = handleTCPClient(clientSock);
    printServerWaiting(driverArrived);
  }

  return 0;
}