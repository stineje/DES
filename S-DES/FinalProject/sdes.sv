/*
 Simplified Data Encryption Standard (S-DES)
 
 8-bit 2-round block cipher encryption and decryption 
 algorithm using 10-bit key.
 
*/

module GenerateKeys (Key, Key1, Key2);
   
   // Generate Key1 and Key2
   input logic [9:0]  Key;
   output logic [7:0] Key1, Key2;
      
   logic [9:0] 	      p10_out, tempA, tempC;

   // P10 Permutations
   assign p10_out = {Key[7], Key[5], Key[8], Key[3], Key[6], 
		     Key[0], Key[9], Key[1], Key[2], Key[4]};
   // Left Rotation by 1 bit
   assign tempA = {p10_out[8:5], p10_out[9], p10_out[3:0], p10_out[4]};	 
   // P8 Permutations
   assign Key1  = {tempA[4], tempA[7], tempA[3], tempA[6], 
		   tempA[2], tempA[5], tempA[0], tempA[1]};
   // Left Rotation by 3 bits
   assign tempC = {p10_out[6:5], p10_out[9:7], p10_out[1:0], p10_out[4:2]};	 
   // P8 Permutations
   assign Key2 = {tempC[4], tempC[7], tempC[3], tempC[6], 
		  tempC[2], tempC[5], tempC[0], tempC[1]};

endmodule // GenerateKeys

module feistel (inp_block, key, out_block);

   input logic [7:0]  inp_block;
   input logic [7:0]  key;
   output logic [7:0] out_block;
   
   logic [3:0] 	      first_chunk, second_chunk;
   logic [3:0] 	      xor_fout, xor_f1, xor_f2;
   logic [3:0] 	      p4_in, p4_out;
   logic [7:0] 	      EP_out, xor_out;
   logic [1:0] 	      s0_out, s1_out;

   assign first_chunk  = inp_block[7:4];
   assign second_chunk = inp_block[3:0];	 
   assign EP_out = {second_chunk[0], second_chunk[3], second_chunk[2], second_chunk[1], 
		    second_chunk[2], second_chunk[1], second_chunk[0], second_chunk[3]};
   assign xor_out = EP_out ^ key;	 
   assign xor_f1 = xor_out[7:4];
   assign xor_f2 = xor_out[3:0];
   
   S0_Box box1 (xor_f1, s0_out);
   S1_Box box2 (xor_f2, s1_out);
	 
   assign p4_in = {s0_out, s1_out};
   assign p4_out = {p4_in[2], p4_in[0], p4_in[1], p4_in[3]};	 
   assign xor_fout = p4_out ^ first_chunk;	 
   assign out_block = {xor_fout, second_chunk};

endmodule // Feistel

module S0_Box (inp_bits, out_bits);

   input logic [3:0] inp_bits;
   output logic [1:0] out_bits;

   /// a_3, a_2, a_1, a_0 -> (a_3, a_0) (a_2 a_0) : (row, column)
   always_comb
     begin
	case(inp_bits)
	  4'b0000: out_bits = 2'b01; // (0,0)
	  4'b0001: out_bits = 2'b11; // (1,0)
	  4'b0010: out_bits = 2'b00; // (0,1)
	  4'b0011: out_bits = 2'b10; // (1,1)	  
	  4'b0100: out_bits = 2'b11; // (0,2)
	  4'b0101: out_bits = 2'b01; // (1,2) 
	  4'b0110: out_bits = 2'b10; // (0,3)
	  4'b0111: out_bits = 2'b00; // (1,3)	  
	  4'b1000: out_bits = 2'b00; // (2,0)
	  4'b1001: out_bits = 2'b11; // (3,0)
	  4'b1010: out_bits = 2'b10; // (2,1)
	  4'b1011: out_bits = 2'b01; // (3,1)	  
	  4'b1100: out_bits = 2'b01; // (2,2)
	  4'b1101: out_bits = 2'b11; // (3,2)
	  4'b1110: out_bits = 2'b11; // (2,3)
	  4'b1111: out_bits = 2'b10; // (3,3)
	endcase
     end // always_comb
   
endmodule // S0_Box

module S1_Box (inp_bits, out_bits);

   input logic [3:0] inp_bits;
   output logic [1:0] out_bits;

   /// a_3, a_2, a_1, a_0 -> (a_3, a_0) (a_2 a_0) : (row, column)   
   always_comb
     begin
	case(inp_bits)
	  4'b0000: out_bits = 2'b00; // (0,0)
	  4'b0001: out_bits = 2'b10; // (1,0)
	  4'b0010: out_bits = 2'b01; // (0,1)
	  4'b0011: out_bits = 2'b00; // (1,1)	  
	  4'b0100: out_bits = 2'b10; // (0,2)
	  4'b0101: out_bits = 2'b01; // (1,2)
	  4'b0110: out_bits = 2'b11; // (0,3)
	  4'b0111: out_bits = 2'b11; // (1,3)	  
	  4'b1000: out_bits = 2'b11; // (2,0)
	  4'b1001: out_bits = 2'b10; // (3,0)
	  4'b1010: out_bits = 2'b00; // (2,1)
	  4'b1011: out_bits = 2'b01; // (3,1)	  
	  4'b1100: out_bits = 2'b01; // (2,2)
	  4'b1101: out_bits = 2'b00; // (3,2)
	  4'b1110: out_bits = 2'b00; // (2,3)
	  4'b1111: out_bits = 2'b11; // (3,3)
	endcase
     end // always_comb

endmodule // S1_Box

module SDES (input logic [9:0] key, input logic [7:0] plaintext, 
	     input logic encrypt, output logic [7:0] ciphertext);

   logic [7:0] K1;
   logic [7:0] K2;
   logic [7:0] IP_out;
   logic [7:0] sw_out;      
	     
   logic [7:0] feistel_K1_out;
   logic [7:0] feistel_K2_out;
   	     
   logic [7:0] K1_out;
   logic [7:0] K2_out;

   // Generate Key1 and Key2
   GenerateKeys keys (key, K1, K2);	
   // Initial Permutation (IP)
   assign IP_out = {plaintext[6], plaintext[2], plaintext[5], plaintext[7], 
		    plaintext[4], plaintext[0], plaintext[3], plaintext[1]};

   // Round 1
   assign K1_out = encrypt ? K1 : K2;
   feistel f1 (IP_out, K1_out, feistel_K1_out);
   
   // Swapping
   assign sw_out = {feistel_K1_out[3:0], feistel_K1_out[7:4]};

   // Round 2
   assign K2_out = encrypt ? K2 : K1;   
   feistel f2 (sw_out, K2_out, feistel_K2_out);
	
   // Inverse Initial Permutation (IP^{-1})
   assign ciphertext = {feistel_K2_out[4], feistel_K2_out[7], 
			feistel_K2_out[5], feistel_K2_out[3], 
			feistel_K2_out[1], feistel_K2_out[6], 
			feistel_K2_out[0], feistel_K2_out[2]};
   
endmodule // SDES
