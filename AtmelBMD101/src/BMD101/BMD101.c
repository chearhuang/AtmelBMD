#include "asf.h"
#include "stdio_serial.h"
#include "conf_uart_serial.h"
#include "BMD101.h"
#define NEL_EXN 30
BMD101 vBMD101;

extern struct usart_module ext3_uart_module;
extern uint8_t *tmp_data;

signed int   Get_PoorSignal_HeartRate(void);
/*extern void nsk_ecg_update();
extern uint8_t nsk_ecg_is_new_beat();
extern uint8_t nsk_ecg_get_hr_robust();
extern int32_t nsk_ecg_get_rri_count();
extern int16_t nsk_ecg_compute_hrv();
extern int16_t nsk_ecg_compute_mood();
extern int16_t nsk_ecg_compute_respiratory_rate();
extern int16_t nsk_ecg_get_rri();
extern int16_t nsk_ecg_get_signal_quality();
extern int16_t nsk_ecg_get_overall_signal_quality();
extern int16_t  nsk_ecg_compute_hr();
uint16_t rri = 0;
uint8_t hrRobust =0;
uint8_t rr_Interval =0;
*/

UINT generatedCheckSum = 0;
uint8_t Touch_Sensor_Flag=0;
UINT ii,jj,kk,ll;
int RawData[20]={0};	
/*
uint8_t  Hr[20]={0};
uint8_t  Hr1[20]={0};
int16_t rhr,ssq,lsq,rhr1;
int32_t hrv,mood,resp_rate;
uint8_t  resp_rateBuffer[20]={0}, hrvBuffer[20]={0},moodBuffer[20]={0};
uint8_t  rriBuffer[20]={0};
uint8_t  ssqBuffer[20] = {0};
uint8_t  lsqBuffer[20] = {0};
uint8_t  versionBuffer[20]={0};
*/
UINT temp;

signed int    Get_PoorSignal_HeartRate(void)
{
	if(Packet_Header == usart_serial_getchar(&ext3_uart_module,tmp_data))  //0xAA
	{
		if(Packet_Header == usart_serial_getchar(&ext3_uart_module,tmp_data))//0xAA
		{
			vBMD101.Rpayload_Length = usart_serial_getchar(&ext3_uart_module,tmp_data);
			if(vBMD101.Rpayload_Length == Payload_Length )
			{
				generatedCheckSum=0;
				for( kk= 0; kk< vBMD101.Rpayload_Length; kk++)
				{
					vBMD101.PayloadData[kk] = usart_serial_getchar(&ext3_uart_module,tmp_data);
					generatedCheckSum += vBMD101.PayloadData[kk];
				}
				vBMD101.PacketCheckSum = usart_serial_getchar(&ext3_uart_module,tmp_data);
				generatedCheckSum &= 0xFF;
				generatedCheckSum = ~generatedCheckSum & 0xFF;
				if(vBMD101.PacketCheckSum == generatedCheckSum)
				{
					//printf("No touch the sensor and the value is short\n");
					vBMD101.PacketCheckSum=0;
					generatedCheckSum=0;
					vBMD101.HighData= vBMD101.PayloadData[2] & 0xFFFF;
					vBMD101.LowData= vBMD101.PayloadData[3] & 0xFFFF;
					temp = (vBMD101.HighData)<<8;
					vBMD101.RawData = (temp|vBMD101.LowData);
					if(vBMD101.RawData > 32768)
					{
						vBMD101.RawData -= 65536;
					}
					if(vBMD101.PoorSignal_Value == 0xC8)
					{
						/*nsk_ecg_update(vBMD101.RawData); // keep updating for every incoming raw ecg sample.
						
						if (nsk_ecg_is_new_beat()) { // usually we only query algorithm outputs when a new beat is detected.
							ssq = nsk_ecg_get_signal_quality(); // Query the short term Signal Quality.
							rhr1 = nsk_ecg_get_hr_robust(); // Query the Robust Heart Rate.
							rhr = nsk_ecg_compute_hr();
							sprintf((char*)Hr,"HR: %2d bmp ",rhr);
							sprintf((char*)Hr1,"RHR: %2d bmp ",rhr1);
							sprintf((char*)ssqBuffer,"SQ: %2d ",ssq);
							lsq = nsk_ecg_get_overall_signal_quality(); // Query the long term Signal Quality.
							rr_Interval = nsk_ecg_get_rri_count(); 	//Interval  count>30,not to zero????
							if (rr_Interval >= NEL_EXN) { // only query ext algorithms when enough RRI is ready.
								hrv = nsk_ecg_compute_hrv(); // only if HRV is needed.
								mood = nsk_ecg_compute_mood(); // only if Mood is needed.
								resp_rate = nsk_ecg_compute_respiratory_rate(); // only if Respiratory Rate is needed.
								rri = nsk_ecg_get_rri();
								sprintf((char*)hrvBuffer,"HRV: %2d",hrv);
								sprintf((char*)moodBuffer,"Mood: %2d",mood);
								sprintf((char*)resp_rateBuffer,"Resp: %2d ",resp_rate);
								sprintf((char*)rriBuffer,"RRI: %2d ms ",rri);
								sprintf((char*)lsqBuffer,"LSQ: %2d ms ",lsq);
							}
						}
						*/
						//printf("\nRawData is %d\n",vBMD101.RawData);
						return vBMD101.RawData;
					}
					else
					{
						return 0;
					}
				}
			}
			else if (vBMD101.Rpayload_Length == 0x12 )
			{
				generatedCheckSum=0;
				for( jj= 0; jj< vBMD101.Rpayload_Length; jj++)
				{
					vBMD101.PayloadData[jj] = usart_serial_getchar(&ext3_uart_module,tmp_data);
					generatedCheckSum += vBMD101.PayloadData[jj];
				}
				vBMD101.PacketCheckSum = usart_serial_getchar(&ext3_uart_module,tmp_data);
				generatedCheckSum &= 0xFF;
				generatedCheckSum = ~generatedCheckSum & 0xFF;
				if(vBMD101.PacketCheckSum == generatedCheckSum)
				{
					vBMD101.PacketCheckSum=0;
					generatedCheckSum=0;
					for(ii=0; ii<vBMD101.Rpayload_Length;ii++)
					{
						switch(ii)
						{
							case 0:
							vBMD101.PoorSignal_Tag = vBMD101.PayloadData[0];
							break;
							case 1:
							vBMD101.PoorSignal_Value = vBMD101.PayloadData[1];
							//printf("PoorSignal_Value is %d",vBMD101.PoorSignal_Value);
							break;
							case 2:
							vBMD101.HeartRate_Tag = vBMD101.PayloadData[2];
							break;
							case 3:
							vBMD101.HeartRate_Value = vBMD101.PayloadData[3];
							break;
							default:
							break;
						}
					}
				}
			}
		}
	}
	return 0;
}



