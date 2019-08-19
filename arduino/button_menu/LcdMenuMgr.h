#ifndef __LCD_MENUMGR_H_
#define __LCD_MENUMGR_H_
typedef const String (*status)();
typedef void (*action)(bool btnState, bool longPress);

class MenuItem {
    String  name;
    boolean editable = false;
    int     value = 0;
    boolean edit = false; // currently editing by pressing click.
    status s_fn;
    action a_fn;

  public:
    MenuItem(String name_, status s, action a, bool editable_ = false, int value_ = 0):
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
      return value;
    }
    void set_value(int value_) {
      value = value_;
    }
};
class MenuDetails {
    MenuDetails * parent = NULL;
    MenuItem *items[20];

  public:
    char * name;
    int pos = 0;
    int count = 0;
    MenuDetails() {
      name = "MENU";
      count = 0;
      pos = 0;
    }
    void add(char *item) {
      add(item, NULL, NULL);
    }
    void add(char* item, status func, action atn) {
      //strcpy(items[count], item); // = item;
      items[count] = new MenuItem(item, func, atn);
      count ++;
    }
    MenuItem* get(int index) {
      return items[index];
    }

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
class MenuMgr {
    MenuDetails main;
    MenuDetails* current = NULL;
    int t0 = 0;
    int lastPosition = 0;
  public:
    MenuMgr() {
      current = &main;
    }

    void loop(int position, boolean click, boolean longPress) {
      // read the
      int t1 = millis();
      if ( (t1 - t0) < 500) {
        if(longPress)  {
            position = lastPosition;
        } else {
            return; // ignore this
        }
      }
      bool increase = (position > lastPosition);
      bool decrease = (position < lastPosition);

      // where are we.
      MenuItem* item = current->get_item();

      if(click && item->is_editable()) {
        item->set_mode(!item->get_edit_mode());
        increase=decrease = false;
      }
      if ( item->is_editable() && item->get_edit_mode()  ) { //currently in edit mode
        item->set_value( item->get_value() + increase ? 1 : decrease ? -1 : 0);
      } else {
        item = current->update_position( increase ? 1 : decrease ? -1 : 0);
      }

      update_menu(item);
    }

    void update_menu(MenuItem *item) {
      Serial.print(item->get_name());
      if ( item->is_editable()) {
        Serial.print(item->get_value());
      }
    }
};
#endif
