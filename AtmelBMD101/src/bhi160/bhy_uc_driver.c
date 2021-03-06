/*
 * bhy_uc_driver.c
 *
 * Created: 8/12/2015 13:33:40
 *  Author: Marc-Andre Harvey
 */ 

 /*  Disclaimer
  *
  * Common: Bosch Sensortec products are developed for the consumer goods
  * industry. They may only be used within the parameters of the respective valid
  * product data sheet.  Bosch Sensortec products are provided with the express
  * understanding that there is no warranty of fitness for a particular purpose.
  * They are not fit for use in life-sustaining, safety or security sensitive
  * systems or any system or device that may lead to bodily harm or property
  * damage if the system or device malfunctions. In addition, Bosch Sensortec
  * products are not fit for use in products which interact with motor vehicle
  * systems.  The resale and/or use of products are at the purchaser's own risk
  * and his own responsibility. The examination of fitness for the intended use
  * is the sole responsibility of the Purchaser.
  *
  * The purchaser shall indemnify Bosch Sensortec from all third party claims,
  * including any claims for incidental, or consequential damages, arising from
  * any product use not covered by the parameters of the respective valid product
  * data sheet or not approved by Bosch Sensortec and reimburse Bosch Sensortec
  * for all costs in connection with such claims.
  *
  * The purchaser must monitor the market for the purchased products,
  * particularly with regard to product safety and inform Bosch Sensortec without
  * delay of all security relevant incidents.
  *
  * Engineering Samples are marked with an asterisk (*) or (e). Samples may vary
  * from the valid technical specifications of the product series. They are
  * therefore not intended or fit for resale to third parties or for use in end
  * products. Their sole purpose is internal client testing. The testing of an
  * engineering sample may in no way replace the testing of a product series.
  * Bosch Sensortec assumes no liability for the use of engineering samples. By
  * accepting the engineering samples, the Purchaser agrees to indemnify Bosch
  * Sensortec from all claims arising from the use of engineering samples.
  *
  * Special: This software module (hereinafter called "Software") and any
  * information on application-sheets (hereinafter called "Information") is
  * provided free of charge for the sole purpose to support your application
  * work. The Software and Information is subject to the following terms and
  * conditions:
  *
  * The Software is specifically designed for the exclusive use for Bosch
  * Sensortec products by personnel who have special experience and training. Do
  * not use this Software if you do not have the proper experience or training.
  *
  * This Software package is provided `` as is `` and without any expressed or
  * implied warranties, including without limitation, the implied warranties of
  * merchantability and fitness for a particular purpose.
  *
  * Bosch Sensortec and their representatives and agents deny any liability for
  * the functional impairment of this Software in terms of fitness, performance
  * and safety. Bosch Sensortec and their representatives and agents shall not be
  * liable for any direct or indirect damages or injury, except as otherwise
  * stipulated in mandatory applicable law.
  *
  * The Information provided is believed to be accurate and reliable. Bosch
  * Sensortec assumes no responsibility for the consequences of use of such
  * Information nor for any infringement of patents or other rights of third
  * parties which may result from its use. No license is granted by implication
  * or otherwise under any patent or patent rights of Bosch. Specifications
  * mentioned in the Information are subject to change without notice.
  *
  */

#include "bhy_uc_driver.h"


/* these FIFO sizes are dependent on the type enumeration  */
/* do not change the order								   */
#define	BHY_DATA_SIZE_PADDING			1
#define	BHY_DATA_SIZE_QUATERNION		11
#define	BHY_DATA_SIZE_VECTOR			8
#define	BHY_DATA_SIZE_SCALAR_U8			2
#define	BHY_DATA_SIZE_SCALAR_U16		3
#define	BHY_DATA_SIZE_SCALAR_S16		3
#define	BHY_DATA_SIZE_SCALAR_U24		4
#define	BHY_DATA_SIZE_SENSOR_EVENT		1
#define	BHY_DATA_SIZE_UNCALIB_VECTOR	14
#define	BHY_DATA_SIZE_META_EVENT		4
#define BHY_DATA_SIZE_BSX				17  
#if BHY_DEBUG
	#define BHY_DATA_SIZE_DEBUG				14
