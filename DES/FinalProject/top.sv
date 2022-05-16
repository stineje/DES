module top (input logic 	clk, Start, reset,
	    input logic [63:0] 	plaintext, ciphertext, 
	    output logic [55:0] count,
	    output logic [63:0] Key, 
	    output logic 	FoundKeyNum);

   logic [63:0] 		DESkey_input;
   
   logic [63:0] 		ciphertext_gen;
   logic [63:0] 		ciphertext_store;
   
   logic 			down;
   logic 			load;
   logic 			Found;   
   
   logic 			UP;
   logic 			en1;
   logic 			en2;   
   logic [55:0] 		in;
   genvar 			index;
   
   assign down = 1'b0;
   assign load = 1'b0;
   assign in = 56'h0;

   control ctrl (~clk, reset, Found, Start, UP, en1, en2, FoundKeyNum);
   
   UDL_Count #(56) p0 (clk, reset, UP, down, load, in, count);
   genParity8 genParity8(.in(count), .out(DESkey_input));
   
   DES p1 (DESkey_input, plaintext, 1'b1, ciphertext_gen);
   flopenr #(64) p2a (clk, reset, en1, ciphertext_gen, ciphertext_store);
   comparator #(64) p3 (ciphertext_store, ciphertext, Found);
   flopenr #(64) p4 (clk, reset, en1, DESkey_input, Key);

endmodule // top

module genParity(input logic [6:0] in, output logic [7:0] out);
   
   assign out[0] = ~^in;
   assign out[7:1] = in;
   
endmodule // genParity

module genParity8(input logic [55:0] in, output logic [63:0] out);
   
   genvar 						index;
   for(index = 0; index < 8; index++) begin
      genParity genParity(.in(in[7*index +: 7]), .out(out[8*index +: 8]));
   end
   
endmodule // genParity8



