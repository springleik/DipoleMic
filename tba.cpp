//---------------------------------------------------------------------
// tba.cpp implements a command line utility to parse and analyze
// wave format sound files containing measured data corresponding to
// tone bursts transmitted from a point source in free-field conditions.
// Originated 10/14/08 M. Williamsen  <http://my.execpc.com/~williamm>
// for an article in audioXpress Magazine. <http://audioxpress.com>
// This version built on 12/6/08 MSW.
//
// Wave data to be analyzed will be loaded from a wave file that
// you name as a command line argument.  Text output giving results
// of the analysis is sent to the console, which you should redirect
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

// main entry point for waveform analyzer
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
	const char *fname = "infile.wav";
	
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
			
		case 2:		// user specified input file name
			fname = argv[1];
			break;
		
		default:	// show usage text if wrong number of args
			cerr << "Useage: tba infile.wav [delay [numAvg [startFreq [sweep|polar]]]]"
				"\nBuilt " << __DATE__ << '.' << endl;
			return -1;
	}
	
	// open input file in binary mode
	ifstream infile(fname, ios::in | ios::binary);
	if (!infile)
	{
		cerr << "Failed to open input file: " << fname << endl;
		return -2;
	}
	
	// read wave file header info from disk
	// this is brute-force deserialization, which assumes that
	// the same chunks always appear in the same order
	infile.read((char *)&myRiff, sizeof(myRiff));
	infile.read((char *)&myFmt, sizeof(myFmt));
	infile.read((char *)&myData, sizeof(myData));
	if (infile.eof())	// test for success
	{
		cerr << "Failed to read header info from disk." << endl;
		return -3;
	}
	
	// let user know who we are
    cout << "executable:\t" << argv[0]
	   << "\n arguments:\t" << argc-1
	   << "\n file name:\t" << fname << endl;
	   
	// show wave file header info on console
	myRiff.dump();
	myFmt.dump();
	myData.dump();
	
	// show setup for tone burst analysis
	myBurst.showSetup();
	
	// show column headings here
	cout << "numCyc\tduration\tnomFreq\tactFreq"
		"\tabs 1\tabs 2\tdB 1\tdB 2\tdB diff"
		"\tphase 1\tphase 2\tphase diff\tbkg 1\tbkg 2" << endl;
		
	// wait for one delay time before analyzing waveform data
	long i = 0;
	short a = 0;
	for (i = 0; i < myBurst.delay; i++)
	{
		// discard data from both channels during delay
		infile.read((char *)&a, 2);
		infile.read((char *)&a, 2);
	}
	
	// iterate over tone bursts while reading from disk
	for(myBurst.reset(); myBurst.good(); myBurst.next())
	{
		// check input file before reading
		if (infile.eof())
		{
			cerr << "Failed to read tone bursts from disk." << endl;
			return -4;
		}
		myBurst.showDetail();
		cout << '\t';
		myBurst.read(infile);
	}

	// report success
    return 0;
}

// show chunk details on console
void chunkHead::dump()
{
	// convert character array to null-terminated string
    char s[5] = "ABCD";
    memcpy(s, chunkID, 4);
	
	// show on console
    cout << "   chunkID:\t" << s << endl;
	cout << " chunkSize:\t" << chunkSize << endl;
}

// show RIFF chunk details on console
void riffChunk::dump()
{
	chunkHead::dump();	// invoke method in superclass

	// convert character array to null-terminated string
    char s[5] = "ABCD";
    memcpy(s, format, 4);
	
	// show on console
    cout << "    format:\t" << s << endl;
}

// show fmt chunk details on console
void fmtChunk::dump()
{
	chunkHead::dump();	// invoke method in superclass
	
	// show remaining data members on console
	cout <<  "   fmtCode:\t" << fmtCode
		<< "\n   numChan:\t" << numChan
		<< "\n  sampRate:\t" << sampRate
		<< "\n  byteRate:\t" << byteRate
		<< "\nblockAlign:\t" << blockAlign
		<< "\n  bitsSamp:\t" << bitsSamp << endl;
}

// show data chunk details on console
void dataChunk::dump()
{
	chunkHead::dump();	// invoke method in superclass
}
