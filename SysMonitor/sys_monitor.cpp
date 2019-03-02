#include "protocol_manage.h"
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
#pragma pack(1)
struct PacketHead{
	short protocol_versoin;
	int packet_len;
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
#pragma pack()
struct timeval time_val;

#ifdef WIN32
#define err_plantform(n, erromsg) printf(erromsg)
#else
#define err_plantform(n, erromsg)	err(1, erromsg)
#endif

#ifdef WIN32
#define WARN(erromsg) printf(erromsg)
#else
#define WARN(erromsg) warn(erromsg)
#endif

#if defined(WIN32)
#include <dbghelp.h>  
#pragma comment(lib,  "dbghelp.lib")
LONG WINAPI MyExptFilter(EXCEPTION_POINTERS *pExptInfo)
{
	LONG ret = EXCEPTION_CONTINUE_SEARCH;
	TCHAR szExePath[MAX_PATH] = { 0 };
	if (::GetModuleFileName(NULL, szExePath, MAX_PATH) > 0)
	{
		int ch = _T('\\');
		*_tcsrchr(szExePath, ch) = _T('\0');

		CString strDumpFileName;
		SYSTEMTIME st = { 0 };
		GetLocalTime(&st);
		CString strTime = _T("");
		strTime.Format("%04d_%02d_%02d_%02d%02d%02d%03d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
		strDumpFileName.Format("\\log\\%s_Dump.dmp", strTime);
		_tcscat(szExePath, strDumpFileName);
	}

	// 程序崩溃时，将写入程序目录下的MyDump.dmp文件  
	HANDLE hFile = ::CreateFile(szExePath, GENERIC_WRITE,
		FILE_SHARE_WRITE, NULL, CREATE_NEW,
		FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		MINIDUMP_EXCEPTION_INFORMATION exptInfo;
		exptInfo.ThreadId = ::GetCurrentThreadId();
		exptInfo.ExceptionPointers = pExptInfo;

		BOOL bOK = ::MiniDumpWriteDump(::GetCurrentProcess(),
			::GetCurrentProcessId(),
			hFile, MiniDumpNormal,
			&exptInfo, NULL, NULL);
		if (bOK)
			ret = EXCEPTION_EXECUTE_HANDLER;
	}

	return ret;
}

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
				sleep(5);
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
			WriteLog(1, LOGFILENAME, "read data before---%s", (char*)client->buffer);
			long read_buf_len = client->proto_manage->read(client->fd, client->buffer);
			WriteLog(1, LOGFILENAME, "read data---%s", client->buffer);
			packet_data->packet_head.packet_len = read_buf_len ;
			packet_data->packet_head.protocol_versoin = 1;
			//int sizelen = sizeof(PacketHead);
			int all_len = packet_data->packet_head.packet_len + sizeof(PacketHead);
			memcpy(packet_data->buf, client->buffer, read_buf_len);
			printf("read data---buf:%s\n", packet_data->buf);
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
		printf("accept failed");
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


int main()
{
#ifdef _WIN32
	LPTOP_LEVEL_EXCEPTION_FILTER pPrevFilter = ::SetUnhandledExceptionFilter(MyExptFilter);
	if (pPrevFilter != NULL)
		_tprintf(_T("Previous exception filter exists.\n"));
	else
		_tprintf(_T("No Previous exception filter.\n"));


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
		WriteLog(1, LOGFILENAME, "listen failed");
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
		WriteLog(1, LOGFILENAME, "bind failed");
		err_plantform(1, "\nbind failed");
	}
	if (listen(listen_fd, 5) < 0){
		blisten = FALSE;
		err_plantform(1, "\nlisten failed");
		WriteLog(1, LOGFILENAME, "listen failed");
	}

	evutil_make_socket_nonblocking(listen_fd);
	event_set(&ev_accept, listen_fd, EV_READ | EV_PERSIST, on_accept, g_monitor);
	event_add(&ev_accept, NULL);
	unsigned threadid;
#ifdef WIN32
	if (blisten){
		load_config->get_sys_os_info();
		g_time_handle = (HANDLE)_beginthreadex(NULL, 0, on_timer, g_monitor, 0, &threadid);
	}
#else
	pthread_t a_thread=0;
	if (blisten){
		threadid = pthread_create(&a_thread, NULL, on_timer, (void*)g_monitor);
	}
#endif // WIN32
	event_dispatch();

	TDEL(g_monitor->proto_manage);
	event_base_free(g_monitor->ev_base);
	TDEL(g_monitor);

#ifdef WIN32
	if (WaitForSingleObject(g_time_handle, 500) == WAIT_TIMEOUT)
		TerminateThread(g_time_handle, 0);
	CLOSEHANDLE(g_time_handle);
#else
	if(a_thread)
		pthread_join(a_thread,NULL);
#endif

	return 0;
}
