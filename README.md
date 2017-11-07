# FileMonitor [![Build Status](https://travis-ci.org/paulorb/FileMonitor.svg?branch=master)](https://travis-ci.org/paulorb/FileMonitor)
Linux implementation for Win32 file monitoring functions


The implementation follows the same idea of Win32 implementation, on linux it is base on the Inotify API that was merged into the 2.6.13 Linux kernel.

# FindFirstChangeNotification
Creates a change notification handle and sets up initial change notification filter conditions. A wait on a notification handle succeeds when a change matching the filter conditions occurs in the specified directory or subtree.

int FindFirstChangeNotification(
	const char * lpPathName,
	int    bWatchSubtree,
	unsigned long  dwNotifyFilter
) 

**Limitation: Currently only FILE_NOTIFY_CHANGE_FILE_NAME are supported as filter (dwNotifyFilter)**

# WaitForSingleObject
unsigned long WaitForSingleObject(
	int hHandle,
	unsigned long  dwMilliseconds
)

**Limitation: Currently the parameter dwMilliseconds are second based, like (2000 will be 2 seconds, 400 will be 0 zero), also the return is limited to WAIT_OBJECT_0 or WAIT_TIMEOUT**






