#include "CaptureAnImage.h"

#include <Wingdi.h>
#include <cstdio>

int CaptureAnImage(HWND hWnd)
{
	HDC hdcScreen;
	HDC hdcMemDC = NULL;
	HBITMAP hbmScreen = NULL;

	// Retrieve the handle to a display device context for the client 
	// area of the window. 
	hdcScreen = GetDC(NULL);

	// Create a compatible DC which is used in a BitBlt from the window DC
	hdcMemDC = CreateCompatibleDC(hdcScreen);

	if (!hdcMemDC)
	{
		MessageBox(hWnd, TEXT("CreateCompatibleDC has failed"), TEXT("Failed"), MB_OK);
		goto done;
	}

	// Create a compatible bitmap from the Window DC
	hbmScreen = CreateCompatibleBitmap(hdcScreen, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));

	if (!hbmScreen)
	{
		MessageBox(hWnd, TEXT("CreateCompatibleBitmap Failed"), TEXT("Failed"), MB_OK);
		goto done;
	}

	// Select the compatible bitmap into the compatible memory DC.
	SelectObject(hdcMemDC, hbmScreen);

	// Bit block transfer into our compatible memory DC.
	if (!BitBlt(hdcMemDC,
		0, 0,
		GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
		hdcScreen,
		0, 0,
		SRCCOPY))
	{
		MessageBox(hWnd, TEXT("BitBlt has failed"), TEXT("Failed"), MB_OK);
		goto done;
	}

/*
	// Get the BITMAP from the HBITMAP
	BITMAP bmpScreen;
	GetObject(hbmScreen, sizeof(BITMAP), &bmpScreen);

	BITMAPFILEHEADER   bmfHeader;
	BITMAPINFOHEADER   bi;

	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = bmpScreen.bmWidth;
	bi.biHeight = bmpScreen.bmHeight;
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	DWORD dwBmpSize = ((bmpScreen.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmpScreen.bmHeight;

	// Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that 
	// call HeapAlloc using a handle to the process's default heap. Therefore, GlobalAlloc and LocalAlloc 
	// have greater overhead than HeapAlloc.
	HANDLE hDIB = GlobalAlloc(GHND, dwBmpSize);
	char *lpbitmap = (char *)GlobalLock(hDIB);

	// Gets the "bits" from the bitmap and copies them into a buffer 
	// which is pointed to by lpbitmap.
	GetDIBits(hdcScreen, hbmScreen, 0,
		(UINT)bmpScreen.bmHeight,
		lpbitmap,
		(BITMAPINFO *)&bi, DIB_RGB_COLORS);

	// A file is created, this is where we will save the screen capture.
	HANDLE hFile = CreateFile(TEXT("captured.bmp"),
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL);

	// Add the size of the headers to the size of the bitmap to get the total file size
	DWORD dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	//Offset to where the actual bitmap bits start.
	bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);

	//Size of the file
	bmfHeader.bfSize = dwSizeofDIB;

	//bfType must always be BM for Bitmaps
	bmfHeader.bfType = 0x4D42; //BM   

	DWORD dwBytesWritten = 0;
	WriteFile(hFile, (LPSTR)&bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
	WriteFile(hFile, (LPSTR)&bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
	WriteFile(hFile, (LPSTR)lpbitmap, dwBmpSize, &dwBytesWritten, NULL);

	//Unlock and Free the DIB from the heap
	GlobalUnlock(hDIB);
	GlobalFree(hDIB);

	//Close the handle for the file that was created
	CloseHandle(hFile);
	*/

	//Clean up
done:
	DeleteObject(hbmScreen);
	DeleteObject(hdcMemDC);
	ReleaseDC(NULL, hdcScreen);

	return 0;
}

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
	m_hbmScreen = CreateCompatibleBitmap(m_hdcScreen, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));

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
	return GetSystemMetrics(SM_CXSCREEN);
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
		GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
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
