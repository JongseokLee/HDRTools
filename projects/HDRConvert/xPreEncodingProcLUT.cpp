#include "xPreEncodingProcLUT.h"



xPreEncodingProcLUT::xPreEncodingProcLUT()
{
}


xPreEncodingProcLUT::~xPreEncodingProcLUT()
{
}

void xPreEncodingProcLUT::init()
{
}

void xPreEncodingProcLUT::process(Frame * out, const Frame * inp)
{
	const int IMG_SIZE = (inp -> m_width[0]) * (inp -> m_height[0]);

	const int START_IDX_R = 0;		  // 0 ~ IMG_SIZE - 1
	const int START_IDX_G = IMG_SIZE; // IMG_SIZE ~ IMG_SIZE * 2 - 1
	const int START_IDX_B = IMG_SIZE * 2; // IMG_SIZE * 2 ~ IMG_SIZE * 3 -1

	const int START_IDX_Y	= START_IDX_R;
	const int START_IDX_Cb	= START_IDX_G;
	const int START_IDX_Cr	= START_IDX_B;

	for (int i = 0; i < IMG_SIZE; i++)
	{
		double linear_light_r = inp -> m_floatData[START_IDX_R + i];
		double linear_light_g = inp -> m_floatData[START_IDX_G + i];
		double linear_light_b = inp -> m_floatData[START_IDX_B + i];

		YCbCr10Bits ycbcr10bits;

		ycbcr10bits = preEncProcLUT(linear_light_r, linear_light_g, linear_light_b);

		out->m_ui16Data[START_IDX_Y + i] = ycbcr10bits.y;
		out->m_ui16Data[START_IDX_Cb + i] = ycbcr10bits.cb;
		out->m_ui16Data[START_IDX_Cr + i] = ycbcr10bits.cr;
	}
}

YCbCr10Bits xPreEncodingProcLUT::preEncProcLUT(double lightValueR, double lightValueG, double lightValueB)
{
	return YCbCr10Bits();
}

