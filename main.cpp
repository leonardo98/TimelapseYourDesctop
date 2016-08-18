#include "CaptureAnImage.h"

//author: Booster:Igor Spiridonov
//email: igwasm@rambler.ru
//record white noise video
#include <windows.h>
#include <vfw.h>
#include <math.h>
#include <stdio.h>
#pragma comment (lib, "vfw32")


bool ChooseCodec(HWND hWnd, AVICOMPRESSOPTIONS *pOptionsVideo);
bool CreateVideoStream(DWORD dwFrameRate, size_t WidthFrame, size_t HeightFrame, PAVISTREAM *ppStreamVideo);
bool MakeCompressedVideoStream(size_t WidthFrame, size_t HeightFrame, AVICOMPRESSOPTIONS *pVideoOptions, PAVISTREAM pStreamVideo, PAVISTREAM *ppCompressedStreamVideo);
bool WriteFrameVideoCompress(PAVISTREAM pCompressedStreamVideo, BITMAPINFO *pBmp, DWORD dwFrameNum);
//bool CreateAudioStream(const WAVEFORMATEX *pwf, PAVIFILE pAviFile, PAVISTREAM *ppStreamAudio);
//bool WriteFrameAudio(PAVISTREAM  pStreamAudio, void *pData, size_t sizeData, DWORD dwFrameNum);
void AviClean();

AVICOMPRESSOPTIONS OptionsVideo;
PAVIFILE pAviFile = 0;
PAVISTREAM pStreamVideo = 0;
PAVISTREAM pCompressedStreamVideo = 0;
//PAVISTREAM pStreamAudio = 0;

const int WidthFrame = 640;
const int HeightFrame = 480;
const int FrameRate = 30;
const int CountSecond = 10;
const int CountFrames = FrameRate * CountSecond;
//const int AmplitudeSound = 0x4000;


int main(int argc, char* argv[])
{
    if (!ChooseCodec(0, &OptionsVideo))
        return 0;

    AVIFileInit();
    if (AVIFileOpen(&pAviFile, TEXT("Video.avi"), OF_WRITE | OF_CREATE, NULL) != AVIERR_OK)
        return 0;

    if (!CreateVideoStream(FrameRate, WidthFrame, HeightFrame, &pStreamVideo))
    {
        AviClean();
        return 0;
    }

    if (!MakeCompressedVideoStream(WidthFrame, HeightFrame, &OptionsVideo, pStreamVideo, &pCompressedStreamVideo))
    {
        AviClean();
        return 0;
    }

    //WAVEFORMATEX wf;
    //ZeroMemory(&wf, sizeof(WAVEFORMATEX));
    //wf.cbSize = sizeof(WAVEFORMATEX);
    //wf.wFormatTag = WAVE_FORMAT_PCM;
    //wf.nChannels = 2;
    //wf.nSamplesPerSec = 44100;
    //wf.wBitsPerSample = 16;
    //wf.nBlockAlign = (wf.wBitsPerSample * wf.nChannels) / 8;
    //wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;

    //if (!CreateAudioStream(&wf, pAviFile, &pStreamAudio))
    //{
    //    AviClean();
    //    return 0;
    //}

    const DWORD sizeVideoFrame = WidthFrame * HeightFrame * 3;
    void *pVideo = malloc(sizeVideoFrame + sizeof(BITMAPINFOHEADER));
    BITMAPINFOHEADER *pBitm = (BITMAPINFOHEADER*)pVideo;
    ZeroMemory(pBitm, sizeof(BITMAPINFOHEADER));
    pBitm->biWidth = WidthFrame;
    pBitm->biHeight = HeightFrame;
    pBitm->biBitCount = 24;
    pBitm->biPlanes = 1;
    pBitm->biSize = sizeof(BITMAPINFOHEADER);
    pBitm->biCompression = BI_RGB;
    pBitm->biSizeImage = sizeVideoFrame;

    char *pBits = (char*)pVideo + sizeof(BITMAPINFOHEADER);

    //int CountSoundInFrame = wf.nAvgBytesPerSec / FrameRate;
    //DWORD *pSound = (DWORD*)malloc(CountSoundInFrame);

    printf("Recording ");

    for (int i = 0; i<CountFrames; i++)
    {
        //Video frame
        for (int i2 = 0; i2<(WidthFrame * HeightFrame); i2++)
        {
            unsigned int color = (((float)rand()) / RAND_MAX) * 0xffffff;
            pBits[i2 * 3] = ((char*)&color)[0];
            pBits[i2 * 3 + 1] = ((char*)&color)[1];
            pBits[i2 * 3 + 2] = ((char*)&color)[2];
        }

        ////Audio frame
        //for (int i3 = 0; i3<CountSoundInFrame / 4; i3++)
        //{
        //    unsigned short x = (((float)rand()) / RAND_MAX)*AmplitudeSound;
        //    unsigned short y = (((float)rand()) / RAND_MAX)*AmplitudeSound;
        //    ((WORD*)pSound)[i3 * 2] = x;
        //    ((WORD*)pSound)[i3 * 2 + 1] = y;
        //}

        if (!WriteFrameVideoCompress(pCompressedStreamVideo, (BITMAPINFO*)pBitm, i))
            break;
        //if (!WriteFrameAudio(pStreamAudio, pSound, CountSoundInFrame, i))
        //    break;

        if (!(i%FrameRate))
            printf(".");
    }
    //free(pSound);
    free(pVideo);
    AviClean();
}

