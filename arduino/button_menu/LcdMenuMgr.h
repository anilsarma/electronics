#ifndef __LCD_MENUMGR_H_
#define __LCD_MENUMGR_H_
#include "ClickMgr.h"
//https://github.com/fdebrabander/Arduino-LiquidCrystal-I2C-library
typedef const String (*StatusFunction)();
typedef void (*OnClickFunction)(bool btnState, bool longPress);

typedef void (*EditFunction)(bool btnState, bool longPress, int direction, bool save);
const char MENU_UP[] = { 24, 0 };
const char MENU_DOWN[] = { 25, 0 };
const char MENU_RIGHT[] = { 26, 0 };
const char MENU_LEFT[] = { 27, 0 };

class MenuItem;
class MenuDetails;

class LcdMenuCallback {
  public:
    virtual void update_lcd(MenuDetails*, MenuItem *item) = 0;
    virtual void sleep() = 0;
    virtual void wakeup() = 0;
};
class MenuItem {
    String name;


    boolean edit = false; // currently editing by pressing click.

    EditFunction e_fn;
    StatusFunction s_fn;
    OnClickFunction a_fn;

    MenuDetails* menu = NULL;
  public:
    MenuItem(String name_, StatusFunction s, OnClickFunction a, EditFunction e_fn_ = NULL) :
      name(name_), s_fn(s), a_fn(a),  e_fn(e_fn_) {
    }

    MenuItem(MenuDetails* menu_): menu(menu_) {

    }
    MenuDetails* get_menu() {
      return menu;
    }
    String get_name();
    bool is_editable() {

      return e_fn != NULL;
    }

    bool get_edit_mode() {
      return edit;
    }

    void set_mode(bool mode_) {
      edit = mode_;
    }

    void set_value(bool clicked, bool longPress, int direction, bool save ) {
      if (e_fn != NULL) {
        e_fn(clicked, longPress, direction, save);
      }
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
    String name;
    int pos = 0;
    int count = 0;
    int dummy = 0;

    MenuDetails(const String name_) {
      name = name_;
      count = 0;
      pos = 0;
    }
    MenuDetails* get_parent() {
      return parent;
    }

    void add_parent(MenuDetails* parent_) {
      this->parent = parent_;
    }
    String get_name() {
      return name;
    }
    void add(String item) {
      add(item, NULL, NULL);
    }

    void add(String item, StatusFunction s_fn, OnClickFunction a_fn, EditFunction e_fn_ = NULL) {
      items[count] = new MenuItem(item, s_fn, a_fn, e_fn_);
      count++;
    }

    void add(MenuDetails* menu_) {
      items[count] = new MenuItem(menu_);
      count ++;
    }

    //    MenuItem *get(int index) {
    //      return items[index];
    //    }

    MenuItem *get_item() {
      if ( pos < 0) {
        return NULL;
      }
      return items[pos];
    }
    void set_pos(int pos_) { pos= pos_; }
    MenuItem *update_position(int upd) {
      if (upd == 0) {
        return get_item();
      }
      pos += upd;
      pos = (pos >= count) ? count - 1 : pos;
      if (parent == NULL) {
        pos = (pos < 0) ? 0 : pos;
      } else {
        pos = (pos < -1) ? -1 : pos;
      }
      return get_item();
    }

};
extern LiquidCrystal_I2C lcd;
class MenuMgr {
    MenuDetails main;
    MenuDetails *current = NULL;
    int m0 = 0; // move time
    int t0 = 0;
    int lastEvent = 0;
    int lastPosition = 0;
    ClickMgr clickmgr;
    LcdMenuCallback *callback;
    bool in_sleep = false;
    int  idle_timeout = 30000;
  public:
    MenuMgr(LcdMenuCallback* callback_, int idle_timeout_): main("MENU"), callback(callback_), idle_timeout(idle_timeout_) {
      current = &main;
    }

    MenuDetails& get_menu() {
      return main;
    }

    void loop(int position, boolean btn_state) {
      int t1 = millis();

      if ((t1 - t0) < 100) {
        return;
      }
      //      lcd.setCursor(6, 1);
      //      lcd.print(t1 - t0);

      int click_state = clickmgr.loop(btn_state);
      bool clicked = (click_state > 0);
      bool longPressed  = (click_state & ClickMgr::LONG_PRESSED) > 0;
      //      lcd.print(" "); lcd.print(clicked);
      //      if (longPressed) {
      //        lcd.print(" "); lcd.print(longPressed);
      //        Serial.println("in long press");
      //      }
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
      if ( increment == 0 && !clicked) {
        if (( t1 - lastEvent) > idle_timeout) {
          if (!in_sleep) {
            callback->sleep();
          }
          in_sleep = true;
        }
      } else {
        lastEvent = t1;
        if (in_sleep) {
          callback->wakeup(); // wakeup the LCD,
          in_sleep = false;
          return;// user cannot see anything right now.
        }
      }
      // where are we.
      MenuItem *item = current->get_item();

      bool save_required = false;
      if ( item == NULL ) {
        if (clicked) {          
          
          current = current->get_parent();
        }
      } else {
        if (clicked ) {
          if(item->get_menu()) {
              current = item->get_menu();
              current->set_pos(0);
              return;
          } else {
          Serial.print("CLKE: " );
          Serial.println((long )item, HEX );
          item->call_action( clicked, longPressed);
          if (item->is_editable()) {
            item->set_mode(!item->get_edit_mode());
            increment = 0;
            save_required = item->get_edit_mode() ? false : true;
          }
          }
        }
        if ( save_required ) {
          item->set_value(clicked, longPressed, increment, true);
        }
        if (item->is_editable() && item->get_edit_mode()) { //currently in edit mode
          item->set_value(clicked, longPressed, increment, false);
        } else {
          item = current->update_position(increment);
        }
      }
      update_menu(current, item);
    }

    void update_menu(MenuDetails* current, MenuItem *item) {
      if (callback != NULL) {
        callback->update_lcd(current, item);
      }
    }
};

String MenuItem::get_name()
{
  if (menu != NULL) {
    return menu->get_name();
  }
  return name;
}

#endif
