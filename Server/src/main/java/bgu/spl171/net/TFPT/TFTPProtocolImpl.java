package bgu.spl171.net.TFPT;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.HashMap;

import bgu.spl171.net.api.BidiMessagingProtocol;
import bgu.spl171.net.api.bidi.Connections;
import bgu.spl171.net.srv.ConnectionsImpl;

// Protocol between the Server and a specific client
public class TFTPProtocolImpl implements BidiMessagingProtocol<ServerPacket>{

	private Integer myId=0; // Being updated be the parameter myId that was given in the constructor, PLUS the numOfUsers.
	private Connections<ServerPacket> myConnections = new ConnectionsImpl<>();
	private boolean myConnectionState;
	private boolean shouldTerminate=false;
	private static HashMap<String, Integer> loggedInUsers = new HashMap<>();
    private ArrayList<byte[]> wholeFile = new ArrayList<byte[]>();
    private int accSize=0;
    private String goingToBeReceived=null;
    //private String goingToBeSent=null;
    private int numberOfPacketsNeeded=0;
    private int lastPacketSize=0;
    private byte[] toDownloadBytesArr;
    private short blockNumToSendByAck=1;
    private short blockNumToSendByDirq=1;
    private byte[] dirqArr=null;

	// @Override
	public void start(int connectionId, Connections<ServerPacket> connections) {
            this.myId=connectionId;
            this.myConnections=connections;
          //myConnectionState=isLoggedIn(connectionId);            
   	}


	// By the assignment instructions. 
	@Override
	public void process(ServerPacket message) {
//			System.out.println("I'm at the process");
            //TODO: Take Care of getting an Acknowledgment
            short opcode=message.getOpcode();       
            if (opcode==7)
            	DealWithLOGRQ(message); // There are 2 possible errors. myConnectionState=true (I'm logged in) or there is a userName with a same userName as me.
            // DEALING WITH : RRQ, WRQ, DATA, ACK, ERROR, DIRQ, DELRQ, DISC
            //Before making any process we need to check the client is logged in, else - making a process is forbidden
            else if(myConnectionState==true){
             		ErrorPacket errorPacket=new ErrorPacket(((short)5), (short)0, "Not defined");
				    if(opcode==1)
						try {
//							System.out.println("Going to deal with RRQ");
							DealWithRRQ(message);
						} catch (IOException e) {
							throw new RuntimeException("ReadingProblem at RRQ");
						}
				    
					else if(opcode==2)
						try {
//							System.out.println("Going to deal with WRQ");
							DealWithWRQ(message);
						} catch (IOException e1) {
							throw new RuntimeException("WritingProblem at WRQ");
						}
				    
					else if(opcode==3){
//						System.out.println("Going to deal with recieving data");
				    	DealWithRecievingData(message); 
					}
				    else if(opcode==4)
						try {
//							System.out.println("Going to deal with ACK");
							DealWithACK(message);
						} catch (IOException e) {
							throw new RuntimeException("ReadingProblem at ACK");
						}
				    
					else if(opcode==5) {// If the client send ErrorPacket, I return error packet
//						System.out.println("Going to deal with ERROR");
						dealWithERROR(message);
					}
				    else if(opcode==6)
						try {
//							System.out.println("Going to deal with DIRQ");
							DealWithDIRQ(message);
						} catch (IOException e) {
							throw new RuntimeException("WritingProblem at DIRQ");
						}
				    
					else if(opcode==8){
//						System.out.println("Going to deal with DELRQ");
						DealWithDELRQ(message);
					}
				    else if(opcode==9) {// If the client send BCastPacket, I return error packet 
//						System.out.println("Send error because of getting BCastPacket");
				    	myConnections.send(myId, errorPacket);
				    }
				    else if(opcode==10) { 
//						System.out.println("Going to deal with disc");
				    	DealWithDisc(message);   
				    } else if (opcode!=7){ // Not a valid opcode
//				    	System.out.println("NOT VALID OPCODE");
				    	ErrorPacket notValidOpcode=new ErrorPacket(((short)5), ((short)4), "Illegal TFTP operation - Unknown opcode");
		                myConnections.send(myId,notValidOpcode);
				    }
           	 } else { // Tryed to make action without being logged in
    			 ErrorPacket notLoggedIn=new ErrorPacket(((short)5), ((short)6), "User not logged in");
                 myConnections.send(myId,notLoggedIn);
           	 }
   	}
	
