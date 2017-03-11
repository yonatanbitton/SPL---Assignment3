package bgu.spl171.net.TFPT;

public class LOGRQPacket extends ServerPacket{
	private String userName;
	
	public LOGRQPacket(short opcode_par, String userName_par) {
		super(opcode_par);
		userName=userName_par;
	}
	
	public String getUserName(){
		return userName;
	}

}
