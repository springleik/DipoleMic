//-----------------------------------------------------------------------------
// toneBurst.cpp implements the toneBurst class for command line utilities which
// use wave format sound files containing measured data corresponding to
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

// includes are limited to just a few standard files
#include <iostream>
#include <fstream>
#include <complex>
// #include <assert.h>	// uncomment this line for MSVC++ 6.0

// include the toneBurst class header file
#include "toneBurst.h"

using namespace std;

// default constructor for tone burst object
toneBurst::toneBurst()
{
	// initialize data members
	sampleRate = SAMPLE_RATE;	// standard audio sample rate
	duration = 0;
	interval = INTERVAL;		// burst repetition rate
	delay = INTERVAL;			// delay one full interval
	burstMin = BURST_LENGTH;	// minimum burst length 2.27 msec.
	numBurst = 201;				// 100 bursts per decade
	burstCount = 0;
	numAvg = 1;
	numCycle = 1;			// start with one cycle per burst
	nominalFreq = 100.0;
	actualFreq = 0.0;
	startFreq = 100.0;
	stopFreq = 10000.0;
	freqIncr = 1.0;
	sweep = true;			// default to freq sweep mode
	
	// calculate details for first frequency
	calc();
}

// only called if user specifies polar mode
void toneBurst::init(bool theSweep)
{
	// set data members
	sweep = theSweep;
	if (sweep)	// handle frequency sweep case
	{
		numBurst = 201;
		startFreq = 100.0;
		stopFreq = 10000.0;
	}
	else		// handle polar plot case
	{
		numBurst = 72;
		startFreq = 1000.0;
		stopFreq = startFreq;
		interval = 2 * INTERVAL;	// allow time to set turntable
	}
}

// always called before analyzing bursts
void toneBurst::reset()
{
	// set burst count for all modes
	numCycle = 1;
	burstCount = numBurst;
	nominalFreq = startFreq;
	
	// handle frequency sweep case
	if (sweep)
	{
		numBurst = 201;
		stopFreq = 10000;
		freqIncr = pow(stopFreq/startFreq, 1.0/(numBurst - 1));
	}
	
	// handle polar plot case
	else
	{
		// set up for polar plot
		numBurst = 72;
		stopFreq = startFreq;
		freqIncr = 1.0;
	}

	// recalculate for specific frequency
	calc();
}

// tone burst iterator
bool toneBurst::next()
{
	// check if more bursts remain
	if (burstCount > 0)
	{
		if (sweep)
		{
			// only update for freq sweep case
			nominalFreq *= freqIncr;
			calc();
		}
		burstCount--; return true;
	}
	else return false;
}

// test to see if we are done
bool toneBurst::good()
{
	// check if more bursts remain
	return (burstCount > 0);
}

// update data members for a particular frequency
void toneBurst::calc()
{
	// find least number of full cycles whose duration exceeds burst minimum
	while ((sampleRate / nominalFreq) * numCycle < burstMin){numCycle++;}
	
	// round off (truncate) duration to exact samples
	duration = long((sampleRate / nominalFreq) * numCycle);
	
	// find frequency that exactly fills the duration
	actualFreq = 1.0 * sampleRate * numCycle / duration;
	
	// calculate common factor for sine and cosine
	factor = 2.0 * M_PI * actualFreq / sampleRate;
}

// show burst parameters on console
void toneBurst::showDetail()
{
	cout << numCycle 
		<< '\t' << duration
		<< '\t' << nominalFreq 
		<< '\t' << actualFreq;
}

// show setup info on console
void toneBurst::showSetup()
{
	// show waveform details on console
	cout << "      mode:\t" << (sweep ? "freq sweep" : "polar plot")
	   << "\nstart freq:\t" << startFreq
	   << "\n  end freq:\t" << stopFreq
	   << "\n num steps:\t" << numBurst
	   << "\n averaging:\t" << numAvg
	   << "\n     delay:\t" << delay
	   << "\n  interval:\t" << interval
	   << endl;
}

// read tone burst from disk, matched filter technique
// looks only for the exact frequency being measured
void toneBurst::read(ifstream &infile)
{
	long i = 0, j = 0;		// local loop indices, NOT sqrt(-1)
	short a1 = 0, a2 = 0;
	complex<double> sum1(0,0);	// channel 1 response
	complex<double> sum2(0,0);	// channel 2 response
	complex<double> sum3(0,0);	// channel 1 background
	complex<double> sum4(0,0);	// channel 2 background
	complex<double> cfactor(0, factor);
	
	// iterate over averaging, burst interval
	for (i = 0; i < numAvg; i++)
	for (j = 0; j < interval; j++)
	{
		// read data from both channels
		infile.read((char *)&a1, 2);
		infile.read((char *)&a2, 2);
		
		// analyze burst response
		// this is a single frequency discrete Fourier transform
		// phase angle is referred to start of burst
		if (j < duration)
		{
			complex<double> ccoeff = exp(cfactor * double(j)); 
			sum1 += double(a1) * ccoeff;
			sum2 += double(a2) * ccoeff;
		}
		
		// analyze background level, near end of burst interval
		if ((j < (interval - duration)) && (j >= (interval -  (2 * duration))))
		{
			complex<double> ccoeff = exp(cfactor * double(j)); 
			sum3 += double(a1) * ccoeff;
			sum4 += double(a2) * ccoeff;
		}
	}
	
	// factor out sample count and averaging, normalize to +0 dB
	sum1 /= (duration * numAvg * AMPLITUDE / 2.0);
	sum2 /= (duration * numAvg * AMPLITUDE / 2.0);
	sum3 /= (duration * numAvg * AMPLITUDE / 2.0);
	sum4 /= (duration * numAvg * AMPLITUDE / 2.0);
	
	// report results to console
	// 0.0 dB reference level when analyzing original generated file
	cout <<        abs(sum1)						// magnitude channel 1
		<< '\t' << abs(sum2)						// magnitude channel 2
		<< '\t' << 20.0*log10(abs(sum1))			// dB channel 1
		<< '\t' << 20.0*log10(abs(sum2))			// dB channel 2
		<< '\t' << 20.0*log10(abs(sum1)/abs(sum2))	// dB difference
		<< '\t' << arg(sum1)						// phase channel 1
		<< '\t' << arg(sum2)						// phase channel 2
		<< '\t' << arg(sum1) -arg(sum2)				// phase difference
		<< '\t' << 20.0*log10(abs(sum3))			// dB background 1
		<< '\t' << 20.0*log10(abs(sum4))			// dB background 2
		<< endl;
	return;
}

// write a burst to output stream
void toneBurst::write(ofstream &outfile)
{
	long i = 0, j = 0;	// local loop indices, NOT sqrt(-1)
	short a = 0;
	double y = 0.0;
	
	// iterate over averaging, burst interval, number of channels
	for (i = 0; i < numAvg; i++)
	for (j = 0; j < interval; j++)
	{
		// calculate burst waveform
		if (j < duration)
		{
			// raised cosine and second harmonic
			y = cos(factor * j) - cos(2.0 * factor * j);
			
			// normalize to +0 dB amplitude, convert to short word
			a = short(y * AMPLITUDE);
		}
		else
		{
			a = 0;	// write silence between bursts
		}

		// write the same data to both channels, for now
		outfile.write((char *)&a, 2);
		outfile.write((char *)&a, 2);
	}
	return;
}

// get byte count for sound data in wave file
long toneBurst::getSize()
{
	// bytes/sample * num channels * (samples/burst * averaging * num bursts + delay)
	// always assumes 2 byte samples, 2 channel stereo
	return (2 * 2 * (interval * numAvg * numBurst + delay));
}

