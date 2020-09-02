package rocks.mpgames.applovin;

import android.app.Activity;
import android.os.Handler;
import org.json.JSONObject;
import org.json.JSONException;
import java.util.concurrent.TimeUnit;

import com.applovin.mediation.MaxAd;
import com.applovin.mediation.MaxReward;
import com.applovin.mediation.MaxRewardedAdListener;
import com.applovin.mediation.ads.MaxRewardedAd;

public class ApplovinRewardedAdHandler implements MaxRewardedAdListener {
    private Applovin applovinAd;

    private MaxRewardedAd rewardedAd;
    private int retryAttempt;
    private long listenerRef;
    private Activity activity;

    public ApplovinRewardedAdHandler(Applovin applovinAd, final Activity activity, final String ad_unit_id, long listener_ref)
    {
        this.applovinAd = applovinAd;
        this.activity = activity;
        this.listenerRef = listener_ref;

        this.rewardedAd = MaxRewardedAd.getInstance( ad_unit_id, activity );
        this.rewardedAd.setListener( this );

        this.rewardedAd.loadAd();

    }

    String maxAdToJson(final MaxAd maxAd)
    {
        JSONObject json = new JSONObject();
        try {
            json.put("AdUnitId", maxAd.getAdUnitId());
            json.put("Format", maxAd.getFormat().getLabel());
            json.put("NetworkName", maxAd.getNetworkName());
        } catch (JSONException e) {
            return "{ 'error': 'Json exception on MaxAd to json convert' }";
        }

        return json.toString();
    }

    // MAX Ad Listener
    @Override
    public void onAdLoaded(final MaxAd maxAd)
    {
        // Reset retry attempt
        retryAttempt = 0;
        applovinAd.onRewardedAdLoaded(this.listenerRef, maxAdToJson(maxAd));
    }

    @Override
    public void onAdLoadFailed(final String adUnitId, final int errorCode)
    {
        retryAttempt++;
        long delayMillis = TimeUnit.SECONDS.toMillis( (long) Math.pow( 2, Math.min( 6, retryAttempt ) ) );

        new Handler().postDelayed( new Runnable()
        {
            @Override
            public void run()
            {
                rewardedAd.loadAd();
            }
        }, delayMillis );

        applovinAd.onRewardedAdLoadFailed(this.listenerRef, adUnitId, errorCode);
    }

    @Override
    public void onAdDisplayFailed(final MaxAd maxAd, final int errorCode)
    {
        // Rewarded ad failed to display. We recommend loading the next ad
        applovinAd.onRewardedAdDisplayFailed(this.listenerRef, maxAdToJson(maxAd), errorCode);
    }

    @Override
    public void onAdDisplayed(final MaxAd maxAd)
    {
        applovinAd.onRewardedAdDisplayed(this.listenerRef, maxAdToJson(maxAd));
    }

    @Override
    public void onAdClicked(final MaxAd maxAd)
    {
        applovinAd.onRewardedAdClicked(this.listenerRef, maxAdToJson(maxAd));
    }

    @Override
    public void onAdHidden(final MaxAd maxAd)
    {
        applovinAd.onRewardedAdHidden(this.listenerRef, maxAdToJson(maxAd));
    }

    @Override
    public void onRewardedVideoStarted(final MaxAd maxAd)
    {
        applovinAd.onRewardedVideoStarted(this.listenerRef, maxAdToJson(maxAd));
    }

    @Override
    public void onRewardedVideoCompleted(final MaxAd maxAd)
    {
        applovinAd.onRewardedVideoCompleted(this.listenerRef, maxAdToJson(maxAd));
    }

    @Override
    public void onUserRewarded(final MaxAd maxAd, final MaxReward maxReward)
    {
        applovinAd.onUserRewarded(this.listenerRef, maxAdToJson(maxAd));
    }

    public boolean isReady()
    {
        return rewardedAd.isReady();
    }

    public void showAd()
    {
        this.activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                rewardedAd.showAd();
            }
        });
    }

    public void loadAd()
    {
        this.activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                rewardedAd.loadAd();
            }
        });
    }
}