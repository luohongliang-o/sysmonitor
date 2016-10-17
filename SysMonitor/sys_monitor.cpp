#include <event2/event-config.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
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
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>
#include "event2/event_compat.h"
#include "event2/event_struct.h"
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
	struct event ev_read;
	struct event ev_write;
	struct event* ev_timer;

	CLoadConfig*     load_config;
	CProtocolManage* proto_manage;
	/* This is the queue of data to be written to this client. As
	* we can't call write(2) until libevent tells us the socket
	* is ready for writing. */
	TAILQ_HEAD(, bufferq) writeq;
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

int
setnonblock(evutil_socket_t fd)
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
on_timer(evutil_socket_t fd, short event, void *arg)
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

/**
* This function will be called by libevent when the client socket is
* ready for reading.
*/	
void
on_read(evutil_socket_t fd, short ev, void *arg)
{
	struct client *client = (struct client *)arg;
	struct bufferq *ins_bufferq;
	char *buf;
	int len;

	/* Because we are event based and need to be told when we can
	* write, we have to malloc the read buffer and put it on the
	* clients write queue. */
	buf = (char*)malloc(BUFLEN);
	if (buf == NULL)
		err_plantform(1, " malloc failed");
#ifdef WIN32
	len = recv(fd, buf, BUFLEN,0);
#else
	len = read(fd, buf, BUFLEN);
#endif
	if (len == 0) {
		/* Client disconnected, remove the read event and the
		* free the client structure. */
		printf("\nClient disconnected.\n");
#ifdef WIN32
		closesocket(fd);
#else
		close(fd);
#endif
		event_del(&client->ev_read);
		evtimer_del(client->ev_timer);
		TDEL(client->proto_manage);
		TDEL(client->load_config);
		free(client);
		return;
	}
	else if (len < 0) {
		/* Some other error occurred, close the socket, remove
		* the event and free the client structure. */
		printf("\nSocket failure, disconnecting client: %s",
			strerror(errno));
#ifdef WIN32
		closesocket(fd);
#else
		close(fd);
#endif
		event_del(&client->ev_read);
		evtimer_del(client->ev_timer);
		TDEL(client->proto_manage);
		TDEL(client->load_config);
		free(client);
		return;
	}

	/* We can't just write the buffer back as we need to be told
	* when we can write by libevent.  Put the buffer on the
	* client's write queue and schedule a write event. */
	ins_bufferq = (bufferq *)calloc(1, sizeof(bufferq));
	if (ins_bufferq == NULL)
		err_plantform(1, "malloc faild");

	if (!client->proto_manage)
		client->proto_manage = new CProtocolManage(client->load_config);
	ins_bufferq->len = client->proto_manage->read(fd, buf);
	ins_bufferq->buf = buf;
	ins_bufferq->offset = 0;
	TAILQ_INSERT_TAIL(&client->writeq, ins_bufferq, entries);
	
	/* Since we now have data that needs to be written back to the
	* client, add a write event. */
	event_add(&client->ev_write, NULL);
}

/**
* This function will be called by libevent when the client socket is
* ready for writing.
*/
void
on_write(evutil_socket_t fd, short ev, void *arg)
{
	struct client *client = (struct client *)arg;
	struct bufferq *ins_bufferq;
	int len;

	/* Pull the first item off of the write queue. We probably
	* should never see an empty write queue, but make sure the
	* item returned is not NULL. */
	ins_bufferq = TAILQ_FIRST(&client->writeq);
	if (ins_bufferq == NULL)
		return;

	/* Write the buffer.  A portion of the buffer may have been
	* written in a previous write, so only write the remaining
	* bytes. */
/*
	struct AnsHeader
	{
		int len;
		long separator;
	};
	struct AnsBuffer
	{
		AnsHeader anshead;
		char ansbuffer[BUFLEN];
	};
	AnsBuffer ansbuf = {0};
	
	ansbuf.anshead.len = len;
	ansbuf.anshead.separator = 7654321;
	memcpy(ansbuf.ansbuffer, (ins_bufferq->buf + ins_bufferq->offset), len);
	char psend[BUFLEN];
	memcpy(psend, &ansbuf, BUFLEN);
*/
	len = ins_bufferq->len - ins_bufferq->offset;
#ifdef WIN32
	len = send(fd, (char*)(ins_bufferq->buf + ins_bufferq->offset),
		ins_bufferq->len - ins_bufferq->offset,0);
#else
	len = write(fd, ins_bufferq->buf + ins_bufferq->offset,
		ins_bufferq->len - ins_bufferq->offset);
#endif
	if (len == -1) {
		if (errno == EINTR || errno == EAGAIN) {
			/* The write was interrupted by a signal or we
			* were not able to write any data to it,
			* reschedule and return. */
			event_add(&client->ev_write, NULL);
			return;
		}
		else {
			/* Some other socket error occurred, exit. */
			err_plantform(1, "write");
		}
	}
	else if ((ins_bufferq->offset + len) < ins_bufferq->len) {
		/* Not all the data was written, update the offset and
		* reschedule the write event. */
		ins_bufferq->offset += len;
		event_add(&client->ev_write, NULL);
		return;
	}

	/* The data was completely written, remove the buffer from the
	* write queue. */
	TAILQ_REMOVE(&client->writeq, ins_bufferq, entries);
	free(ins_bufferq->buf);
	free(ins_bufferq);
}

