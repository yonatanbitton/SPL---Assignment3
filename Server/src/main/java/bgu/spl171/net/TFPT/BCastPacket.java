package bgu.spl171.net.TFPT;

public class BCastPacket extends ServerPacket{
	
	private byte deleted_or_added;
	private String fileName;
	
	public BCastPacket(short opcode_par, byte deleted_or_added_par, String fileName_par) {
		super(opcode_par);
		deleted_or_added=deleted_or_added_par;
		fileName=fileName_par;
	}
	
	public String getFileName(){
		return fileName;
	}
	
	public byte getDelOrAdd(){
		return deleted_or_added;
	}
	
}
