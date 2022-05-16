// BattleShip Game
// Game design for ECE 476 Final Project
// By: Tomasz Micula and Gilbert Cheng

//video gen and sound
//D.5 is sync:1000 ohm + diode to 75 ohm resistor
//D.6 is video:330 ohm + diode to 75 ohm resistor  
//C.0 is sound

#pragma regalloc-    //I allocate the registers myself
#pragma optsize-     //optimize for speed
                    
#include <Mega32.h>   
#include <stdio.h>
#include <stdlib.h> 
#include <math.h> 
#include <delay.h>   

//cycles = 63.625 * 16 Note NTSC is 63.55 
//but this line duration makes each frame exactly 1/60 sec
//which is nice for keeping a realtime clock 
#define lineTime 1018
 
#define begin {
#define end   }
#define ScreenTop 30
#define ScreenBot 230
#define T0reload 256-60

// define max number of keys on the key pad      
#define maxkeys 16    

// predefine states for state machine that reads 
// inputs from the keypad
#define Pushed 0
#define Debounce 1
#define Detect 2
#define DebouncedEnd 3

// predefine states for state machine that controls
// the battleship game
#define Start 0        // Starts game with our Names and Game title displayed 
#define SelectLevel 1  // Allows user to select game level
#define GridSetup 2    // Allow user to allocate its ships
#define PlayGame 3     // This controls game play between Player A and Computer                                    
#define EndGame 4      // If someone looses all ships display end game and ask for replay                                                         
#define CleanState 5   // Clean Screen when necessary

//NOTE that v1 to v8 and i must be in registers!  
register char v1 @4; 
register char v2 @5;
register char v3 @6;
register char v4 @7;
register char v5 @8;
register char v6 @9;
register char v7 @10;
register char v8 @11; 
register int i @12;

#pragma regalloc+ 

char syncON, syncOFF; 
int LineCount;              
char screen[1600]; 

// Grid representation :
// each square in the game grid holds two values
// it tells if there is any ship at all and the type of ship
// for example: if square 1,1 contains carrier ship then
// it is represented in the following way:
// B7 B6 B5 B4 B3 B2 B1 B0
// 0  1  0  1  0  0  0  1   
// low order bits are used to detect presence of ship and its status
// 4 bits are divided into two groups, lower 2 bits tell if 
// ship is present and whether it was hit or missed 
// the next upper two bits tell the status of the grid box (if sunk)
// B3 B2 B1 B0    description
// 0  0  0  0     no ship present
// 0  0  0  1     ship present and not yet hit not sunk with horizontal orient
// 1  0  0  1     ship present and not yet hit not sunk with vertical orient
// 0  0  1  0     ship not present, hit attempted and missed
// 0  0  1  1     ship present, hit but not yet sunk (horizontal orientation)
// 1  0  1  1     ship present, hit but not yet sunk (vertical orientation)
// 0  1  1  1     ship present/hit and sunk (horizontal orientation)
// 1  1  1  1     ship present/hit and sunk (vertical orientation)
// holds all the information for the Player A grid
char playerAGrid[10][10];                         
// holds all the information for Computer grid
char computerGrid[10][10];
// Ships in the game are represented as follows :
// Carrier : 1
// Destroyer : 2
// Aegis Cruiser : 3
// Submarine : 4
// Patrol Boat : 5
char shiplength[5] = {5, 4, 4, 3, 2};
// remember beginning and ending position of each of the 5 Player A ships
char playerAships[5]; 
// keeps track of the number of hits needed to sink a particular Player A ship
char lefttosinkPlayerA[5] = {5, 4, 4, 3, 2};
// remember beginning and ending position of each of the 5 Computer ships
char computerships[5];                      
// keeps track of the number of hits needed to sink a particular Computer ship
char lefttosinkComputer[5] = {5, 4, 4, 3, 2};                             
// keep track of Computer hits on Player A Grid
char computerDirectHits;                       
// keep track of Player A hits of Computer Grid
char playerADirectHits;

// Keypad representation
// 0xee = 1, 0xed = 2, 0xeb = 3, 0xe7 = A, 
// 0xde = 4, 0xdd = 5, 0xdb = 6, 0xd7 = B, 
// 0xbe = 7, 0xbd = 8, 0xbb = 9, 0xb7 = C, 
// 0x7e = *, 0x7d = 0, 0x7b = #, 0x77 = D      

// key pad scan table : we reordered it from the orginal 
// so that we could easily index into the table 
flash unsigned char keytbl[16]={0xd7, 0xee, 0xde, 0xbe, 0xed, 
                                0xdd, 0xbd, 0xeb, 0xdb, 0xbb, 
                                0x7e, 0x7d, 0x7b, 0x77, 0xe7,0xb7};
unsigned char key, butnum, lastbutnum;     // vars for reading from key pad                                  

// Vars to display logos
char playerALogo[]  = "PLAYERAJ"; 
char computerLogo[] = "COMPUTER"; 

// Used in the displaying of grid
char c[] = "C";     // carrier
char p[] = "P";     // patrol boat
char d[] = "D";     // destoryer
char s[] = "S";     // submarine
char k[] = "K";     // sunk
char x[] = "X";     // missed
char h[] = "H";     // hit
char a[] = "A";     // aegies cruiser
char blank[] = "J"; // blank
            
// these variables are used to display each ships 
// section seperately. eg for Patrol Boat we would use only 
// displayShipsSection 0 and 1 since its length is 2 etc.
char displayShipsSection0[] = "0";
char displayShipsSection1[] = "1";
char displayShipsSection2[] = "2";
char displayShipsSection3[] = "3";
char displayShipsSection4[] = "4";

// Counters for each ship to keep track what section of the
// current ship has been displayed
char countPatrolBoat;
char countSubmarine;
char countAegisCruiser;
char countDestroyer;
char countCarrier;

/* Vars used in the Key Pad state machine */
char keyPadState;           

// game State machine variable to control game play
char gameState;                                    
// these variables enable us to draw computer and Player A grids and
// location of thier ships
char row, col;            
// keeps track of user specified location of Player A ships
char shipAllocRow, shipAllocCol;
// holds whose turn is now: 0 => Computer Turn and 1 => Player Turn
char playerTurn;                                           
// hold information about shooting place
char shootAtRow;
// if we are trying to get row number than isRowSet is 1 otherwise we
// want to set column with isRowSet = 0
char isRowSet;                          
// holds information about the game level being played
char gameLevel;                                       
// notifies game State machine if the input is ready from the KeyPad
char inputReady;                                                    
// this allows us to display ships images and words on screen without jitter
char timer; 
// allows to display ships and clear screen without any jitter
char displaytimer;

/* This section contains vars for medium algorithm */
// We added these variables
char isShipSunk;       
// stores first successful hit
char firstHitShipPos;  
// stores last successful hit  
char lastHitShipPos; 
// represents direction in which we will keep shooting
char direction;        
// store last 10 possible shooting positions 
char StackOfMoves[10]; 
// holds current ship type to be allocated in the gird by Player A
char typeofship;
// notify game State machine if we finished printing grid
char doneprintgrid;                                      
// notify game State machine if player A entered coordinate to shoot
char doneenteringshippos;

//Point plot lookup table
flash char pos[8]={0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};

// counter used to count down the songs
int songcount = 1000;
			

//define bitmap for Carrier Battleship
flash char extCarrierbitmap[5][7] = {
	//0
	0b10000000,
	0b10000000,
	0b10000000,
	0b10000000,
	0b10000000,
	0b10000000,
	0b10000000,
	//1
	0b10000000,
	0b10000000,
	0b10000000,
	0b10000000,
	0b10000000,
	0b10000000,
	0b10000000,
	//2
	0b10000000,
	0b10000000,
	0b10000000,
	0b10000000,
	0b10000000,
	0b10000000,
	0b10000000,
	//3
	0b00000000,
	0b10000000,
	0b00000000,
	0b10000000,
	0b00000000,
	0b10000000,
	0b00000000,
	//4
	0b10000000,
	0b10000000,
	0b10000000,
	0b10000000,
	0b10000000,
	0b10000000,
	0b10000000
};
                                
flash char Carrierbitmap[5][7]={ 
	//0
         0b00000001,
	0b11111111,
	0b10101010,
	0b11111111,
	0b10101010,
	0b11111111,
	0b00000001,
	//1
	0b11111111,
	0b11011111,
	0b11100111,
	0b11000001,
	0b11100111,
	0b11011111,
	0b11111111,
	//2
	0b11111111,
	0b11011111,
	0b11100111,
	0b11000001,
	0b11100111,
	0b11011111,
	0b11111111,
	//3
	0b11110000,
	0b11011111,
	0b11100111,
	0b11000001,
	0b11100111,
	0b11011111,
	0b11110000,
	//4
	0b00000000,
	0b11111111,
	0b01010101,
	0b11111111,
	0b01010101,
	0b11111111,
	0b00000000	
};

//define bitmap for Aegis Cruiser Battleship
flash char extAegisCruiserbitmap[4][7] = {
	//0
	0b10000000,
	0b10000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b10000000,
	0b10000000,
	//1
	0b10000000,
	0b10000000,
	0b10000000,
	0b10000000,
	0b10000000,
	0b10000000,
	0b10000000,
	//2
    0b00000000,
	0b10000000,
	0b10000000,
	0b10000000,
	0b10000000,
	0b10000000,
	0b00000000,
	//3
	0b10000000,
	0b10000000,
	0b10000000,
	0b10000000,
	0b10000000,
	0b10000000,
	0b10000000
};

flash char AegisCruiserbitmap[4][7]={ 
	//0
	0b00000111,
	0b00011111,
	0b01110000,
	0b11111101,
	0b01110000,
	0b00011111,
	0b00000111,
	//1
	0b11101101,
	0b11101101,
	0b11100001,
	0b11101101,
	0b11100001,
	0b11101101,
	0b11101101,
	//2
	0b11011010,
	0b11011011,
	0b11000011,
	0b11011011,
	0b11000011,
	0b11011011,
	0b11011010,
	//3
	0b00000000,
	0b11111110,
	0b10000011,
	0b10101111,
	0b10000011,
	0b11111110,
	0b00000000
};

