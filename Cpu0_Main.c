#include "main.h"
#include "IO/GPIO.h"
#include "my_stdio.h"
#include "asclin.h"
#include "gpt12.h"
#include "Motor.h"
#include "Drivers/asclin.h"
#include "IO/ToF.h"
#include "Ultrasonic.h"

IfxCpu_syncEvent g_cpuSyncEvent = 0;

int core0_main (void)
{
    IfxCpu_enableInterrupts();

    /* !!WATCHDOG0 AND SAFETY WATCHDOG ARE DISABLED HERE!!
     * Enable the watchdogs and service them periodically if it is required
     */
    IfxScuWdt_disableCpuWatchdog(IfxScuWdt_getCpuWatchdogPassword());
    IfxScuWdt_disableSafetyWatchdog(IfxScuWdt_getSafetyWatchdogPassword());

    /* Wait for CPU sync event */
    IfxCpu_emitEvent(&g_cpuSyncEvent);
    IfxCpu_waitEvent(&g_cpuSyncEvent, 1);

    /* Module Initialize */
    Init_Mystdio();
    _init_uart3();
    _init_uart1();
    Init_Ultrasonics();
    Init_GPIO();
    init_gpt2();
    Init_DCMotors();
    Init_Buzzer();

    unsigned char ch;
    int res = 0;
    int A_dir = 1, B_dir = 1;
    int A_duty = 0, B_duty = 0;
    int duty = 0;
    char save_ch;
    int distance;
    float U_distance;

    while (1)
    {
        res = _poll_uart3(&ch);
        distance = getTofDistance();
        my_printf("distance : %d\n", distance);
        if (save_ch == 'k' || save_ch == 'K')
        {
            U_distance = ReadRearUltrasonic_noFilt();
            delay_ms(20);
            my_printf("U_distance : %f\n", U_distance);
            if (U_distance > 25.0f)
            {
                A_duty = 25;
                B_duty = 25;

            }
            else if (U_distance <= 5.0f)
            {
                A_duty = 0;
                B_duty = 0;
                setBeepCycle(1);
            }
            else if (U_distance <= 15.0f)
            {
                A_duty = 15;
                B_duty = 15;
                setBeepCycle(50);
            }
            else if (U_distance <= 25.0f)
            {
                A_duty = 22;
                B_duty = 22;
                setBeepCycle(80);
            }
           // my_printf("A_duty : %d ", A_duty);
           // my_printf("B_duty : %d\n", B_duty);
        }

       if ((distance <= 150 && distance > 0) && save_ch == 'p' || save_ch == 'P')
        {
            A_duty = 0;
            B_duty = 0;
        }


        if (res)
        {
            if (ch == 'i' || ch == 'I')
            {
                //go
                A_duty = 30;
                B_duty = 30;
                A_dir = 1;
                B_dir = 1;
            }

            else if (ch == 'k' || ch == 'K')
            {
                //back
                A_duty = 25;
                B_duty = 25;
                A_dir = 0;
                B_dir = 0;

            }
            else if (ch == 'm' || ch == 'M')
            {
                //stop
                A_duty = 0;
                B_duty = 0;
            }
            else if (ch == 'l' || ch == 'L')
            {
                //left

                A_duty = 40;
                B_duty = 35;
                A_dir = 1;
                B_dir = 0;
            }
            else if (ch == 'j' || ch == 'J')
            {
                //right

                A_duty = 35;
                B_duty = 40;
                A_dir = 0;
                B_dir = 1;
            }
            else if (ch == 'p' || ch == 'P')
            {
                //fast

                A_duty = 70;
                B_duty = 70;
                A_dir = 1;
                B_dir = 1;
            }

            if (save_ch != ch)
            {
                movChA_PWM(80, A_dir);
                movChB_PWM(80, B_dir);
                delay_ms(20);
            }
            save_ch = ch;
        }

        movChA_PWM(A_duty, A_dir);
        movChB_PWM(B_duty, B_dir);

    }

    return 0;
}
