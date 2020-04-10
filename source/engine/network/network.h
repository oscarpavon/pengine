#ifndef ENGINE_NETWORK
#define ENGINE_NETWORK
#ifdef LINUX
	#include "../platforms/linux/network.h"
#endif

typedef struct NetworkConnecion;
typedef struct NetworkConnecion{
	int server_socket;
	int server_in_socket;
	int port;	
	void(*data_process)(struct NetworkConnecion*);
	int server_running;
	char name[20];
};

#endif