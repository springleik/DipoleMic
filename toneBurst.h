//-----------------------------------------------------------------------------
// toneBurst.h is a common header for command line utilities which use
// wave format sound files containing measured data corresponding to
// tone bursts transmitted from a point source in free-field conditions.
// Originated 10/14/08 M. Williamsen  <http://my.execpc.com/~williamm>
// for an article in audioXpress Magazine. <http://audioxpress.com>
// This version built on 12/6/08 MSW.
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
// are encouraged, but not supported by the author.
//----------------------------------------------------------------------------

// define constant values used in generation and analysis
#define SAMPLE_RATE 44100	// number of samples per second
#define INTERVAL 22050		// number of samples per burst
#define BURST_LENGTH 100	// minimum length, in samples
#define AMPLITUDE 12000.0		// nominal +0 dB signal level
// #define M_PI 3.1415926535898	// uncomment this line for MSVC++ 6.0

// tone burst object, does not get written to disk
// so it's not sensitive to byte order and packing
class toneBurst
{
private:
	// data members
	long sampleRate;	// number of samples per second
	long duration;		// number of samples within burst
	long interval;		// number of samples per burst
	long burstMin;		// minimum number of samples within burst
	long numBurst;		// number of bursts
	long burstCount;	// index to count bursts
	long numCycle;		// number of cycles per burst
	double nominalFreq;	// nominal tone burst frequency
	double actualFreq;	// actual tone burst frequency
	double stopFreq;	// sweep end frequency
	double freqIncr;	// freq sweep increment
	double factor;		// frequency in sample-based units
	bool sweep;			// true if freq sweep, false if polar

public:
	double startFreq;	// sweep start frequency
	long delay;			// offset to start of first burst
	long numAvg;		// number of bursts to average over

private:
	// method members
	void calc();		// calculate next frequency

public:
	void showDetail();	// show details at one frequency
	void showSetup();	// show general setup info
	void read(std::ifstream &infile);	// read tone burst from disk
	void write(std::ofstream &outfile);	// write tone burst to disk
	void reset();		// reset burst object
	bool next();		// increment frequency, return false if done
	bool good();		// return false if done
	long getSize();		// get byte count for generated tone bursts
	toneBurst();		// default constructor
	void init(bool theSweep);  // calculate internal values
};

// container for ID and size
// must be exactly 8 bytes, in order as shown
class chunkHead
{
protected:
	// data members
	char chunkID[4];	// 4 characters, not null-terminated
	long chunkSize;		// remaining byte count in this file

	// method member
	void dump();
};

// RIFF chunk descriptor, exactly as written out to disk
// must be exactly 12 bytes, in order as shown
class riffChunk: chunkHead
{
private:
	// data member
	char format[4];		// 4 characters, not null-terminated
	
public:
	// method members
	void dump();
	void setSize(long theSize);
};

// FMT sub-chunk, exactly as written out to disk
// must be exactly 24 bytes, in order as shown
class fmtChunk: chunkHead
{
private:
	// data members
	short fmtCode;		// data format code
	short numChan;		// number of audio channels
	long sampRate;		// sample rate per second
	long byteRate;		// byte rate per second
	short blockAlign;	// byte count per sample
	short bitsSamp;		// bits count per sample
	
public:
	// method members
	void dump();
	void setSize();
};

// DATA sub-chunk, exactly as written out to disk
// must be exactly 8 bytes, plus data which follows
class dataChunk: chunkHead
{
	// actual sound data starts here, but we won't try to load it into memory
	
public:
	// method members
	void dump();
	void setSize(long theSize);
};
