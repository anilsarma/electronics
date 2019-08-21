template <typename T> class BufferedData {
  T value;
  long t0;
  long time; // time period

  public:
  BufferedData(T value_, long time_period_):value(value_), time(time_period_)
  {}
  
  T get() { return value; }
  void set(T value_) { value = value_; t0 = millis(); Serial.println(value_); }

  bool is_stale() {
     long t1 = millis();
     return ( t1-t0) > time?true:false;
  }
};
