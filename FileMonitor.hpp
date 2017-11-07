//PauloRB - 2012 - File Monitor

#ifndef FILE_MONITOR_HPP
#define FILE_MONITOR_HPP

#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <map> 
#include <poll.h>
#include <errno.h>

#define WAIT_OBJECT_0 0x0
#define FILE_NOTIFY_CHANGE_FILE_NAME    0x00000001   
#define FILE_NOTIFY_CHANGE_DIR_NAME     0x00000002   
#define FILE_NOTIFY_CHANGE_ATTRIBUTES   0x00000004   
#define FILE_NOTIFY_CHANGE_SIZE         0x00000008   
#define FILE_NOTIFY_CHANGE_LAST_WRITE   0x00000010   
#define FILE_NOTIFY_CHANGE_LAST_ACCESS  0x00000020   
#define FILE_NOTIFY_CHANGE_CREATION     0x00000040   
#define FILE_NOTIFY_CHANGE_SECURITY     0x00000100  

typedef void *HANDLE;
typedef unsigned long	 LONG_PTR;
#define INVALID_HANDLE_VALUE -1
#define EVENT_SIZE  ( sizeof (struct inotify_event) )


struct paramThread {
	pthread_t id;
	pthread_mutex_t mutex_id;
	int fd;
	int wd;
	int mapIndex;
};


pthread_cond_t g_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t g_mutex;
pthread_t g_tid;
int g_id_Counter = 1;
std::map<int,pthread_mutex_t *> g_map_IDtoMutex;
std::map<int,pthread_t *> g_map_IDtoThread;
std::map<int,pthread_cond_t *> g_map_IDtoCond;




void* ThreadFileMon(void *arg)
{
	printf("\nThreadFileMon Initiated");
	fflush(stdout);
	char buffer[1024];
	int i = 0;
	struct paramThread *th_params = (struct paramThread *)arg;

	while (1) {

		struct pollfd pfd = { th_params->fd, POLLIN, 0 };
		int ret = poll(&pfd, 1, 50);  // timeout of 50ms
		if (ret < 0) {
			printf("\failed poll");
			pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
			sleep(1); //Slice of time that the thread becomes cancelable
			pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
		}
		else if (ret == 0) {
			// Timeout with no events, move on.
			printf("\nTimeout poll");
			pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
			sleep(1); //Slice of time that the thread becomes cancelable
			pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
		}
		else {
			pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
			i = 0;
			int lenght = read(th_params->fd, buffer, 1024);
			printf("\nRead lenght=%d", lenght);
			if(lenght != -1)
			while (i < lenght) {
				printf("\nThreadFileMon while");
				fflush(stdout);
				struct inotify_event *event = (struct inotify_event *) &buffer[i];
				if (event->len) {

					sleep(1);
					if (event->mask & IN_CREATE) {
						if (event->mask & IN_ISDIR) {
							printf("The directory %s was created.\n", event->name);
						}
						else {
							pthread_mutex_lock(g_map_IDtoMutex[th_params->mapIndex]);
							pthread_cond_signal(g_map_IDtoCond[th_params->mapIndex]);
							pthread_mutex_unlock(g_map_IDtoMutex[th_params->mapIndex]);
							printf("The file %s was created.\n", event->name);
						}
					}
					else if (event->mask & IN_DELETE) {
						if (event->mask & IN_ISDIR) {
							printf("The directory %s was deleted.\n", event->name);
						}
						else {
							printf("The file %s was deleted.\n", event->name);
						}
					}
					else if (event->mask & IN_MODIFY) {
						if (event->mask & IN_ISDIR) {
							printf("The directory %s was modified.\n", event->name);
						}
						else {
							pthread_mutex_lock(g_map_IDtoMutex[th_params->mapIndex]);
							pthread_cond_signal(g_map_IDtoCond[th_params->mapIndex]);
							pthread_mutex_unlock(g_map_IDtoMutex[th_params->mapIndex]);
							printf("The file %s was modified.\n", event->name);
						}
					}
				}
				i += EVENT_SIZE + event->len;
			}
		}
	}

	(void)inotify_rm_watch(th_params->fd, th_params->wd);
	(void)close(th_params->fd);
}

