import java.awt.datatransfer.StringSelection;
import java.io.*;
import java.util.*;

public class sdes_project {

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
	m = SW (m);
	m = fK (m, K2);
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
    public sdes_project (int K) {
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

	// loop
	int i;
	// Key
	int [] k;
	int K;
	// Message (Plaintext)
	int [] M;
	int m;
	int c;

	// generates random numbers in the range 0 to upperbound-1
	Random rand = new Random();	

	k = new int[32];
	M = new int[32];	
	k[0]  = 0x14c;
	k[1]  = 0x20c;
	k[2]  = 0x355;
	k[3]  = 0x30e;
	k[4]  = 0x019;
	k[5]  = 0x25c;
	k[6]  = 0x0a7;
	k[7]  = 0x2e0;
	k[8]  = 0x3a1;
		     
	k[9]  = 0x075;
	k[10] = 0x137;
	k[11] = 0x2f5;
	k[12] = 0x24d;
	k[13] = 0x328;
	k[14] = 0x3c8;
	k[15] = 0x086;
	k[16] = 0x1fe;
	k[17] = 0x3fd;
	k[18] = 0x100;	
	k[19] = 0x364;
		     
	k[20] = 0x2de;
	k[21] = 0x12c;
	k[22] = 0x0fa;
	k[23] = 0x08d;
	k[24] = 0x300;
	k[25] = 0x374;
	k[26] = 0x35d;
	k[27] = 0x221;
	k[28] = 0x132;
	k[29] = 0x27e;
	k[30] = 0x050;
	k[31] = 0x116;

	M[0]  = 0xc5;
	M[1]  = 0xfa;
	M[2]  = 0x51;
	M[3]  = 0x4c;
	M[4]  = 0x49;
	M[5]  = 0x93;
	M[6]  = 0xb0;
	M[7]  = 0x89;
	M[8]  = 0x85;
		    
	M[9]  = 0x45;
	M[10] = 0x01;
	M[11] = 0x27;
	M[12] = 0xbb;
	M[13] = 0x97;
	M[14] = 0x30;
	M[15] = 0x2c;
	M[16] = 0x58;
	M[17] = 0x71;
	M[18] = 0x0f;	
	M[19] = 0xdb;
		    
	M[20] = 0x1a;
	M[21] = 0x6d;
	M[22] = 0x44;
	M[23] = 0xeb;
	M[24] = 0x05;
	M[25] = 0xec;
	M[26] = 0x2e;
	M[27] = 0x41;
	M[28] = 0x76;
	M[29] = 0xc6;
	M[30] = 0x3f;
	M[31] = 0xfc;

	System.out.print("(key) (Plaintext) (Ciphertext)\n");
	
	for (i=0; i < 32; i++) {
	    m = M[i];
	    K = k[i];
	    sdes_project A = new sdes_project (K);	    	    
	    System.out.print(String.format("0x%3x", K));
   
	    System.out.print ("_");	    
	    System.out.print(Integer.toHexString(m & 0xFF));
	    System.out.print ("_");	    
	    c = A.encrypt (m);
	    System.out.print(Integer.toHexString(c & 0xFF));
	    System.out.print ("\n");	    
	}




    }
}
