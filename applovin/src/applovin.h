#if defined(DM_PLATFORM_ANDROID)

#ifndef DM_APPLOVIN_EXTENSION
#define DM_APPLOVIN_EXTENSION


dmExtension::Result Platform_AppInitializeApplovin(dmExtension::AppParams* params, const char* mediation_provider);
dmExtension::Result Platform_AppFinalizeApplovin(dmExtension::AppParams* params);
dmExtension::Result Platform_InitializeApplovin(dmExtension::Params* params, bool verbose_logs);
dmExtension::Result Platform_FinalizeApplovin(dmExtension::Params* params);
dmExtension::Result Platform_UpdateApplovin(dmExtension::Params* params, dmApplovin::ApplovinListener* listener);
void                Platform_OnEventApplovin(dmExtension::Params* params, const dmExtension::Event* event);

void Platform_initSdk(lua_State* L);
void Platform_createRewardedAd(lua_State* L, const char* adUnitId);
void Platform_loadRewardedAd(lua_State* L);
void Platform_showRewardedAd(lua_State* L);

#endif // DM_APPLOVIN_EXTENSION

#endif // DM_PLATFORM_ANDROID