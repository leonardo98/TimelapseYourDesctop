#include "CaptureAnImage.h"

#include <Wingdi.h>
#include <cstdio>

CaptureWindow::CaptureWindow(HWND hWnd)
{
    m_hWnd = hWnd;
    init();
}

CaptureWindow::~CaptureWindow()
{
    release();
}

int CaptureWindow::init()
{
    m_hdcScreen = NULL;
    m_hdcMemDC = NULL;
    m_hbmScreen = NULL;
    m_hDIB = NULL;

    // Retrieve the handle to a display device context for the client 
    // area of the window. 
    m_hdcScreen = GetDC(m_hWnd);

    // Create a compatible DC which is used in a BitBlt from the window DC
    m_hdcMemDC = CreateCompatibleDC(m_hdcScreen);

    if (!m_hdcMemDC)
    {
        MessageBox(m_hWnd, TEXT("CreateCompatibleDC has failed"), TEXT("Failed"), MB_OK);
        release();
        return -1;
    }

    // Create a compatible bitmap from the Window DC
    m_hbmScreen = CreateCompatibleBitmap(m_hdcScreen, width(), height());

    if (!m_hbmScreen)
    {
        MessageBox(m_hWnd, TEXT("CreateCompatibleBitmap Failed"), TEXT("Failed"), MB_OK);
        release();
        return -1;
    }

    // Select the compatible bitmap into the compatible memory DC.
    SelectObject(m_hdcMemDC, m_hbmScreen);


    DWORD dwBmpSize = ((width() * 32 + 31) / 32) * 4 * height();

    // Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that 
    // call HeapAlloc using a handle to the process's default heap. Therefore, GlobalAlloc and LocalAlloc 
    // have greater overhead than HeapAlloc.
    m_hDIB = GlobalAlloc(GHND, dwBmpSize);
    m_lpbitmap = (char *)GlobalLock(m_hDIB);

    return 0;
}

unsigned int CaptureWindow::width()
{
    static int w = (GetSystemMetrics(SM_CXSCREEN) / 8) * 8;
    return w;
}

unsigned int CaptureWindow::height()
{
    return GetSystemMetrics(SM_CYSCREEN);
}

void CaptureWindow::release()
{
    DeleteObject(m_hbmScreen);
    DeleteObject(m_hdcMemDC);
    ReleaseDC(NULL, m_hdcScreen);

    if (m_hDIB)
    {
        //Unlock and Free the DIB from the heap
        GlobalUnlock(m_hDIB);
        GlobalFree(m_hDIB);
    }
}

int CaptureWindow::capture(char *data)
{
    // Bit block transfer into our compatible memory DC.
    if (!BitBlt(m_hdcMemDC,
        0, 0,
        width(), height(),
        m_hdcScreen,
        0, 0,
        SRCCOPY))
    {
        MessageBox(m_hWnd, TEXT("BitBlt has failed"), TEXT("Failed"), MB_OK);
        release();
        return -1;
    }

    BITMAPINFOHEADER   bi;

    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width();
    bi.biHeight = height();
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    // Gets the "bits" from the bitmap and copies them into a buffer 
    // which is pointed to by lpbitmap.
    GetDIBits(m_hdcScreen, m_hbmScreen, 0,
        height(),
        m_lpbitmap,
        (BITMAPINFO *)&bi, DIB_RGB_COLORS);

    int h = height();
    int w = width();
    for (int i = 0; i < h; i++)
    {
        for (int j = 0; j < w; j++)
        {
            data[j * 3 + i * 3 * w] = m_lpbitmap[j * 4 + i * 4 * w];
            data[j * 3 + i * 3 * w + 1] = m_lpbitmap[j * 4 + i * 4 * w + 1];
            data[j * 3 + i * 3 * w + 2] = m_lpbitmap[j * 4 + i * 4 * w + 2];
        }
    }

    return 0;
}
