`timescale 1ns/1ps
module stimulus();

   logic 	Start;
   logic 	reset;
   logic [63:0] Key;
   logic [55:0] count;
   logic [63:0] plaintext, ciphertext;
   logic [63:0] ExpectedKey;
   
   localparam [55:0] countThreshold = 2**23;
   logic 	Found;   
   
   logic 	clk;   
   
   integer 	outputFilePointer;
   integer 	testVectorFilePointer;
   
   logic [31:0] vectornum;   

   top dut (clk, Start, reset, plaintext, ciphertext, count, Key, Found);

   initial 
     begin	
	clk = 1'b0;
	forever #5 clk = ~clk;
     end

   initial
     begin
	vectornum = 0;
	outputFilePointer = $fopen("count.out");
	testVectorFilePointer = $fopen("pkc.txt", "r");
	$display("initial read of test vector file.");
	if($fscanf(testVectorFilePointer, 
		   "%x %x %x", plaintext, ExpectedKey, ciphertext) != 3) 
	  begin
	     $display("Improperly formated test vector file.");
	  end
	
     end // initial begin

   always @(posedge clk) 
     begin
	if (Found == 1'b1) 
	  begin
	     $fdisplay(outputFilePointer, "%h %b | %h", Key, Found, count);
	     $display("Cracked Key = %x ExpectedKey = %x", Key, ExpectedKey);
	     #20;
	     reset = 1;
	     #20;
	     if($fscanf(testVectorFilePointer, 
			"%x %x %x", plaintext, ExpectedKey, ciphertext) == 3) 
	       begin
		  $display("got it. %x, %x, %x", 
			   plaintext, ExpectedKey, ciphertext);
	     end 
	     else 
	       begin
		  $display("End of test vector file.");
		  $stop();
	       end
	     reset = 0;
	     #20;
	     Start = 1;
	     #10;
	     Start = 0;
	     
	     //$stop();	   
	  end else if(count > countThreshold) begin
	     $display("Not able to find key.");
	     #20;
	     reset = 1;
	     #20;
	     if($fscanf(testVectorFilePointer, 
			"%x %x %x", plaintext, ExpectedKey, ciphertext) == 3) 
	       begin
		  $display("got it. %x, %x, %x", 
			   plaintext, ExpectedKey, ciphertext);
	       end 
	     else 
	       begin
		  $display("End of test vector file.");
		  $stop();
	       end
	     reset = 0;
	     #20;
	     Start = 1;
	     #10;
	     Start = 0;
	  end
     end
   
   initial
     begin
	// Initialize
	#0   Start = 1'b0;	
	#0   reset = 1'b1;
	#101 reset = 1'b0;
	#32  Start = 1'b1;
	#20  Start = 1'b0;	
     end 

endmodule // tb
