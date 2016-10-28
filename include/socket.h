

int socket_var(char *address, int var, float *f);
int socket_thingspeak(char *key, int field, float _val);
int socket_mac(char *mac,char *ip);
static char *ethernet_mactoa(struct sockaddr *addr);
int socket_post(char *post, char *out);
int socket_post1(char *post, char *buf);