	public void DealWithLOGRQ(ServerPacket message){
//		System.out.println("I'm dealing with the LOGRQ");
		if (myConnectionState==false){ 
            String clientName=((LOGRQPacket)message).getUserName();
            //Add a new connection to connections and update state to logged in  
        	boolean loggedIn=false;
        	if (loggedInUsers!=null){
	        	for (String s : loggedInUsers.keySet()){
	        		if (s.equals(clientName)) {
	        			 ErrorPacket alreadyLoggedIn=new ErrorPacket(((short)5), ((short)7), "User already logged in");
	                     myConnections.send(myId,alreadyLoggedIn);
	        			 // it will sent error in calling function
	                     loggedIn=true;
	        		}
	        	}
        	}
        	if (!loggedIn){
            	AckPacket ack = new AckPacket((short)4, (short)0);  // Except data & dirq
	        //	numOfUsers=numOfUsers+1;
	           //myId=myId.intValue()+numOfUsers.intValue();
	            loggedInUsers.put(clientName, myId);
//	            System.out.println("At the process, before sending ACK 0 in return");
	        	myConnections.send(myId, ack);
	            myConnectionState=true;
        	}
        }
		else { // I'm already connected
			ErrorPacket alreadyLoggedIn=new ErrorPacket(((short)5), ((short)7), "User already logged in");
            myConnections.send(myId,alreadyLoggedIn);
		}
	}


	// DISC request 
	@Override
	public boolean shouldTerminate() {
 		return shouldTerminate;
	}
	
	// Cleans the data to send
	public void dealWithERROR(ServerPacket message){
		toDownloadBytesArr=null;
		wholeFile.clear();
	    accSize=0;
	    goingToBeReceived=null;
	    numberOfPacketsNeeded=0;
	    lastPacketSize=0;
	    blockNumToSendByAck=1;
	    blockNumToSendByDirq=1;
	    dirqArr=null;
	}
	
	
	
	private void DealWithRRQ(ServerPacket message) throws IOException {
		RRQPacket p = (RRQPacket) message;
		String fileNameToDownload=p.getFileName();
		if (fileNameToDownload.length()>0){
			File[] files = new File("Files").listFiles();
			//If this pathname does not denote a directory, then listFiles() returns null. 
			boolean found=false;
			for (File file : files) {
			    if (file.isFile()) {
			        if (file.getName().equals(fileNameToDownload)) {
			        	 found=true;
			        	 FileInputStream fis = new FileInputStream(file);
			           	 toDownloadBytesArr = new byte[(int)file.length()];
			        	 fis.read(toDownloadBytesArr);
			        	 // Here the toDownloadBytesArr is full with data
			         	 fis.close();
			        	 handleReadingACK();
			        }
			    }
			}
			if (!found){
	    		ErrorPacket errorPacket = new ErrorPacket((short)5, (short)1, "File not found");
				myConnections.send(myId, errorPacket);
			}
		} else {
    		ErrorPacket errorPacket = new ErrorPacket((short)5, (short)0, "Asked to read illegal file");
			myConnections.send(myId, errorPacket);
		}
	}
	
	
	// Cuts from b the first 512bytes and sends it with the appropriate blockNum
	private void handleReadingACK() throws IOException{		 
    	// b is a large byte array
//		System.out.println("I'm at handleReadingACK");
		if (toDownloadBytesArr!=null){
	    	numberOfPacketsNeeded=(toDownloadBytesArr.length / 512);
	    	lastPacketSize = toDownloadBytesArr.length % 512;
//	    	System.out.println("numberOfPacketsNeeded is " + numberOfPacketsNeeded);
//	    	System.out.println("LastPacketSize is " + lastPacketSize);
	    	if (lastPacketSize>=0) numberOfPacketsNeeded++;
//	    	System.out.println("numberOfPacketsNeeded is " + numberOfPacketsNeeded);
	    	
	    	// If there is a need for >2 packets, create the first one, shorten the big one, remember who we sent, reduce the number of packets needed, and send.
	    	if (toDownloadBytesArr.length>=512) {
	    		byte[] prefix = cutFirst512toDownloadBytesArr();
	    		numberOfPacketsNeeded--; 
	    		DataPacket ans = new DataPacket((short)3, (short)prefix.length, (short) blockNumToSendByAck, prefix);
	    		blockNumToSendByAck++;
				myConnections.send(myId, ans);
	    	}
	    	if (numberOfPacketsNeeded==1)	    	
	    		if(lastPacketSize>0){
		    		DataPacket ans = new DataPacket((short)3, (short)toDownloadBytesArr.length, (short) blockNumToSendByAck, toDownloadBytesArr);
					myConnections.send(myId, ans);
					toDownloadBytesArr=null;
					blockNumToSendByAck=1;
	    		}
	    		else{ //lastPacketSize=0
//		    		DataPacket ans = new DataPacket((short)3, (short)toDownloadBytesArr.length, (short) blockNumToSendByAck, toDownloadBytesArr);
//					myConnections.send(myId, ans);
					byte[] emptyData = new byte[0];
					DataPacket emptyPacket = new DataPacket((short)3, (short)0, (short)blockNumToSendByAck, emptyData);
					myConnections.send(myId, emptyPacket);
					toDownloadBytesArr=null;
					blockNumToSendByAck=1;
	    		}
//	    	{
//	    		System.out.println("Number of packets Needed = 1. Prepering DataPacket");
//	    		DataPacket ans = new DataPacket((short)3, (short)toDownloadBytesArr.length, (short) blockNumToSendByAck, toDownloadBytesArr);
//				myConnections.send(myId, ans);
//				toDownloadBytesArr=null;
//				blockNumToSendByAck=1;
//	    	}
    	} else {
    	}
	}
	
