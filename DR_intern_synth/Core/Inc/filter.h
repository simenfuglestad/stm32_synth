/*
 * filter.h
 *
 *  Created on: Feb 15, 2022
 *      Author: sfs
 */

#ifndef INC_FILTER_H_
#define INC_FILTER_H_

#include <stdint.h>

//defines

//type defs
typedef enum {
	LOWPASS, HIGHPASS, BANDPASS, NOTCH
} filter_response_t;

typedef struct {
	filter_response_t response;
	uint16_t prev_y;
	float fc;
	float delta_t;
	float* R;
	float* C;
	float gain;
	float* coeff;
} filter_t;

typedef uint16_t (filter_f)(filter_t*, uint16_t);

uint16_t filter_apply_all(filter_f** filter_functions, filter_t* filters, uint16_t inp, uint8_t nf);


// Low pass

void filter_lowpass_RC_init(filter_t* filter, float delta_t, float fc, float gain);

uint16_t filter_lowpass_RC_get_next(filter_t* filter, uint16_t x);

void filter_lowpass_RC_set_R(filter_t* filter,  float R, uint8_t ri);
void filter_lowpass_RC_set_C(filter_t* filter, float C, uint8_t ci);
void filter_lowpass_RC_set_coeffs(filter_t* filter, float delta_t, float R, float C);

float filter_lowpass_RC_get_fc(filter_t* filter);
float filter_lowpass_compute_fc(float R, float C);
float filter_lowpass_compute_R(float fc, float C);
float filter_lowpass_compute_C(float fc, float R);

#endif /* INC_FILTER_H_ */
