package bgu.spl171.net.TFPT;

public class AckPacket extends ServerPacket{
	private short blockNum;

	public AckPacket(short opcode_par, short blockNum_par) {
		super(opcode_par);
		this.blockNum=blockNum_par;
	}
	
	public short getBlockNum(){
		return blockNum;
	}

}
