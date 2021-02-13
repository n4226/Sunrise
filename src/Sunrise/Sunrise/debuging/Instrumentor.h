//
// Basic instrumentation profiler by Cherno

// Usage: include this header file somewhere in your code (eg. precompiled header), and then use like:
//
// Instrumentor::Get().BeginSession("Session Name");        // Begin session 
// {
//     InstrumentationTimer timer("Profiled Scope Name");   // Place code like this in scopes you'd like to include in profiling
//     // Code
// }
// Instrumentor::Get().EndSession();                        // End Session
//
// You will probably want to macro-fy this, to switch on/off easily and use things like __FUNCSIG__ for the profile name.
//
#pragma once

#include "srpch.h"
#include "Sunrise/Sunrise/core/environment.h"

struct ProfileResult
{
    std::string Name;
    long long Start, End;
    uint32_t ThreadID;
    std::thread::id ThreadIDID;
};

struct InstrumentationSession
{
    std::string Name;
};

class Instrumentor
{
private:
    InstrumentationSession* m_CurrentSession;
    //libguarded::plain_guarded<std::ofstream> m_OutputStream;
    std::ofstream m_OutputStream;
    
    libguarded::plain_guarded<std::vector<ProfileResult>> pendingResults;

    int m_ProfileCount;
    std::thread::id main_thread_id;
public:
    Instrumentor()
        : m_CurrentSession(nullptr), m_ProfileCount(0), main_thread_id(std::this_thread::get_id())
    {
    }

    void BeginSession(const std::string& name, const std::string& filepath = "results.json")
    {
        //auto m_OutputStream = this->m_OutputStream.lock();
        m_OutputStream.open(filepath);
        WriteHeader();
        m_CurrentSession = new InstrumentationSession{ name };
    }

    void EndSession()
    {
#if SR_MULTI_THREADED_PROFILING
        {
            auto queue = pendingResults.lock();
            std::vector<ProfileResult>& results = *queue;
            for (ProfileResult& result : results) {
                WriteProfile(result);
            }
            queue->clear();
        }
#endif
        //auto m_OutputStream = this->m_OutputStream.lock();
        WriteFooter();
        m_OutputStream.close();
        delete m_CurrentSession;
        m_CurrentSession = nullptr;
        m_ProfileCount = 0;
    }


    void pendWriteProfile(const ProfileResult& result) {
        auto queue = pendingResults.lock();
        queue->push_back(result);
    }

    void WriteProfile(const ProfileResult& result)
    {
        //auto m_OutputStream = this->m_OutputStream.lock();
        


        

        std::string name = result.Name;
        std::replace(name.begin(), name.end(), '"', '\'');
        
        auto mainThread = false;

        if (result.ThreadIDID == main_thread_id)
            mainThread = true;
#if !SR_MULTI_THREADED_PROFILING
        else return;
#endif
        

        if (m_ProfileCount++ > 0)
            m_OutputStream << ",";

        m_OutputStream << "{";
        m_OutputStream << "\"cat\":\"function\",";
        m_OutputStream << "\"dur\":" << (result.End - result.Start) << ',';
        m_OutputStream << "\"name\":\"" << name << "\",";
        m_OutputStream << "\"ph\":\"X\",";
        m_OutputStream << "\"pid\":0,";
        m_OutputStream << "\"tid\":" << (mainThread ? 0 : result.ThreadID) << ",";
        m_OutputStream << "\"ts\":" << result.Start;
        m_OutputStream << "}";

        m_OutputStream.flush();
    }

    void WriteHeader()
    {
        //auto m_OutputStream = this->m_OutputStream.lock();

        m_OutputStream << "{\"otherData\": {},\"traceEvents\":[";
        m_OutputStream.flush();
    }

    void WriteFooter()
    {
        //auto m_OutputStream = this->m_OutputStream.lock();
        m_OutputStream << "]}";
        m_OutputStream.flush();
    }

    static Instrumentor& Get()
    {
        static Instrumentor instance;
        return instance;
    }
};

class InstrumentationTimer
{
public:
    InstrumentationTimer(const char* name)
        : m_Name(name), m_Stopped(false)
    {
        m_StartTimepoint = std::chrono::high_resolution_clock::now();
    }

    ~InstrumentationTimer()
    {
        if (!m_Stopped)
            Stop();
    }

    void Stop()
    {
        auto endTimepoint = std::chrono::high_resolution_clock::now();

        long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch().count();
        long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

        uint32_t threadID = std::hash<std::thread::id>{}(std::this_thread::get_id());

#if SR_MULTI_THREADED_PROFILING
        Instrumentor::Get().pendWriteProfile({ m_Name, start, end, threadID, std::this_thread::get_id() });
#else
        Instrumentor::Get().WriteProfile({ m_Name, start, end, threadID, std::this_thread::get_id() });
#endif
        m_Stopped = true;
    }
private:
    const char* m_Name;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTimepoint;
    bool m_Stopped;
};

////////// VULKAN DEBUG MARKERs


///// do here