`timescale 1ns/1ps
module UDL_Count  #(parameter WIDTH=8) 
   (clk, rst, up, down, load, in, out) ;

   input logic              clk;
   input logic 		    rst;
   input logic 		    up;
   input logic 		    down;
   input logic 		    load;
   input logic [WIDTH-1:0]  in;
   
   output logic [WIDTH-1:0] out;

   logic [WIDTH-1:0] 	    next;
   
   flop #(WIDTH) count(clk, next, out);

   always_comb begin
      if (rst)
	next = {WIDTH{1'b0}};
      else if (load)
	next = in;
      else if (up)
	next = out + 1'b1;
      else if (down)
	next = out - 1'b1;
      else
	next = out;
   end // always@ *
   
endmodule 

// ordinary flip-flop
module flop #(parameter WIDTH=8) ( 
  input  logic             clk,
  input  logic [WIDTH-1:0] d, 
  output logic [WIDTH-1:0] q);

  always_ff @(posedge clk)
    q <= d;
   
endmodule

