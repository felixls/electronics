package felixls.comm;

import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.util.TooManyListenersException;

import felixls.battery.Battery;
import felixls.battery.BatteryType;
import felixls.battery.ChargeFinishInfo;
import felixls.battery.ChargeInfo;
import felixls.battery.DischargeFinishInfo;
import felixls.battery.DischargeInfo;
import gnu.io.CommPortIdentifier;
import gnu.io.SerialPortEvent;
import gnu.io.SerialPortEventListener;

public class SerialProtocol extends SerialComm  implements SerialPortEventListener {

	private ProtocolEventListener protoListener;
    boolean isCharge = true;
    boolean started = false;

	public SerialProtocol(String portName, CommPortIdentifier portIdentifier)	{
		super(portName, portIdentifier);
	}

	public void start()	{
		
		port.notifyOnDataAvailable(true);
		
	    try {
	    	
	    	port.addEventListener(this);
	    	
	    } catch (TooManyListenersException ev) {
	      System.err.println("Too many listeners(!) " + ev);
	      System.exit(0);
	    }

	}
	
	public void addEventListener(ProtocolEventListener el)	{
		this.protoListener = el;
	}
	
	public void startSave(String fileName) throws FileNotFoundException {
		FileOutputStream fs = new FileOutputStream(fileName, false);
		writer = new OutputStreamWriter(fs);
		started = true;
	}
	
	public void close() throws IOException	{
		started = false;
		writer.close();
	}
	