#endif
/* these FIFO sizes are dependent on the enumeration above */
/* do not change the order								   */
const u8 _fifoSizes[] = {
	BHY_DATA_SIZE_PADDING, BHY_DATA_SIZE_QUATERNION,
	BHY_DATA_SIZE_VECTOR, BHY_DATA_SIZE_SCALAR_U8,
	BHY_DATA_SIZE_SCALAR_U16, BHY_DATA_SIZE_SCALAR_S16,
	BHY_DATA_SIZE_SCALAR_U24, BHY_DATA_SIZE_SENSOR_EVENT,
	BHY_DATA_SIZE_UNCALIB_VECTOR, BHY_DATA_SIZE_META_EVENT,
	BHY_DATA_SIZE_BSX	
#if BHY_DEBUG
	,	BHY_DATA_SIZE_DEBUG
#endif
};

#if BHY_CALLBACK_MODE
/* The callback feature is a type of software interrupt.   */
/* The driver keeps in RAM an array of function pointers   */
/* for every sensor ID, wakeup and non-wakeup, every meta  */
/* event, and both wakeup and non-wakeup timestamps. When  */
/* parsing the fifo within the bhy_parse_next_fifo_packet  */
/* function, it will jump into the callback if the pointer */
/* is non-null.											   */
/* to sort through the array, it use either the sensor_id  */
/* or the event id.										   */

static void (*g_sensor_callback_list[63])(bhy_data_generic_t *, bhy_virtual_sensor_t) = {0};
static void (*g_timestamp_callback_list[2])(bhy_data_scalar_u16_t *) = {0};
static void (*g_meta_event_callback_list[17])(bhy_data_meta_event_t *, bhy_meta_event_type_t) = {0};

#endif




/* initializes the driver, the API and loads the ram patch into the sensor */
BHY_RETURN_FUNCTION_TYPE bhy_driver_init(
	const u8 *bhy_fw_data, const u32 bhy_fw_len)
{
	BHY_RETURN_FUNCTION_TYPE result= BHY_SUCCESS;
	
	bhy_initialize_support();
	
	/* downloads the ram patch to the BHy */
	result += bhy_initialize_from_rom(bhy_fw_data, bhy_fw_len);

	return result;
}

/* this functions enables the selected virtual sensor */
BHY_RETURN_FUNCTION_TYPE bhy_enable_virtual_sensor
	(	bhy_virtual_sensor_t sensor_id, u8 wakeup_status, u16 sample_rate, 
		u16 max_report_latency_ms, u8 flush_sensor, u16 change_sensitivity,
		u16 dynamic_range ) {
		
	BHY_RETURN_FUNCTION_TYPE result= BHY_SUCCESS;
	union {
		struct sensor_configuration_wakeup_t sensor_configuration_wakeup;
		struct sensor_configuration_non_wakeup_t sensor_configuration_non_wakeup;
	} sensor_configuration;
	
	/* checks if sensor id is in range */
	if (sensor_id >= 32)
		return BHY_OUT_OF_RANGE;
	
	/*computes the sensor id */
	sensor_id += wakeup_status;
	
	/* flush the fifo if requested */
	switch (flush_sensor) {
		case VS_FLUSH_SINGLE:
			result += bhy_set_fifo_flush(sensor_id);
			break;
		case VS_FLUSH_ALL:
			result += bhy_set_fifo_flush(VS_FLUSH_ALL);
			break;
		case VS_FLUSH_NONE:
			break;
		default:
			return BHY_OUT_OF_RANGE;
	}
	
	/* computes the param page as sensor_id + 0xC0 (sensor parameter write)*/
	sensor_id+= 0xC0;
	
	/*calls the right function */
	switch (wakeup_status) {
		case VS_NON_WAKEUP:
		sensor_configuration.sensor_configuration_non_wakeup.non_wakeup_sample_rate = sample_rate;
		sensor_configuration.sensor_configuration_non_wakeup.non_wakeup_max_report_latency = max_report_latency_ms;
		sensor_configuration.sensor_configuration_non_wakeup.non_wakeup_change_sensitivity = change_sensitivity;
		sensor_configuration.sensor_configuration_non_wakeup.non_wakeup_dynamic_range = dynamic_range;
		result += bhy_set_non_wakeup_sensor_configuration( &sensor_configuration.sensor_configuration_non_wakeup, sensor_id);
		return result;
		
		case VS_WAKEUP:
		sensor_configuration.sensor_configuration_wakeup.wakeup_sample_rate = sample_rate;
		sensor_configuration.sensor_configuration_wakeup.wakeup_max_report_latency = max_report_latency_ms;
		sensor_configuration.sensor_configuration_wakeup.wakeup_change_sensitivity = change_sensitivity;
		sensor_configuration.sensor_configuration_wakeup.wakeup_dynamic_range = dynamic_range;
		result += bhy_set_non_wakeup_sensor_configuration( &sensor_configuration.sensor_configuration_non_wakeup, sensor_id);
		return result;
		
		default:
		return BHY_OUT_OF_RANGE;
	}
}

