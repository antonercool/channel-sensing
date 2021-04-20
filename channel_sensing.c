#include "rssi_messurment.h"

/*----------------------------Build commands---------------------------------*/
// Make and upload telos b
// make TARGET=sky MOTES=/dev/ttyUSB0 channel_sensing.upload login
// Check serial port:
// - make TARGET=sky motelist
// compile and upload program
//-  make TARGET=sky PORT=/dev/ttyUSB0 channel_sensing.upload
// ssh on the sensor
// - make TARGET=sky PORT=/dev/ttyUSB0 login
// Clean build
//- make TARGET=sky distclean
// local sky build
// - make TARGET=sky

// Globals 
static int channel_RSSI_dBm[MAX_CHANNELS];

// Events
static process_event_t start_meassuring_event;
static process_event_t end_meassuring_event;

/*----------------------------Processes--------------------------------------*/
/*
    Holds the logic for communication between processes (Who starts when)
*/
PROCESS(main_process, "main_process");
/*
    Holds the logic for measuring the average RSSI in dBm over 10 times slots of 100 ms (128 clock ticks)
*/
PROCESS(rssi_process, "rssi_process");
AUTOSTART_PROCESSES(&main_process);
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(main_process, ev, data)
{

  static struct etimer timer;
  static int current_selected_channel;

  PROCESS_BEGIN();
  LOG_DBG("Init - main \n");

  // init rssi module
  init();

  // alloc unique event ids
  start_meassuring_event = process_alloc_event();
  end_meassuring_event = process_alloc_event();

  // wait for login from serial line
  etimer_set(&timer, CLOCK_SECOND * 5);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));

  for (current_selected_channel = 11; current_selected_channel <= 26; current_selected_channel++)
  {
    // start rssi measuring process
    process_start(&rssi_process, NULL);
    // post channel to messaure to process
    process_post(&rssi_process, start_meassuring_event, &current_selected_channel);
    // wait for result
    PROCESS_WAIT_EVENT_UNTIL(ev == end_meassuring_event);

    int rssi_dBm_average_10 = *((int *)data);
    channel_RSSI_dBm[current_selected_channel-11] = rssi_dBm_average_10;

    printf("RSSI Measurement : channel %d Averege RSSI dBm %d \n", current_selected_channel, rssi_dBm_average_10);
  }

  select_best_channel(channel_RSSI_dBm);
  LOG_DBG("Main finish \n");
  PROCESS_END();
}


PROCESS_THREAD(rssi_process, ev, data)
{

  static struct etimer timer;
  static int rssi_dBm_sum;
  static int counter;
  static int selected_channel;
  static int average_rssi_dBm;

  PROCESS_BEGIN();

  // wait for event from main
  PROCESS_WAIT_EVENT_UNTIL(ev == start_meassuring_event);

  selected_channel = *((int*) data);
  printf("RSSI Measurement : selected channel %d \n", selected_channel);

  rssi_dBm_sum = 0;
  for (counter = 0; counter < 10; counter++)
  {
    // Messuage 10 samples, each within 100ms time slot (128 ticks)
    etimer_set(&timer, (CLOCK_SECOND * 1) / 10);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    etimer_reset(&timer);

    rssi_dBm_sum = rssi_dBm_sum + get_rssi_dBm_from_channel(selected_channel);
  }

  // average the messured rssi values in dBm
  average_rssi_dBm = rssi_dBm_sum / 10;
  
  process_post(&main_process, end_meassuring_event, &average_rssi_dBm);

  PROCESS_END();
}