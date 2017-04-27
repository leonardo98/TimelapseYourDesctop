#include "CaptureAnImage.h"
#include "OutputVideo.h"

#include <stdio.h>

OutputVideo ov;
CaptureWindow screen(NULL);

int main(int argc, char* argv[])
{
    if (!ov.ChooseCodec(0))
        return 0;

    if (!ov.Init("video.avi", screen.width(), screen.height(), 24))
        return 0;

    printf("Recording. Press <Esc> to exit\n");

    MSG msg;
    memset(&msg, 0, sizeof(MSG));

    HWND hWnd = GetConsoleWindow();

    int i = 0;
    while (true)
    {
        if (screen.capture(ov.getFrameBuffer()) != 0)
        {
            break;
        }

        if (!ov.AddFrame())
            break;
        ++i;

        if (!(i % (ov.getFrameRate())))
            printf(".");

        if (GetAsyncKeyState(VK_ESCAPE))
        {
            break;
        }
    }

    ov.Stop();
}