//define bitmap for Destroyer Battleship
flash char extDestroyerbitmap[4][7] = {
   	//0
	0b00000000,
	0b10000000,
	0b00000000,
	0b10000000,
	0b00000000,
	0b10000000,
	0b00000000,
         //1
	0b00000000,
	0b10000000,
	0b10000000,
	0b10000000,
	0b10000000,
	0b10000000,
	0b00000000,
	//2
    0b00000000,
	0b10000000,
	0b10000000,
	0b10000000,
	0b10000000,
	0b10000000,
	0b00000000,
	//3
    0b10000000,
	0b10000000,
	0b10000000,
	0b10000000,
	0b10000000,
	0b10000000,
	0b10000000
};

flash char Destroyerbitmap[4][7]={ 
	//0
	0b00000000,
	0b00001111,
	0b01111000,
	0b11111110,
	0b01111000,
	0b00001111,
	0b00000000,
         //1
	0b11101011,
	0b11101011,
	0b11100011,
	0b01101011,
	0b11110111,
	0b11111111,
	0b11111111,
	//2
	0b00011110,
	0b11111111,
	0b10000011,
	0b10101111,
	0b10000011,
	0b11111111,
	0b00011110,
	//3
	0b00000000,
	0b11111100,
	0b00000111,
	0b01011111,
	0b00000111,
	0b11111100,
	0b00000000
};

//define bitmap for Submarine Battleship
flash char extSubmarinebitmap[3][7] = {
    	//0
	0b10000000,
	0b10000000,
	0b10000000,
	0b10000000,
	0b10000000,
	0b10000000,
	0b10000000,
	//1
	0b00000000,
	0b10000000,
	0b10000000,
	0b10000000,
	0b10000000,
	0b10000000,
	0b00000000,
	//2
	0b10000000,
	0b10000000,
	0b10000000,
	0b10000000,
	0b10000000,
	0b10000000,
	0b10000000
};

flash char Submarinebitmap[3][7]={ 
	//0
	0b00000011,
	0b00111111,
	0b01111111,
	0b11111111,
	0b01111111,
	0b00111111,
	0b00000011,
	//1
	0b00000000,
	0b11111111,
	0b11010101,
	0b11111111,
	0b11010101,
	0b11111111,
	0b00000000,
	//2
	0b00000000,
	0b00000001,
	0b11111111,
	0b11111111,
	0b11111111,
	0b00000001,
	0b00000000
};

//define bitmap for PatrolBoat Battleship
flash char extPatrolBoatbitmap[2][7] = {
    // 0
    0b00000000,
    0b10000000,
    0b10000000,
    0b10000000,
    0b10000000,
    0b10000000,
    0b00000000,
    // 1
    0b10000000,
    0b10000000,
    0b10000000,
    0b10000000,
    0b10000000,
    0b10000000,
    0b10000000
};

flash char PatrolBoatbitmap[2][7]={ 
    //0
    0b00011111,
    0b00111111,
    0b01000001,
    0b11110100,
    0b01000001,
    0b00111111,
    0b00011111,
    //1
    0b00000000,
    0b11111100,
    0b10001111,
    0b11001111,
    0b10001111,
    0b11111100,
    0b00000000 
};

//define bitmap for Supporting options
flash char extSupportbitmap[3][7] = {
    // 0
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    //H
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    //K
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000    
};

//define bitmap for PatrolBoat Battleship
flash char Supportbitmap[3][7] = {
    // 0
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    //H
    0b00100010,
    0b00100010,
    0b00100010,
    0b00111110,
    0b00100010,
    0b00100010,
    0b00100010,
    //K
    0b00100010,
    0b00100100,
    0b00101000,
    0b00110000,
    0b00101000,
    0b00100100,
    0b00100010
};

//define some character bitmaps
//5x7 characters
flash char bitmap[38][7]={ 
	//0
	0b01110000,
	0b10001000,
	0b10011000,
	0b10101000,
	0b11001000,
	0b10001000,
	0b01110000,
	//1
	0b00100000,
	0b01100000,
	0b00100000,
	0b00100000,
	0b00100000,
	0b00100000,
	0b01110000, 
	//2
	0b01110000,
	0b10001000,
	0b00001000,
	0b00010000,
	0b00100000,
	0b01000000,
	0b11111000,
    //3
	0b11111000,
	0b00010000,
	0b00100000,
	0b00010000,
	0b00001000,
	0b10001000,
	0b01110000,
	//4
	0b00010000,
	0b00110000,
	0b01010000,
	0b10010000,
	0b11111000,
	0b00010000,
	0b00010000,
	//5
	0b11111000,
	0b10000000,
	0b11110000,
	0b00001000,
	0b00001000,
	0b10001000,
	0b01110000,
	//6
	0b01000000,
	0b10000000,
	0b10000000,
	0b11110000,
	0b10001000,
	0b10001000,
	0b01110000,
	//7
	0b11111000,
	0b00001000,
	0b00010000,
	0b00100000,
	0b01000000,
	0b10000000,
	0b10000000,
	//8
	0b01110000,
	0b10001000,
	0b10001000,
	0b01110000,
	0b10001000,
	0b10001000,
	0b01110000,
	//9
	0b01110000,
	0b10001000,
	0b10001000,
	0b01111000,
	0b00001000,
	0b00001000,
	0b00010000,  
	//A
	0b01110000,
	0b10001000,
	0b10001000,
	0b10001000,
	0b11111000,
	0b10001000,
	0b10001000,
	//B
	0b11110000,
	0b10001000,
	0b10001000,
	0b11110000,
	0b10001000,
	0b10001000,
	0b11110000,
	//C
	0b01110000,
	0b10001000,
	0b10000000,
	0b10000000,
	0b10000000,
	0b10001000,
	0b01110000,
	//D
	0b11110000,
	0b10001000,
	0b10001000,
	0b10001000,
	0b10001000,
	0b10001000,
	0b11110000,
	//E
	0b11111000,
	0b10000000,
	0b10000000,
	0b11111000,
	0b10000000,
	0b10000000,
	0b11111000,
	//F
	0b11111000,
	0b10000000,
	0b10000000,
	0b11111000,
	0b10000000,
	0b10000000,
	0b10000000,
	//G
	0b01110000,
	0b10001000,
	0b10000000,
	0b10011000,
	0b10001000,
	0b10001000,
	0b01110000,
	//H
	0b10001000,
	0b10001000,
	0b10001000,
	0b11111000,
	0b10001000,
	0b10001000,
	0b10001000,
	//I
	0b01110000,
	0b00100000,
	0b00100000,
	0b00100000,
	0b00100000,
	0b00100000,
	0b01110000,
	//J => Blank
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	//K
	0b10001000,
	0b10010000,
	0b10100000,
	0b11000000,
	0b10100000,
	0b10010000,
	0b10001000,
	//L
	0b10000000,
	0b10000000,
	0b10000000,
	0b10000000,
	0b10000000,
	0b10000000,
	0b11111000,
	//M
	0b10001000,
	0b11011000,
	0b10101000,
	0b10101000,
	0b10001000,
	0b10001000,
	0b10001000,
	//N
	0b10001000,
	0b10001000,
	0b11001000,
	0b10101000,
	0b10011000,
	0b10001000,
	0b10001000,
	//O
	0b01110000,
	0b10001000,
	0b10001000,
	0b10001000,
	0b10001000,
	0b10001000,
	0b01110000,
	//P
	0b11110000,
	0b10001000,
	0b10001000,
	0b11110000,
	0b10000000,
	0b10000000,
	0b10000000,
	// Q = #
	0b00000000,
	0b01010000,
	0b11111000,
	0b01010000,
	0b11111000,
	0b01010000,
	0b00000000,
	//R
	0b11110000,
	0b10001000,
	0b10001000,
	0b11110000,
	0b10100000,
	0b10010000,
	0b10001000,
	//S
	0b01111000,
	0b10000000,
	0b10000000,
	0b01110000,
	0b00001000,
	0b00001000,
	0b11110000,
	//T
	0b11111000,
	0b00100000,
	0b00100000,
	0b00100000,
	0b00100000,
	0b00100000,
	0b00100000,
	//U
	0b10001000,
	0b10001000,
	0b10001000,
	0b10001000,
	0b10001000,
	0b10001000,
	0b01110000,
	//V
	0b10001000,
	0b10001000,
	0b10001000,
	0b10001000,
	0b10001000,
	0b01010000,
	0b00100000,
	//W
	0b10001000,
	0b10001000,
	0b10001000,
	0b10101000,
	0b10101000,
	0b10101000,
	0b01010000,
	//X
	0b10001000,
	0b10001000,
	0b01010000,
	0b00100000,
	0b01010000,
	0b10001000,
	0b10001000,
	//Y
	0b10001000,
	0b10001000,
	0b10001000,
	0b01010000,
	0b00100000,
	0b00100000,
	0b00100000,
	//Z
	0b11111000,
	0b00001000,
	0b00010000,
	0b00100000,
	0b01000000,
	0b10000000,
	0b11111000,
	//figure1
	0b01110000,
	0b00100000,
	0b01110000,
	0b10101000,
	0b00100000,
	0b01010000,
	0b10001000,
	//figure2
	0b01110000,
	0b10101000,
	0b01110000,
	0b00100000,
	0b00100000,
	0b01010000,
	0b10001000};

flash char song[] = {120,107,95,90,80,30,60,20,24};

