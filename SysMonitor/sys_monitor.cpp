#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include "load_config.h"
#ifndef _WIN32
#include <netinet/in.h>
# ifdef _XOPEN_SOURCE_EXTENDED
#  include <arpa/inet.h>
# endif
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <err.h>
#include <unistd.h>
#include <fcntl.h>
#else
#include <time.h>
#include <WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#include <io.h>
#include <process.h>
#endif

#include "protocol_manage.h"
#include <event2/event.h>
#ifdef __cplusplus
extern "C" {
#endif
#include <event2/bufferevent.h>
#include <event2/bufferevent_compat.h>
#include <event2/bufferevent_struct.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event_compat.h>
#include <event2/event_struct.h>
#ifdef __cplusplus
}
#endif

/* Easy sensible linked lists. */
#include "queue.h"



/* Length of each buffer in the buffer queue.  Also becomes the amount
* of data we try to read per call to read(2). */
#define BUFLEN 1024*4

/**
* In event based programming we need to queue up data to be written
* until we are told by libevent that we can write.  This is a simple
* queue of buffers to be written implemented by a TAILQ from queue.h.
*/
struct bufferq {
	/* The buffer. */
	char *buf;
	/* The length of bufferq. */
	int len;

	/* The offset into buf to start writing from. */
	int offset;

	/* For the linked list structure. */
	TAILQ_ENTRY(bufferq) entries;
};

/**
* A struct for client specific data, also includes pointer to create
* a list of clients.
*
* In event based programming it is usually necessary to keep some
* sort of object per client for state information.
*/
struct client {
	/* Events. We need 2 event structures, one for read event
	* notification and the other for writing. */
// 	struct event ev_read;
// 	struct event ev_write;
 	struct event* ev_timer;
	struct bufferevent *buf_ev;
	int fd;
	CLoadConfig*     load_config;
	CProtocolManage* proto_manage;
	/* This is the queue of data to be written to this client. As
	* we can't call write(2) until libevent tells us the socket
	* is ready for writing. */
	//TAILQ_HEAD(, bufferq) writeq;
};

/**
* Set a socket to non-blocking mode.
*/

struct timeval time_val;

#ifdef WIN32
#define err_plantform(n, erromsg) printf(erromsg)
#else
#define err_plantform(n, erromsg)	err(1, erromsg)
#endif

#ifdef WIN32
#define WARN(erromsg) printf(erromsg);
#else
#define WARN(erromsg) warn(erromsg);
#endif

int
setnonblock(int fd)
{
	
#ifndef WIN32
	int flags;
	flags = fcntl(fd, F_GETFL);
	if (flags < 0)
		return flags;
	flags |= O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flags) < 0)
		return -1;
#else
	unsigned long flags;
	/* ÉèÖÃ·Ç×èÈû */
	flags = 1;
	ioctlsocket(fd, FIONBIO, &flags);
	return WSAGetLastError();
#endif
}


void
on_timer(int fd, short event, void *arg)
{
	client *ins_client = (client*)arg;
	if (ins_client){
		if (!ins_client->proto_manage)
			ins_client->proto_manage = new CProtocolManage(ins_client->load_config);
		ins_client->proto_manage->write(fd);

		time_val.tv_sec = 1;
		time_val.tv_usec = 0;
		evtimer_del(ins_client->ev_timer);
		evtimer_add(ins_client->ev_timer, &time_val);
	}
	
}

void
buffered_on_read(struct bufferevent *bev, void *arg)
{
	struct client *client = (struct client *)arg;
	size_t buffer_len = evbuffer_get_length(bev->input);
	if (buffer_len > 0)
	{
		char *buf;
		buf = (char*)malloc(BUFLEN);
		if (buf == NULL)
			err_plantform(1, " malloc failed");
		evbuffer_copyout(bev->input, buf, buffer_len);

		if (!client->proto_manage)
			client->proto_manage = new CProtocolManage(client->load_config);
		client->proto_manage->read(client->fd, buf);
		bufferevent_write(bev, buf, strlen(buf)+1);
		if (buf)
			free(buf);
	}
	
}

