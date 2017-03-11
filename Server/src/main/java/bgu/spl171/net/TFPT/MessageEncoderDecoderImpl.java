package bgu.spl171.net.TFPT;

import java.nio.charset.StandardCharsets;
import java.util.Arrays;

import bgu.spl171.net.api.MessageEncoderDecoder;

public class MessageEncoderDecoderImpl implements MessageEncoderDecoder<ServerPacket>{
	private byte[] dataToDecode =new byte [518];
	private short opcode;
	private byte[] opcodeBytes=new byte[2];
	private boolean startedInitializingOpCode=false;
	private boolean isOpCodeInitialized=false;
	private boolean doneDecoding=false;
	private int len=0;
	private int lenDana=2;
	private boolean startedACKBlockNum=false;
	private byte[] blockNumBytes=new byte[2];
	private boolean finishedAckBlockNum=false;
	private boolean startedPacketSize=false;
	private boolean finishedPacketSize=false;
	private byte[] packetSizeByte=new byte[2];
	private boolean startedDataBlockNum=false;
	private boolean finishedDataBlockNum=false;
	private byte[] blockNumDataBytes=new byte[2];
	short packetSize=0;
	short blockNum=0;
	short leftToRead;
	byte[] data;
	int index=0;
	int currPos=0;
	private boolean removedOpcode=false;
	private boolean readSecond=false;

	// TODO: Handle the DataPacket Case
	@Override
	public ServerPacket decodeNextByte(byte nextByte) {
		ServerPacket packet_6_OR_10=null;
			if (!isOpCodeInitialized) {
//				System.out.println("Should be printed twice ");
				packet_6_OR_10= initializeOpCode_and_dataToDecode_or_returnRelevantPackets(nextByte);
			}
			if (!removedOpcode && isOpCodeInitialized && readSecond) { 
				removedOpcode=true;
				popTwoFirstBytes(dataToDecode);
			}
			// When entering this segment, the opcode is initialized
			if (packet_6_OR_10!=null){ // It means we need to return DIRQPacket(6) or DISCPacket(10)
				cleanFields();
				return packet_6_OR_10;
			}
			else {// When entering this segment, the opcode is initialized and NOT 6 or 10
			
			if (readSecond){
				// Zero at the end:  BCastPacket(9), DELRQPacket(8), LOGRQ(7), ErrorPacket(5), WRQPacket(2), RRQPacket(1)
				ServerPacket packet = handleCasesWithZeroAtTheEnd(nextByte); // If the nextByte='\0', the function return the relevant packet
				if (packet!=null) {
					cleanFields();
					return packet; 
					}// return BCastPacket(9), DELRQPacket(8), LOGRQ(7), ErrorPacket(5), WRQPacket(2), RRQPacket(1)
				// DataPacket(3) or AckPacket(4)
				if (opcode==3) {
					DataPacket d = readDataPacket(nextByte);
					if (d!=null) {
						cleanFields();
						return d; 
					} 
					else return null; // Why return null ? Because at this cases I have to keep reading ACCORDING the packetSize
				}
				if (opcode==4) {
					AckPacket d = readAckPacket(nextByte);
					if (d!=null) {
						cleanFields();
						return d;
					}
					else return null; // Why return null ? Because at this cases I have to keep reading ACCORDING the blockNum
				}
			}
			// STILL READING 
			pushByte(nextByte); // Meaning that i'm still at the reading of the message, (Or at DataPacket case) 
			currPos++;
//			System.out.println("The currPos is " + currPos);
			readSecond=isOpCodeInitialized;
			return null;
			
		}
	}
	
