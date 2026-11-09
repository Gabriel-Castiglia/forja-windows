/**
  Random Password Generator -- the engine.
  Author: Gabriel Castiglia - 2026-11-09

  The same rules as password_gen.py (MyFolder\RandomPassGen), which stays the reference:

    - four character classes: lowercase, uppercase, digits, and the symbols  !@#$%&*+-=?
    - with "avoid ambiguous" on, 0 O 1 l I are removed from the letter and digit classes;
    - one character of each class is guaranteed, the rest is drawn from all four together,
      and the result is shuffled so the guaranteed ones land anywhere.

  The randomness comes from BCryptGenRandom (the system CSPRNG, the same source Python's
  `secrets` reads on Windows). Draws use rejection sampling, so every character of a class
  is exactly as likely as any other: a plain "x % n" would favour the first few.
**/

#ifndef PWGEN_H
#define PWGEN_H

#define PW_MIN_LEN   4      /* one per class; password_gen.py also never gives fewer */
#define PW_MAX_LEN   128

#define PW_SYMBOLS   "!@#$%&*+-=?"
#define PW_AMBIGUOUS "0O1lI|`'\""

/* Number of distinct characters a position can take with this setting
 * (73 with everything, 68 with the ambiguous ones removed). */
int pw_pool_size(int avoidAmbiguous);

/* Writes `length` characters and a terminating zero into out[length + 1].
 * `length` is clamped to [PW_MIN_LEN, PW_MAX_LEN]. Returns the length written,
 * or 0 if the system random number generator failed (out is then emptied). */
int pw_generate(int length, int avoidAmbiguous, char* out);

/* About how many bits of entropy: length * log2(pool). */
double pw_entropy_bits(int length, int avoidAmbiguous);

#endif
