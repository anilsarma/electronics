#ifndef __LCD_MENUMGR_H_
#define __LCD_MENUMGR_H_
#include "ClickMgr.h"
//https://github.com/fdebrabander/Arduino-LiquidCrystal-I2C-library
typedef const String (*status)();

typedef void (*action)(bool btnState, bool longPress);
class MenuItem;
class LcdMenuCallback {
  public:
    virtual void update_lcd(MenuItem *item) = 0;
};
class MenuItem {
    String name;
    boolean editable = false;
    int *value = 0;
    boolean edit = false; // currently editing by pressing click.
    status s_fn;
    action a_fn;
    

  public:
    MenuItem(String name_, status s, action a, bool editable_ = false, int *value_=NULL) :
      name(name_), s_fn(s), a_fn(a), editable(editable_), value(value_) {
    }

    String get_name() {
      return name;
    }

    bool is_editable() {
      return editable;
    }

    bool get_edit_mode() {
      return edit;
    }

    void set_mode(bool mode_) {
      edit = mode_;
    }

    int get_value() {
      return *value;
    }

    void set_value(int value_) {
      *value = value_;
    }

    String get_status() {
      if (s_fn == NULL) {
        return "";
      }
      return s_fn();
    }

    void call_action(bool clicked, bool longPressed) {
      if (a_fn != NULL) {
        a_fn(clicked, longPressed);
      }
    }
};

class MenuDetails {
    MenuDetails *parent = NULL;
    MenuItem *items[20];

  public:
    char *name;
    int pos = 0;
    int count = 0;
    int dummy=0;

    MenuDetails() {
      name = "MENU";
      count = 0;
      pos = 0;
    }

    void add(char *item) {
      add(item, NULL, NULL);
    }

    void add(String item, status func, action atn, bool editable_=false, int *value_=NULL) {
      items[count] = new MenuItem(item, func, atn,editable_, value_);
      count++;
    }

    //    MenuItem *get(int index) {
    //      return items[index];
    //    }

    MenuItem *get_item() {
      return items[pos];
    }

    MenuItem *update_position(int upd) {
      if (upd == 0) {
        return get_item();
      }
      pos += upd;
      pos = (pos >= count) ? count - 1 : pos;
      pos = (pos < 0) ? 0 : pos;
      return get_item();
    }

};
extern LiquidCrystal_I2C lcd;
class MenuMgr {
    MenuDetails main;
    MenuDetails *current = NULL;
    int m0 = 0; // move time
    int t0 = 0;
    int lastPosition = 0;
    ClickMgr clickmgr;
    LcdMenuCallback *callback;
  public:
    MenuMgr(LcdMenuCallback* callback_): callback(callback_) {
      current = &main;
    }

    MenuDetails& get_menu() {
      return main;
    }

    void loop(int position, boolean btn_state) {
      int t1 = millis();

      if ((t1 - t0) < 200) {
        return;
      }
      //      lcd.setCursor(6, 1);
      //      lcd.print(t1 - t0);

      int click_state = clickmgr.loop(btn_state);
      bool clicked = (click_state > 0);
      bool longPressed  = (click_state & ClickMgr::LONG_PRESSED) > 0;
      lcd.print(" "); lcd.print(clicked);
      if (longPressed) {
        lcd.print(" "); lcd.print(longPressed);
        Serial.println("in long press");
      }
      // read the
      if (longPressed || clicked) {
        lastPosition = position;
      }
      t0 = t1;

      int increment = (position > lastPosition) ? 1 : (position < lastPosition) ? -1 : 0;
      lastPosition = position;
      if (increment != 0) {
        if ( (t1 - m0) < 500) {
          increment = 0;
        } else {
          m0 = t1;

        }
      }

      //      {
      //        Serial.print("increment ");
      //        Serial.print(increment);
      //        Serial.print(" CLK:" );
      //        Serial.print(clicked );
      //        Serial.print(" LP:" );
      //        Serial.println(longPressed );
      //      }

      // where are we.
      MenuItem *item = current->get_item();

      if (clicked  ) {
        Serial.print("CLK: " );
        Serial.println((long )item, HEX );
        item->call_action( clicked, longPressed);
        if (item->is_editable()) {
          item->set_mode(!item->get_edit_mode());
          increment = 0;
        }
      }

      if (item->is_editable() && item->get_edit_mode()) { //currently in edit mode
        item->set_value(item->get_value() + increment);
      } else {
        item = current->update_position(increment);
      }

      update_menu(item);
    }

    void update_menu(MenuItem *item) {
//      Serial.print(item->get_name());
//      if (item->is_editable()) {
//        Serial.println(item->get_value());
//      } else {
//        Serial.println(item->get_status());
//      }

      if (callback != NULL) {
        callback->update_lcd(item);
      }
    }
};

#endif
