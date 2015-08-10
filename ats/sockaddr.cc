#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <cassert>
#include <cstring>
#include <string>
#include <cstdlib> // For rand function.
#include <sstream>

// Convert a struct sockaddr address to a string, IPv4 and IPv6:
std::string get_ip_str(const struct sockaddr *sa) {
  char s[INET6_ADDRSTRLEN];
  switch (sa->sa_family) {
  case AF_INET:
    if (inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr), s, INET_ADDRSTRLEN) == NULL) {
      perror("Failed to print the ipv4: ");
    }
    break;
  case AF_INET6:
    if (inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr), s, INET6_ADDRSTRLEN) == NULL) {
      perror("Failed to print ipv6 address: ");
    }
    break;
  default:
    strncpy(s, "Unknown AF", INET6_ADDRSTRLEN);
    return NULL;
  }
  return std::string(s);
}

// Convert an ip string into a sockaddr for both ipv4 and ipv6.
// Return null on failure.
struct sockaddr *convertToSockaddr(const std::string &ip) {
  struct sockaddr *result = NULL;
  int retno;
  if (ip.find(".") != std::string::npos) {
    // IPv4 addresses are separated by '.'.
    result = (sockaddr *)new sockaddr_in;
    result->sa_family = AF_INET;
    retno = inet_pton(AF_INET, ip.c_str(), &((sockaddr_in *)result)->sin_addr);
  } else if (ip.find(":") != std::string::npos) {
    // ipv6 addresses are separated by ':'.
    result = (struct sockaddr *)new sockaddr_in6;
    result->sa_family = AF_INET6;
    retno = inet_pton(AF_INET6, ip.c_str(), &((sockaddr_in6 *)result)->sin6_addr);
  } else {
    std::cerr << "Invalid ip address: " << ip << "\n";
    return NULL;
  }
  if (retno == 0) {
    std::cerr << "Invalid ipv4 address: " << ip << "\n";
    delete result;
    return NULL;
  } else if (retno == -1) {
    perror("Error: ");
    delete result;
    return NULL;
  } else
    return (struct sockaddr*)result;
}

void test_ipv4_1() {
  char buffer[INET_ADDRSTRLEN];
  unsigned end = 0xffffffff / 1000;
  for (unsigned i = 0; i < end; ++i) {
    int i1 = rand() & 0xff;
    int i2 = rand() & 0xff;
    int i3 = rand() & 0xff;
    int i4 = rand() & 0xff;
    int n = sprintf(buffer, "%d.%d.%d.%d", i1, i2, i3, i4);
    std::string s(buffer, buffer + n);
    assert(get_ip_str(convertToSockaddr(s)) == s);
  }
}

// Exhaustively test all cases.
void test_ipv4() {
  std::ostringstream os;
  unsigned end = 0xffffffff / 1000;
  for (unsigned i = 0; i < end; ++i) {
    int r = rand();
    int i1 = r & 0xff;
    int i2 = (r >> 8)  & 0xff;
    int i3 = (r >> 16) & 0xff;
    int i4 = (r >> 24) & 0xff;
    os << i1 << '.' << i2 << '.' << i3 << '.' << i4;
    std::string s = os.str();
    assert(get_ip_str(convertToSockaddr(s)) == s);
    os.str(""); // Clear the buffer.
    os.clear(); // Clear the flag.
  }
  /*
  for (int i = 0; i < 256; ++i) {
    for (int j = 0; j < 256; ++j) {
      for (int k = 0; k < 256; ++k) {
	for (int l = 0; l < 256; ++l) {
	  os << i << '.' << j << '.' << k << '.' << l;
	  std::string s = os.str();
	  assert(get_ip_str(convertToSockaddr(s)) == s);
	  os.str("");
	  os.clear();
	}
      }
    }
  }
  */
}

void test_ipv6() {
  assert(get_ip_str(convertToSockaddr("2001:db8:8714:3a90::12")) == "2001:db8:8714:3a90::12");
  assert(get_ip_str(convertToSockaddr("0:0:0:0:0:0:0:5")) == "::5");
  assert(get_ip_str(convertToSockaddr("::5")) == "::5");
  assert(get_ip_str(convertToSockaddr("ABC:567:0:0:8888:9999:1111:0")) == "abc:567::8888:9999:1111:0");
  assert(get_ip_str(convertToSockaddr("2001:db8:8714:3a90::12")) == "2001:db8:8714:3a90::12");
  assert(get_ip_str(convertToSockaddr("ABC:567::8888:9999:1111:0")) == "abc:567::8888:9999:1111:0");
  
}

int main() {
  // test_ipv4();
  // test_ipv4_1();
  test_ipv6();

  std::cout << "size of struct sockaddr:" << sizeof(struct sockaddr) << "\n";
  std::cout << "size of struct sockaddr_in:" << sizeof(struct sockaddr_in) << "\n";
  std::cout << "size of struct in_addr:" << sizeof(struct in_addr) << "\n";
  std::cout << "size of struct sockaddr_in6:" << sizeof(struct sockaddr_in6) << "\n";
  std::cout << "size of struct in6_addr:" << sizeof(struct sockaddr_in) << "\n";

  // Ipv4
  struct sockaddr_in ip4addr;
  ip4addr.sin_family = AF_INET;
  ip4addr.sin_port = htons(3490);
  inet_pton(AF_INET, "10.0.0.1", &ip4addr.sin_addr);
  for (unsigned i = 0; i < 4; ++i)
    std::cout << (int)((char *)&ip4addr.sin_addr)[i] << " ";
  std::cout << "\n";

  // Ipv6.
  struct sockaddr_in6 ip6addr;
  ip6addr.sin6_family = AF_INET6;
  ip6addr.sin6_port = htons(3490);
  inet_pton(AF_INET, "2001:db8:8714:3a90::12", &ip6addr.sin6_addr);
  std::cout << std::setbase(16);
  for (unsigned i = 0; i < 16; ++i)
    std::cout << (unsigned)((unsigned char *)&ip6addr.sin6_addr)[i] << " ";
  std::cout << "\n";

}