	// Here i'm dealing with continues reading request.
	private void DealWithACK(ServerPacket message) throws IOException{
		AckPacket p = (AckPacket) message;
		//p.getBlockNum();
 		if ((toDownloadBytesArr != null) && (toDownloadBytesArr.length>0)) handleReadingACK(); // If there is something to read - Handle it! 
 		else if (dirqArr!=null && dirqArr.length>0) handleReadingDIRQ();
	}
	
	private void handleReadingDIRQ() throws IOException{		 
    	// b is a large byte array
    	numberOfPacketsNeeded=(dirqArr.length / 512);
    	lastPacketSize = dirqArr.length % 512;
    	if (lastPacketSize>0) numberOfPacketsNeeded++;
    	
    	
    	// If there is a need for >2 packets, create the first one, shorten the big one, remember who we sent, reduce the number of packets needed, and send.
    	if (dirqArr.length>=512) {
    		byte[] prefix = cutFirst512dirqArr();
    		numberOfPacketsNeeded--; 
    		DataPacket ans = new DataPacket((short)3, (short)prefix.length, (short) blockNumToSendByDirq, prefix);
    		blockNumToSendByAck++;
			myConnections.send(myId, ans);
    	}
    	if (numberOfPacketsNeeded==1)
    	{
    		DataPacket ans = new DataPacket((short)3, (short)dirqArr.length, (short) blockNumToSendByDirq, dirqArr);
			myConnections.send(myId, ans);
			dirqArr=null;
			blockNumToSendByDirq=1;
    	}		   
	}
	
	// If the client requests to upload a file to the server
	// If exists - errorPacket, if not, I receive it in HandelWithData
	private void DealWithWRQ(ServerPacket message) throws IOException // Sends a file to me
	{
		WRQPacket p = (WRQPacket) message;
		String fileNameToUpload=p.getFileName();
 		//If this pathname does not denote a directory, then listFiles() returns null. 
		if (fileNameToUpload.length()>0){
			File[] files = new File("Files").listFiles();
			//If this pathname does not denote a directory, then listFiles() returns null. 
			boolean found=false;
			for (File file : files) {
			    if (file.isFile()) {
			        if (file.getName().equals(fileNameToUpload)) {
			    		ErrorPacket errorPacket = new ErrorPacket((short)5, (short)5, "File already exists");
						myConnections.send(myId, errorPacket);
						found=true;
			        }		     
			    }
			}
	    	if (!found) {
	    		goingToBeReceived=fileNameToUpload;
	    		AckPacket ackPacket = new AckPacket((short)4, (short)(0));
				myConnections.send(myId, ackPacket);
				accSize=0;
				wholeFile = new ArrayList<>();
	    	}
		}
		else {
			ErrorPacket wrongInput = new ErrorPacket((short)5, (short)0, "The fileName to download is illegal");
			myConnections.send(myId, wrongInput);
		}
	}
	
