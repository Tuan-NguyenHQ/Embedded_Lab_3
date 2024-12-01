/*
 * system.c
 *
 *  Created on: Nov 24, 2023
 *      Author: huaho
 */

#include "system.h"

#define MAX_POINTS 20  // Số điểm tối đa trên biểu đồ (giới hạn bởi màn hình)
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320
#define GRAPH_WIDTH 200
#define GRAPH_HEIGHT 60
#define GRAPH_X_START 20
#define GRAPH_Y_START 240

#define BACKGROUND_COLOR 0xFFFF // Trắng
#define LINE_COLOR 0x001F       // Xanh
#define POINT_COLOR 0xF800      // Đỏ

uint16_t currentValues[MAX_POINTS]; // Lưu các giá trị dòng điện
uint8_t numPoints = 0;             // Số điểm hiện tại trong mảng


int volval = 99;//duty cycle of volume
uint8_t isnotify = 0;//to notify to computer
int uart_print = 0;

void LCD_show_sensor();

/*
 * @brief:	update sensor value
 * @para:	none
 * @retval:	none
 * */
void sensor_diplay(){
	  if(timer_flag[0] == 1){
		  set_timer(0, READ_BUTTON_TIME);
		  ds3231_ReadTime();
		  button_Scan();
		  sensor_Read();
		  //Check Potentiometer value and send notify
		  if(sensor_GetPotentiometer() >= 4095 * 0.7){
			  uart_print = 1;
			  if(isnotify == 1){
				  buzzer_SetVolume(volval);
				  if(!is_timer_on(4)){
					  set_timer(4, ONE_SECOND);
					  isnotify = 0;
	  				  uart_Rs232SendString("Potentiometer > 70%\n");
	  				  uart_Rs232SendString("Please reduce Potentiometer\n\n");
				  }
			  }
			  else if(isnotify == 0){
				  buzzer_SetVolume(0);
				  if(!is_timer_on(4)){
					  set_timer(4, ONE_SECOND);
					  isnotify = 1;
				  }
			  }
	  	  }
		  else{
			  uart_print = 0;
			  buzzer_SetVolume(0);
		  }

		  //change volume value
		  if(button_count[11] == 1){
			  volval += 10;
			  if(volval > 99){
				  volval = 0;
			  }
			  lcd_ShowIntNum(10, 300, volval, 2, WHITE, BLACK, 16);
		  }
		  if(button_count[15] == 1){
			  volval -= 10;
			  if(volval < 0){
				  volval = 99;
			  }
			  lcd_ShowIntNum(10, 300, volval, 2, WHITE, BLACK, 16);
		  }
	  }
	  //show sensor value to LCD
	  if(timer_flag[3] == 1){
		  set_timer(3, LCD_SENSOR_TIME);
		  LCD_show_sensor();
	  }
}

/*
 * @brief:	show sensor value to screen
 * @para:	none
 * @retval:	none
 * */
void LCD_show_sensor(){
	lcd_ShowStr(10, 100, "Voltage(V):", WHITE, BLACK, 16, 0);
	lcd_ShowFloatNum(130, 100, sensor_GetVoltage(), 4, WHITE, BLACK, 16);

	lcd_ShowStr(10, 120, "Current(mA):", WHITE, BLACK, 16, 0);
	lcd_ShowFloatNum(130, 120, sensor_GetCurrent(), 4, WHITE, BLACK, 16);

	lcd_ShowStr(10, 140, "Power(mW):", WHITE, BLACK, 16, 0);
	lcd_ShowFloatNum(130, 140, sensor_GetCurrent() * sensor_GetVoltage(), 5, WHITE, BLACK, 16);

	lcd_ShowStr(10, 160, "Light:", WHITE, BLACK, 16, 0);
	if(sensor_GetLight() <= 4095*0.75){
			lcd_ShowStr(60, 160, "(Strong)", WHITE, BLACK, 16, 0);
		}
		else{
			lcd_ShowStr(60, 160, "(Weak)  ", WHITE, BLACK, 16, 0);
		}
	lcd_ShowIntNum(130, 160, sensor_GetLight(), 4, WHITE, BLACK, 16);


	lcd_ShowStr(10, 180, "Poten(Ohm):", WHITE, BLACK, 16, 0);
	lcd_ShowIntNum(130, 180, sensor_GetPotentiometer()*100/4095, 2, WHITE, BLACK, 16);
	lcd_ShowStr(180, 180, "%", WHITE, BLACK, 16, 0);

	lcd_ShowStr (10, 200, "Temp(C):", WHITE, BLACK, 16, 0);
	lcd_ShowFloatNum (130, 200, sensor_GetTemperature(), 4, WHITE, BLACK, 16);
}


