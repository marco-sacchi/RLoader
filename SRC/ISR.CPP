#include <mem.h>
#include <dos.h>

#include "isr.hpp"

isr::isrptr_t isr::set(int int_num, isrptr_t new_isr) {
	isr::isrptr_t old_isr = getvect(int_num);
	setvect(int_num, new_isr);

	return old_isr;
}
