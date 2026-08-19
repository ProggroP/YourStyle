#include <pebble.h>
#include "main.h"

static Window *s_window;
static Layer *s_hands_layer, *s_dot_layer;
static GPath *small_sign, *medium_sign, *huge_sign, *sign;

typedef struct ClaySettings {
  GColor backcolor;
  GColor hourdialcolor;
  GColor minsdialcolor;
  GColor secsdialcolor;
  GColor minsmarkcolor;
  GColor hourmarkcolor;
  int fullscreen;
  int showsecond;
  int showminsmark;
  int showhourmark;
  int hourlength;
  int minslength;
  int secslength;
  int hourbalancelength;
  int minsbalancelength;
  int secsbalancelength;
  int hourthickness;
  int minsthickness;
  int secsthickness;
  int hourmarkinnerpos;
  int hourmarkouterpos;
  int minsmarkinnerpos;
  int minsmarkouterpos;
  int hourmarkthickness;
  int minsmarkthickness;
  GColor hourfillcolor;
  GColor minsfillcolor;
  GColor secsfillcolor;
  int hourfillthickness;
  int minsfillthickness;
  int secsfillthickness;
  int hourfillinnerpos;
  int hourfillouterpos;
  int minsfillinnerpos;
  int minsfillouterpos;
  int secsfillinnerpos;
  int secsfillouterpos;
  int date;
  int dateshape;
  int dateposdegree;
  int datesize;
  GColor datetextcolor;
  GColor datebackcolor;
  GColor datebordercolor;
  int datepositionrim;
  int bluetooth;
  int btshape; 
  int btposdegree;
  int btsize;
  GColor btcolor;
  GColor btbackcolor;
  GColor btbordercolor;
  int btpositionrim;
  int btalways;
  int btvibe;
  GColor btoffcolor;
  GColor btoffbackcolor;
  GColor btoffbordercolor;
  int battery;
  int battshape; 
  int battposdegree;
  int battsize;
  GColor battcolor;
  GColor battbackcolor;
  GColor battbordercolor;
  int battpositionrim;
  int battvibe;
  int battwarnlevel;
  int battwarninterval;
  GColor battwarncolor;
  GColor battwarnbackcolor;
  GColor battwarnbordercolor;  
  int shownumbers;
  int numberstyle;
  int numberfont;
  int numberpos;
  int numberset;
  GColor numbercolor;
  int minutemarkstyle;
  int hourmarkstyle;
  int hourmarkrim;
  int minsmarkrim;
  int numberrim;
  int showmoonphase;
  int moonposdegree;
  int moonpositionrim;
  int moonradius;
  GColor moonlightcolor;
  GColor moondarkcolor;
  int moonborder;
  GColor moonbordercolor;
  int moonhemisphere;
  int showsteps;
  int stepsstyle;
  int stepsposdegree;
  int stepspositionrim;
  int stepssize;
  int stepsshape;
  GColor stepslabelcolor;
  GColor stepsvaluecolor;
  GColor stepstextbackcolor;
  GColor stepstextbordercolor;
  int stepsdialradius;
  int stepsneedlelength;
  int stepsneedlebalancelength;
  int stepsneedlethickness;
  GColor stepsneedlecolor;
  GColor stepsfillcolor;
  int stepsfillthickness;
  int stepsfillinnerpos;
  int stepsfillouterpos;
  int stepsdialborder;
  int stepsdialborderthickness;
  GColor stepsdialbordercolor;
  GColor stepsmarkcolor;
  int stepsmarkthickness;
  int stepsmarkinnerpos;
  int stepsmarkouterpos;
  int screenoffsety;
  int screenoffsetx;
  // Settings are persisted as a raw byte dump of this struct, so field order
  // is part of the storage format: only append here, never insert above, or
  // every already-saved configuration is read back shifted.
  int hourneedlestart;
  int minsneedlestart;
  int secsneedlestart;
  int discretehands;
  int stepsontop;
  int stepsneedlestart;
  int stepssegment;
  int showcenterhub;
  int centerhubauto;
  int showcenteraxis;
  GColor centerhubcolor;
  GColor centeraxiscolor;
} ClaySettings;

// An instance of the struct
static ClaySettings settings;

// Persistent storage key
#define SETTINGS_KEY 1
#define SETTINGS_KEY2 2

// Pebble's persist_write_data/persist_read_data allow at most 256 bytes per
// key (PERSIST_DATA_MAX_LENGTH). The settings struct has grown past that,
// so it is split into two chunks stored under separate keys.
//
// This offset is a fixed part of the storage format, not sizeof/2: deriving
// it from the struct size would move the boundary whenever a field is added,
// and every already-saved second chunk would be read back at the wrong offset.
#define SETTINGS_SPLIT 196

_Static_assert(SETTINGS_SPLIT <= PERSIST_DATA_MAX_LENGTH, "first settings chunk exceeds persist limit");
_Static_assert(sizeof(ClaySettings) - SETTINGS_SPLIT <= PERSIST_DATA_MAX_LENGTH, "second settings chunk exceeds persist limit");

// Clay delivers every setting in a single dictionary. With ~130 keys that is
// roughly 1.4 kB, and an inbox even slightly too small makes the firmware drop
// the whole message -- no setting arrives at all, rather than just the last few.
// The watchface never sends anything back, so the outbox only needs to be
// non-zero.
//
// The inbox is requested from the app heap, and aplite only has 24 kB of RAM
// in total: there the roomy buffer fails outright with APP_MSG_OUT_OF_MEMORY
// and no setting would ever arrive. So try progressively smaller buffers and
// keep the largest one the platform actually grants. The smallest entry must
// stay above the real dictionary size (see APP_MSG_DICT_BYTES) or settings
// break silently on that platform.
#define APP_MSG_OUTBOX_SIZE 64
#define APP_MSG_DICT_BYTES  1400

static const uint16_t APP_MSG_INBOX_SIZES[] = { 2048, 1792, 1600, 1472 };

// The moon's lit half and its terminator are filled polygons approximating a
// circle. The segment counts are sized for the largest moon the config allows
// (radius 130, a full-screen moon on gabbro): at 130 px both keep the polygon
// within ~0.3 px of a true circle, so the edge still reads as round.
#define MOON_HALF_SEGMENTS    24
#define MOON_ELLIPSE_SEGMENTS 48
#define MOON_RADIUS_MAX      130

// Initialize the default settings
static void default_settings() {
  settings.backcolor = GColorWhite;
  settings.hourdialcolor = GColorBlack;
  settings.minsdialcolor = GColorBlack;
  settings.secsdialcolor = GColorBlack;
  settings.minsmarkcolor = GColorBlack;
  settings.hourmarkcolor = GColorBlack;
  settings.fullscreen    = 1;
  settings.discretehands = 0;
  settings.showsecond    = 0;
  settings.showminsmark  = 1;
  settings.showhourmark  = 1;
  settings.hourlength    = 65;
  settings.minslength    = 85;
  settings.secslength    = 90;
  settings.hourbalancelength = 0;
  settings.minsbalancelength = 0;
  settings.secsbalancelength = 20;
  settings.hourneedlestart = 0;
  settings.minsneedlestart = 0;
  settings.secsneedlestart = 0;
  settings.hourthickness = 9;
  settings.minsthickness = 7;
  settings.secsthickness = 5;
  settings.hourmarkinnerpos = 85;
  settings.hourmarkouterpos = 100;
  settings.minsmarkinnerpos = 90;
  settings.minsmarkouterpos = 100;
  settings.hourmarkthickness = 5;
  settings.minsmarkthickness = 3;
  settings.hourfillcolor = GColorBlack;
  settings.minsfillcolor = GColorBlack;
  settings.secsfillcolor = GColorBlack;
  settings.hourfillthickness = 7;
  settings.minsfillthickness = 5;
  settings.secsfillthickness = 3;
  settings.hourfillinnerpos  = 50;
  settings.hourfillouterpos  = 62;
  settings.minsfillinnerpos  = 70;
  settings.minsfillouterpos  = 82;
  settings.secsfillinnerpos  = 76;
  settings.secsfillouterpos  = 88; 
  settings.date              = 0;
  settings.dateshape         = 0;  
  settings.dateposdegree     = 90;
  settings.datesize          = 1;
  settings.datetextcolor     = GColorWhite;
  settings.datebackcolor     = GColorBlack;
  settings.datebordercolor   = GColorBlack;
  settings.datepositionrim   = 80;
  settings.bluetooth         = 0;
  settings.btshape           = 0;  
  settings.btposdegree       = 180;
  settings.btsize            = 1;
  settings.btcolor           = GColorWhite;
  settings.btbackcolor       = GColorWhite;
  settings.btbordercolor     = GColorWhite;
  settings.btpositionrim     = 80;
  settings.btalways          = 1;
  settings.btvibe            = 0;  
  settings.btoffcolor        = GColorBlack;
  settings.btoffbackcolor    = GColorWhite;
  settings.btoffbordercolor  = GColorBlack;
  settings.battery           = 0;
  settings.battshape         = 0;  
  settings.battposdegree     = 270;
  settings.battsize          = 1;
  settings.battcolor         = GColorBlack;
  settings.battbackcolor     = GColorWhite;
  settings.battbordercolor   = GColorBlack;
  settings.battpositionrim   = 80;
  settings.battvibe          = 0;  
  settings.battwarncolor     = GColorBlack;
  settings.battwarnbackcolor = GColorWhite;
  settings.battwarnbordercolor = GColorBlack;
  settings.battwarnlevel     = 30;
  settings.battwarninterval  = 30;
  settings.shownumbers       = 0;
  settings.numberstyle       = 0;
  settings.numberfont        = 1;
  settings.numberpos         = 60;
  settings.numberset         = 0;
  settings.numbercolor       = GColorBlack;
  settings.minutemarkstyle   = 0;
  settings.hourmarkstyle     = 0;
  settings.hourmarkrim       = 0;
  settings.minsmarkrim       = 0;
  settings.numberrim         = 0;
  settings.showmoonphase     = 0;
  settings.moonposdegree     = 45;
  settings.moonpositionrim   = 50;
  settings.moonradius        = 15;
  settings.moonlightcolor    = GColorWhite;
  settings.moondarkcolor     = GColorBlack;
  settings.moonborder        = 1;
  settings.moonbordercolor   = GColorBlack;
  settings.moonhemisphere    = 0;
  settings.showsteps              = 0;
  settings.stepsstyle             = 0;
  settings.stepsontop             = 0;
  settings.stepsposdegree         = 135;
  settings.stepspositionrim       = 50;
  settings.stepssize              = 1;
  settings.stepsshape             = 0;
  settings.stepslabelcolor        = GColorBlack;
  settings.stepsvaluecolor        = GColorBlack;
  settings.stepstextbackcolor     = GColorWhite;
  settings.stepstextbordercolor   = GColorBlack;
  settings.stepsdialradius        = 35;
  settings.stepsneedlelength      = 70;
  settings.stepsneedlebalancelength = 0;
  settings.stepsneedlestart       = 0;
  settings.stepsneedlethickness   = 5;
  settings.stepsneedlecolor       = GColorBlack;
  settings.stepsfillcolor         = GColorBlack;
  settings.stepsfillthickness     = 3;
  settings.stepsfillinnerpos      = 50;
  settings.stepsfillouterpos      = 62;
  settings.stepsdialborder        = 1;
  settings.stepsdialborderthickness = 2;
  settings.stepsdialbordercolor   = GColorBlack;
  settings.stepsmarkcolor         = GColorBlack;
  settings.stepsmarkthickness     = 3;
  settings.stepsmarkinnerpos      = 80;
  settings.stepsmarkouterpos      = 100;
  settings.screenoffsety          = 0;
  settings.screenoffsetx          = 0;
  settings.stepssegment           = 0;
  settings.showcenterhub          = 1;
  settings.centerhubauto          = 1;
  settings.showcenteraxis         = 1;
  settings.centerhubcolor         = GColorBlack;
  settings.centeraxiscolor        = GColorDarkGray;
}

// Read settings from persistent storage
static void load_settings() {
  // Load the default settings
  default_settings();

  // Read settings from persistent storage, if they exist.
  // Split across two keys since one blob would exceed the 256 byte cap.
  uint8_t *raw = (uint8_t *)&settings;
  persist_read_data(SETTINGS_KEY, raw, SETTINGS_SPLIT);
  persist_read_data(SETTINGS_KEY2, raw + SETTINGS_SPLIT, sizeof(settings) - SETTINGS_SPLIT);
}

// Save the settings to persistent storage
static void save_settings() {
  uint8_t *raw = (uint8_t *)&settings;
  persist_write_data(SETTINGS_KEY, raw, SETTINGS_SPLIT);
  persist_write_data(SETTINGS_KEY2, raw + SETTINGS_SPLIT, sizeof(settings) - SETTINGS_SPLIT);
}

static void handle_time_tick(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(window_get_root_layer(s_window));
}

