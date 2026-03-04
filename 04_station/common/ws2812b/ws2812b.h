#ifndef WS2812B_H
#define WS2812B_H


void WS2812b_Init(int tx_pin, int number_led);
void WS2812b_Set_Color(int index, int r, int g, int b);
void WS2812b_Update_Color(void);


#endif