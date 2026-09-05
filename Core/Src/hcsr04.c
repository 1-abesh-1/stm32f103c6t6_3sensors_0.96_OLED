#include "hcsr04.h"

extern TIM_HandleTypeDef htim2;

#define HCSR04_TRIG_PORT GPIOA
#define HCSR04_TRIG_PIN  GPIO_PIN_12

#define HCSR04_ECHO_PORT GPIOA
#define HCSR04_ECHO_PIN  GPIO_PIN_15


/* ----------------------------------------------------------
   Microsecond delay using TIM2
   ---------------------------------------------------------- */
static void HCSR04_DelayUs(uint16_t us)
{
    __HAL_TIM_SET_COUNTER(&htim2, 0);

    while (__HAL_TIM_GET_COUNTER(&htim2) < us)
    {
    }
}


/* ----------------------------------------------------------
   Read distance in centimeters
   Returns 0 if measurement fails.
   ---------------------------------------------------------- */
uint16_t HCSR04_ReadDistance(void)
{
    uint32_t start;
    uint32_t end;
    uint32_t pulse_width;


    /* ------------------------------------------------------
       1. Make sure TRIG is LOW
       ------------------------------------------------------ */
    HAL_GPIO_WritePin(
        HCSR04_TRIG_PORT,
        HCSR04_TRIG_PIN,
        GPIO_PIN_RESET
    );

    HCSR04_DelayUs(2);


    /* ------------------------------------------------------
       2. Send 10 us trigger pulse
       ------------------------------------------------------ */
    HAL_GPIO_WritePin(
        HCSR04_TRIG_PORT,
        HCSR04_TRIG_PIN,
        GPIO_PIN_SET
    );

    HCSR04_DelayUs(10);

    HAL_GPIO_WritePin(
        HCSR04_TRIG_PORT,
        HCSR04_TRIG_PIN,
        GPIO_PIN_RESET
    );


    /* ------------------------------------------------------
       3. Wait for ECHO to become HIGH
       ------------------------------------------------------ */

    __HAL_TIM_SET_COUNTER(&htim2, 0);

    while (HAL_GPIO_ReadPin(
               HCSR04_ECHO_PORT,
               HCSR04_ECHO_PIN
           ) == GPIO_PIN_RESET)
    {
        if (__HAL_TIM_GET_COUNTER(&htim2) > 30000)
        {
            return 0;
        }
    }

    start = __HAL_TIM_GET_COUNTER(&htim2);


    /* ------------------------------------------------------
       4. Wait for ECHO to become LOW
       ------------------------------------------------------ */

    while (HAL_GPIO_ReadPin(
               HCSR04_ECHO_PORT,
               HCSR04_ECHO_PIN
           ) == GPIO_PIN_SET)
    {
        if ((__HAL_TIM_GET_COUNTER(&htim2) - start) > 30000)
        {
            return 0;
        }
    }

    end = __HAL_TIM_GET_COUNTER(&htim2);


    /* ------------------------------------------------------
       5. Calculate pulse width
       ------------------------------------------------------ */

    pulse_width = end - start;


    /* ------------------------------------------------------
       6. Convert microseconds to centimeters

       Distance = time / 58

       because approximately:

       1 cm round trip ≈ 58 us
       ------------------------------------------------------ */

    return (uint16_t)(pulse_width / 58);
}