`timescale 1ns/1ps
module stimulus;

   logic 	clk;   
   logic 	enable;   
   logic 	seed_dv;   
   logic [8:0] 	Seed_Data;   
   logic [8:0] 	LFSR_Data;   
   logic 	LFSR_Done;

   logic 	regen;   
   
   lfsr #(9) dut (clk, enable, seed_dv, Seed_Data, LFSR_Data, LFSR_Done);

   // 1 ns clock
   initial 
     begin	
	clk = 1'b1;
	forever #5 clk = ~clk;
     end

   initial
     begin
	enable = 1'b1;
	seed_dv = 1'b0;	
	Seed_Data = 9'h0;
     end

endmodule // stimulus
