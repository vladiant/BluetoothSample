#include <alsa/asoundlib.h>

#include "btinclude.h"

int set_class(unsigned int cls, int timeout) {
  int id;
  int fh;
  bdaddr_t btaddr;
  char pszaddr[18];

  // get the device ID
  // passing in NULL instead of a bdaddr_t will
  // give the ID of the first bluetooth device
  if ((id = hci_get_route(NULL)) < 0) return -1;

  // convert the device ID into a 6 byte bluetooth address
  if (hci_devba(id, &btaddr) < 0) return -1;

  // convert the address into a zero terminated string
  if (ba2str(&btaddr, pszaddr) < 0) return -1;

  // open a file handle to the HCI
  if ((fh = hci_open_dev(id)) < 0) return -1;

  // set the class
  if (hci_write_class_of_dev(fh, cls, timeout) != 0) {
    perror("hci_write_class ");
    return -1;
  }

  // close the file handle
  hci_close_dev(fh);

  printf("set device %s to class: 0x%06x\n", pszaddr, cls);

  return 0;
}

/***********************************************************************/

// source adapted from:
// http://people.csail.mit.edu/albert/bluez-intro/x604.html and
// http://nohands.sourceforge.net/source.html (libhfp/hfp.cpp: SdpRegister)

int register_sdp(uint8_t channel) {
  const char *service_name = "HSP service";
  const char *service_dsc = "HSP";
  const char *service_prov = "nebland software, LLC";

  uuid_t hs_uuid, ga_uuid;

  sdp_profile_desc_t desc;

  uuid_t root_uuid, l2cap_uuid, rfcomm_uuid;
  sdp_list_t *l2cap_list = 0, *rfcomm_list = 0, *root_list = 0, *proto_list = 0,
             *access_proto_list = 0;

  sdp_data_t *channel_d = 0;

  int err = 0;
  sdp_session_t *session = 0;

  sdp_record_t *record = sdp_record_alloc();

  // set the name, provider, and description
  sdp_set_info_attr(record, service_name, service_prov, service_dsc);

  // service class ID (HEADSET)
  sdp_uuid16_create(&hs_uuid, HEADSET_SVCLASS_ID);

  if (!(root_list = sdp_list_append(0, &hs_uuid))) return -1;

  sdp_uuid16_create(&ga_uuid, GENERIC_AUDIO_SVCLASS_ID);

  if (!(root_list = sdp_list_append(root_list, &ga_uuid))) return -1;

  if (sdp_set_service_classes(record, root_list) < 0) return -1;

  sdp_list_free(root_list, 0);
  root_list = 0;

  // make the service record publicly browsable
  sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);

  root_list = sdp_list_append(0, &root_uuid);
  sdp_set_browse_groups(record, root_list);

  // set l2cap information
  sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
  l2cap_list = sdp_list_append(0, &l2cap_uuid);
  proto_list = sdp_list_append(0, l2cap_list);

  // set rfcomm information
  sdp_uuid16_create(&rfcomm_uuid, RFCOMM_UUID);
  channel_d = sdp_data_alloc(SDP_UINT8, &channel);
  rfcomm_list = sdp_list_append(0, &rfcomm_uuid);

  sdp_list_append(rfcomm_list, channel_d);
  sdp_list_append(proto_list, rfcomm_list);

  // attach protocol information to service record
  access_proto_list = sdp_list_append(0, proto_list);
  sdp_set_access_protos(record, access_proto_list);

  sdp_uuid16_create(&desc.uuid, HEADSET_PROFILE_ID);

  // set the version to 1.0
  desc.version = 0x0100;

  if (!(root_list = sdp_list_append(NULL, &desc))) return -1;

  if (sdp_set_profile_descs(record, root_list) < 0) return -1;

  // connect to the local SDP server and register the service record
  session = sdp_connect(BDADDR_ANY, BDADDR_LOCAL, SDP_RETRY_IF_BUSY);
  err = sdp_record_register(session, record, 0);

  // cleanup
  sdp_data_free(channel_d);
  sdp_list_free(l2cap_list, 0);
  sdp_list_free(rfcomm_list, 0);
  sdp_list_free(root_list, 0);
  sdp_list_free(access_proto_list, 0);

  return err;
}

/***********************************************************************/