static void in_received_handler(DictionaryIterator *iter, void *context) {
 
  // read background color setting
  Tuple *backcolor_conf = dict_find(iter, MESSAGE_KEY_backcolor);
  if ( backcolor_conf ) {
    settings.backcolor = GColorFromHEX(backcolor_conf->value->int32);
  }

  // read hour dial color setting
  Tuple *hourdialcolor_conf = dict_find(iter, MESSAGE_KEY_hourdialcolor);
  if ( hourdialcolor_conf ) {
    settings.hourdialcolor = GColorFromHEX(hourdialcolor_conf->value->int32);
  }
  
  // read mins dial color setting
  Tuple *minsdialcolor_conf = dict_find(iter, MESSAGE_KEY_minsdialcolor);
  if ( minsdialcolor_conf ) {
    settings.minsdialcolor = GColorFromHEX(minsdialcolor_conf->value->int32);
  }
  
  // read secs dial color setting
  Tuple *secsdialcolor_conf = dict_find(iter, MESSAGE_KEY_secsdialcolor);
  if ( secsdialcolor_conf ) {
    settings.secsdialcolor = GColorFromHEX(secsdialcolor_conf->value->int32);
  }
  
  // read mins mark color setting
  Tuple *minsmarkcolor_conf = dict_find(iter, MESSAGE_KEY_minsmarkcolor);
  if ( minsmarkcolor_conf ) {
    settings.minsmarkcolor = GColorFromHEX(minsmarkcolor_conf->value->int32);
  }
  
  // read hour mark color setting
  Tuple *hourmarkcolor_conf = dict_find(iter, MESSAGE_KEY_hourmarkcolor);
  if ( hourmarkcolor_conf ) {
    settings.hourmarkcolor = GColorFromHEX(hourmarkcolor_conf->value->int32);
  }
  
  // read show second setting
  Tuple *showsecond_conf = dict_find(iter, MESSAGE_KEY_showsecond);
  if( showsecond_conf ) { 
    settings.showsecond = showsecond_conf->value->int32;
  }
  
  // read fullscreen setting
  Tuple *fullscreen_conf = dict_find(iter, MESSAGE_KEY_fullscreen);
  if( fullscreen_conf ) {
    settings.fullscreen = fullscreen_conf->value->int32;
  }

  // read discrete (6-degree stepped) hands setting
  Tuple *discretehands_conf = dict_find(iter, MESSAGE_KEY_discretehands);
  if( discretehands_conf ) {
    settings.discretehands = discretehands_conf->value->int32;
  }

  // read show minute mark setting
  Tuple *showminsmark_conf = dict_find(iter, MESSAGE_KEY_showminsmark);
  if( showminsmark_conf ) { 
    settings.showminsmark = showminsmark_conf->value->int32;
  }
  
  // read show hour mark setting
  Tuple *showhourmark_conf = dict_find(iter, MESSAGE_KEY_showhourmark);
  if( showhourmark_conf ) { 
    settings.showhourmark = showhourmark_conf->value->int32;
  }

  // read hour length setting
  Tuple *hourlength_conf = dict_find(iter, MESSAGE_KEY_hourlength);
  if( hourlength_conf ) { 
    settings.hourlength = hourlength_conf->value->int32;
  } 

  // read mins length setting
  Tuple *minslength_conf = dict_find(iter, MESSAGE_KEY_minslength);
  if( minslength_conf ) { 
    settings.minslength = minslength_conf->value->int32;
  } 

  // read mins length setting
  Tuple *secslength_conf = dict_find(iter, MESSAGE_KEY_secslength);
  if( secslength_conf ) { 
    settings.secslength = secslength_conf->value->int32;
  } 

    // read hour balance length setting
  Tuple *hourbalancelength_conf = dict_find(iter, MESSAGE_KEY_hourbalancelength);
  if( hourbalancelength_conf ) { 
    settings.hourbalancelength = hourbalancelength_conf->value->int32;
  } 

  // read mins balance length setting
  Tuple *minsbalancelength_conf = dict_find(iter, MESSAGE_KEY_minsbalancelength);
  if( minsbalancelength_conf ) { 
    settings.minsbalancelength = minsbalancelength_conf->value->int32;
  } 

  // read mins length setting
  Tuple *secsbalancelength_conf = dict_find(iter, MESSAGE_KEY_secsbalancelength);
  if( secsbalancelength_conf ) {
    settings.secsbalancelength = secsbalancelength_conf->value->int32;
  }

  // read hour needle start setting (floating needle, e.g. a dot indicator)
  Tuple *hourneedlestart_conf = dict_find(iter, MESSAGE_KEY_hourneedlestart);
  if( hourneedlestart_conf ) {
    settings.hourneedlestart = hourneedlestart_conf->value->int32;
  }

  // read minute needle start setting
  Tuple *minsneedlestart_conf = dict_find(iter, MESSAGE_KEY_minsneedlestart);
  if( minsneedlestart_conf ) {
    settings.minsneedlestart = minsneedlestart_conf->value->int32;
  }

  // read seconds needle start setting
  Tuple *secsneedlestart_conf = dict_find(iter, MESSAGE_KEY_secsneedlestart);
  if( secsneedlestart_conf ) {
    settings.secsneedlestart = secsneedlestart_conf->value->int32;
  }

  // read hour hand thickness setting
  Tuple *hourthickness_conf = dict_find(iter, MESSAGE_KEY_hourthickness);
  if( hourthickness_conf ) {
    settings.hourthickness = hourthickness_conf->value->int32;
  } 

  // read minute hand thickness setting
  Tuple *minsthickness_conf = dict_find(iter, MESSAGE_KEY_minsthickness);
  if( minsthickness_conf ) {
    settings.minsthickness = minsthickness_conf->value->int32;
  } 
  
  // read seconds hand thickness setting
  Tuple *secsthickness_conf = dict_find(iter, MESSAGE_KEY_secsthickness);
  if( secsthickness_conf ) {
    settings.secsthickness = secsthickness_conf->value->int32;
  } 

  // read hour mark inner position setting
  Tuple *hourmarkinnerpos_conf = dict_find(iter, MESSAGE_KEY_hourmarkinnerpos);
  if( hourmarkinnerpos_conf ) {
    settings.hourmarkinnerpos = hourmarkinnerpos_conf->value->int32;
  } 
  
  // read hour mark outer position setting
  Tuple *hourmarkouterpos_conf = dict_find(iter, MESSAGE_KEY_hourmarkouterpos);
  if( hourmarkouterpos_conf ) {
    settings.hourmarkouterpos = hourmarkouterpos_conf->value->int32;
  } 

  // read hour mark "reach case rim" setting
  Tuple *hourmarkrim_conf = dict_find(iter, MESSAGE_KEY_hourmarkrim);
  if( hourmarkrim_conf ) {
    settings.hourmarkrim = hourmarkrim_conf->value->int32;
  } 

  // read minute mark "reach case rim" setting
  Tuple *minsmarkrim_conf = dict_find(iter, MESSAGE_KEY_minsmarkrim);
  if( minsmarkrim_conf ) {
    settings.minsmarkrim = minsmarkrim_conf->value->int32;
  } 

  // read hour number "reach case rim" setting
  Tuple *numberrim_conf = dict_find(iter, MESSAGE_KEY_numberrim);
  if( numberrim_conf ) {
    settings.numberrim = numberrim_conf->value->int32;
  } 

  // read moon phase settings
  Tuple *showmoonphase_conf = dict_find(iter, MESSAGE_KEY_showmoonphase);
  if( showmoonphase_conf ) {
    settings.showmoonphase = showmoonphase_conf->value->int32;
  } 

  Tuple *moonposdegree_conf = dict_find(iter, MESSAGE_KEY_moonposdegree);
  if( moonposdegree_conf ) {
    settings.moonposdegree = moonposdegree_conf->value->int32;
  } 

  Tuple *moonpositionrim_conf = dict_find(iter, MESSAGE_KEY_moonpositionrim);
  if( moonpositionrim_conf ) {
    settings.moonpositionrim = moonpositionrim_conf->value->int32;
  } 

  Tuple *moonradius_conf = dict_find(iter, MESSAGE_KEY_moonradius);
  if( moonradius_conf ) {
    settings.moonradius = moonradius_conf->value->int32;
    // guard the fixed-size polygon buffers' accuracy assumption
    if( settings.moonradius > MOON_RADIUS_MAX ) {
      settings.moonradius = MOON_RADIUS_MAX;
    }
  }

  Tuple *moonlightcolor_conf = dict_find(iter, MESSAGE_KEY_moonlightcolor);
  if( moonlightcolor_conf ) {
    settings.moonlightcolor = GColorFromHEX(moonlightcolor_conf->value->int32);
  } 

  Tuple *moondarkcolor_conf = dict_find(iter, MESSAGE_KEY_moondarkcolor);
  if( moondarkcolor_conf ) {
    settings.moondarkcolor = GColorFromHEX(moondarkcolor_conf->value->int32);
  } 

  Tuple *moonborder_conf = dict_find(iter, MESSAGE_KEY_moonborder);
  if( moonborder_conf ) {
    settings.moonborder = moonborder_conf->value->int32;
  } 

  Tuple *moonbordercolor_conf = dict_find(iter, MESSAGE_KEY_moonbordercolor);
  if( moonbordercolor_conf ) {
    settings.moonbordercolor = GColorFromHEX(moonbordercolor_conf->value->int32);
  } 

  Tuple *moonhemisphere_conf = dict_find(iter, MESSAGE_KEY_moonhemisphere);
  if( moonhemisphere_conf ) {
    settings.moonhemisphere = moonhemisphere_conf->value->int32;
  } 

  // read steps settings
  Tuple *showsteps_conf = dict_find(iter, MESSAGE_KEY_showsteps);
  if( showsteps_conf ) {
    settings.showsteps = showsteps_conf->value->int32;
  } 

  char stepsbuf[8];
  Tuple *stepsstyle_conf = dict_find(iter, MESSAGE_KEY_stepsstyle);
  if( stepsstyle_conf ) {
    strcpy( stepsbuf, stepsstyle_conf->value->cstring);
    settings.stepsstyle = atoi(stepsbuf);
  }

  // read which layer the steps display sits on:
  // 0 = above the marks but below the hands, 1 = above everything,
  // 2 = underneath the hour/minute marks
  Tuple *stepsontop_conf = dict_find(iter, MESSAGE_KEY_stepsontop);
  if( stepsontop_conf ) {
    strcpy( stepsbuf, stepsontop_conf->value->cstring);
    settings.stepsontop = atoi(stepsbuf);
  }

  Tuple *stepsposdegree_conf = dict_find(iter, MESSAGE_KEY_stepsposdegree);
  if( stepsposdegree_conf ) {
    settings.stepsposdegree = stepsposdegree_conf->value->int32;
  } 

  Tuple *stepspositionrim_conf = dict_find(iter, MESSAGE_KEY_stepspositionrim);
  if( stepspositionrim_conf ) {
    settings.stepspositionrim = stepspositionrim_conf->value->int32;
  } 

  Tuple *stepssize_conf = dict_find(iter, MESSAGE_KEY_stepssize);
  if( stepssize_conf ) {
    strcpy( stepsbuf, stepssize_conf->value->cstring);
    settings.stepssize = atoi(stepsbuf);
  } 

  Tuple *stepsshape_conf = dict_find(iter, MESSAGE_KEY_stepsshape);
  if( stepsshape_conf ) {
    strcpy( stepsbuf, stepsshape_conf->value->cstring);
    settings.stepsshape = atoi(stepsbuf);
  } 

  Tuple *stepslabelcolor_conf = dict_find(iter, MESSAGE_KEY_stepslabelcolor);
  if( stepslabelcolor_conf ) {
    settings.stepslabelcolor = GColorFromHEX(stepslabelcolor_conf->value->int32);
  } 

  Tuple *stepsvaluecolor_conf = dict_find(iter, MESSAGE_KEY_stepsvaluecolor);
  if( stepsvaluecolor_conf ) {
    settings.stepsvaluecolor = GColorFromHEX(stepsvaluecolor_conf->value->int32);
  } 

  Tuple *stepstextbackcolor_conf = dict_find(iter, MESSAGE_KEY_stepstextbackcolor);
  if( stepstextbackcolor_conf ) {
    settings.stepstextbackcolor = GColorFromHEX(stepstextbackcolor_conf->value->int32);
  } 

  Tuple *stepstextbordercolor_conf = dict_find(iter, MESSAGE_KEY_stepstextbordercolor);
  if( stepstextbordercolor_conf ) {
    settings.stepstextbordercolor = GColorFromHEX(stepstextbordercolor_conf->value->int32);
  } 

  Tuple *stepsdialradius_conf = dict_find(iter, MESSAGE_KEY_stepsdialradius);
  if( stepsdialradius_conf ) {
    settings.stepsdialradius = stepsdialradius_conf->value->int32;
  } 

  Tuple *stepsneedlelength_conf = dict_find(iter, MESSAGE_KEY_stepsneedlelength);
  if( stepsneedlelength_conf ) {
    settings.stepsneedlelength = stepsneedlelength_conf->value->int32;
  } 

  Tuple *stepsneedlebalancelength_conf = dict_find(iter, MESSAGE_KEY_stepsneedlebalancelength);
  if( stepsneedlebalancelength_conf ) {
    settings.stepsneedlebalancelength = stepsneedlebalancelength_conf->value->int32;
  }

  // read steps needle start setting (floating needle, e.g. a dot indicator)
  Tuple *stepsneedlestart_conf = dict_find(iter, MESSAGE_KEY_stepsneedlestart);
  if( stepsneedlestart_conf ) {
    settings.stepsneedlestart = stepsneedlestart_conf->value->int32;
  }

  // read steps progress segment setting (filled arc from 12 o'clock to value)
  Tuple *stepssegment_conf = dict_find(iter, MESSAGE_KEY_stepssegment);
  if( stepssegment_conf ) {
    settings.stepssegment = stepssegment_conf->value->int32;
  }

  // CENTER HUB & AXIS SETTINGS

  Tuple *showcenterhub_conf = dict_find(iter, MESSAGE_KEY_showcenterhub);
  if( showcenterhub_conf ) {
    settings.showcenterhub = showcenterhub_conf->value->int32;
  }

  Tuple *centerhubauto_conf = dict_find(iter, MESSAGE_KEY_centerhubauto);
  if( centerhubauto_conf ) {
    settings.centerhubauto = centerhubauto_conf->value->int32;
  }

  Tuple *centerhubcolor_conf = dict_find(iter, MESSAGE_KEY_centerhubcolor);
  if( centerhubcolor_conf ) {
    settings.centerhubcolor = GColorFromHEX(centerhubcolor_conf->value->int32);
  }

  Tuple *showcenteraxis_conf = dict_find(iter, MESSAGE_KEY_showcenteraxis);
  if( showcenteraxis_conf ) {
    settings.showcenteraxis = showcenteraxis_conf->value->int32;
  }

  Tuple *centeraxiscolor_conf = dict_find(iter, MESSAGE_KEY_centeraxiscolor);
  if( centeraxiscolor_conf ) {
    settings.centeraxiscolor = GColorFromHEX(centeraxiscolor_conf->value->int32);
  }

  Tuple *stepsneedlethickness_conf = dict_find(iter, MESSAGE_KEY_stepsneedlethickness);
  if( stepsneedlethickness_conf ) {
    settings.stepsneedlethickness = stepsneedlethickness_conf->value->int32;
  } 

  Tuple *stepsneedlecolor_conf = dict_find(iter, MESSAGE_KEY_stepsneedlecolor);
  if( stepsneedlecolor_conf ) {
    settings.stepsneedlecolor = GColorFromHEX(stepsneedlecolor_conf->value->int32);
  } 

  Tuple *stepsfillcolor_conf = dict_find(iter, MESSAGE_KEY_stepsfillcolor);
  if( stepsfillcolor_conf ) {
    settings.stepsfillcolor = GColorFromHEX(stepsfillcolor_conf->value->int32);
  } 

  Tuple *stepsfillthickness_conf = dict_find(iter, MESSAGE_KEY_stepsfillthickness);
  if( stepsfillthickness_conf ) {
    settings.stepsfillthickness = stepsfillthickness_conf->value->int32;
  } 

  Tuple *stepsfillinnerpos_conf = dict_find(iter, MESSAGE_KEY_stepsfillinnerpos);
  if( stepsfillinnerpos_conf ) {
    settings.stepsfillinnerpos = stepsfillinnerpos_conf->value->int32;
  } 

  Tuple *stepsfillouterpos_conf = dict_find(iter, MESSAGE_KEY_stepsfillouterpos);
  if( stepsfillouterpos_conf ) {
    settings.stepsfillouterpos = stepsfillouterpos_conf->value->int32;
  } 

  Tuple *stepsdialborder_conf = dict_find(iter, MESSAGE_KEY_stepsdialborder);
  if( stepsdialborder_conf ) {
    settings.stepsdialborder = stepsdialborder_conf->value->int32;
  } 

  Tuple *stepsdialborderthickness_conf = dict_find(iter, MESSAGE_KEY_stepsdialborderthickness);
  if( stepsdialborderthickness_conf ) {
    strcpy( stepsbuf, stepsdialborderthickness_conf->value->cstring);
    settings.stepsdialborderthickness = atoi(stepsbuf);
  } 

  Tuple *stepsdialbordercolor_conf = dict_find(iter, MESSAGE_KEY_stepsdialbordercolor);
  if( stepsdialbordercolor_conf ) {
    settings.stepsdialbordercolor = GColorFromHEX(stepsdialbordercolor_conf->value->int32);
  } 

  Tuple *stepsmarkcolor_conf = dict_find(iter, MESSAGE_KEY_stepsmarkcolor);
  if( stepsmarkcolor_conf ) {
    settings.stepsmarkcolor = GColorFromHEX(stepsmarkcolor_conf->value->int32);
  } 

  Tuple *stepsmarkthickness_conf = dict_find(iter, MESSAGE_KEY_stepsmarkthickness);
  if( stepsmarkthickness_conf ) {
    settings.stepsmarkthickness = stepsmarkthickness_conf->value->int32;
  } 

  Tuple *stepsmarkinnerpos_conf = dict_find(iter, MESSAGE_KEY_stepsmarkinnerpos);
  if( stepsmarkinnerpos_conf ) {
    settings.stepsmarkinnerpos = stepsmarkinnerpos_conf->value->int32;
  } 

  Tuple *stepsmarkouterpos_conf = dict_find(iter, MESSAGE_KEY_stepsmarkouterpos);
  if( stepsmarkouterpos_conf ) {
    settings.stepsmarkouterpos = stepsmarkouterpos_conf->value->int32;
  } 

  Tuple *screenoffsety_conf = dict_find(iter, MESSAGE_KEY_screenoffsety);
  if( screenoffsety_conf ) {
    settings.screenoffsety = screenoffsety_conf->value->int32;
  } 

  Tuple *screenoffsetx_conf = dict_find(iter, MESSAGE_KEY_screenoffsetx);
  if( screenoffsetx_conf ) {
    settings.screenoffsetx = screenoffsetx_conf->value->int32;
  } 
  
  // read minute mark inner position setting
  Tuple *minsmarkinnerpos_conf = dict_find(iter, MESSAGE_KEY_minsmarkinnerpos);
  if( minsmarkinnerpos_conf ) {
    settings.minsmarkinnerpos = minsmarkinnerpos_conf->value->int32;
  } 

  // read minute mark outer position setting
  Tuple *minsmarkouterpos_conf = dict_find(iter, MESSAGE_KEY_minsmarkouterpos);
  if( minsmarkouterpos_conf ) {
    settings.minsmarkouterpos = minsmarkouterpos_conf->value->int32;
  } 

  // read hour mark thickness setting
  Tuple *hourmarkthickness_conf = dict_find(iter, MESSAGE_KEY_hourmarkthickness);
  if( hourmarkthickness_conf ) {
    settings.hourmarkthickness = hourmarkthickness_conf->value->int32;
  } 
  
  // read hour mark thickness setting
  Tuple *minsmarkthickness_conf = dict_find(iter, MESSAGE_KEY_minsmarkthickness);
  if( minsmarkthickness_conf ) {
    settings.minsmarkthickness = minsmarkthickness_conf->value->int32;
  } 

  // read hour filling setting
  Tuple *hourfillcolor_conf = dict_find(iter, MESSAGE_KEY_hourfillcolor);
  if ( hourfillcolor_conf ) {
    settings.hourfillcolor = GColorFromHEX(hourfillcolor_conf->value->int32);
  }
  
  // read minute filling setting
  Tuple *minsfillcolor_conf = dict_find(iter, MESSAGE_KEY_minsfillcolor);
  if ( minsfillcolor_conf ) {
    settings.minsfillcolor = GColorFromHEX(minsfillcolor_conf->value->int32);
  }
  
  // read seconds filling setting
  Tuple *secsfillcolor_conf = dict_find(iter, MESSAGE_KEY_secsfillcolor);
  if ( secsfillcolor_conf ) {
    settings.secsfillcolor = GColorFromHEX(secsfillcolor_conf->value->int32);
  }
  
  // read hour hand filling thickness setting
  Tuple *hourfillthickness_conf = dict_find(iter, MESSAGE_KEY_hourfillthickness);
  if( hourfillthickness_conf ) {
    settings.hourfillthickness = hourfillthickness_conf->value->int32;
  } 

  // read minute hand filling thickness setting
  Tuple *minsfillthickness_conf = dict_find(iter, MESSAGE_KEY_minsfillthickness);
  if( minsfillthickness_conf ) {
    settings.minsfillthickness = minsfillthickness_conf->value->int32;
  } 
  
  // read seconds hand filling thickness setting
  Tuple *secsfillthickness_conf = dict_find(iter, MESSAGE_KEY_secsfillthickness);
  if( secsfillthickness_conf ) {
    settings.secsfillthickness = secsfillthickness_conf->value->int32;
  } 

  // read hour filling inner position setting
  Tuple *hourfillinnerpos_conf = dict_find(iter, MESSAGE_KEY_hourfillinnerpos);
  if( hourfillinnerpos_conf ) {
    settings.hourfillinnerpos = hourfillinnerpos_conf->value->int32;
  } 
  
  // read hour filling outer position setting
  Tuple *hourfillouterpos_conf = dict_find(iter, MESSAGE_KEY_hourfillouterpos);
  if( hourfillouterpos_conf ) {
    settings.hourfillouterpos = hourfillouterpos_conf->value->int32;
  } 
  
  // read minute filling inner position setting
  Tuple *minsfillinnerpos_conf = dict_find(iter, MESSAGE_KEY_minsfillinnerpos);
  if( minsfillinnerpos_conf ) {
    settings.minsfillinnerpos = minsfillinnerpos_conf->value->int32;
  } 

  // read minute filling outer position setting
  Tuple *minsfillouterpos_conf = dict_find(iter, MESSAGE_KEY_minsfillouterpos);
  if( minsfillouterpos_conf ) {
    settings.minsfillouterpos = minsfillouterpos_conf->value->int32;
  } 

  // read seconds filling inner position setting
  Tuple *secsfillinnerpos_conf = dict_find(iter, MESSAGE_KEY_secsfillinnerpos);
  if( secsfillinnerpos_conf ) {
    settings.secsfillinnerpos = secsfillinnerpos_conf->value->int32;
  } 

  // read seconds filling outer position setting
  Tuple *secsfillouterpos_conf = dict_find(iter, MESSAGE_KEY_secsfillouterpos);
  if( secsfillouterpos_conf ) {
    settings.secsfillouterpos = secsfillouterpos_conf->value->int32;
  } 

  // read date display enabled setting
  Tuple *date_conf = dict_find(iter, MESSAGE_KEY_date);
  if( date_conf ) {
    settings.date = date_conf->value->int32;
  } 
  
  // read date shape setting
  Tuple *dateshape_conf = dict_find(iter, MESSAGE_KEY_dateshape);
  if( dateshape_conf ) { 
    char buf[10];
    strcpy( buf, dateshape_conf->value->cstring);
    settings.dateshape = atoi(buf);
  }

  // read date position degree setting
  Tuple *dateposdegree_conf = dict_find(iter, MESSAGE_KEY_dateposdegree);
  if( dateposdegree_conf ) {
    settings.dateposdegree = dateposdegree_conf->value->int32;
  }   

  // read date position rim setting
  Tuple *datepositionrim_conf = dict_find(iter, MESSAGE_KEY_datepositionrim);
  if( datepositionrim_conf ) {
    settings.datepositionrim = datepositionrim_conf->value->int32;
  }   

  // read date size setting
  Tuple *datesize_conf = dict_find(iter, MESSAGE_KEY_datesize);
  if( datesize_conf ) { 
    char buf[10];
    strcpy( buf, datesize_conf->value->cstring);
    settings.datesize = atoi(buf);
  }
  
  // read date text color setting
  Tuple *datetextcolor_conf = dict_find(iter, MESSAGE_KEY_datetextcolor);
  if ( datetextcolor_conf ) {
    settings.datetextcolor = GColorFromHEX(datetextcolor_conf->value->int32);
  }

  // read date background color setting
  Tuple *datebackcolor_conf = dict_find(iter, MESSAGE_KEY_datebackcolor);
  if ( datebackcolor_conf ) {
    settings.datebackcolor = GColorFromHEX(datebackcolor_conf->value->int32);
  }

  // read date border color setting
  Tuple *datebordercolor_conf = dict_find(iter, MESSAGE_KEY_datebordercolor);
  if ( datebordercolor_conf ) {
    settings.datebordercolor = GColorFromHEX(datebordercolor_conf->value->int32);
  }

  // BLUETOOTH SETTINGS
  
  // read bluetooth display enabled setting
  Tuple *bluetooth_conf = dict_find(iter, MESSAGE_KEY_bluetooth);
  if( bluetooth_conf ) {
    settings.bluetooth = bluetooth_conf->value->int32;
  } 
  
  // read bluetooth shape setting
  Tuple *btshape_conf = dict_find(iter, MESSAGE_KEY_btshape);
  if( btshape_conf ) { 
    char buf[10];
    strcpy( buf, btshape_conf->value->cstring);
    settings.btshape = atoi(buf);
  }

  // read bt position degree setting
  Tuple *btposdegree_conf = dict_find(iter, MESSAGE_KEY_btposdegree);
  if( btposdegree_conf ) {
    settings.btposdegree = btposdegree_conf->value->int32;
  }   

  // read bt position rim setting
  Tuple *btpositionrim_conf = dict_find(iter, MESSAGE_KEY_btpositionrim);
  if( btpositionrim_conf ) {
    settings.btpositionrim = btpositionrim_conf->value->int32;
  }   

  // read bt size setting
  Tuple *btsize_conf = dict_find(iter, MESSAGE_KEY_btsize);
  if( btsize_conf ) { 
    char buf[10];
    strcpy( buf, btsize_conf->value->cstring);
    settings.btsize = atoi(buf);
  }
  
  // read bluetooth connected sign color setting
  Tuple *btcolor_conf = dict_find(iter, MESSAGE_KEY_btcolor);
  if ( btcolor_conf ) {
    settings.btcolor = GColorFromHEX(btcolor_conf->value->int32);
  }

  // read bluetooth disconnected sign color setting
  Tuple *btoffcolor_conf = dict_find(iter, MESSAGE_KEY_btoffcolor);
  if ( btoffcolor_conf ) {
    settings.btoffcolor = GColorFromHEX(btoffcolor_conf->value->int32);
  }
  
  // read bluetooth background color setting
  Tuple *btbackcolor_conf = dict_find(iter, MESSAGE_KEY_btbackcolor);
  if ( btbackcolor_conf ) {
    settings.btbackcolor = GColorFromHEX(btbackcolor_conf->value->int32);
  }

  // read bluetooth disconnected background color setting
  Tuple *btoffbackcolor_conf = dict_find(iter, MESSAGE_KEY_btoffbackcolor);
  if ( btoffbackcolor_conf ) {
    settings.btoffbackcolor = GColorFromHEX(btoffbackcolor_conf->value->int32);
  }
  
  // read bluetooth rim color setting
  Tuple *btbordercolor_conf = dict_find(iter, MESSAGE_KEY_btbordercolor);
  if ( btbordercolor_conf ) {
    settings.btbordercolor = GColorFromHEX(btbordercolor_conf->value->int32);
  }

  // read bluetooth disconnected rim color setting
  Tuple *btoffbordercolor_conf = dict_find(iter, MESSAGE_KEY_btoffbordercolor);
  if ( btoffbordercolor_conf ) {
    settings.btoffbordercolor = GColorFromHEX(btoffbordercolor_conf->value->int32);
  }
  
  // read bluetooth always enabled setting
  Tuple *btalways_conf = dict_find(iter, MESSAGE_KEY_btalways);
  if( btalways_conf ) {
    settings.btalways = btalways_conf->value->int32;
  } 
  
  // read bluetooth vibration enabled setting
  Tuple *btvibe_conf = dict_find(iter, MESSAGE_KEY_btvibe);
  if( btvibe_conf ) {
    settings.btvibe = btvibe_conf->value->int32;
  } 

  // BATTERY SETTINGS
  
  // read battery display enabled setting
  Tuple *battery_conf = dict_find(iter, MESSAGE_KEY_battery);
  if( battery_conf ) {
    settings.battery = battery_conf->value->int32;
  } 
  
  // read battery shape setting
  Tuple *battshape_conf = dict_find(iter, MESSAGE_KEY_battshape);
  if( battshape_conf ) { 
    char buf[10];
    strcpy( buf, battshape_conf->value->cstring);
    settings.battshape = atoi(buf);
  }

  // read batt position degree setting
  Tuple *battposdegree_conf = dict_find(iter, MESSAGE_KEY_battposdegree);
  if( battposdegree_conf ) {
    settings.battposdegree = battposdegree_conf->value->int32;
  }   

  // read batt position rim setting
  Tuple *battpositionrim_conf = dict_find(iter, MESSAGE_KEY_battpositionrim);
  if( battpositionrim_conf ) {
    settings.battpositionrim = battpositionrim_conf->value->int32;
  }   

  // read bt size setting
  Tuple *battsize_conf = dict_find(iter, MESSAGE_KEY_battsize);
  if( battsize_conf ) { 
    char buf[10];
    strcpy( buf, battsize_conf->value->cstring);
    settings.battsize = atoi(buf);
  }
  
  // read batt ok sign color setting
  Tuple *battcolor_conf = dict_find(iter, MESSAGE_KEY_battcolor);
  if ( battcolor_conf ) {
    settings.battcolor = GColorFromHEX(battcolor_conf->value->int32);
  }

  // read batt warn sign color setting
  Tuple *battwarncolor_conf = dict_find(iter, MESSAGE_KEY_battwarncolor);
  if ( battwarncolor_conf ) {
    settings.battwarncolor = GColorFromHEX(battwarncolor_conf->value->int32);
  }
  
  // read batt background color setting
  Tuple *battbackcolor_conf = dict_find(iter, MESSAGE_KEY_battbackcolor);
  if ( battbackcolor_conf ) {
    settings.battbackcolor = GColorFromHEX(battbackcolor_conf->value->int32);
  }

  // read batt warn background color setting
  Tuple *battwarnbackcolor_conf = dict_find(iter, MESSAGE_KEY_battwarnbackcolor);
  if ( battwarnbackcolor_conf ) {
    settings.battwarnbackcolor = GColorFromHEX(battwarnbackcolor_conf->value->int32);
  }
  
  // read bat rim color setting
  Tuple *battbordercolor_conf = dict_find(iter, MESSAGE_KEY_battbordercolor);
  if ( battbordercolor_conf ) {
    settings.battbordercolor = GColorFromHEX(battbordercolor_conf->value->int32);
  }

  // read batt warn rim color setting
  Tuple *battwarnbordercolor_conf = dict_find(iter, MESSAGE_KEY_battwarnbordercolor);
  if ( battwarnbordercolor_conf ) {
    settings.battwarnbordercolor = GColorFromHEX(battwarnbordercolor_conf->value->int32);
  }
  
  // read batt vibration enabled setting
  Tuple *battvibe_conf = dict_find(iter, MESSAGE_KEY_battvibe);
  if( battvibe_conf ) {
    settings.battvibe = battvibe_conf->value->int32;
  } 
  
  // read batt warn level setting
  Tuple *battwarnlevel_conf = dict_find(iter, MESSAGE_KEY_battwarnlevel);
  if( battwarnlevel_conf ) {
    settings.battwarnlevel = battwarnlevel_conf->value->int32;
  } 
  
  // read batt warn interval setting
  Tuple *battwarninterval_conf = dict_find(iter, MESSAGE_KEY_battwarninterval);
  if( battwarninterval_conf ) {
    settings.battwarninterval = battwarninterval_conf->value->int32;
  } 
  
  // read shownumbers setting
  Tuple *shownumbers_conf = dict_find(iter, MESSAGE_KEY_shownumbers);
  if( shownumbers_conf ) {
    settings.shownumbers = shownumbers_conf->value->int32;
  }   
  
  // read number pos setting
  Tuple *numberpos_conf = dict_find(iter, MESSAGE_KEY_numberpos);
  if( numberpos_conf ) {
    settings.numberpos = numberpos_conf->value->int32;
  }   
  
  // read number font setting
  Tuple *numberfont_conf = dict_find(iter, MESSAGE_KEY_numberfont);
  if( numberfont_conf ) { 
    char buf[10];
    strcpy( buf, numberfont_conf->value->cstring);
    settings.numberfont = atoi(buf);
  }

  // read number style setting
  Tuple *numberstyle_conf = dict_find(iter, MESSAGE_KEY_numberstyle);
  if( numberstyle_conf ) { 
    char buf[10];
    strcpy( buf, numberstyle_conf->value->cstring);
    settings.numberstyle = atoi(buf);
  }

  // read number set setting
  Tuple *numberset_conf = dict_find(iter, MESSAGE_KEY_numberset);
  if( numberset_conf ) { 
    char buf[10];
    strcpy( buf, numberset_conf->value->cstring);
    settings.numberset = atoi(buf);
  }
  
  // read numbers text color setting
  Tuple *numbercolor_conf = dict_find(iter, MESSAGE_KEY_numbercolor);
  if ( numbercolor_conf ) {
    settings.numbercolor = GColorFromHEX(numbercolor_conf->value->int32);
  }
  
  // read minute mark style setting
  Tuple *minutemarkstyle_conf = dict_find(iter, MESSAGE_KEY_minutemarkstyle);
  if( minutemarkstyle_conf ) { 
    char buf[10];
    strcpy( buf, minutemarkstyle_conf->value->cstring);
    settings.minutemarkstyle = atoi(buf);
  }

  // read hour mark style setting
  Tuple *hourmarkstyle_conf = dict_find(iter, MESSAGE_KEY_hourmarkstyle);
  if( hourmarkstyle_conf ) { 
    char buf[10];
    strcpy( buf, hourmarkstyle_conf->value->cstring);
    settings.hourmarkstyle = atoi(buf);
  }

  save_settings();
  
  layer_mark_dirty(window_get_root_layer(s_window));
  //layer_mark_dirty(s_dot_layer);
  //layer_mark_dirty(s_hands_layer);

  if( settings.showsecond == 0 ) {
    tick_timer_service_unsubscribe();
    tick_timer_service_subscribe(MINUTE_UNIT, handle_time_tick );
  } else {
    tick_timer_service_unsubscribe();
    tick_timer_service_subscribe(SECOND_UNIT, handle_time_tick );
  }
}


