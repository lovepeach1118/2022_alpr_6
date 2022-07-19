
#include <iostream>
#include <windows.h>
#include <tchar.h>

#include "LaptopModules.h"
#include "opencv2/opencv.hpp"
#include "DeviceEnumerator.h"
#include "json.hpp"

using namespace cv;
using namespace std;
using namespace client;
using json = nlohmann::json;

static bool getconchar(KEY_EVENT_RECORD& krec)
{
    DWORD cc;
    INPUT_RECORD irec;
    HANDLE h = GetStdHandle(STD_INPUT_HANDLE);

    if (h == NULL)
    {
        return false; // console not found
    }

    for (; ; )
    {
        ReadConsoleInput(h, &irec, 1, &cc);
        if (irec.EventType == KEY_EVENT
            && ((KEY_EVENT_RECORD&)irec.Event).bKeyDown
            )//&& ! ((KEY_EVENT_RECORD&)irec.Event).wRepeatCount )
        {
            krec = (KEY_EVENT_RECORD&)irec.Event;
            return true;
        }
    }
    return false; //future ????
}

Mode UIManager::GetVideoMode(void)
{
    KEY_EVENT_RECORD key;
    Mode mode = Mode::mNone;
    do
    {
        std::cout << "Select PlayBack File, Live Video or Image File" << std::endl;
        std::cout << "1 - PlayBack File" << std::endl;
        std::cout << "2 - Live Video" << std::endl;
        std::cout << "3 - Image File" << std::endl;
        std::cout << "E - Exit" << std::endl;

        getconchar(key);
        std::cout << key.uChar.AsciiChar << std::endl;
        if ((key.uChar.AsciiChar == 'E') || (key.uChar.AsciiChar == 'e')) break;
        else if (key.uChar.AsciiChar == '1') mode = Mode::mPlayback_Video;
        else if (key.uChar.AsciiChar == '2') mode = Mode::mLive_Video;
        else if (key.uChar.AsciiChar == '3') mode = Mode::mImage_File;
        else std::cout << "Invalid Input" << std::endl << std::endl;
    } while (mode == Mode::mNone);
    return(mode);
}

int UIManager::GetVideoDevice(void)
{
    int deviceID = -1;
    KEY_EVENT_RECORD key;
    int numdev;
    DeviceEnumerator de;
    std::map<int, Device> devices = de.getVideoDevicesMap();

    int* deviceid = new int[devices.size()];
    do {
        numdev = 0;
        std::cout << "Select video Device" << std::endl;
        for (auto const& device : devices)
        {
            deviceid[numdev] = device.first;
            std::cout << numdev + 1 << " - " << device.second.deviceName << std::endl;
            numdev++;
        }
        std::cout << "E - exit" << std::endl;
        getconchar(key);
        if ((key.uChar.AsciiChar == 'E') || (key.uChar.AsciiChar == 'e')) break;
        int value = static_cast<int>(key.uChar.AsciiChar) - 48;
        if ((value >= 1) && value <= numdev) deviceID = deviceid[value - 1];
        else std::cout << "Invalid Input" << std::endl << std::endl;
    } while (deviceID == -1);
    delete[] deviceid;
    return(deviceID);
}

VideoResolution UIManager::GetVideoResolution(void)
{
    VideoResolution vres = VideoResolution::rNone;
    KEY_EVENT_RECORD key;
    do
    {
        std::cout << "Select Video Resolution" << std::endl;
        std::cout << "1 - 640 x 480" << std::endl;
        std::cout << "2 - 1280 x 720" << std::endl;
        std::cout << "E - Exit" << std::endl;

        getconchar(key);
        std::cout << key.uChar.AsciiChar << std::endl;
        if ((key.uChar.AsciiChar == 'E') || (key.uChar.AsciiChar == 'e')) break;
        else if (key.uChar.AsciiChar == '1') vres = VideoResolution::r640X480;
        else if (key.uChar.AsciiChar == '2') vres = VideoResolution::r1280X720;
        else std::cout << "Invalid Input" << std::endl << std::endl;
    } while (vres == VideoResolution::rNone);

    return(vres);
}

