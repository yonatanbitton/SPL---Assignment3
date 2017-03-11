package bgu.spl171.net.TFPT;

public class WRQPacket extends ServerPacket{
private String fileName;

	public WRQPacket(short opcode_par, String fileName_par) {
		super(opcode_par);
		fileName=fileName_par;
	}

	public String getFileName() {
		return fileName;
	}

}



