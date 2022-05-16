`timescale 1ns/1ps
module tb;

   logic 	      rst;
   logic 	      up;
   logic 	      down;
   logic 	      load;
   logic [9:0] 	      in;
   
   logic [9:0] 	      out;
   
   logic 	      clk;   
   
   integer 	 handle3;
   integer 	 desc3;
   
   logic [31:0] vectornum;   

   UDL_Count  #(10) dut (clk, rst, up, down, load, in, out);
   
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


   initial
     begin
	#0 rst = 1'b1;
	#0 in = 10'h0;	
	#0 load = 1'b0;
	#0 up = 1'b0;
	#0 down = 1'b0;	
	#112 rst = 1'b0;
	#41 up = 1'b1;
	#292 up = 1'b0;
     end 

endmodule // tb
