// rtPerfTimer.h CopyRight 2005-2006 John Robinson
#pragma once

#ifndef RTPERFTIMERPREFIX
#define RTPERFTIMERPREFIX "rtPerfTimer:"
#endif


#ifdef RT_PLATFORM_WINDOWS
static bool gTimerFreqInitialized = false;
static LARGE_INTEGER gTimerFreq;
#endif

inline double rtGetMilliseconds()
{
#ifdef RT_PLATFORM_WINDOWS
    if (!gTimerFreqInitialized)
    {
        ::QueryPerformanceFrequency(&gTimerFreq);
        gTimerFreqInitialized = true;
    }

    LARGE_INTEGER t;

    QueryPerformanceCounter(&t);

    return ((double)t.QuadPart/(double)gTimerFreq.QuadPart) * 1000.0;
#else
    return 0;
#endif
}

inline double rtGetFPS(double ms, int count)
{
#ifdef RT_PLATFORM_WINDOWS
    return count * 1000 / ms;
#else
    return 0;
#endif
}

class rtPerfTimer
{
public:
    rtPerfTimer()
    {
#ifdef RT_PLATFORM_WINDOWS
        ::QueryPerformanceFrequency(&mFreq);
        init();
#endif
    }

#if 0
    rtPerfTimer(char* message)
    {
        ::QueryPerformanceFrequency(&mFreq);
       // init(message);
    }
#endif

#if 0
    void init()
    {
        init("\n");
    }
#endif

  //  void init(const char* format, ...)
    void init()
    {
#ifdef RT_PLATFORM_WINDOWS
        ::QueryPerformanceCounter(&mInitCount);
        mLastCount = mInitCount;
        markIndex = 0;
#if 0
        char buffer[1024];
        buffer[0] = 0;

        sprintf(buffer, "%s: mark[%d]: ", RTPERFTIMERPREFIX, markIndex++); 

        va_list ptr; va_start(ptr, format);
        //strcpy(buffer, RTLOGPREFIX);
        if (_vsnprintf(buffer+strlen(buffer), sizeof(buffer)-strlen(buffer)-1, format, ptr) < 1)
        {
            strcpy(buffer, "rtPerfTimer buffer overflow\n");
        }
        OutputDebugStringA(buffer);
        va_end(ptr);     
#endif
#endif
    }

#ifdef RT_PLATFORM_WINDOWS
    void mark()
    {
        mark("\n");
    }
#endif

#ifdef RT_PLATFORM_WINDOWS
    rtString mark(const char* format, ...)
    {
#ifdef RT_PLATFORM_WINDOWS
        LARGE_INTEGER currentCount;
        ::QueryPerformanceCounter(&currentCount);

        //unsigned long initDelta = (unsigned long)(((double)(currentCount.QuadPart-mInitCount.QuadPart)/(double)mFreq.QuadPart)*1000);
        double initDelta = ((double)(currentCount.QuadPart-mInitCount.QuadPart)/(double)mFreq.QuadPart) * 1000;
        //unsigned long delta = (unsigned long)(((double)(currentCount.QuadPart-mLastCount.QuadPart)/(double)mFreq.QuadPart)*1000);
        
        char buffer[1024];
        buffer[0] = 0;

#if 0
        sprintf(buffer, "%s: mark[%d]: init(%ldms) mark[%d](%ldms): ",
            RTPERFTIMERPREFIX, markIndex, initDelta, markIndex-1, delta); 
#else
#if 0
        sprintf(buffer, "%s: mark[%d]: init(%ldms): ",
            RTPERFTIMERPREFIX, markIndex, initDelta); 
#else
        sprintf(buffer, "%s: mark[%d]: init(%.3g ms): ",
            RTPERFTIMERPREFIX, markIndex, initDelta); 
#endif
#endif
        markIndex++;

        va_list ptr; va_start(ptr, format);
        //strcpy(buffer, RTLOGPREFIX);
        if (_vsnprintf(buffer+strlen(buffer), sizeof(buffer)-strlen(buffer)-1, format, ptr) < 1)
        {
            strcpy(buffer, "rtPerfTimer buffer overflow\n");
        }
        OutputDebugStringA(buffer);
        va_end(ptr);        

//        ::MessageBoxA(NULL, buffer, "", MB_OK);

        mLastCount = currentCount;

        return buffer;
#else
	return "";
#endif
    }

    rtString markFPS(long count)
    {
        LARGE_INTEGER currentCount;
        ::QueryPerformanceCounter(&currentCount);

//        unsigned long initDelta = (unsigned long)(((double)(currentCount.QuadPart-mInitCount.QuadPart)/(double)mFreq.QuadPart)*1000);
//        unsigned long delta = (unsigned long)(((double)(currentCount.QuadPart-mLastCount.QuadPart)/(double)mFreq.QuadPart)*1000);
        double initDelta = ((double)(currentCount.QuadPart-mInitCount.QuadPart)/(double)mFreq.QuadPart) * 1000;
        
        char buffer[1024];
        buffer[0] = 0;
        
        if (initDelta > 0)
        {
            sprintf(buffer, "count(%d) time(%3.3f ms) rate(%3.3f fps)",
                count, initDelta, (double)(count * 1000) / initDelta); 
        }
        else 
        {
            sprintf(buffer, "count(%d) time(%d ms) (really fast)",
                count, initDelta); 
        }

        markIndex++;

        OutputDebugStringA(RTPERFTIMERPREFIX);
        OutputDebugStringA(buffer);
        OutputDebugStringA("\n");

//        ::MessageBoxA(NULL, buffer, "", MB_OK);

        mLastCount = currentCount;

        return buffer;
    }
#endif

private:
#ifdef RT_PLATFORM_WINDOWS
    LARGE_INTEGER mFreq;
    LARGE_INTEGER mInitCount;
    LARGE_INTEGER mLastCount;
    int markIndex;
#endif
};