/* this functions disables the selected virtual sensor */
BHY_RETURN_FUNCTION_TYPE bhy_disable_virtual_sensor
	(bhy_virtual_sensor_t sensor_id, u8 wakeup_status) {
	union {
		struct sensor_configuration_wakeup_t sensor_configuration_wakeup;
		struct sensor_configuration_non_wakeup_t sensor_configuration_non_wakeup;
	} sensor_configuration;

	/* checks if sensor id is in range */
	if (sensor_id >= 32)
		return BHY_OUT_OF_RANGE;
	
	/* computes the param page as */
	/* wakeup_status + sensor_id + 0xC0 (sensor parameter write) */
	sensor_id += wakeup_status + 0xC0;
	
	/*calls the right function */
	switch (wakeup_status) {
		case VS_NON_WAKEUP:
		sensor_configuration.sensor_configuration_non_wakeup.non_wakeup_sample_rate = 0;
		sensor_configuration.sensor_configuration_non_wakeup.non_wakeup_max_report_latency = 0;
		sensor_configuration.sensor_configuration_non_wakeup.non_wakeup_change_sensitivity = 0;
		sensor_configuration.sensor_configuration_non_wakeup.non_wakeup_dynamic_range = 0;
		return bhy_set_non_wakeup_sensor_configuration( &sensor_configuration.sensor_configuration_non_wakeup, sensor_id);
		
		case VS_WAKEUP:
		sensor_configuration.sensor_configuration_wakeup.wakeup_sample_rate = 0;
		sensor_configuration.sensor_configuration_wakeup.wakeup_max_report_latency = 0;
		sensor_configuration.sensor_configuration_wakeup.wakeup_change_sensitivity = 0;
		sensor_configuration.sensor_configuration_wakeup.wakeup_dynamic_range = 0;
		return bhy_set_non_wakeup_sensor_configuration( &sensor_configuration.sensor_configuration_non_wakeup, sensor_id);
		
		default:
		return BHY_OUT_OF_RANGE;
	}
}

/* retrieves the fifo data. it needs a buffer of at least 51 bytes to work */
/* it outputs the data into the variable buffer. the number of bytes read */
/* into bytes_read  and the bytes remaining in the fifo into bytes_left */
BHY_RETURN_FUNCTION_TYPE bhy_read_fifo(u8 * buffer, const u16 buffer_size, u16 * bytes_read, u16 * bytes_left) {
	BHY_RETURN_FUNCTION_TYPE result = BHY_SUCCESS;
	static u16 current_index = 0;
	static u16 current_transaction_size = 0;
	
	if (buffer_size <= 50)
		return BHY_OUT_OF_RANGE;
	
	/* gets the number of bytes left in the fifo either from memory of from */
	/* the register															*/
	if (current_transaction_size == 0)
		result = bhy_read_bytes_remaining(&current_transaction_size);

	/* if there are bytes in the fifo to read */
	if (current_transaction_size) {
		
		/* calculates the number of bytes to read. either the number of 	*/
		/* bytes left, or the buffer size, or just enough so the last page 	*/
		/* does not get turned												*/
		if (buffer_size >= current_transaction_size-current_index)
			*bytes_read = current_transaction_size-current_index;
		
		else if (current_transaction_size - (current_index+buffer_size) <= 50)
			*bytes_read = (current_transaction_size-51) - current_index;
		
		else
			*bytes_read = buffer_size;

		result += bhy_read_reg(current_index%50, buffer, *bytes_read );
		
		current_index += *bytes_read;
		
		*bytes_left = current_transaction_size-current_index;
		
		if (*bytes_left == 0) {
			current_index = 0;
			current_transaction_size = 0;
		}

	} else {
		*bytes_read=0;
		*bytes_left=0;
		return result;
	}

	return result;
}

