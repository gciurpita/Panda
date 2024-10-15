#ifndef PINS_H
# define PINS_H

# define ADC_SLOPE    34
# define ADC_WHISTLE  35
# define ADC_REV      36
# define ADC_THR      39

# define Board1

#ifdef Board1
# define Enc_A_Clk    23
# define Enc_A_Dt     18
#else
# define Enc_A_Clk    18
# define Enc_A_Dt     23
#endif

# define Enc_B_Clk    32
# define Enc_B_Dt     26

# define KB_C0   4
# define KB_C1   0
# define KB_C2   2
# define KB_C3   15

# define KB_R0   13
# define KB_R1   12
# define KB_R2   14
# define KB_R3   27

# define LED     5

#endif
