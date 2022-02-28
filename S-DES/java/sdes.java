import java.awt.datatransfer.StringSelection;
import java.io.*;
import java.util.*;

public class sdes {

    public int sbox0, sbox1, f1, f2, f3, f4, K1, K2;
    // For performing P10 permutation
    public static final int P10[] = {3, 5, 2, 7, 4, 10, 1, 9, 8, 6};  
    public static final int P10max = 10;
    // 10 to 8 bit permutation
    public static final int P8[] = {6, 3, 7, 4, 8, 5, 10, 9};
    public static final int P8max = 10;
    public static final int P4[] = {2, 4, 3, 1};
    public static final int P4max = 4;
    // Performing initial permutaion
    public static final int IP[] = {2, 6, 3, 1, 4, 8, 5, 7};
    public static final int IPmax = 8;
    // Performing final permutaion (Inverse of initial permutation)
    public static final int IPI[] = {4, 1, 3, 5, 7, 2, 8, 6};
    public static final int IPImax = 8;
    // Expansion/permutation operation : Used in F1 Function
    public static final int EP[] = {4, 1, 2, 3, 2, 3, 4, 1};
    public static final int EPmax = 4;
    // sbox
    public static final int S0[][] = {{1, 0, 3, 2},
				      {3, 2, 1, 0},
				      {0, 2, 1, 3},
				      {3, 1, 3, 2}};
    public static final int S1[][] = {{0, 1, 2, 3},
				      {2, 0, 1, 3},
				      {3, 0, 1, 0},
				      {2, 1, 0, 3}};
    /*
     * Generic Function to perform all permutations
     */
    public static int permute(int x, int p[], int pmax) {
    	int y=0;
    	for(int i=0; i < p.length; i++) {
	    y=y<<1;
	    y=y|(x>>(pmax-p[i]))&1;
    	}
    	return y;
    }

    // Feistel
    public int F (int R, int K) {
	int t = permute (R, EP, EPmax) ^ K;
	int t0 = (t >> 4) & 0xF;
	int t1 = t & 0xF;
	// sbox lookup
	sbox0 = S0[((t0 & 0x8) >> 2) | (t0 & 1) ][ (t0 >> 1) & 0x3];
	sbox1 = S1[((t1 & 0x8) >> 2) | (t1 & 1) ][ (t1 >> 1) & 0x3];
        t = permute ((sbox0 << 2) | sbox1, P4, P4max);
	return t;    
    }

    public int fK (int m, int K) {
	int L = (m >> 4) & 0xF;
	int R = m & 0xF;
	return ((L ^ F(R,K)) << 4) | R;
    }

    public static int SW (int x) {
	return ((x & 0xF) << 4) | ((x >> 4) & 0xF);
    }
    
    /*
     * Ciphertext = IP-1 (fK2 (SW (fk1 (IP (plaintext))))
     */
    public byte encrypt (int m) {
	
	m = permute (m, IP, IPmax);
	m = fK (m, K1);
	// print sbox0/sbox1 (4 bits)
	sdes.printData (sbox0, 4);
	System.out.print (" ");	    		
	sdes.printData (sbox1, 4);
	System.out.print (" ");	  		
	m = SW (m);
	m = fK (m, K2);
	// print sbox0/sbox1 (4 bits)	
	sdes.printData (sbox0, 4);
	System.out.print (" ");	    		
	sdes.printData (sbox1, 4);
	System.out.print (" ");	  			
	m = permute (m, IPI, IPImax);
	return (byte) m;
    
    }
    
    /*
     * Plaintext = IP-1 (fK1 (SW (fk2 (IP (ciphertext)))))
     */
    public byte decrypt (int m) {
	
	m = permute (m, IP, IPmax);
	m = fK (m, K2);
	m = SW (m);
	m = fK (m, K1);
	m = permute (m, IPI, IPImax);
	return (byte) m;
    
    }
    
    public static void outData (int x, int n, PrintStream out) {
	int mask = 1 << (n-1);
        while (mask > 0) {
	    out.print (((x & mask) == 0) ? '0' : '1');	    
	    mask >>= 1;
	}
    }

    public static void printData (int x, int n) {
	int mask = 1 << (n-1);
        while (mask > 0) {
	    System.out.print (((x & mask) == 0) ? '0' : '1');
	    mask >>= 1;
	}
    }    
    
    // K1 = P8 (Shift_1 (P10 (Key)))
    // K2 = P8 (Shift_2 (shift_1 (P10 (Key))))
    public sdes (int K) {
	K = permute (K, P10, P10max);
	f1 = (K >> 5) & 0x1F;
	f2 = K & 0x1F;
	f1 = ((f1 & 0xF) << 1) | ((f1 & 0x10) >> 4);
	f2 = ((f2 & 0xF) << 1) | ((f2 & 0x10) >> 4);
	K1 = permute ((f1 << 5)| f2, P8, P8max);
	f3 = ((f1 & 0x7) << 2) | ((f1 & 0x18) >> 3);
	f4 = ((f2 & 0x7) << 2) | ((f2 & 0x18) >> 3);
	K2 = permute ((f3 << 5)| f4, P8, P8max);    
    }

    @SuppressWarnings("deprecation")
    public static void main (String args[]) throws Exception {

	System.out.print ("Oklahoma State University\n");
	System.out.print ("ECEN 3233, Fall 2021\n");
	System.out.print ("Digital Logic Design\n");
	System.out.print ("Simplified Digital Encryption Standard (DES)\n");
	System.out.print ("\n");

	// Open file
	PrintStream out = new PrintStream (new File("vectors.in"));
	// loop
	int i;
	// Key	    		
	int K;
	// Message (Plaintext)	    	
	int m;
	// generates random numbers in the range 0 to upperbound-1
	Random rand = new Random();
	System.out.print("(Key) (Plaintext) (K1) (K2) (sbox0_K0) (sbox1_K0) (sbox0_K1) (sbox1_K1) (Ciphertext)\n");

	for (i=0; i < 1024; i++) {
	    K = rand.nextInt (1024);
	    K = 0x116;
	    sdes A = new sdes (K);	    
	    m = rand.nextInt (256);
	    m = 0xfc;
	    
	    sdes.outData (K, 10, out);
	    out.print ("_");
	    sdes.outData (m, 8, out);
	    out.print ("_");

	    // (for debugging)

	    sdes.printData (K, 10);
	    System.out.print (" ");
	    sdes.printData (m, 8);
	    System.out.print (" ");

	    // Key K1
	    sdes.printData (A.K1, 8);
	    System.out.print(" ");			
	    // Key K2
	    sdes.printData (A.K2, 8);
	    System.out.print (" ");

	    // Encrypt
	    m = A.encrypt (m);
	    sdes.outData (m, 8, out);
	    sdes.printData (m, 8);
	    out.print (" ");
	    // Decrypt	    
	    m = A.decrypt (m);
	    out.print ("\n");
	    System.out.print ("\n");	    	    
	}
	out.close();
    }
}