/**
* This function will be called by libevent when there is a connection
* ready to be accepted.
*/
void
on_accept(evutil_socket_t fd, short ev, void *arg)
{
	CLoadConfig *load_config_ins = new CLoadConfig;
	CLoadConfig::LoadConfig(load_config_ins);
#ifdef WIN32
	load_config_ins->get_sys_os_info();
#endif // WIN32
	struct event_base* eventbase = (event_base*)arg;
	evutil_socket_t client_fd;
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	struct client *ins_client;
	
	/* Accept the new connection. */
	client_fd = accept(fd, (struct sockaddr *)&client_addr, &client_len);
	if (client_fd == -1) {
#ifdef WIN32
		printf("accept failed");
#else
		warn("accept failed");
#endif
		return;
	}

	/* Set the client socket to non-blocking mode. */
	if (setnonblock(client_fd) < 0)
#ifdef WIN32
		printf("\nfailed to set client socket non-blocking");
#else
		warn("failed to set client socket non-blocking");
#endif

	/* We've accepted a new client, allocate a client object to
	* maintain the state of this client. */
	ins_client = (client*)calloc(1, sizeof(client));
	if (ins_client == NULL)
		err_plantform(1, "malloc failed");
	ins_client->load_config=load_config_ins;

	/* Setup the read event, libevent will call on_read() whenever
	* the clients socket becomes read ready.  We also make the
	* read event persistent so we don't have to re-add after each
	* read. */
	event_set(&ins_client->ev_read, client_fd, EV_READ | EV_PERSIST, on_read,
		ins_client);
	
	/* Setting up the event does not activate, add the event so it
	* becomes active. */
	event_add(&ins_client->ev_read, NULL);

	/* Create the write event, but don't add it until we have
	* something to write. */
	event_set(&ins_client->ev_write, client_fd, EV_WRITE, on_write, ins_client);

	/*add timer to stat system performance data. */
	
	ins_client->ev_timer = event_new(eventbase, fd, 0, on_timer, ins_client);
	time_val.tv_sec = 1;
	time_val.tv_usec = 0;
	evtimer_add(ins_client->ev_timer, &time_val);
	
	/* Initialize the clients write queue. */
	TAILQ_INIT(&ins_client->writeq);

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
	evutil_socket_t listen_fd;
	struct sockaddr_in listen_addr;
	int reuseaddr_on = 1;
	int port = 0;
	/* The socket accept event. */
	struct event ev_accept;
	
	/* Initialize libevent. */
	eventbase = event_init();

	/* Create our listening socket. This is largely boiler plate
	* code that I'll abstract away in the future. */
	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_fd < 0)
		err_plantform(1, "listen failed");
	CLoadConfig *load_config_ins = new CLoadConfig;
	CLoadConfig::LoadConfig(load_config_ins);

	if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&reuseaddr_on,
		sizeof(reuseaddr_on)) == -1)
		err_plantform(1, "setsockopt failed");
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

	
	/* Set the socket to non-blocking, this is essential in event
	* based programming with libevent. */
	if (setnonblock(listen_fd) < 0)
		err_plantform(1, "failed to set server socket to non-blocking");
	printf("system monitor start success,listen to %d\n", port);
	/* We now have a listening socket, we create a read event to
	* be notified when a client connects. */
	event_set(&ev_accept, listen_fd, EV_READ | EV_PERSIST, on_accept, eventbase);
	event_add(&ev_accept, NULL);

	/* Start the libevent event loop. */
	event_dispatch();

	return 0;
}