BHY_RETURN_FUNCTION_TYPE bhy_parse_next_fifo_packet 
	(u8 **fifo_buffer, u16 *fifo_buffer_length, 
	bhy_data_generic_t * fifo_data_output, bhy_data_type_t * fifo_data_type) {
	
	if (*fifo_buffer_length == 0)
		/* there are no more bytes in the fifo buffer to read */
		return BHY_SUCCESS;
	
	/* the first fifo byte should be a known virtual sensor ID */
	switch (**fifo_buffer) {
	
		case VS_ID_PADDING:
			*fifo_data_type = BHY_DATA_TYPE_PADDING;
			
			fifo_data_output->data_padding.sensor_id = **fifo_buffer;
			
			break;
			
		case VS_ID_ROTATION_VECTOR:
		case VS_ID_ROTATION_VECTOR_WAKEUP:
		case VS_ID_GAME_ROTATION_VECTOR:
		case VS_ID_GAME_ROTATION_VECTOR_WAKEUP:
		case VS_ID_GEOMAGNETIC_ROTATION_VECTOR:
		case VS_ID_GEOMAGNETIC_ROTATION_VECTOR_WAKEUP:
			if (*fifo_buffer_length < _fifoSizes[BHY_DATA_TYPE_QUATERNION])
				return BHY_OUT_OF_RANGE;
				
			*fifo_data_type = BHY_DATA_TYPE_QUATERNION;
			
			fifo_data_output->data_quaternion.sensor_id = **fifo_buffer;
			fifo_data_output->data_quaternion.x = 
			(s16)( ((u16)*(*fifo_buffer+1)) | ((u16)*(*fifo_buffer+2) << 8));
			fifo_data_output->data_quaternion.y = 
			(s16)( ((u16)*(*fifo_buffer+3)) | ((u16)*(*fifo_buffer+4) << 8));
			fifo_data_output->data_quaternion.z = 
			(s16)( ((u16)*(*fifo_buffer+5)) | ((u16)*(*fifo_buffer+6) << 8));
			fifo_data_output->data_quaternion.w = 
			(s16)( ((u16)*(*fifo_buffer+7)) | ((u16)*(*fifo_buffer+8) << 8));
			fifo_data_output->data_quaternion.estimated_accuracy = 
			(s16)( ((u16)*(*fifo_buffer+9)) | ((u16)*(*fifo_buffer+10) << 8));
			
			break;
			
		case VS_ID_ACCELEROMETER:
		case VS_ID_ACCELEROMETER_WAKEUP:
		case VS_ID_MAGNETOMETER:
		case VS_ID_MAGNETOMETER_WAKEUP:
		case VS_ID_ORIENTATION:
		case VS_ID_ORIENTATION_WAKEUP:
		case VS_ID_GYROSCOPE:
		case VS_ID_GYROSCOPE_WAKEUP:
		case VS_ID_GRAVITY:
		case VS_ID_GRAVITY_WAKEUP:
		case VS_ID_LINEAR_ACCELERATION:
		case VS_ID_LINEAR_ACCELERATION_WAKEUP:
			if (*fifo_buffer_length < _fifoSizes[BHY_DATA_TYPE_VECTOR])
				return BHY_OUT_OF_RANGE;
			
			*fifo_data_type = BHY_DATA_TYPE_VECTOR;
			
			fifo_data_output->data_vector.sensor_id = **fifo_buffer;
			fifo_data_output->data_vector.x =
			(s16)( ((u16)*(*fifo_buffer+1)) | ((u16)*(*fifo_buffer+2) << 8));
			fifo_data_output->data_vector.y =
			(s16)( ((u16)*(*fifo_buffer+3)) | ((u16)*(*fifo_buffer+4) << 8));
			fifo_data_output->data_vector.z =
			(s16)( ((u16)*(*fifo_buffer+5)) | ((u16)*(*fifo_buffer+6) << 8));
			fifo_data_output->data_vector.status = *(*fifo_buffer+7);
			
			break;
		
		case VS_ID_HEART_RATE:
		case VS_ID_HEART_RATE_WAKEUP:
			if (*fifo_buffer_length < _fifoSizes[BHY_DATA_TYPE_SCALAR_U8])
				return BHY_OUT_OF_RANGE;
				
			*fifo_data_type = BHY_DATA_TYPE_SCALAR_U8;
			fifo_data_output->data_scalar_u8.sensor_id = **fifo_buffer;
			fifo_data_output->data_scalar_u8.data = *(*fifo_buffer+1);

			break;
			
		case VS_ID_LIGHT:
		case VS_ID_LIGHT_WAKEUP:
		case VS_ID_PROXIMITY:
		case VS_ID_PROXIMITY_WAKEUP:
		case VS_ID_HUMIDITY:
		case VS_ID_HUMIDITY_WAKEUP:
		case VS_ID_STEP_COUNTER:
		case VS_ID_STEP_COUNTER_WAKEUP:
		case VS_ID_ACTIVITY:
		case VS_ID_ACTIVITY_WAKEUP:
		case VS_ID_TIMESTAMP_LSW:
		case VS_ID_TIMESTAMP_LSW_WAKEUP:
		case VS_ID_TIMESTAMP_MSW:
		case VS_ID_TIMESTAMP_MSW_WAKEUP:
			if (*fifo_buffer_length < _fifoSizes[BHY_DATA_TYPE_SCALAR_U16])
				return BHY_OUT_OF_RANGE;
			
			*fifo_data_type = BHY_DATA_TYPE_SCALAR_U16;
			fifo_data_output->data_scalar_u16.sensor_id = **fifo_buffer;
			fifo_data_output->data_scalar_u16.data = 
			(u16)( ((u16)*(*fifo_buffer+1)) | ((u16)*(*fifo_buffer+2) << 8));
			
			break;
			
		case VS_ID_TEMPERATURE:
		case VS_ID_TEMPERATURE_WAKEUP:
		case VS_ID_AMBIENT_TEMPERATURE:
		case VS_ID_AMBIENT_TEMPERATURE_WAKEUP:
			if (*fifo_buffer_length < _fifoSizes[BHY_DATA_TYPE_SCALAR_S16])
				return BHY_OUT_OF_RANGE;
			
			*fifo_data_type = BHY_DATA_TYPE_SCALAR_S16;
			fifo_data_output->data_scalar_s16.sensor_id = **fifo_buffer;
			fifo_data_output->data_scalar_s16.data =
			(s16)( ((u16)*(*fifo_buffer+1)) | ((u16)*(*fifo_buffer+2) << 8));
			
			break;
		
		case VS_ID_BAROMETER:
		case VS_ID_BAROMETER_WAKEUP:
			if (*fifo_buffer_length < _fifoSizes[BHY_DATA_TYPE_SCALAR_U24])
				return BHY_OUT_OF_RANGE;
			
			*fifo_data_type = BHY_DATA_TYPE_SCALAR_U24;
			fifo_data_output->data_scalar_u24.sensor_id = **fifo_buffer;
			fifo_data_output->data_scalar_u24.data =
			(u32)( ((u32)*(*fifo_buffer+1)) | ((u32)*(*fifo_buffer+2) << 8) |
					((u32)*(*fifo_buffer+3) << 16));
			
			break;
			
		case VS_ID_SIGNIFICANT_MOTION:
		case VS_ID_SIGNIFICANT_MOTION_WAKEUP:
		case VS_ID_STEP_DETECTOR:
		case VS_ID_STEP_DETECTOR_WAKEUP:
		case VS_ID_TILT_DETECTOR:
		case VS_ID_TILT_DETECTOR_WAKEUP:
		case VS_ID_WAKE_GESTURE:
		case VS_ID_WAKE_GESTURE_WAKEUP:
		case VS_ID_GLANCE_GESTURE:
		case VS_ID_GLANCE_GESTURE_WAKEUP:
		case VS_ID_PICKUP_GESTURE:
		case VS_ID_PICKUP_GESTURE_WAKEUP:
			*fifo_data_type = BHY_DATA_TYPE_SENSOR_EVENT;
			
			fifo_data_output->data_sensor_event.sensor_id = **fifo_buffer;
			
			break;
		case VS_ID_UNCALIBRATED_MAGNETOMETER:
		case VS_ID_UNCALIBRATED_MAGNETOMETER_WAKEUP:
		case VS_ID_UNCALIBRATED_GYROSCOPE:
		case VS_ID_UNCALIBRATED_GYROSCOPE_WAKEUP:
			if (*fifo_buffer_length < _fifoSizes[BHY_DATA_TYPE_UNCALIB_VECTOR])
				return BHY_OUT_OF_RANGE;
			
			*fifo_data_type = BHY_DATA_TYPE_UNCALIB_VECTOR;
			fifo_data_output->data_uncalib_vector.sensor_id = **fifo_buffer;
			fifo_data_output->data_uncalib_vector.x =
			(s16)( ((u16)*(*fifo_buffer+1)) | ((u16)*(*fifo_buffer+2) << 8));
			fifo_data_output->data_uncalib_vector.y =
			(s16)( ((u16)*(*fifo_buffer+3)) | ((u16)*(*fifo_buffer+4) << 8));
			fifo_data_output->data_uncalib_vector.z =
			(s16)( ((u16)*(*fifo_buffer+5)) | ((u16)*(*fifo_buffer+6) << 8));
			fifo_data_output->data_uncalib_vector.x_bias =
			(s16)( ((u16)*(*fifo_buffer+7)) | ((u16)*(*fifo_buffer+8) << 8));
			fifo_data_output->data_uncalib_vector.y_bias =
			(s16)( ((u16)*(*fifo_buffer+9)) | ((u16)*(*fifo_buffer+10) << 8));
			fifo_data_output->data_uncalib_vector.z_bias =
			(s16)( ((u16)*(*fifo_buffer+11)) | ((u16)*(*fifo_buffer+12) << 8));
			fifo_data_output->data_uncalib_vector.status = *(*fifo_buffer+13);
			break;
			
		case VS_ID_META_EVENT:
		case VS_ID_META_EVENT_WAKEUP:
			if (*fifo_buffer_length < _fifoSizes[BHY_DATA_TYPE_META_EVENT])
				return BHY_OUT_OF_RANGE;
			
			*fifo_data_type = BHY_DATA_TYPE_META_EVENT;
			fifo_data_output->data_meta_event.meta_event_id =	**fifo_buffer;
			fifo_data_output->data_meta_event.event_number =	(bhy_meta_event_type_t)(*(*fifo_buffer+1));
			fifo_data_output->data_meta_event.sensor_type =		*(*fifo_buffer+2);
			fifo_data_output->data_meta_event.event_specific =	*(*fifo_buffer+3);
			
			break;
#if BHY_DEBUG
		case VS_ID_DEBUG:
			if (*fifo_buffer_length < _fifoSizes[BHY_DATA_TYPE_DEBUG])
				return BHY_OUT_OF_RANGE;
					
			*fifo_data_type = BHY_DATA_TYPE_DEBUG;
			fifo_data_output->data_debug.sensor_id	=	**fifo_buffer;
			fifo_data_output->data_debug.data[0]	=	*(*fifo_buffer+1);
			fifo_data_output->data_debug.data[1]	=	*(*fifo_buffer+2);
			fifo_data_output->data_debug.data[2]	=	*(*fifo_buffer+3);
			fifo_data_output->data_debug.data[3]	=	*(*fifo_buffer+4);
			fifo_data_output->data_debug.data[4]	=	*(*fifo_buffer+5);
			fifo_data_output->data_debug.data[5]	=	*(*fifo_buffer+6);
			fifo_data_output->data_debug.data[6]	=	*(*fifo_buffer+7);
			fifo_data_output->data_debug.data[7]	=	*(*fifo_buffer+8);
			fifo_data_output->data_debug.data[8]	=	*(*fifo_buffer+9);
			fifo_data_output->data_debug.data[9]	=	*(*fifo_buffer+10);
			fifo_data_output->data_debug.data[10]	=	*(*fifo_buffer+11);
			fifo_data_output->data_debug.data[11]	=	*(*fifo_buffer+12);
			fifo_data_output->data_debug.data[12]	=	*(*fifo_buffer+13);
			
			break;
#endif
		
		case VS_ID_BSX_C:
		case VS_ID_BSX_B:
		case VS_ID_BSX_A:
			if (*fifo_buffer_length < _fifoSizes[BHY_DATA_TYPE_BSX])
				return BHY_OUT_OF_RANGE;
			
			*fifo_data_type = BHY_DATA_TYPE_BSX;
			fifo_data_output->data_bsx.sensor_id =	**fifo_buffer;

			fifo_data_output->data_bsx.x =
			(u32)( ((u32)*(*fifo_buffer+1)) | ((u32)*(*fifo_buffer+2) << 8) |
					((u32)*(*fifo_buffer+3) << 16) | ((u32)*(*fifo_buffer+4) << 24));
			
			fifo_data_output->data_bsx.y =
			(u32)( ((u32)*(*fifo_buffer+5)) | ((u32)*(*fifo_buffer+6) << 8) |
					((u32)*(*fifo_buffer+7) << 16) | ((u32)*(*fifo_buffer+8) << 24));
			
			fifo_data_output->data_bsx.z =
			(u32)( ((u32)*(*fifo_buffer+9)) | ((u32)*(*fifo_buffer+10) << 8) |
					((u32)*(*fifo_buffer+11) << 16) | ((u32)*(*fifo_buffer+12) << 24));		
			
			fifo_data_output->data_bsx.timestamp =
			(u32)( ((u32)*(*fifo_buffer+13)) | ((u32)*(*fifo_buffer+14) << 8) |
					((u32)*(*fifo_buffer+15) << 16) | ((u32)*(*fifo_buffer+16) << 24));
			
			break;
	
	
		/* the VS sensor ID is unknown. Either the sync has been lost or the */
		/* ram patch implements a new sensor ID that this driver doesn't yet */
		/* support															 */
		default:
		return BHY_OUT_OF_RANGE;
	}
	

	
	
#if BHY_CALLBACK_MODE
	if( (**fifo_buffer) < 0x40 ) {
		if (g_sensor_callback_list[(**fifo_buffer)] != 0) {
			g_sensor_callback_list[(**fifo_buffer)](fifo_data_output, (**fifo_buffer));
		}
	} else if ( (**fifo_buffer) == VS_ID_TIMESTAMP_LSW ||
				(**fifo_buffer) == VS_ID_TIMESTAMP_MSW ) {
		if ( g_timestamp_callback_list[0] != 0 ) {
			g_timestamp_callback_list[0](&fifo_data_output->data_scalar_u16);
		}
	} else if ( (**fifo_buffer) == VS_ID_TIMESTAMP_LSW_WAKEUP ||
				(**fifo_buffer) == VS_ID_TIMESTAMP_MSW_WAKEUP ) {
		if ( g_timestamp_callback_list[1] != 0 ) {
			g_timestamp_callback_list[1](&fifo_data_output->data_scalar_u16);
		}	
	} else if ( (**fifo_buffer) == VS_ID_META_EVENT ||
				(**fifo_buffer) == VS_ID_META_EVENT_WAKEUP ) {
		if ( g_meta_event_callback_list[fifo_data_output->data_meta_event.event_number] != 0) {
			g_meta_event_callback_list[fifo_data_output->data_meta_event.event_number](&fifo_data_output->data_meta_event, fifo_data_output->data_meta_event.event_number);
		}
	}

#endif	

	(*fifo_buffer) += _fifoSizes[*fifo_data_type];
	*fifo_buffer_length -= _fifoSizes[*fifo_data_type];
	
	return BHY_SUCCESS;
};


