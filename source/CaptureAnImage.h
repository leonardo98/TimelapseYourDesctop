#ifndef CAPTURE_AN_IMAGE
#define CAPTURE_AN_IMAGE

#include <Windows.h>
#include <windef.h>

class CaptureWindow
{
public:
    CaptureWindow(HWND hWnd);
    virtual ~CaptureWindow();

    int capture(char *data);

    unsigned int width();
    unsigned int height();
    unsigned char *data();

private:
    void release();
    int init();

    HWND m_hWnd;
    HDC m_hdcScreen;
    HDC m_hdcMemDC;
    HBITMAP m_hbmScreen;

    HANDLE m_hDIB;
    char *m_lpbitmap;
};

#endif//CAPTURE_AN_IMAGE
