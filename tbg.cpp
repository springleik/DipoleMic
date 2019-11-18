//---------------------------------------------------------------------
// tbg.cpp implements a command line utility to generate
// wave format sound files containing tone bursts intended to be
// transmitted from a point source in free-field conditions.
// Originated 10/16/08 M. Williamsen  <http://my.execpc.com/~williamm>
// for an article in audioXpress Magazine. <http://audioxpress.com>
// This version built on 12/6/08 MSW.
//
// Generated wave data is written to disk in a file that you
// name as a command line argument.  Text output describing the
// wave data is sent to the console, which you should redirect
// to a text file.
//
// This source code was originally built using XCode 2.2.1
// on an Apple MacBook Pro with Intel Core Duo processor.
// Also built (with trivial changes) and tested under MSVC++ 6.0.
// While it is meant to be easily ported to other environments,
// it should NOT be considered portable, in that it relies heavily
// on specific data sizes and struct packing to match the .WAV format.
// It is meant for Intel byte ordering, and will almost certainly fail
// if built for Motorola or PPC processors.
//
// This code is placed in the public domain for the benefit and
// entertaiment of audio enthusiasts and hobbyists.  Any and all uses
// are encouraged but not supported by the author.
//---------------------------------------------------------------------

// includes are limited to just a few standard files
#include <iostream>
#include <fstream>
// #include <assert.h>	// uncomment this line for MSVC++ 6.0

// include the toneBurst class for this project
#include "toneBurst.h"

using namespace std;

// main entry point for waveform generator
int main (int argc, char * const argv[])
{	
	// check object sizes while debugging
	assert(sizeof(long) == 4);
	assert(sizeof(short) == 2);
	assert(sizeof(riffChunk) == 12);
	assert(sizeof(fmtChunk) == 24);
	assert(sizeof(dataChunk) == 8);
	
	// instantiate a tone burst object
	toneBurst myBurst;

	// containers for wave file header info
	riffChunk myRiff;
	fmtChunk myFmt;
	dataChunk myData;
	
	// set default values
	const char *fname = "outfile.wav";
	
	// check for additional arguments
	// TODO argument bounds checking not implemented
	switch(argc)
	{
		case 6:		// user specified sweep (the default) or polar
			if (toupper(*argv[5]) == 'P')
				{myBurst.init(false);}
			
		case 5:		// user specified start frequency
			myBurst.startFreq = atof(argv[4]);
			
		case 4:		// user specified averaging
			myBurst.numAvg = atol(argv[3]);
		
		case 3:		// user specified delay time in samples
			myBurst.delay = atol(argv[2]);
			
		case 2:		// user specified output file name
			fname = argv[1];
			break;
		
		default:	// show usage text if wrong number of args
			cerr << "Useage: tbg outfile.wav [delay [numAvg [startFreq [sweep|polar]]]]"
				"\nBuilt " << __DATE__ << '.' << endl;
			return -1;
	}
	
	// calculate header details for this wave file
	long theSize = myBurst.getSize();
	myRiff.setSize(theSize);
	myFmt.setSize();
	myData.setSize(theSize);
	
	// open output file in binary mode
	ofstream outfile(fname, ios::out | ios::binary);
	if (!outfile)	// test for success
	{
		cerr << "Failed to open output file: " << fname << endl;
		return -2;
	}
		
	// write wave file header info to disk
	// this is brute-force serialization, which
	// always writes the same chunks in the same order
	outfile.write((char *)&myRiff, sizeof(myRiff));
	outfile.write((char *)&myFmt, sizeof(myFmt));
	outfile.write((char *)&myData, sizeof(myData));
	
	if (outfile.fail())	// test for success
	{
		cerr << "Failed to write header info to disk." << endl;
		return -3;
	}
	
	// show setup info on console
    cout << "executable:\t" << argv[0]
	   << "\n arguments:\t" << argc-1
	   << "\n file name:\t" << fname << endl;
	myBurst.showSetup();
	cout << "numCyc\tduration\tnomFreq\tactFreq " << endl;
	
	// wait for one delay time before calculating waveform data
	long i = 0;
	short a = 0;
	for (i = 0; i < myBurst.delay; i++)
	{
		// write zeroes to both channels during delay
		outfile.write((char *)&a, 2);
		outfile.write((char *)&a, 2);
	}

	// iterate over tone bursts while writing to disk
	for(myBurst.reset(); myBurst.good(); myBurst.next())
	{
		// check output file before writing
		if (outfile.bad())	// test for success
		{
			cerr << "Failed to write tone bursts to disk." << endl;
			return -4;
		}
		myBurst.write(outfile);
		myBurst.showDetail();	// show burst info on console
		cout << endl;
	}
	
	// report success
	return 0;
}

// set data members of RIFF chunk
void riffChunk::setSize(long theSize)
{
	memcpy(chunkID, "RIFF", 4);	// not a null-terminated string
	chunkSize = theSize + 36;
	memcpy(format, "WAVE", 4);	// not a null-terminated string
}

// set data members of fmt chunk
void fmtChunk::setSize()
{
	// assume always 16 bit samples, 2 channel stereo
	memcpy(chunkID, "fmt ", 4);	// not a null-terminated string
	chunkSize = 16 ;	// fixed size = 16 for PCM
	fmtCode = 1;		// code = 1 for PCM
	numChan = 2;		// number of audio channels
	sampRate = SAMPLE_RATE;					// sample rate per second
	byteRate = numChan * sampRate * 2;		// byte rate per second
	blockAlign = numChan * 2;				// byte count per sample
	bitsSamp = 16;		// bit count per sample
}

// set data members of data chunk
void dataChunk::setSize(long theSize)
{
	memcpy(chunkID, "data", 4);	// not a null-terminated string
	chunkSize = theSize;
}