	public void serialEvent(SerialPortEvent ev) {
	    String line;
	    
	    try {
	    	line = reader.readLine();
	    	
	    	if (line == null) {
	    		return;
	    	}
	    	
	    	if (started)
	    		writer.write(line + "\r\n");

	    	String lineb = line;
	
			// La idea aquí es parsear la línea que envía el uC y preparar los objetos
			// ChargeInfo/ChargeFinishInfo/DischargeInfo, etc según corresponda,
			// y envíe el evento con por ej.:
			// protoListener.notifyChageFinish(chargeFinishInfo)
		      
	    	if (protoListener != null)
	    	{
	    		int i;
			      
	    		switch(line.charAt(0))
	    		{
	    		case '#':
	    			if (line.startsWith("#CHR") || line.startsWith("#DIS"))
	    			{
	    				if (line.startsWith("#DIS"))
	    					isCharge = false;
	    				else
	    					isCharge = true;
			    		  
						line = line.substring(5);
						
						Battery battery = new Battery();
						  
						i = line.indexOf('-');
						  
						if (line.substring(0, i).compareTo("NICD")==0)
							  battery.setType(BatteryType.NiCd);
						  
						if (line.substring(0, i).compareTo("NIMH")==0)
							  battery.setType(BatteryType.NimH);
						  
						if (line.substring(0, i).compareTo("SLA")==0)
							  battery.setType(BatteryType.SLA);
						  
						if (line.substring(0, i).compareTo("LIPO")==0)
							  battery.setType(BatteryType.LiPO);
						  
						if (line.substring(0, i).compareTo("LIIO")==0)
							  battery.setType(BatteryType.LiIO);
						  
						line = line.substring(i+1);
						i = line.indexOf('-');
	
			    		battery.setCapacity(Integer.parseInt(line.substring(0, i)));
			    		  
			    		line = line.substring(i+1);
			    		i = line.indexOf('-');
			    		  
			    		battery.setChargeDischargeCurrent(Integer.parseInt(line.substring(0, i)));
			    		  
			    		line = line.substring(i+1);
	
			    		battery.setCells(Integer.parseInt(line.substring(0)));
	
			    		if (isCharge)
			    			protoListener.startCharge(battery);
			    		else
			    			protoListener.startDischarge(battery);

			    		if (started)
			    			writer.write(lineb + "\r\n");
	    			}
	    			else if (line.compareTo("#END:UC")==0)
			    	{
	    				if (isCharge)
	    					protoListener.notifyChageFinish(ChargeFinishInfo.UserCancel);
	    				else
	    					protoListener.notifyDischargeFinish(DischargeFinishInfo.UserCancel);
			    	}
			    	else if (line.compareTo("#END:DC")==0)
			    	{
			    		protoListener.notifyDischargeFinish(DischargeFinishInfo.DischargeComplete);		    			  
			    	}
			    	else if (line.compareTo("#END:HT")==0)
			    	{
			    		protoListener.notifyChageFinish(ChargeFinishInfo.HighTemp);
			    	}
			    	else if (line.compareTo("#END:LT")==0)
			    	{
			    		protoListener.notifyChageFinish(ChargeFinishInfo.LowTemp);
			    	}
			    	else if (line.compareTo("#END:CC")==0)
			    	{
			    		protoListener.notifyChageFinish(ChargeFinishInfo.ChageComplete);		    			  
			    	}
			    	else if (line.compareTo("#END:TO")==0)
			    	{
			    		protoListener.notifyChageFinish(ChargeFinishInfo.Timeout);  			  
			    	}
			    	else if (line.compareTo("#END:DTDT")==0)
			    	{
			    		protoListener.notifyChageFinish(ChargeFinishInfo.DTdt);  		   			  
			    	}
			    	else if (line.compareTo("#END:DVDT")==0)
			    	{
			    		protoListener.notifyChageFinish(ChargeFinishInfo.DVdt);  			    			  
			    	}
			    	else if (line.compareTo("#END:VM")==0)
			    	{
			    		protoListener.notifyChageFinish(ChargeFinishInfo.VoltageMax);  			    			  
			    	}		    	  
			    	break;
			    	  
			      case '>':
		    		line = line.substring(1);
			    	if (isCharge)
			    	{
			    		  ChargeInfo chargeInfo = new ChargeInfo();
			    		  i = line.indexOf(':');
			    		  chargeInfo.setMinute(Integer.parseInt(line.substring(0, i)));
			    		  
			    		  line = line.substring(i+1);
			    		  i = line.indexOf(' ');
			    		  
			    		  chargeInfo.setSecond(Integer.parseInt(line.substring(0, i)));
			    		  
			    		  line = line.substring(i+1);
			    		  i = line.indexOf("mV ");
			    		  
			    		  chargeInfo.setVoltage(Integer.parseInt(line.substring(0, i)));
			    		  
			    		  line = line.substring(i+3);
			    		  i = line.indexOf("mA ");
			    		  
			    		  chargeInfo.setCurrent(Integer.parseInt(line.substring(0, i)));
			    		  
			    		  line = line.substring(i+3);
			    		  i = line.indexOf("C ");
			    		  
			    		  chargeInfo.setTemperature(Integer.parseInt(line.substring(0, i)));
			    		  
			    		  protoListener.notifyCharge(chargeInfo);
			    	}
			    	else
			    	{
			    		  DischargeInfo info = new DischargeInfo();
			    		  i = line.indexOf(':');
			    		  info.setMinute(Integer.parseInt(line.substring(0, i)));
			    		  
			    		  line = line.substring(i+1);
			    		  i = line.indexOf(' ');
			    		  
			    		  info.setSecond(Integer.parseInt(line.substring(0, i)));
			    		  
			    		  line = line.substring(i+1);
			    		  i = line.indexOf("mV ");
			    		  
			    		  info.setVoltage(Integer.parseInt(line.substring(0, i)));
			    		  
			    		  line = line.substring(i+3);
			    		  i = line.indexOf("mA ");
			    		  
			    		  info.setCurrent(Integer.parseInt(line.substring(0, i)));
	
			    		  line = line.substring(i+3);
			    		  i = line.indexOf("C ");
			    		  
			    		  info.setTemperature(Integer.parseInt(line.substring(0, i)));
			    		  
			    		  protoListener.notifyDischarge(info);
			    	}
			    	break;
			    }
		    }
	    } catch (IOException ex) {
	      System.err.println("IO Error " + ex);
	    }	
	}

	public boolean isStarted() {
		return started;
	}
	
}
