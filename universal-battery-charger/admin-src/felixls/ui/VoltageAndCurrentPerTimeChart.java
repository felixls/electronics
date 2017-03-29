package felixls.ui;

import java.awt.Color;

import org.jfree.data.xy.XYDataset;
import org.jfree.data.xy.XYSeries;
import org.jfree.data.xy.XYSeriesCollection;

public class VoltageAndCurrentPerTimeChart extends BaseChart  {

	private XYSeries currentSerie;
	
	public VoltageAndCurrentPerTimeChart() {
		super("Time", "Voltage, Current");		
	}
	
	protected VoltageAndCurrentPerTimeChart(String xAxisLabel, String yAxisLabel) {
		super(xAxisLabel, yAxisLabel);
	}

	@Override
	protected void setRenderer() {
		renderer.setSeriesPaint(0, Color.blue);
		renderer.setSeriesPaint(1, Color.red);
	}
	
	protected XYDataset createDataset() {
    	firstSerie = new XYSeries("Voltage / Time");
    	currentSerie = new XYSeries("Current / Time");
      	
        XYSeriesCollection dataset = new XYSeriesCollection();
              
        dataset.addSeries(firstSerie);
        dataset.addSeries(currentSerie);

        return dataset;
	}

    protected void addCurrentXY(double x, double y)
    {
    	currentSerie.add(x, y);
    }
    
	public void reset()	{
		firstSerie.clear();
		currentSerie.clear();
	}


}
