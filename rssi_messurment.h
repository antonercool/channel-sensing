#include "contiki.h"
#include "CC2420.h"
#include "sys/log.h"
#include <limits.h>


#define LOG_MODULE "Sensing Log"
#define LOG_LEVEL LOG_LEVEL_DBG
#define MAX_CHANNELS 16

/*--------------prototypes---------------*/
/*
    Initialized the cc2420 module

    $$return$$
        void
*/
void init();


/*
    gets the rssi value in dBm from the selected channel

    $$Params$$
        channel : selected channel (Only valid with [11-26])

    $$return$$
        return The RSSI value converted to dBm. The value is always averaged over 8 symbol periods (128 μs) 
*/
int get_rssi_dBm_from_channel(int channel);


/*
    Selects the best channel (heighest dBm)

    $$Params$$
        rssi_values_dBm : averaged rssi_dBm values from channels [11-26]

    $$return$$
       void
*/
void select_best_channel(int rssi_values_dBm[]);


/*--------------Implementation--------*/
void init()
{
    // init the cc2420 moudle
    cc2420_init();
}


int get_rssi_dBm_from_channel(int channel)
{
    if (channel < 11 || channel > 26)
    {
        LOG_ERR("Channel is not supported \n");
        return -1;
    }
    
    // Turn on radio
    cc2420_on();

    // select channel
    cc2420_set_channel(channel);

    // cc2420_rssi() uses the follwing convertion from the datasheet to get it in dBm
    // rssi_dBm = RSSI_VALUE + RSSI_OFFSET;
    int current_RSSI_dBm = cc2420_rssi();

    // turn off radio to safe power
    cc2420_off();

    return current_RSSI_dBm;
}

void select_best_channel(int rssi_values_dBm[])
{
    int best_rssi_dBm = rssi_values_dBm[0];
    int best_channel = 0;
    int i;  
    for (i = 0; i < MAX_CHANNELS; i++)
    {
        if(rssi_values_dBm[i] > best_rssi_dBm)
        {
            best_rssi_dBm = rssi_values_dBm[i];
            best_channel = 11 + i;
        }
    }
    
    printf("RSSI Measurement : Best channel = %d \n", best_channel);
    cc2420_set_channel(best_channel);
}

