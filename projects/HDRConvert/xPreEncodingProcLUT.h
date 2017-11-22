#pragma once

#include "Global.H"
#include "Parameters.H"
#include "Frame.H"
#include "LookUpTable.H"

struct YCbCr10Bits
{
	int y;	//0~1023
	int cb; //0~1023
	int cr; //0~1023
};

class xPreEncodingProcLUT
{

public:

	// Construct/Deconstruct
	xPreEncodingProcLUT();
	~xPreEncodingProcLUT();
	//----------------------//

	void init();
	void process(Frame* out, const Frame *inp);
	
private:
	//input: linear light R:G:B 4:4:4 Float
	//output: Y (0~1023), Cb (0~1023), Cr (0~1023)
	YCbCr10Bits preEncProcLUT(double lightValueR, double lightValueG, double lightValueB);
};

