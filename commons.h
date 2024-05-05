// C Standard Libraries used
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

// Socket related libraries
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/**
 * @file commons.h
 * @brief This file contains common definitions and structures used in the project.
 */

// The code representing an IPv4 address.
#define IPV4_CODE 1001

// The code representing an IPv6 address.
#define IPV6_CODE 1002

// The size of the message.
#define MESSAGE_SIZE 40

// Represents a coordinate with latitude and longitude.
typedef struct {
  double latitude; /**< The latitude value. */
  double longitude; /**< The longitude value. */
} Coordinate;

// Represents a server address that can be either IPv4 or IPv6.
typedef union {
  struct sockaddr_in6 IPV6; /**< The IPv6 address. */
  struct sockaddr_in IPV4; /**< The IPv4 address. */
} ServerAddress;

/**
 * @brief Prints an error message to stderr and exits the program.
 *
 * This function prints the given error message and detail to the standard error stream (stderr),
 * followed by a newline character, and then terminates the program with an exit status of 1.
 *
 * @param msg The error message to be printed.
 * @param detail The additional detail or explanation of the error.
 */
void exitWithUserMessage(const char *msg, const char *detail) {
  fputs(msg, stderr);
  fputs(": ", stderr);
  fputs(detail, stderr);
  fputc('\n', stderr);
  exit(1);
}

/**
 * Prints an error message corresponding to the current value of errno,
 * followed by the provided message, and exits the program with a status of 1.
 *
 * @param msg The error message to be printed.
 */
void exitWithSystemMessage(const char *msg) {
  perror(msg);
  exit(1);
}

/**
 * Builds the server address based on the IP type and port number.
 *
 * @param ipType The IP type (IPV4_CODE or IPV6_CODE).
 * @param port The port number.
 * @param serverAddress Pointer to the ServerAddress structure to be filled.
 * @param serverAddressLen Pointer to the length of the serverAddress structure.
 * @param addrPtr Pointer to the address pointer to be set.
 */
void buildServerAddress(
  int ipType, 
  int port, 
  ServerAddress *serverAddress,
  socklen_t *serverAddressLen,
  void* *addrPtr
) {
  memset(serverAddress, 0, sizeof(*serverAddress)); // Zero out structure
  switch (ipType) {
    case IPV4_CODE:
      serverAddress->IPV4.sin_family = AF_INET; // IPv4 address family
      serverAddress->IPV4.sin_addr.s_addr = htonl(INADDR_ANY); // Any incoming interface
      *addrPtr = (void*)&serverAddress->IPV4.sin_addr.s_addr;
      serverAddress->IPV4.sin_port = htons(port); // Local port
      *serverAddressLen = sizeof(serverAddress->IPV4);
      break;
    
    case IPV6_CODE:
      serverAddress->IPV6.sin6_family = AF_INET6; // IPv6 address family
      serverAddress->IPV6.sin6_addr = in6addr_any; // Any incoming interface
      *addrPtr = (void*)&serverAddress->IPV6.sin6_addr;
      serverAddress->IPV6.sin6_port = htons(port); // Local port
      *serverAddressLen = sizeof(serverAddress->IPV6);
      break;
  }
}
