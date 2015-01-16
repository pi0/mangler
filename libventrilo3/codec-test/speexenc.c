#include <speex/speex.h>
#include <stdio.h>
#include <arpa/inet.h>

/*The frame size in hardcoded for this sample code but it doesn't have to be*/
#define FRAME_SIZE 640
int main(int argc, char **argv)
{
   char *inFile;
   short in[FRAME_SIZE];
   float input[FRAME_SIZE];
   char cbits[200];
   short nbBytes;
   /*Holds the state of the encoder*/
   void *state;
   /*Holds bits so they can be read and written to by the Speex routines*/
   SpeexBits bits;
   int i, tmp;

   /*Create a new encoder state in narrowband mode*/
   state = speex_encoder_init(&speex_uwb_mode);

   /*Set the quality to 8 (15 kbps)*/
   tmp=10;
   speex_encoder_ctl(state, SPEEX_SET_QUALITY, &tmp);

   /*Initialization of the structure that holds the bits*/
   speex_bits_init(&bits);
   while (1)
   {
      /*Read a 16 bits/sample audio frame*/
      fread(in, sizeof(short), FRAME_SIZE, stdin);
      if (feof(stdin))
         break;
      /*Copy the 16 bits values to float so Speex can work on them*/
      for (i=0;i<FRAME_SIZE;i++)
         input[i]=in[i];

      /*Flush all the bits in the struct so we can encode a new frame*/
      speex_bits_reset(&bits);

      /*Encode the frame*/
      speex_encode(state, input, &bits);
      /*Copy the bits to an array of char that can be written*/
      nbBytes = speex_bits_write(&bits, cbits, 200);

      /*Write the size of the frame first. This is what sampledec expects but
       it's likely to be different in your own application*/
      fprintf(stderr, "nbBytes sent: %d\n", nbBytes);
      //nbBytes = htons(nbBytes);
      fwrite(&nbBytes, sizeof(short), 1, stdout);
      /*Write the compressed data*/
      fwrite(cbits, 1, nbBytes, stdout);

   }

   /*Destroy the encoder state*/
   speex_encoder_destroy(state);
   /*Destroy the bit-packing struct*/
   speex_bits_destroy(&bits);
   return 0;
}
