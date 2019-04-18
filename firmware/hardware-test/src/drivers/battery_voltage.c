#include "battery_voltage.h"
#include <adc.h>

#define LOG_LEVEL 4
#include <logging/log.h>
LOG_MODULE_REGISTER(adc_drv);

#include <hal/nrf_saadc.h>

#define BATTERY_VOLTAGE_PIN NRF_SAADC_INPUT_AIN6

static struct device *adc_dev;
static struct adc_sequence sequence = {};
static int16_t adc_buffer[1] = {0};


static float adc_val_to_voltage(int16_t adc_val) {
	const u8_t adc_resolution = 10;
    const float adc_gain = 1/6.;
		const float adc_ref_voltage = 0.6f;
		if (adc_val < 0) {
			adc_val = 0;
		}
    return adc_val / ((adc_gain / (adc_ref_voltage)) * (1 << adc_resolution));
}

void init_battery_voltage()
{
  adc_dev = device_get_binding(DT_ADC_0_NAME);
	if (!adc_dev) {
		LOG_ERR("adc init failed :-(");
		return;
	}
	
	struct adc_channel_cfg adc_channel_cfg = {
		.gain             = ADC_GAIN_1_6,
		.reference        = ADC_REF_INTERNAL,
		.acquisition_time = ADC_ACQ_TIME(ADC_ACQ_TIME_MICROSECONDS, 10),
		.channel_id       = 0,
		.input_positive   = BATTERY_VOLTAGE_PIN,
	};

	u32_t ret = adc_channel_setup(adc_dev, &adc_channel_cfg);
	if (ret) {
		LOG_ERR("adc channel setup failed :-(");
		return;
	}

	
		sequence.channels    = BIT(0);
		sequence.buffer      = adc_buffer;
		sequence.buffer_size = sizeof(adc_buffer);
		sequence.resolution  = 10;
}

void sample_battery_voltage()
{
	int ret = adc_read(adc_dev, &sequence);
	if (ret) {
		LOG_ERR("adc read failed: %d", ret);
	}
}

float read_battery_voltage()
{
	return adc_val_to_voltage(adc_buffer[0]);
}