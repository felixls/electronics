package felixls.ui;

import java.awt.Color;
import java.awt.GridLayout;
import java.awt.event.WindowEvent;
import java.io.FileNotFoundException;
import java.io.IOException;

import javax.swing.BorderFactory;
import javax.swing.JPanel;

import org.jfree.chart.ChartPanel;
import org.jfree.ui.ApplicationFrame;

import felixls.battery.Battery;
import felixls.battery.ChargeFinishInfo;
import felixls.battery.ChargeInfo;
import felixls.battery.DischargeFinishInfo;
import felixls.battery.DischargeInfo;
import felixls.comm.ProtocolEventListener;
import felixls.comm.SerialProtocol;

@SuppressWarnings("serial")
public class AppFrame extends ApplicationFrame implements ProtocolEventListener{
	private SerialProtocol proto;
	
    private VoltagePerTimeChart voltagePerTimeChart;
    private CurrentPerTimeChart currentPerTimeChart;
    private TemperaturePerTimeChart temperaturePerTimeChart;
    private VoltageAndCurrentPerTimeChart voltageAndCurrentPerTimeChart;
    
	public AppFrame(String title, SerialProtocol proto){
		super(title);
						
		this.proto = proto;
		
		this.proto.start();
		this.proto.addEventListener(this);
	
		JPanel mainPanel = new JPanel(new GridLayout(2, 2, 2, 2));
		mainPanel.setBorder(BorderFactory.createEmptyBorder(2,2,2,2));
		mainPanel.setBackground(Color.blue);
		
		voltagePerTimeChart = new VoltagePerTimeChart();
		currentPerTimeChart = new CurrentPerTimeChart();
		temperaturePerTimeChart = new TemperaturePerTimeChart();
		voltageAndCurrentPerTimeChart = new VoltageAndCurrentPerTimeChart();
		
        ChartPanel chartPanel = voltagePerTimeChart.createPanel(); 
        chartPanel.setPreferredSize(new java.awt.Dimension(500, 270));
        mainPanel.add(chartPanel);

        chartPanel = currentPerTimeChart.createPanel();
        chartPanel.setPreferredSize(new java.awt.Dimension(500, 270));
        mainPanel.add(chartPanel);

        chartPanel = temperaturePerTimeChart.createPanel();
        chartPanel.setPreferredSize(new java.awt.Dimension(500, 270));
        mainPanel.add(chartPanel);

        chartPanel = voltageAndCurrentPerTimeChart.createPanel();
        chartPanel.setPreferredSize(new java.awt.Dimension(500, 270));
        mainPanel.add(chartPanel);
        
        setContentPane(mainPanel);        
	}
	
	public void notifyCharge(ChargeInfo chargeInfo) {
		
		if (!proto.isStarted())
		{
			try {
				proto.startSave("data.txt");
			}
			catch (FileNotFoundException ex)
			{
			}			
		}
		
		double x = chargeInfo.getMinute() * 60 + chargeInfo.getSecond();
		double y = chargeInfo.getVoltage();
		voltagePerTimeChart.addXY(x, y);
		voltageAndCurrentPerTimeChart.addXY(x, y);
		
		y = chargeInfo.getCurrent();		
		currentPerTimeChart.addXY(x, y);
		voltageAndCurrentPerTimeChart.addCurrentXY(x, y);
		
		y = chargeInfo.getTemperature();
		temperaturePerTimeChart.addXY(x, y);		

	}

	public void notifyDischarge(DischargeInfo dischargeInfo) {

		if (!proto.isStarted())
		{
			try {
				proto.startSave("data.txt");
			}
			catch (FileNotFoundException ex)
			{
			}			
		}

		double x = dischargeInfo.getMinute() * 60 + dischargeInfo.getSecond();
		double y = dischargeInfo.getVoltage();
		voltagePerTimeChart.addXY(x, y);
		voltageAndCurrentPerTimeChart.addXY(x, y);
		
		y = dischargeInfo.getCurrent();		
		currentPerTimeChart.addXY(x, y);
		voltageAndCurrentPerTimeChart.addCurrentXY(x, y);
		
		y = dischargeInfo.getTemperature();
		temperaturePerTimeChart.addXY(x, y);		
		
	}

	public void notifyChageFinish(ChargeFinishInfo chargeFinishInfo) {
		try {
	    	proto.close();
		}
		catch (IOException ex)
		{
		}
	}

	public void notifyDischargeFinish(DischargeFinishInfo dischargeFinishInfo) {
		try {
	    	proto.close();
		}
		catch (IOException ex)
		{
		}
	}

	public void startCharge(Battery battery) {
		String title;
		double charge = battery.getChargeDischargeCurrent() / 10d;
		
		title = battery.getType().name() + " Charge " + battery.getCapacity() * 100 + "mAh " + charge + "C " +
		battery.getCells() + " cells";
	
		try {
			proto.startSave("chargedata.txt");
		}
		catch (FileNotFoundException ex)
		{
		}
		
		setTitles(title);
	}

	public void startDischarge(Battery battery) {
		String title;
		title = battery.getType().name() + " Discharge " + battery.getCapacity() * 100 + "mAh " + battery.getChargeDischargeCurrent() + "mA " +
		battery.getCells() + " cells";
	
		try {
			proto.startSave("dischargedata.txt");
		}
		catch (FileNotFoundException ex)
		{
		}
		
		setTitles(title);
	}
	
	private void setTitles(String title)
	{
		voltagePerTimeChart.reset();
		currentPerTimeChart.reset();
		temperaturePerTimeChart.reset();
		voltageAndCurrentPerTimeChart.reset();
		
		voltagePerTimeChart.setTitle(title);
		voltagePerTimeChart.setZoom(0);
		
		currentPerTimeChart.setTitle(title);
		currentPerTimeChart.setZoom(0);
		
		temperaturePerTimeChart.setTitle(title);
		temperaturePerTimeChart.setZoom(0);

		voltageAndCurrentPerTimeChart.setTitle(title);
		voltageAndCurrentPerTimeChart.setZoom(0);		
	}

	public void windowClosing(WindowEvent e) {
		System.out.println("bye");

		try {
	    	proto.close();
		}
		catch (Exception ex)
		{
		}
		
		super.windowClosing(e);
	}

}
