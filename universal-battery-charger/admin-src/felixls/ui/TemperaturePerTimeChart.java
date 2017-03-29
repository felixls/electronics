package felixls.ui;

import java.awt.Color;

public class TemperaturePerTimeChart extends BaseChart {

	public TemperaturePerTimeChart() {
		super("Time", "Temperature");		
	}
	
	protected TemperaturePerTimeChart(String xAxisLabel, String yAxisLabel) {
		super(xAxisLabel, yAxisLabel);
	}

	@Override
	protected void setRenderer() {
		renderer.setSeriesPaint(0, Color.green);		
	}
}
