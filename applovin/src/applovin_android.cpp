#if defined(DM_PLATFORM_ANDROID)
#include <assert.h>

#include <dmsdk/sdk.h>
#include <dmsdk/extension/extension.h>
#include <dmsdk/dlib/log.h>
#include <dmsdk/dlib/array.h>
#include <dmsdk/dlib/mutex.h>
#include <dmsdk/script/script.h>

#include "queue.h"
#include "applovin.h"

struct Applovin {
    Applovin() {
        memset(this, 0, sizeof(*this));
    }

    jobject m_ApplovinJava;
    jmethodID m_CreateRewardedAd;
    jmethodID m_LoadRewardedAd;
    jmethodID m_ShowRewardedAd;
    jmethodID m_InitSdk;
    bool m_VerboseLogs;

    dmApplovin::EventQueue m_EventQueue;
};

static Applovin g_Applovin;

static const char* StrDup(JNIEnv* env, jstring s)
{
    if (s != NULL)
    {
        const char* str = env->GetStringUTFChars(s, 0);
        const char* dup = strdup(str);
        env->ReleaseStringUTFChars(s, str);
        return dup;
    }
    else
    {
        return 0x0;
    }
}


//JNI methods declarations

static JNIEnv* Attach()
{
    JNIEnv* env;
    dmGraphics::GetNativeAndroidJavaVM()->AttachCurrentThread(&env, NULL);
    return env;
}