//==================================
//This is the sync generator and raster generator. It MUST be entered from 
//sleep mode to get accurate timing of the sync pulses
#pragma warn-
interrupt [TIM1_COMPA] void t1_cmpA(void)  
begin 
  //start the Horizontal sync pulse    
  PORTD = syncON;     
  //count timer 0 at 1/usec
  TCNT0=0;
  //update the curent scanline number
  LineCount ++ ;   
  //begin inverted (Vertical) synch after line 247
  if (LineCount==248)
  begin 
    syncON = 0b00100000;
    syncOFF = 0;
  end
  //back to regular sync after line 250
  if (LineCount==251)	
  begin
    syncON = 0;
    syncOFF = 0b00100000;
  end  
  //start new frame after line 262
  if (LineCount==263) 
  begin
     LineCount = 1;
  end  
  
  delay_us(2); //adjust to make 5 us pulses
  //end sync pulse
  PORTD = syncOFF;   
  
  if (LineCount<ScreenBot && LineCount>=ScreenTop) 
    begin 
       
       //compute byte index for beginning of the next line
       //left-shift 4 would be individual lines
       // <<3 means line-double the pixels 
       //The 0xfff8 truncates the odd line bit
       //i=(LineCount-ScreenTop)<<3 & 0xfff8; //
       
       #asm
       push r16
       lds   r12, _LineCount
       lds   r13, _Linecount+1
       ldi   r16, 30
       sub  r12, r16 
       ldi  r16,0
       sbc  r13, r16 
       lsl  r12
       rol  r13
       lsl  r12
       rol  r13
       lsl  r12    
       rol  r13
       mov  r16,r12
       andi r16,0xf0
       mov  r12,r16
       pop r16 
       #endasm
        
       //load 16 registers with screen info
       #asm
       push r14
       push r15
       push r16
       push r17
       push r18 
       push r19 
       push r26
       push r27
       
       ldi  r26,low(_screen)   ;base address of screen
       ldi  r27,high(_screen)   
       add  r26,r12            ;offset into screen (add i)
       adc  r27,r13
       ld   r4,x+   	       ;load 16 registers and inc pointer
       ld   r5,x+
       ld   r6,x+  
       ld   r7,x+
       ld   r8,x+ 
       ld   r9,x+
       ld   r10,x+  
       ld   r11,x+
       ld   r12,x+ 
       ld   r13,x+
       ld   r14,x+  
       ld   r15,x+
       ld   r16,x+   
       ld   r17,x+  
       ld   r18,x+
       ld   r19,x 
       
       pop  r27
       pop  r26
       #endasm  

       delay_us(4);  //adjust to center image on screen
       
       //blast 16 bytes to the screen   
       #asm
       ;but first a macro to make the code shorter  
       ;the macro takes a register number as a parameter
       ;and dumps its bits serially to portD.6   
       ;the nop can be eliminated to make the display narrower
       .macro videobits ;regnum
        BST  @0,7
	IN   R30,0x12
	BLD  R30,6
	nop
	OUT  0x12,R30  
	
	BST  @0,6
	IN   R30,0x12
	BLD  R30,6
	nop
	OUT  0x12,R30 
	
	BST  @0,5
	IN   R30,0x12
	BLD  R30,6 
	nop
	OUT  0x12,R30 
	
	BST  @0,4
	IN   R30,0x12
	BLD  R30,6
	nop
	OUT  0x12,R30 
	
	BST  @0,3
	IN   R30,0x12
	BLD  R30,6
	nop
	OUT  0x12,R30 
	
	BST  @0,2
	IN   R30,0x12
	BLD  R30,6
	nop
	OUT  0x12,R30 
	
	BST  @0,1
	IN   R30,0x12
	BLD  R30,6 
	nop
	OUT  0x12,R30 
	
	BST  @0,0
	IN   R30,0x12
	BLD  R30,6
	nop
	OUT  0x12,R30 
       .endm     
        
	videobits r4 ;video line -- byte 1
        videobits r5 ;byte 2  
        videobits r6 ;byte 3
        videobits r7 ;byte 4
        videobits r8 ;byte 5
        videobits r9 ;byte 6
        videobits r10 ;byte 7
        videobits r11 ;byte 8 
        videobits r12 ;byte 9
        videobits r13 ;byte 10  
        videobits r14 ;byte 11
        videobits r15 ;byte 12
        videobits r16 ;byte 13
        videobits r17 ;byte 14
        videobits r18 ;byte 15
        videobits r19 ;byte 16
	clt   ;clear video after the last pixel on the line
	IN   R30,0x12
	BLD  R30,6
	OUT  0x12,R30
								
       pop r19
       pop r18
       pop r17 
       pop r16  
       pop r15
       pop r14
       #endasm
              
    end         
end  
#pragma warn+
                  
//timer 0 overflow ISR
interrupt [TIM0_OVF] void timer0_overflow(void) {
  //reload to force 1 mSec overflow
  TCNT0=T0reload;
} 

//==================================
//plot one point 
//at x,y with color 1=white 0=black 2=invert 
#pragma warn-
void video_pt(char x, char y, char c)
begin   
	
	#asm
	;  i=(x>>3) + ((int)y<<4) ;   the byte with the pixel in it

	push r16
	ldd r30,y+2 		;get x
	lsr r30
	lsr r30
	lsr r30     		;divide x by 8
	ldd r12,y+1 		;get y
       	lsl r12     		;mult y by 16
       	clr r13
	lsl r12
	rol r13
	lsl r12
	rol r13
	lsl r12
	rol r13
	add r12, r30     	;add in x/8
	
	;v2 = screen[i];   r5
        ;v3 = pos[x & 7];  r6
	;v4 = c            r7
	ldi r30,low(_screen)
	ldi r31,high(_screen)
	add r30, r12
	adc r31, r13
	ld r5,Z  		;get screen byte
	ldd r26,y+2 		;get x
	ldi r27,0
	andi r26,0x07           ;form x & 7 
	ldi r30,low(_pos*2)  
	ldi r31,high(_pos*2)
	add r30,r26
	adc r31,r27
	lpm r6,Z
	ld r16,y 		;get c 
       
       ;if (v4==1) screen[i] = v2 | v3 ; 
       ;if (v4==0) screen[i] = v2 & ~v3; 
       ;if (v4==2) screen[i] = v2 ^ v3 ; 
       
       cpi r16,1
       brne tst0
       or  r5,r6
       tst0:
       cpi r16,0
       brne tst2 
       com r6
       and r5,r6
       tst2:
       cpi r16,2
       brne writescrn
       eor r5,r6
       writescrn:
       	ldi r30,low(_screen)
	ldi r31,high(_screen)
	add r30, r12
	adc r31, r13
	st Z, r5        	;write the byte back to the screen
	
	pop r16
	#endasm
       
end
#pragma warn+

//==================================
// put a big character on the screen
// c is index into bitmap
void video_putchar(char x, char y, char c)  
begin 
    v7 = x;
    for (v6=0;v6<7;v6++) 
    begin
        v1 = bitmap[c][v6]; 
        v8 = y+v6;
        video_pt(v7,   v8, (v1 & 0x80)==0x80);  
        video_pt(v7+1, v8, (v1 & 0x40)==0x40); 
        video_pt(v7+2, v8, (v1 & 0x20)==0x20);
        video_pt(v7+3, v8, (v1 & 0x10)==0x10);
        video_pt(v7+4, v8, (v1 & 0x08)==0x08);
    end
end

// this method allows us to display digital ships vertically
// based on the shiptype we select what we want to display in certian
// grid section. We can either display H for hit, K for sunk or section
// of the not sunk yet ship
void video_putchar_Vertical (char x, char y, char c, char shiptype)  
begin 
    v7 = x;
    for (v6=0;v6<7;v6++) 
    begin 
        // select type of ship to be displayed
        if (shiptype == 1) v1 = Carrierbitmap[c][v6]; 
        else if (shiptype == 2) v1 = Destroyerbitmap[c][v6]; 
        else if (shiptype == 3) v1 = AegisCruiserbitmap[c][v6]; 
        else if (shiptype == 4) v1 = Submarinebitmap[c][v6];  
        else if (shiptype == 5) v1 = PatrolBoatbitmap[c][v6];        
        v8 = y+v6;
        video_pt(v8, v7,   (v1 & 0x80)==0x80);  
        video_pt(v8, v7+1, (v1 & 0x40)==0x40); 
        video_pt(v8, v7+2, (v1 & 0x20)==0x20);
        video_pt(v8, v7+3, (v1 & 0x10)==0x10);
        video_pt(v8, v7+4, (v1 & 0x08)==0x08);        
        video_pt(v8, v7+5, (v1 & 0x04)==0x04);
        video_pt(v8, v7+6, (v1 & 0x02)==0x02);        
        video_pt(v8, v7+7, (v1 & 0x01)==0x01);                
        // since our grid box is 10x10 than we needed to add extra bits to each ships section
        // to fill most of the grid's box                     
        if (shiptype == 1) video_pt(v8, v7+8, (extCarrierbitmap[c][v6] & 0x80)==0x80);                        
        else if (shiptype == 2) video_pt(v8, v7+8, (extDestroyerbitmap[c][v6] & 0x80)==0x80);
        else if (shiptype == 3) video_pt(v8, v7+8, (extAegisCruiserbitmap[c][v6] & 0x80)==0x80);
        else if (shiptype == 4) video_pt(v8, v7+8, (extSubmarinebitmap[c][v6] & 0x80)==0x80);
        else if (shiptype == 5) video_pt(v8, v7+8, (extPatrolBoatbitmap[c][v6] & 0x80)==0x80);
    end   
