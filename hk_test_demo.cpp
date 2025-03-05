


#include <iostream>
#include <HCNetSDK.h>
#include <cstring>
#include <string>

int main() { 
    if (!NET_DVR_Init()) {
        std::cout << "Failed to initialize HKVISION SDK, error code: " << NET_DVR_GetLastError() << std::endl;
        return -1;
    }   
    
    DWORD ver = NET_DVR_GetSDKVersion();
    std::cout << "SDK Ver: " << ver << std::endl;

    NET_DVR_DEVICEINFO_V30 device_info;
    memset(&device_info, 0, sizeof(NET_DVR_DEVICEINFO_V30));

    std::string ip_str = "169.254.36.138";
    int port = 8000;
    std::string username_str = "admin";
    std::string password_str = "w12345678";

    char ip[256];
    char username[256];
    char password[256];

    strcpy(ip, ip_str.c_str());
    strcpy(username, username_str.c_str());
    strcpy(password, password_str.c_str());

    LONG user_id = NET_DVR_Login_V30(ip, port, username, password, &device_info);
    if (user_id < 0) {
        std::cout << "Failed to login, error code: " << NET_DVR_GetLastError() << std::endl;
        NET_DVR_Cleanup();
        return -1;
    }
    std::cout << "Login successfully, id: " << user_id << std::endl;

    NET_DVR_PREVIEWINFO preview_info;
    memset(&preview_info, 0, sizeof(NET_DVR_PREVIEWINFO));
    preview_info.hPlayWnd = NULL;
    preview_info.lChannel = 1;
    preview_info.dwStreamType = 0;
    preview_info.dwLinkMode = 0;
    preview_info.bBlocked = 1;

    LONG real_play_handle = NET_DVR_RealPlay_V40(user_id, &preview_info, NULL, NULL);
    if (real_play_handle < 0) {
        std::cout << "Failed to start preview, error code: " << NET_DVR_GetLastError() << std::endl;
        NET_DVR_Logout(user_id);
        NET_DVR_Cleanup();
        return -1;
    }
    std::cout << "Start previewing, handle: " << real_play_handle << std::endl;

    std::cout << "Press Enter to stop previewing..." << std::endl;
    std::cin.get();

    if (!NET_DVR_StopRealPlay(real_play_handle)) {
        std::cout << "Stop previewing, error code: " << NET_DVR_GetLastError() << std::endl;
    } else {
        std::cout << "Stop previewing successfully!" << std::endl;
    }

    if (!NET_DVR_Logout(user_id)) {
        std::cout << "Failed to logout, error code: " << NET_DVR_GetLastError() << std::endl;
    } else {
        std::cout << "Logout successfully!" << std::endl;
    }

    NET_DVR_Cleanup();
}
