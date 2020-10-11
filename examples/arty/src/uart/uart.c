#define UART_CLK_MHZ 25.0
#define UART_CLKS_PER_SEC (UART_CLK_MHZ*1000000.0)
#define SEC_PER_UART_CLK (1.0/UART_CLKS_PER_SEC)

#define UART_BAUD 115200
#define UART_WORD_BITS 8
#define uart_word_t uint8_t
#define uart_bit_count_t uint4_t
#define uart_word_from_bits uint1_array8_le // PipelineC built in func
#define UART_SEC_PER_BIT (1.0/UART_BAUD)
#define UART_CLKS_PER_BIT_FLOAT (UART_SEC_PER_BIT/SEC_PER_UART_CLK)
#define UART_CLKS_PER_BIT ((uart_clk_count_t)UART_CLKS_PER_BIT_FLOAT)
#define UART_CLKS_PER_BIT_DIV2 ((uart_clk_count_t)(UART_CLKS_PER_BIT_FLOAT/2.0))
#define uart_clk_count_t uint16_t
#define UART_IDLE 1
#define UART_START 0
#define UART_STOP UART_IDLE

// Globally visible port/wire names
uint1_t uart_data_in;
uint1_t uart_data_out; 
// This should be in a macro somehow TODO \/
#include "uint1_t_array_N_t.h"
#include "uart_data_in_clock_crossing.h"
#include "uart_data_out_clock_crossing.h"

// Delcare as top level module io and connect to internal wires
MAIN_MHZ(uart_module,UART_CLK_MHZ)
uint1_t uart_module_data_in_regs[2];
uint1_t uart_module(uint1_t data_in)
{
  // Double register async input signal
  uint1_t data_in_registered = uart_module_data_in_regs[1];
  uart_module_data_in_regs[1] = uart_module_data_in_regs[0];
  uart_module_data_in_regs[0] = data_in;
  // Connect to ports  
  WIRE_WRITE(uint1_t, uart_data_in, data_in_registered)
  uint1_t data_out;
  WIRE_READ(uint1_t, data_out, uart_data_out)
  return data_out;
}