bool ChooseCodec(HWND hWnd, AVICOMPRESSOPTIONS *pOptionsVideo)
{
    COMPVARS cv;
    ZeroMemory(&cv, sizeof(cv));
    cv.cbSize = sizeof(cv);

    if (ICCompressorChoose(hWnd, ICMF_CHOOSE_DATARATE |
        ICMF_CHOOSE_KEYFRAME, NULL, NULL, &cv, ("Выбор и настройка кодека")))
    {
        ZeroMemory(pOptionsVideo, sizeof(AVICOMPRESSOPTIONS));

        pOptionsVideo->fccType = streamtypeVIDEO;
        pOptionsVideo->fccHandler = cv.fccHandler;

        if (cv.lKey)
        {
            pOptionsVideo->dwKeyFrameEvery = cv.lKey;
            pOptionsVideo->dwFlags |= AVICOMPRESSF_KEYFRAMES;
        }

        pOptionsVideo->dwQuality = cv.lQ;

        if (cv.lDataRate)
        {
            pOptionsVideo->dwFlags |= AVICOMPRESSF_DATARATE;
            pOptionsVideo->dwBytesPerSecond = cv.lDataRate * 1024;
        }

        ICCompressorFree(&cv);
        return true;
    }
    return false;
}

bool CreateVideoStream(DWORD dwFrameRate, size_t WidthFrame, size_t HeightFrame, PAVISTREAM *ppStreamVideo)
{
    AVISTREAMINFO sStreamInfo;
    ZeroMemory(&sStreamInfo, sizeof(sStreamInfo));
    sStreamInfo.fccType = streamtypeVIDEO;
    sStreamInfo.fccHandler = 0;
    sStreamInfo.dwScale = 1;
    sStreamInfo.dwRate = dwFrameRate;
    sStreamInfo.dwSuggestedBufferSize = 0;
    SetRect(&sStreamInfo.rcFrame, 0, 0, WidthFrame, HeightFrame);
    return (AVIERR_OK == AVIFileCreateStream(pAviFile, ppStreamVideo, &sStreamInfo));
}

