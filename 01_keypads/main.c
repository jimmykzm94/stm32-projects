// Variables
GPIO_TypeDef *rowPorts[2] = {ROW1_GPIO_Port, ROW2_GPIO_Port};
uint16_t rowPins[2] = {ROW1_Pin, ROW2_Pin};
GPIO_TypeDef *colPorts[5] = {COL1_GPIO_Port, COL2_GPIO_Port, COL3_GPIO_Port,
                             COL4_GPIO_Port, COL5_GPIO_Port};
uint16_t colPins[5] = {COL1_Pin, COL2_Pin, COL3_Pin, COL4_Pin, COL5_Pin};
const char keys[2][5] = {{'1', '2', '3', '4', '5'}, {'6', '7', '8', '9', '0'}};

// Functions
static uint8_t scanKey() {
    for (int col = 0; col < 5; col++) {
        HAL_GPIO_WritePin(colPorts[col], colPins[col], GPIO_PIN_SET);
    }
    for (int col = 0; col < 5; col++) {
        HAL_GPIO_WritePin(colPorts[col], colPins[col], GPIO_PIN_RESET);
        HAL_Delay(1); // allow time for the signal to stabilize
        for (int row = 0; row < 2; row++) {
        GPIO_PinState state = HAL_GPIO_ReadPin(rowPorts[row], rowPins[row]);
        if (state == GPIO_PIN_RESET) {
            HAL_Delay(20); // debounce delay
            while (HAL_GPIO_ReadPin(rowPorts[row], rowPins[row]) == GPIO_PIN_RESET);
            return keys[row][col];
        }
        }
        HAL_GPIO_WritePin(colPorts[col], colPins[col], GPIO_PIN_SET);
    }
    return 99; // return 99 if no key is pressed
}

// Main function
int main(void) {
    while (1) {
        // scan keypad
        HAL_Delay(50);
        uint8_t key = scanKey();
        if (key != 99)
            HAL_UART_Transmit(&huart2, &key, 1, 500);
    }
}
