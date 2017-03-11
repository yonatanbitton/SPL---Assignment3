package bgu.spl171.net.TFPT;

public class RRQPacket extends ServerPacket {
	private String fileName;

	public RRQPacket(short opcode_par, String fileName_par) {
		super(opcode_par);
		fileName=fileName_par;
	}

	public String getFileName() {
		return fileName;
	}

	}
