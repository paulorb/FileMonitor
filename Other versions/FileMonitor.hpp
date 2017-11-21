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
#include <vector>

#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/exception_ptr.hpp>
#include <boost/thread/pthread/condition_variable_fwd.hpp>
#include <boost/make_shared.hpp>

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
	boost::thread thread_handle;
	boost::condition_variable   data_ready_cond;
	boost::mutex                data_ready_mutex;
	bool                        data_ready = false;
	int fd;
	int wd;
	int mapIndex;
};


int g_id_Counter = 1;
std::map<int, boost::shared_ptr<paramThread > > g_map_IDtoThreadInfo;
boost::mutex g_mutex;


void ThreadFileMon(struct paramThread * th_params)
{
	try{
	printf("\nThreadFileMon Initiated");
	fflush(stdout);
	char buffer[1024];
	int i = 0;

	printf("\nThread Index %d",th_params->mapIndex);
	while (1) {
		boost::this_thread::interruption_point();
		
		//Make sure thread dies if interruption point not work as planned 
g_mutex.lock();
	//Critical section BEGIN
	std::map<int, boost::shared_ptr<paramThread > >::iterator it = g_map_IDtoThreadInfo.find(th_params->mapIndex);
	//Critical section END
g_mutex.unlock();
	if(it == g_map_IDtoThreadInfo.end()){
		printf("\nThread should die");
		return;
	}
	

		struct pollfd pfd = { th_params->fd, POLLIN, 0 };
		int ret = poll(&pfd, 1, 50);  // timeout of 50ms
		if (ret < 0) {
			printf("\failed poll");
	
			
		}
		else if (ret == 0) {
			// Timeout with no events, move on.
			printf("\nTimeout poll");

		}
		else {
			
			i = 0;
			int lenght = read(th_params->fd, buffer, 1024);
			if(lenght != -1)
			while (i < lenght) {
				boost::this_thread::sleep(boost::posix_time::milliseconds(100));
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
							boost::unique_lock<boost::mutex> lock(g_map_IDtoThreadInfo[th_params->mapIndex]->data_ready_mutex);
							g_map_IDtoThreadInfo[th_params->mapIndex]->data_ready = true;
							g_map_IDtoThreadInfo[th_params->mapIndex]->data_ready_cond.notify_one();					
							
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
							
							boost::unique_lock<boost::mutex> lock(g_map_IDtoThreadInfo[th_params->mapIndex]->data_ready_mutex);
							g_map_IDtoThreadInfo[th_params->mapIndex]->data_ready = true;
							g_map_IDtoThreadInfo[th_params->mapIndex]->data_ready_cond.notify_one();
							
							printf("The file %s was modified.\n", event->name);
						}
					}
				}
				i += EVENT_SIZE + event->len;
			}
		}
	}

	} 
  catch (boost::thread_interrupted&) {
	  printf("\nThread INTERRUPTED");
	  fflush(stdout);
	  return;
  }
  return;
}

int FindFirstChangeNotification(
	const char * lpPathName,
	int    bWatchSubtree,
	unsigned long  dwNotifyFilter
) {
	boost::shared_ptr<paramThread > pThread;
	int length, i = 0;
	int fd;
	int wd;
	char buffer[1024];
	int err;
	
	
	
	
	g_id_Counter++;
	
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
	
	pThread = boost::make_shared<paramThread>();
	//pThread->thread_handle = boost::make_shared<pthread_t>();
	pThread->fd = fd;
	pThread->wd = wd;	
	pThread->mapIndex = g_id_Counter;
	
	try
    {
		//Create the boost thread
		pThread->thread_handle = boost::thread(ThreadFileMon,(struct paramThread *)pThread.get());  
	}catch(...){
		//error = boost::current_exception();
		//printf("\npthread_create ERROR %d",errno);
		return INVALID_HANDLE_VALUE;
	}
	
	pThread->thread_handle.detach();
	
	

	//Store all thread info to a map
	g_mutex.lock();
	g_map_IDtoThreadInfo[g_id_Counter] = pThread;
	g_mutex.unlock();
	
	

	return g_id_Counter;

}

unsigned long WaitForSingleObject(
	int hHandle,
	unsigned long  dwMilliseconds
) {
	boost::unique_lock<boost::mutex> lock(g_map_IDtoThreadInfo[g_id_Counter]->data_ready_mutex);
	while (!g_map_IDtoThreadInfo[g_id_Counter]->data_ready)
	{
		if(g_map_IDtoThreadInfo[g_id_Counter]->data_ready_cond.timed_wait(lock,boost::posix_time::milliseconds(dwMilliseconds)) ==false){
			return 0x00000102L; //WAIT_TIMEOUT (Win32)
		}else{
			return 0; //WAIT_OBJECT_0 (Win32)
		}
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
	printf("\nFindCloseChangeNotification %d",hChangeHandle);
	
g_mutex.lock();
	//Critical section BEGIN
	std::map<int, boost::shared_ptr<paramThread > >::iterator it = g_map_IDtoThreadInfo.find(hChangeHandle);
	if(it != g_map_IDtoThreadInfo.end()){
		printf("\nFindCloseChangeNotification Found %d",hChangeHandle);
		it->second.get()->thread_handle.interrupt();
		inotify_rm_watch(it->second.get()->fd, it->second.get()->wd);
	    close(it->second.get()->fd);
	    g_map_IDtoThreadInfo.erase(it);
	}
	//Critical section END
g_mutex.unlock();	
		
	return 0;
}



#endif  // FILE_MONITOR_HPP