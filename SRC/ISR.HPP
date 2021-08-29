#ifndef ISR_HPP
#define ISR_HPP

#ifdef __cplusplus
	#define __CPPARGS ...
#else
	#define __CPPARGS
#endif

class isr {
public:
	typedef void interrupt (*isrptr_t)(__CPPARGS);

	static isrptr_t set(int int_num, isrptr_t new_isr);
};

#endif
