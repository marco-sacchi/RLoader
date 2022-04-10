#ifndef ISR_HPP
#define ISR_HPP

#ifdef __cplusplus
	#define __CPPARGS ...
#else
	#define __CPPARGS
#endif

class isr {
public:
	/** Interrupt service routine definition. */
	typedef void interrupt (far *isrptr_t)(__CPPARGS);

	/** Get pointer to specified interrupt service routine.
	 *
	 * @param int_num Interrupt number.
	 * @return Interrupt service routine address.
	 */
	static isrptr_t get(int int_num);

	/** Set new interrupt service routine for specified interrupt.
	 *
	 * @param int_num Interrupt number.
	 * @param new_isr New interrupt service routine address.
	 * @return Old interrupt service routine address.
	 */
	static isrptr_t set(int int_num, isrptr_t new_isr);
};

#endif