bool UIManager::GetFileName(Mode mode, char filename[MAX_PATH])
{
    TCHAR CWD[MAX_PATH];
    bool retval = true;
    OPENFILENAME ofn;
    TCHAR szFile[MAX_PATH];
    ZeroMemory(&szFile, sizeof(szFile));
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;  // If you have a window to center over, put its HANDLE here
    if (mode == Mode::mImage_File) ofn.lpstrFilter = _T("Image Files\0*.png;*.jpg;*.tif;*.bmp;*.jpeg;*.gif\0Any File\0*.*\0");
    else if (mode == Mode::mPlayback_Video) ofn.lpstrFilter = _T("Video Files\0*.avi;*.mp4;*.webm;*.flv;*.mjpg;*.mjpeg\0Any File\0*.*\0");
    else ofn.lpstrFilter = _T("Text Files\0*.txt\0Any File\0*.*\0");
    ofn.lpstrFile = LPWSTR(szFile);
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = _T("Select a File, to Processs");
    ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;
    GetCurrentDirectory(MAX_PATH, CWD);
    if (GetOpenFileName(&ofn))
    {
        size_t output_size;
        wcstombs_s(&output_size, filename, MAX_PATH, ofn.lpstrFile, MAX_PATH);
    }
    else
    {
        // All this stuff below is to tell you exactly how you messed up above. 
        // Once you've got that fixed, you can often (not always!) reduce it to a 'user cancelled' assumption.
        switch (CommDlgExtendedError())
        {
        case CDERR_DIALOGFAILURE: std::cout << "CDERR_DIALOGFAILURE\n";   break;
        case CDERR_FINDRESFAILURE: std::cout << "CDERR_FINDRESFAILURE\n";  break;
        case CDERR_INITIALIZATION: std::cout << "CDERR_INITIALIZATION\n";  break;
        case CDERR_LOADRESFAILURE: std::cout << "CDERR_LOADRESFAILURE\n";  break;
        case CDERR_LOADSTRFAILURE: std::cout << "CDERR_LOADSTRFAILURE\n";  break;
        case CDERR_LOCKRESFAILURE: std::cout << "CDERR_LOCKRESFAILURE\n";  break;
        case CDERR_MEMALLOCFAILURE: std::cout << "CDERR_MEMALLOCFAILURE\n"; break;
        case CDERR_MEMLOCKFAILURE: std::cout << "CDERR_MEMLOCKFAILURE\n";  break;
        case CDERR_NOHINSTANCE: std::cout << "CDERR_NOHINSTANCE\n";     break;
        case CDERR_NOHOOK: std::cout << "CDERR_NOHOOK\n";          break;
        case CDERR_NOTEMPLATE: std::cout << "CDERR_NOTEMPLATE\n";      break;
        case CDERR_STRUCTSIZE: std::cout << "CDERR_STRUCTSIZE\n";      break;
        case FNERR_BUFFERTOOSMALL: std::cout << "FNERR_BUFFERTOOSMALL\n";  break;
        case FNERR_INVALIDFILENAME: std::cout << "FNERR_INVALIDFILENAME\n"; break;
        case FNERR_SUBCLASSFAILURE: std::cout << "FNERR_SUBCLASSFAILURE\n"; break;
        default: std::cout << "You cancelled.\n";
            retval = false;
        }
    }
    SetCurrentDirectory(CWD);

    if (retval) std::cout << "Filename is " << filename << std::endl;
    return(retval);
}

VideoSaveMode UIManager::GetVideoSaveMode(void)
{
    VideoSaveMode videosavemode = VideoSaveMode::vNone;
    KEY_EVENT_RECORD key;
    do
    {
        std::cout << "Select Video Save Mode" << std::endl;
        std::cout << "1 - No Save" << std::endl;
        std::cout << "2 - Save" << std::endl;
        std::cout << "3 - Save With No ALPR" << std::endl;
        std::cout << "E - Exit" << std::endl;

        getconchar(key);
        std::cout << key.uChar.AsciiChar << std::endl;
        if ((key.uChar.AsciiChar == 'E') || (key.uChar.AsciiChar == 'e')) exit(0);
        else if (key.uChar.AsciiChar == '1') videosavemode = VideoSaveMode::vNoSave;
        else if (key.uChar.AsciiChar == '2') videosavemode = VideoSaveMode::vSave;
        else if (key.uChar.AsciiChar == '3') videosavemode = VideoSaveMode::vSaveWithNoALPR;
        else std::cout << "Invalid Input" << std::endl << std::endl;
    } while (videosavemode == VideoSaveMode::vNone);

    return(videosavemode);
}

