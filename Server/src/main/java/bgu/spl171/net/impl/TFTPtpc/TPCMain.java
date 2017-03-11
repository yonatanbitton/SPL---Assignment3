package bgu.spl171.net.impl.TFTPtpc;

import java.util.function.Supplier;

import bgu.spl171.net.TFPT.MessageEncoderDecoderImpl;
import bgu.spl171.net.TFPT.ServerPacket;
import bgu.spl171.net.TFPT.TFTPProtocolImpl;
import bgu.spl171.net.api.BidiMessagingProtocol;
import bgu.spl171.net.api.MessageEncoderDecoder;
import bgu.spl171.net.srv.Server;

public class TPCMain {

    public static void main(String[] args) {
  
        
        Supplier<MessageEncoderDecoder<ServerPacket>> encdecFactory=new Supplier<MessageEncoderDecoder<ServerPacket>>() {

			@Override
			public MessageEncoderDecoder<ServerPacket> get() {
				// TODO Auto-generated method stub
				return new MessageEncoderDecoderImpl();
			}
		};

        Supplier<BidiMessagingProtocol<ServerPacket>> protocolFactory=new Supplier<BidiMessagingProtocol<ServerPacket>>() {

			@Override
			public BidiMessagingProtocol<ServerPacket> get() {
				// TODO Auto-generated method stub
				return (BidiMessagingProtocol<ServerPacket>) new TFTPProtocolImpl();
			}
		};
		//int port = Integer.parseInt(args[0]);
	    int port = 6666;
		Server<ServerPacket> srv = Server.threadPerClient(port, protocolFactory, encdecFactory);
        srv.serve();
        
    }
}