	public void cleanFields(){
//		for (int i=0; i<518; i++)
//			dataToDecode[i]=0;
		dataToDecode =new byte [518];
		opcodeBytes[0]=0;
		opcodeBytes[1]=0;
		opcodeBytes=new byte[2];
		startedInitializingOpCode=false;
		isOpCodeInitialized=false;
		doneDecoding=false;
		len=0;
		lenDana=2;
		startedACKBlockNum=false;
		blockNumBytes[0]=0;
		blockNumBytes[1]=0;
		blockNumBytes=new byte[2];
		finishedAckBlockNum=false;
		startedPacketSize=false;
		finishedPacketSize=false;
		packetSizeByte[0]=0;
		packetSizeByte[1]=0;
		packetSizeByte=new byte[2];
		startedDataBlockNum=false;
		finishedDataBlockNum=false;
		blockNumDataBytes[0]=0;
		blockNumDataBytes[1]=0;
		blockNumDataBytes=new byte[2];
	    packetSize=0;
		blockNum=0;
		index=0;
		currPos=0;
		removedOpcode=false;
		readSecond=false;
		opcode=0;
	}
	
	public DataPacket readDataPacket(byte nextByte){
		if(startedPacketSize==false){
			startedPacketSize=true;
			packetSizeByte[0]=nextByte;
			return null;
		}
		else if(finishedPacketSize==false){
				packetSizeByte[1]=nextByte;
				finishedPacketSize=true;
				packetSize=bytesToShort(packetSizeByte);
				return null;
		}
		else{
			// At this point, we have the packetSize, and i'm reading the nextByte
			
			if(startedDataBlockNum==false){
				blockNumDataBytes[0]=nextByte;
				startedDataBlockNum=true;
				return null;
			}
			else if(finishedDataBlockNum==false){
					blockNumDataBytes[1]=nextByte;
					finishedDataBlockNum=true;
					blockNum=bytesToShort(blockNumDataBytes);
					leftToRead=packetSize; // Before: leftToRead=packetSize
				//	System.out.println("Left to read is " + leftToRead);
					data=new byte[leftToRead];
					return null;
				}
			else {
				//At this point, we also have the blockNum, and I'm reading the nextByte
						if(leftToRead>1){
						data[index]=nextByte;
						index++;
						leftToRead=(short) (leftToRead-(short)1);
						return null;
					}
					else{ // Nothing left to read - FINISHED 
						if (leftToRead==1){
						data[index]=nextByte;
						DataPacket packet=new DataPacket(opcode, packetSize, blockNum, data);
						return packet;
					}									
				}
			}
		}
		return null;
	}
	
	
	public AckPacket readAckPacket(byte nextByte){
		if (startedACKBlockNum==false) {
			blockNumBytes[0]=nextByte;
			startedACKBlockNum=true;
			return null;
		}
		else { 
			if (finishedAckBlockNum==false)
				blockNumBytes[1]=nextByte;
			finishedAckBlockNum=true;
			short blockNum = bytesToShort(blockNumBytes);
			AckPacket packet = new AckPacket(opcode, blockNum);
			return packet;
		}
	}

			
	
