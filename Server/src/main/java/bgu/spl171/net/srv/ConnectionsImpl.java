package bgu.spl171.net.srv;

import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map.Entry;
import bgu.spl171.net.api.bidi.Connections;
import bgu.spl171.net.srv.bidi.ConnectionHandler;

public class ConnectionsImpl<T> implements Connections<T>{
private HashMap<Integer, ConnectionHandler<T>> connectionsQueue = new HashMap<Integer, ConnectionHandler<T>>();
	
	
//
//	/** Constructor
//     * Initialize the ConnectionsImpl
//     * @param connectionsQueue - HashMap of connectionsQueue
//     */
//	public ConnectionsImpl(HashMap<Integer, ConnectionHandler<T>> connectionsQueue) {
//		this.connectionsQueue = connectionsQueue;
//	}
	
	public void add(int connectionID, ConnectionHandler<T> connection){
		boolean contains=connectionsQueue.containsValue(connection);
		if (!contains) connectionsQueue.put(connectionID, connection);
//		else if (contains) throw new RuntimeException ("client already connected");
	}

	/**
     * Sends a message to a given client (The connections are stored as field)
     *
     * @param connectionId - the destination to send to
     * @param msg - the message to send 
     * @return true if the message was sent
     */
	@SuppressWarnings("unchecked")
	@Override
	public boolean send(int connectionId, Object msg) { 		
		try{
			connectionsQueue.get(connectionId).send((T) msg);
			return true;
		}
		catch (Exception e){

		}
		return false;
	}

    static String popString(byte[] bytes) {
        //notice that we explicitly requesting that the string will be decoded from UTF-8
        //this is not actually required as it is the default encoding in java.
    	int len=bytes.length;
        String result = new String(bytes, 0, len, StandardCharsets.UTF_8);
        len = 0;
        return result;
    }
    
    

	/**
     * Sends a message to all of the connections
     * @param msg - the messege to broadcast
     */
	@SuppressWarnings("unchecked")
	@Override
	public void broadcast(Object msg) {
		for(Entry<Integer, ConnectionHandler<T>> entry : connectionsQueue.entrySet()) {
			((ConnectionHandler<T>) entry).send((T) msg);
		}
	}
	

	/**
     * Find the connection by the connectionId given, close it, and then remove it from the queue.
     * @param connectionId - the Id to find&remove
     */
	@Override
	public void disconnect(int connectionId){
		ConnectionHandler<T> c = connectionsQueue.get(connectionId);
		if (c!=null)
			try {
				c.close();
			} catch (IOException e) {
				// TODO Auto-generated catch block
			 System.out.println("Can't close it");
			}
		connectionsQueue.remove(connectionId);
	}
	
}
