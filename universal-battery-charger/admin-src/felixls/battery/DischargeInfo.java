package felixls.battery;

public class DischargeInfo {
	private int minute;
	private int second;
	private int voltage;
	private int current;
	private int temperature;
	
	public int getCurrent() {
		return current;
	}
	public void setCurrent(int current) {
		this.current = current;
	}
	public int getMinute() {
		return minute;
	}
	public void setMinute(int minute) {
		this.minute = minute;
	}
	public int getSecond() {
		return second;
	}
	public void setSecond(int second) {
		this.second = second;
	}
	public int getTemperature() {
		return temperature;
	}
	public void setTemperature(int temperature) {
		this.temperature = temperature;
	}
	public int getVoltage() {
		return voltage;
	}
	public void setVoltage(int voltage) {
		this.voltage = voltage;
	}
	
	

}
