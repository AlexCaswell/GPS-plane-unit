void setup() {
  Serial.begin(9600);

}


String addZeros(int num) {
  String pnum = String(num);
  String zeros = "0";
  if(num < 100 && num >= 10) {
      zeros = "00";
  }
  if(num < 10) {
    zeros = "000";
  }
  if(num >= 1000) {
    return pnum;
  }
  zeros.concat(pnum);
  return zeros;
}

void loop() {
  Serial.print(addZeros(pulseIn(2, HIGH))); Serial.print(' ');
  Serial.print(addZeros(pulseIn(3, HIGH))); Serial.print(' ');
  Serial.print(addZeros(pulseIn(4, HIGH))); Serial.print(' ');
  Serial.print(addZeros(pulseIn(5, HIGH))); Serial.print(' '); Serial.println();
}