bool MakeCompressedVideoStream(size_t WidthFrame, size_t HeightFrame, AVICOMPRESSOPTIONS *pVideoOptions,
    PAVISTREAM pStreamVideo, PAVISTREAM *ppCompressedStreamVideo)
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
    if (AVIERR_OK == AVIMakeCompressedStream(ppCompressedStreamVideo, pStreamVideo, pVideoOptions, NULL))
    {
        if (AVIERR_OK == AVIStreamSetFormat(*ppCompressedStreamVideo, 0, &bi, ((LPBYTE)pBits) - ((LPBYTE)&bi)))
            return true;

        AVIStreamRelease(*ppCompressedStreamVideo);
        pCompressedStreamVideo = 0;
    }

    bi.bmiHeader.biBitCount = 16;
    if (AVIERR_OK == AVIMakeCompressedStream(ppCompressedStreamVideo, pStreamVideo, pVideoOptions, NULL))
    {
        if (AVIERR_OK == AVIStreamSetFormat(*ppCompressedStreamVideo, 0, &bi, ((LPBYTE)pBits) - ((LPBYTE)&bi)))
            return true;

        AVIStreamRelease(*ppCompressedStreamVideo);
        pCompressedStreamVideo = 0;
    }

    bi.bmiHeader.biBitCount = 8;
    if (AVIERR_OK == AVIMakeCompressedStream(ppCompressedStreamVideo, pStreamVideo, pVideoOptions, NULL))
    {
        if (AVIERR_OK == AVIStreamSetFormat(*ppCompressedStreamVideo, 0, &bi, ((LPBYTE)pBits) - ((LPBYTE)&bi)))
            return true;

        AVIStreamRelease(*ppCompressedStreamVideo);
        pCompressedStreamVideo = 0;
    }

    return false;
}

bool CreateAudioStream(const WAVEFORMATEX *pwf, PAVIFILE pAviFile, PAVISTREAM *ppStreamAudio)
{
    AVISTREAMINFO sStreamInfo;
    ZeroMemory(&sStreamInfo, sizeof(sStreamInfo));
    sStreamInfo.fccType = streamtypeAUDIO;
    sStreamInfo.dwScale = pwf->nBlockAlign;
    sStreamInfo.dwRate = pwf->nSamplesPerSec * sStreamInfo.dwScale;
    sStreamInfo.dwInitialFrames = 1;
    sStreamInfo.dwSampleSize = pwf->nBlockAlign;
    sStreamInfo.dwQuality = (DWORD)-1;
    if (AVIFileCreateStream(pAviFile, ppStreamAudio, &sStreamInfo) != AVIERR_OK)
        return false;
    return (AVIERR_OK == AVIStreamSetFormat(*ppStreamAudio, 0, (void*)pwf, sizeof(WAVEFORMATEX)));
}

bool WriteFrameVideoCompress(PAVISTREAM pCompressedStreamVideo, BITMAPINFO *pBmp, DWORD dwFrameNum)
{
    return (AVIERR_OK == (AVIStreamWrite(pCompressedStreamVideo, dwFrameNum, 1,
        ((LPBYTE)pBmp) + pBmp->bmiHeader.biSize + pBmp->bmiHeader.biClrUsed * sizeof(RGBQUAD),
        pBmp->bmiHeader.biSizeImage, 0, NULL, NULL)));
}

bool WriteFrameAudio(PAVISTREAM pStreamAudio, void *pData, size_t sizeData, DWORD dwFrameNum)
{
    return (AVIERR_OK == AVIStreamWrite(pStreamAudio, dwFrameNum,
        1, pData, sizeData, 0, NULL, NULL));
}

void AviClean()
{
    if (pStreamVideo)
    {
        AVIStreamRelease(pStreamVideo);
        pStreamVideo = 0;
    }

    if (pCompressedStreamVideo)
    {
        AVIStreamRelease(pCompressedStreamVideo);
        pCompressedStreamVideo = 0;
    }

    //if (pStreamAudio)
    //{
    //    AVIStreamRelease(pStreamAudio);
    //    pStreamAudio = 0;
    //}

    if (pAviFile)
    {
        AVIFileRelease(pAviFile);
        pAviFile = 0;
    }

    AVIFileExit();
}