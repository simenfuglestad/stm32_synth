/*
 * MIDI_application.c
 *
 *  First created on: 6 dec. 2014
 *      Author: Xavier Halgand
 */

/* Includes ------------------------------------------------------------------*/

//#include "main.h"
#include "modulator.h"
#include "MIDI_application.h"
#include "usb_host.h"
#include "mixer.h"
#include "modulator.h"
#include "envelope.h"

/* Private define ------------------------------------------------------------*/

#define RX_BUFF_SIZE 64 /* USB MIDI buffer : max received data 64 bytes */

uint8_t MIDI_RX_Buffer[RX_BUFF_SIZE]; // MIDI reception buffer

static const uint16_t MIDI_PITCH_REF 	= 0x3fff; //maximum when midi data bytes are 0x7f and 0x7f
static uint16_t MIDI_ctrl_pitch 		= 0x2000; //centered when midi data bytes are 0x00 0x40
static const float OCT_D2 				= NOTE_F_DOWN * NOTE_F_DOWN;
static const float OCT_U2				= NOTE_F_UP * NOTE_F_UP;
static const float OCT_R2				= NOTE_F_UP - NOTE_F_DOWN;

#define N_MIDI_NOTES 127

static uint8_t MIDI_input_keys[MODULATOR_MAX_VOICES];
static uint8_t next_key_index;

//static uint8_t MIDI_input_codes[MODULATOR_MAX_VOICES];
//static uint8_t MIDI_input_indexes[MODULATOR_MAX_VOICES];
//static uint8_t input_index;

static int on_event;
static int off_event;

static uint8_t next_free;

static env_t envelopes[MODULATOR_MAX_VOICES];

/* Private function prototypes -----------------------------------------------*/
void ProcessReceivedMidiData(uint8_t midi_code, uint8_t midi_data1, uint8_t midi_data2);

/*-----------------------------------------------------------------------------*/
/**
 * @brief  Main routine for MIDI application, looped in main.c
 * @param  None
 * @retval none
 */
void MIDI_Application(void)
{
	if (Appli_state == MIDI_APPLICATION_READY)
	{
		USBH_MIDI_Receive(&hUsbHostFS, MIDI_RX_Buffer, RX_BUFF_SIZE); // just once at the beginning, start the first reception
		Appli_state = APPLICATION_RUNNING;
	}
	if (Appli_state == APPLICATION_RUNNING)
	{
		//....pffff......grrrrr......
	}

	if (Appli_state == MIDI_APPLICATION_DISCONNECT)
	{
		Appli_state = MIDI_APPLICATION_IDLE;
		USBH_MIDI_Stop(&hUsbHostFS);
	}
}

/*-----------------------------------------------------------------------------*/
/**
 * @brief  MIDI data receive callback.
 * @param  phost: Host handle
 * @retval None
 */
void USBH_MIDI_ReceiveCallback(USBH_HandleTypeDef *phost)
{
	for (uint8_t i = 0; i < RX_BUFF_SIZE; i+=MIDI_MSG_LEN) {
		ProcessReceivedMidiData(MIDI_RX_Buffer[i], MIDI_RX_Buffer[i + 2], MIDI_RX_Buffer[i+3]);
	}

	USBH_MIDI_Receive(&hUsbHostFS, MIDI_RX_Buffer, RX_BUFF_SIZE); // start a new reception
}

uint8_t* MIDI_get_input_keys(void) {
	return MIDI_input_keys;
}

void MIDI_update_input(float* f_steps) {
	float pitch_factor;
	pitch_factor = OCT_D2 + OCT_R2 *((float)MIDI_ctrl_pitch/MIDI_PITCH_REF);
	for (uint8_t i = 0; i < MODULATOR_MAX_VOICES; i++) {
//		float fmidi = MIDI_key2f(MIDI_input_keys[i]);
		float fmidi = MIDI_key2f(MIDI_input_keys[i]);
		float f = fmidi/FREQ_BASE;
		if (f < 1)
			f = 0;
		f_steps[i] = f * pitch_factor;
	}
}

float MIDI_key2f(uint8_t key) {
	float exponent = (float)key - 69;
	exponent = exponent/12;
	return 440 * powf(2, exponent);
}

env_t* MIDI_get_envelopes() {
	return envelopes;
}

void MIDI_note_disable(uint8_t midi_code, int index) {
	MIDI_input_keys[index] = 0;
}

void ProcessReceivedMidiData(uint8_t midi_code, uint8_t midi_data1, uint8_t midi_data2) {
	switch(midi_code) {
	case MIDI_CODE_NOTE_OFF:
		env_release(midi_data1);

		break;

	case MIDI_CODE_NOTE_ON:
		on_event++;
		next_free = 0;

		for(uint8_t i = 0; i < MODULATOR_MAX_VOICES; ++i) {
			uint8_t midi = MIDI_input_keys[i];

			if (midi == midi_data1) {
				next_key_index = i;
				break;
			} else if (midi == 0) {
				next_free = i;
			}
		}

		if (next_free != 0 && next_key_index == 0) {
			next_key_index = next_free;
		}
		MIDI_input_keys[next_key_index] = midi_data1;

		env_create(next_key_index, midi_data1);

		next_key_index = 0;

		break;
	case MIDI_CODE_POLY_KEY_PRESS: ; //currently not registering any from microkey S25
	int test_poly_br = 10;
	break;
	case MIDI_CODE_CTRL_CHANGE: ;
	//from sustain btn and north/south joystick movement on KORG microkey s25
	int test_ctrl_br = 10;
	break;
	case MIDI_CODE_PITCH_BEND: ;
	MIDI_ctrl_pitch = midi_data1 + (midi_data2 << 7);
	break;
	default:
		break;
	}
}
