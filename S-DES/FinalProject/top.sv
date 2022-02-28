module top (input logic clk, Start, reset, 
	    output logic [9:0] Key, output logic FoundKeyNum);

   logic [9:0] key;
   
   logic [7:0] ciphertext_gen;
   logic [7:0] ciphertext_store;
   logic [7:0] ciphertext_known;

   logic [7:0] plaintext_gen;
   logic [7:0] plaintext_store;
   logic [7:0] plaintext_known;
   
   logic       down;
   logic       load;
   logic       Found;   

   logic       UP;
   logic       en1;
   logic       en2;   
   logic [9:0] in;   

   assign plaintext_known = 8'h90;
   assign ciphertext_known = 8'h44;
   assign down = 1'b0;
   assign load = 1'b0;
   assign in = 10'h0;   

   control ctrl (~clk, reset, Found, Start, UP, en1, en2, FoundKeyNum);
   
   UDL_Count #(10) p0 (clk, reset, UP, down, load, in, key) ;
   SDES p1 (key, plaintext_known, 1'b1, ciphertext_gen);
   flopenr #(8) p2a (clk, reset, en1, ciphertext_gen, ciphertext_store);
   comparator #(8) p3 (ciphertext_store, ciphertext_known, Found);
   flopenr #(10) p4 (clk, reset, en2, key, Key);

endmodule // top

   