static void Detach()
{
    dmGraphics::GetNativeAndroidJavaVM()->DetachCurrentThread();
}


    dmExtension::Result Platform_AppInitializeApplovin(dmExtension::AppParams* params, const char* mediation_provider)
    {
        if (!g_Applovin.m_ApplovinJava)
        {
            JNIEnv* env = Attach();
            jclass activity_class = env->FindClass("android/app/NativeActivity");
            jmethodID get_class_loader = env->GetMethodID(activity_class,"getClassLoader", "()Ljava/lang/ClassLoader;");
            jobject cls = env->CallObjectMethod(dmGraphics::GetNativeAndroidActivity(), get_class_loader);
            jclass class_loader = env->FindClass("java/lang/ClassLoader");
            jmethodID find_class = env->GetMethodID(class_loader, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");

            jstring str_class_name = env->NewStringUTF("rocks.mpgames.applovin.Applovin");
            jclass applovin_class = (jclass)env->CallObjectMethod(cls, find_class, str_class_name);
            env->DeleteLocalRef(str_class_name);

            // Create JNI app instance
            jstring str_mediation_provider = env->NewStringUTF(mediation_provider);
            jmethodID jni_constructor = env->GetMethodID(applovin_class, "<init>", "(Landroid/app/Activity;Ljava/lang/String;)V");
            g_Applovin.m_ApplovinJava = env->NewGlobalRef(env->NewObject(applovin_class, jni_constructor, dmGraphics::GetNativeAndroidActivity(), str_mediation_provider));
            env->DeleteLocalRef(str_mediation_provider);

            // JNI app methods mapping
            g_Applovin.m_CreateRewardedAd = env->GetMethodID(applovin_class, "createRewardedAd", "(Ljava/lang/String;J)V");
            g_Applovin.m_LoadRewardedAd = env->GetMethodID(applovin_class, "loadRewardedAd", "()V");
            g_Applovin.m_ShowRewardedAd = env->GetMethodID(applovin_class, "showRewardedAd", "()V");
            g_Applovin.m_InitSdk = env->GetMethodID(applovin_class, "initSdk", "(Z)V");

            Detach();
        }

        return dmExtension::RESULT_OK;
    }

    dmExtension::Result Platform_InitializeApplovin(dmExtension::Params* params, bool verbose_logs)
    {
        dmApplovin::QueueCreate(&g_Applovin.m_EventQueue);
        g_Applovin.m_VerboseLogs = verbose_logs;
        return dmExtension::RESULT_OK;
    }

    dmExtension::Result Platform_AppFinalizeApplovin(dmExtension::AppParams* params)
    {
        //todo make finalization
        return dmExtension::RESULT_OK;
    }

    dmExtension::Result Platform_FinalizeApplovin(dmExtension::Params* params)
    {
        JNIEnv* env = Attach();
        env->DeleteGlobalRef(g_Applovin.m_ApplovinJava);
        g_Applovin.m_ApplovinJava = NULL;
        Detach();
        return dmExtension::RESULT_OK;
    }

    dmExtension::Result Platform_UpdateApplovin(dmExtension::Params* params, dmApplovin::ApplovinListener* listener)
    {
        dmApplovin::QueueFlush(&g_Applovin.m_EventQueue, listener);
        return dmExtension::RESULT_OK;
    }

    void Platform_OnEventApplovin(dmExtension::Params* params, const dmExtension::Event* event)
    {
    }

#ifdef __cplusplus
extern "C" {
#endif
    //onRewardedAdLoaded(final long listenerRef, final String maxAdJson);
    JNIEXPORT void JNICALL Java_rocks_mpgames_applovin_Applovin_onRewardedAdLoaded(JNIEnv* env, jobject, jlong listenerRef, jstring jmaxadjson)
    {
        dmApplovin::ApplovinEvent event;
        event.m_Type = dmApplovin::EVENT_AD_LOADED;
        event.m_Callback = (dmScript::LuaCallbackInfo*)listenerRef;
        event.m_Results = StrDup(env, jmaxadjson);
        dmApplovin::QueuePush(&g_Applovin.m_EventQueue, &event);
    }

    //onRewardedAdLoadFailed(final long listenerRef, final String adUnitId, final int errorCode);
    JNIEXPORT void JNICALL Java_rocks_mpgames_applovin_Applovin_onRewardedAdLoadFailed(JNIEnv* env, jobject, jlong listenerRef, jstring adUnitId, jint errorCode)
    {
        dmApplovin::ApplovinEvent event;
        event.m_Type = dmApplovin::EVENT_AD_LOAD_FAILED;
        event.m_Callback = (dmScript::LuaCallbackInfo*)listenerRef;
        event.m_Results = StrDup(env, adUnitId);
        //todo add error description
        event.m_ErrorCode = (int)errorCode;
        dmApplovin::QueuePush(&g_Applovin.m_EventQueue, &event);
    }

    //onRewardedAdDisplayFailed(final long listenerRef, final String maxAdJson, final int errorCode);
    JNIEXPORT void JNICALL Java_rocks_mpgames_applovin_Applovin_onRewardedAdDisplayFailed(JNIEnv* env, jobject, jlong listenerRef, jstring jmaxadjson, jint errorCode)
    {
        dmApplovin::ApplovinEvent event;
        event.m_Type = dmApplovin::EVENT_AD_DISPLAY_FAILED;
        event.m_Callback = (dmScript::LuaCallbackInfo*)listenerRef;
        event.m_Results = StrDup(env, jmaxadjson);
        //todo add error description
        event.m_ErrorCode = (int)errorCode;
        dmApplovin::QueuePush(&g_Applovin.m_EventQueue, &event);
    }

    //onRewardedAdDisplayed(final long listenerRef, final String maxAdJson);
    JNIEXPORT void JNICALL Java_rocks_mpgames_applovin_Applovin_onRewardedAdDisplayed(JNIEnv* env, jobject, jlong listenerRef, jstring jmaxadjson)
    {
        dmApplovin::ApplovinEvent event;
        event.m_Type = dmApplovin::EVENT_AD_DISPLAYED;
        event.m_Callback = (dmScript::LuaCallbackInfo*)listenerRef;
        event.m_Results = StrDup(env, jmaxadjson);
        dmApplovin::QueuePush(&g_Applovin.m_EventQueue, &event);
    }

    //onRewardedAdClicked(final long listenerRef, final String maxAdJson);
    JNIEXPORT void JNICALL Java_rocks_mpgames_applovin_Applovin_onRewardedAdClicked(JNIEnv* env, jobject, jlong listenerRef, jstring jmaxadjson)
    {
        dmApplovin::ApplovinEvent event;
        event.m_Type = dmApplovin::EVENT_AD_CLICKED;
        event.m_Callback = (dmScript::LuaCallbackInfo*)listenerRef;
        event.m_Results = StrDup(env, jmaxadjson);
        dmApplovin::QueuePush(&g_Applovin.m_EventQueue, &event);
    }

    //onRewardedAdHidden(final long listenerRef, final String maxAdJson);
    JNIEXPORT void JNICALL Java_rocks_mpgames_applovin_Applovin_onRewardedAdHidden(JNIEnv* env, jobject, jlong listenerRef, jstring jmaxadjson)
    {
        dmApplovin::ApplovinEvent event;
        event.m_Type = dmApplovin::EVENT_AD_HIDDEN;
        event.m_Callback = (dmScript::LuaCallbackInfo*)listenerRef;
        event.m_Results = StrDup(env, jmaxadjson);
        dmApplovin::QueuePush(&g_Applovin.m_EventQueue, &event);
    }

    //onRewardedVideoStarted(final long listenerRef, final String maxAdJson);
    JNIEXPORT void JNICALL Java_rocks_mpgames_applovin_Applovin_onRewardedVideoStarted(JNIEnv* env, jobject, jlong listenerRef, jstring jmaxadjson)
    {
        dmApplovin::ApplovinEvent event;
        event.m_Type = dmApplovin::EVENT_REWARDED_VIDEO_STARTED;
        event.m_Callback = (dmScript::LuaCallbackInfo*)listenerRef;
        event.m_Results = StrDup(env, jmaxadjson);
        dmApplovin::QueuePush(&g_Applovin.m_EventQueue, &event);
    }

    //onRewardedVideoCompleted(final long listenerRef, final String maxAdJson);
    JNIEXPORT void JNICALL Java_rocks_mpgames_applovin_Applovin_onRewardedVideoCompleted(JNIEnv* env, jobject, jlong listenerRef, jstring jmaxadjson)
    {
        dmApplovin::ApplovinEvent event;
        event.m_Type = dmApplovin::EVENT_REWARDED_VIDEO_COMPLETED;
        event.m_Callback = (dmScript::LuaCallbackInfo*)listenerRef;
        event.m_Results = StrDup(env, jmaxadjson);
        dmApplovin::QueuePush(&g_Applovin.m_EventQueue, &event);
    }

    //onUserRewarded(final long listenerRef, final String maxAdJson);
    JNIEXPORT void JNICALL Java_rocks_mpgames_applovin_Applovin_onUserRewarded(JNIEnv* env, jobject, jlong listenerRef, jstring jmaxadjson)
    {
        dmApplovin::ApplovinEvent event;
        event.m_Type = dmApplovin::EVENT_USER_REWARDED;
        event.m_Callback = (dmScript::LuaCallbackInfo*)listenerRef;
        event.m_Results = StrDup(env, jmaxadjson);
        dmApplovin::QueuePush(&g_Applovin.m_EventQueue, &event);
    }

#ifdef __cplusplus
}
#endif

    void Platform_initSdk(lua_State* L) {
            JNIEnv* env = Attach();
            env->CallVoidMethod(g_Applovin.m_ApplovinJava, g_Applovin.m_InitSdk, g_Applovin.m_VerboseLogs);
            Detach();
    }

    void Platform_createRewardedAd(lua_State* L, const char* adUnitId) {
        JNIEnv* env = Attach();

        jstring str_adUnitId = env->NewStringUTF(adUnitId);
        env->CallVoidMethod(g_Applovin.m_ApplovinJava, g_Applovin.m_CreateRewardedAd, str_adUnitId, 0);
        env->DeleteLocalRef(str_adUnitId);
        Detach();
    }

    void Platform_loadRewardedAd(lua_State* L) {
        JNIEnv* env = Attach();
        env->CallVoidMethod(g_Applovin.m_ApplovinJava, g_Applovin.m_LoadRewardedAd);
        Detach();
    }

    void Platform_showRewardedAd(lua_State* L) {
        JNIEnv* env = Attach();
        env->CallVoidMethod(g_Applovin.m_ApplovinJava, g_Applovin.m_ShowRewardedAd);
        Detach();
    }



#endif // DM_PLATFORM_ANDROID