void UIManager::PrintErrMsg(std::string msg)
{
    std::cerr << msg << std::endl;
}

static void puttext_info(Mat plate, const char* d1, const char* d2, const char* d3,
    const char* d4, int x, int y)
{
    cv::putText(plate, d1,
        cv::Point(x, y),
        FONT_HERSHEY_COMPLEX_SMALL, 0.4,
        Scalar(211, 211, 211), 1, LINE_AA, false
    );

    cv::putText(plate, d2,
        cv::Point(x + 50, y),
        FONT_HERSHEY_COMPLEX_SMALL, 0.4,
        Scalar(0, 255, 0), 0, LINE_AA, false
    );

    cv::putText(plate, d3,
        cv::Point(x + 450, y),
        FONT_HERSHEY_COMPLEX_SMALL, 0.4,
        Scalar(255, 224, 145), 0, LINE_AA, false
    );

    cv::putText(plate, d4,
        cv::Point(x, y + 10),
        FONT_HERSHEY_COMPLEX_SMALL, 0.4,
        Scalar(20, 20, 255), 1.3, LINE_AA, false
    );
}

void UIManager::UpdateVinfo(string plate_number, int puid, Mat pimag, json jsonRetPlateInfo)
{
    Mat image, info, overlay;
    //imshow("Laptop Application", pimag);
    // image = imread(cropped_image);

    info.copyTo(overlay);

    // cv::Mat color(roi.size(), CV_8UC3, cv::Scalar(0, 125, 125)); yellowish color
    rectangle(overlay, Rect(10, 10, info.cols - 20, 115), Scalar(67, 67, 71), -1);

    double alpha = 0.7;
    addWeighted(overlay, alpha, info, 1 - alpha, 0, info);

    char owner_info[256];
    // string text = "FH7093,,02/20/2022,Robert Kennedy,02/18/1983,"PSC 3345, Box 2552","APO AE 39458",2004,Lexus,Intrepid,blue";

    char plate_num[8] = "FHF7093";
    char status[32] = "No Wants / Warrants";

    char reg_expiration[16] = "02/20/2022";
    char owner_name[32] = "Robert Kennedy";
    char owner_birthdate[16] = "02/18/1983";
    char owner_address_1[32] = "PSC 3345, Box 2552";
    char owner_address_2[32] = "APO AE 39458";
    char vehicle_year[5] = "2004";
    char vehicle_make[16] = "Lexus";
    char vehicle_model[16] = "Intrepid";
    char vehicle_color[8] = "blue";

    sprintf_s(owner_info, sizeof(owner_info), "%-8s %-8s %-8s %-8s %-8s",
        // status,
        reg_expiration,
        owner_name,
        owner_birthdate,
        owner_address_1,
        owner_address_2
    );

    char vehicle_info[256];
    sprintf_s(vehicle_info, sizeof(vehicle_info), "%-5s %-8s %-8s %-8s",
        vehicle_year,
        vehicle_make,
        vehicle_model,
        vehicle_color
    );

    puttext_info(info, plate_num, owner_info, vehicle_info, status, 15, 25);
    puttext_info(info, plate_num, owner_info, vehicle_info, status, 15, 45);
    puttext_info(info, plate_num, owner_info, vehicle_info, status, 15, 65);
    puttext_info(info, plate_num, owner_info, vehicle_info, status, 15, 85);
    puttext_info(info, plate_num, owner_info, vehicle_info, status, 15, 105);

    vconcat(image, overlay, image);
    image.copyTo(vinfo);
}

void UIManager::UpdateVideo(void)
{
    Mat ui;

    if (!vinfo.empty())
        hconcat(ui, video, vinfo);
    else
        video.copyTo(ui);

    imshow("Laptop Application", ui);
}

