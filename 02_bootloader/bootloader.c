/* Private includes ----------------------------------------------------------*/
#include <string.h>
#include <stdbool.h>

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;

/* Private user code ---------------------------------------------------------*/
void erase_flash(void)
{
    FLASH_EraseInitTypeDef erase;
    uint32_t error = 0;
    erase.TypeErase = FLASH_TYPEERASE_SECTORS;
    erase.Sector = FLASH_SECTOR_2;
    erase.NbSectors = 6;
    erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;

    HAL_FLASH_Unlock();
    HAL_FLASHEx_Erase(&erase, &error);
    HAL_FLASH_Lock();
}

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
    // Initialize
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_USART2_UART_Init();

    // Start
    uint8_t res[50];
    uint8_t rcv[256];
    uint8_t boot[] = "Hello from bootloader.\n";
    uint8_t jump[] = "Jumping to application.\n";

    if (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET)
    {
        HAL_UART_Transmit(&huart2, boot, sizeof(boot), 100);
    }
    else
    {
        uint32_t *reg = (uint32_t *)0x08008004;
        if (*reg == 0xFFFFFFFFU)
        {
            uint8_t emsg[] = "No application code.\n";
            HAL_UART_Transmit(&huart2, emsg, sizeof(emsg), 500);
        }
        else
        {
            HAL_UART_Transmit(&huart2, jump, sizeof(jump), 500);
            void (*application)(void) = (void *)(*reg);

            // 1. Set MSP
            uint32_t msp_value = *(uint32_t *)0x08008000;
            __set_MSP(msp_value);
            // 2. Jump to application
            application();
        }
    }

    while (1)
    {
        memset(res, 0, sizeof(res));
        memset(rcv, 0, sizeof(res));

        HAL_UART_Receive(&huart2, rcv, 4, 200);
        if (strcmp((char *)rcv, "0x11") == 0)
        {
            erase_flash();
            strcpy((char *)res, "ACK\n");
            HAL_UART_Transmit(&huart2, res, sizeof(res), 100);
        }
        else if (strcmp((char *)rcv, "0x21") == 0)
        {
            // addr + len + data
            // 8 + 1 + len(data), now one byte
            uint8_t buff[256];
            uint16_t length;
            uint32_t addr = 0;
            uint32_t *data;
            HAL_StatusTypeDef status = HAL_OK;

            HAL_UART_Receive(&huart2, rcv, 200, 200);
            memcpy(buff, rcv, 200);

            for (int i = 0; i < 4; i++)
            {
                addr <<= 8;
                addr |= buff[i];
            }
            length = buff[4];

            HAL_FLASH_Unlock();
            for (int i = 0; i < length; i += 4)
            {
                // 32bit word
                data = (uint32_t *)(&buff[i + 5]);
                status |= HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr + i, *data);
            }
            HAL_FLASH_Lock();

            // Set response
            if (status == HAL_OK)
            {
                strcpy((char *)res, "ACK\n\0");
            }
            else
            {
                strcpy((char *)res, "NACK\n\0");
            }
            HAL_UART_Transmit(&huart2, res, strlen((char *)res), 100);
        }
    }
}