end
// this method allows us to display digital ships horizontally
// based on the shiptype we select what we want to display in certian
// grid section. We can either display H for hit, K for sunk or section
// of the not sunk yet ship   
void video_putchar_Horizontal (char x, char y, char c, char shiptype)  
begin   
    v7 = x;
    for (v6=0;v6<7;v6++) 
    begin 
        // select type of ship to be displayed
        if (shiptype == 1) v1 = Carrierbitmap[c][v6]; 
        else if (shiptype == 2) v1 = Destroyerbitmap[c][v6]; 
        else if (shiptype == 3) v1 = AegisCruiserbitmap[c][v6]; 
        else if (shiptype == 4) v1 = Submarinebitmap[c][v6];  
        else if (shiptype == 5) v1 = PatrolBoatbitmap[c][v6];        
        else if (shiptype == 6) v1 = Supportbitmap[c][v6];
        v8 = y+v6;  
        video_pt(v7,   v8, (v1 & 0x80)==0x80);  
        video_pt(v7+1, v8, (v1 & 0x40)==0x40); 
        video_pt(v7+2, v8, (v1 & 0x20)==0x20);
        video_pt(v7+3, v8, (v1 & 0x10)==0x10);
        video_pt(v7+4, v8, (v1 & 0x08)==0x08);
        video_pt(v7+5, v8, (v1 & 0x04)==0x04);
        video_pt(v7+6, v8, (v1 & 0x02)==0x02);        
        video_pt(v7+7, v8, (v1 & 0x01)==0x01);                 
        // since our grid box is 10x10 than we needed to add extra bits to each ships section
        // to fill most of the grid's box
        if (shiptype == 1) video_pt(v7+8, v8, (extCarrierbitmap[c][v6] & 0x80)==0x80);                        
        else if (shiptype == 2) video_pt(v7+8, v8, (extDestroyerbitmap[c][v6] & 0x80)==0x80);
        else if (shiptype == 3) video_pt(v7+8, v8, (extAegisCruiserbitmap[c][v6] & 0x80)==0x80);
        else if (shiptype == 4) video_pt(v7+8, v8, (extSubmarinebitmap[c][v6] & 0x80)==0x80);
        else if (shiptype == 5) video_pt(v7+8, v8, (extPatrolBoatbitmap[c][v6] & 0x80)==0x80);
        else if (shiptype == 6) video_pt(v7+8, v8, (extSupportbitmap[c][v6] & 0x80)==0x80);
    end
end

//==================================
// put a string of big characters on the screen
void video_puts(char x, char y, char *str, char incy)
begin
	char i ;
	for (i=0; str[i]!=0; i++)
	begin  
		if (str[i]>=0x30 && str[i]<=0x3a) 
			video_putchar(x,y,str[i]-0x30);
		else video_putchar(x,y,str[i]-0x40+9);      
		if (incy == 1)
		  y = y+8;
		else x = x+6;	
	end
end

// this function decodes passed in character string and selects proper orientation
// for the string to be displayed
void video_outputs_ship(char x, char y, char *str, char shiptype, char orientation)
begin
  char i;

  for (i=0; str[i]!=0; i++)
  begin  
    if (str[i]>=0x30 && str[i]<=0x3a) {
      if (orientation==0) video_putchar_Horizontal (x,y,str[i]-0x30, shiptype);
      else video_putchar_Vertical (x,y,str[i]-0x30, shiptype);
    }  
    else { 
      if (orientation==0) video_putchar_Horizontal (x,y,str[i]-0x40+9, shiptype);
      else video_putchar_Vertical (x,y,str[i]-0x40+9, shiptype);      
    }
  end
end

//==================================
//plot a line 
//at x1,y1 to x2,y2 with color 1=white 0=black 2=invert 
//NOTE: this function requires signed chars   
//Code is from David Rodgers,
//"Procedural Elements of Computer Graphics",1985
void video_line(char x1, char y1, char x2, char y2, char c)
begin   
	int e;
	signed char dx,dy,j, temp;
	signed char s1,s2, xchange;
        signed char x,y;
        
	x = x1;
	y = y1;
	dx = cabs(x2-x1);
	dy = cabs(y2-y1);
	s1 = csign(x2-x1);
	s2 = csign(y2-y1);
	xchange = 0;   
	if (dy>dx)
	begin
		temp = dx;
		dx = dy;
		dy = temp;
		xchange = 1;
	end 
	e = ((int)dy<<1) - dx;   
	for (j=0; j<=dx; j++)
	begin
		video_pt(x,y,c) ; 
		if (e>=0)
		begin
			if (xchange==1) x = x + s1;
			else y = y + s2;
			e = e - ((int)dx<<1);
		end
		if (xchange==1) y = y + s2;
		else x = x + s1;
		e = e + ((int)dy<<1);
	end
end

//==================================
//return the value of one point 
//at x,y with color 1=white 0=black 2=invert
char video_set(char x, char y)
begin
	//The following construction 
  	//detects exactly one bit at the x,y location
	i=((int)x>>3) + ((int)y<<4) ;  
    return ( screen[i] & 1<<(7-(x & 0x7)));   	
end

// clear VIDEO screen 
void clearScreen (int startPos, int endPos) {
 
  int index;
  
  for (index = startPos; index < endPos; index++)
     screen[index] = 0;
} 

// allocated Player A ships based on the starting position, orientation,
// and shiptype. These are all the information needed since we can just
// add the ship' length to get its ending position
char allocateShipPlayerA (char startposrow, char startposcol, 
                          char orientation, char shiptype) {
                  
  int gridindex = 0;    
  char tempBox = 0;            
  // horizontal allocation of Player A ship      
  if (orientation == 0) {
    // check if Player A tries to allocate ship which ending position
    // goes out of gird boundries set at 9 vertically and 9 horizontally.
    // If it is outside game playing area then return 0 which means that we
    // were not able to allocate requested ship
    if ((startposcol + shiplength[shiptype-1]) > 10) {
      return 0;
    }  
    else {                             
      // check if the ship to be allocated does not interfere with another
      // ship that already exists on Player's A grid
      while (gridindex < shiplength[shiptype-1]) {
         if (((playerAGrid[startposrow][startposcol+gridindex]&0x0f) == 0x1)||
             ((playerAGrid[startposrow][startposcol+gridindex]&0x0f) == 0x9)) {
           return 0;
         }
         gridindex++;  
      }                                             
     // save the starting position of allocated ship for future computations 
     playerAships[shiptype-1] = (startposcol << 4) | startposrow;           
     // since the current request to place ship on the grid can be serviced
     // we place such ship based on the supplied input such as starting pos, loc..
     for (gridindex = 0; gridindex < shiplength[shiptype-1]; gridindex++) { 
        tempBox = shiptype << 4;                                       
        playerAGrid[startposrow][startposcol+gridindex] = tempBox | 0x1;  
     }   
    }    
  }                               
  // vertical allocation of Player A ship 
  else if (orientation == 1) {
    // same idea like above, check if it is within grid boundries
    if ((startposrow + shiplength[shiptype-1]) > 10) {
      return 0;
    }  
    else {                                                                
      // check if the ship to be allocated does not interfere with another
      // ship that already exists on Player's A grid
      while (gridindex < shiplength[shiptype-1]) {
         if (((playerAGrid[startposrow+gridindex][startposcol]&0x0f) == 0x1)||
             ((playerAGrid[startposrow+gridindex][startposcol]&0x0f) == 0x9)) {
           return 0;
         }
         gridindex++;  
      }                                                              
     // save the starting position of allocated ship for future computations  
     playerAships[shiptype-1] = (startposcol << 4) | startposrow;    
     // since the current request to place ship on the grid can be serviced
     // we place such ship based on the supplied input such as starting pos, loc..
     for (gridindex = 0; gridindex < shiplength[shiptype-1]; gridindex++) {
        tempBox = shiptype << 4;
        playerAGrid[startposrow+gridindex][startposcol] = tempBox | 0x9;   
     }   
    }    
  }      
  // wrong orientation 
  else return 0;
  // we were able to place ship into the grid
  return 1; 
}
// allocated Computer ships based on the starting position, orientation,
// and shiptype. These are all the information needed since we can just
// add the ship' length to get its ending position
char allocateShipComputer (char startposrow, char startposcol, 
                           char orientation, char shiptype) {
                  
  int gridindex = 0;    
  char tempBox = 0;            
  // horizontal allocation of Computer ship      
  if (orientation == 0) {
    // check if Computer tries to allocate ship which ending position
    // goes out of gird boundries set at 9 vertically and 9 horizontally.
    // If it is outside game playing area then return 0 which means that we
    // were not able to allocate requested ship
    if ((startposcol + shiplength[shiptype-1]) > 10) {        
      return 0;
    }  
    else {                    
      // check if ship to be allocated does not interfere with another already
      // existing ship
      while (gridindex < shiplength[shiptype-1]) {
         if (((computerGrid[startposrow][startposcol+gridindex]&0x0f) == 0x1)||
             ((computerGrid[startposrow][startposcol+gridindex]&0x0f) == 0x9)) {
           return 0;
         }
         gridindex++;  
      }               
     // save starting positons of the current ship 
     computerships[shiptype-1] = (startposcol << 4) | startposrow; 
     // allocated requaest ship 
     for (gridindex = 0; gridindex < shiplength[shiptype-1]; gridindex++) { 
        tempBox = shiptype << 4;                                       
        computerGrid[startposrow][startposcol+gridindex] = tempBox | 0x1;  
     }   
    }                                     
  }                                       
    // vertical allocation of Computer ship 
  else if (orientation == 1) { 
    // same idea as above, we check if current ship to be alloacted fits on the grid
    if ((startposrow + shiplength[shiptype-1]) > 10) {      
      return 0;
    }  
    else {  
      // check if none of the previously allocated ships are on the way of new ship
      while (gridindex < shiplength[shiptype-1]) {
         if (((computerGrid[startposrow+gridindex][startposcol]&0x0f) == 0x1)||
             ((computerGrid[startposrow+gridindex][startposcol]&0x0f) == 0x9)) {
           return 0;
         }
         gridindex++;  
      }                                                                            
     // keeps track of the starting pos of ship  
     computerships[shiptype-1] = (startposcol << 4) | startposrow; 
     // allocated current ship vertically 
     for (gridindex = 0; gridindex < shiplength[shiptype-1]; gridindex++) {
        tempBox = shiptype << 4;
        computerGrid[startposrow+gridindex][startposcol] = tempBox | 0x9;   
     }   
    }    
  }  
  // wrong orientation     
  else return 0;
  // we were able to place ship into the grid
  return 1; 
} 