void
buffered_on_write(struct bufferevent *bev, void *arg)
{
}

void
buffered_on_error(struct bufferevent *bev, short what, void *arg)
{
	struct client *client = (struct client *)arg;

	if (what & EVBUFFER_EOF) {
		/* Client disconnected, remove the read event and the
		* free the client structure. */
		printf("\nClient disconnected.\n");
	}
	else {
		WARN("\nClient socket error, disconnecting.\n");
	}
	bufferevent_free(client->buf_ev);
	TDEL(client->load_config);
	TDEL(client->proto_manage);
	evtimer_del(client->ev_timer);
#ifdef WIN32
	closesocket(client->fd);
#else
	close(client->fd);
#endif
	free(client);
}

void
on_accept(int fd, short ev, void *arg)
{
	CLoadConfig *load_config_ins = new CLoadConfig;
	CLoadConfig::LoadConfig(load_config_ins);
#ifdef WIN32
	load_config_ins->get_sys_os_info();
#endif // WIN32
	struct event_base* eventbase = (event_base*)arg;
	int client_fd;
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	struct client *ins_client;

	client_fd = accept(fd, (struct sockaddr *)&client_addr, &client_len);
	if (client_fd < 0) {
		WARN("accept failed");
		return;
	}
	if (setnonblock(client_fd) < 0)
		WARN("failed to set client socket non-blocking");

	ins_client = (client*)calloc(1, sizeof(*ins_client));
	if (ins_client == NULL)
		err_plantform(1, "malloc failed");

	ins_client->load_config = load_config_ins;
	ins_client->fd = client_fd;
	/*ins_client->buf_ev->ev_base = eventbase;*/
	ins_client->buf_ev = bufferevent_new(client_fd, buffered_on_read,
		buffered_on_write, buffered_on_error, ins_client);
	
	bufferevent_enable(ins_client->buf_ev, EV_READ);

 	ins_client->ev_timer = event_new(eventbase, fd, 0, on_timer, ins_client);
 	time_val.tv_sec = 1;
 	time_val.tv_usec = 0;
 	evtimer_add(ins_client->ev_timer, &time_val);
	printf("Accepted connection from addr:%s port:%d\n",
		inet_ntoa(client_addr.sin_addr), htons(client_addr.sin_port));
}

int
main(int argc, char **argv)
{
#ifdef _WIN32
	WORD w_version_requested;
	WSADATA wsa_data;
	w_version_requested = MAKEWORD(2, 2);

	(void)WSAStartup(w_version_requested, &wsa_data);
#endif
	struct event_base* eventbase;
	int listen_fd;
	struct sockaddr_in listen_addr;
	struct event ev_accept;
	int reuseaddr_on;
	int port = 0;
	/* Initialize libevent. */
	eventbase = event_init();

	/* Create our listening socket. */
	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_fd < 0)
		err_plantform(1, "listen failed");

	CLoadConfig *load_config_ins = new CLoadConfig;
	CLoadConfig::LoadConfig(load_config_ins);

	memset(&listen_addr, 0, sizeof(listen_addr));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = INADDR_ANY;
	port = load_config_ins->get_port();
	listen_addr.sin_port = htons(port);
	TDEL(load_config_ins);

	if (bind(listen_fd, (struct sockaddr *)&listen_addr,
		sizeof(listen_addr)) < 0)
		err_plantform(1, "bind failed");
	if (listen(listen_fd, 5) < 0)
		err_plantform(1, "listen failed");
	reuseaddr_on = 1;
	setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&reuseaddr_on,
		sizeof(reuseaddr_on));

	if (setnonblock(listen_fd) < 0)
		err_plantform(1, "failed to set server socket to non-blocking");

	event_set(&ev_accept, listen_fd, EV_READ | EV_PERSIST, on_accept, eventbase);
	event_add(&ev_accept, NULL);

	event_dispatch();

	return 0;
}