/* This function will detect the timestamp packet accordingly and update	*/
/* either the MSW or the LSW of the system timestamp						*/
BHY_RETURN_FUNCTION_TYPE bhy_update_system_timestamp(bhy_data_scalar_u16_t *timestamp_packet,
									u32 * system_timestamp) {
	switch (timestamp_packet->sensor_id) {
		case VS_ID_TIMESTAMP_LSW:
		case VS_ID_TIMESTAMP_LSW_WAKEUP:
			*system_timestamp = (*system_timestamp & 0xFFFF0000) | (u32)timestamp_packet->data;
			return BHY_SUCCESS;
		case VS_ID_TIMESTAMP_MSW:
		case VS_ID_TIMESTAMP_MSW_WAKEUP:
			*system_timestamp = (*system_timestamp & 0x0000FFFF) | ((u32)timestamp_packet->data << 16);
			return BHY_SUCCESS;
		default:
			return BHY_OUT_OF_RANGE;
	}
}

#if BHY_DEBUG
/* This function outputs the debug data to function pointer. You need to 	*/
/* provide a function that takes as argument a zero-terminated string and	*/
/* prints it 																*/
void bhy_print_debug_packet
(bhy_data_debug_t *packet, void (*debug_print_ptr)(const u8 *)) {
	u8 len;
	u8 tempstr[25];
	u8 i=0;
	
	len = packet->data[0] & 0x3F;
	if (packet->data[0] & 0x40) { /* Binary data */
		while (i < len) {
		tempstr[i*2] = (packet->data[i+1] & 0xF0) > 0x90 ?
						((packet->data[i+1] & 0xF0)>>4) - 0x0A + 'A' :
						((packet->data[i+1] & 0xF0)>>4) + '0' ;
		tempstr[i*2+1] = (packet->data[i+1] & 0x0F) > 0x09 ?
						(packet->data[i+1] & 0x0F) - 0x0A + 'A' :
						(packet->data[i+1] & 0x0F) + '0' ;
		++i;
		}
		tempstr[i*2] = '\0';
	} else {
		
		while (i < len) {
			tempstr[i] = packet->data[i+1];
			++i;
		}
		tempstr[i] = '\0';
		
	}
	debug_print_ptr(tempstr);
}

