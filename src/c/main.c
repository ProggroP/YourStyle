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
} ClaySettings;

// An instance of the struct
static ClaySettings settings;

// Persistent storage key
#define SETTINGS_KEY 1

// Initialize the default settings
static void default_settings() {
  settings.backcolor = GColorWhite;
  settings.hourdialcolor = GColorBlack;
  settings.minsdialcolor = GColorBlack;
  settings.secsdialcolor = GColorBlack;
  settings.minsmarkcolor = GColorBlack;
  settings.hourmarkcolor = GColorBlack;
  settings.fullscreen    = 1;
  settings.showsecond    = 0;
  settings.showminsmark  = 1;
  settings.showhourmark  = 1;
  settings.hourlength    = 65;
  settings.minslength    = 85;
  settings.secslength    = 90;
  settings.hourbalancelength = 0;
  settings.minsbalancelength = 0;
  settings.secsbalancelength = 20;
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
}

// Read settings from persistent storage
static void load_settings() {
  // Load the default settings
  default_settings();

  // Read settings from persistent storage, if they exist
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}

// Save the settings to persistent storage
static void save_settings() {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
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


static void hands_update_proc(Layer *layer, GContext *ctx) {

  time_t now = time(NULL);
  struct tm *t = localtime(&now);

  int hour = t->tm_hour%12;
  int min = t->tm_min;
  int sec = t->tm_sec;
  int day = t->tm_mday;
  
  int angle = 0;
 
  GPoint inner, outer;
  int inner_pos, outer_pos;
  GRect bounds  = layer_get_bounds(s_hands_layer);
  GPoint center = GPoint( bounds.size.w/2, bounds.size.h/2);
    
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
  if( settings.hourlength > 0 || settings.hourbalancelength > 0 ) {
    angle = DEG_TO_TRIGANGLE((hour*30)+(min*5/10));
    inner_pos = -1*(bounds.size.w*settings.hourbalancelength/200);
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
    graphics_fill_circle(ctx, center, settings.hourthickness-1);
  }
 
  // hour hand filling
  if( settings.hourfillinnerpos > 0 || settings.hourfillouterpos > 0 ) {
    angle = DEG_TO_TRIGANGLE((hour*30)+(min*5/10));
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
  if( settings.minslength > 0 || settings.minsbalancelength > 0 ) {
    angle = DEG_TO_TRIGANGLE(min*6);
    inner_pos = -1*(bounds.size.w*settings.minsbalancelength/200);
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
    // draw circle 
    graphics_fill_circle(ctx, center, settings.minsthickness-1);
  }
  
  // minute hand filling
  if( settings.minsfillinnerpos > 0 || settings.minsfillouterpos > 0 ) {
    angle = DEG_TO_TRIGANGLE(min*6);
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
  if( settings.secslength > 0 || settings.secsbalancelength > 0 ) {
    if( settings.showsecond == 1 ) {
      angle = DEG_TO_TRIGANGLE(sec*6);
      inner_pos = -1*(bounds.size.w*settings.secsbalancelength/200);
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
      // draw circle 
      graphics_fill_circle(ctx, center, settings.secsthickness-1);
    }  
  }
  
  // seconds hand filling
  if( settings.secsfillinnerpos > 0 || settings.secsfillouterpos > 0 ) {
    if( settings.showsecond == 1 ) {
      angle = DEG_TO_TRIGANGLE(sec*6);
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
 
  // dot in the middle
  if( settings.hourlength > 0 || settings.minslength > 0 || settings.secslength > 0) {
    graphics_context_set_fill_color(ctx, GColorDarkGray);
    graphics_context_set_stroke_width(ctx, 1);
    graphics_fill_circle(ctx, center, 2);
  }  
}

static void dot_update_proc(Layer *layer, GContext *ctx) {
  
  GRect bounds  = layer_get_bounds(s_dot_layer);
  GPoint center = GPoint( bounds.size.w/2, bounds.size.h/2);
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

  // minute marks
  if( settings.showminsmark == 1 ) {
    inner_pos = bounds.size.w*settings.minsmarkinnerpos/200;
    outer_pos = bounds.size.w*settings.minsmarkouterpos/200;
    graphics_context_set_fill_color(ctx, settings.minsmarkcolor);
    graphics_context_set_stroke_color(ctx, settings.minsmarkcolor);
    graphics_context_set_stroke_width(ctx, settings.minsmarkthickness );
    switch( settings.minutemarkstyle ) {
      // all marks
      case 0: { 
	      for (int i=0; i<=60; i+=1) {
		      angle = DEG_TO_TRIGANGLE(i*6);
	       	inner.y = (int16_t)(-cos_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.y;
		      inner.x = (int16_t)(sin_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.x;
		      outer.y = (int16_t)(-cos_lookup(angle) * (int32_t)outer_pos / TRIG_MAX_RATIO) + center.y;
		      outer.x = (int16_t)(sin_lookup(angle) * (int32_t)outer_pos / TRIG_MAX_RATIO) + center.x;
		      // Draw tick mark
          graphics_draw_line(ctx, inner, outer);
        }  
        break;
      }
      // every fifth minute
      case 1: { 
	      for (int i=0; i<=60; i+=5) {
		      angle = DEG_TO_TRIGANGLE(i*6);
	       	inner.y = (int16_t)(-cos_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.y;
		      inner.x = (int16_t)(sin_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.x;
		      outer.y = (int16_t)(-cos_lookup(angle) * (int32_t)outer_pos / TRIG_MAX_RATIO) + center.y;
		      outer.x = (int16_t)(sin_lookup(angle) * (int32_t)outer_pos / TRIG_MAX_RATIO) + center.x;
		      // Draw tick mark
          graphics_draw_line(ctx, inner, outer);
        }  
        break;
      }
      // every fifteenth minute
      case 2: { 
	      for (int i=0; i<=60; i+=15) {
		      angle = DEG_TO_TRIGANGLE(i*6);
	       	inner.y = (int16_t)(-cos_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.y;
		      inner.x = (int16_t)(sin_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.x;
		      outer.y = (int16_t)(-cos_lookup(angle) * (int32_t)outer_pos / TRIG_MAX_RATIO) + center.y;
		      outer.x = (int16_t)(sin_lookup(angle) * (int32_t)outer_pos / TRIG_MAX_RATIO) + center.x;
		      // Draw tick mark
          graphics_draw_line(ctx, inner, outer);
        }  
        break;
      }
      // all with a 3 minute gap around all hours
      case 3: { 
	      for (int i=2; i<60; i+=5) {
		      angle = DEG_TO_TRIGANGLE(i*6);
	       	inner.y = (int16_t)(-cos_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.y;
		      inner.x = (int16_t)(sin_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.x;
		      outer.y = (int16_t)(-cos_lookup(angle) * (int32_t)outer_pos / TRIG_MAX_RATIO) + center.y;
		      outer.x = (int16_t)(sin_lookup(angle) * (int32_t)outer_pos / TRIG_MAX_RATIO) + center.x;
		      // Draw tick mark
          graphics_draw_line(ctx, inner, outer);
        }  
	      for (int i=3; i<60; i+=5) {
		      angle = DEG_TO_TRIGANGLE(i*6);
	       	inner.y = (int16_t)(-cos_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.y;
		      inner.x = (int16_t)(sin_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.x;
		      outer.y = (int16_t)(-cos_lookup(angle) * (int32_t)outer_pos / TRIG_MAX_RATIO) + center.y;
		      outer.x = (int16_t)(sin_lookup(angle) * (int32_t)outer_pos / TRIG_MAX_RATIO) + center.x;
		      // Draw tick mark
          graphics_draw_line(ctx, inner, outer);
        }  
        break;
      }
      // All with a 3 minute gap around 4 hours
      case 4: { 
	      for (int i=2; i<14; i+=1) {
		      angle = DEG_TO_TRIGANGLE(i*6);
	       	inner.y = (int16_t)(-cos_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.y;
		      inner.x = (int16_t)(sin_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.x;
		      outer.y = (int16_t)(-cos_lookup(angle) * (int32_t)outer_pos / TRIG_MAX_RATIO) + center.y;
		      outer.x = (int16_t)(sin_lookup(angle) * (int32_t)outer_pos / TRIG_MAX_RATIO) + center.x;
		      // Draw tick mark
          graphics_draw_line(ctx, inner, outer);
        }  
	      for (int i=17; i<29; i+=1) {
		      angle = DEG_TO_TRIGANGLE(i*6);
	       	inner.y = (int16_t)(-cos_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.y;
		      inner.x = (int16_t)(sin_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.x;
		      outer.y = (int16_t)(-cos_lookup(angle) * (int32_t)outer_pos / TRIG_MAX_RATIO) + center.y;
		      outer.x = (int16_t)(sin_lookup(angle) * (int32_t)outer_pos / TRIG_MAX_RATIO) + center.x;
		      // Draw tick mark
          graphics_draw_line(ctx, inner, outer);
        }  
	      for (int i=32; i<44; i+=1) {
		      angle = DEG_TO_TRIGANGLE(i*6);
	       	inner.y = (int16_t)(-cos_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.y;
		      inner.x = (int16_t)(sin_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.x;
		      outer.y = (int16_t)(-cos_lookup(angle) * (int32_t)outer_pos / TRIG_MAX_RATIO) + center.y;
		      outer.x = (int16_t)(sin_lookup(angle) * (int32_t)outer_pos / TRIG_MAX_RATIO) + center.x;
		      // Draw tick mark
          graphics_draw_line(ctx, inner, outer);
        }  
	      for (int i=47; i<59; i+=1) {
		      angle = DEG_TO_TRIGANGLE(i*6);
	       	inner.y = (int16_t)(-cos_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.y;
		      inner.x = (int16_t)(sin_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.x;
		      outer.y = (int16_t)(-cos_lookup(angle) * (int32_t)outer_pos / TRIG_MAX_RATIO) + center.y;
		      outer.x = (int16_t)(sin_lookup(angle) * (int32_t)outer_pos / TRIG_MAX_RATIO) + center.x;
		      // Draw tick mark
          graphics_draw_line(ctx, inner, outer);
        }  
        break;
      }
      // all with a 3 minute gap around 12
      case 5: { 
	      for (int i=2; i<59; i+=15) {
		      angle = DEG_TO_TRIGANGLE(i*6);
	       	inner.y = (int16_t)(-cos_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.y;
		      inner.x = (int16_t)(sin_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.x;
		      outer.y = (int16_t)(-cos_lookup(angle) * (int32_t)outer_pos / TRIG_MAX_RATIO) + center.y;
		      outer.x = (int16_t)(sin_lookup(angle) * (int32_t)outer_pos / TRIG_MAX_RATIO) + center.x;
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
    inner_pos = bounds.size.w*settings.hourmarkinnerpos/200;
    outer_pos = bounds.size.w*settings.hourmarkouterpos/200;
    graphics_context_set_fill_color(ctx, settings.hourmarkcolor);
    graphics_context_set_stroke_color(ctx, settings.hourmarkcolor);
    graphics_context_set_stroke_width(ctx, settings.hourmarkthickness );
    switch( settings.hourmarkstyle ) {
      // all marks
      case 0: { 
	      for (int i=0; i<=12; i+=1) {
		      angle = DEG_TO_TRIGANGLE(i*30);
		      inner.y = (int16_t)(-cos_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.y;
		      inner.x = (int16_t)(sin_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.x;
		      outer.y = (int16_t)(-cos_lookup(angle) * (int32_t)outer_pos / TRIG_MAX_RATIO) + center.y;
		      outer.x = (int16_t)(sin_lookup(angle) * (int32_t)outer_pos / TRIG_MAX_RATIO) + center.x;
		      // Draw tick mark
          graphics_draw_line(ctx, inner, outer);
        }
        break;
   	  }
      // Leave one out (1, 3, 5, ...)
      case 1: { 
	      for (int i=1; i<=12; i+=2) {
		      angle = DEG_TO_TRIGANGLE(i*30);
		      inner.y = (int16_t)(-cos_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.y;
		      inner.x = (int16_t)(sin_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.x;
		      outer.y = (int16_t)(-cos_lookup(angle) * (int32_t)outer_pos / TRIG_MAX_RATIO) + center.y;
		      outer.x = (int16_t)(sin_lookup(angle) * (int32_t)outer_pos / TRIG_MAX_RATIO) + center.x;
		      // Draw tick mark
          graphics_draw_line(ctx, inner, outer);
        }
        break;
   	  }
      // Leave one out (12, 2, 4, ...)
      case 2: { 
	      for (int i=0; i<=12; i+=2) {
		      angle = DEG_TO_TRIGANGLE(i*30);
		      inner.y = (int16_t)(-cos_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.y;
		      inner.x = (int16_t)(sin_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.x;
		      outer.y = (int16_t)(-cos_lookup(angle) * (int32_t)outer_pos / TRIG_MAX_RATIO) + center.y;
		      outer.x = (int16_t)(sin_lookup(angle) * (int32_t)outer_pos / TRIG_MAX_RATIO) + center.x;
		      // Draw tick mark
          graphics_draw_line(ctx, inner, outer);
        }
        break;
   	  }
      // Between 4 hour numbers (1,2,4,5,...)
      case 3: { 
	      for (int i=1; i<=12; i+=3) {
		      angle = DEG_TO_TRIGANGLE(i*30);
		      inner.y = (int16_t)(-cos_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.y;
		      inner.x = (int16_t)(sin_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.x;
		      outer.y = (int16_t)(-cos_lookup(angle) * (int32_t)outer_pos / TRIG_MAX_RATIO) + center.y;
		      outer.x = (int16_t)(sin_lookup(angle) * (int32_t)outer_pos / TRIG_MAX_RATIO) + center.x;
		      // Draw tick mark
          graphics_draw_line(ctx, inner, outer);
        }
	      for (int i=2; i<=12; i+=3) {
		      angle = DEG_TO_TRIGANGLE(i*30);
		      inner.y = (int16_t)(-cos_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.y;
		      inner.x = (int16_t)(sin_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.x;
		      outer.y = (int16_t)(-cos_lookup(angle) * (int32_t)outer_pos / TRIG_MAX_RATIO) + center.y;
		      outer.x = (int16_t)(sin_lookup(angle) * (int32_t)outer_pos / TRIG_MAX_RATIO) + center.x;
		      // Draw tick mark
          graphics_draw_line(ctx, inner, outer);
        }
        break;
   	  }
      // All except of twelve
      case 4: { 
	      for (int i=1; i<12; i+=1) {
		      angle = DEG_TO_TRIGANGLE(i*30);
		      inner.y = (int16_t)(-cos_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.y;
		      inner.x = (int16_t)(sin_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.x;
		      outer.y = (int16_t)(-cos_lookup(angle) * (int32_t)outer_pos / TRIG_MAX_RATIO) + center.y;
		      outer.x = (int16_t)(sin_lookup(angle) * (int32_t)outer_pos / TRIG_MAX_RATIO) + center.x;
		      // Draw tick mark
          graphics_draw_line(ctx, inner, outer);
        }
        break;
   	  }
      default: break;
    }  
  }
  
  if( settings.shownumbers == 1 ) {
    inner_pos = bounds.size.w*settings.numberpos/200;
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
	        inner.y = (int16_t)(-cos_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.y;
	        inner.x = (int16_t)(sin_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.x;
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
	        inner.y = (int16_t)(-cos_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.y;
	        inner.x = (int16_t)(sin_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.x;
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
	        inner.y = (int16_t)(-cos_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.y;
	        inner.x = (int16_t)(sin_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.x;
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
	        inner.y = (int16_t)(-cos_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.y;
	        inner.x = (int16_t)(sin_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.x;
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
	        inner.y = (int16_t)(-cos_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.y;
	        inner.x = (int16_t)(sin_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.x;
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
	        inner.y = (int16_t)(-cos_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.y;
	        inner.x = (int16_t)(sin_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.x;
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
	        inner.y = (int16_t)(-cos_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.y;
	        inner.x = (int16_t)(sin_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.x;
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
	        inner.y = (int16_t)(-cos_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.y;
	        inner.x = (int16_t)(sin_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.x;
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
	        inner.y = (int16_t)(-cos_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.y;
	        inner.x = (int16_t)(sin_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.x;
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
	        inner.y = (int16_t)(-cos_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.y;
	        inner.x = (int16_t)(sin_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.x;
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
        inner.y = (int16_t)(-cos_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.y;
        inner.x = (int16_t)(sin_lookup(angle) * (int32_t)inner_pos / TRIG_MAX_RATIO) + center.x;
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
  //Largest possible input and output buffer sizes  
  app_message_open(1280,1280);

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