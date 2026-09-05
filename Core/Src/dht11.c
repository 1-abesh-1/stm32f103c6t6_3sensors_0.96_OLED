#include "dht11.h"

extern TIM_HandleTypeDef htim2;

#define DHT11_PORT GPIOA
#define DHT11_PIN  GPIO_PIN_5


/* Change PA5 to output */
static void DHT11_SetOutput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
}


/* Change PA5 to input */
static void DHT11_SetInput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;

    HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
}


/* Microsecond delay using TIM2 */
static void DHT11_DelayUs(uint16_t us)
{
    __HAL_TIM_SET_COUNTER(&htim2, 0);

    while (__HAL_TIM_GET_COUNTER(&htim2) < us)
    {
    }
}


/* Wait until PA5 becomes the requested state */
static uint8_t DHT11_WaitForPin(GPIO_PinState state, uint16_t timeout)
{
    uint16_t start = __HAL_TIM_GET_COUNTER(&htim2);

    while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) != state)
    {
        uint16_t now = __HAL_TIM_GET_COUNTER(&htim2);

        if ((uint16_t)(now - start) > timeout)
            return 0;
    }

    return 1;
}


uint8_t DHT11_Read(uint8_t *temperature, uint8_t *humidity)
{
    uint8_t data[5] = {0};
    uint8_t i;

    /*
     * 1. MCU sends start signal
     */

    DHT11_SetOutput();

    /* Pull LOW for at least 18 ms */
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_RESET);
    HAL_Delay(18);

    /* Release line */
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET);

    /* Wait 20-40 us */
    DHT11_DelayUs(30);


    /*
     * 2. Let DHT11 control the line
     */

    DHT11_SetInput();


    /*
     * 3. DHT11 response:
     *
     * LOW  ~80 us
     * HIGH ~80 us
     */

    if (!DHT11_WaitForPin(GPIO_PIN_RESET, 100))
        return 0;

    if (!DHT11_WaitForPin(GPIO_PIN_SET, 100))
        return 0;

    if (!DHT11_WaitForPin(GPIO_PIN_RESET, 100))
        return 0;


    /*
     * 4. Read 40 bits
     */

    for (i = 0; i < 40; i++)
    {
        /*
         * Each bit starts with approximately
         * 50 us LOW.
         */

        if (!DHT11_WaitForPin(GPIO_PIN_SET, 100))
            return 0;


        /*
         * HIGH duration:
         *
         * ~26 us = 0
         * ~70 us = 1
         *
         * Wait 40 us and sample.
         */

        DHT11_DelayUs(40);

        if (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_SET)
        {
            data[i / 8] |= (1 << (7 - (i % 8)));
        }


        /*
         * Wait for HIGH pulse to finish
         */

        if (!DHT11_WaitForPin(GPIO_PIN_RESET, 100))
            return 0;
    }


    /*
     * 5. Check checksum
     */

    if ((uint8_t)(data[0] +
                  data[1] +
                  data[2] +
                  data[3]) != data[4])
    {
        return 0;
    }


    /*
     * 6. Extract values
     */

    *humidity = data[0];
    *temperature = data[2];

    return 1;
}