	// TODO: Problem with the opcode of '10' and '1'
	// Checks the 6 cases of BCastPacket(9), ErrorPacket(5), RRQPacket(1), WRQPacket(2), DELRQPacket(8), LOGRQPacket(7)
	public ServerPacket handleCasesWithZeroAtTheEnd(byte nextByte){
		if (nextByte=='\0') {
			if (opcode==9){ // Meaning we need to return a BCastPacket
				byte deleted_or_added=returnFirstByte(dataToDecode);
				popFirstByte(dataToDecode);
			//	popLastByte(dataToDecode);
			 	dataToDecode=Arrays.copyOfRange(dataToDecode, 0, currPos-2); // Getting rid from the finish '0' and the rest
				String s = popString(dataToDecode);
				BCastPacket packet = new BCastPacket(opcode,deleted_or_added,s);
				return packet;
			}
			
			else if (opcode==8){ // Meaning we need to return a DELRQPacket
				dataToDecode=Arrays.copyOfRange(dataToDecode, 0, currPos-2); // Getting rid from the finish '0' and the rest
				//	popLastByte(dataToDecode);
				String s = popString(dataToDecode);
				DELRQPacket packet = new DELRQPacket(opcode,s);
				return packet;
			}
			else if (opcode==7){ // Meaning we need to return a LOGRQPacket
				//popLastByte(dataToDecode);
				dataToDecode=Arrays.copyOfRange(dataToDecode, 0, currPos-2); // Getting rid from the finish '0' and the rest
				String s = popString(dataToDecode);
				LOGRQPacket packet = new LOGRQPacket(opcode,s);
				return packet;
			}
			else if (opcode==5){ // Meaning we need to return a ErrorPacket
				short errorCode=returnTwoFirstBytes(dataToDecode);
				popTwoFirstBytes(dataToDecode);
				dataToDecode=Arrays.copyOfRange(dataToDecode, 0, currPos-2); // Getting rid from the finish '0' and the rest
				//	popLastByte(dataToDecode);
				String s = popString(dataToDecode);
				ErrorPacket packet = new ErrorPacket(opcode, errorCode, s);
				return packet;
			}
			else if (opcode==2){ // Meaning we need to return a WRQPacket
				dataToDecode=Arrays.copyOfRange(dataToDecode, 0, currPos-2); // Getting rid from the finish '0' and the rest
				//	popLastByte(dataToDecode);
				String s = popString(dataToDecode);
				WRQPacket packet = new WRQPacket(opcode, s);
				return packet;
			}
			else if (opcode==1){ // Meaning we need to return a RRQPacket
			//	popLastByte(dataToDecode);
				dataToDecode=Arrays.copyOfRange(dataToDecode, 0, currPos-2); // Getting rid from the finish '0' and the rest
				String s = popString(dataToDecode);
				RRQPacket packet = new RRQPacket(opcode, s);
				return packet;
			}
		}
	return null;
	}
	
	public ServerPacket initializeOpCode_and_dataToDecode_or_returnRelevantPackets(byte nextByte){
		if (!startedInitializingOpCode) { 
			//pushByte(nextByte);
			opcodeBytes[0]=nextByte;
			startedInitializingOpCode=true;
			return null;
		}
		else {
			opcodeBytes[1]=nextByte;
			//pushByte(nextByte);
			this.opcode=bytesToShort(opcodeBytes);
			isOpCodeInitialized=true;
		//	popTwoFirstBytes(dataToDecode);
			if (opcode==6) return new DIRQPacket(opcode);
			else if (opcode==10) return new DiscPacket(opcode);
			else return null;
		}
	}

	@Override
	public byte[] encode(ServerPacket message) {
		short opcode=((ServerPacket)message).getOpcode();
		byte[] opcodeBytes=shortToBytes(opcode);
		
		if(opcode==1){
			byte[] response = encodeRRQ(message, opcodeBytes);
			return response;
		}
		
		else if (opcode==2){
			byte[] response=encodeWRQ(message, opcodeBytes);
			return response;
		}
		
		else if (opcode==3){
			byte[] response=encodeData(message, opcodeBytes);
			return response;
		}
		
		else if (opcode==4){
			byte[] response=encodeAck(message, opcodeBytes);
			return response;
		}
		
		else if (opcode==5){
			byte[] response=encodeError(message, opcodeBytes);
			return response;			
		}
		
		else if (opcode==6){
			return opcodeBytes;
		}
		
		else if (opcode==7){
			byte[] response=encodeLogRQ(message, opcodeBytes);
			return response;			
		}
		
		else if (opcode==8){
			byte[] response=encodeDelRQ(message, opcodeBytes);
			return response;	
		}
		
		else if (opcode==9){
			byte[] response=encodeBCast(message, opcodeBytes);
			return response;	
		}
		
		else if (opcode==10){
			return opcodeBytes;
		}
		
		return null;
	}
	
	
	private byte[] encodeRRQ(ServerPacket message, byte[] opcodeBytes){
		String fileName=((RRQPacket)message).getFileName();
		byte[] fileNameBytes=fileName.getBytes();
		lenDana=lenDana+fileNameBytes.length+1;
		byte[] response=new byte[lenDana];
		for(int i=0;i<2;i++){
			response[i]=opcodeBytes[i];
		}
		for(int j=0; j<fileNameBytes.length;j++){
			response[j+2]=fileNameBytes[j];
		}
		response[response.length-1]=0;
		lenDana=2; // EXPERIMENTIAL
		return response;
	}
	
	
	private byte[] encodeWRQ(ServerPacket message, byte[] opcodeBytes){
		String fileName=((WRQPacket)message).getFileName();
		byte[] fileNameBytes=fileName.getBytes();
		lenDana=lenDana+fileNameBytes.length+1;
		byte[] response=new byte[lenDana];
		for(int i=0;i<2;i++){
			response[i]=opcodeBytes[i];
		}
		for(int j=0; j<fileNameBytes.length;j++){
			response[j+2]=fileNameBytes[j];
		}
		response[response.length-1]=0;
		lenDana=2; // EXPERIMENTIAL
		return response;
	}
	
