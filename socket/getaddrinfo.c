/** Convert the domain name into a list of ip addresses
 *  Look up the domain name from an ip address.
 */
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#ifndef NI_MAXHOST
#define NI_MAXHOST 1025
#endif

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Usage: getaddrinfo domain port\n");
    return 1;
  }
  struct addrinfo *result;
  struct addrinfo *res;
  int error;

  /* resolve the domain name into a list of addresses */
  error = getaddrinfo(argv[1], argv[2], NULL, &result);
  if (error != 0) {
    if (error == EAI_SYSTEM) {
      perror("getaddrinfo");
    } else {
      fprintf(stderr, "error in getaddrinfo: %s\n", gai_strerror(error));
    }
    exit(EXIT_FAILURE);
  }

  /* loop over all returned results and do inverse lookup */
  for (res = result; res != NULL; res = res->ai_next) {
    char hostname[NI_MAXHOST];
    char servicename[NI_MAXHOST];
    char ipaddress[INET6_ADDRSTRLEN];

    error = getnameinfo(res->ai_addr, res->ai_addrlen, hostname, NI_MAXHOST, servicename, NI_MAXHOST, 0);
    if (error != 0) {
      fprintf(stderr, "error in getnameinfo: %s\n", gai_strerror(error));
      continue;
    }
    /* convert the sockaddr to ip string */
    switch(res->ai_addr->sa_family) {
    case AF_INET: {
      struct sockaddr_in *addr_in = (struct sockaddr_in *)res->ai_addr;
      inet_ntop(AF_INET, &(addr_in->sin_addr), ipaddress, INET6_ADDRSTRLEN);
      break;
    }
    case AF_INET6: {
      struct sockaddr_in6 *addr_in = (struct sockaddr_in6 *)res->ai_addr;
      inet_ntop(AF_INET6, &(addr_in->sin6_addr), ipaddress, INET6_ADDRSTRLEN);
      break;
    }
    default: break;
    }
    if (*hostname != '\0')
      printf("hostname: %s, servicename: %s, ip: %s\n", hostname, servicename, ipaddress);
  }
  freeaddrinfo(result);
  return 0;
}
