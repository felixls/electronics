package felixls.ui;

import java.awt.Color;

public class CurrentPerTimeChart extends BaseChart {
	public CurrentPerTimeChart() {
		super("Time", "Current");		
	}
	
	protected CurrentPerTimeChart(String xAxisLabel, String yAxisLabel) {
		super(xAxisLabel, yAxisLabel);
	}

	@Override
	protected void setRenderer() {
		renderer.setSeriesPaint(0, Color.red);		
	}
}
