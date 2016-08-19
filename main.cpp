#include "CaptureAnImage.h"
#include "OutputVideo.h"

#include <stdio.h>

OutputVideo ov;

int main(int argc, char* argv[])
{
    if (!ov.ChooseCodec(0))
        return 0;

	if (!ov.Init("video.avi", 640, 480))
		return 0;

    printf("Recording ");

	char *pBits = ov.getFrameBuffer();
    for (int i = 0; i < 300; i++)
    {
        //Video frame
        for (int i2 = 0; i2 < (ov.getWidth() * ov.getHeight()); i2++)
        {
            pBits[i2 * 3] = 
            pBits[i2 * 3 + 1] = 
            pBits[i2 * 3 + 2] = rand() % 256;
        }

		if (!ov.AddFrame())
			break;

        if (!(i%(ov.getFrameRate())))
            printf(".");
    }

	ov.Stop();
}
