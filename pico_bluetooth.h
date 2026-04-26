#ifndef PICO_BLUETOOTH_H_
#define PICO_BLUETOOTH_H_

void bluetooth_init(void);

void bluetooth_run(void);

void mpc_trigger_loop(void);
void sp404_trigger_loop(void);

#endif  // PICO_BLUETOOTH_H_