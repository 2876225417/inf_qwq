


#include <iostream>
#include <HCNetSDK.h>


int main() { 
    NET_DVR_Init();
    DWORD ver = NET_DVR_GetSDKVersion();
    std::cout << "SDK build version: " << ver << std::endl;


}
