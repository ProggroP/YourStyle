#include <pebble.h>
    // small sign
    static const GPathInfo SMALL_SIGN = {
      8, (GPoint []){
        {   0,   0 },
        { - 3, -3 },
        {  3,  3 },
        {   0,  6 },
        {   0, -6 },
        {  3, -3 },
        { -3,  3 },
        {   0,   0 }  
      }
    };

    // medium sign
    static const GPathInfo MEDIUM_SIGN = {
      8, (GPoint []){
        {   0,   0 },
        { - 5, -5 },
        {  5,  5 },
        {   0,  10 },
        {   0, -10 },
        {  5, -5 },
        { -5,  5 },
        {   0,   0 }  
      }
    };
    
    // big sign
    static const GPathInfo HUGE_SIGN = {
      8, (GPoint []){
        {   0,   0 },
        { - 7, -7 },
        {  7,  7 },
        {   0,  14 },
        {   0, -14 },
        {  7, -7 },
        { -7,  7 },
        {   0,   0 }  
      }
    };

