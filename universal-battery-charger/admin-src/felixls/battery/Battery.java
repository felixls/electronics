package felixls.battery;

public class Battery {
	private String name;
	private BatteryType type;
	private int capacity;
	private int cells;
	private int temperature;
	private int chargeDischargeCurrent;
	
	public int getChargeDischargeCurrent() {
		return chargeDischargeCurrent;
	}
	public void setChargeDischargeCurrent(int chargeDischargeCurrent) {
		this.chargeDischargeCurrent = chargeDischargeCurrent;
	}
	public int getCapacity() {
		return capacity;
	}
	public void setCapacity(int capacity) {
		this.capacity = capacity;
	}
	public int getCells() {
		return cells;
	}
	public void setCells(int cells) {
		this.cells = cells;
	}
	public String getName() {
		return name;
	}
	public void setName(String name) {
		this.name = name;
	}
	public int getTemperature() {
		return temperature;
	}
	public void setTemperature(int temperature) {
		this.temperature = temperature;
	}
	public BatteryType getType() {
		return type;
	}
	public void setType(BatteryType type) {
		this.type = type;
	}
	
}
