module lfsr #(parameter NUM_BITS = 5) 
   (input logic 		clk,
    input logic 		enable, 
    input logic 		seed_dv,
    input logic [NUM_BITS-1:0] 	Seed_Data, 
    output logic [NUM_BITS-1:0] LFSR_Data,
    output logic 		LFSR_Done);
   
   logic [NUM_BITS-1:0] 	r_LFSR = 0;
   logic 			r_XNOR;
   
   // Purpose: Load up LFSR with Seed if Data Valid (DV) pulse is detected.
   // Othewise just run LFSR when enabled.
   always @(posedge clk)
     begin
	if (enable == 1'b1)
          begin
             if (seed_dv == 1'b1)
               r_LFSR <= Seed_Data;
             else
               r_LFSR <= {r_LFSR[NUM_BITS-2:0], r_XNOR};
          end
     end
   
   // Create Feedback Polynomials.  Based on Application Note:
   // http://www.xilinx.com/support/documentation/application_notes/xapp052.pdf
   // Original publication doesn't use 0 as LSB for some reason
   always_comb
     begin
	case (NUM_BITS)
          3:  r_XNOR = r_LFSR[3-1] ^~ r_LFSR[2-1];
          4:  r_XNOR = r_LFSR[4-1] ^~ r_LFSR[3-1];
          5:  r_XNOR = r_LFSR[5-1] ^~ r_LFSR[3-1];
          6:  r_XNOR = r_LFSR[6-1] ^~ r_LFSR[5-1];
          7:  r_XNOR = r_LFSR[7-1] ^~ r_LFSR[6-1];
          8:  r_XNOR = r_LFSR[8-1] ^~ r_LFSR[6-1] ^~ r_LFSR[5-1] ^~ r_LFSR[4-1];
          9:  r_XNOR = r_LFSR[9-1] ^~ r_LFSR[5-1];
          10: r_XNOR = r_LFSR[10-1] ^~ r_LFSR[7-1];
          11: r_XNOR = r_LFSR[11-1] ^~ r_LFSR[9-1];
          12: r_XNOR = r_LFSR[12-1] ^~ r_LFSR[6-1] ^~ r_LFSR[4-1] ^~ r_LFSR[1-1];
          13: r_XNOR = r_LFSR[13-1] ^~ r_LFSR[4-1] ^~ r_LFSR[3-1] ^~ r_LFSR[1-1];
          14: r_XNOR = r_LFSR[14-1] ^~ r_LFSR[5-1] ^~ r_LFSR[3-1] ^~ r_LFSR[1-1];
          15: r_XNOR = r_LFSR[15-1] ^~ r_LFSR[14-1];
          16: r_XNOR = r_LFSR[16-1] ^~ r_LFSR[15-1] ^~ r_LFSR[13-1] ^~ r_LFSR[4-1];
          17: r_XNOR = r_LFSR[17-1] ^~ r_LFSR[14-1];
          18: r_XNOR = r_LFSR[18-1] ^~ r_LFSR[11-1];
          19: r_XNOR = r_LFSR[19-1] ^~ r_LFSR[6-1] ^~ r_LFSR[2-1] ^~ r_LFSR[1-1];
          20: r_XNOR = r_LFSR[20-1] ^~ r_LFSR[17-1];
          21: r_XNOR = r_LFSR[21-1] ^~ r_LFSR[19-1];
          22: r_XNOR = r_LFSR[22-1] ^~ r_LFSR[21-1];
          23: r_XNOR = r_LFSR[23-1] ^~ r_LFSR[18-1];
          24: r_XNOR = r_LFSR[24-1] ^~ r_LFSR[23-1] ^~ r_LFSR[22-1] ^~ r_LFSR[17-1];
          25: r_XNOR = r_LFSR[25-1] ^~ r_LFSR[22-1];
          26: r_XNOR = r_LFSR[26-1] ^~ r_LFSR[6-1] ^~ r_LFSR[2-1] ^~ r_LFSR[1-1];
          27: r_XNOR = r_LFSR[27-1] ^~ r_LFSR[5-1] ^~ r_LFSR[2-1] ^~ r_LFSR[1-1];
          28: r_XNOR = r_LFSR[28-1] ^~ r_LFSR[25-1];
          29: r_XNOR = r_LFSR[29-1] ^~ r_LFSR[27-1];
          30: r_XNOR = r_LFSR[30-1] ^~ r_LFSR[6-1] ^~ r_LFSR[4-1] ^~ r_LFSR[1-1];
          31: r_XNOR = r_LFSR[31-1] ^~ r_LFSR[28-1];
          32: r_XNOR = r_LFSR[32-1] ^~ r_LFSR[22-1] ^~ r_LFSR[2-1] ^~ r_LFSR[1-1];
	  default: r_XNOR = 0;	
	endcase // case (NUM_BITS)
     end // always @ (*)
   
   assign LFSR_Data = r_LFSR[NUM_BITS-1:0];
   
   // Conditional Assignment (?)
   assign LFSR_Done = (r_LFSR[NUM_BITS-1:0] == Seed_Data) ? 1'b1 : 1'b0;
   
endmodule // lfsr
