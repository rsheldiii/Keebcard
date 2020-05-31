#include "settings.h"
#include "BusinessCard.h"

void businessCard(SSD1306Device* _oled) {
    int state = 0;

    _oled->setMemoryAddressingMode(0);
    _oled->clear();

    while(1){
      int old_state = state;
      if (digitalRead(LEFT_BUTTON) == LOW) state = 1;
      else if (digitalRead(MIDDLE_BUTTON) == LOW) state = 2;
      else if (digitalRead(RIGHT_BUTTON) == LOW) state = 3;
      else state = 4;

      if(old_state != state) {
        if(state == 0 || state == 4) {
          _oled->clear();
          _oled->setCursor(25, 1);
          _oled->print("FALLS GEEK");
        }
        else if(state == 2) {          
          _oled->clear();
          _oled->setCursor(5, 0);
          _oled->print("fallsgeek.com");
          _oled->setCursor(5, 2);
          _oled->print("940-400-0411");

        }
        else if(state == 1) {
          _oled->clear();
          _oled->setCursor(5, 0);
          _oled->print("Web Design");
          _oled->setCursor(5,2);
          _oled->print("Computer Repair");
        }
        else if(state == 3) {  
          _oled->clear();
          _oled->setCursor(5, 0);
          _oled->print("3D Printing");
          _oled->setCursor(5,2);
          _oled->print("Laser Engraving");
        }
     }
  }
}
