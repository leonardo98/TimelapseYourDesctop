#ifndef OUTPUT_VIDEO_H

#include <windows.h>
#include <vfw.h>

class OutputVideo
{
public:
	OutputVideo();
	bool Init(const char *filename, int widthFrame, int heightFrame, int frameRate = 30);
	bool AddFrame();
	bool Stop();
	int getWidth();
	int getHeight();
	int getFrameRate();
	char *getFrameBuffer();
	bool ChooseCodec(HWND hWnd);

private:
	bool CreateVideoStream(DWORD dwFrameRate, size_t WidthFrame, size_t HeightFrame, PAVISTREAM *ppStreams);
	bool MakeCompressedVideoStream(size_t WidthFrame, size_t HeightFrame, AVICOMPRESSOPTIONS *pVideoOptions, PAVISTREAM pStreamVideo, PAVISTREAM *ppCompressedStreamVideo);
	bool WriteFrameVideoCompress(PAVISTREAM pCompressedStreamVideo, BITMAPINFO *pBmp, DWORD dwFrameNum);
	void AviClean();

	AVICOMPRESSOPTIONS m_optionsVideo;

	PAVIFILE m_pAviFile;
	PAVISTREAM m_pStreamVideo;
	PAVISTREAM m_pCompressedStreamVideo;

	int m_widthFrame;
	int m_heightFrame;
	int m_frameRate;
	int m_currentFrame;
	void *m_pVideo;
	char *m_pBits;
	BITMAPINFOHEADER *m_pBitm;
};

#endif//OUTPUT_VIDEO_H