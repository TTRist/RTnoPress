#pragma once

class HX711_verKi
{
  public:
    HX711_verKi(uint8_t DT, uint8_t SCK) {
      _DT = DT;
      _SCK = SCK;
      pinMode(_SCK, OUTPUT);
      pinMode(_DT, INPUT);

      _offset = 0;
    };
    
    float readDataAve(int ave_num) {
      long sum = 0;
      for (int i = 0; i < ave_num; i++) sum += readData();
      float data = sum / (float)ave_num;
      float volt = data * (4.2987 / 16777216.0 / 128);
      float gram = volt / (0.000669 * 4.2987 / 200.0);

      // gram調整
      gram = gram * 5000 / 313; // <- 確かここで調整。gram以下を削除し、gram * 調整用重り重量 / そのときの値?
      return gram - _offset;
    }
    
    long readData() {
      long data = 0;
      while (digitalRead(_DT));
      for (char i = 0; i < 24; i++) {
        digitalWrite(_SCK, 1); delayMicroseconds(1);
        digitalWrite(_SCK, 0); delayMicroseconds(1);
        data = (data << 1) | digitalRead(_DT);
      }
      //gain=128
      digitalWrite(_SCK, 1); delayMicroseconds(1);
      digitalWrite(_SCK, 0); delayMicroseconds(1);
      data = data ^ 0x800000;
      return data;
    }

    void setOffset(int ave_num){
      _offset = 0;
      _offset = readDataAve(50);
    }

  private:
    uint8_t _DT;
    uint8_t _SCK;
    float _offset;
};
