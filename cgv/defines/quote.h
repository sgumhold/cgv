#pragma once
// helper define for QUOTE_SYMBOL_VALUE
#define QUOTE_SYMBOL_VALUE_(x) #x
/// this macro encloses the value of a defined symbol in double quotes as needed for a string constant
#define QUOTE_SYMBOL_VALUE(x) QUOTE_SYMBOL_VALUE_(x)
