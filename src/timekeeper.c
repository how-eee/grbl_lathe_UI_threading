#include "timekeeper.h"

#define MIN_DEBOUNCE_TICS 10

volatile uint32_t overflow_offset;

// overflow interrupt for timekeeper - counts timer ticks beyond 16 bits
ISR(TIMER5_OVF_vect) {
  overflow_offset+=(1UL<<16); //use of offset saves calculation time because get_timer_tics is called more often then the overflow interrupt
}
// Compare Match A interrupt for Index pulse debouncing
ISR(TIMER5_COMPA_vect) {
  bit_false(TIMSK5,(1<<OCIE5A));									              // Disable Output compare A match interrupt, must be set at the receive of the next Index pulse
  if (index_pulse_active()){									                  // Index pulse is still active
    system_set_threading_exec_flag(EXEC_SPINDLE_INDEX_PULSE);		// Signal the receive of an index pulse
  }
}
// Compare Match B interrupt for Synchronization pulse debouncing
ISR(TIMER5_COMPB_vect) {
  bit_false(TIMSK5,(1<<OCIE5B));									              // Disable Output compare B match interrupt, must be set at the receive of the next Synchronization pulse
  if (sync_pulse_active())	{									                  // Synchronization pulse is still active
    system_set_threading_exec_flag(EXEC_PLANNER_SYNC_PULSE);		// Signal the receive of an Synchronization pulse
  }
}
// Debounce the receive of an index pulse by delayed checking if the pulse is still active
void debounce_index_pulse() {
  if (settings.debounce_tics==0) {							                  // No index pulse debouncing
    system_set_threading_exec_flag(EXEC_SPINDLE_INDEX_PULSE);	  // Signal the receive of an index pulse
  }
  else {
    OCR5A=(TCNT5+settings.debounce_tics);   // Setup the debounce delayed trigger, debounce_tics x 4 us
    bit_true(TIFR5,(1<<OCF5A));							// clear any pending OC5A interrupts
    bit_true(TIMSK5, 1<<OCIE5A);						// Enable Output compare A match interrupt
  }
}
// Debounce the receive of a  Synchronization pulse by delayed checking if the pulse is still active
void debounce_sync_pulse() {
  if (settings.debounce_tics==0){							               // No Synchronization pulse debouncing
    system_set_threading_exec_flag(EXEC_PLANNER_SYNC_PULSE); // Signal the receive of an synchronization pulse
  }
  else {
    OCR5B=(TCNT5+settings.debounce_tics);                    // Setup the debounce delayed trigger, debounce_tics x 4 us
    bit_true(TIFR5,(1<<OCF5B));							                 // clear any pending OC5B interrupts
    bit_true(TIMSK5, 1<<OCIE5B);						                 // Enable Output compare B match interrupt
  }
}
// Initialises Timer 5 for measuring time between pulses.
// Used for G33 spindle synchronization (threading)
void timekeeper_init() {
  TCCR5A =0;						                                     // timer outputs disconnected, no waveform generation.
  bit_true(TCCR5B,(1<<CS51) | (1<<CS50));                    // pre-scaler: 1/64 (4 microseconds per tick @ 16MHz)
  bit_true(TIMSK5, 1<<TOIE5);                                // enable overflow interrupt for couning longer periods
  timekeeper_reset();
}
// reset the timer
void timekeeper_reset(){
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
    TCNT5=0;					                                       // reset the counter
    overflow_offset = 0;		                                 // set the offset to zero
    bit_true(TIFR5,(1<<(TOV5 | OCF5A | OCF5B)));             // clear any pending interrupts (Overflow and compare A,B
  }
}
// calculate the timer and overflow tics
uint32_t get_timer_ticks() {
  uint32_t ticks;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE){                          // Code must not be interrupted when adding overflow counts
    ticks = ((uint32_t) TCNT5) + overflow_offset;				      // read the timer register and add the overflow offset
    if (bit_istrue(TIFR5,1<<TOV5))								            // if during reading and calculating, a new timer overflow occurred, read again and add 0x10000 to overflow_offset (no overflow will be processed)
    ticks = ((uint32_t) TCNT5) + overflow_offset + (1UL<<16); // read again and add 0x10000 to overflow_offset
  }
  return ticks;
}

