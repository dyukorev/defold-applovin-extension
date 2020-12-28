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

    void QueueFlush(EventQueue* queue, ApplovinListener* m_Callback)
    {
        if (queue->m_Events.Empty())
            return;
        DM_MUTEX_SCOPED_LOCK(queue->m_Mutex);
        for(uint32_t i = 0; i != queue->m_Events.Size(); ++i)
        {
            ApplovinEvent event = queue->m_Events[i];
            EventHandler(&event, m_Callback);
        }
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

    void EventHandler(ApplovinEvent* event, ApplovinListener* cbk)
    {
        dmLogWarning("Handle event %d", event->m_Type);
        if(cbk->m_Callback == LUA_NOREF)
        {
            dmLogWarning("No ref on callback");
            return;
        }

        lua_State* L = cbk->m_L;
        int top = lua_gettop(L);

        lua_rawgeti(L, LUA_REGISTRYINDEX, cbk->m_Callback);

        // Setup self (the script instance)
        lua_rawgeti(L, LUA_REGISTRYINDEX, cbk->m_Self);
        lua_pushvalue(L, -1);

        dmScript::SetInstance(L);

        lua_pushnumber(L, event->m_Type);
        pushResult(L, event);
        pushError(L, event->m_Error, event->m_ErrorCode);

        int number_of_arguments = 4; // instance + 2
        int ret = lua_pcall(L, number_of_arguments, 0, 0);
        if(ret != 0) {
            dmLogError("Error running callback: %s", lua_tostring(L, -1));
            lua_pop(L, 1);
        }

        free((void*)event->m_Results);
        free((void*)event->m_Error);

        assert(top == lua_gettop(L));
    }

    void RegisterCallback(lua_State* L, int index, ApplovinListener* cbk)
    {
        dmLogWarning("Register callback");
        if(cbk->m_Callback != LUA_NOREF)
        {
            dmScript::Unref(cbk->m_L, LUA_REGISTRYINDEX, cbk->m_Callback);
            dmScript::Unref(cbk->m_L, LUA_REGISTRYINDEX, cbk->m_Self);
        }

        cbk->m_L = L;

        luaL_checktype(L, index, LUA_TFUNCTION);
        lua_pushvalue(L, index);
        cbk->m_Callback = dmScript::Ref(L, LUA_REGISTRYINDEX);

        dmScript::GetInstance(L);
        cbk->m_Self = dmScript::Ref(L, LUA_REGISTRYINDEX);
    }

    void UnregisterCallback(ApplovinListener* cbk)
    {
        if(cbk->m_Callback != LUA_NOREF)
        {
            dmScript::Unref(cbk->m_L, LUA_REGISTRYINDEX, cbk->m_Callback);
            dmScript::Unref(cbk->m_L, LUA_REGISTRYINDEX, cbk->m_Self);
            cbk->m_Callback = LUA_NOREF;
        }
    }
}