module control (clk, reset, Found, Start, UP, en1, en2, FoundKeyNum);

   input logic  clk;
   input logic  reset;
   input logic 	Start;
   input logic 	Found;   
   
   output logic en1;
   output logic en2;
   output logic UP;
   output logic FoundKeyNum;   

   typedef enum logic [1:0] {Idle, Up, Store, FoundKey} statetype;
   statetype state, nextstate;
   
   // state register
   always_ff @(posedge clk, posedge reset)
     if (reset) state <= Idle;
     else       state <= nextstate;
   
   // next state logic
   always_comb
     case (state)
       Idle: begin
	  UP = 1'b0;	  
	  en1 = 1'b0;
	  en2 = 1'b0;
	  FoundKeyNum = 1'b0;
	  if (Start == 1'b1)
	    nextstate = Store;
	  else 
	    nextstate = Idle;
       end // case: S0
       Up: begin
	  UP = 1'b1;	  
	  en1 = 1'b0;
	  en2 = 1'b0;	  
	  FoundKeyNum = 1'b0;	  
	  nextstate = Store;
       end // case: S1
       Store: begin
	  UP = 1'b0;	  
	  en1 = 1'b1;
	  en2 = 1'b0;
	  FoundKeyNum = 1'b0;	  
	  if (Found == 1'b1)
	    nextstate = FoundKey;
	  else 
	    nextstate = Up;	  
       end  
       FoundKey: begin
	  UP = 1'b0;	  
	  en1 = 1'b0;
	  en2 = 1'b1;
	  FoundKeyNum = 1'b1;	  
	  if (Start == 1'b1)
	    nextstate = FoundKey;
	  else 
	    nextstate = Idle;
       end
       default: begin
	  UP = 1'b0;	  
	  en1 = 1'b0;
	  en2 = 1'b0;	  
	  nextstate = Idle;
	  FoundKeyNum = 1'b0;	  
       end
     endcase
   
endmodule
