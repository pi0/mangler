#include <speex/speex.h>
#include <stdio.h>
#include <arpa/inet.h>

/*The frame size in hardcoded for this sample code but it doesn't have to be*/
#define FRAME_SIZE 640
int main(int argc, char **argv)
{
   char *outFile;
   /*Holds the audio that will be written to file (16 bits per sample)*/
   short out[FRAME_SIZE];
   /*Speex handle samples as float, so we need an array of floats*/
   float output[FRAME_SIZE];
   char cbits[200];
   short nbBytes;
   /*Holds the state of the decoder*/
   void *state;
   /*Holds bits so they can be read and written to by the Speex routines*/
   SpeexBits bits;
   int i, tmp;

   /*Create a new decoder state in narrowband mode*/
   state = speex_decoder_init(&speex_uwb_mode);

   /*Set the perceptual enhancement on*/
   tmp=1;
   speex_decoder_ctl(state, SPEEX_SET_ENH, &tmp);

   /*Initialization of the structure that holds the bits*/
   speex_bits_init(&bits);
   while (1)
   {
      /*Read the size encoded by sampleenc, this part will likely be
        different in your application*/
      fread(&nbBytes, sizeof(short), 1, stdin);
      //nbBytes = ntohs(nbBytes);

      fprintf (stderr, "nbBytes recv: %d\n", nbBytes);
      if (feof(stdin))
         break;

      /*Read the "packet" encoded by sampleenc*/
      fread(cbits, 1, nbBytes, stdin);
      /*Copy the data into the bit-stream struct*/
      speex_bits_read_from(&bits, cbits, nbBytes);

      /*Decode the data*/
      speex_decode(state, &bits, output);

      /*Copy from float to short (16 bits) for output*/
      for (i=0;i<FRAME_SIZE;i++)
         out[i]=output[i];

      /*Write the decoded audio to file*/
      fwrite(out, sizeof(short), FRAME_SIZE, stdout);
   }

   /*Destroy the decoder state*/
   speex_decoder_destroy(state);
   /*Destroy the bit-stream truct*/
   speex_bits_destroy(&bits);
   return 0;
}
