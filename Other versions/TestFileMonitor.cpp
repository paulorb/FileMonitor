#include "FileMonitor.hpp"

int main(void)
{
	char temp[10];

	strncpy(temp, "test", 4);
	temp[4] = 0;
	
	for(;;){

	int m_EventID = FindFirstChangeNotification("/media/sf_P_DRIVE2/PauloRB/FileMonitor/", 0, FILE_NOTIFY_CHANGE_FILE_NAME);

	int ret = WaitForSingleObject(m_EventID, 100);
	printf("\nFinish %d", ret);
	fflush(stdout);
	

	FindCloseChangeNotification(m_EventID);
	printf("\nChangeNotification done");
	fflush(stdout);
	}

	printf("%s", temp);
	return 0;
}
