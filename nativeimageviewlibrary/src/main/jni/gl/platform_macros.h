//
// Created by Semyon on 10.01.2016.
//

#if defined(__GNUC__)
#define PRINTF_ATTRIBUTE(format_pos, arg_pos) __attribute__((format(printf, format_pos, arg_pos)))
#else
#define PRINTF_ATTRIBUTE(format_pos, arg_pos)
#endif