int rfcomm_listen(uint8_t channel) {
  int sock;    // socket descriptor for local listener
  int client;  // socket descriptor for remote client
  unsigned int len = sizeof(struct sockaddr_rc);

  struct sockaddr_rc remote;  // local rfcomm socket address
  struct sockaddr_rc local;   // remote rfcomm socket address
  char pszremote[18];

  // initialize a bluetooth socket
  sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

  local.rc_family = AF_BLUETOOTH;

  // TODO: change this to a local address if you know what
  // address to use
  local.rc_bdaddr = *BDADDR_ANY;
  local.rc_channel = channel;

  // bind the socket to a bluetooth device
  if (bind(sock, (struct sockaddr *)&local, sizeof(struct sockaddr_rc)) < 0)
    return -1;

  // set the listening queue length
  if (listen(sock, 1) < 0) return -1;

  printf("accepting connections on channel: %d\n", channel);

  // accept incoming connections; this is a blocking call
  client = accept(sock, (struct sockaddr *)&remote, &len);

  ba2str(&remote.rc_bdaddr, pszremote);

  printf("received connection from: %s\n", pszremote);

  // turn off blocking
  if (fcntl(client, F_SETFL, O_NONBLOCK) < 0) return -1;

  // return the client socket descriptor
  return client;
}

/***********************************************************************/

int sco_listen() {
  int sock;
  int client;
  unsigned int len = sizeof(struct sockaddr_sco);
  char pszremote[18];

  struct sockaddr_sco local;
  struct sockaddr_sco remote;

  sock = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_SCO);

  local.sco_family = AF_BLUETOOTH;

  local.sco_bdaddr = *BDADDR_ANY;

  if (bind(sock, (struct sockaddr *)&local, sizeof(struct sockaddr_sco)) < 0)
    return -1;

  if (listen(sock, 1) < 0) return -1;

  client = accept(sock, (struct sockaddr *)&remote, &len);

  ba2str(&remote.sco_bdaddr, pszremote);

  printf("sco received connection from: %s\n", pszremote);

  // close the listener
  close(sock);

  return client;
}

/***********************************************************************/

int handle_connection(int rfcommsock, int scosock) {
  char scobuffer[255];
  char rfcommbuffer[255];
  int len;

  struct snd_pcm_t *sndhandle;

  // open the default sound device
  if (snd_pcm_open((snd_pcm_t **)&sndhandle, "default", SND_PCM_STREAM_PLAYBACK,
                   0) < 0) {
    perror("snd_pcm_open ");
    return -1;
  }

  // initialize the device to handle an 8khz, single channel,
  // little endian audio data stream
  if (snd_pcm_set_params((snd_pcm_t *)sndhandle, SND_PCM_FORMAT_S16_LE,
                         SND_PCM_ACCESS_RW_INTERLEAVED, 1, 8000, 0, 0) < 0) {
    perror("set_params ");
    return -1;
  }

  while (1) {
    // read from the SCO socket
    // it should constantly stream data
    if ((len = recv(scosock, scobuffer, 255, 0)) <= 0) break;

    // send the sound data to the sound device
    // the sound device expects "frames".  since we have 16 bit, single
    // channel data, each 2 bytes is one "frame"
    snd_pcm_writei((snd_pcm_t *)sndhandle, scobuffer, len / 2);

    // read from the RFCOMM socket
    // this socket has blocking turned off so it will never block,
    // even if no data is available
    len = recv(rfcommsock, rfcommbuffer, 255, 0);

    // EWOULDBLOCK indicates the socket would block if we had a
    // blocking socket.  we'll safely continue if we receive that
    // error.  treat all other errors as fatal
    if (len < 0 && errno != EWOULDBLOCK) {
      perror("rfcomm recv ");
      break;
    } else if (len > 0) {
      // received a message; print it to the screen and
      // return ATOK to the remote device
      rfcommbuffer[len] = '\0';

      printf("rfcomm received: %s\n", rfcommbuffer);
      send(rfcommsock, "ATOK\r\n", 6, 0);
    }

    // printf("loop\n");
  }

  // close the sound device
  snd_pcm_close((snd_pcm_t *)sndhandle);

  close(scosock);
  close(rfcommsock);

  printf("client disconnected\n");

  return 0;
}

/***********************************************************************/

unsigned int cls = 0x280404;
int timeout = 1000;
uint8_t channel = 3;

int main() {
  int rfcommsock;
  int scosock;

  if (set_class(cls, timeout) < 0) {
    perror("set_class ");
  }

  if (register_sdp(channel) < 0) {
    perror("register_sdp ");
    return -1;
  }

  if ((rfcommsock = rfcomm_listen(channel)) < 0) {
    perror("set_class ");
    return -1;
  }

  if ((scosock = sco_listen()) < 0) {
    perror("sco_listen ");
    return -1;
  }

  handle_connection(rfcommsock, scosock);

  return 0;
}
