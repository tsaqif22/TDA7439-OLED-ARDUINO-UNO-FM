
#define DEBUG_LED       5
#define INFRARED_PIN    3

#define VOLUME_LEVEL    0
#define BASS_LEVEL      1
#define MID_LEVEL       2
#define TREBLE_LEVEL    3
#define SELECT_INPUT    4
#define FM_FREQUENCY    5
#define INPUT_GAIN      6
#define BRIGHTNESS      7

const unsigned int ENCODER_CLK     = 0;
const unsigned int ENCODER_DT      = 1;
const unsigned int ENCODER_SW      = 2;
const unsigned int ENCODER_STEPS   = 4;

char* MenuItems[7] = {"VOL ", "BASS", "TONE", "TREB", "CHNL", "FM", "GAIN"};
const unsigned int DEFAULT_MENU    = 0;

const unsigned int AUX_INPUT_ONE   = 1;
const unsigned int AUX_INPUT_TWO   = 2;
const unsigned int BLUETOOTH       = 3;
const unsigned int FM_RADIO        = 4;

char* InputChannels[5] = {"NA", "P1", "P2", "BT", "FM"};

const unsigned int MAX_MENU_ITEMS  = 6;
const unsigned int MAX_INPUTS      = 4;

const unsigned int MAX_START_VOLUME = 3;
const unsigned int MAX_VOLUME       = 48;
const unsigned int MENU_TIMEOUT     = 5000;

struct config_t {
    int activeInput;
    int volumeLevel;
    int bassLevel;
    int midLevel;
    int trebLevel;
    int attLevel;
    int gainLevel;
    double frequency;
    double station_1;
    double station_2;
    double station_3;
    double station_4;
    double station_5;
    double station_6;
    double station_7;
    double station_8;
    double station_9;
    
} configuration;


const double VIVIDH_BHARATHI  = 102.80;
const double RAINBOW          = 101.90;
const double MIRCHI           = 98.30;
const double RADIO_CITY       = 91.10;
const double BIG_FM           = 92.70;
const double RED_FM           = 93.50;
const double IGNOU            = 105.60;


#define CA_PAUSE          0x843501FE
#define CA_PLAY_NEXT      0x843502FD
#define CA_STOP           0x843503FC
#define CA_MENU           0x843504FB
#define CA_REWIND         0x843505FA
#define CA_PREVIOUS       0x843506F9
#define CA_FORWARD        0x843507F8
#define CA_MOUSE          0x843508F7
#define CA_KEY_ONE        0x843509F6
#define CA_KEY_TWO        0x84350AF5
#define CA_KEY_THREE      0x84350BF4
#define CA_MUTE           0x84350CF3
#define CA_KEY_FOUR       0x84350DF2
#define CA_KEY_FIVE       0x84350EF1
#define CA_KEY_SIX        0x84350FF0
#define CA_VOL_UP         0x843510EF
#define CA_KEY_SEVEN      0x843511EE
#define CA_KEY_EIGHT      0x843512ED
#define CA_KEY_NINE       0x843513EC
#define CA_VOL_DOWN       0x843514EB
#define CA_SHIFT          0x843515EA
#define CA_CANCEL         0x843516E9
#define CA_ENTER          0x843517E8
#define CA_SEARCH         0x843518E7
#define CA_REPEAT         0xFFFFFFFF


