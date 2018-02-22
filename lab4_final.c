/*************************************************************************************
			       DEPARTMENT OF ELECTRICAL AND ELECTRONIC ENGINEERING
					   		     IMPERIAL COLLEGE LONDON 

 				      EE 3.19: Real Time Digital Signal Processing
					       Dr Paul Mitcheson and Daniel Harvey

				        		  LAB 4: Interrupt I/O

 				            ********* I N T I O. C **********

  Demonstrates inputing and outputing data from the DSK's audio port using interrupts. 

 *************************************************************************************
 				Updated for use on 6713 DSK by Danny Harvey: May-Aug 2006
				Updated for CCS V4 Sept 10
 ************************************************************************************/
/*
 *	You should modify the code so that interrupts are used to service the 
 *  audio port.
 */
/**************************** Pre-processor statements ******************************/

#include <stdlib.h>
//  Included so program can make use of DSP/BIOS configuration tool.  
#include "dsp_bios_cfg.h"

/* The file dsk6713.h must be included in every program that uses the BSL.  This 
   example also includes dsk6713_aic23.h because it uses the 
k   AIC23 codec module (audio interface). */
#include "dsk6713.h"
#include "dsk6713_aic23.h"

// math library (trig functions)
#include <math.h>

// Some functions to help with writing/reading the audio ports when using interrupts.
#include <helper_functions_ISR.h>
#include "fir_coef.txt"
/******************************* Global declarations ********************************/
double x[N] = {0.0};	//initialise buffer to zero
int ptr = N-1;		//initialise pointer to size-1
double extend[2*N] = {0.0};	//initialise double sized buffer to zero

/* Audio port configuration settings: these values set registers in the AIC23 audio 
   interface to configure it. See TI doc SLWS106D 3-3 to 3-10 for more info. */
DSK6713_AIC23_Config Config = { \
			 /**********************************************************************/
			 /*   REGISTER	            FUNCTION			      SETTINGS         */ 
			 /**********************************************************************/\
    0x0017,  /* 0 LEFTINVOL  Left line input channel volume  0dB                   */\
    0x0017,  /* 1 RIGHTINVOL Right line input channel volume 0dB                   */\
    0x01f9,  /* 2 LEFTHPVOL  Left channel headphone volume   0dB                   */\
    0x01f9,  /* 3 RIGHTHPVOL Right channel headphone volume  0dB                   */\
    0x0011,  /* 4 ANAPATH    Analog audio path control       DAC on, Mic boost 20dB*/\
    0x0000,  /* 5 DIGPATH    Digital audio path control      All Filters off       */\
    0x0000,  /* 6 DPOWERDOWN Power down control              All Hardware on       */\
    0x0043,  /* 7 DIGIF      Digital audio interface format  16 bit                */\
    0x008d,  /* 8 SAMPLERATE Sample rate control             8 KHZ                 */\
    0x0001   /* 9 DIGACT     Digital interface activation    On                    */\
			 /**********************************************************************/
};


// Codec handle:- a variable used to identify audio interface  
DSK6713_AIC23_CodecHandle H_Codec;

 /******************************* Function prototypes ********************************/
void init_hardware(void);     
void init_HWI(void);                  
void lab4(void); 
// ---------- implement filter ------
double non_cir (void);//original non-circular
double non_cir_op_1 (void); //optimised version 1 of non-circular
double non_cir_op_2(void); //optimisation 2
double cir(void); //original circular implementation
double cir_op_1 (void); //obtimised 1
double cir_op_2 (void); //optimised 2
/********************************** Main routine ************************************/
void main(){      

	// initialize board and the audio port
  init_hardware();

  /* initialize hardware interrupts */
  init_HWI();
  //sineinit();
  	 		
  /* loop indefinitely, waiting for interrupts */  					
  while(1) 
  {};
  
}
        
/********************************** init_hardware() **********************************/  
void init_hardware()
{
    // Initialize the board support library, must be called first 
    DSK6713_init();
    
    // Start the AIC23 codec using the settings defined above in config 
    H_Codec = DSK6713_AIC23_openCodec(0, &Config);

	/* Function below sets the number of bits in word used by MSBSP (serial port) for 
	receives from AIC23 (audio port). We are using a 32 bit packet containing two 
	16 bit numbers hence 32BIT is set for  receive */
	MCBSP_FSETS(RCR1, RWDLEN1, 32BIT);	

	/* Configures interrupt to activate on each consecutive available 32 bits 
	from Audio port hence an interrupt is generated for each L & R sample pair */	
	MCBSP_FSETS(SPCR1, RINTM, FRM);

	/* These commands do the same thing as above but applied to data transfers to  
	the audio port */
	MCBSP_FSETS(XCR1, XWDLEN1, 32BIT);	
	MCBSP_FSETS(SPCR1, XINTM, FRM);	
	
}


/********************************** init_HWI() **************************************/  
void init_HWI(void)
{
	IRQ_globalDisable();			// Globally disables interrupts
	IRQ_nmiEnable();				// Enables the NMI interrupt (used by the debugger)
	IRQ_map(IRQ_EVT_RINT1,4);		// Maps an event to a physical interrupt (X for transmit, Ex2: R for recieve, Ex1)
	IRQ_enable(IRQ_EVT_RINT1);		// Enables the event (X for transmit, Ex2: R for recieve, Ex1)
	IRQ_globalEnable();				// Globally enables interrupts

} 

