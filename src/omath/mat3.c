
#include "mat3.h"

inline mat3f *mat3f_div_s( const mat3f *const a, const float s, mat3f *out ) {
	for( unsigned int i = 0; i < 9; ++i )
		out->data[i] = a->data[i] / s;
	return out;
}

inline mat3f *mat3f_inverse( const mat3f *in, mat3f *out ) {
	const float S00 = in->data[0];
	const float S01 = in->data[1];
	const float S02 = in->data[2];
	const float S10 = in->data[3];
	const float S11 = in->data[4];
	const float S12 = in->data[5];
	const float S20 = in->data[6];
	const float S21 = in->data[7];
	const float S22 = in->data[8];
	mat3f inv = { .data = {
			S11 * S22 - S21 * S12,
            S12 * S20 - S22 * S10,
            S10 * S21 - S20 * S11,
            S02 * S21 - S01 * S22,
            S00 * S22 - S02 * S20,
            S01 * S20 - S00 * S21,
            S12 * S01 - S11 * S02,
            S10 * S02 - S12 * S00,
            S11 * S00 - S10 * S01 }
	};
	const float determinant =
			S00 * (S11 * S22 - S21 * S12) -
			S10 * (S01 * S22 - S21 * S02) +
			S20 * (S01 * S12 - S11 * S02);
	mat3f_div_s( &inv, determinant, out );
	return out;
}

inline mat3f *mat3f_transpose( const mat3f *const in, mat3f *out ) {
	out->data[0] = in->data[0];
	out->data[1] = in->data[3];
	out->data[2] = in->data[6];
	out->data[3] = in->data[1];
	out->data[4] = in->data[4];
	out->data[5] = in->data[7];
	out->data[6] = in->data[2];
	out->data[7] = in->data[5];
	out->data[8] = in->data[8];
	return out;
}
