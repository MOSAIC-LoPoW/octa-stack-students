#include "scd30-example.h"
#include "scd30.h"

#define co2_timer    5

osTimerId co2_timer_id;
float co2_ppm, temperature, relative_humidity;

int main(void)
{
  Initialize_Platform();

  scd30_Initialize();

  scd30_set_measurement_interval(5);
  sensirion_sleep_usec(20000u);
  scd30_start_periodic_measurement(0);

  //feed IWDG every 5 seconds
  IWDG_feed(NULL);
  osTimerDef(iwdgTim, IWDG_feed);
  iwdgTimId = osTimerCreate(osTimer(iwdgTim), osTimerPeriodic, NULL);
  osTimerStart(iwdgTimId, 5 * 1000);

  osTimerDef(co2_Tim, co2_measurement);
  co2_timer_id = osTimerCreate(osTimer(co2_Tim), osTimerPeriodic, NULL);
  osTimerStart(co2_timer_id, co2_timer * 1000);

  osKernelStart();

  while (1)
  {
  }
}

void co2_measurement(void)
{
    int16_t err;
    if (scd30_probe() != NO_ERROR) {
      printf("SCD30 sensor probing failed\n");
      return;
    }

    uint16_t data_ready = 0;
    uint16_t timeout = 0;

    /* Poll data_ready flag until data is available. Allow 20% more than
    * the measurement interval to account for clock imprecision of the
    * sensor.
    */
    err = scd30_get_data_ready(&data_ready);
    if (err != NO_ERROR) {
        printf("Error reading data_ready flag: %i\n", err);
    }

    /* Measure co2, temperature and relative humidity and store into
      * variables.
      */
    err = scd30_read_measurement(&co2_ppm, &temperature, &relative_humidity);
    if (err != NO_ERROR) {
        printf("error reading measurement\n");
        return;
    } else {
        printf("co2: %0.2f ppm, "
                "temp: %0.2f C, "
                "hum: %0.2f %%RH\n",
                co2_ppm, temperature, relative_humidity);
    }
    osDelay(1);
}