/******************** WRITE YOUR INTERRUPT SERVICE ROUTINE HERE***********************/  
void lab4(void)
{
	//mono_write_16Bit(non_cir());
	//mono_write_16Bit(non_cir_op_1());
	mono_write_16Bit(non_cir_op_2());
	/*write the filtered samples to the output*/
	//mono_write_16Bit(cir());
	//mono_write_16Bit(cir_op_1());//original circular 
	//mono_write_16Bit(cir_op_2());//double size buffer 
}

// ------------------------------------------------------------------------------
/******** using non-circular buffer **********/
double non_cir (void)
{
		//declare variables:
	int i;
	int j;
		//initilise y to 0 each time 
	double y = 0.0;
		//shift the buffer
	for( i = N-1; i > 0; i--){
		x[i] = x[i-1];
	}
		//put new sample into buffer
	x[0] = mono_read_16Bit();
		//do convolution for each pair of sample and tabs
	for( j = N-1 ; j > 0 ; j--){
		y += x[j]*b[j];
	}
	return y;
	
}

//------------------------------------------------------------------------
double non_cir_op_1 (void)
{
	int i;
	int j;
		//initilise y to 0 each time
	double y = 0.0;
		//shift the buffer
	for( i = N-1; i > 0; i--){
		x[i] = x[i-1];
	}
	x[0] = mono_read_16Bit(); //put new sample into buffer
	
		//even number of tabs and do convolution for a tab and sum of mirror-imaged samples
	if ( N % 2 == 0 )
	{
		for( i= 0; i <= N/2 - 1; i++)
		{
			y+= b[i]* ( x[i] + x[N-1-i] ); //coeffs are symmetric so b[i] == b[N-1-i]
		}
	}
		//odd number of tabs
	else 
	{
		for( j = 0; j < (N-1)/2; j++)
		{
			y+= b[j]* ( x[j] + x[N-1-j] ); //doesn't include midpoint since b[(N-1)/2 ] is not duplicated
		}
			//for odd cases the midpoint does not have a mirror-imaged pair and need to be computed on its own
		y += b[(N-1)/2] * x[(N-1)/2]; // add the midpoint on outside of the for loop 
	}
	return y;
}

//-----------------------------------------------------------------------------------------
double non_cir_op_2(void)
{
	int i;
	double y = 0.0; // initialise y to 0 each time
		//shift the buffer
	for( i = N-1; i > 0; i--)
	{
		x[i] = x[i-1];
	}
		//put new sample into buffer
	x[0] = mono_read_16Bit();
	//do convolution before midpoint is reached
	for( i = 0; i < (N-1)/2; i++) // true for both odd and even number of samples
	{
		y += b[i]* ( x[i] + x[N-1-i] );
	} // don't need to add anything else for even
		
	if( N%2 != 0 ) //odd case
	{
		y += b[(N-1)/2] * x[(N-1)/2]; // add in the midpoint
	}
	
	return y;
}

// ------------------------------------------------------------------------------
/******** using circular buffer *********/
double cir(void)//original circular 
{
	int i;
	double y = 0.0;
	x[ptr] = mono_read_16Bit(); // read in new samples
	
	// from x[ptr](*b[0]) to end of buffer x[N-1]
	for ( i=ptr; i<N; i++)
	{
		y += x[i]*b[i-ptr]; // i = N-1, x[N-1]* b[N-1- ptr]
	}
	
	//from x[0] to x[ptr-1] 
	// x[0]*b[ (N-1-ptr)+1 + 0]
	for (i = 0; i <ptr ; i++)
	{
		y+= x[i]*b[(N-ptr)+ i ];// i = ptr-1; x[ptr-1]*b[N--ptr+ptr-1] = x[ptr-1]*b[N-1]
	}
		//reset the ptr when reaches zero,decrement otherwise
	if(ptr ==0)
	{
		ptr = N-1;
	}
	else
	{
		ptr--;
	}
	return y;
}

//--------------------------------------------------------------------------------
double cir_op_1 (void)// circular with only half of the tabs
{
	int i;
	double y = 0.0;
	x[ptr] = mono_read_16Bit();
	
	//convolution
	for(i=0; i<(N-1)/2; i++)
	{
		int index = ptr-i-1;
			//make sure the index does not go below 0. when it does, warp it around
		if( index < 0 )
		{
			index = N+index;
		}
		y += b[i]* ( x[(ptr+i)%N ] + x[index] );
	}
	
	if( N % 2 != 0 )//odd, add in the midpoint
	{
		y+= b[(N-1)/2]* x[ (ptr + (N-1)/2) % N ];
	}
	
	if(ptr == 0)
	{
		ptr = N-1;
	}
	else
	{
		ptr--;
	}
	return y;
}

//-----------------------------------------------------------------------------------------------
double cir_op_2 (void)//circular with double sized buffer
{
	int i;
	double y = 0.0;
	extend[ptr] = mono_read_16Bit();
	extend[N+ptr]= extend[ptr];
	
	for( i = 0; i < (N-1)/2; i++) // true for both odd and even number of samples
	{
		y += b[i]* ( extend[ptr+i] + extend[ (N+ptr-1) - i] );
	} // don't need to add anything else for even case
	
	if( N%2 != 0 ) //odd case
	{
		y+= b[(N-1)/2]* extend[ (ptr + (N-1)/2) ];
	}
		
	if(ptr == 0)
	{
		ptr = N-1;
	}
	else
	{
		ptr--;
	}
	return y;
	
}