	private byte[] encodeData(ServerPacket message, byte[] opcodeBytes){
		short packetSize=((DataPacket)message).getPacketSize();
		byte[] packetSizeBytes=shortToBytes(packetSize);
		short blockNum=((DataPacket)message).getBlockNum();
		byte[] blockNumBytes=shortToBytes(blockNum);
		byte[] DataBytes=((DataPacket)message).getData();
		
		lenDana=lenDana+packetSizeBytes.length+blockNumBytes.length+DataBytes.length;
		byte[] response=new byte[lenDana];
		for(int i=0;i<2;i++){
			response[i]=opcodeBytes[i];
		}
		for(int j=0; j<2;j++){
			response[j+2]=packetSizeBytes[j];
		}
		for(int k=0;k<2;k++){
			response[k+4]=blockNumBytes[k];
		}
		for(int p=0; p<DataBytes.length;p++){
			response[p+6]=DataBytes[p];
		}
		lenDana=2; // EXPERIMENTIAL
		return response;
	}
	
	private byte[] encodeAck(ServerPacket message, byte[] opcodeBytes){
		short blockNum=((AckPacket)message).getBlockNum();
		byte[] blockNumBytes=shortToBytes(blockNum);
		
		lenDana=lenDana+2;
		byte[] response=new byte[lenDana];
		for(int i=0;i<2;i++){
			response[i]=opcodeBytes[i];
		}
		for(int j=0; j<2;j++){
			response[j+2]=blockNumBytes[j];
		}	
		lenDana=2; // EXPERIMENTIAL
		return response;
	}
	
	private byte[] encodeError(ServerPacket message, byte[] opcodeBytes){
		short errorCode=((ErrorPacket)message).getErrorCode();
		byte[] errorCodeBytes=shortToBytes(errorCode);
		String errMsg=((ErrorPacket)message).getErrMsg();
		byte[] errMsgBytes=errMsg.getBytes();
		
		
		lenDana=opcodeBytes.length+errorCodeBytes.length+errMsgBytes.length+1;
		byte[] response=new byte[lenDana];
		
		for(int i=0;i<2;i++){
			response[i]=opcodeBytes[i];
		}
		for(int j=0;j<2;j++){
			response[j+2]=errorCodeBytes[j];
		}
		for(int j=0;j<errMsgBytes.length;j++){
			response[j+4]=errMsgBytes[j];
		}
		response[response.length-1]=0;
		lenDana=2; // EXPERIMENTIAL
		return response;
	}
	
	private byte[] encodeLogRQ(ServerPacket message, byte[] opcodeBytes){
		String userName=((LOGRQPacket)message).getUserName();
		byte[] userNameBytes=userName.getBytes();
		lenDana=lenDana+userNameBytes.length+1;
		byte[] response=new byte[lenDana];
		for(int i=0;i<2;i++){
			response[i]=opcodeBytes[i];
		}
		for(int j=0; j<userNameBytes.length;j++){
			response[j+2]=userNameBytes[j];
		}
		response[response.length-1]=0;
		lenDana=2; // EXPERIMENTIAL
		return response;
	}
	