void uart_test(){
	if(!is_timer_on(5)){
			  set_timer(5, ONE_SECOND*5);
			  uart_Rs232SendString("Voltage: ");
			  int t = sensor_GetVoltage();
			  uart_Rs232SendNum(t);
			  uart_Rs232SendString("\n");

			  uart_Rs232SendString("Current: ");
			  t = sensor_GetCurrent();
			  uart_Rs232SendNum(t);
			  uart_Rs232SendString("\n");

			  uart_Rs232SendString("Power: ");
			  t = sensor_GetCurrent() * sensor_GetVoltage();
			  uart_Rs232SendNum(t);
			  uart_Rs232SendString("\n");

			  uart_Rs232SendString("Light: ");
			  t = sensor_GetLight();
			  uart_Rs232SendNum(t);
			  uart_Rs232SendString("\n");

			  uart_Rs232SendString("Poter: ");
			  t = sensor_GetPotentiometer()*100/4095;
			  uart_Rs232SendNum(t);
			  uart_Rs232SendString("\n");

			  uart_Rs232SendString("Temperature: ");
			  t = sensor_GetTemperature();
			  uart_Rs232SendNum(t);
			  uart_Rs232SendString("\n");
		  }
}


void drawGraph() {
    // Xóa vùng biểu đồ
    lcd_Fill(GRAPH_X_START, GRAPH_Y_START, GRAPH_X_START + GRAPH_WIDTH, GRAPH_Y_START + GRAPH_HEIGHT, BACKGROUND_COLOR);

    // Vẽ khung biểu đồ
    lcd_DrawRectangle(GRAPH_X_START, GRAPH_Y_START, GRAPH_X_START + GRAPH_WIDTH, GRAPH_Y_START + GRAPH_HEIGHT, LINE_COLOR);

    // Vẽ các điểm trên biểu đồ
    for (uint8_t i = 0; i < numPoints - 1; i++) {
        uint16_t x1 = GRAPH_X_START + (i * GRAPH_WIDTH) / MAX_POINTS;
        uint16_t y1 = GRAPH_Y_START + GRAPH_HEIGHT - (currentValues[i] * GRAPH_HEIGHT) / (4096 * 20) ; // Chuẩn hóa giá trị
        uint16_t x2 = GRAPH_X_START + ((i + 1) * GRAPH_WIDTH) / MAX_POINTS;
        uint16_t y2 = GRAPH_Y_START + GRAPH_HEIGHT - (currentValues[i + 1] * GRAPH_HEIGHT) / (4096 * 20);

        lcd_DrawLine(x1, y1, x2, y2, LINE_COLOR); // Vẽ đường nối giữa 2 điểm
        lcd_DrawPoint(x1, y1, POINT_COLOR);      // Hiển thị điểm
    }

    // Hiển thị giá trị tại điểm cuối cùng
    if (numPoints > 0) {
        char str[10];
        sprintf(str, "%d mA", currentValues[numPoints - 1]); // Giá trị cuối cùng
        lcd_ShowStr(GRAPH_X_START + GRAPH_WIDTH + 10, GRAPH_Y_START + 10, str, LINE_COLOR, BACKGROUND_COLOR, 12, 1);
    }
}

void updateGraph() {
    // Đọc giá trị dòng điện
    uint16_t power = sensor_GetCurrent()*sensor_GetVoltage();

    // Lưu giá trị vào mảng
    if (numPoints < MAX_POINTS) {
        currentValues[numPoints++] = power;
    } else {
        // Nếu đầy, dịch các giá trị sang trái
        for (uint8_t i = 1; i < MAX_POINTS; i++) {
            currentValues[i - 1] = currentValues[i];
        }
        currentValues[MAX_POINTS - 1] = power;
    }

    // Vẽ lại biểu đồ
    drawGraph();
}


void system_loop(void) {
	sensor_diplay();
	fsm_clock();
	if(uart_print == 0) uart_test();
	if(timer_flag[6] == 1){
		set_timer(6, 3000);
		updateGraph();
	}

}

