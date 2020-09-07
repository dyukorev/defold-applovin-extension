#if defined(DM_PLATFORM_ANDROID) || defined(DM_PLATFORM_IOS)

#ifndef DM_APPLOVIN_EXTENSION
#define DM_APPLOVIN_EXTENSION


dmExtension::Result Platform_AppInitializeApplovin(dmExtension::AppParams* params, const char* mediation_provider);
dmExtension::Result Platform_AppFinalizeApplovin(dmExtension::AppParams* params);
dmExtension::Result Platform_InitializeApplovin(dmExtension::Params* params, const bool verbose_logs, const char* test_device_id);
dmExtension::Result Platform_FinalizeApplovin(dmExtension::Params* params);
dmExtension::Result Platform_UpdateApplovin(dmExtension::Params* params);
void                Platform_OnEventApplovin(dmExtension::Params* params, const dmExtension::Event* event);

void Platform_createRewardedAd(lua_State* L, dmScript::LuaCallbackInfo* listener, const char* adUnitId);
void Platform_loadRewardedAd(lua_State* L);
void Platform_showRewardedAd(lua_State* L);

#endif // DM_APPLOVIN_EXTENSION

#endif // DM_PLATFORM_ANDROID || DM_PLATFORM_IOS