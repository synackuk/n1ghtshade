#include <netinet/tcp.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>

int tcp_init() {
	// from launchd
	struct ifaliasreq ifra;
	struct ifreq ifr;
	int s;

	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, "lo0");

	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		return -1;

	if (ioctl(s, SIOCGIFFLAGS, &ifr) != -1) {
		ifr.ifr_flags |= IFF_UP;
		if(ioctl(s, SIOCSIFFLAGS, &ifr) == -1) {
			return -1;
		} 
	}

	memset(&ifra, 0, sizeof(ifra));
	strcpy(ifra.ifra_name, "lo0");
	((struct sockaddr_in *)&ifra.ifra_addr)->sin_family = AF_INET;
	((struct sockaddr_in *)&ifra.ifra_addr)->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	((struct sockaddr_in *)&ifra.ifra_addr)->sin_len = sizeof(struct sockaddr_in);
	((struct sockaddr_in *)&ifra.ifra_mask)->sin_family = AF_INET;
	((struct sockaddr_in *)&ifra.ifra_mask)->sin_addr.s_addr = htonl(IN_CLASSA_NET);
	((struct sockaddr_in *)&ifra.ifra_mask)->sin_len = sizeof(struct sockaddr_in);

	if(ioctl(s, SIOCAIFADDR, &ifra) == -1) {
		return -1;
	} 

	return close(s);

}