#endif

#if BHY_CALLBACK_MODE

BHY_RETURN_FUNCTION_TYPE bhy_install_sensor_callback ( bhy_virtual_sensor_t sensor_id, u8 wakeup_status, void (*sensor_callback)(bhy_data_generic_t *, bhy_virtual_sensor_t) ){
	if (sensor_id > 0x3F) {
		/* Invalid sensor ID */
		return BHY_OUT_OF_RANGE;
	}
	sensor_id = (sensor_id&0x1F) + wakeup_status;
	if (g_sensor_callback_list[sensor_id] != 0) {
		/* There is already a callback installed */
		return BHY_OUT_OF_RANGE;
	} else {
		g_sensor_callback_list[sensor_id] = sensor_callback;
		return BHY_SUCCESS;
	}
}

BHY_RETURN_FUNCTION_TYPE bhy_uninstall_sensor_callback ( bhy_virtual_sensor_t sensor_id, u8 wakeup_status ) {
	if (sensor_id > 0x3F) {
		/* Invalid sensor ID */
		return BHY_OUT_OF_RANGE;
	}
	sensor_id = (sensor_id&0x1F) + wakeup_status;
	if (g_sensor_callback_list[sensor_id] == 0) {
		/* There are no callback installed */
		return BHY_OUT_OF_RANGE;
	} else {
		g_sensor_callback_list[sensor_id] = 0;
		return BHY_SUCCESS;
	}
	/*the only way to get here is with invalid wakeup_status parameter*/
	return BHY_ERROR;
}

