#pragma once

#include "AudioPluginInterface.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#if PLATFORM_WIN
#   include <windows.h>
#else
#   include <pthread.h>
#   define strcpy_s strcpy
#   define vsprintf_s vsprintf
#endif

namespace AudioPluginUtil
{

typedef int (*InternalEffectDefinitionRegistrationCallback)(UnityAudioEffectDefinition& desc);

const float kMaxSampleRate = 22050.0f;

//char* strnew(const char* src);

template<const int _LENGTH, typename T = float>
class RingBuffer
{
public:
    enum { LENGTH = _LENGTH };

    volatile int readpos;
    volatile int writepos;
    T buffer[LENGTH];

    inline bool Read(T& val)
    {
        int r = readpos;
        if (r == writepos)
            return false;
        r = (r == LENGTH - 1) ? 0 : (r + 1);
        val = buffer[r];
        readpos = r;
        return true;
    }

    inline void Skip(int num)
    {
        int r = readpos + num;
        if (r >= LENGTH)
            r -= LENGTH;
        readpos = r;
    }

    inline void SyncWritePos()
    {
        writepos = readpos;
    }

    inline bool Feed(const T& input)
    {
        int w = (writepos == LENGTH - 1) ? 0 : (writepos + 1);
        buffer[w] = input;
        writepos = w;
        return true;
    }

    inline int GetNumBuffered() const
    {
        int b = writepos - readpos;
        if (b < 0)
            b += LENGTH;
        return b;
    }

    inline void Clear()
    {
        writepos = 0;
        readpos = 0;
    }
};

class Mutex
{
public:
    Mutex();
    ~Mutex();
public:
    bool TryLock();
    void Lock();
    void Unlock();
protected:
#if PLATFORM_WIN
    CRITICAL_SECTION crit_sec;
#else
    pthread_mutex_t mutex;
#endif
};

class MutexScopeLock
{
public:
    MutexScopeLock(Mutex& _mutex, bool condition = true) : mutex(condition ? &_mutex : NULL) { if (mutex != NULL) mutex->Lock(); }
    ~MutexScopeLock() { if (mutex != NULL) mutex->Unlock(); }
protected:
    Mutex* mutex;
};

void RegisterParameter(
    UnityAudioEffectDefinition& desc,
    const char* name,
    const char* unit,
    float minval,
    float maxval,
    float defaultval,
    float displayscale,
    float displayexponent,
    int enumvalue,
    const char* description = NULL
    );

void InitParametersFromDefinitions(
    InternalEffectDefinitionRegistrationCallback registereffectdefcallback,
    float* params
    );

void DeclareEffect(
    UnityAudioEffectDefinition& desc,
    const char* name,
    UnityAudioEffect_CreateCallback createcallback,
    UnityAudioEffect_ReleaseCallback releasecallback,
    UnityAudioEffect_ProcessCallback processcallback,
    UnityAudioEffect_SetFloatParameterCallback setfloatparametercallback,
    UnityAudioEffect_GetFloatParameterCallback getfloatparametercallback,
    UnityAudioEffect_GetFloatBufferCallback getfloatbuffercallback,
    InternalEffectDefinitionRegistrationCallback registereffectdefcallback
    );

} // namespace AudioPluginUtil
