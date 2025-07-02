// Main function
int main(void) {
  while (1) {
    // This is a simple example to blink an LED connected to on-board LD2 pin
    HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
    HAL_Delay(50);
  }
}