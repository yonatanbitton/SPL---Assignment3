package bgu.spl171.net.impl.TFTPreactor;
import bgu.spl171.net.TFPT.MessageEncoderDecoderImpl;
import bgu.spl171.net.TFPT.TFTPProtocolImpl;
import bgu.spl171.net.srv.Server;

public class ReactorMain {
	public static void main(String[] args) {
	    System.out.println ("Serve!");
        Server.reactor(
                Runtime.getRuntime().availableProcessors(),
                6666, //port
                () ->   new TFTPProtocolImpl(), //protocol factory
                MessageEncoderDecoderImpl::new //message encoder decoder factory
        ).serve();

    }

}