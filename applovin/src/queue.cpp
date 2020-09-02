#include "queue.h"

#include <stdlib.h>
#include <dmsdk/dlib/mutex.h>
#include <dmsdk/dlib/json.h>
#include <dmsdk/dlib/log.h>

namespace dmApplovin
{
    void QueueCreate(EventQueue* queue)
    {
        queue->m_Mutex = dmMutex::New();
    }

    void QueueDestroy(EventQueue* queue)
    {
        dmMutex::Delete(queue->m_Mutex);
    }

    void QueuePush(EventQueue* queue, ApplovinEvent* event)
    {
        dmLogError("Event added to queue %d", event->m_Type);
        DM_MUTEX_SCOPED_LOCK(queue->m_Mutex);
        if (queue->m_Events.Full())
        {
            queue->m_Events.OffsetCapacity(8);
        }
        queue->m_Events.Push(*event);
    }

    void QueueFlush(EventQueue* queue, void* ctx)
    {
        if (queue->m_Events.Empty())
            return;
        DM_MUTEX_SCOPED_LOCK(queue->m_Mutex);
        queue->m_Events.Map(dmApplovin::EventHandler, ctx);
        queue->m_Events.SetSize(0);
    }

    static void pushResult(lua_State* L, ApplovinEvent* event)
    {
        bool is_fail = false;
        dmJson::Document doc;
        dmJson::Result r = dmJson::Parse((const char*) event->m_Results, &doc);
        if (r == dmJson::RESULT_OK && doc.m_NodeCount > 0) {
            char error_str_out[128];
            if (dmScript::JsonToLua(L, &doc, 0, error_str_out, sizeof(error_str_out)) < 0) {
                dmLogError("Failed converting object JSON to Lua: %s", error_str_out);
                is_fail = true;
            }
        } else {
            dmLogError("Failed to parse JSON object(%d): (%s)", r, (const char*) event->m_Results);
            is_fail = true;
        }
        dmJson::Free(&doc);
        if (is_fail) {
            lua_pushnil(L);
        }
    }

    static void pushError(lua_State* L, const char* error, int error_code) {
        if (error != 0) {
            lua_newtable(L);
            lua_pushstring(L, "error");
            lua_pushstring(L, error);
            lua_rawset(L, -3);
            lua_pushstring(L, "error_code");
            lua_pushnumber(L, error_code);
            lua_rawset(L, -3);
        } else {
            lua_pushnil(L);
        }
    }

    void EventHandler(ApplovinEvent* event, void* ctx)
    {
        dmLogError("Processing event %d", event->m_Type);
        // Checking that we're in the correct context (in case of a non shared Lua state)
        lua_State* L = dmScript::GetCallbackLuaContext(event->m_Callback);
        int top = lua_gettop(L);

        DM_LUA_STACK_CHECK(L, 0);

        if (!dmScript::SetupCallback(event->m_Callback)) {
            assert(top == lua_gettop(L));
            return;
        }
        lua_pushnumber(L, event->m_Type);
        pushResult(L, event);
        pushError(L, event->m_Error, event->m_ErrorCode);

        dmScript::PCall(L, 4, 0);

        free((void*)event->m_Results);
        free((void*)event->m_Error);
        dmScript::TeardownCallback(event->m_Callback);

        assert(top == lua_gettop(L));
    }

}