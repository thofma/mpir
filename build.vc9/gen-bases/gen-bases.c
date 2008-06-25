/* Generate mp_bases data.

Copyright 1991, 1993, 1994, 1996, 2000, 2002, 2004 Free Software Foundation,
Inc.

This file is part of the GNU MP Library.

The GNU MP Library is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version.

The GNU MP Library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details.

You should have received a copy of the GNU Lesser General Public License
along with the GNU MP Library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
MA 02110-1301, USA. */

#include <math.h>

#include "dumbmp.c"


int    chars_per_limb;
double chars_per_bit_exactly;
mpz_t  big_base;
int    normalization_steps;
mpz_t  big_base_inverted;

mpz_t  t;

#define POW2_P(n)  (((n) & ((n) - 1)) == 0)

unsigned int
ulog2 (unsigned int x)
{
  unsigned int i;
  for (i = 0;  x != 0;  i++)
    x >>= 1;
  return i;
}

void
generate (int limb_bits, int nail_bits, int base)
{
  int  numb_bits = limb_bits - nail_bits;

  mpz_set_ui (t, 1L);
  mpz_mul_2exp (t, t, numb_bits);
  mpz_set_ui (big_base, 1L);
  chars_per_limb = 0;
  for (;;)
    {
      mpz_mul_ui (big_base, big_base, (long) base);
      if (mpz_cmp (big_base, t) > 0)
        break;
      chars_per_limb++;
    }

  chars_per_bit_exactly = 0.69314718055994530942 / log ((double) base);

  mpz_ui_pow_ui (big_base, (long) base, (long) chars_per_limb);

  normalization_steps = limb_bits - mpz_sizeinbase (big_base, 2);

  mpz_set_ui (t, 1L);
  mpz_mul_2exp (t, t, 2*limb_bits - normalization_steps);
  mpz_tdiv_q (big_base_inverted, t, big_base);
  mpz_set_ui (t, 1L);
  mpz_mul_2exp (t, t, limb_bits);
  mpz_sub (big_base_inverted, big_base_inverted, t);
}

void
header (int limb_bits, int nail_bits)
{
  int  numb_bits = limb_bits - nail_bits;

  generate (limb_bits, nail_bits, 10);

  printf ("/* This file generated by gen-bases.c - DO NOT EDIT. */\n");
  printf ("\n");
  printf ("#if GMP_NUMB_BITS != %d\n", numb_bits);
  printf ("#error, error, this data is for %d bits\n", numb_bits);
  printf ("#endif\n");
  printf ("\n");
  printf ("/* mp_bases[10] data, as literal values */\n");
  printf ("#define MP_BASES_CHARS_PER_LIMB_10      %d\n", chars_per_limb);
  printf ("#define MP_BASES_BIG_BASE_10            CNST_LIMB(0x");
  mpz_out_str (stdout, 16, big_base);
  printf (")\n");
  printf ("#define MP_BASES_BIG_BASE_INVERTED_10   CNST_LIMB(0x");
  mpz_out_str (stdout, 16, big_base_inverted);
  printf (")\n");
  printf ("#define MP_BASES_NORMALIZATION_STEPS_10 %d\n", normalization_steps);
}

void
table (int limb_bits, int nail_bits)
{
  int  numb_bits = limb_bits - nail_bits;
  int  base;

  printf ("/* This file generated by gen-bases.c - DO NOT EDIT. */\n");
  printf ("\n");
  printf ("#include \"gmp.h\"\n");
  printf ("#include \"gmp-impl.h\"\n");
  printf ("\n");
  printf ("#if GMP_NUMB_BITS != %d\n", numb_bits);
  printf ("#error, error, this data is for %d bits\n", numb_bits);
  printf ("#endif\n");
  printf ("\n");
  puts ("const struct bases mp_bases[257] =\n{");
  puts ("  /*   0 */ { 0, 0.0, 0 },");
  puts ("  /*   1 */ { 0, 1e37, 0 },");
  for (base = 2; base <= 256; base++)
    {
      generate (limb_bits, nail_bits, base);

      printf ("  /* %3u */ { ", base);
      if (POW2_P (base))
	{
          printf ("%u, %.16f, 0x%x },\n",
                  chars_per_limb, chars_per_bit_exactly, ulog2 (base) - 1);
	}
      else
	{
          printf ("%u, %.16f, CNST_LIMB(0x",
                  chars_per_limb, chars_per_bit_exactly);
	  mpz_out_str (stdout, 16, big_base);
          printf ("), CNST_LIMB(0x");
	  mpz_out_str (stdout, 16, big_base_inverted);
          printf (") },\n");
	}
    }

  puts ("};");
}

int
main (int argc, char **argv)
{
  int  limb_bits, nail_bits;

  mpz_init (big_base);
  mpz_init (big_base_inverted);
  mpz_init (t);

  if (argc != 4)
    {
      fprintf (stderr, "Usage: gen-bases <header|table> <limbbits> <nailbits>\n");
      exit (1);
    }

  limb_bits = atoi (argv[2]);
  nail_bits = atoi (argv[3]);

  if (limb_bits <= 0
      || nail_bits < 0
      || nail_bits >= limb_bits)
    {
      fprintf (stderr, "Invalid limb/nail bits: %d %d\n",
               limb_bits, nail_bits);
      exit (1);
    }

  if (strcmp (argv[1], "header") == 0)
    header (limb_bits, nail_bits);
  else if (strcmp (argv[1], "table") == 0)
    table (limb_bits, nail_bits);
  else
    {
      fprintf (stderr, "Invalid header/table choice: %s\n", argv[1]);
      exit (1);
    }

  return 0;
}

