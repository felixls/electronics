package felixls.comm;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;

import gnu.io.*;

public class SerialComm{

	public static final int TIMEOUTSECONDS = 30;
	public static final int BAUD = 9600;
	protected String portName;
	protected CommPortIdentifier portIdentifier;
	protected SerialPort port;
	protected BufferedReader reader;
	protected OutputStreamWriter writer;

	public SerialComm(String portName, CommPortIdentifier portIdentifier)	{
		this.portName = portName;
		this.portIdentifier = portIdentifier;		
	}
	
	public void open() throws IOException, PortInUseException, UnsupportedCommOperationException {
		port = (SerialPort) portIdentifier.open("felixls", TIMEOUTSECONDS * 1000);

		port.setSerialPortParams(BAUD, SerialPort.DATABITS_8,
				SerialPort.STOPBITS_1, SerialPort.PARITY_NONE);

		reader = new BufferedReader(new InputStreamReader(port.getInputStream()));
	}
}