// algorithm that allocate computer ships randomly until allocation was possible
// that is there might have been already earlier place ship or computer generated
// starting pos goes out of bounds
void allocationComputerShips () {
 
 char row, col, orientation;
 char noOfShipsAllocated = 0;
 // allocated up to 5 ships
 while (noOfShipsAllocated < 5) {
     // randomly select starting position 
     row = rand() % 10;
     col = rand() % 10;                  
     // randomize ship's orientations
     orientation = rand() % 2;       
     // check if the allocation was successful
     if (allocateShipComputer(row, col, orientation, noOfShipsAllocated+1))
       noOfShipsAllocated++;   
 }  
}                    

// detect if the current hit is the last one necessary to sink current ship
void shipIsSunk (char startpos, char endpos, char index, 
                 char isrow, char isPlayer, char orientation) {
    
  char temppos;
  char low4bits = 0xf7;
              
  temppos = startpos;
  
  if (orientation == 1)
    low4bits = 0xff;
  // for each section of the ship assign correct label that is assign 0xf7 meaning
  // that the ship has been hit and is being sunk now. Do this for both computer and
  // Player A 
  while (temppos <= endpos) {
     if (isPlayer) {
       if (isrow)
         playerAGrid[index][temppos] = playerAGrid[index][temppos] | low4bits;
       else playerAGrid[temppos][index] = playerAGrid[temppos][index] | low4bits;   
     }  
     else {
       if (isrow)
         computerGrid[index][temppos] = computerGrid[index][temppos] | low4bits;
       else computerGrid[temppos][index] = computerGrid[temppos][index] | low4bits;            
     }  
     temppos++;
  }
}            

// given coordinates where aiming was done and whose turn was it, figures out if shot
// was successful if so then it changes configuration of the corresponding gid else it marks
// correspongind entery in the grid as missed "X"
char detectHitOrMiss (char row, char col, char isPlayerActive) {
  
  char typeofshiphit = 0;
  char temprow = 0, tempcol = 0;
  char orientation = 0;
    
   // this tells that computer missed player A ship in difficult mode    
   isShipSunk = 0;
   // check if its Computer turn to shoot 
   if (isPlayerActive == 0) {                             
      // deterimine type of ship that was in that box if any
      typeofshiphit = (playerAGrid[row][col] & 0xf0) >> 4;  
      // if ship is already sunk then return flase to caller
      if (lefttosinkPlayerA[typeofshiphit-1] == 0) return 0;
      // determine ships orientation on the grid
      orientation = (playerAGrid[row][col] & 0x08)>>3;     
      // check if ship is place vertically or horizontally 
      if (((playerAGrid[row][col] & 0x0f) == 1)||
          ((playerAGrid[row][col] & 0x0f) == 9)) {        
        // increment number of computer direct hits       
        computerDirectHits++;  
        // this tells that computer hit player A ship
        isShipSunk = 0x1;                          
        // decrement number of hits necessary to sink current ship if it is not 0 already
        if (--lefttosinkPlayerA[typeofshiphit-1] == 0) {
          // save current location for future computation
          temprow = playerAships[typeofshiphit-1] & 0x0f;
          tempcol = (playerAships[typeofshiphit-1] & 0xf0) >> 4; 
          // this tells that computer sunk player A ship
          // upper four bits will tell us type of ship sunk and lower if hit was successfull
          isShipSunk = (shiplength[typeofshiphit-1]<<4)|0x3;    
          // check where the hit occured and hande corresponding case
          if ((temprow == row) && (tempcol == col)) {
            if (!orientation) shipIsSunk (tempcol, tempcol+shiplength[typeofshiphit-1]-1, row, 1, 1, orientation);                           
            else shipIsSunk (temprow, temprow+shiplength[typeofshiphit-1]-1, col, 0, 1, orientation);           
          }  
          else if (temprow == row) 
             shipIsSunk (tempcol, tempcol+shiplength[typeofshiphit-1]-1, row, 1, 1, orientation);         
          else 
           shipIsSunk (temprow, temprow+shiplength[typeofshiphit-1]-1, col, 0, 1, orientation);           
        }
        else playerAGrid[row][col] = playerAGrid[row][col] | 0x03;  
      }                               
      // if there was not ship in the box at which computer shot than mark that spot as missed
      else if ((playerAGrid[row][col] & 0x0f) == 0)              
         playerAGrid[row][col] = playerAGrid[row][col] | 0x02; 
      else return 0;   
    }
    // handles case when player A shoots with same control and logic as for computer case
    else {                   
      typeofshiphit = (computerGrid[row][col] & 0xf0) >> 4;
      orientation = (computerGrid[row][col] & 0x08)>>3;    
      // ship present and hit
      if (((computerGrid[row][col] & 0x0f) == 1)||
          ((computerGrid[row][col] & 0x0f) == 9)) {             
        // increment player A direct hits  
        playerADirectHits++;  
        // check if currently hit ship is not already sunk and handle specific case below
        if (--lefttosinkComputer[typeofshiphit-1] == 0) {
          temprow = computerships[typeofshiphit-1] & 0x0f;
          tempcol = (computerships[typeofshiphit-1] & 0xf0) >> 4;            
          if ((temprow == row) && (tempcol == col)) {
            if (!orientation) shipIsSunk (tempcol, tempcol+shiplength[typeofshiphit-1]-1, row, 1, 0, orientation);                           
            else shipIsSunk (temprow, temprow+shiplength[typeofshiphit-1]-1, col, 0, 0, orientation);           
          }  
          else if (temprow == row) 
             shipIsSunk (tempcol, tempcol+shiplength[typeofshiphit-1]-1, row, 1, 0, orientation);         
          else 
           shipIsSunk (temprow, temprow+shiplength[typeofshiphit-1]-1, col, 0, 0, orientation);               
        }  
        else computerGrid[row][col] = computerGrid[row][col] | 0x03;
      }  
      // if ship was not located there than mark as missed
      else if ((computerGrid[row][col] & 0x0f) == 0) 
        computerGrid[row][col] = computerGrid[row][col] | 0x02;              
    }        
    // ship hit or sunk successfull
    return 1;
}

// Game playing plan 
// Algorithm # 1: Easy, not sophisticated randomly tries to hit remaining ships
// Algorithm # 2: Difficult, makes somewhat more sophisticated judgements
void easyComputerMode () {
 
 char row;
 char col; 
 char hitsuccess = 0;   
 // choose randomly where we want compute to shoot
 row = rand() % 10;
 col = rand() % 10;
 
 // if we made multiple shoots to the same grids section than detectHitOrMiss will
 // return 0 and we will keep trying to shoot again until successfull 
 while (!hitsuccess) {
    hitsuccess = detectHitOrMiss(row, col, 0);
    if (!hitsuccess) { 
      row = rand() % 10;
      col = rand() % 10;  
    }
 }   
}               
// its the most sophisticated AI we have in the game. Once it hits some section of any ship
// it keeps shooting until ship is sunk and then it shoots randomly again until it hits.
void difficultComputerMode () {
 
 char row;
 char col; 
 char hitsuccess = 0;  
 char stackIndex = 0;  
 char saveFirstHitShipPos = -1;
   
 while (!hitsuccess) {
   // shoot randomly 
   if (direction == 0) {
     row = rand() % 10;
     col = rand() % 10;    
   }                
   // shoot in -y direction and check if we are not out of bounds
   else if (direction == 0x1) {
     row = (StackOfMoves[lastHitShipPos] & 0x0f) - 0x1;
     col = (StackOfMoves[lastHitShipPos] & 0xf0)>>4;
     if (row < 0) {
       row = (StackOfMoves[firstHitShipPos] & 0x0f) + 0x1;
       direction = 0x2;
     }                                 
   } 
   // shoot in +y direction and check if we are not out of bounds
   else if (direction == 0x2) {
     row = (StackOfMoves[lastHitShipPos] & 0x0f) + 0x1;
     col = (StackOfMoves[lastHitShipPos] & 0xf0)>>4;           
     if (row > 9) {
       row = StackOfMoves[firstHitShipPos] & 0x0f;
       col = ((StackOfMoves[firstHitShipPos] & 0xf0)>>4) - 0x1;
       direction = 0x3;
     }                 
   }                                                             
   // shoot in -x direction and check if we are not out of bounds
   else if (direction == 0x3) {
     row = StackOfMoves[lastHitShipPos] & 0x0f;
     col = ((StackOfMoves[lastHitShipPos] & 0xf0)>>4) - 0x1;
     if (col < 0) {
       col = ((StackOfMoves[firstHitShipPos] & 0xf0)>>4) + 0x1;
       direction = 0x4;  
     }                   
   }            
   // shoot in +x direction and check if we are not out of bounds
   else if (direction == 0x4) {
     row = StackOfMoves[lastHitShipPos] & 0x0f;
     col = ((StackOfMoves[lastHitShipPos] & 0xf0)>>4) + 0x1;
     if (col > 9) direction = 0;
   }      
   // shoot in the chosen direction and part of the grid
   hitsuccess = detectHitOrMiss(row, col, 0);
   
   // check if ship was hit and/or sunk
   if (isShipSunk&0x1) { 
     // check if ship was sunk with latest hit. If that is the case then look into the stack
     // and find any part of the grid that does not line up with either row or col of the last
     // hit. If we find such a shooting box then we initialize lastHitShipPos to that. This is done
     // in particularly for ships that happen to be underneath each other eg
     // A A A A
     //   S S S where A is the Aegis Cruiser and S is submarine. 
     if ((isShipSunk&0x3)==0x3) {
       // check if lastHitShipPos which is an index into the Stack with last 10 moves is grater than
       // the sunk ships' length. If this truns out to be true than we know that we hit more than one
       // ship and we need to keep shooting in its area to sunk it as well
       if (lastHitShipPos > ((isShipSunk&0xf0)>>4)) saveFirstHitShipPos = firstHitShipPos; 
       // initialize first hit with -1 since we want to start all over 
       firstHitShipPos = -1;
       // traverse stack to find any move among last 10 moves on the Stack which row and col does not match
       // currently sunk ship. If so then we pick that row and col for new target otherwise we initialize 
       // everything back to starting position and start shooting randomly
       for (stackIndex = 0; stackIndex < lastHitShipPos; stackIndex++) {
          if (((StackOfMoves[stackIndex]&0x0f)!=row)&& ((direction==0x3)||(direction==0x4))) { 
            if (firstHitShipPos == -1) {
              firstHitShipPos = stackIndex;
              lastHitShipPos = stackIndex;
            }  
            else lastHitShipPos = stackIndex;                                       
          }   
          else if (((StackOfMoves[stackIndex]&0xf0)!=col)&&((direction==0x1)||(direction==0x2))) { 
            if (firstHitShipPos == -1) {
              firstHitShipPos = stackIndex;
              lastHitShipPos = stackIndex;
            }  
            else lastHitShipPos = stackIndex;                       
          }  
       }     
        // if we did not find a new target in the stack above restore everything as it was before we 
        // performed search so that we can start shooting randomly if we miss next time 
       if (firstHitShipPos == -1) {
         lastHitShipPos = saveFirstHitShipPos;
         firstHitShipPos = saveFirstHitShipPos;
       }  
       // if we found a target than start shooting in -y direction otherwise we start at random
       if (firstHitShipPos != -1) direction = 0x1;  
       else if (lastHitShipPos == -1) direction = 0x0;
     }  
     // this means that we hit some part of the ship and it is not sunk yet     
     else {  
       // if this is the very first hit of the ship than save the starting target hit on the stack for further computation 
       if (firstHitShipPos == -1) {
         StackOfMoves[++firstHitShipPos] = (col << 4) | row;
         lastHitShipPos = firstHitShipPos;
         direction = 0x1;
       }  
       // this is subsequent hit of the ship so we need to save hit target accordingly to lastHitShipPos since this
       // tells our last successful hit
       else {
         // handle case when two ships are on top of each other
         if (((StackOfMoves[lastHitShipPos+1]&0x0f)!=row) && (((StackOfMoves[lastHitShipPos+1]&0x0f))>>4!=col)) {
          lastHitShipPos += 2;
         }                    
         // othewise just store the target at next location on the Stack
         else lastHitShipPos += 1;
         StackOfMoves[lastHitShipPos] = (col << 4) | row;  
       }
     }
   }
   // handles case when we miss and figures out based on the previos direction where to shoot next if we were shooting
   // in all directions and did not hit than we start shooting randomly
   else {
     lastHitShipPos = firstHitShipPos;
     if (direction == 0x1) direction = 0x2;
     else if (direction == 0x2) direction = 0x3; 
     else if (direction == 0x3) direction = 0x4;
     else {
       firstHitShipPos = -1;
       lastHitShipPos = -1;
       direction = 0;
     }   
   }
 }  
}

