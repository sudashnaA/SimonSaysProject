typedef enum
    {
    START,
    SIMON,
    DISPLAY,
    PLAYING,
    SUCCESS,
    FAIL,
    TONE0,
    TONE1,
    TONE2,
    TONE3,
    WAITING,
    WAITINGLONG,
    BUTTONCHECK,
    BUTTONON,
    DISPLAY_SCORE,
    OFF
    } SP_STATE;

extern volatile SP_STATE state;
extern volatile SP_STATE nextstate;