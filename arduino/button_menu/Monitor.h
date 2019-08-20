#ifndef __MONITOR_H__
#define __MONITOR_H__
class Monitor {
  
  bool monitoring = false;
  bool pause = false;
  long t0 = 0;
  int runtime = 0;
  int pausetime = 0;
  
  public:
  enum State { STOPPED, STARTED, PAUSED };
  
  Monitor(int runtime_, int pausetime_): runtime(runtime_), pausetime(pausetime_)
  {}

  int get_runtime() {
    return runtime;
  
  }
  void set_runtime(int runtime_) {
    runtime = runtime_;
  }

  int get_pausetime() {
    return pausetime;
  
  }
  void set_pausetime(int pausetime_) {
    pausetime = pausetime_;
  }
  void monitor(bool mon) {
    mon?start_monitor():stop_monitor();
  }
  void start_monitor() {
    monitoring = true;
    pause = false;
    t0 = millis();
  }

  void stop_monitor() {
    monitoring = false;
    pause = false;
  }

  int get_time() {
    if(!monitoring) {
      return -1;
    }
    return millis()-t0;
  }
  State check() {
    if(!monitoring) {
      return STOPPED;
    }

    long t1 = millis();
    int interval = pause?pausetime:runtime;
    if(( t1-t0) > interval) {
      if( pause ) {
        pause = false;
      } else {
        // in paused
        pause = true;
      }
      t0 = t1;
    }
    return pause?PAUSED:STARTED;
  }
};

#endif
