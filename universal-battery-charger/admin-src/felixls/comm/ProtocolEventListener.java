package felixls.comm;

import felixls.battery.*;

public interface ProtocolEventListener extends java.util.EventListener {

	public abstract void startCharge(Battery battery);	
	
	public abstract void startDischarge(Battery battery);	
	
	public abstract void notifyCharge(ChargeInfo chargeInfo);

	public abstract void notifyDischarge(DischargeInfo dischargeInfo);
	
	public abstract void notifyChageFinish(ChargeFinishInfo chargeFinishInfo);
	
	public abstract void notifyDischargeFinish(DischargeFinishInfo dischargeFinishInfo);
}
