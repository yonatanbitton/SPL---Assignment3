package bgu.spl171.net.TFPT;

public class DataPacket extends ServerPacket{
	private short opcode;
	private short packetSize; 
	private short blockNum;
	private byte[] data;
	
	public DataPacket(short opcode_par, short packetSize_par, short blockNum_par, byte[] data_par) {
		super(opcode_par);
		opcode = opcode_par;
		packetSize = packetSize_par;
		blockNum = blockNum_par;
		data = data_par;
	}

	public short getPacketSize() {
		return packetSize;
	}


	public short getBlockNum() {
		return blockNum;
	}


	public byte[] getData() {
		return data;
	}

	
}
