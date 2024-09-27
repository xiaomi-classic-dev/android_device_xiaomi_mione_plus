package com.android.internal.telephony;

import static com.android.internal.telephony.RILConstants.*;

import android.content.Context;
import android.os.AsyncResult;
import android.os.Message;
import android.os.Parcel;
import android.telephony.SignalStrength;

import java.util.ArrayList;

public class XiaomiMsm8660RIL extends RIL {

    public XiaomiMsm8660RIL(Context context, int preferredNetworkType, int cdmaSubscription) {
        super(context, preferredNetworkType, cdmaSubscription);
    }

    public XiaomiMsm8660RIL(Context context, int preferredNetworkType, int cdmaSubscription, Integer instanceId) {
        super(context, preferredNetworkType, cdmaSubscription, instanceId);
    }

    @Override
    protected void
    processUnsolicited(Parcel p, int type) {
        int dataPosition = p.dataPosition();
        int origResponse = p.readInt();
        int newResponse = origResponse;
        switch (origResponse) {
            case 1038:
                newResponse = RIL_UNSOL_ON_SS;
                break;
            case 1039:
                newResponse = RIL_UNSOL_STK_CC_ALPHA_NOTIFY;
                break;
            case 1040:
                newResponse = RIL_UNSOL_UICC_SUBSCRIPTION_STATUS_CHANGED;
                break;
        }
        if (newResponse != origResponse) {
            riljLog("XiaomiMsm8660RIL: remap unsolicited response from " +
                    origResponse + " to " + newResponse);
            p.setDataPosition(dataPosition);
            p.writeInt(newResponse);
        }
        p.setDataPosition(dataPosition);
        super.processUnsolicited(p, type);
    }

    @Override
    public void setCellInfoListRate(int rateInMillis, Message response) {
        riljLog("setCellInfoListRate: not supported");
        if (response != null) {
            CommandException ex = new CommandException(
                CommandException.Error.REQUEST_NOT_SUPPORTED);
            AsyncResult.forMessage(response, null, ex);
            response.sendToTarget();
        }
    }

    // This call causes ril to crash the socket, stopping further communication
    @Override
    public void
    getHardwareConfig (Message result) {
        riljLog("Ignoring call to 'getHardwareConfig'");
        if (result != null) {
            CommandException ex = new CommandException(
                CommandException.Error.REQUEST_NOT_SUPPORTED);
            AsyncResult.forMessage(result, null, ex);
            result.sendToTarget();
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setDataAllowed(boolean allowed, Message result) {
        riljLog("setDataAllowed: not supported");

        if (result != null) {
            CommandException e = new CommandException(
                CommandException.Error.REQUEST_NOT_SUPPORTED);
            AsyncResult.forMessage(result, null, e);
            result.sendToTarget();
        }
    }
}
