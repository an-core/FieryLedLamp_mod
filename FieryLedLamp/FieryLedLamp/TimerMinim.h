// ***************************************************************************** TimerMinim.h ************************************************************

class timerMinim {
  public:
    timerMinim(uint32_t interval);         // объявление таймера с указанием интервала
    void setInterval(uint32_t interval);   // установка интервала работы таймера
    bool isReady();                        // возвращает true, когда пришло время. Сбрасывается в false сам (AUTO) или вручную (MANUAL)
    void reset();                          // ручной сброс таймера на установленный интервал

  private:
    uint32_t _timer = 0;
    uint32_t _interval = 0;
}; // class timerMinim

timerMinim::timerMinim(uint32_t interval) {
  _interval = interval;
  _timer = millis();
}

void timerMinim::setInterval(uint32_t interval) {
  _interval = interval;
}

bool timerMinim::isReady() {
  if ((uint32_t)millis() - _timer >= _interval) {
    _timer = millis();
    return true;
  } // if ((uint32_t)millis() - _timer >= _interval)
  else {
    return false;
  }
} // bool timerMinim::isReady()

void timerMinim::reset() {
  _timer = millis();
}

// *******************************************************************************************************************************************************
