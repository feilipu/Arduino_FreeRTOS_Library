#ifndef GA_HEADER_h // include guard
#define GA_HEADER_h

#ifdef __cplusplus
extern "C" {
#endif

#include <avr/io.h>

/*--------------------------------------------------*/
/*------------Often Configured Parameters-----------*/
/*--------------------------------------------------*/

#define SAMPLE_RATE   16000   // samples per second
#define DELAY         128000  // bytes of delay

/* Working buffer */
#define CMD_BUFFER_SIZE 8192  // size of working buffer (on heap)

/*--------------------------------------------------*/
/*---------------Public Functions-------------------*/
/*--------------------------------------------------*/

void AudioCodec_ADC_init(void) __attribute__((flatten));
void AudioCodec_ADC(uint16_t* _modvalue) __attribute__((hot, flatten));

void alaw_compress1(int16_t* linval, uint8_t* logval) __attribute__ ((hot, flatten));
void alaw_expand1(uint8_t* logval, int16_t* linval) __attribute__ ((hot, flatten));

void audioCodec_dsp( uint16_t* ch_A, uint16_t* ch_B) __attribute__ ((hot, flatten));
// prototype for the DSP function to be implemented.
// needs to at least provide *ch_A and *ch_B
// within Timer1 interrupt routine - time critical I/O. Keep it short and punchy!

/*--------------------------------------------------*/

void AudioCodec_ADC_init(void)
{
  // setup ADCs
  ADMUX  = _BV(REFS1) | _BV(REFS0) | _BV(ADLAR) | _BV(MUX2) | _BV(MUX1) | _BV(MUX0); // 2.56V reference with external capacitor at AREF pin - left justify - start sampling MIC input ADC7
  ADCSRA = _BV(ADEN) | _BV(ADSC) | _BV(ADATE) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0); // ADC enable, auto trigger, ck/128 = 192kHz
  ADCSRB =  0x00;     // free running mode
  DIDR0  = _BV(ADC7D) | _BV(ADC6D) | _BV(ADC2D) | _BV(ADC1D) | _BV(ADC0D); // turn off digital input for pin ADC6 Line and ADC7 Mic input (and ADC2, ADC1, & ADC0)

  // Analogue Comparator Disable
  // When the ACD bit is written logic one, the power to the Analogue Comparator is switched off.
  // This bit can be set at any time to turn off the Analogue Comparator.
  // This will reduce power consumption in Active and Idle mode.
  // When changing the ACD bit, the Analogue Comparator Interrupt must be disabled by clearing the ACIE bit in ACSR.
  // Otherwise an interrupt can occur when the ACD bit is changed.
  ACSR &= ~_BV(ACIE);
  ACSR |= _BV(ACD);

}

// adc sampling routine
void AudioCodec_ADC(uint16_t* _modvalue)
{
  if (ADCSRA & _BV(ADIF))       // check if sample ready
  {
    *_modvalue = ADCW;          // fetch ADCL first to freeze sample, then ADCH. It is done by the compiler.
    ADCSRA |= _BV(ADIF);        // reset the interrupt flag
  }
}

/*
  ==========================================================================

   FUNCTION NAME: alaw_compress11

   DESCRIPTION: ALaw encoding rule according ITU-T Rec. G.711.

   PROTOTYPE: int8_t alaw_compress1( int16_t linval )
   PARAMETERS:
     linval:  (In)  linear samples (only 12 MSBits are taken into account)
     logval:  (Out) compressed sample (8 bit right justified without sign extension)

   RETURN VALUE: none.

  ==========================================================================
*/
void alaw_compress1 (int16_t* linval, uint8_t* logval)
{
  uint16_t ix, iexp;

  ix = *linval < 0    /* 0 <= ix < 2048 */
       ? ~*linval >> 4 /* 1's complement for negative values */
       :  *linval >> 4;

  /* Do more, if exponent > 0 */
  if (ix > 15)    /* exponent=0 for ix <= 15 */
  {
    iexp = 1;     /* first step: */
    while (ix > 16 + 15)  /* find mantissa and exponent */
    {
      ix >>= 1;
      iexp++;
    }
    ix -= 16;     /* second step: remove leading '1' */

    ix += iexp << 4;    /* now compute encoded value */
  }

  if (*linval >= 0) ix |= (0x0080); /* add sign bit */

  *logval = (uint8_t)(ix ^ (0x0055)); /* toggle even bits */
}
/* ................... End of alaw_compress1() ..................... */


/*
  ==========================================================================

   FUNCTION NAME: alaw_expand1

   DESCRIPTION: ALaw decoding rule according ITU-T Rec. G.711.

   PROTOTYPE: int16_t alaw_expand1( uint8_t logval )

   PARAMETERS:
     logval:  (In)  buffer with compressed samples (8 bit right justified,
                      without sign extension)
     linval:  (Out) buffer with linear samples (13 bits left justified)

   RETURN VALUE: none.
  ============================================================================
*/
void alaw_expand1(uint8_t* logval, int16_t* linval)
{
  uint8_t  ix, iexp;
  int16_t mant;

  ix = (*logval ^ (0x55));  /* re-toggle toggled even bits */

  ix &= 0x7F;     /* remove sign bit */
  iexp = ix >> 4;   /* extract exponent */
  mant = ix & 0x0f; /* now get mantissa */
  if (iexp > 0)
    mant = mant + 16; /* add leading '1', if exponent > 0 */

  mant = (mant << 4) + (0x0008);  /* now mantissa left justified and */
  /* 1/2 quantization step added */
  if (iexp > 1)   /* now left shift according exponent */
    mant = mant << (iexp - 1);

  *linval = *logval > 127 /* invert, if negative sample */
            ? mant
            : -mant;
}
/* ................... End of alaw_expand1() ..................... */

#ifdef __cplusplus
}
#endif

#endif // GA_HEADER_h end include guard
