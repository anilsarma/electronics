
#ifndef __CLICK_MGR_H__
#define __CLICK_MGR_H__
extern LiquidCrystal_I2C lcd;
class ClickMgr {
  long t0;
  bool btn_pressed=false;
  public:
  static const int LONG_PRESSED= 1<<2;

  public:
  ClickMgr():t0(0) {
    
  }


  int loop(bool btn_state) {
      int clicked = 0;
      if(btn_state) {
        if(!btn_pressed) {
          btn_pressed = true;
          t0 = millis();
        }
      } else {
        if(btn_pressed) {
          btn_pressed  =false;
          long t1 = millis();
          clicked = 1;
          if((t1-t0)> 5000) {
            // long press
           clicked |= LONG_PRESSED;
          
          }
        }
      }
      return clicked;
  }
};
#endif
