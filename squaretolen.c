/*
 * This algorithm is meant to reimplement the OpenJDK's BigInteger.SquareToLen
 * function. You may find it on:
 * http://hg.openjdk.java.net/jdk9/dev/jdk/file/c35ff69c2852/src/java.base/share/classes/java/math/BigInteger.java#l2035
 */

#include "squaretolen.h"

int32_t* SquareToLen(int32_t *in, int in_len, int32_t *out, int out_len) {

#ifdef _ASM

  long lplw_s, in_aux, out_aux, piece, product, product_s;
  long lplw = 0;

  asm volatile (
      // Store the squares, right shifted one bit (i.e., divided by 2)
      "subi    %[out_aux],   %[out],     8\n\t"
      "subi    %[in_aux],    %[in],      4\n\t"
      "cmpwi   %[in_len],    0\n\t"
      "ble     SKIP_LOOP_SQUARE\n\t"    // in_len <= 0
      "mtctr   %[in_len]\n\t"
      "\n\t"
      "LOOP_SQUARE:\n\t"
      "lwzu    %[piece],      4(%[in_aux])\n\t"
      "mulld   %[product],   %[piece],   %[piece]\n\t"
      // shift left 31 bits and only keep the 31th bit
      "rlwinm  %[lplw_s],    %[lplw],    31, 0, 0\n\t"
      // shift right 33 bits without sign extension
      "srdi    %[product_s], %[product], 33\n\t"
      "or      %[product_s], %[lplw_s],  %[product_s]\n\t"
      "mr      %[lplw],      %[product]\n\t"
      "rldicr  %[product],   %[product], 31, 31\n\t"
      "or      %[product],   %[product], %[product_s]\n\t"
      "stdu    %[product],    8(%[out_aux])\n\t"
      "bdnz    LOOP_SQUARE\n\t"
      "\n\t"
      "SKIP_LOOP_SQUARE:\n\t"
      : /* output / temporaries */
        [lplw] "=&b" (lplw),
        [lplw_s] "=&b" (lplw_s),
        [out_aux] "=&b" (out_aux),
        [in_aux] "=&b" (in_aux),
        [piece] "=&b" (piece),
        [product] "=&b" (product),
        [product_s] "=&b" (product_s)
      : /* input */
        [in] "b" (in),
        [in_len] "b" (in_len),
        [out] "b" (out)
      : /* clobber list */
        "memory", "ctr", "cr0"
      );

  long i_minus1, carry, offset;
  int32_t off_aux, t, mlen, len, a, b;

  asm volatile (
      // Add in off-diagonal sums
      "cmpwi   %[in_len],  0\n\t"
      "ble     SKIP_DIAGONAL_SUM\n\t"
      // Avoid CTR usage here in order to use it at mulAdd
      "sldi    %[i_minus1],%[in_len],   2\n\t"
      "subi    %[i_minus1],%[i_minus1], 4\n\t"
      "li      %[offset],  4\n\t"
      "\n\t"
      "LOOP_DIAGONAL_SUM:\n\t"
      "\n\t"

      // begin<mulAdd>
      "cmpdi   %[i_minus1],0\n\t"
      "li      %[carry],   0\n\t"
      "ble     SKIP_MULADD\n\t"
      "lwzx    %[t],       %[in],       %[i_minus1]\n\t"
      "sldi    %[off_aux], %[out_len],  2\n\t"
      "subi    %[off_aux], %[off_aux],  4\n\t"
      "sub     %[off_aux], %[off_aux],  %[offset]\n\t"
      "subi    %[len],     %[i_minus1], 4\n\t"
      "srdi    %[b],       %[i_minus1], 2\n\t"
      "mtctr   %[b]\n\t"

      "LOOP_MULADD:\n\t"
      "lwzx    %[a],       %[len],      %[in]\n\t"
      "lwzx    %[b],       %[off_aux],  %[out]\n\t"
      "mulld   %[a],       %[a],        %[t]\n\t"
      "add     %[b],       %[b],        %[a]\n\t"
      "add     %[b],       %[b],        %[carry]\n\t"
      "stwx    %[b],       %[off_aux],  %[out]\n\t"
      // shift right 32 bits without sign extension
      "srdi    %[carry],   %[b],        32\n\t"
      "subi    %[off_aux], %[off_aux],  4\n\t"
      "subi    %[len],     %[len],      4\n\t"
      "bdnz    LOOP_MULADD\n\t"
      "SKIP_MULADD:\n\t"
      // end<mulAdd>

      // begin<addOne>
      // off_aux = out_len*4 - 4 - mlen - offset*4 - 4;
      "addi    %[mlen],    %[i_minus1], 4\n\t"
      "sldi    %[a],       %[out_len],  2\n\t"
      "subi    %[a],       %[a],        4\n\t"
      "sub     %[a],       %[a],        %[mlen]\n\t"
      "subi    %[off_aux], %[offset],   4\n\t"
      "sub     %[off_aux], %[a],        %[off_aux]\n\t"

      "lwzx    %[b],       %[off_aux],  %[out]\n\t"
      "add     %[b],       %[b],        %[carry]\n\t"
      "stwx    %[b],       %[off_aux],  %[out]\n\t"

      // if (((uint64_t)s >> 32) != 0) {
      "srdi.   %[a],   %[b],        32\n\t"
      "beq     SKIP_ADDONE\n\t"

      // while (--mlen >= 0) {
      "LOOP_ADDONE:\n\t"
      "subi    %[mlen],    %[mlen],     4\n\t"
      "cmpwi   %[mlen],    0\n\t"
      "beq     SKIP_ADDONE\n\t"

      // if (--offset_aux < 0) { // Carry out of number
      "subi    %[off_aux], %[off_aux],  4\n\t"
      "cmpwi   %[off_aux], 0\n\t"
      "blt     SKIP_ADDONE\n\t"

      // } else {
      "lwzx    %[b],       %[off_aux],  %[out]\n\t"
      "addi    %[b],       %[b],        1\n\t"
      "stwx    %[b],       %[off_aux],  %[out]\n\t"
      "cmpwi   %[b],       0\n\t"
      "bne     SKIP_ADDONE\n\t"
      "b       LOOP_ADDONE\n\t"

      "SKIP_ADDONE:\n\t"
      // end<addOne>

      "\n\t"
      "addi    %[offset],  %[offset],   8\n\t"
      "subi    %[i_minus1],%[i_minus1], 4\n\t"
      "cmpwi   %[i_minus1],0\n\t"
      "bge     LOOP_DIAGONAL_SUM\n\t"
      "\n\t"
      "SKIP_DIAGONAL_SUM:\n\t"
      : /* output / temporaries */
        [carry] "=&b" (carry),
        [a] "=&b" (a),
        [b] "=&b" (b),
        [t] "=&b" (t),
        [off_aux] "=&b" (off_aux),
        [i_minus1] "=&b" (i_minus1),
        [mlen] "=&b" (mlen),
        [offset] "=&b" (offset),
        [len] "=&b" (len)
      : /* input */
        [in] "b" (in),
        [in_len] "b" (in_len),
        [out] "b" (out),
        [out_len] "b" (out_len)
      : /* clobber list */
        "memory", "ctr", "cr0"
      );

  int i, c, cs;
  asm volatile (
      // Shift back up and set low bit
      // Shifts 1 bit left up to len positions. Assumes no leading zeros
      // begin<primitiveLeftShift>
      "cmpwi   %[out_len], 0\n\t"
      "ble     SKIP_LSHIFT\n\t"
      "li      %[i],       0\n\t"
      "lwz     %[c],       0(%[out])\n\t"
      "subi    %[b],       %[out_len],  1\n\t"
      "mtctr   %[b]\n\t"
      "\n\t"
      "LOOP_LSHIFT:\n\t"
      "mr      %[b],       %[c]\n\t"
      "addi    %[cs],      %[i],        4\n\t"
      "lwzx    %[c],       %[out],      %[cs]\n\t"

      "sldi    %[b],       %[b],        1\n\t"
      "srwi    %[cs],      %[c],        31\n\t"
      "or      %[b],       %[b],        %[cs]\n\t"
      "stwx    %[b],       %[i],        %[out]\n\t"

      "addi    %[i],       %[i],        4\n\t"
      "bdnz    LOOP_LSHIFT\n\t"

      "sldi    %[c],       %[out_len],  2\n\t"
      "subi    %[c],       %[c],        4\n\t"
      "lwzx    %[b],       %[out],      %[c]\n\t"
      "sldi    %[b],       %[b],        1\n\t"
      "stwx    %[b],       %[out],      %[c]\n\t"

      "SKIP_LSHIFT:\n\t"
      // end<primitiveLeftShift>

      // Set low bit
      "sldi    %[i],       %[in_len],   2\n\t"
      "subi    %[i],       %[i],        4\n\t"
      "lwzx    %[i],       %[in],       %[i]\n\t"
      "sldi    %[c],       %[out_len],  2\n\t"
      "subi    %[c],       %[c],        4\n\t"
      "lwzx    %[b],       %[out],      %[c]\n\t"

      "andi.   %[i],       %[i],        1\n\t"
      "or      %[i],       %[b],        %[i]\n\t"

      "stwx    %[i],       %[out],      %[c]\n\t"
      : /* output / temporaries */
        [i] "=&b" (i),
        [b] "=&b" (b),
        [c] "=&b" (c),
        [cs] "=&b" (cs)
      : /* input */
        [in] "b" (in),
        [in_len] "b" (in_len),
        [out] "b" (out),
        [out_len] "b" (out_len)
      : /* clobber list */
        "memory", "ctr", "cr0"
      );

#else

  const int64_t LONG_MASK = 0xffffffffL;

  // Store the squares, right shifted one bit (i.e., divided by 2)
  int lastProductLowWord = 0;
  for (int j=0, i=0; j < in_len; j++) {
    long piece = (in[j] & LONG_MASK);
    long product = piece * piece;
    out[i++] = (lastProductLowWord << 31) | (int)((uint64_t)product >> 33);
    out[i++] = (int)(((uint64_t)product) >> 1);
    lastProductLowWord = (int)product;
  }

  // Add in off-diagonal sums
  for (int i=in_len, offset=1; i > 0; i--, offset+=2) {
    int t = in[i-1];
    //t = mulAdd(out, in, offset, i-1, t);
    //  static int mulAdd(int[] out, int[] in, int offset, int len, int t)
    {
      int64_t kLong = t & LONG_MASK;
      int64_t carry = 0;
      int offset_aux = offset;

      offset_aux = out_len - offset_aux - 1;
      for (int j=(i-1)-1; j >= 0; j--) {
        int64_t product = (in[j] & LONG_MASK) * kLong + (out[offset_aux] & LONG_MASK) + carry;
        out[offset_aux--] = (int32_t)(product);
        carry = ((uint64_t)product) >> 32;
      }
      t = (int32_t)carry;
    } // mulAdd
    //addOne(out, offset-1, i, t);
    /**
     * Add one word to the number a mlen words into a. Return the resulting
     * carry.
     */
    //static int addOne(int* out, int offset_aux, int mlen, int carry)
    {
      int offset_aux = offset-1;
      int mlen = i;
      int carry = t;

      offset_aux = out_len-1-mlen-offset_aux;
      long s = (out[offset_aux] & LONG_MASK) + (carry & LONG_MASK);

      out[offset_aux] = (int)s;
      if (((uint64_t)s >> 32) != 0) {
        while (--mlen >= 0) {
          if (--offset_aux < 0) { // Carry out of number
            break;
          } else {
            out[offset_aux]++;
            if (out[offset_aux] != 0)
              break;
          }
        }
      }
    } // addOne
  }

  // Shift back up and set low bit
  //primitiveLeftShift(out, out_len, 1);
  // shifts a up to len left n bits assumes no leading zeros, 0<=n<32
  //static void primitiveLeftShift(int[] out, int out_len, int n)
  {
    if (out_len != 0) {
      for (int i=0, c=out[i], m=out_len-1; i < m; i++) {
        int b = c;
        c = out[i+1];
        out[i] = (b << 1) | ((c & LONG_MASK) >> 31);
      }
      out[out_len-1] <<= 1;
    }
  } // primitiveLeftShift

  out[out_len-1] |= in[in_len-1] & 1;

#endif

  return out;
}
