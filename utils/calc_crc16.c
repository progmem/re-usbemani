#include <stdlib.h>
#include <stdio.h>
#include "config.h"

uint16_t crc16_update(uint16_t crc, uint8_t a) {
  crc ^= a;

  for (uint8_t i = 0; i < 8; i++) {
    if (crc & 1)
      crc = (crc >> 1) ^ 0xA001;
    else
      crc >>= 1;
  }

  return crc;
}

int main(void) {
  uint16_t crc16, i, filesize;

  Config_t config;
  memset(&config, 0, sizeof(config));

  FILE *file = fopen("test.eep", "rb");
  fseek(file, 0L, SEEK_END);
  filesize = ftell(file);
  fseek(file, 0L, SEEK_SET);
  fread(&config, sizeof(config), 1, file);

  uint8_t *device_bytes = (uint8_t *)&config.Device;
  uint8_t *user_bytes   = (uint8_t *)&config.User;

  printf("%lu :: %lu\n", sizeof(CONFIG_DATASOURCE), sizeof(PS2_INPUT));

  crc16 = 0xffff;
  for (i = 0; i < sizeof(config.Device); ++i) {
    printf(".");
    crc16 = crc16_update(crc16, device_bytes[i]);
  }
  printf("\n%04x\n", crc16);
  config.Header.crc16_device = crc16;

  crc16 = 0xffff;
  for (uint16_t i = 0; i < sizeof(config.User); i++) {
    printf(".");
    crc16 = crc16_update(crc16, user_bytes[i]);
  }
  printf("\n%04x\n", crc16);
  config.Header.crc16_user = crc16;

  FILE *out = fopen("out.eep", "wb");
  fwrite(&config, sizeof(config), 1, out);

  return 0;
}