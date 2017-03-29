package felixls.ui;

import org.jfree.ui.RefineryUtilities;

import felixls.comm.PortChooser;
import felixls.comm.SerialProtocol;

public class Runner {

	public static void main(String[] args) {
		PortChooser portChooser = new PortChooser(null);
		portChooser.pack();
		RefineryUtilities.centerFrameOnScreen(portChooser);
		portChooser.setVisible(true);
	
		String portName = portChooser.getSelectedName();
				
		if (portName==null)
			System.exit(0);
		
		SerialProtocol proto = new SerialProtocol(portName, portChooser.getSelectedIdentifier());
		
		try{		
			proto.open();
	    } catch (Exception ex) {
	    	System.err.println("Error " + ex);
	    	System.exit(0);
	    }
    
	    AppFrame appFrame = new AppFrame("Univeral Battery Charger - Administrator", proto);
	    appFrame.pack();
		RefineryUtilities.centerFrameOnScreen(appFrame);
	    appFrame.setVisible(true);
	}

}
