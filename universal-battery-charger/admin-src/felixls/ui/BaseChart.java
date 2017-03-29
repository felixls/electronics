package felixls.ui;

import java.awt.Color;
import java.awt.Font;
import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.plot.PlotOrientation;
import org.jfree.chart.plot.XYPlot;
import org.jfree.chart.renderer.xy.XYItemRenderer;
import org.jfree.chart.renderer.xy.XYLineAndShapeRenderer;
import org.jfree.data.xy.XYDataset;
import org.jfree.data.xy.XYSeries;
import org.jfree.data.xy.XYSeriesCollection;
import org.jfree.ui.RectangleInsets;

public abstract class BaseChart  {

	protected JFreeChart chart;
	protected XYPlot plot;
	protected XYLineAndShapeRenderer renderer;
	protected XYSeries firstSerie;
	protected String xAxisLabel;
	protected String yAxisLabel;
	
	public BaseChart(String xAxisLabel, String yAxisLabel)
	{
    	this.xAxisLabel = xAxisLabel;
    	this.yAxisLabel = yAxisLabel;

		createChart(createDataset(), xAxisLabel, yAxisLabel);
        setRenderer();
	}

	public void reset()	{
		firstSerie.clear();
	}
	
    protected JFreeChart createChart(XYDataset dataset, String xAxisLabel, String yAxisLabel) {
    	
        chart = ChartFactory.createXYLineChart(
            "",					// title
            xAxisLabel,         // x-axis label
            yAxisLabel,   		// y-axis label
            dataset,            // data
            PlotOrientation.VERTICAL,
            true,               // create legend?
            true,               // generate tooltips?
            false               // generate URLs?
        );
        
        chart.setBackgroundPaint(Color.white);

        plot = (XYPlot) chart.getPlot();
        plot.setBackgroundPaint(Color.lightGray);
        plot.setDomainGridlinePaint(Color.white);
        plot.setRangeGridlinePaint(Color.white);
        plot.setAxisOffset(new RectangleInsets(5.0, 5.0, 5.0, 5.0));
        plot.setDomainCrosshairVisible(true);
        plot.setRangeCrosshairVisible(true);

        plot.setNoDataMessage("Esperando datos...");
        plot.setNoDataMessagePaint(Color.yellow);
        plot.setNoDataMessageFont(new Font("Verdana", Font.BOLD, 18));
        
        XYItemRenderer r = plot.getRenderer();
        if (r instanceof XYLineAndShapeRenderer) {
            renderer = (XYLineAndShapeRenderer) r;
            renderer.setBaseShapesVisible(true);
            renderer.setBaseShapesFilled(true);
            renderer.setDrawSeriesLineAsPath(true);
        }

        return chart;

    }
    
    protected void addXY(double x, double y)
    {
		firstSerie.add(x, y);
    }

	protected XYDataset createDataset() {
    	firstSerie = new XYSeries(getFirstSerieName());
      	
        XYSeriesCollection dataset = new XYSeriesCollection();
              
        dataset.addSeries(firstSerie);

        return dataset;
	}
	
	protected String getFirstSerieName()
	{
		return yAxisLabel + " / " + xAxisLabel;
	}

	protected void setRenderer()
	{		
	}

	public void setTitle(String title) {
		chart.setTitle(title);
	}
	
	public void setZoom(double value) {
		plot.zoom(value);
	}
	
	public ChartPanel createPanel() {
        ChartPanel panel = new ChartPanel(chart);        
        panel.setFillZoomRectangle(true);
        panel.setMouseWheelEnabled(true);
        return panel;		
	}
	
}