int FindFirstChangeNotification(
	const char * lpPathName,
	int    bWatchSubtree,
	unsigned long  dwNotifyFilter
) {
	struct paramThread pThread;
	int length, i = 0;
	int fd;
	int wd;
	char buffer[1024];
	int err;
	
	if(lpPathName == NULL){
		printf("\nFileMonitor - lpPathName must not be NULL");
		return INVALID_HANDLE_VALUE;
	}
	
	if(strlen(lpPathName) == 0){
		printf("\nFileMonitor - lpPathName must have a value");
		return INVALID_HANDLE_VALUE;
	}
		
	fd = inotify_init();
	printf("\ninotify_init");
	if (fd < 0) {
		printf("inotify_init ERROR %d",errno);
		return INVALID_HANDLE_VALUE;
	}

	int filter = -1;
	if (dwNotifyFilter != FILE_NOTIFY_CHANGE_FILE_NAME) {
		//not implemented   
		return INVALID_HANDLE_VALUE;
	}
	else {
		//filter = IN_MODIFY | IN_CREATE | IN_DELETE; 
		filter = IN_ALL_EVENTS;
	}

	printf("\ninotify_add_watch %s",lpPathName);
	wd = inotify_add_watch(fd, lpPathName, filter);

	if (wd == -1) {
		printf("\ninotify_add_watch ERROR %d",errno);
		return INVALID_HANDLE_VALUE;
	}

	pThread.fd = fd;
	pThread.wd = wd;

	pthread_mutex_t *mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));

	printf("\ncreating pthread_mutex_init");
	if (pthread_mutex_init(mutex, NULL) != 0)
	{
		printf("\npthread_mutex_init INVALID_HANDLE_VALUE");
		return INVALID_HANDLE_VALUE;
	}

	pthread_t * tid = (pthread_t *)malloc(sizeof(pthread_t));

	err = pthread_create(tid, NULL, &ThreadFileMon, &pThread);
	if (err != 0) {
		printf("\npthread_create INVALID_HANDLE_VALUE");
		return INVALID_HANDLE_VALUE;
	}
	pThread.id = *tid;
	
	pthread_cond_t *cond = (pthread_cond_t *)malloc(sizeof(pthread_cond_t));
	pthread_cond_init(cond,NULL);
	
	pThread.mapIndex = g_id_Counter;
	
	//Store all thread info to a map
	g_map_IDtoMutex[g_id_Counter] =  mutex;
    g_map_IDtoThread[g_id_Counter] = tid;
    g_map_IDtoCond[g_id_Counter] = cond;
	
	

	return g_id_Counter++;

}

unsigned long WaitForSingleObject(
	int hHandle,
	unsigned long  dwMilliseconds
) {

	int ret = 0;
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_sec += dwMilliseconds / 1000; // +seconds
	pthread_mutex_lock(g_map_IDtoMutex[hHandle]);
	ret = pthread_cond_timedwait(g_map_IDtoCond[hHandle], g_map_IDtoMutex[hHandle], &ts);
	pthread_mutex_unlock(g_map_IDtoMutex[hHandle]);

	if (ret == 0) {
		return 0; //WAIT_OBJECT_0 (Win32)
	}
	else {
		//TODO Other return cases
		return 0x00000102L; //WAIT_TIMEOUT (Win32)
	}


}


int FindNextChangeNotification(
	int hChangeHandle
) {
	printf("\FindNextChangeNotification");
	return 0;
}

int FindCloseChangeNotification(
	int hChangeHandle
) {
	printf("\nFindCloseChangeNotification");
	pthread_cond_destroy(&g_cond);
	pthread_mutex_destroy(&g_mutex);
	pthread_cancel(g_tid);
	
	//TODO clear map and free all pointers
	return 0;
}



#endif  // FILE_MONITOR_HPP
