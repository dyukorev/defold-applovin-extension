#pragma once

#include <dmsdk/dlib/array.h>
#include <dmsdk/dlib/mutex.h>
#include <dmsdk/script/script.h>
#include <dmsdk/extension/extension.h>
struct lua_State;

namespace dmApplovin {
    enum EventType {
        EVENT_SDK_INITIALIZED = 1,
        EVENT_AD_LOADED = 2,
        EVENT_AD_LOAD_FAILED = 3,
        EVENT_AD_DISPLAY_FAILED = 4,
        EVENT_AD_DISPLAYED = 5,
        EVENT_AD_CLICKED = 6,
        EVENT_AD_HIDDEN = 7,
        EVENT_REWARDED_VIDEO_STARTED = 8,
        EVENT_REWARDED_VIDEO_COMPLETED = 9,
        EVENT_USER_REWARDED = 10,
        EVENT_AD_EXPANDED = 11,
        EVENT_AD_COLLAPSED = 12
    };


    struct ApplovinEvent
    {
        ApplovinEvent()
        {
            memset(this, 0, sizeof(ApplovinEvent));
        }

        dmScript::LuaCallbackInfo*  m_Callback;
        const char*                 m_Results;
        const char*                 m_Error;
        int                         m_ErrorCode;
        uint8_t                     m_Type;
        uint16_t                    m_State;
    };

    struct EventQueue
    {
        dmArray<ApplovinEvent>      m_Events;
        dmMutex::HMutex             m_Mutex;
    };

    struct ApplovinListener
    {
        ApplovinListener() : m_L(0), m_Callback(LUA_NOREF), m_Self(LUA_NOREF) {}
        lua_State* m_L;
        int        m_Callback;
        int        m_Self;
    };

    typedef void (*ApplovinProcessingFn)(ApplovinEvent* event, void* ctx);

    void QueueCreate(EventQueue* queue);
    void QueueDestroy(EventQueue* queue);
    void QueuePush(EventQueue* queue, ApplovinEvent* event);
    void QueueFlush(EventQueue* queue, ApplovinListener* cbk);

    void EventHandler(ApplovinEvent* event, ApplovinListener* cbk);
    void RegisterCallback(lua_State* L, int index, ApplovinListener* cbk);
    void UnregisterCallback(ApplovinListener* cbk);
}