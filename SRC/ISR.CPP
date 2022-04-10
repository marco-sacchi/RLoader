#include <mem.h>
#include <dos.h>

#include "isr.hpp"

isr::isrptr_t isr::get(int int_num) {
	return (isr::isrptr_t)getvect(int_num);
}

isr::isrptr_t isr::set(int int_num, isrptr_t new_isr) {
	isr::isrptr_t old_isr = isr::get(int_num);
	setvect(int_num, new_isr);

	return old_isr;
}
