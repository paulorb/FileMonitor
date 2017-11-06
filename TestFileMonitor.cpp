#include "FileMonitor.hpp"

int main(void)
{
	char temp[10];

	strncpy(temp, "test", 4);
	temp[4] = 0;

	int m_EventID = FindFirstChangeNotification("/media/sf_P_DRIVE/PauloRB/FileMonitor/", 0, FILE_NOTIFY_CHANGE_FILE_NAME);

	int ret = WaitForSingleObject(m_EventID, 10000);
	printf("\nFinish %d", ret);
	fflush(stdout);
	
	FindNextChangeNotification(m_EventID);

	int ret2 = WaitForSingleObject(m_EventID, 10000);
	printf("\nFinish %d", ret2);

	FindCloseChangeNotification(1);
	printf("\nChangeNotification done");
	fflush(stdout);

	printf("%s", temp);
	return 0;
}