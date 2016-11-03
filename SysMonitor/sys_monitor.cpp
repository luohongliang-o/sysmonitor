#include "protocol_manage.h"
#include "sys_monitor.h"
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>

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
#include <pthread.h>
#define  __stdcall
#else
#include <time.h>
#include <WinSock2.h>
#include <io.h>
#include <process.h>
#endif
#ifdef __cplusplus
extern "C" {
#endif
#include <event2/event-config.h>
#include <event2/event.h>
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
#include "func.h"
#define BUFLEN 1024*4
#ifdef WIN32
HANDLE g_time_handle = NULL;
#endif
int    g_log_flag = 0;
volatile bool g_thread_on_of = TRUE;


struct PacketHead{
	short protocol_versoin;
	long packet_len;
};
struct packet{
	PacketHead  packet_head;
	char buf[1];
};

struct client {
	int fd;
	struct bufferevent *buf_ev;
	CProtocolManage* proto_manage;
	char*            buffer;
};
struct monitor_global 
{
	struct event_base *ev_base;
	CProtocolManage* proto_manage;
};
struct timeval time_val;


//monitor_global* g_monitor = NULL;

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

#if defined(WIN32)
static unsigned __stdcall
on_timer(void *arg)
{
	::CoInitialize(NULL);
	monitor_global *g_monitor = (monitor_global*)arg;
	if (g_monitor){
		while (g_thread_on_of){
			if (!g_monitor->proto_manage)
				g_monitor->proto_manage = new CProtocolManage();
			int write_len = g_monitor->proto_manage->write();
			char current_time[128] = "";
			GetFormatSystemTime(current_time,128);
			printf("\nwrite time:%s data len:%d bytes\n", current_time, write_len);
			bool bsleep = TRUE;
			int object_num = CLoadConfig::CreateInstance()->get_object_num();
			vector< short > object_type = CLoadConfig::CreateInstance()->get_object_type();
			for (int i = 0; i < object_num;i++){
				if (object_type[i] == 1 || object_type[i] == 3) {
					bsleep = FALSE;
					break;
				}
			}
			if (bsleep)
				Sleep(1000);
		}
	}
	::CoUninitialize();
	return 0;
}
#else 
static void*
on_timer(void *arg)
{
	monitor_global *g_monitor = (monitor_global*)arg;
	if (g_monitor){
		while (g_thread_on_of){
			if (!g_monitor->proto_manage)
				g_monitor->proto_manage = new CProtocolManage();
			int write_len = g_monitor->proto_manage->write();
			char current_time[128] = "";
			GetFormatSystemTime(current_time, 128);
			printf("\nwrite time:%s data len:%d bytes\n", current_time, write_len);
			bool bsleep = TRUE;
			int object_num = CLoadConfig::CreateInstance()->get_object_num();
			vector< short > object_type = CLoadConfig::CreateInstance()->get_object_type();
			for (int i = 0; i < object_num; i++){
				if (object_type[i] == 1 || object_type[i] == 3) {
					bsleep = FALSE;
					break;
				}
			}
			if (bsleep)
				sleep(1);
		}
	}
	return 0;
}

#endif


void
buffered_on_read(struct bufferevent *bev, void *arg)
{
	size_t buffer_len = evbuffer_get_length(bev->input);
	if (buffer_len > 0)
	{
		struct client *client = (struct client *)arg;
		evbuffer_remove(bev->input, client->buffer, buffer_len);
		if (client->proto_manage){
			char send_buf[BUFLEN] = {0};
			packet* packet_data = (packet*)send_buf;
			client->buffer[buffer_len] = '\0';
			int read_buf_len = client->proto_manage->read(client->fd, client->buffer);
			packet_data->packet_head.packet_len = read_buf_len ;
			packet_data->packet_head.protocol_versoin = 1;
			int all_len = packet_data->packet_head.packet_len + sizeof(packet_data->packet_head);
			memcpy(packet_data->buf, client->buffer, read_buf_len);
			if (read_buf_len > 0 && !strstr(client->buffer, "check error.")){
				char current_time[128] = "";
				GetFormatSystemTime(current_time,128);
				printf("\nread time:%s data len:%d bytes\n", current_time, packet_data->packet_head.packet_len);
				bufferevent_write(bev, packet_data, all_len);
			}
		}
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
		WriteLog(g_log_flag,LOGFILENAME, "Client disconnected.");
		printf("\nClient disconnected.\n");
	}
	else {
		WriteLog(g_log_flag,LOGFILENAME, "Client socket error, disconnecting.");
		WARN("\nClient socket error, disconnecting.\n");
	}
	bufferevent_free(client->buf_ev);
	client->proto_manage = NULL;
	TDELARRAY(client->buffer);
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
	monitor_global *g_monitor = (monitor_global*)arg;
	int client_fd;
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	struct client *ins_client;

	client_fd = accept(fd, (struct sockaddr *)&client_addr, &client_len);
	if (client_fd < 0) {
		WARN("accept failed");
		return;
	}
	ins_client = (client*)calloc(1, sizeof(*ins_client));
	if (ins_client == NULL)
		err_plantform(1, "malloc failed");
	
	ins_client->proto_manage = g_monitor->proto_manage;
	ins_client->buffer = new char[BUFLEN];
	
	ins_client->fd = client_fd;
	
	ins_client->buf_ev = bufferevent_socket_new(g_monitor->ev_base, client_fd, BEV_OPT_CLOSE_ON_FREE);

	evutil_make_socket_nonblocking(client_fd);

	bufferevent_setcb(ins_client->buf_ev, buffered_on_read,
		NULL, buffered_on_error, ins_client);

	bufferevent_enable(ins_client->buf_ev, EV_READ|EV_WRITE|EV_PERSIST);

	printf("Accepted connection from addr:%s port:%d\n",
		inet_ntoa(client_addr.sin_addr), htons(client_addr.sin_port));

	WriteLog(g_log_flag,LOGFILENAME, "Accepted connection from addr:%s port:%d",
		inet_ntoa(client_addr.sin_addr), htons(client_addr.sin_port));
}

EXTERN_C MYDLLAPI
void start()
{
#ifdef _WIN32
	WORD w_version_requested;
	WSADATA wsa_data;
	w_version_requested = MAKEWORD(2, 2);
	(void)WSAStartup(w_version_requested, &wsa_data);
#endif
	bool blisten = TRUE;
	struct event_base* eventbase;
	int listen_fd;
	struct sockaddr_in listen_addr;
	struct event ev_accept;

	int port = 0;
	/* Initialize libevent. */
	eventbase = event_init();

	/* Create our listening socket. */
	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_fd < 0){
		err_plantform(1, "listen failed");
		blisten = FALSE;
	}
	evutil_make_listen_socket_reuseable(listen_fd);

	monitor_global* g_monitor = new monitor_global;
	g_monitor->ev_base = eventbase;
	CLoadConfig* load_config = CLoadConfig::CreateInstance();
	load_config->LoadConfig();
	g_log_flag = load_config->get_log_flag();
	g_monitor->proto_manage = new CProtocolManage();

	memset(&listen_addr, 0, sizeof(listen_addr));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = INADDR_ANY;
	port = load_config->get_port();
	listen_addr.sin_port = htons(port);

	if (bind(listen_fd, (struct sockaddr *)&listen_addr,
		sizeof(listen_addr)) < 0){
		blisten = FALSE;
		err_plantform(1, "\nbind failed");
	}	
	if (listen(listen_fd, 5) < 0){
		blisten = FALSE;
		err_plantform(1, "\nlisten failed");
	}
	
	evutil_make_socket_nonblocking(listen_fd);
	event_set(&ev_accept, listen_fd, EV_READ | EV_PERSIST, on_accept, g_monitor);
	event_add(&ev_accept, NULL);
	if (blisten){
		unsigned threadid;
#ifdef WIN32
		load_config->get_sys_os_info();
		g_time_handle = (HANDLE)_beginthreadex(NULL, 0, on_timer, g_monitor, 0, &threadid);
#else
		pthread_t a_thread; 
		threadid = pthread_create(&a_thread, NULL, on_timer, (void*)g_monitor);
#endif // WIN32

	}
	event_dispatch();


	TDEL(g_monitor->proto_manage);
	event_base_free(g_monitor->ev_base);
	TDEL(g_monitor);

#ifdef WIN32
/*
	if (WaitForSingleObject(g_time_handle, 500) == WAIT_TIMEOUT)
		TerminateThread(g_time_handle, 0);
*/
	WaitForSingleObject(g_time_handle, INFINITE);
	CLOSEHANDLE(g_time_handle);
#endif

}

/*
MYDLLAPI
void __stdcall stop()
{
	TDEL(g_monitor->proto_manage);
	event_base_free(g_monitor->ev_base);
	TDEL(g_monitor);

#ifdef WIN32
	//if (WaitForSingleObject(g_time_handle, 500) == WAIT_TIMEOUT)
		//TerminateThread(g_time_handle, 0);
	WaitForSingleObject(g_time_handle, INFINITE);
	CLOSEHANDLE(g_time_handle);
#endif
}
*/