`timescale 1ns/1ps
module tb;

   logic [9:0]  key;
   logic [7:0]  plaintext;
   logic        encrypt;
   logic [7:0]  ciphertext;

   logic        clk;
   logic [7:0] 	plaintext_decrypt;      
   logic [7:0]  correct_ciphertext;   
   logic [25:0] testvectors[50000:0]; 
   logic [25:0] vectornum;   
   
   integer 	 handle3;
   integer 	 desc3;
   integer 	 i;   

   SDES dut1 (key, plaintext, encrypt, ciphertext);
   SDES dut2 (key, ciphertext, 1'b0, plaintext_decrypt);   

   initial 
     begin	
	clk = 1'b0;
	forever #5 clk = ~clk;
     end

   initial
     begin
	vectornum = 0;
	handle3 = $fopen("sdes.out");
	$readmemb("vectors.in", testvectors);

	desc3 = handle3;	
     end // initial begin


   initial
     begin
	for (i=0; i < 1024; i=i+1)
	  begin
	     @(posedge clk);
	     begin
		encrypt = 1'b1;
		{key, plaintext, correct_ciphertext} = testvectors[vectornum];

		// Used for inidividualized testing
		// key = 10'h104;		
		// plaintext = 8'hc5;
		// correct_ciphertext = 8'h66;		
	     end
	     @(negedge clk)
	       begin
		  vectornum = vectornum + 1;
		  $fdisplay(desc3, "%b %b || %b | %b | %b | %b",
			    key, plaintext, ciphertext, correct_ciphertext, 
			    (ciphertext == correct_ciphertext), (plaintext == plaintext_decrypt));
	       end	     
	  end // for (i=0; i<2, i=i+1)

	$display("%d tests completed", vectornum);	
	$finish;			
     end // always @ (posedge clk, posedge reset)

endmodule // tb
