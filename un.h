int serv_listen(const char *name);
int serv_accept(int listenfd, uid_t *uidptr);
int cli_conn(const char *name);

int send_fd(int fd, int fd_to_send, const char *errmsg);
int recv_fd(int fd);