// display welcome message along with our names 
// User must press # key on the keypad in order to start playing game
void displayFrontPage (char timer) {
  // Front Page Definition
  char welcomeMsg[] = "WELCOMEJTO";
  char battleship[] = "BATTLESHIP";
  char by[] = "BY";
  char designerA[] = "TOMASZJMICULA";
  char designerB[] = "GILBERTJCHENG";
  char startGameMsg[] = "PRESSJQ"; 
  char startGame[] = "TOJSTARTJGAME";
  
  if (timer==0) video_puts(2, 40, welcomeMsg, 0);
  else if (timer==1) video_puts(67, 40, battleship, 0);
  else if (timer==2) video_puts(55, 50, by, 0);
  else if (timer==3) video_puts(25, 60, designerA, 0);
  else if (timer==4) video_puts(25, 70, designerB, 0);
  else if (timer==5) video_puts(2,  80, startGameMsg, 0);
  else if (timer==6) video_puts(49, 80, startGame, 0);
   
  return;
}
 
// displays type of algorithms supported in the game (easy and difficult)
// and corresponding numbers that hold the state of difficulty
void displayLevelChoice(char timer) {
  //Level Page Definiton              
   char choose[] = "CHOOSE";
   char level[] = "LEVEL";
   char easyLevel[] = "1JEASY";
   char difficultLevel[] = "2JDIFFICULT";

   if (timer==0) video_puts(25, 40, choose, 0);
   else if (timer==1) video_puts(65, 40, level, 0);
   else if (timer==2) video_puts(35, 50, easyLevel, 0);
   else if (timer==3) video_puts(35, 60, difficultLevel, 0);
} 

// displayes ending message along with the winner and message that
// invites user to play again by pressing # key to start game and
// select game mode again
void displayEndGame(char timer) {
  // End page Definition
  char gameEnd[] = "GAMEJOVER";
  char win[] = "WINS";
  char startGameMsg[] = "PRESSJQ"; 
  char startGame[] = "TOJPLAYJAGAIN";
  
  // display "Game Over" and ask if player wants a rematch    
  if (timer==0) video_puts(40, 40, gameEnd, 0);
  if ((playerADirectHits == 18)&&(timer==20)) video_puts(30, 50, playerALogo, 0);
  else if (timer==20) video_puts(30, 50, computerLogo, 0);
  if ((playerADirectHits == 18)&&(timer==21)) video_puts(75, 50, win, 0);        
  else if (timer==21) video_puts(80, 50, win, 0);        
  
  if (timer == 21) video_puts(2, 70, startGameMsg, 0);
  else if (timer==22) video_puts(49, 70, startGame, 0);
} 
 
// displayes player A ships on to the grid scanning PlayerAGrid row by row
void displayPlayerAShips (char i, char ycount, char jvalue) {
  char j;           
  char gridv;     
  char width = 100;
  char xcount = 0;   
  char orientation = 0; 
  
  //********************** PRINT GRID FOR PLAYER A ***************
  xcount = 10*jvalue;
  // check if there is a ship in any of the 5 currently selected slots on the grid
  for (j = jvalue; j < (jvalue+5); j++) {                                
    // checks if there is a ship allocated   
    if (((playerAGrid[i][j] & 0x03) == 1) ||
        ((playerAGrid[i][j] & 0x03) == 3)) {
      // get the type of ship allocated on that particular section of grid
      gridv = (playerAGrid[i][j] & 0xf0) >> 4;
      // get the orientation of the ship 
      orientation = ((playerAGrid[i][j] & 0x08) == 0x08);      
       // displays "K" whenever ship is marked as sunk either vertically of horizontally
      if (((playerAGrid[i][j] & 0x0f) == 7) || 
          ((playerAGrid[i][j] & 0x0f) == 0xf)) 
        video_outputs_ship(xcount+1,ycount+2,displayShipsSection2, 6, 0);
      // display "H" for hit and play muiscal note when ship is marked as hit on the grid
      // and increments counters for ship that we are looking at so that on the next pass 
      // through the grid we display correct parts of the ship that were not hit yet and those that were
      else if((playerAGrid[i][j] & 0x03) == 3) {
        TCCR2 = 0x00;      
        video_outputs_ship(xcount+1,ycount+2,displayShipsSection1, 6, 0);
        if (gridv == 1) countCarrier++;
        else if (gridv == 2) countDestroyer++;      
        else if (gridv == 3) countAegisCruiser++;      
        else if (gridv == 4) countSubmarine++;   
        else if (gridv == 5) countPatrolBoat++;
      }  
      // check which ship was hit and display that part of the ship which was not hit yet. Counters enable us to track 
      // which part of the ship is hit and which isnt.  
      else if ((gridv == 1)||(gridv==2)||(gridv==3)||(gridv==4)||(gridv==5)) {
        // display section 1 of ship if count for the particular ship is 0  
        if (((countCarrier==0)&&(gridv == 1)) || ((countDestroyer==0)&&(gridv==2))||((countAegisCruiser==0)&&(gridv==3))||
            ((countSubmarine==0)&&(gridv==4)) || ((countPatrolBoat==0)&&(gridv==5)))  { 
          if (orientation==0) video_outputs_ship(xcount+1, ycount+2, displayShipsSection0, gridv, ((playerAGrid[i][j] & 0x08) == 0x08));
          else video_outputs_ship(ycount+1, xcount+2, displayShipsSection0, gridv, ((playerAGrid[i][j] & 0x08) == 0x08));
        }  
        // display section 2 of ship if count for the particular ship is 1
        else if (((countCarrier==1)&&(gridv == 1)) || ((countDestroyer==1)&&(gridv==2))||((countAegisCruiser==1)&&(gridv==3))||
                 ((countSubmarine==1)&&(gridv==4)) || ((countPatrolBoat==1)&&(gridv==5))) {
          if (orientation==0) video_outputs_ship(xcount+1, ycount+2, displayShipsSection1, gridv, ((playerAGrid[i][j] & 0x08) == 0x08));
          else video_outputs_ship(ycount+1, xcount+2, displayShipsSection1, gridv, ((playerAGrid[i][j] & 0x08) == 0x08));
        }  
        // display section 3 of ship if count for the particular ship is 2
        else if (((countCarrier==2)&&(gridv == 1)) || ((countDestroyer==2)&&(gridv==2))||((countAegisCruiser==2)&&(gridv==3))||
                 ((countSubmarine==2)&&(gridv==4))) {
          if (orientation==0) video_outputs_ship(xcount+1, ycount+2, displayShipsSection2, gridv, ((playerAGrid[i][j] & 0x08) == 0x08));
          else video_outputs_ship(ycount+1, xcount+2, displayShipsSection2, gridv, ((playerAGrid[i][j] & 0x08) == 0x08));
        }  
        // display section 4 of ship if count for the particular ship is 3
        else if (((countCarrier==3)&&(gridv == 1)) || ((countDestroyer==3)&&(gridv==2))||((countAegisCruiser==3)&&(gridv==3))) {
          if (orientation==0) video_outputs_ship(xcount+1, ycount+2, displayShipsSection3, gridv, ((playerAGrid[i][j] & 0x08) == 0x08));
          else video_outputs_ship(ycount+1, xcount+2, displayShipsSection3, gridv, ((playerAGrid[i][j] & 0x08) == 0x08));              
        }                                                                 
        // display section 5 of ship if count for the particular ship is 4
        else if (((countCarrier==4)&&(gridv == 1))) {
          if (orientation==0) video_outputs_ship(xcount+1, ycount+2, displayShipsSection4, gridv, ((playerAGrid[i][j] & 0x08) == 0x08));
          else video_outputs_ship(ycount+1, xcount+2, displayShipsSection4, gridv, ((playerAGrid[i][j] & 0x08) == 0x08));
        }  
        // once we display part of the ship that is not hit than increment corresponding counter so that next section can be displayed  
        if (gridv == 1) {
          if (countCarrier < 4) countCarrier++;
          else countCarrier=0;
        } 
        else if (gridv == 2) {
          if (countDestroyer < 3) countDestroyer++;
          else countDestroyer=0;      
        }
        else if (gridv == 3) {
          if (countAegisCruiser < 3) countAegisCruiser++;
          else countAegisCruiser=0;      
        }
        else if (gridv == 4) {
          if (countSubmarine < 2) countSubmarine++;
          else countSubmarine=0;      
        }
        else if (gridv == 5) {
          if (countPatrolBoat < 1) countPatrolBoat++;
          else countPatrolBoat=0;      
        }            
      }  
    }    
    // display "X" whenever there is a mark on the gird that we missed ship
    else if((playerAGrid[i][j] & 0x0f) == 2){
      TCCR2 =0x00;
      video_puts(xcount+3,ycount+2,x, 0);                       
    }
    // display blank grid box since nothing was allocated there and we did not shoot yet 
    else video_puts(xcount+3,ycount+2,blank, 0);                    
      xcount = xcount + 10;               
    }          
}