static void draw_steps(GContext *ctx, GPoint center, GRect bounds);

static void hands_update_proc(Layer *layer, GContext *ctx) {

  time_t now = time(NULL);
  struct tm *t = localtime(&now);

  int hour = t->tm_hour%12;
  int min = t->tm_min;
  int sec = t->tm_sec;
  int day = t->tm_mday;

  // hand angles in whole degrees; optionally snapped to a 6-degree grid so
  // hands can land exactly on top of each other (60 discrete positions,
  // like a classic jumping minute hand). Minute and second already fall on
  // that grid (min*6 / sec*6); only the hour hand needs the rounding.
  int hourdeg = (hour*30)+(min*5/10);
  int mindeg  = min*6;
  int secdeg  = sec*6;
  if( settings.discretehands == 1 ) {
    hourdeg = (hourdeg/6)*6;
    mindeg  = (mindeg/6)*6;
    secdeg  = (secdeg/6)*6;
  }

  int angle = 0;

  GPoint inner, outer;
  int inner_pos, outer_pos;
  GRect bounds  = layer_get_bounds(s_hands_layer);
  GPoint center = GPoint( bounds.size.w/2 + settings.screenoffsetx, bounds.size.h/2 + settings.screenoffsety);

  graphics_context_set_antialiased(ctx, true);

  // date display
  if( settings.date == 1 ) {
    // angle where the display will appear
    angle = DEG_TO_TRIGANGLE(settings.dateposdegree);
    // calculate center position of date display
    inner_pos = bounds.size.w*settings.datepositionrim/200;
 	  inner.y = (int16_t)(-cos_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.y;
    inner.x = (int16_t)(sin_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.x;

    char buf[10];
    snprintf(buf, sizeof(buf), "%d", day);
    
    GFont font;
    int radius;
    GRect square, text;
    
    // setup of size params 
    switch( settings.datesize ) {
      case 0: {
        graphics_context_set_stroke_width(ctx, 1);
        font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
        radius = 10;
        square = GRect( inner.x-9, inner.y-9, 18, 18);
        text   = GRect( square.origin.x+1, square.origin.y-3, square.size.w, square.size.h);      
        break;
      }
      case 1: {
        graphics_context_set_stroke_width(ctx, 3);
        font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
        radius = 14;
        square = GRect( inner.x-13, inner.y-13, 26, 26);
        text   = GRect( square.origin.x, square.origin.y-3, square.size.w, square.size.h);      
        break;
      }
      case 2: {
        graphics_context_set_stroke_width(ctx, 3);
        font = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
        radius = 18;
        square = GRect( inner.x-15, inner.y-15, 30, 30);
        text   = GRect( square.origin.x, square.origin.y-3, square.size.w, square.size.h);      
        break;
      }
      default: {
        graphics_context_set_stroke_width(ctx, 3);
        font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
        radius = 14;
        square = GRect( inner.x-13, inner.y-13, 26, 26);
        text   = GRect( square.origin.x-2, square.origin.y-4, square.size.w, square.size.h);      
      }
    }
    
    // setup of colors
    graphics_context_set_text_color(ctx, settings.datetextcolor );
    graphics_context_set_fill_color(ctx, settings.datebackcolor );
    graphics_context_set_stroke_color(ctx, settings.datebordercolor );

    // depending on style (round, square)
    // round date display
    if( settings.dateshape == 0 ) {
      graphics_fill_circle( ctx, inner, radius );
      graphics_draw_circle( ctx, inner, radius );
    }
    // square date display
    else {
      graphics_fill_rect(ctx, GRect(square.origin.x, square.origin.y, square.size.w, square.size.h),0,GCornerNone);
      graphics_draw_rect(ctx, GRect(square.origin.x, square.origin.y, square.size.w, square.size.h));
    }
    
    // draw date number  
    graphics_draw_text(ctx, buf, font, text, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);

  }

  // bluetooth display
  if( settings.bluetooth == 1 ) {
    
    bool connected = connection_service_peek_pebble_app_connection();
    
    // angle where the display will appear
    angle = DEG_TO_TRIGANGLE(settings.btposdegree);
    // calculate center position of bluetooth display
    inner_pos = bounds.size.w*settings.btpositionrim/200;
 	  inner.y = (int16_t)(-cos_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.y;
    inner.x = (int16_t)(sin_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.x;

    int   radius;
    GRect square;
        
    // setup of size params 
    switch( settings.btsize ) {
      case 0: {
        graphics_context_set_stroke_width(ctx, 1);
        radius = 10;
        square = GRect( inner.x-10, inner.y-10, 20, 20);
        sign = small_sign;
        break;
      }
      case 1: {
        graphics_context_set_stroke_width(ctx, 3);
        radius = 14;
        square = GRect( inner.x-14, inner.y-14, 28, 28);
        sign = medium_sign;
        break;
      }
      case 2: {
        graphics_context_set_stroke_width(ctx, 3);
        radius = 18;
        square = GRect( inner.x-18, inner.y-18, 36, 36);
        sign = huge_sign;
        break;
      }
      default: {
        graphics_context_set_stroke_width(ctx, 1);
        radius = 10;
        square = GRect( inner.x-10, inner.y-10, 20, 20);
        sign = small_sign;
      }
    }
    
    // setup of colors for display window
    if( connected ) {
      graphics_context_set_fill_color(ctx, settings.btbackcolor );
      graphics_context_set_stroke_color(ctx, settings.btbordercolor );
    }
    else {
      graphics_context_set_fill_color(ctx, settings.btoffbackcolor );
      graphics_context_set_stroke_color(ctx, settings.btoffbordercolor );
    }
    
    // depending on style (round, square)
    // round display
    if( settings.btshape == 0 ) {
      graphics_fill_circle( ctx, inner, radius );
      graphics_draw_circle( ctx, inner, radius );
    }
    // square display
    else {
      graphics_fill_rect(ctx, GRect(square.origin.x, square.origin.y, square.size.w, square.size.h),0,GCornerNone);
      graphics_draw_rect(ctx, GRect(square.origin.x, square.origin.y, square.size.w, square.size.h));
    }
    
    // setup of colors for sign
    graphics_context_set_stroke_color(ctx, settings.btcolor );
    
    // draw sign
    if( connected ) {
      if( settings.btalways == 1 ) {
        graphics_context_set_stroke_color(ctx, settings.btcolor );   
        gpath_move_to(sign, inner);
        gpath_draw_outline(ctx, sign);
      }  
    }
    else {
      graphics_context_set_stroke_color(ctx, settings.btoffcolor );   
      gpath_move_to(sign, inner);
      gpath_draw_outline(ctx, sign);
      if( settings.btvibe == 1 ) {
        static const uint32_t segments[] = 
        { 200, 200, 100, 100, 100, 100, 100, 100, 200 };
         VibePattern pat = {
            .durations = segments,
            .num_segments = ARRAY_LENGTH(segments),
          };
        vibes_enqueue_custom_pattern(pat);
      }    
    }
  }

  // battery display
  if( settings.battery == 1 ) {
    BatteryChargeState battstate = battery_state_service_peek();
    
    int battpercent = battstate.charge_percent;
    bool battischarging = battstate.is_charging;
    
    // angle where the display will appear
    angle = DEG_TO_TRIGANGLE(settings.battposdegree);
    // calculate center position of batt display
    inner_pos = bounds.size.w*settings.battpositionrim/200;
 	  inner.y = (int16_t)(-cos_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.y;
    inner.x = (int16_t)(sin_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.x;

    int   radius;
    GRect square, text;   
    GFont font;
      
    // setup of size params 
    switch( settings.battsize ) {
      case 0: {
        graphics_context_set_stroke_width(ctx, 1);
        radius = 13;
        font = fonts_get_system_font(FONT_KEY_GOTHIC_09);
        square = GRect( inner.x-13, inner.y-8, 26, 16);
        text   = GRect( square.origin.x+2, square.origin.y+2, square.size.w, square.size.h);     
        break;
      }
      case 1: {
        graphics_context_set_stroke_width(ctx, 3);
        radius = 17;
        font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD );
        square = GRect( inner.x-17, inner.y-12, 34, 24);
        text   = GRect( square.origin.x, square.origin.y+3, square.size.w, square.size.h);      
        break;
      }
      case 2: {
        graphics_context_set_stroke_width(ctx, 3);
        radius = 21;
        font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
        square = GRect( inner.x-21, inner.y-14, 42, 28);
        text   = GRect( square.origin.x, square.origin.y+2, square.size.w, square.size.h);      
        break;
      }
      default: {
        graphics_context_set_stroke_width(ctx, 3);
        radius = 17;
        font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD );
        square = GRect( inner.x-17, inner.y-12, 34, 24);
        text   = GRect( square.origin.x, square.origin.y+3, square.size.w, square.size.h);      
      }
    }
    
    // setup of colors for battery rim and background window
    if( battpercent > settings.battwarnlevel ) {
      graphics_context_set_fill_color(ctx, settings.battbackcolor );
      graphics_context_set_stroke_color(ctx, settings.battbordercolor );
    }
    else {
      graphics_context_set_fill_color(ctx, settings.battwarnbackcolor );
      graphics_context_set_stroke_color(ctx, settings.battwarnbordercolor );
    }
    
    // depending on style (round, square)
    // round display
    if( settings.battshape == 0 ) {
      graphics_fill_circle( ctx, inner, radius );
      graphics_draw_circle( ctx, inner, radius );
    }
    // square display
    else {
      graphics_fill_rect(ctx, GRect(square.origin.x, square.origin.y, square.size.w, square.size.h),0,GCornerNone);
      graphics_draw_rect(ctx, GRect(square.origin.x, square.origin.y, square.size.w, square.size.h));
    }
    
    // setup of colors for text
    if( battpercent > settings.battwarnlevel ) {
      graphics_context_set_text_color(ctx, settings.battcolor );
    }
    else {
      graphics_context_set_text_color(ctx, settings.battwarncolor );
    }
    
    // draw battery number  
    char buf[10];
    if( battischarging ) {
      strcpy( buf, "CHG");      
    }
    else {
      snprintf(buf, sizeof(buf), "%d%%", battpercent);
    } 
    graphics_draw_text(ctx, buf, font, text, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
    
  }
 
  
  // hour hand
  bool hour_touches_center = false;
  if( settings.hourlength > 0 || settings.hourbalancelength > 0 || settings.hourneedlestart > 0 ) {
    angle = DEG_TO_TRIGANGLE(hourdeg);
    // needlestart moves the inner end out along the pointing direction (a
    // floating needle, e.g. a dot); balancelength extends it the opposite
    // way through the center (a counterweight). They share the same point.
    inner_pos = bounds.size.w*(settings.hourneedlestart - settings.hourbalancelength)/200;
    outer_pos = bounds.size.w*settings.hourlength/200;
    graphics_context_set_stroke_width(ctx, settings.hourthickness);
    graphics_context_set_stroke_color(ctx, settings.hourdialcolor );
    graphics_context_set_fill_color(ctx, settings.hourdialcolor );
 	  inner.y = (int16_t)(-cos_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.y;
    inner.x = (int16_t)(sin_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.x;
    outer.y = (int16_t)(-cos_lookup(angle) * (int32_t)outer_pos / TRIG_MAX_RATIO) + center.y;
    outer.x = (int16_t)(sin_lookup(angle) * (int32_t)outer_pos / TRIG_MAX_RATIO) + center.x;
    // draw hour hand
    graphics_draw_line(ctx, inner, outer);
    if( inner_pos <= 0 ) {
      hour_touches_center = true;
      if( settings.showcenterhub == 1 ) {
        graphics_context_set_fill_color(ctx, settings.centerhubauto == 1 ? settings.hourdialcolor : settings.centerhubcolor);
        graphics_fill_circle(ctx, center, settings.hourthickness-1);
      }
    }
  }

  // hour hand filling
  if( settings.hourfillinnerpos > 0 || settings.hourfillouterpos > 0 ) {
    angle = DEG_TO_TRIGANGLE(hourdeg);
    inner_pos = bounds.size.w*settings.hourfillinnerpos/200;
    outer_pos = bounds.size.w*settings.hourfillouterpos/200;
    graphics_context_set_stroke_width(ctx, settings.hourfillthickness);
    graphics_context_set_stroke_color(ctx, settings.hourfillcolor );
    graphics_context_set_fill_color(ctx, settings.hourfillcolor );
 	  inner.y = (int16_t)(-cos_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.y;
    inner.x = (int16_t)(sin_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.x;
    outer.y = (int16_t)(-cos_lookup(angle) * (int32_t)outer_pos / TRIG_MAX_RATIO) + center.y;
    outer.x = (int16_t)(sin_lookup(angle) * (int32_t)outer_pos / TRIG_MAX_RATIO) + center.x;
    // draw hour hand
    graphics_draw_line(ctx, inner, outer);
  }
  
  // minute hand
  bool mins_touches_center = false;
  if( settings.minslength > 0 || settings.minsbalancelength > 0 || settings.minsneedlestart > 0 ) {
    angle = DEG_TO_TRIGANGLE(mindeg);
    inner_pos = bounds.size.w*(settings.minsneedlestart - settings.minsbalancelength)/200;
    outer_pos = bounds.size.w*settings.minslength/200;
    graphics_context_set_stroke_width(ctx, settings.minsthickness);
    graphics_context_set_stroke_color(ctx, settings.minsdialcolor);
    graphics_context_set_fill_color(ctx, settings.minsdialcolor );
 	  inner.y = (int16_t)(-cos_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.y;
    inner.x = (int16_t)(sin_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.x;
    outer.y = (int16_t)(-cos_lookup(angle) * (int32_t)outer_pos / TRIG_MAX_RATIO) + center.y;
    outer.x = (int16_t)(sin_lookup(angle) * (int32_t)outer_pos / TRIG_MAX_RATIO) + center.x;
    // draw minute hand
    graphics_draw_line(ctx, inner, outer);
    if( inner_pos <= 0 ) {
      mins_touches_center = true;
      if( settings.showcenterhub == 1 ) {
        graphics_context_set_fill_color(ctx, settings.centerhubauto == 1 ? settings.minsdialcolor : settings.centerhubcolor);
        graphics_fill_circle(ctx, center, settings.minsthickness-1);
      }
    }
  }

  // minute hand filling
  if( settings.minsfillinnerpos > 0 || settings.minsfillouterpos > 0 ) {
    angle = DEG_TO_TRIGANGLE(mindeg);
    inner_pos = bounds.size.w*settings.minsfillinnerpos/200;
    outer_pos = bounds.size.w*settings.minsfillouterpos/200;
    graphics_context_set_stroke_width(ctx, settings.minsfillthickness);
    graphics_context_set_stroke_color(ctx, settings.minsfillcolor );
    graphics_context_set_fill_color(ctx, settings.minsfillcolor );
 	  inner.y = (int16_t)(-cos_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.y;
    inner.x = (int16_t)(sin_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.x;
    outer.y = (int16_t)(-cos_lookup(angle) * (int32_t)outer_pos / TRIG_MAX_RATIO) + center.y;
    outer.x = (int16_t)(sin_lookup(angle) * (int32_t)outer_pos / TRIG_MAX_RATIO) + center.x;
    // draw hour hand
    graphics_draw_line(ctx, inner, outer);
  }
  
  // seconds hand
  bool secs_touches_center = false;
  if( settings.secslength > 0 || settings.secsbalancelength > 0 || settings.secsneedlestart > 0 ) {
    if( settings.showsecond == 1 ) {
      angle = DEG_TO_TRIGANGLE(secdeg);
      inner_pos = bounds.size.w*(settings.secsneedlestart - settings.secsbalancelength)/200;
      outer_pos = bounds.size.w*settings.secslength/200;
      graphics_context_set_stroke_width(ctx, settings.secsthickness);
      graphics_context_set_stroke_color(ctx, settings.secsdialcolor);
      graphics_context_set_fill_color(ctx, settings.secsdialcolor );
     	inner.y = (int16_t)(-cos_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.y;
      inner.x = (int16_t)(sin_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.x;
      outer.y = (int16_t)(-cos_lookup(angle) * (int32_t)outer_pos / TRIG_MAX_RATIO) + center.y;
      outer.x = (int16_t)(sin_lookup(angle) * (int32_t)outer_pos / TRIG_MAX_RATIO) + center.x;
      // draw seconds hand
      graphics_draw_line(ctx, inner, outer);
      if( inner_pos <= 0 ) {
        secs_touches_center = true;
        if( settings.showcenterhub == 1 ) {
          graphics_context_set_fill_color(ctx, settings.centerhubauto == 1 ? settings.secsdialcolor : settings.centerhubcolor);
          graphics_fill_circle(ctx, center, settings.secsthickness-1);
        }
      }
    }
  }

  // seconds hand filling
  if( settings.secsfillinnerpos > 0 || settings.secsfillouterpos > 0 ) {
    if( settings.showsecond == 1 ) {
      angle = DEG_TO_TRIGANGLE(secdeg);
      inner_pos = bounds.size.w*settings.secsfillinnerpos/200;
      outer_pos = bounds.size.w*settings.secsfillouterpos/200;
      graphics_context_set_stroke_width(ctx, settings.secsfillthickness);
      graphics_context_set_stroke_color(ctx, settings.secsfillcolor );
      graphics_context_set_fill_color(ctx, settings.secsfillcolor );
 	    inner.y = (int16_t)(-cos_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.y;
      inner.x = (int16_t)(sin_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.x;
      outer.y = (int16_t)(-cos_lookup(angle) * (int32_t)outer_pos / TRIG_MAX_RATIO) + center.y;
      outer.x = (int16_t)(sin_lookup(angle) * (int32_t)outer_pos / TRIG_MAX_RATIO) + center.x;
      // draw seconds hand filling
      graphics_draw_line(ctx, inner, outer);
    }  
  }
 
  // axis dot in the middle (only if at least one hand actually reaches the
  // center; needle-start hands that float away from the center skip this too)
  if( settings.showcenteraxis == 1 &&
      (hour_touches_center || mins_touches_center || secs_touches_center) ) {
    graphics_context_set_fill_color(ctx, settings.centeraxiscolor);
    graphics_context_set_stroke_width(ctx, 1);
    graphics_fill_circle(ctx, center, 2);
  }

  // steps dial/text drawn above the hands, if configured that way
  if( settings.stepsontop == 1 ) {
    draw_steps(ctx, center, bounds);
  }
}

// Computes a point at the given angle and percent distance from center.
// When use_rect is false, 100% lands on an inscribed circle (classic round-face look).
// When use_rect is true, 100% lands exactly on the physical case rim of a
// rectangular display along that angle (Cartier-style markers reaching every edge).
static GPoint mark_point(GPoint center, GRect bounds, int32_t angle, int32_t percent, bool use_rect) {
  GPoint p;
  if (use_rect) {
    int32_t s = sin_lookup(angle);
    int32_t c = -cos_lookup(angle);
    int32_t abs_s = (s < 0) ? -s : s;
    int32_t abs_c = (c < 0) ? -c : c;
    int32_t half_w = bounds.size.w / 2;
    int32_t half_h = bounds.size.h / 2;
    // distance from center to the rectangle edge along this angle
    int32_t t_w = (abs_s == 0) ? 0x7FFFFFFF : (int32_t)(((int64_t)half_w * TRIG_MAX_RATIO) / abs_s);
    int32_t t_h = (abs_c == 0) ? 0x7FFFFFFF : (int32_t)(((int64_t)half_h * TRIG_MAX_RATIO) / abs_c);
    int32_t t   = (t_w < t_h) ? t_w : t_h;
    p.x = (int16_t)((((int64_t)s * t / TRIG_MAX_RATIO) * percent / 100)) + center.x;
    p.y = (int16_t)((((int64_t)c * t / TRIG_MAX_RATIO) * percent / 100)) + center.y;
  } else {
    int32_t pos = bounds.size.w * percent / 200;
    p.y = (int16_t)(-cos_lookup(angle) * (int32_t)pos / TRIG_MAX_RATIO) + center.y;
    p.x = (int16_t)(sin_lookup(angle) * (int32_t)pos / TRIG_MAX_RATIO) + center.x;
  }
  return p;
}

// Computes the current moon phase and draws it as a small disc.
// Phase is derived from the wall clock date using a fixed synodic month
// length, no network/phone connection needed. angle 0/TRIG_MAX_RATIO = new
// moon, TRIG_MAX_RATIO/2 = full moon (same fixed-point trig used elsewhere
// in this file, so no floating point is needed).
static void draw_moon_phase(GContext *ctx, GPoint moon_center, int32_t radius,
                             GColor light, GColor dark, bool border, GColor border_color,
                             bool southern) {
  // Seconds of a known new moon (Jan 6 2000, 18:14 UTC) and the synodic
  // month length in seconds (29.530588 days), both as fixed integers.
  const int32_t reference_new_moon = 947182440;
  const int32_t synodic_seconds    = 2551443;

  time_t now = time(NULL);
  int32_t elapsed = (int32_t)now - reference_new_moon;
  int32_t age = elapsed % synodic_seconds;
  if (age < 0) {
    age += synodic_seconds;
  }
  // age/synodic_seconds is the phase fraction (0..1), expressed directly as
  // a fixed-point angle so sin_lookup/cos_lookup can be used on it.
  int32_t angle = (int32_t)(((int64_t)age * TRIG_MAX_RATIO) / synodic_seconds);

  int32_t phase_cos = cos_lookup(angle);
  // Terminator half-width: how far the day/night edge sits from the
  // center, shrinking to 0 at the quarters and to +/-radius at new/full.
  int32_t terminator_x = (int32_t)(((int64_t)radius * phase_cos) / TRIG_MAX_RATIO);

  // Waxing (first half of the cycle) is lit on the right in the northern
  // hemisphere; waning is lit on the left. Southern hemisphere mirrors it.
  bool waxing = angle < (TRIG_MAX_RATIO / 2);
  bool bright_right = waxing;
  if (southern) {
    bright_right = !bright_right;
  }

  GColor bright_color = light;
  GColor dim_color    = dark;
  // illuminated fraction < 0.5 while close to new moon (phase_cos > 0)
  GColor ellipse_color = (phase_cos > 0) ? dark : light;

  // Base: fill the whole disc dim, then the bright half on top.
  graphics_context_set_fill_color(ctx, dim_color);
  graphics_fill_circle(ctx, moon_center, (uint16_t)radius);

  GPoint half_pts[MOON_HALF_SEGMENTS + 1];
  int32_t half_start = bright_right ? 0 : (TRIG_MAX_RATIO / 2);
  for (int i = 0; i <= MOON_HALF_SEGMENTS; i++) {
    int32_t a = half_start + (int32_t)(((int64_t)i * (TRIG_MAX_RATIO / 2)) / MOON_HALF_SEGMENTS);
    half_pts[i].x = (int16_t)(((int64_t)radius * sin_lookup(a)) / TRIG_MAX_RATIO) + moon_center.x;
    half_pts[i].y = (int16_t)(-((int64_t)radius * cos_lookup(a)) / TRIG_MAX_RATIO) + moon_center.y;
  }
  GPath half_path = { .num_points = MOON_HALF_SEGMENTS + 1, .points = half_pts, .rotation = 0, .offset = GPointZero };
  graphics_context_set_fill_color(ctx, bright_color);
  gpath_draw_filled(ctx, &half_path);

  // Terminator: a full ellipse squeezed to |terminator_x|, on top of the
  // half-and-half base. This alone is what turns it into a crescent or a
  // gibbous shape instead of a plain half moon.
  GPoint ellipse_pts[MOON_ELLIPSE_SEGMENTS];
  int32_t ellipse_rx = (terminator_x < 0) ? -terminator_x : terminator_x;
  for (int i = 0; i < MOON_ELLIPSE_SEGMENTS; i++) {
    int32_t a = (int32_t)(((int64_t)i * TRIG_MAX_RATIO) / MOON_ELLIPSE_SEGMENTS);
    ellipse_pts[i].x = (int16_t)(((int64_t)ellipse_rx * sin_lookup(a)) / TRIG_MAX_RATIO) + moon_center.x;
    ellipse_pts[i].y = (int16_t)(-((int64_t)radius * cos_lookup(a)) / TRIG_MAX_RATIO) + moon_center.y;
  }
  GPath ellipse_path = { .num_points = MOON_ELLIPSE_SEGMENTS, .points = ellipse_pts, .rotation = 0, .offset = GPointZero };
  graphics_context_set_fill_color(ctx, ellipse_color);
  gpath_draw_filled(ctx, &ellipse_path);

  if (border) {
    graphics_context_set_stroke_color(ctx, border_color);
    graphics_context_set_stroke_width(ctx, 1);
    graphics_draw_circle(ctx, moon_center, (uint16_t)radius);
  }
}

// Draws the "STEPS" label plus the step count as two lines inside a small
// round or square badge, positioned like the date/bluetooth/battery displays.
static void draw_steps_text(GContext *ctx, GPoint pos, int32_t steps) {
  if (steps > 99999) {
    steps = 99999;
  }
  char buf[8];
  snprintf(buf, sizeof(buf), "%d", (int)steps);

  GFont label_font, value_font;
  int32_t radius;
  GRect square, label_rect, value_rect;

  switch (settings.stepssize) {
    case 0: {
      label_font = fonts_get_system_font(FONT_KEY_GOTHIC_09);
      value_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
      radius = 20;
      square = GRect(pos.x - 19, pos.y - 19, 38, 38);
      label_rect = GRect(square.origin.x, square.origin.y + 2, square.size.w, 12);
      value_rect = GRect(square.origin.x, square.origin.y + 13, square.size.w, 20);
      break;
    }
    case 2: {
      label_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
      value_font = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
      radius = 30;
      square = GRect(pos.x - 29, pos.y - 29, 58, 58);
      label_rect = GRect(square.origin.x, square.origin.y + 4, square.size.w, 16);
      value_rect = GRect(square.origin.x, square.origin.y + 20, square.size.w, 30);
      break;
    }
    default: {
      label_font = fonts_get_system_font(FONT_KEY_GOTHIC_09);
      value_font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
      radius = 25;
      square = GRect(pos.x - 24, pos.y - 24, 48, 48);
      label_rect = GRect(square.origin.x, square.origin.y + 3, square.size.w, 14);
      value_rect = GRect(square.origin.x, square.origin.y + 16, square.size.w, 26);
    }
  }

  graphics_context_set_stroke_width(ctx, 2);
  graphics_context_set_fill_color(ctx, settings.stepstextbackcolor);
  graphics_context_set_stroke_color(ctx, settings.stepstextbordercolor);

  if (settings.stepsshape == 0) {
    graphics_fill_circle(ctx, pos, (uint16_t)radius);
    graphics_draw_circle(ctx, pos, (uint16_t)radius);
  } else {
    graphics_fill_rect(ctx, square, 0, GCornerNone);
    graphics_draw_rect(ctx, square);
  }

  graphics_context_set_text_color(ctx, settings.stepslabelcolor);
  graphics_draw_text(ctx, "STEPS", label_font, label_rect, GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  graphics_context_set_text_color(ctx, settings.stepsvaluecolor);
  graphics_draw_text(ctx, buf, value_font, value_rect, GTextOverflowModeFill, GTextAlignmentCenter, NULL);
}

// Draws the steps sub-dial: 12 configurable marks (a full turn = 12000
// steps), an optional border with selectable thickness, and an
// hour-hand-style needle (with its own accent fill). The needle simply
// caps at the top once 12000 steps is reached.
static void draw_steps_dial(GContext *ctx, GPoint center, int32_t dial_radius, int32_t steps) {
  const int32_t scale = 12000;
  int32_t shown_steps = (steps > scale) ? scale : steps;

  // needle angle; snapped to the same 6-degree grid as the clock hands when
  // discrete mode is on (6 degrees = 200 steps on the 12000-step dial)
  int32_t needle_angle;
  if (settings.discretehands == 1) {
    int32_t deg = (int32_t)(((int64_t)shown_steps * 360 / scale) / 6 * 6);
    needle_angle = DEG_TO_TRIGANGLE(deg);
  } else {
    needle_angle = (int32_t)(((int64_t)shown_steps * TRIG_MAX_RATIO) / scale);
  }

  // progress segment from 12 o'clock to the current value, in needle color,
  // spanning the needle's radial range (start..length). Drawn first so the
  // marks and border stay visible on top of it.
  //
  // The needle is a thick line with round end caps, so it actually reaches
  // half its thickness beyond both of its endpoints. The segment has to be
  // inflated by the same amount, otherwise a thick needle visibly sticks out
  // of the arc that is supposed to end in it.
  if (settings.stepssegment == 1) {
    int32_t cap = settings.stepsneedlethickness / 2;
    int32_t seg_outer = dial_radius * settings.stepsneedlelength / 100 + cap;
    int32_t seg_inner = dial_radius * settings.stepsneedlestart / 100 - cap;
    if (seg_inner < 0) seg_inner = 0;
    if (seg_outer > seg_inner && needle_angle > 0) {
      GRect seg_rect = GRect(center.x - seg_outer, center.y - seg_outer,
                             2 * seg_outer, 2 * seg_outer);
      graphics_context_set_fill_color(ctx, settings.stepsneedlecolor);
      graphics_fill_radial(ctx, seg_rect, GOvalScaleModeFitCircle,
                           (uint16_t)(seg_outer - seg_inner), 0, needle_angle);
    }
  }

  // 12 evenly spaced marks (one per 1000 steps)
  graphics_context_set_stroke_color(ctx, settings.stepsmarkcolor);
  graphics_context_set_stroke_width(ctx, settings.stepsmarkthickness);
  int32_t mark_outer_r = dial_radius * settings.stepsmarkouterpos / 100;
  int32_t mark_inner_r = dial_radius * settings.stepsmarkinnerpos / 100;
  for (int i = 0; i < 12; i++) {
    int32_t a = (int32_t)(((int64_t)i * TRIG_MAX_RATIO) / 12);
    GPoint m_outer, m_inner;
    m_outer.x = (int16_t)(((int64_t)mark_outer_r * sin_lookup(a)) / TRIG_MAX_RATIO) + center.x;
    m_outer.y = (int16_t)(-((int64_t)mark_outer_r * cos_lookup(a)) / TRIG_MAX_RATIO) + center.y;
    m_inner.x = (int16_t)(((int64_t)mark_inner_r * sin_lookup(a)) / TRIG_MAX_RATIO) + center.x;
    m_inner.y = (int16_t)(-((int64_t)mark_inner_r * cos_lookup(a)) / TRIG_MAX_RATIO) + center.y;
    graphics_draw_line(ctx, m_inner, m_outer);
  }

  if (settings.stepsdialborder == 1) {
    graphics_context_set_stroke_color(ctx, settings.stepsdialbordercolor);
    graphics_context_set_stroke_width(ctx, settings.stepsdialborderthickness);
    graphics_draw_circle(ctx, center, (uint16_t)dial_radius);
  }

  // needle fill (accent stripe), same idea as the hour hand filling
  if (settings.stepsfillinnerpos > 0 || settings.stepsfillouterpos > 0) {
    int32_t f_inner = dial_radius * settings.stepsfillinnerpos / 100;
    int32_t f_outer = dial_radius * settings.stepsfillouterpos / 100;
    graphics_context_set_stroke_width(ctx, settings.stepsfillthickness);
    graphics_context_set_stroke_color(ctx, settings.stepsfillcolor);
    GPoint f_inner_p, f_outer_p;
    f_inner_p.y = (int16_t)(-cos_lookup(needle_angle) * f_inner / TRIG_MAX_RATIO) + center.y;
    f_inner_p.x = (int16_t)(sin_lookup(needle_angle) * f_inner / TRIG_MAX_RATIO) + center.x;
    f_outer_p.y = (int16_t)(-cos_lookup(needle_angle) * f_outer / TRIG_MAX_RATIO) + center.y;
    f_outer_p.x = (int16_t)(sin_lookup(needle_angle) * f_outer / TRIG_MAX_RATIO) + center.x;
    graphics_draw_line(ctx, f_inner_p, f_outer_p);
  }

  // needle
  // needlestart moves the inner end out along the pointing direction (a
  // floating needle, e.g. a dot); balancelength extends it the opposite way
  // through the center (a counterweight). They share the same point.
  int32_t n_inner = dial_radius * (settings.stepsneedlestart - settings.stepsneedlebalancelength) / 100;
  int32_t n_outer = dial_radius * settings.stepsneedlelength / 100;
  graphics_context_set_stroke_width(ctx, settings.stepsneedlethickness);
  graphics_context_set_stroke_color(ctx, settings.stepsneedlecolor);
  graphics_context_set_fill_color(ctx, settings.stepsneedlecolor);
  GPoint n_inner_p, n_outer_p;
  n_inner_p.y = (int16_t)(-cos_lookup(needle_angle) * n_inner / TRIG_MAX_RATIO) + center.y;
  n_inner_p.x = (int16_t)(sin_lookup(needle_angle) * n_inner / TRIG_MAX_RATIO) + center.x;
  n_outer_p.y = (int16_t)(-cos_lookup(needle_angle) * n_outer / TRIG_MAX_RATIO) + center.y;
  n_outer_p.x = (int16_t)(sin_lookup(needle_angle) * n_outer / TRIG_MAX_RATIO) + center.x;
  graphics_draw_line(ctx, n_inner_p, n_outer_p);
  if( n_inner <= 0 ) {
    int32_t hub = settings.stepsneedlethickness - 1;
    graphics_fill_circle(ctx, center, (uint16_t)(hub > 0 ? hub : 1));
  }
}

// Draws the steps text/dial at its configured position. Shared between
// dot_update_proc and hands_update_proc so the "stepsontop" setting can
// place it below or above the hour/minute/second hands.
static void draw_steps(GContext *ctx, GPoint center, GRect bounds) {
  if( settings.showsteps != 1 ) {
    return;
  }

  int32_t steps = 0;
  HealthServiceAccessibilityMask mask = health_service_metric_accessible(HealthMetricStepCount, time_start_of_today(), time(NULL));
  if (mask & HealthServiceAccessibilityMaskAvailable) {
    steps = (int32_t)health_service_sum_today(HealthMetricStepCount);
  }

  int32_t angle = DEG_TO_TRIGANGLE(settings.stepsposdegree);
  int32_t inner_pos = bounds.size.w*settings.stepspositionrim/200;
  GPoint inner;
  inner.y = (int16_t)(-cos_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.y;
  inner.x = (int16_t)(sin_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.x;

  if( settings.stepsstyle == 0 ) {
    draw_steps_text(ctx, inner, steps);
  } else {
    int32_t dial_radius = bounds.size.w*settings.stepsdialradius/200;
    draw_steps_dial(ctx, inner, dial_radius, steps);
  }
}

static void dot_update_proc(Layer *layer, GContext *ctx) {
  
  GRect bounds  = layer_get_bounds(s_dot_layer);
  GPoint center = GPoint( bounds.size.w/2 + settings.screenoffsetx, bounds.size.h/2 + settings.screenoffsety);
  GFont font = NULL;
  
  int inner_pos, outer_pos;
  int textsize;
  int angle = 0;
  GPoint inner, outer;
  
  // fullscreen
  if( settings.fullscreen == 0 ) {
    window_set_background_color(s_window, GColorBlack);
    graphics_context_set_fill_color(ctx, settings.backcolor);
    graphics_fill_circle(ctx, center, (bounds.size.w/2));
  } 
  else {
    window_set_background_color(s_window, settings.backcolor);
  }

  // steps drawn underneath the hour/minute marks, if configured that way
  if( settings.stepsontop == 2 ) {
    draw_steps(ctx, center, bounds);
  }

  // minute marks
  if( settings.showminsmark == 1 ) {
    graphics_context_set_fill_color(ctx, settings.minsmarkcolor);
    graphics_context_set_stroke_color(ctx, settings.minsmarkcolor);
    graphics_context_set_stroke_width(ctx, settings.minsmarkthickness );
    switch( settings.minutemarkstyle ) {
      // all marks
      case 0: { 
	      for (int i=0; i<=60; i+=1) {
		      angle = DEG_TO_TRIGANGLE(i*6);
	      inner = mark_point(center, bounds, angle, settings.minsmarkinnerpos, settings.minsmarkrim);
	      outer = mark_point(center, bounds, angle, settings.minsmarkouterpos, settings.minsmarkrim);
		      // Draw tick mark
          graphics_draw_line(ctx, inner, outer);
        }  
        break;
      }
      // every fifth minute
      case 1: { 
	      for (int i=0; i<=60; i+=5) {
		      angle = DEG_TO_TRIGANGLE(i*6);
	      inner = mark_point(center, bounds, angle, settings.minsmarkinnerpos, settings.minsmarkrim);
	      outer = mark_point(center, bounds, angle, settings.minsmarkouterpos, settings.minsmarkrim);
		      // Draw tick mark
          graphics_draw_line(ctx, inner, outer);
        }  
        break;
      }
      // every fifteenth minute
      case 2: { 
	      for (int i=0; i<=60; i+=15) {
		      angle = DEG_TO_TRIGANGLE(i*6);
	      inner = mark_point(center, bounds, angle, settings.minsmarkinnerpos, settings.minsmarkrim);
	      outer = mark_point(center, bounds, angle, settings.minsmarkouterpos, settings.minsmarkrim);
		      // Draw tick mark
          graphics_draw_line(ctx, inner, outer);
        }  
        break;
      }
      // all with a 3 minute gap around all hours
      case 3: { 
	      for (int i=2; i<60; i+=5) {
		      angle = DEG_TO_TRIGANGLE(i*6);
	      inner = mark_point(center, bounds, angle, settings.minsmarkinnerpos, settings.minsmarkrim);
	      outer = mark_point(center, bounds, angle, settings.minsmarkouterpos, settings.minsmarkrim);
		      // Draw tick mark
          graphics_draw_line(ctx, inner, outer);
        }  
	      for (int i=3; i<60; i+=5) {
		      angle = DEG_TO_TRIGANGLE(i*6);
	      inner = mark_point(center, bounds, angle, settings.minsmarkinnerpos, settings.minsmarkrim);
	      outer = mark_point(center, bounds, angle, settings.minsmarkouterpos, settings.minsmarkrim);
		      // Draw tick mark
          graphics_draw_line(ctx, inner, outer);
        }  
        break;
      }
      // All with a 3 minute gap around 4 hours
      case 4: { 
	      for (int i=2; i<14; i+=1) {
		      angle = DEG_TO_TRIGANGLE(i*6);
	      inner = mark_point(center, bounds, angle, settings.minsmarkinnerpos, settings.minsmarkrim);
	      outer = mark_point(center, bounds, angle, settings.minsmarkouterpos, settings.minsmarkrim);
		      // Draw tick mark
          graphics_draw_line(ctx, inner, outer);
        }  
	      for (int i=17; i<29; i+=1) {
		      angle = DEG_TO_TRIGANGLE(i*6);
	      inner = mark_point(center, bounds, angle, settings.minsmarkinnerpos, settings.minsmarkrim);
	      outer = mark_point(center, bounds, angle, settings.minsmarkouterpos, settings.minsmarkrim);
		      // Draw tick mark
          graphics_draw_line(ctx, inner, outer);
        }  
	      for (int i=32; i<44; i+=1) {
		      angle = DEG_TO_TRIGANGLE(i*6);
	      inner = mark_point(center, bounds, angle, settings.minsmarkinnerpos, settings.minsmarkrim);
	      outer = mark_point(center, bounds, angle, settings.minsmarkouterpos, settings.minsmarkrim);
		      // Draw tick mark
          graphics_draw_line(ctx, inner, outer);
        }  
	      for (int i=47; i<59; i+=1) {
		      angle = DEG_TO_TRIGANGLE(i*6);
	      inner = mark_point(center, bounds, angle, settings.minsmarkinnerpos, settings.minsmarkrim);
	      outer = mark_point(center, bounds, angle, settings.minsmarkouterpos, settings.minsmarkrim);
		      // Draw tick mark
          graphics_draw_line(ctx, inner, outer);
        }  
        break;
      }
      // all with a 3 minute gap around 12
      case 5: { 
	      for (int i=2; i<59; i+=15) {
		      angle = DEG_TO_TRIGANGLE(i*6);
	      inner = mark_point(center, bounds, angle, settings.minsmarkinnerpos, settings.minsmarkrim);
	      outer = mark_point(center, bounds, angle, settings.minsmarkouterpos, settings.minsmarkrim);
		      // Draw tick mark
          graphics_draw_line(ctx, inner, outer);
        }  
        break;
      }
      default: break;
    }  
  }
  
  //APP_LOG( APP_LOG_LEVEL_DEBUG, "hourstyle: %d\nminutestyle: %d",settings.hourmarkstyle, settings.minutemarkstyle);
  
  // hour marks
  if( settings.showhourmark == 1 ) {
    graphics_context_set_fill_color(ctx, settings.hourmarkcolor);
    graphics_context_set_stroke_color(ctx, settings.hourmarkcolor);
    graphics_context_set_stroke_width(ctx, settings.hourmarkthickness );
    switch( settings.hourmarkstyle ) {
      // all marks
      case 0: { 
	      for (int i=0; i<=12; i+=1) {
		      angle = DEG_TO_TRIGANGLE(i*30);
		      inner = mark_point(center, bounds, angle, settings.hourmarkinnerpos, settings.hourmarkrim);
		      outer = mark_point(center, bounds, angle, settings.hourmarkouterpos, settings.hourmarkrim);
		      // Draw tick mark
          graphics_draw_line(ctx, inner, outer);
        }
        break;
   	  }
      // Leave one out (1, 3, 5, ...)
      case 1: { 
	      for (int i=1; i<=12; i+=2) {
		      angle = DEG_TO_TRIGANGLE(i*30);
		      inner = mark_point(center, bounds, angle, settings.hourmarkinnerpos, settings.hourmarkrim);
		      outer = mark_point(center, bounds, angle, settings.hourmarkouterpos, settings.hourmarkrim);
		      // Draw tick mark
          graphics_draw_line(ctx, inner, outer);
        }
        break;
   	  }
      // Leave one out (12, 2, 4, ...)
      case 2: { 
	      for (int i=0; i<=12; i+=2) {
		      angle = DEG_TO_TRIGANGLE(i*30);
		      inner = mark_point(center, bounds, angle, settings.hourmarkinnerpos, settings.hourmarkrim);
		      outer = mark_point(center, bounds, angle, settings.hourmarkouterpos, settings.hourmarkrim);
		      // Draw tick mark
          graphics_draw_line(ctx, inner, outer);
        }
        break;
   	  }
      // Between 4 hour numbers (1,2,4,5,...)
      case 3: { 
	      for (int i=1; i<=12; i+=3) {
		      angle = DEG_TO_TRIGANGLE(i*30);
		      inner = mark_point(center, bounds, angle, settings.hourmarkinnerpos, settings.hourmarkrim);
		      outer = mark_point(center, bounds, angle, settings.hourmarkouterpos, settings.hourmarkrim);
		      // Draw tick mark
          graphics_draw_line(ctx, inner, outer);
        }
	      for (int i=2; i<=12; i+=3) {
		      angle = DEG_TO_TRIGANGLE(i*30);
		      inner = mark_point(center, bounds, angle, settings.hourmarkinnerpos, settings.hourmarkrim);
		      outer = mark_point(center, bounds, angle, settings.hourmarkouterpos, settings.hourmarkrim);
		      // Draw tick mark
          graphics_draw_line(ctx, inner, outer);
        }
        break;
   	  }
      // All except of twelve
      case 4: { 
	      for (int i=1; i<12; i+=1) {
		      angle = DEG_TO_TRIGANGLE(i*30);
		      inner = mark_point(center, bounds, angle, settings.hourmarkinnerpos, settings.hourmarkrim);
		      outer = mark_point(center, bounds, angle, settings.hourmarkouterpos, settings.hourmarkrim);
		      // Draw tick mark
          graphics_draw_line(ctx, inner, outer);
        }
        break;
   	  }
      default: break;
    }  
  }
  
  if( settings.shownumbers == 1 ) {
    graphics_context_set_text_color(ctx, settings.numbercolor);
    switch( settings.numberfont ) 
    {
      case 0:  { 
                  font = fonts_get_system_font(FONT_KEY_GOTHIC_09); 
                  textsize = 9;
                  break;
               } 
      case 1:  { 
                  font = fonts_get_system_font(FONT_KEY_GOTHIC_14); 
                  textsize = 14;
                  break; 
               } 
      case 2:  { 
                  font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD); 
                  textsize = 14;
                  break; 
               } 
      case 3:  { 
                  font = fonts_get_system_font(FONT_KEY_GOTHIC_18); 
                  textsize = 20;
                  break; 
               } 
      case 4:  { 
                  font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD); 
                  textsize = 20;
                  break; 
               } 
      case 5:  { 
                  font = fonts_get_system_font(FONT_KEY_GOTHIC_24); 
                  textsize = 28;
                  break; 
               } 
      case 6:  { 
                  font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD); 
                  textsize = 28;
                  break; 
               } 
      case 7:  { 
                  font = fonts_get_system_font(FONT_KEY_GOTHIC_28); 
                  textsize = 32;
                  break; 
               } 
      case 8:  { 
                  font = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD); 
                  textsize = 32;
                  break; 
               } 
      case 9:  { 
                  font = fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK); 
                  textsize = 34;
                  break; 
               } 
      default: { 
                  font = fonts_get_system_font(FONT_KEY_GOTHIC_09); 
                  textsize = 9;
                  break;
               } 
    }
    
    switch( settings.numberstyle ) 
    {
      // all numbers as digits
      case 0: {
        const char *nums[12] =  
        {
          "12", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11"
        };
        const char *romans[12] = 
        {
          "XII", "I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX", "X", "XI"
        };
        for (int i=0; i<12; i+=1) 
        {
	        angle = DEG_TO_TRIGANGLE(i*30);
        inner = mark_point(center, bounds, angle, settings.numberpos, settings.numberrim);
	        // Draw number
          if( settings.numberset == 0 ) {
            graphics_draw_text(ctx, nums[i], font, GRect(inner.x-textsize, inner.y-(textsize/2)-2, 2*textsize, textsize), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
          } else {
            graphics_draw_text(ctx, romans[i], font, GRect(inner.x-textsize, inner.y-(textsize/2)-2, 2*textsize, textsize), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
          }  
 	      }
        break;
      }
      // Leave one out
      case 1: {
        const char *nums[6] =  
        {
          "12", "2", "4", "6", "8", "10"
        };
        const char *romans[6] = 
        {
          "XII", "II", "IV", "VI", "VIII", "X"
        };
        for (int i=0; i<6; i+=1) 
        {
	        angle = DEG_TO_TRIGANGLE(i*60);
        inner = mark_point(center, bounds, angle, settings.numberpos, settings.numberrim);
	        // Draw number
          if( settings.numberset == 0 ) {
            graphics_draw_text(ctx, nums[i], font, GRect(inner.x-textsize, inner.y-(textsize/2)-2, 2*textsize, textsize), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
          } else {
            graphics_draw_text(ctx, romans[i], font, GRect(inner.x-textsize, inner.y-(textsize/2)-2, 2*textsize, textsize), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
          }  
 	      }
        break;
      }
      // Leave the other out
      case 2: {
        const char *nums[6] =  
        {
          "1", "3", "5", "7", "9", "11"
        };
        const char *romans[6] = 
        {
          "I", "III", "V", "VII", "IX", "XI"
        };
        for (int i=0; i<6; i+=1) 
        {
	        angle = DEG_TO_TRIGANGLE((i*60)+30);
        inner = mark_point(center, bounds, angle, settings.numberpos, settings.numberrim);
	        // Draw number
          if( settings.numberset == 0 ) {
            graphics_draw_text(ctx, nums[i], font, GRect(inner.x-textsize, inner.y-(textsize/2)-2, 2*textsize, textsize), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
          } else {
            graphics_draw_text(ctx, romans[i], font, GRect(inner.x-textsize, inner.y-(textsize/2)-2, 2*textsize, textsize), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
          }  
 	      }
        break;
      }
      // 4 numbers only
      case 3: {
        const char *nums[4] = 
        {
          "12", "3", "6", "9"
        };
        const char *romans[4] = 
        {
          "XII", "III", "VI", "IX"
        };
        for (int i=0; i<4; i+=1) 
        {
	        angle = DEG_TO_TRIGANGLE(i*90);
        inner = mark_point(center, bounds, angle, settings.numberpos, settings.numberrim);
	        // Draw number
          if( settings.numberset == 0 ) {
            graphics_draw_text(ctx, nums[i], font, GRect(inner.x-textsize, inner.y-(textsize/2)-2, 2*textsize, textsize), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
          } else {
            graphics_draw_text(ctx, romans[i], font, GRect(inner.x-textsize, inner.y-(textsize/2)-2, 2*textsize, textsize), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
          }  
 	      }
        break;
      }
      // 3 numbers V1 (12,3,6)
      case 4: {
        const char *nums[3] = 
        {
          "12", "3", "6"
        };
        const char *romans[3] = 
        {
          "XII", "III", "VI"
        };
        for (int i=0; i<3; i+=1) 
        {
	        angle = DEG_TO_TRIGANGLE(i*90);
        inner = mark_point(center, bounds, angle, settings.numberpos, settings.numberrim);
	        // Draw number
          if( settings.numberset == 0 ) {
            graphics_draw_text(ctx, nums[i], font, GRect(inner.x-textsize, inner.y-(textsize/2)-2, 2*textsize, textsize), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
          } else {
            graphics_draw_text(ctx, romans[i], font, GRect(inner.x-textsize, inner.y-(textsize/2)-2, 2*textsize, textsize), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
          }  
 	      }
        break;
      }
      // 3 numbers V2 (12,6,9)
      case 5: {
        const char *nums[3] = 
        {
          "6", "9", "12"
        };
        const char *romans[3] = 
        {
          "VI", "IX", "XII"
        };
        for (int i=0; i<3; i+=1) 
        {
	        angle = DEG_TO_TRIGANGLE((i*90)+180);
        inner = mark_point(center, bounds, angle, settings.numberpos, settings.numberrim);
	        // Draw number
          if( settings.numberset == 0 ) {
            graphics_draw_text(ctx, nums[i], font, GRect(inner.x-textsize, inner.y-(textsize/2)-2, 2*textsize, textsize), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
          } else {
            graphics_draw_text(ctx, romans[i], font, GRect(inner.x-textsize, inner.y-(textsize/2)-2, 2*textsize, textsize), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
          }  
 	      }
        break;
      }
      // 3 numbers V3 (12,3,9)
      case 6: {
        const char *nums[3] = 
        {
          "9", "12", "3"
        };
        const char *romans[3] = 
        {
          "IX", "XII", "III"
        };
        for (int i=0; i<3; i+=1) 
        {
	        angle = DEG_TO_TRIGANGLE((i*90)+270);
        inner = mark_point(center, bounds, angle, settings.numberpos, settings.numberrim);
	        // Draw number
          if( settings.numberset == 0 ) {
            graphics_draw_text(ctx, nums[i], font, GRect(inner.x-textsize, inner.y-(textsize/2)-2, 2*textsize, textsize), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
          } else {
            graphics_draw_text(ctx, romans[i], font, GRect(inner.x-textsize, inner.y-(textsize/2)-2, 2*textsize, textsize), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
          }  
 	      }
        break;
      }
      // 3 numbers V4 (3,6,9)
      case 7: {
        const char *nums[3] = 
        {
          "3", "6", "9"
        };
        const char *romans[3] = 
        {
          "III", "XI", "IX"
        };
        for (int i=0; i<3; i+=1) 
        {
	        angle = DEG_TO_TRIGANGLE((i*90)+90);
        inner = mark_point(center, bounds, angle, settings.numberpos, settings.numberrim);
	        // Draw number
          if( settings.numberset == 0 ) {
            graphics_draw_text(ctx, nums[i], font, GRect(inner.x-textsize, inner.y-(textsize/2)-2, 2*textsize, textsize), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
          } else {
            graphics_draw_text(ctx, romans[i], font, GRect(inner.x-textsize, inner.y-(textsize/2)-2, 2*textsize, textsize), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
          }  
 	      }
        break;
      }
      // 2 numbers V1 (12,6)
      case 8: {
        const char *nums[2] = 
        {
          "12", "6"
        };
        const char *romans[2] = 
        {
          "XII", "XI"
        };
        for (int i=0; i<2; i+=1) 
        {
	        angle = DEG_TO_TRIGANGLE(i*180);
        inner = mark_point(center, bounds, angle, settings.numberpos, settings.numberrim);
	        // Draw number
          if( settings.numberset == 0 ) {
            graphics_draw_text(ctx, nums[i], font, GRect(inner.x-textsize, inner.y-(textsize/2)-2, 2*textsize, textsize), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
          } else {
            graphics_draw_text(ctx, romans[i], font, GRect(inner.x-textsize, inner.y-(textsize/2)-2, 2*textsize, textsize), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
          }  
 	      }
        break;
      }
      // 2 numbers V2 (3,9)
      case 9: {
        const char *nums[2] = 
        {
          "3", "9"
        };
        const char *romans[2] = 
        {
          "III", "IX"
        };
        for (int i=0; i<2; i+=1) 
        {
	        angle = DEG_TO_TRIGANGLE((i*180)+90);
        inner = mark_point(center, bounds, angle, settings.numberpos, settings.numberrim);
	        // Draw number
          if( settings.numberset == 0 ) {
            graphics_draw_text(ctx, nums[i], font, GRect(inner.x-textsize, inner.y-(textsize/2)-2, 2*textsize, textsize), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
          } else {
            graphics_draw_text(ctx, romans[i], font, GRect(inner.x-textsize, inner.y-(textsize/2)-2, 2*textsize, textsize), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
          }  
 	      }
        break;
      }
      // only twelve as digit
      case 10: {
        angle = DEG_TO_TRIGANGLE(0);
        inner = mark_point(center, bounds, angle, settings.numberpos, settings.numberrim);
        // Draw number
        if( settings.numberset == 0 ) {
          graphics_draw_text(ctx, "12", font, GRect(inner.x-textsize, inner.y-(textsize/2)-2, 2*textsize, textsize), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
        } else {
          graphics_draw_text(ctx, "XII", font, GRect(inner.x-textsize, inner.y-(textsize/2)-2, 2*textsize, textsize), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
        }  
        break;
      }
      default: break;
    }  
  }
  
  // moon phase
  if( settings.showmoonphase == 1 ) {
    angle = DEG_TO_TRIGANGLE(settings.moonposdegree);
    inner_pos = bounds.size.w*settings.moonpositionrim/200;
    inner.y = (int16_t)(-cos_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.y;
    inner.x = (int16_t)(sin_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.x;

    draw_moon_phase(ctx, inner, settings.moonradius, settings.moonlightcolor, settings.moondarkcolor,
                     settings.moonborder == 1, settings.moonbordercolor, settings.moonhemisphere == 1);
  }

  // steps above the marks but below the hands (the default layer)
  if( settings.stepsontop == 0 ) {
    draw_steps(ctx, center, bounds);
  }

  if( settings.fullscreen == 0 ) {
    graphics_context_set_stroke_color(ctx, GColorBlack);
    graphics_context_set_stroke_width(ctx, 3 );
    graphics_draw_circle(ctx, center, (bounds.size.w/2)+1);
  } 
  /* else {
    graphics_context_set_stroke_color(ctx, settings.backcolor);
    graphics_context_set_stroke_width(ctx, 3 );
    graphics_draw_circle(ctx, center, (bounds.size.w/2)+1);
  }
  */  
  
}
  
static void app_focus_changing(bool focusing) {
	if (focusing) {
	   layer_set_hidden(window_get_root_layer(s_window), true); 
	}
}

static void app_focus_changed(bool focused) {
  if (focused) {
    layer_set_hidden(window_get_root_layer(s_window), false); 
    layer_mark_dirty(window_get_root_layer(s_window));
  }
}

static void window_load(Window *window) {

  load_settings();
  
  GRect bounds = layer_get_bounds(window_get_root_layer(s_window));

  s_dot_layer = layer_create(bounds);
  layer_set_update_proc(s_dot_layer, dot_update_proc);
  layer_add_child(window_get_root_layer(s_window), s_dot_layer);  
  
  s_hands_layer = layer_create(bounds);
  layer_set_update_proc(s_hands_layer, hands_update_proc);
  layer_add_child(window_get_root_layer(s_window), s_hands_layer);
  
  app_focus_service_subscribe_handlers((AppFocusHandlers){
	  .did_focus = app_focus_changed,
	  .will_focus = app_focus_changing
	});
  
  if( settings.showsecond == 1 ) {
    tick_timer_service_subscribe(SECOND_UNIT, handle_time_tick );
  }
  else {
    tick_timer_service_subscribe(MINUTE_UNIT, handle_time_tick );
  }
}

static void window_unload(Window *window) {
}

static void init() {
  s_window = window_create();
  
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  
  //Register AppMessage events
  app_message_register_inbox_received(in_received_handler);
  AppMessageResult msg_result = APP_MSG_OUT_OF_MEMORY;
  for( unsigned i = 0; i < ARRAY_LENGTH(APP_MSG_INBOX_SIZES); i++ ) {
    msg_result = app_message_open(APP_MSG_INBOX_SIZES[i], APP_MSG_OUTBOX_SIZE);
    if( msg_result == APP_MSG_OK ) {
      if( i > 0 ) {
        APP_LOG(APP_LOG_LEVEL_INFO, "inbox fell back to %d bytes (dictionary is %d)",
                APP_MSG_INBOX_SIZES[i], APP_MSG_DICT_BYTES);
      }
      if( APP_MSG_INBOX_SIZES[i] < APP_MSG_DICT_BYTES ) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "inbox %d < dictionary %d - settings will be dropped",
                APP_MSG_INBOX_SIZES[i], APP_MSG_DICT_BYTES);
      }
      break;
    }
  }
  if( msg_result != APP_MSG_OK ) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "app_message_open failed (%d) - settings cannot arrive", msg_result);
  }

  window_stack_push(s_window, true);

  small_sign = gpath_create(&SMALL_SIGN);
  medium_sign = gpath_create(&MEDIUM_SIGN);
  huge_sign = gpath_create(&HUGE_SIGN);
  
}

static void deinit() {
  save_settings();
  app_focus_service_unsubscribe();
  tick_timer_service_unsubscribe();
  gpath_destroy(small_sign);
  gpath_destroy(medium_sign);
  gpath_destroy(huge_sign);

  window_destroy(s_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}