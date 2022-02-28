`timescale 1ns/1ps
module stimulus();

   logic 	Start;
   logic 	reset;
   logic [9:0] 	Key;
   logic 	Found;   
   
   logic 	clk;   
   
   integer 	handle3;
   integer 	desc3;
   
   logic [31:0] vectornum;   

   top dut (clk, Start, reset, Key, Found);

   initial 
     begin	
	clk = 1'b0;
	forever #5 clk = ~clk;
     end

   initial
     begin
	vectornum = 0;
	handle3 = $fopen("count.out");
	desc3 = handle3;	
     end // initial begin

   always 
     begin
        desc3 = handle3;	
        #10 $fdisplay(desc3, "%b | %h %b | %d", 
		      Start, Key, Found, vectornum);
	if (Found == 1'b1)
	  vectornum = vectornum + 1;	
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