// displayes computer ships on to the grid scanning computerGrid row by row
void displayComputerShips (char i, char ycount) {

  char j;
  char gridv;     
  char width = 100;   
  char xcount = 0; 
  
   // check if there is a ship in any of the 5 currently selected slots on the grid
  for (j = 0; j < 10; j++) {   
    // checks if there is a ship allocated                       
    if (((computerGrid[i][j] & 0x03) == 1) ||
        ((computerGrid[i][j] & 0x03) == 3)) {
      // get the type of ship allocated on that particular section of grid
      gridv = (computerGrid[i][j] & 0xf0) >> 4;
      // displays "K" whenever ship is marked as sunk either vertically of horizontally
      if (((computerGrid[i][j] & 0x0f) == 7) || 
          ((computerGrid[i][j] & 0x0f) == 0xf) ) 
        video_puts(xcount+3,ycount+2,k, 0);
      // display "H" for hit and play muiscal note when ship is marked as hit on the grid
      else if((computerGrid[i][j] & 0x03) == 3){
        TCCR2 = 0x00; 
        video_puts(xcount+3,ycount+2,h, 0);
      }
    } 
    // display "X" whenever there is a mark on the gird that we missed ship
    else if((computerGrid[i][j] & 0x0f) == 2){ 
       TCCR2 = 0x00;    
       video_puts(xcount+3,ycount+2,x, 0);                       
    }
    // display blank grid box since nothing was allocated there and we did not shoot yet 
    else video_puts(xcount+3,ycount+2,blank, 0);                   
       xcount = xcount + 10;               
    }          
}

// clears entire Video screen                               
void cleanDisplay (char ycount) {
    
  char j;
  char xcount = 0;
  
  for (j = 0; j < 10; j++) {                           
     video_outputs_ship(xcount+3,ycount+2,displayShipsSection0, 6, 0);   
     xcount = xcount + 10;               
  }          
}

// displayes " Place Ships" vertically when user "PlayerA" places
// its ships onto the grid    
void displayPlaceShips() {

 char place[] = "PLACE";
 char ships[] = "SHIPS";
 
 video_puts(110,10, place, 1);
 video_puts(110,55, ships, 1);
}

// initializes all the variables used to control Battleship Game
void initializeGameBoard() {
 
 char i, j;
 
 for (i = 0; i < 10; i++) {
   for (j = 0; j < 10; j++) 
      playerAGrid[i][j] = 0;
 } 
  
 for (i = 0; i < 10; i++) {
   for (j = 0; j < 10; j++)  
      computerGrid[i][j] = 0;
 }                            
 // initialize vars that control the printing of girds and ships
 // for computer and playerA               
 row = 0; 
 col = 0;
 doneprintgrid = 0;    
 doneenteringshippos = 0;
 // set initially player to start the battleship game   
 playerTurn = 1;
 // reset supporting variables for game state
 isRowSet = 0;
 inputReady = 0;
 timer = 0;
 displaytimer=0;
 // reset all the dificult alogrithm indexes into the stack  
 lastHitShipPos = -1; 
 firstHitShipPos = -1;
 // start the allocation of ships with Carrier type ship 
 direction = 0; 
 typeofship = 1;                     
 // initialize key pad state to pushed
 keyPadState = Pushed;
 // set the game play to select level 
 gameState = SelectLevel;
 // reset allocation coordinates for player A ships 
 shipAllocRow = 0;
 shipAllocCol = 0; 
 // reset all the counters that control display of sections of ship  
 countPatrolBoat=0;
 countSubmarine=0;
 countAegisCruiser=0;
 countDestroyer=0;
 countCarrier=0;
 // reset direct hits for player and computer  
 playerADirectHits = 0;
 computerDirectHits = 0; 
} 