	private void DealWithRecievingData(ServerPacket message)
	{
		ErrorPacket errorPacket = new ErrorPacket((short)5, (short)2, "File cannot be written, read or deleted");
		if (goingToBeReceived==null) {}
		else { // There is a file waiting to be received
			DataPacket p = (DataPacket) message;
			byte[] data = p.getData();
			short packetSize = p.getPacketSize();
			short blockNum = p.getBlockNum();
			if (data.length != packetSize){
				ErrorPacket wrongInput = new ErrorPacket((short)5, (short)0, "The packet received is larger then 512!");
				myConnections.send(myId, wrongInput);
			}
			else if (packetSize>512) {
				ErrorPacket wrongInput = new ErrorPacket((short)5, (short)0, "The packet received is larger then 512!");
				myConnections.send(myId, wrongInput);
				wholeFile.clear();
			}
			else if (packetSize==512) {
				byte[] lastData = p.getData();
				wholeFile.add(lastData);
				accSize=accSize+packetSize;
				AckPacket ackPacket = new AckPacket((short)4, blockNum);
				myConnections.send(myId, ackPacket);
			}
			else if (packetSize<512){ // Knows I need to assemble the file.
				byte[] lastData = p.getData();
				wholeFile.add(lastData);
				//File freeSpace = new File("Files/"); // Checks if there is enough space
				//long space = freeSpace.getFreeSpace();
				accSize=accSize+packetSize;
				byte[] res = new byte[accSize];
				int index=0;
	 			while(!wholeFile.isEmpty()) {
				    byte[] tmpArr=wholeFile.remove(0);
					for (int j=0; j<tmpArr.length; j++){
						res[index]=tmpArr[j];
						index++;
					}					
				}
//	 			if (res.length>space) {
//	 				ErrorPacket error = new ErrorPacket((short)5, (short)3, "Disk full or allocation exceeded - no room in disk");
//	 				myConnections.send(numOfUsers, error);
//	 			}
//	 			else { // There is enough space, can create the new file with the result
	 				String path = "Files"+File.separator+goingToBeReceived;
	 				try {
						FileOutputStream stream = new FileOutputStream(path);
							stream.write(res);
							stream.close();
					} catch (FileNotFoundException e) {
						throw new RuntimeException("Deal with data error have been thrown, FLAG 1");
					} catch (IOException e) {
						throw new RuntimeException("Deal with data error have been thrown, FLAG 2");
					}		
					AckPacket ackPacket = new AckPacket((short)4, blockNum);
					myConnections.send(myId, ackPacket);
	 				BCastPacket broad = new BCastPacket((short)9, (byte) 1, goingToBeReceived);
	 				for (Integer s : loggedInUsers.values()){
	 					myConnections.send(s.intValue(), broad);
	 				}
	 				goingToBeReceived=null;
		 							
			}				
		}
	}
	 
	private void DealWithDIRQ(ServerPacket message) throws IOException
	{ 
		ArrayList<String> results = new ArrayList<String>();
		String path = "Files";
		File[] files = new File(path).listFiles();
		//If this pathname does not denote a directory, then listFiles() returns null. 
		for (File file : files) {
		    if (file.isFile()) {		    	
		        results.add(file.getName());
		    }
		}
		String ans_str="";
		int firstSize=results.size();
		for (int i = 0; i < firstSize; i++) {
			ans_str=ans_str+(results.remove(0)+"\0");	
		}
		byte[] ans_bytes = ans_str.getBytes();
		if (ans_bytes.length>0)
			handleSendingData(ans_bytes, (short)1);
		else {
			byte[] emptyData = new byte[0];
			DataPacket emptyPacket = new DataPacket((short)3, (short)0, (short)1, emptyData);
			myConnections.send(myId, emptyPacket);
		}
	}
	
	private void handleSendingData(byte[] b, short blockNumToSend) throws IOException{
    	// b is a large byte array
    	numberOfPacketsNeeded=(b.length / 512);
    	lastPacketSize = b.length % 512;
    	if (lastPacketSize>0) numberOfPacketsNeeded++;
    	// If there is a need for 2 packets, create the first one, shorten the big one, remember who we sent, reduce the number of packets needed, and send.
    	if (b.length>=512) {
    		byte[] prefix = cutFirst512dirqArr(); // b array was cut down
    		numberOfPacketsNeeded--; 
    		DataPacket ans = new DataPacket((short)3, (short)prefix.length, (short) blockNumToSend, prefix);
			myConnections.send(myId, ans);
			if (toDownloadBytesArr!=null) throw new RuntimeException("MD Ashakran"); // NOT RELATED
			dirqArr=b;			
    	}
    	if (numberOfPacketsNeeded==1)
    	{
    		if(lastPacketSize>0){
	    		DataPacket ans = new DataPacket((short)3, (short)b.length, (short) blockNumToSend, b);
				myConnections.send(myId, ans);
    		}
    		else{ //lastPacketSize=0
//    			DataPacket ans = new DataPacket((short)3, (short)b.length, (short) blockNumToSend, b);
//				myConnections.send(myId, ans);
				byte[] emptyData = new byte[0];
				DataPacket emptyPacket = new DataPacket((short)3, (short)0, (short)blockNumToSend, emptyData);
				myConnections.send(myId, emptyPacket);
    		}
    	}		   
	}
	
