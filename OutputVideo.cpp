#include "OutputVideo.h"

#pragma comment (lib, "vfw32")

OutputVideo::OutputVideo()
{
	m_pAviFile = 0;
	m_pStreamVideo = 0;
	m_pCompressedStreamVideo = 0;
	m_currentFrame = 0;
}

bool OutputVideo::Stop()
{
    AviClean();
	free(m_pVideo);
	m_pAviFile = 0;
	return true;
}

bool OutputVideo::Init(const char *filename, int widthFrame, int heightFrame, int frameRate)
{
	if (m_pAviFile)
		return false;

	m_currentFrame = 0;
	m_widthFrame = widthFrame;
	m_heightFrame = heightFrame;
	m_frameRate = frameRate;

	AVIFileInit();
	if (AVIFileOpen(&m_pAviFile, TEXT(filename), OF_WRITE | OF_CREATE, NULL) != AVIERR_OK)
		return false;

	if (!CreateVideoStream(frameRate, widthFrame, heightFrame, &m_pStreamVideo))
	{
		AviClean();
		return false;
	}

	if (!MakeCompressedVideoStream(widthFrame, heightFrame, &m_optionsVideo, m_pStreamVideo, &m_pCompressedStreamVideo))
	{
		AviClean();
		return false;
	}

    const DWORD sizeVideoFrame = widthFrame * heightFrame * 3;
    m_pVideo = malloc(sizeVideoFrame + sizeof(BITMAPINFOHEADER));

	m_pBitm = (BITMAPINFOHEADER*)m_pVideo;
    ZeroMemory(m_pBitm, sizeof(BITMAPINFOHEADER));
    
	m_pBitm->biWidth = widthFrame;
    m_pBitm->biHeight = heightFrame;
    m_pBitm->biBitCount = 24;
    m_pBitm->biPlanes = 1;
    m_pBitm->biSize = sizeof(BITMAPINFOHEADER);
    m_pBitm->biCompression = BI_RGB;
    m_pBitm->biSizeImage = sizeVideoFrame;

    m_pBits = (char*)m_pVideo + sizeof(BITMAPINFOHEADER);

	return true;
}

int OutputVideo::getWidth() {
	return m_widthFrame;
}

int OutputVideo::getHeight() {
	return m_heightFrame;
}

int OutputVideo::getFrameRate() {
	return m_frameRate;
}

char *OutputVideo::getFrameBuffer() {
	return m_pBits;
}
bool OutputVideo::ChooseCodec(HWND hWnd)
{
    COMPVARS cv;
    ZeroMemory(&cv, sizeof(cv));
    cv.cbSize = sizeof(cv);

    if (ICCompressorChoose(hWnd, ICMF_CHOOSE_DATARATE |
        ICMF_CHOOSE_KEYFRAME, NULL, NULL, &cv, ("Выбор и настройка кодека")))
    {
        ZeroMemory(&m_optionsVideo, sizeof(AVICOMPRESSOPTIONS));

        m_optionsVideo.fccType = streamtypeVIDEO;
        m_optionsVideo.fccHandler = cv.fccHandler;

        if (cv.lKey)
        {
            m_optionsVideo.dwKeyFrameEvery = cv.lKey;
            m_optionsVideo.dwFlags |= AVICOMPRESSF_KEYFRAMES;
        }

        m_optionsVideo.dwQuality = cv.lQ;

        if (cv.lDataRate)
        {
            m_optionsVideo.dwFlags |= AVICOMPRESSF_DATARATE;
            m_optionsVideo.dwBytesPerSecond = cv.lDataRate * 1024;
        }

        ICCompressorFree(&cv);
        return true;
    }
    return false;
}