	private byte[] encodeDelRQ(ServerPacket message, byte[] opcodeBytes){
		String fileName=((DELRQPacket)message).getFileName();
		byte[] fileNameBytes=fileName.getBytes();
		lenDana=lenDana+fileNameBytes.length+1;
		byte[] response=new byte[lenDana];
		for(int i=0;i<2;i++){
			response[i]=opcodeBytes[i];
		}
		for(int j=0; j<fileNameBytes.length;j++){
			response[j+2]=fileNameBytes[j];
		}
		response[response.length-1]=0;
		lenDana=2; // EXPERIMENTIAL
		return response;
	}
	
	private byte[] encodeBCast(ServerPacket message, byte[] opcodeBytes){
		byte delOrAdd=((BCastPacket)message).getDelOrAdd();
		String fileName=((BCastPacket)message).getFileName();
//		System.out.println("The filename string's length is " + fileName.length());
		byte[] fileNameBytes=fileName.getBytes();
//		System.out.println("The fileNameBytes array size is " + fileNameBytes.length);
		int size = fileNameBytes.length-1;
//		System.out.println("The last byte is " + fileNameBytes[size]);
//		System.out.println("The lenDana BEFORE is " + lenDana);
		lenDana=lenDana+1+fileNameBytes.length+1;
		byte[] response=new byte[lenDana];
//		System.out.println("The lenDana AFTER is " + lenDana);
		for(int i=0;i<2;i++){
			response[i]=opcodeBytes[i];
		}
		
		response[2]=delOrAdd;
		
		for(int j=0;j<fileNameBytes.length;j++){
			response[j+3]=fileNameBytes[j];
		}
		
		response[response.length-1]=0;
		lenDana=2; // EXPERIMENTIAL
		return response;	
	}
	

	
	
    public short bytesToShort(byte[] byteArr)
    {
        short result = (short)((byteArr[0] & 0xff) << 8);
        result += (short)(byteArr[1] & 0xff);
        return result;
    }
    
    public byte[] shortToBytes(short num)
    {
        byte[] bytesArr = new byte[2];
        bytesArr[0] = (byte)((num >> 8) & 0xFF);
        bytesArr[1] = (byte)(num & 0xFF);
        return bytesArr;
    }
    
    public short returnTwoFirstBytes(byte[] dataToDecode){
		byte[] newToDecode= new byte[2];
		for(int i=0; i<2;i++)
			newToDecode[i]=dataToDecode[i];
		short returnShort=bytesToShort(newToDecode);
		return returnShort;
    }
    
    public void popTwoFirstBytes(byte[] dataToDecode){
		byte[] newDataToDecode= new byte[dataToDecode.length-2];
		for(int i=0; i<newDataToDecode.length;i++)
			newDataToDecode[i]=dataToDecode[i+2];
		this.dataToDecode=newDataToDecode;
		this.len=this.len-2;
    }
    
    public byte returnFirstByte(byte[] dataToDecode){
		byte firstByte=dataToDecode[0];
		return firstByte;
    }
    
    public void popFirstByte(byte[] dataToDecode){
		byte[] newDataToDecode= new byte[dataToDecode.length-1];
		for(int i=0; i<newDataToDecode.length;i++)
			newDataToDecode[i]=dataToDecode[i+1];
		this.dataToDecode=newDataToDecode;
    }
    
    public void popLastByte(byte[] dataToDecode){
		byte[] newDataToDecode= new byte[dataToDecode.length-1];
		for(int i=1; i<newDataToDecode.length;i++)
			newDataToDecode[i]=dataToDecode[i];
		this.dataToDecode=newDataToDecode;
    }
    
    private String popString(byte[] bytes) {
        //notice that we explicitly requesting that the string will be decoded from UTF-8
        //this is not actually required as it is the default encoding in java.
//    	System.out.println("Before the popString, the length is " + bytes.length);
    	int len=bytes.length;
        String result = new String(bytes, 0, len, StandardCharsets.UTF_8);
        len = 0;
        return result;
    }
    
   // Not for DataPacket
    private void pushByte(byte nextByte) {
        if (len >= dataToDecode.length) {
            dataToDecode = Arrays.copyOf(dataToDecode, len * 2);
        }

        dataToDecode[len++] = nextByte;
    }
    
}