BHY_RETURN_FUNCTION_TYPE bhy_install_timestamp_callback ( u8 wakeup_status, void (*timestamp_callback)(bhy_data_scalar_u16_t *) ){
	if (g_timestamp_callback_list[wakeup_status == VS_WAKEUP] != 0) {
		/* There is already a callback installed */
		return BHY_OUT_OF_RANGE;
	} else {
		g_timestamp_callback_list[wakeup_status == VS_WAKEUP] = timestamp_callback;
		return BHY_SUCCESS;
	}
}

BHY_RETURN_FUNCTION_TYPE bhy_uninstall_timestamp_callback ( u8 wakeup_status ) {
	if (g_timestamp_callback_list[wakeup_status == VS_WAKEUP] == 0) {
		/* There are no callback installed */
		return BHY_OUT_OF_RANGE;
	} else {
		g_timestamp_callback_list[wakeup_status == VS_WAKEUP] = 0;
		return BHY_SUCCESS;
	}
}

BHY_RETURN_FUNCTION_TYPE bhy_install_meta_event_callback ( bhy_meta_event_type_t meta_event_id, void (*meta_event_callback)(bhy_data_meta_event_t *, bhy_meta_event_type_t) ) {
	if (meta_event_id > 17) {
		/* Invalid meta event ID */
		return BHY_OUT_OF_RANGE;
	} else if (g_meta_event_callback_list[meta_event_id] != 0) {
		/* There is already a callback installed */
		return BHY_OUT_OF_RANGE;
	} else {
		g_meta_event_callback_list[meta_event_id] = meta_event_callback;
		return BHY_SUCCESS;
	}
}

BHY_RETURN_FUNCTION_TYPE bhy_uninstall_meta_event_callback ( bhy_meta_event_type_t meta_event_id ) {
	if (meta_event_id > 17) {
		/* Invalid meta event ID */
		return BHY_OUT_OF_RANGE;
	} else if (g_meta_event_callback_list[meta_event_id] == 0) {
		/* There is no callback installed */
		return BHY_OUT_OF_RANGE;
	} else {
		g_meta_event_callback_list[meta_event_id] = 0;
		return BHY_SUCCESS;
	}
}

#endif






























