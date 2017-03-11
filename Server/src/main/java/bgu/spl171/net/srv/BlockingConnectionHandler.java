package bgu.spl171.net.srv;

import bgu.spl171.net.api.BidiMessagingProtocol;
import bgu.spl171.net.api.MessageEncoderDecoder;
import bgu.spl171.net.srv.bidi.ConnectionHandler;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.IOException;
import java.net.Socket;
import java.nio.charset.StandardCharsets;

public class BlockingConnectionHandler<T> implements Runnable, ConnectionHandler<T> {

    private final BidiMessagingProtocol protocol;
    private final MessageEncoderDecoder<T> encdec;
    private final Socket sock;
    private BufferedInputStream in;
    private BufferedOutputStream out;
    private volatile boolean connected = true;
    private int connectionId;
    private ConnectionsImpl<T> myConnections;
    private boolean started=false;
    
    public int getConnectionId(){
    	return connectionId;
    }
    
    public BlockingConnectionHandler(Socket sock, MessageEncoderDecoder<T> reader, BidiMessagingProtocol<T> protocol, ConnectionsImpl<T> connections, int connectionId_par) {
        this.sock = sock;
        this.encdec = reader;
        this.protocol = protocol;
        myConnections=connections;
        connectionId=connectionId_par;
    }

	@Override
    public void run() {
    	
    	// TODO: what if at this position, someone will connect, but stops here, And the in&out would be null.
    	// So someone is logged in, but is not reachable. (Exception - if he will be called)
        try (Socket sock = this.sock) { //just for automatic closing
            int read;

//            if (!started) {
//            	protocol.start(connectionId, myConnections);
//            	started=true;
//            }
            in = new BufferedInputStream(sock.getInputStream());
            out = new BufferedOutputStream(sock.getOutputStream());
            protocol.start(connectionId, myConnections);
            myConnections.add(connectionId, this);

            while (!protocol.shouldTerminate() && connected && (read = in.read()) >= 0) {
                T nextMessage = encdec.decodeNextByte((byte) read);            
      //          System.out.print((byte)read);
                if (nextMessage != null) {
                	protocol.process(nextMessage);
                }
//            T response = protocol.process(nextMessage);                  
//                    if (response != null) {
//                        out.write(encdec.encode(response));
//                        out.flush();
//                    }
            }

        } catch (IOException ex) {
            ex.printStackTrace();
        }   
    }


    static String popString(byte[] bytes) {
        //notice that we explicitly requesting that the string will be decoded from UTF-8
        //this is not actually required as it is the default encoding in java.
    	int len=bytes.length;
        String result = new String(bytes, 0, len, StandardCharsets.UTF_8);
        len = 0;
        return result;
    }
    
    
    @Override
    public void close() throws IOException {
        connected = false;
        sock.close();
    }
    // At the process calculate the response, and there make the connections.send .
    // The connection makes this send
	@Override
	public synchronized void send(T msg) {
		byte[] msgToSend = encdec.encode(msg);
		try {
			out.write(msgToSend);
			out.flush(); 
		} catch (IOException e) {
		}
		
	}
}
