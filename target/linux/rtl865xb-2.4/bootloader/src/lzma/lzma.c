#include "LzmaDecode.h"

int lzma_decompress(unsigned char *ibuf, unsigned char *obuf, unsigned int isize)
{
   CLzmaDecoderState state;
   unsigned int osize;

   /* Uncompressed file size*/
   osize = (ibuf[8]<<24)+(ibuf[7]<<16)+(ibuf[6]<<8)+ibuf[5];

   if (LzmaDecodeProperties(&state.Properties, ibuf, LZMA_PROPERTIES_SIZE) != LZMA_RESULT_OK)
   {
      printf("Not a legal LZMA Image!\n");
      return 1;
   }

   {
      unsigned int iproc, oproc;
      unsigned char probs[LzmaGetNumProbs(&state.Properties) * sizeof(CProb)];

      state.Probs = (CProb *) probs;

      if (LzmaDecode(&state, ((unsigned char *)ibuf) + 5 + 8, isize - 5 - 8, &iproc, obuf, osize, &oproc) != 0)
         return 1;
   }

   return 0;
}