	private void DealWithDELRQ(ServerPacket message)
	{
		DELRQPacket p = (DELRQPacket) message;
		String fileToDelete = p.getFileName();
		File[] files = new File("Files").listFiles();
		//If this pathname does not denote a directory, then listFiles() returns null. 
		boolean deleted=false;
		if (fileToDelete.length()>0){
			for (File file : files) {
			    if (file.isFile()) {
			        if (file.getName().equals(fileToDelete)) {
			        	 file.delete();
			        	 AckPacket ack = new AckPacket((short)4, (short)0);  // Except data & dirq
			 			 myConnections.send(myId, ack); // Send ack or error. If sends ack, now the client sends the file. 
			 			 deleted=true;
			 			 BCastPacket broad = new BCastPacket((short)9, (byte) 0, fileToDelete);
			 			 for (Integer s : loggedInUsers.values()){
		 					myConnections.send(s.intValue(), broad);
			 			 }	
			        }
			    }
			}
			if (!deleted){  // FILE NOT FOUND
				String s=  "RRQ / DELRQ of non-existing file";
				ErrorPacket error = new ErrorPacket((short)5, (short)1, s);
				myConnections.send(myId, error);
			}	
		} else {
			ErrorPacket wrongInput = new ErrorPacket((short)5, (short)0, "The fileName to DELETE is illegal");
			myConnections.send(myId, wrongInput);
		}
	}
	
	private void DealWithDisc(ServerPacket message)
	{
          // DiscPacket p = (DiscPacket) message; 
		   shouldTerminate=true;
		   String keyRepresent = findKeyRepresentMyId();
           loggedInUsers.remove(keyRepresent);
		   //numOfUsers=numOfUsers-1; // Reduce the number of connected
           AckPacket packet = new AckPacket((short)4, (short)0);
           myConnections.send(myId, packet);
           //myConnections.disconnect(numOfUsers);
	}
	
	private String findKeyRepresentMyId(){
		String key=null;
		for (HashMap.Entry<String, Integer> entry : loggedInUsers.entrySet()){
			String tmpKey = entry.getKey();
			Integer tmpId = entry.getValue();
			if (myId.intValue()==tmpId.intValue())
				key = tmpKey;
		}
		return key;
	}
	 
	// Takes 512 from toDownloadBytesArr and return prefix=512 bytes
	private byte[] cutFirst512toDownloadBytesArr(){
		byte[] prefix = new byte[512];
		for (int i=0; i<512; i++){
			prefix[i]=toDownloadBytesArr[i];
		}
		byte[] rest=new byte[toDownloadBytesArr.length-512];
		for(int j=0;j<rest.length;j++){
			rest[j]=toDownloadBytesArr[j+512];
		}
		toDownloadBytesArr=rest;
		return prefix;
	}
	
	private byte[] cutFirst512dirqArr(){
		byte[] prefix = new byte[512];
		for (int i=0; i<512; i++){
			prefix[i]=dirqArr[i];
		}
		byte[] rest=new byte[dirqArr.length-512];
		for(int j=0;j<rest.length;j++){
			rest[j]=dirqArr[j+512];
		}
		dirqArr=rest;
		return prefix;
	}
	
	public HashMap<String, Integer> getLoggedInUsers(){
		return loggedInUsers;
	}
	
	public void InitializeLoggedInUsers(){
		loggedInUsers.clear();
	}
	
	public void addToLoggedInUsers(String name, int id){
		loggedInUsers.put(name, id);
	}
	
	public void setConnectionState(boolean bool){
		myConnectionState=bool;
	}
 
	
	public String getGoingToBeReceived(){
		return goingToBeReceived;
	}
	
	public void setGoingToBeReceived(String x){
		goingToBeReceived=x;
	}


	public boolean getConnectionState() {
		// TODO Auto-generated method stub
		return myConnectionState;
	}
	
}