bool OutputVideo::CreateVideoStream(DWORD dwFrameRate, size_t WidthFrame, size_t HeightFrame, PAVISTREAM *pm_pStreamVideo)
{
    AVISTREAMINFO sStreamInfo;
    ZeroMemory(&sStreamInfo, sizeof(sStreamInfo));
    sStreamInfo.fccType = streamtypeVIDEO;
    sStreamInfo.fccHandler = 0;
    sStreamInfo.dwScale = 1;
    sStreamInfo.dwRate = dwFrameRate;
    sStreamInfo.dwSuggestedBufferSize = 0;
    SetRect(&sStreamInfo.rcFrame, 0, 0, WidthFrame, HeightFrame);
    return (AVIERR_OK == AVIFileCreateStream(m_pAviFile, pm_pStreamVideo, &sStreamInfo));
}

bool OutputVideo::MakeCompressedVideoStream(size_t WidthFrame, size_t HeightFrame, AVICOMPRESSOPTIONS *pVideoOptions,
    PAVISTREAM m_pStreamVideo, PAVISTREAM *ppCompressedStreamVideo)
{	
	BITMAPINFO bi;
    ZeroMemory(&bi, sizeof(BITMAPINFO));
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = WidthFrame;
    bi.bmiHeader.biHeight = HeightFrame;
    bi.bmiHeader.biSizeImage = WidthFrame * HeightFrame * 3;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 24;
    bi.bmiHeader.biCompression = BI_RGB;
    void *pBits = ((LPBYTE)&bi) + bi.bmiHeader.biSize + bi.bmiHeader.biClrUsed * sizeof(RGBQUAD);
    if (AVIERR_OK == AVIMakeCompressedStream(ppCompressedStreamVideo, m_pStreamVideo, pVideoOptions, NULL))
    {
        if (AVIERR_OK == AVIStreamSetFormat(*ppCompressedStreamVideo, 0, &bi, ((LPBYTE)pBits) - ((LPBYTE)&bi)))
            return true;

        AVIStreamRelease(*ppCompressedStreamVideo);
        m_pCompressedStreamVideo = 0;
    }

    bi.bmiHeader.biBitCount = 16;
    if (AVIERR_OK == AVIMakeCompressedStream(ppCompressedStreamVideo, m_pStreamVideo, pVideoOptions, NULL))
    {
        if (AVIERR_OK == AVIStreamSetFormat(*ppCompressedStreamVideo, 0, &bi, ((LPBYTE)pBits) - ((LPBYTE)&bi)))
            return true;

        AVIStreamRelease(*ppCompressedStreamVideo);
        m_pCompressedStreamVideo = 0;
    }

    bi.bmiHeader.biBitCount = 8;
    if (AVIERR_OK == AVIMakeCompressedStream(ppCompressedStreamVideo, m_pStreamVideo, pVideoOptions, NULL))
    {
        if (AVIERR_OK == AVIStreamSetFormat(*ppCompressedStreamVideo, 0, &bi, ((LPBYTE)pBits) - ((LPBYTE)&bi)))
            return true;

        AVIStreamRelease(*ppCompressedStreamVideo);
        m_pCompressedStreamVideo = 0;
    }

    return false;
}

bool OutputVideo::AddFrame()
{
    if (!WriteFrameVideoCompress(m_pCompressedStreamVideo, (BITMAPINFO*)m_pBitm, m_currentFrame++))
        return false;
	return true;
}


bool OutputVideo::WriteFrameVideoCompress(PAVISTREAM m_pCompressedStreamVideo, BITMAPINFO *pBmp, DWORD dwFrameNum)
{
    return (AVIERR_OK == (AVIStreamWrite(m_pCompressedStreamVideo, dwFrameNum, 1,
        ((LPBYTE)pBmp) + pBmp->bmiHeader.biSize + pBmp->bmiHeader.biClrUsed * sizeof(RGBQUAD),
        pBmp->bmiHeader.biSizeImage, 0, NULL, NULL)));
}

void OutputVideo::AviClean()
{
    if (m_pStreamVideo)
    {
        AVIStreamRelease(m_pStreamVideo);
        m_pStreamVideo = 0;
    }

    if (m_pCompressedStreamVideo)
    {
        AVIStreamRelease(m_pCompressedStreamVideo);
        m_pCompressedStreamVideo = 0;
    }

    if (m_pAviFile)
    {
        AVIFileRelease(m_pAviFile);
        m_pAviFile = 0;
    }

    AVIFileExit();
}