//==================================
// set up the ports and timers
void main(void)
begin 
  //init timer 1 to generate sync
  OCR1A = lineTime; 	//One NTSC line
  TCCR1B = 9; 		//full speed; clear-on-match
  TCCR1A = 0x00;	//turn off pwm and oc lines
  TIMSK = 0x11;		//enable interrupt T1 cmp 
  
  //init ports
  DDRD = 0xf0;		//video out and switches 

  TCNT2 = 0;
  //D.5 is sync:1000 ohm + diode to 75 ohm resistor
  //D.6 is video:330 ohm + diode to 75 ohm resistor
  DDRB = 0xff;
  PORTB = 0x00;
       
  // set up timer 0
  TCNT0=T0reload;    
  TCCR0=3;	     // prescalar to 64
  
  //initialize synch constants 
  LineCount = 1;
  syncON = 0b00000000;
  syncOFF = 0b00100000;  
          
  #define width 100                         
  // initialize all the variables in the game
  initializeGameBoard();      
  // set the starting state to be Welcome Page
  gameState = Start;
  
  //enable sleep mode
  MCUCR = 0b10000000;
  #asm ("sei");

  //The following loop executes once/video line during lines
  //1-230, then does all of the frame-end processing
  while(1)
  begin
    
    //stall here until next line starts
    //sleep enable; mode=idle  
    //use sleep to make entry into sync ISR uniform time  
     
    #asm ("sleep"); 
    
    //The following code executes during the vertical blanking
    //Code here can be as long as  
    //a total of 60 lines x 63.5 uSec/line x 8 cycles/uSec     
    if (LineCount==231)
    begin 
        //get lower nibble
        DDRC = 0x0f;
        PORTC = 0xf0; 
        delay_us(5);
        key = PINC;
  	  
        //get upper nibble
        DDRC = 0xf0;
        PORTC = 0x0f; 
        delay_us(5);
        key = key | PINC;
  	  
        //find matching keycode in keytbl
        if (key != 0xff) {   
          for (butnum=0; butnum<maxkeys; butnum++) {    
             if (keytbl[butnum]==key)  break;   
          }                        
        }  
        else butnum=maxkeys;  
        PORTB = ~butnum;   
        
        // State machine to debounce key presses    
        if ((keyPadState == Pushed) && ((butnum < 10)||(butnum == 15)||(butnum == 14))) {
          lastbutnum = butnum;
          keyPadState = Debounce;
          // generate new seed for random number generator 
          srand(TCNT0+rand());   
        }   
        else if (keyPadState == Debounce) {
          if (butnum == lastbutnum) { 
              // get the difficulty level 
              if ((gameState == SelectLevel)&&((butnum==1) || (butnum==2))) {
                gameLevel = butnum;       
                // clear Video screen
                clearScreen(0,1600);  
                gameState = GridSetup;
              }                      
              // get the row coordinate from user were he wants to shoot
              else if ((gameState == PlayGame)&&(!isRowSet)&&(butnum < 10)&&playerTurn&&(timer==0)) {
                shootAtRow = butnum;
                isRowSet = 1;
              }                                                         
              // get the column coordinate from user where he wants to shoot and compute outcome of
              else if ((gameState == PlayGame)&&(butnum < 10)&&playerTurn&&(timer==0)) {
                isRowSet = 0;  
                inputReady = 1;  
                // start note playing mechanism
                TCCR2 = 0x1F;
                OCR2 = 40;                     
                // detect if there was a hit or miss
                detectHitOrMiss(shootAtRow, butnum, 1);   
              }  
              // this handles allocation of player A ships by moving alphabetic letters corresponding to
              // ships available in game
              else if ((gameState == GridSetup)&&((butnum==2)||(butnum==4)||(butnum==8)||(butnum==6))) {
                if (butnum == 2) {
                  if (shipAllocRow > 0) shipAllocRow -= 1;
                }  
                else if (butnum == 4) {                     
                  if (shipAllocCol > 0) shipAllocCol -= 1;
                }  
                else if (butnum == 6) {
                  if (shipAllocCol < 9) shipAllocCol += 1;
                }  
                else if (butnum == 8) {
                  if (shipAllocRow < 9) shipAllocRow += 1;
                }  
              }
              // perform allocation of player A ships
              else if ((gameState == GridSetup)&&((butnum==0)||(butnum==1))) {
                  if (allocateShipPlayerA(shipAllocRow, shipAllocCol, butnum, typeofship)) {
                    typeofship++;                                                           
                    doneenteringshippos = 1;
                  }  
              }                                      
              // detect if # key was pressed
              else if (butnum == 15) {      
                // if we are in the start state than move to the select level
                if (gameState == Start) {
                  clearScreen(0,1600);
                  gameState = SelectLevel;  
                  timer=0;
                  TCCR2 = 0x00;
                }
                // if player A is done with the allocation of his ships than 
                // generate computer ships and start game play 
                else if ((gameState == GridSetup)&&(typeofship==6)) {
                  allocationComputerShips ();  
                  gameState = CleanState;
                  displaytimer=0;
                  timer = 0;     
                  doneprintgrid = 0;                                                        
                  row = 0;
                  col = 0;          
                }  
                // if we are in the end state clean video screen and display winning individual
                else if (gameState == EndGame) {
                  direction = EndGame;
                  gameState = CleanState;
                }
              } 
              keyPadState = Detect;
          }
          else keyPadState = Pushed;
        }                  
        // debounce key presses
        else if (keyPadState == Detect) { 
          if (butnum != lastbutnum)  keyPadState = DebouncedEnd;
        }                      
        // debounce key presses
        else if (keyPadState == DebouncedEnd) { 
          if (butnum == lastbutnum) keyPadState = Detect;                      
          else keyPadState = Pushed;     
        }

       
     if (gameState == Start) {
       // display a battleship and some other info eg our names
       // # starts game  
       displayFrontPage (timer);
       /*TCCR2 = 0x1F;
       if (songcount > 1980){ 
         OCR2 = song[0];
         songcount--;   
       }
       else if (songcount > 1960){
         OCR2 = song[0];
         songcount--;
       }
       else if (songcount > 1940){
         OCR2 = song[0];
         songcount--;
       }
       else if (songcount > 1920){
         OCR2 = song[1];
         songcount--;
       }
       else if (songcount > 1910){
         OCR2 = song[2];
         songcount--;
       }
       else if (songcount > 1900){
         OCR2 = song[2];
         songcount--;
       }
       else if (songcount > 1880){
         OCR2 = song[1];
         songcount--;
       }              
       else if (songcount > 1860){
         OCR2 = song[3];
         songcount--;
       }         
       else if (songcount > 1840){
         OCR2 = song[4];
         songcount--;
       }         
       else if (songcount > 1820){
         OCR2 = song[5];
         songcount--;
       }
       else if (songcount > 1810){
         OCR2 = song[5];
         songcount--;
       }         
       else if (songcount > 1800){
         OCR2 = song[5];
         songcount--;
       }         
       else if (songcount > 1790){
         OCR2 = song[7];
         songcount--;
       }         
       else if (songcount > 1780){
         OCR2 = song[7];
         songcount--;
       }         
       else if (songcount > 1770){
         OCR2 = song[7];
         songcount--;
       }         
       else if (songcount > 1760){
         OCR2 = song[8];
         songcount--;
       }         
       else if (songcount > 1750){
         OCR2 = song[8];
         songcount--;
       }    
       else if (songcount > 1740){
         OCR2 = song[8];
         songcount--;
       }         
       else if (songcount > 1730){
         OCR2 = song[5];
         songcount--;
       }         
       else if (songcount > 1720){
         OCR2 = song[5];
         songcount--;
       }         
       else if (songcount > 1710){
         OCR2 = song[5];
         songcount--;
       }         
       else if (songcount > 1690){
         OCR2 = song[4];
         songcount--;
       }         
       else if (songcount > 1670){
         OCR2 = song[3];
         songcount--;
       }         
       else if (songcount > 1650){
         OCR2 = song[2];
         songcount--;
       }         
       else if (songcount > 1630){
         OCR2 = song[1];
         songcount--;
       }   
       else if (songcount > 1610){
         OCR2 = song[0];
         songcount--;
       }               
       else if (songcount = 1610){
         songcount = 2000;
         TCCR2 = 0x00; 
       }*/
       timer++;
     } 
     else if (gameState == SelectLevel) {
        // display levels of difficulty and wait for the selection
        displayLevelChoice(timer);
        timer++;
     }
     else if (gameState == GridSetup) {
       // display player A game grid initially without any ships allocated
       if (!doneprintgrid) { 
         if (row < 100) {                                       
           if (col < 100) {
            video_line(col,0,col,99,1);     
            col += 10; 
           }
           else {
            video_line(0,row,width,row,1); 
            video_line(width,0,width,99,1);           
            row += 10;  
           }       
         }
         else  {
           video_line(0,99,width,99,1);        
           displayPlaceShips();
           row = 0;
           col = 0;           
           doneprintgrid = 1;
         }
       }  
       else {                                       
         // display player A ships if any were allocated
         if ((row < 10) && (col < 100)) {                                      
           if (displaytimer==0) {
             displayPlayerAShips(row, col, 0);
             displaytimer++;
           }         
           else if (displaytimer==1) {
             displayPlayerAShips(row, col, 5);    
             displaytimer++;
           }  
           else {  
             row++;
             col += 10;             
             displaytimer=0;
           } 
         }  
         else {  
             // for each type of the ship flash corresponding alphabetic letter and start redraw proccess
             if (typeofship==1) video_puts(10*shipAllocCol+3, 10*shipAllocRow+2, c, 0);
             else if (typeofship==2) video_puts(10*shipAllocCol+3, 10*shipAllocRow+2, d, 0); 
             else if (typeofship==3) video_puts(10*shipAllocCol+3, 10*shipAllocRow+2, a, 0);              
             else if (typeofship==4) video_puts(10*shipAllocCol+3, 10*shipAllocRow+2, s, 0); 
             else if (typeofship==5) video_puts(10*shipAllocCol+3, 10*shipAllocRow+2, p, 0);
             
             if (timer < 10) { 
               doneprintgrid = 0;                                                        
               row = 0;
               col = 0;      
               timer++;
             } 
             else timer=0;     
         } 
       }   
     }                                
     // this is the actual game play state                 
     else if (gameState == PlayGame) {    
       // draw computer and player A grids internchangably every 2 secs
       if (!doneprintgrid) { 
         if (row < 100) {                                       
           if (col < 100) {
            video_line(col,0,col,99,1);     
            col += 10; 
           }
           else {
            video_line(0,row,width,row,1); 
            video_line(width,0,width,99,1);           
            row += 10;  
           } 
         }
         else  {
           video_line(0,99,width,99,1);      
           doneprintgrid = 1;
           row = 0;
           col = 0;
         }
       }  
       else {
         // display either computer ships or player A ships based on the playerTurn flag
         // if it is set to 1 than display computer ships and wait for user to shoot otherwise
         // display computer ships
         if ((row < 10) && (col < 100)) {                                       
           if (playerTurn) {
             displayComputerShips(row, col);    
             row++;
             col += 10;              
           }  
           else { 
             if (displaytimer==0) {
               displayPlayerAShips(row, col, 0);
               displaytimer++;
             }         
             else if (displaytimer==1) {
               displayPlayerAShips(row, col, 5);    
               displaytimer++;
             }  
             else {  
               row++;
               col += 10;             
               displaytimer=0;
             }
           }              
         }  
         else {                   
           // display appriopriate logo based on who is shooting
           if (playerTurn) video_puts(110, 20, computerLogo, 1); 
           else video_puts(110, 20, playerALogo, 1);
           // wait for the user to input coordinates   
           if (inputReady) {
             row = 0;
             col = 0;
             timer = 1;
             inputReady = 0;
           } 
           // based on the level of difficulty computer shoots at Player A ships
           else if ((timer==40)&&!playerTurn) {
             if (gameLevel==1) easyComputerMode ();       
             else if (gameLevel==2) difficultComputerMode();
             // start musial notes
             TCCR2 = 0x1F;
             OCR2 = 40;
             row = 0;
             col = 0;        
           } 
           // increment 2 sec counter based on the turn 
           if ((timer>0)&&playerTurn) timer++;  
           else if (!playerTurn) timer++;
           // check if there are any ships that are not sunk and reset game play flags
           if ((timer == 120)&&(playerADirectHits < 18)&&(computerDirectHits < 18)) {
             doneprintgrid = 0;
             playerTurn = playerTurn^0x1;
             timer = 0;        
             row = 0;
             col = 0;      
             gameState=CleanState;
             displaytimer=0;
           }
           // if the 2 secs pass and there are no more ships to sink than go the the end state 
           else if ((timer == 121)&&((playerADirectHits == 18)||(computerDirectHits == 18))) {
              clearScreen(0,1600);
              timer = 0;
              gameState = EndGame;  
           }   
         }   
       } 
     }  
     // display end game message, winning party, and ask if player A wants rematch
     else if (gameState == EndGame) {
       displayEndGame(timer);
       timer++;
     }                               
     // clean video screen and move to the appriopriate state 
     else if (gameState == CleanState) {
       if (displaytimer < 4) {
         clearScreen(displaytimer*400, (displaytimer+1)*400);
         displaytimer++;
       }
       else {
         col = 0;
         row = 0;    
         displaytimer=0;
         if (direction == PlayGame) gameState = EndGame;
         else if (direction == EndGame) initializeGameBoard();
         else gameState = PlayGame;
       } 
     } 
     end  //line 231 
  end  //while
end  //main
