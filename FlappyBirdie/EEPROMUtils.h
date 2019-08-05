#ifndef EEPROMUTILS_H
#define EEPROMUTILS_H
#define EEPROM_START_C1  EEPROM_STORAGE_SPACE_START
#define EEPROM_START_C2  EEPROM_START_C1 + 1
#define EEPROM_SCORE     EEPROM_START_C1 + 2

void initEEPROM() {
  unsigned char c1 = EEPROM.read(EEPROM_START_C1);
  unsigned char c2 = EEPROM.read(EEPROM_START_C2);

  if(c1 != 'L' || c2 != 'Z') {
    EEPROM.update(EEPROM_START_C1, 'L');
    EEPROM.update(EEPROM_START_C2, 'Z');
    EEPROM.put(EEPROM_SCORE, (unsigned int)0);
  }
}
#endif
