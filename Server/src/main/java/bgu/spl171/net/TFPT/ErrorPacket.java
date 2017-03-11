package bgu.spl171.net.TFPT;


public class ErrorPacket extends ServerPacket{
	private short errorCode;
	private String errMsg;

	public ErrorPacket(short opcode_par, short errorCode_par, String errMsg_par) {
		super(opcode_par);
		errorCode=errorCode_par;
		errMsg=errMsg_par;
	}

	public short getErrorCode() {
		return errorCode;
	}

	public String getErrMsg() {
		return errMsg;
	}

}
