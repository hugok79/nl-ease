#ifndef LOGIC_H
#define LOGIC_H

typedef struct {
    int enabled;
    int temperature;
    int start_hour;
    int end_hour;
    int use_12h;  // 0 = 24h, 1 = 12h 
} AppState;

void logic_init(void);
AppState *logic_get_state(void);

void logic_set_enabled(int enabled);
void logic_set_temperature(int temp);
void logic_set_schedule(int start_hour, int end_hour);
void logic_run_daemon(void);

void logic_apply(void);
void logic_save(void);
void logic_load(void);

#endif
