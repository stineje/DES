import javax.crypto.*;
import java.security.*;
import java.util.*;

public class genDESkey {

    public static void main (String[] args) throws NoSuchAlgorithmException,
						   NoSuchPaddingException,
						   BadPaddingException,
						   IllegalBlockSizeException,
						   InvalidKeyException {

	//Creating a KeyGenerator object
	KeyGenerator keyGen = KeyGenerator.getInstance("DES");	
	//Creating a SecureRandom object
	SecureRandom secRandom = new SecureRandom();	
	//Initializing the KeyGenerator
	keyGen.init(secRandom);
	//Creating/Generating a key
	Key key;
	int i;
	// Reading from System.in	
	Scanner reader = new Scanner(System.in);  
	System.out.print("Enter the number of keys you wish to generate: ");
	int n = reader.nextInt();
	for (i = 0; i < n; i++) {
	    key = keyGen.generateKey();
	    System.out.println("key = " + Utils.toHex(key.getEncoded()));
	}
	reader.close();
	
    }
}

class Utils {
    
    private static String digits = "0123456789abcdef";
    
    /**
     * Return length many bytes of the passed in byte array as a hex string.
     * 
     * @param data the bytes to be converted.
     * @param length the number of bytes in the data block to be converted.
     * @return a hex representation of length bytes of data.
     */
    public static String toHex(byte[] data, int length)  {
        StringBuffer  buf = new StringBuffer();
        
        for (int i = 0; i != length; i++) {
            int v = data[i] & 0xff;            
            buf.append(digits.charAt(v >> 4));
            buf.append(digits.charAt(v & 0xf));
        }        
        return buf.toString();
    }
    
    /**
     * Return the passed in byte array as a hex string.
     * 
     * @param data the bytes to be converted.
     * @return a hex representation of data.
     */
    public static String toHex(byte[] data) {
        return toHex(data, data.length);
    }
}

