package rocks.mpgames.applovin;

import android.app.Activity;
import android.content.Intent;
import android.util.Log;
import java.util.Arrays;

import com.applovin.sdk.AppLovinSdk;
import com.applovin.sdk.AppLovinSdkConfiguration;
import com.applovin.mediation.MaxAd;
import com.applovin.mediation.MaxReward;
import com.applovin.mediation.MaxRewardedAdListener;
import com.applovin.mediation.ads.MaxRewardedAd;

import rocks.mpgames.applovin.ApplovinRewardedAdHandler;

public class Applovin implements AppLovinSdk.SdkInitializationListener {
    private Activity      activity;
    private ApplovinRewardedAdHandler rewardedAdHandler;
    private String mediationProvider;

    private static final String TAG = "rock.mpgames.applovin";

    public Applovin(Activity activity, String mediationProvider) {
        this.activity = activity;
        this.mediationProvider = mediationProvider;
    }

    public void initSdk() {
        AppLovinSdk.getInstance( this.activity ).setMediationProvider( mediationProvider );
        AppLovinSdk.getInstance( this.activity ).getSettings().setVerboseLogging( true );
        AppLovinSdk.getInstance( this.activity ).getSettings().setTestDeviceAdvertisingIds(Arrays.asList("1b4e91e5-f586-4e86-9943-d2c67383ca78"));
        AppLovinSdk.initializeSdk( this.activity, this );
    }

    @Override
    public void onSdkInitialized(final AppLovinSdkConfiguration configuration)
    {
        // AppLovin SDK is initialized, start loading ads
        Log.d(TAG, "AppLovin SDK is initialized.");
    }

    public void createRewardedAd(String ad_unit_id, long listener_ref)
    {
        if (this.rewardedAdHandler == null) {
            this.rewardedAdHandler = new ApplovinRewardedAdHandler(this, this.activity, ad_unit_id, listener_ref);
        } else {
            this.rewardedAdHandler.loadAd();
        }

    }

    public void loadRewardedAd()
    {
        this.rewardedAdHandler.loadAd();
    }

    public void showRewardedAd()
    {
        this.rewardedAdHandler.showAd();
    }

    // JNI methods rewarded Ad
    public native void onRewardedAdLoaded(final long listenerRef, final String maxAdJson);
    public native void onRewardedAdLoadFailed(final long listenerRef, final String adUnitId, final int errorCode);
    public native void onRewardedAdDisplayFailed(final long listenerRef, final String maxAdJson, final int errorCode);
    public native void onRewardedAdDisplayed(final long listenerRef, final String maxAdJson);
    public native void onRewardedAdClicked(final long listenerRef, final String maxAdJson);
    public native void onRewardedAdHidden(final long listenerRef, final String maxAdJson);
    public native void onRewardedVideoStarted(final long listenerRef, final String maxAdJson);
    public native void onRewardedVideoCompleted(final long listenerRef, final String maxAdJson);
    public native void onUserRewarded(final long listenerRef, final String maxAdJson);


}