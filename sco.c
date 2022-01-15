
#include "btinclude.h"

int main()
{
	int sock;
	int client;
	unsigned int len = sizeof(struct sockaddr_sco);

	struct sockaddr_sco local;
	struct sockaddr_sco remote;

	char pszremote[18];

	sock = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_SCO);

	local.sco_family = AF_BLUETOOTH;

	local.sco_bdaddr = *BDADDR_ANY;

	if (bind(sock, (struct sockaddr*)&local, sizeof(struct sockaddr_sco)) < 0)
		return -1;

	if (listen(sock, 1) < 0)
		return -1;

	client = accept(sock, (struct sockaddr*)&remote, &len);
	printf("client id: %d\n", client);

	ba2str(&remote.sco_bdaddr, pszremote);

	printf("sco received connection from: %s\n", pszremote);

	return 0;
}
