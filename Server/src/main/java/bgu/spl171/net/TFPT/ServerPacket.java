package bgu.spl171.net.TFPT;

public class ServerPacket {
	private short opcode;

	/** Constructor
     * Initialize the ServerPacket
     * @param opcode - the opcode to initialize;
     */
	public ServerPacket(short opcode_par) {
		opcode = opcode_par;
	}
	
	public short getOpcode (){
		return opcode;
	}

}
