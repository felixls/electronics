package felixls.ui;

import java.awt.Color;

public class VoltagePerTimeChart extends BaseChart {

	public VoltagePerTimeChart() {
		super("Time", "Voltage");		
	}
	
	protected VoltagePerTimeChart(String xAxisLabel, String yAxisLabel) {
		super(xAxisLabel, yAxisLabel);
	}

	@Override
	protected void setRenderer() {
		renderer.setSeriesPaint(0, Color.blue);		
	}
}
