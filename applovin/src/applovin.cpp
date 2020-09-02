#if defined(DM_PLATFORM_ANDROID) || defined(DM_PLATFORM_IOS)

#include "queue.h"
#include "applovin.h"
#include <dmsdk/sdk.h>
#include <dmsdk/dlib/log.h>

#define MODULE_NAME "applovin"
#define LIB_NAME "Applovin"

static int Applovin_createRewardedAd(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);

    luaL_checktype(L, 1, LUA_TSTRING);
    const char* ad_unit_id = luaL_checkstring(L, 1);
    dmScript::LuaCallbackInfo* callback = dmScript::CreateCallback(L, 2);


    Platform_createRewardedAd(L, callback, ad_unit_id);
    return 0;
}

static int Applovin_loadRewardedAd(lua_State* L)
{
    Platform_loadRewardedAd(L);
    return 0;
}

static int Applovin_showRewardedAd(lua_State* L)
{
    Platform_showRewardedAd(L);
    return 0;
}

static const luaL_reg Applovin_methods[] = {
    //{"createInterstitialAd", Applovin_createInterstitialAd},
    //{"loadInterstitialAd", Applovin_loadInterstitialAd},
    //{"showInterstitialAd", Applovin_showInterstitialAd},
    {"createRewardedAd", Applovin_createRewardedAd},
    {"loadRewardedAd", Applovin_loadRewardedAd},
    {"showRewardedAd", Applovin_showRewardedAd},
    {0, 0}
};

static void LuaInit(lua_State* L)
{
    printf("LuaInit %s Extension\n", MODULE_NAME);
    dmLogError("LuaInit Applovin Extension");
    int top = lua_gettop(L);
    luaL_register(L, MODULE_NAME, Applovin_methods);

    #define SETCONSTANT(name, val) \
    lua_pushnumber(L, (lua_Number) val); \
    lua_setfield(L, -2, #name);\

    SETCONSTANT(EVENT_SDK_INITIALIZED,      dmApplovin::EVENT_SDK_INITIALIZED);
    SETCONSTANT(EVENT_AD_LOADED,            dmApplovin::EVENT_AD_LOADED);
    SETCONSTANT(EVENT_AD_LOAD_FAILED,       dmApplovin::EVENT_AD_LOAD_FAILED);
    SETCONSTANT(EVENT_AD_DISPLAY_FAILED,    dmApplovin::EVENT_AD_DISPLAY_FAILED);
    SETCONSTANT(EVENT_AD_DISPLAYED,         dmApplovin::EVENT_AD_DISPLAYED);
    SETCONSTANT(EVENT_AD_CLICKED,           dmApplovin::EVENT_AD_CLICKED);
    SETCONSTANT(EVENT_AD_HIDDEN,            dmApplovin::EVENT_AD_HIDDEN);
    SETCONSTANT(EVENT_REWARDED_VIDEO_STARTED, dmApplovin::EVENT_REWARDED_VIDEO_STARTED);
    SETCONSTANT(EVENT_REWARDED_VIDEO_COMPLETED, dmApplovin::EVENT_REWARDED_VIDEO_COMPLETED);
    SETCONSTANT(EVENT_USER_REWARDED,            dmApplovin::EVENT_USER_REWARDED);
    SETCONSTANT(EVENT_AD_EXPANDED,              dmApplovin::EVENT_AD_EXPANDED);
    SETCONSTANT(EVENT_AD_COLLAPSED,             dmApplovin::EVENT_AD_COLLAPSED);
    #undef SETCONSTANT

    lua_pop(L, 1);
    assert(top == lua_gettop(L));
}

static dmExtension::Result AppInitializeApplovin(dmExtension::AppParams* params)
{
    const char* mediation_provider = dmConfigFile::GetString(params->m_ConfigFile, "applovin.mediation_provider", "max");
    return Platform_AppInitializeApplovin(params, mediation_provider);
}

static dmExtension::Result AppFinalizeApplovin(dmExtension::AppParams* params)
{
    return Platform_AppFinalizeApplovin(params);
}

static dmExtension::Result InitializeApplovin(dmExtension::Params* params)
{
    printf("Registered %s Extension\n", MODULE_NAME);
    LuaInit(params->m_L);
    return Platform_InitializeApplovin(params);
}

static dmExtension::Result FinalizeApplovin(dmExtension::Params* params)
{
    return Platform_FinalizeApplovin(params);
}

static dmExtension::Result UpdateApplovin(dmExtension::Params* params)
{
    return Platform_UpdateApplovin(params);
}

static void OnEventApplovin(dmExtension::Params* params, const dmExtension::Event* event)
{
    Platform_OnEventApplovin(params, event);
}

DM_DECLARE_EXTENSION(ApplovinExt, LIB_NAME, AppInitializeApplovin, AppFinalizeApplovin, InitializeApplovin, UpdateApplovin, OnEventApplovin, FinalizeApplovin)
#else
extern "C" void ApplovinExt()
{
}

#endif //defined(DM_PLATFORM_ANDROID) || defined(DM_PLATFORM_IOS) || defined(DM_PLATFORM_HTML5)