#include "em_device.h"
#include "em_chip.h"
#include "em_usart.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_i2c.h"
#include "key_input.h"
#include <stdio.h>
#ifdef LIBMATH_INCLUDE
#include <math.h>
#endif
#include <stdlib.h>
#include "user_usart.h"

#define WMA_FACTOR	3
//#undef WMA_FACTOR
#define WEIGHT	0.8
#undef WEIGHT
#define NORMALIZATION_FACTOR 32767

int16_t SENSOR [AXIS_NUM][SAMPLE_NUM];
#ifdef WMA_FACTOR
	#define WMA_SAMPLE_NUM	SAMPLE_NUM - WMA_FACTOR + 1
	int16_t WMA_SENSOR [AXIS_NUM][WMA_SAMPLE_NUM];
	int16_t WMA_temp [AXIS_NUM][WMA_SAMPLE_NUM];

#endif
#define CROSS_FACTOR	3
int8_t WEIGHTED_AXIS [AXIS_NUM];
#define NUM_OF_DB	8
#define ELE_OF_DB	2

#ifndef LIBMATH_INCLUDE
double mysqrt(unsigned int x);
#endif

enum sensors_	{
	ACC_X,
	ACC_Y,
	ACC_Z,

};
extern int biggestValue [2];
extern int corr_int [AXIS_NUM][AXIS_NUM];


///////////////////////////////////////////////////////////////////////
void deriveCorrelation ()	{
///////////////////////////////////////////////////////////////////////
// A factor: The array of sensor data
// Return: NULL
//////////////////////////////////////////////////////////////////////

	float cov [AXIS_NUM][AXIS_NUM];
	float corr [AXIS_NUM][AXIS_NUM];
	float average [] = {0,0,0};
	int nLoop, nLoop2, nLoop3;

	for (nLoop = 0; nLoop < AXIS_NUM; nLoop ++)	{
		for (nLoop2 = 0; nLoop2 < SAMPLE_NUM; nLoop2 ++)	{
			average [nLoop] += SENSOR [nLoop][nLoop2];
		} //for nLoop2
		average [nLoop] /= SAMPLE_NUM;
	} //for nLoop



	for (nLoop = 0; nLoop < AXIS_NUM; nLoop ++)	{
		for (nLoop2 = 0; nLoop2 < AXIS_NUM; nLoop2 ++)	{
			cov [nLoop][nLoop2] = corr [nLoop][nLoop2] = corr_int[nLoop][nLoop2] = 0;
			for (nLoop3 = 0; nLoop3 < SAMPLE_NUM; nLoop3 ++)	{
				cov [nLoop][nLoop2] += (SENSOR [nLoop][nLoop3] - average [nLoop]) * (SENSOR [nLoop2][nLoop3] - average [nLoop2]);
			} // for nLoop3
			cov [nLoop][nLoop2] /= SAMPLE_NUM;

		} // for nLoop2
	} //for nLoop


	for (nLoop = 0; nLoop < AXIS_NUM; nLoop ++)	{
		for (nLoop2 = nLoop+1; nLoop2 < AXIS_NUM; nLoop2 ++)	{
#ifdef LIBMATH_INCLUDE
			if (cov [nLoop][nLoop] * cov [nLoop2][nLoop2])
				corr [nLoop][nLoop2] = cov [nLoop][nLoop2] / sqrt(cov [nLoop][nLoop] * cov [nLoop2][nLoop2]);
#else
			if (cov [nLoop][nLoop] * cov [nLoop2][nLoop2])
					corr [nLoop][nLoop2] = cov [nLoop][nLoop2] / mysqrt(cov [nLoop][nLoop] * cov [nLoop2][nLoop2]);
#endif
			else
				corr [nLoop][nLoop2] = 0;

			if ((corr [nLoop][nLoop2] < 0.5) && (corr [nLoop][nLoop2] > -0.5))	{	//filtering
				corr [nLoop][nLoop2] = 0;
			} //if
			else	{
				corr_int [nLoop][nLoop2] = (int)(corr [nLoop][nLoop2] *100);
			}
		} //for nLoop2
	} //for nLoop

/*
	for (nLoop = 0; nLoop < AXIS_NUM; nLoop ++)	{
		for (nLoop2 = 0; nLoop2 < AXIS_NUM; nLoop2 ++)	{
			printf ("%d\t", corr_int [nLoop][nLoop2] );
		}
		printf ("\n");
	}
	printf ("\n");

*/
} //function calculatCorrelation

//////////////////////////////////////////////////////////////////////
int compareSelfCorrleation (int db_id)	{
///////////////////////////////////////////////////////////////////////
// A factor: A DB id among 2, 3, 4, 5, 6, 7, 8, 9, 14, 15
// Return: detected DB id
//////////////////////////////////////////////////////////////////////

	int avg_gap [2] = {0,0};
	int result_ = 0;
	int nLoop, nLoop2, nLoop3;
	int filter_count = 0;
	float MIN = 10000.0;


	int total_db_count [2] = {22, 27};

	int db_corr [2][AXIS_NUM][AXIS_NUM] = {
			{		//for sensor1, CLOCK
					{0, 61, 00 },
					{00, 00, 04 },
					{00, 00, 00 },

				},

				{		//for sensor1, ANTI-CLOCK
					{00, 81, 59,  },
					{00, 00, -31,  },
					{00, 00, 00, },

				}
		};
	int db_count [2][AXIS_NUM][AXIS_NUM]= {
			{		//for sensor1, CLOCK
				{0, 3, 0, },
				{0, 0, 4, },
				{0, 0, 0, } ,

			},

			{		//for sensor1, ANTI-CLOCK
				{0, 18, 5 },
				{0, 0, 12 },
				{0, 0, 0 },

			},

	};

	for (nLoop = 0; nLoop < 2; nLoop ++)	{

		for (nLoop2 = 0; nLoop2 < AXIS_NUM; nLoop2 ++)	{
			for (nLoop3 = nLoop2 + 1; nLoop3 < AXIS_NUM; nLoop3 ++)	{

				if ((db_count [nLoop][nLoop2][nLoop3] * 5) >  total_db_count [nLoop])	{
					avg_gap [nLoop] += abs (corr_int [nLoop2][nLoop3] - db_corr [nLoop][nLoop2][nLoop3]);
					filter_count ++;
				}


			} //for nLoop3
		} //for nLoop2

		if (filter_count == 0)
			filter_count ++;


		avg_gap [nLoop] /= filter_count;

		filter_count = 0;

		if ((avg_gap [nLoop] != 0) && (MIN > avg_gap [nLoop]))	{
			MIN = avg_gap [nLoop];
		}

	} //for nLoop


	if (avg_gap [0] < avg_gap [1])	{
		result_ = (db_id % 2 == 1) ? (db_id - 1) : (db_id);		//CLOCK
	}
	else	{
		result_ = (db_id % 2 == 1) ? (db_id) : (db_id + 1);		//ANTI-CLOCK
	}

	printf ("<%d vs. %d> corr result is %d \n",  avg_gap[0],avg_gap[1], result_);

	return result_;
}


#ifdef WMA_FACTOR
void weightedMovingAverage (int16_t sensor [AXIS_NUM][SAMPLE_NUM])	{
	int nLoop, nLoop2, nLoop3;
	int16_t temp_avr;

	memset (WMA_SENSOR, '\0', sizeof(WMA_SENSOR));

	//printf ("<< WMA\n");
	for (nLoop = 0; nLoop < AXIS_NUM; nLoop ++)	{
		for (nLoop2 = 0; nLoop2 < (SAMPLE_NUM - WMA_FACTOR + 1); nLoop2 ++)	{
			temp_avr = 0;
#ifndef WEIGHT
			for (nLoop3 = 0; nLoop3 < WMA_FACTOR; nLoop3 ++)	{
				temp_avr +=  SENSOR [nLoop][nLoop2 + nLoop3];
			}
			WMA_SENSOR [nLoop][nLoop2] = temp_avr / WMA_FACTOR;

#else
			for (nLoop3 = 0; nLoop3 < WMA_FACTOR-1; nLoop3 ++)	{
				temp_avr +=  SENSOR [nLoop][nLoop2 + nLoop3];
			}
			temp_avr *= (1-WEIGHT);
			temp_avr += (WEIGHT * SENSOR [nLoop][nLoop2 + nLoop3]);
			WMA_SENSOR [nLoop][nLoop2] = temp_avr;
#endif
		}
	}
}
#endif



void compareOtherCorrelation (int arrayLen, int flag_, float db_sample[ELE_OF_DB][AXIS_NUM][WMA_SAMPLE_NUM], float db_average[ELE_OF_DB][AXIS_NUM], float db_cov[ELE_OF_DB][AXIS_NUM])	{

	int16_t WMA_SENSOR_local [AXIS_NUM][WMA_SAMPLE_NUM];
	int nLoopLen = 0, nLoopStart = 0;//, db_start = 0, db_end = 0;
	float average [AXIS_NUM];
	int nLoop, nLoop2, nLoop3;
	float cov [ELE_OF_DB][AXIS_NUM];
	float cov_self [AXIS_NUM];
	float corr [ELE_OF_DB][AXIS_NUM];
	float corr_int_local [ELE_OF_DB][AXIS_NUM];

	memset (cov, '\0', sizeof(cov));
	memset (average, '\0', sizeof(average));
	memset (cov_self, '\0', sizeof(cov_self));
	memset (corr, '\0', sizeof(corr));
	memset (corr_int_local, '\0', sizeof (corr_int_local));


	memset (WMA_SENSOR_local, '\0', sizeof(WMA_SENSOR_local));
	if (flag_ < 1)	{
		nLoopLen = arrayLen;
		nLoopStart = 0;
		memcpy (WMA_SENSOR_local, WMA_temp, sizeof(WMA_SENSOR_local));
	}
	else	{
		nLoopLen = WMA_SAMPLE_NUM;
		nLoopStart = flag_;
		memcpy (WMA_SENSOR_local, WMA_SENSOR, sizeof(WMA_SENSOR_local));
	}

	for (nLoop = 0; nLoop < AXIS_NUM; nLoop ++)	{	//average of incoming sensor value
		for (nLoop2 = 0; nLoop2 < nLoopLen ; nLoop2 ++)	{
			average [nLoop] += WMA_SENSOR_local [nLoop][nLoop2];
		} //for nLoop2

		average [nLoop] /= nLoopLen;

	} //for nLoop

	for (nLoop = 0; nLoop < AXIS_NUM; nLoop ++)	{
		float temp_float=0;
		for (nLoop2 = nLoopStart; nLoop2 < nLoopLen; nLoop2 ++)	{ //co-variance //분자계산
			temp_float = WMA_SENSOR_local [nLoop][nLoop2 - nLoopStart] - average [nLoop];

			for (nLoop3 = 0; nLoop3 < ELE_OF_DB; nLoop3 ++)
				cov [nLoop3][nLoop] += (temp_float * (db_sample [nLoop3][nLoop][nLoop2] - db_average [nLoop3][nLoop]));


			cov_self [nLoop] += (temp_float * temp_float);
		} //for nLoop2

		for (nLoop3 = 0; nLoop3 < ELE_OF_DB; nLoop3 ++)
			cov [nLoop3][nLoop] /= (nLoopLen - nLoopStart);

		cov_self [nLoop] /= (nLoopLen - nLoopStart);

	} // for nLoop

	int tempSUM [ELE_OF_DB];
	memset (tempSUM, 0, sizeof(tempSUM));

		for (nLoop = 0; nLoop <= 1; nLoop ++)	{	//correlation. //분모 계산

		for (nLoop2 = 0; nLoop2 < AXIS_NUM; nLoop2 ++)	{
#ifdef LIBMATH_INCLUDE
			if (cov_self [nLoop2] * db_cov [nLoop][nLoop2])
				corr [nLoop][nLoop2] = cov [nLoop][nLoop2] / sqrt (cov_self [nLoop2] * db_cov [nLoop][nLoop2]);
#else
			if (cov_self [nLoop2] * cov [nLoop][nLoop2])	{
				corr [nLoop][nLoop2] = 0;
				corr [nLoop][nLoop2] = cov [nLoop][nLoop2] / (mysqrt ((cov_self [nLoop2] * db_cov [nLoop][nLoop2])* 10000) / 100);
			}
#endif
			else
				corr [nLoop][nLoop2] = 0;

			corr_int_local [nLoop][nLoop2] = 0;
			corr_int_local [nLoop][nLoop2] = (int)(corr [nLoop][nLoop2] *100);

			tempSUM [nLoop] += ((int)(corr_int_local [nLoop][nLoop2] * WEIGHTED_AXIS [nLoop2]) / 100);

		} //for nLoop2

		if (tempSUM[nLoop] < 20)
			 tempSUM[nLoop] = 0;
		else
			tempSUM[nLoop] *= tempSUM[nLoop];


	} //for nLoop


	//------------------------------------------------ deviation --------------
		int gap_mean [NUM_OF_DB][AXIS_NUM];
		int gap_deviation [NUM_OF_DB][AXIS_NUM];
		memset (gap_mean, 0x0, sizeof(gap_mean));
		memset (gap_deviation, 0x0, sizeof(gap_deviation));


		int tempSUM_dev [NUM_OF_DB];
		memset (tempSUM_dev, 0, sizeof(tempSUM_dev));


		for (nLoop = 0; nLoop <= 1; nLoop ++)	{
			for (nLoop2 = 0; nLoop2 < AXIS_NUM; nLoop2 ++){
				for (nLoop3 = nLoopStart; nLoop3 < nLoopLen; nLoop3 ++){
					gap_mean [nLoop][nLoop2] += abs( (int)(db_sample [nLoop][nLoop2][nLoop3] * 1000) - (int) WMA_SENSOR_local[nLoop2][nLoop3]);
				}
				gap_mean [nLoop][nLoop2] /= nLoopLen;

				for (nLoop3 = nLoopStart; nLoop3 < nLoopLen; nLoop3 ++){
					gap_deviation [nLoop][nLoop2] += abs( (int)(db_sample [nLoop][nLoop2][nLoop3] * 1000) - (int)WMA_SENSOR_local[nLoop2][nLoop3] - (int)gap_mean [nLoop][nLoop2] );
				}

				gap_deviation [nLoop][nLoop2] /= nLoopLen;

				tempSUM_dev [nLoop] +=  gap_deviation [nLoop][nLoop2];
			}

		}

		int temp_int;
		for (nLoop = 0; nLoop <= 1; nLoop ++)	{
			if (tempSUM [nLoop] > 0)	{
				temp_int = (int)((tempSUM [nLoop] * 1000) / tempSUM_dev [nLoop]);
				//temp_int = tempSUM[nLoop];

			}
			else	{
				temp_int = 0;
			}
			if (biggestValue [nLoop] < temp_int)	{
				biggestValue [nLoop] = temp_int;
			}
		}



}


void normalizingFunction (int16_t sensor [AXIS_NUM][SAMPLE_NUM]){
	int nLoop, nLoop2;

	for (nLoop = 0; nLoop < AXIS_NUM; nLoop ++)	{
		for (nLoop2 = 0; nLoop2 < SAMPLE_NUM; nLoop2 ++)	{
			SENSOR [nLoop][nLoop2] = (int)(((float)sensor [nLoop][nLoop2] / NORMALIZATION_FACTOR) * 1000);
		}
	}

}




int crossCorrelation (float db_sample[ELE_OF_DB][AXIS_NUM][WMA_SAMPLE_NUM], float db_average[ELE_OF_DB][AXIS_NUM], float db_cov[ELE_OF_DB][AXIS_NUM])	{

	int nLoop, nLoop2, result_ = 0;
	memset (WMA_temp, '\0', sizeof(WMA_temp));

	biggestValue [0] = biggestValue [1] = 0;

	for (nLoop = -CROSS_FACTOR; nLoop < 1; nLoop ++)	{
		for (nLoop2 = 0; nLoop2 < WMA_SAMPLE_NUM + nLoop ; nLoop2 ++)	{
			WMA_temp [0][nLoop2] = WMA_SENSOR [0][-nLoop+ nLoop2];
			WMA_temp [1][nLoop2] = WMA_SENSOR [1][-nLoop+ nLoop2];
			WMA_temp [2][nLoop2] = WMA_SENSOR [2][-nLoop+ nLoop2];

		}
		compareOtherCorrelation (WMA_SAMPLE_NUM + nLoop, nLoop, db_sample, db_average, db_cov);
		memset (WMA_temp, '\0', sizeof(WMA_temp));


	}
	for (nLoop = 1; nLoop < CROSS_FACTOR; nLoop ++)	{

		compareOtherCorrelation (WMA_SAMPLE_NUM, nLoop, db_sample, db_average, db_cov);
	}

#if 0
	printf ("%d vs. %d  ",  biggestValue [0], biggestValue [1]);
#else
	char temp_p[18] = {0, };
	sprintf (temp_p, "%d vs. %d  /  ", biggestValue [0], biggestValue [1]);

	USART1_SEND (temp_p, 17);
#endif

	result_ = biggestValue [0] > biggestValue [1] ? 0 : 1;
	return result_;

}



int assignDB_FRONT (void)	{
	int result_;
	printf ("\nassign DB Front\n");

	float db_sample [ELE_OF_DB][AXIS_NUM][WMA_SAMPLE_NUM] = {
		{ //front
				0.0060 , 0.0050 , 0.0350 , 0.0400 , 0.0320 , 0.0300 , 0.0160 , 0.0330 , 0.0190 , 0.0210 , -0.0030 , -0.0490 , -0.0550 , -0.0280 , -0.0070 , 0.0420 , 0.0570 , 0.0910 ,
				0.0180 , 0.0420 , 0.1080 , 0.2080 , 0.2380 , 0.1670 , 0.0130 , -0.0950 , -0.1610 , -0.1650 , -0.1520 , -0.1180 , -0.1050 , -0.0720 , -0.0080 , 0.0810 , 0.1730 , 0.2240 ,
				0.5020 , 0.5010 , 0.5380 , 0.5710 , 0.5520 , 0.5250 , 0.4900 , 0.5160 , 0.5260 , 0.5250 , 0.4710 , 0.4450 , 0.4850 , 0.5430 , 0.5590 , 0.5530 , 0.5410 , 0.5350
		},
		{// back
				-0.0290 , -0.0300 , -0.0310 , -0.0260 , -0.0200 , -0.0060 , -0.0310 , -0.0550 , -0.1080 , -0.1090 , -0.1130 , -0.0700 , -0.0630 , -0.0550 , -0.0850 , -0.0880 , -0.0600 , -0.0100 ,
				0.0190 , 0.0100 , -0.0250 , -0.1040 , -0.2170 , -0.2840 , -0.2140 , -0.0350 , 0.1840 , 0.2980 , 0.3070 , 0.2940 , 0.2470 , 0.2310 , 0.1520 , 0.1330 , 0.0670 , 0.0160 ,
				0.5040 , 0.4980 , 0.4960 , 0.5090 , 0.5350 , 0.5390 , 0.5120 , 0.4690 , 0.4520 , 0.4630 , 0.4760 , 0.4900 , 0.4890 , 0.4930 , 0.4960 , 0.5390 , 0.5030 , 0.4720
		}
	};
	float db_average [ELE_OF_DB][AXIS_NUM] = {
			0.0158 , 0.0220 , 0.5210 ,
			-0.0549 , 0.0599 , 0.4964

	};

	float db_cov [ELE_OF_DB][AXIS_NUM] = {
			0.001295 , 0.019714 , 0.001098 ,
			0.001196 , 0.034426 , 0.000622

	};


	if (result_ = crossCorrelation(db_sample, db_average, db_cov))
		result_ = BACK;
	else
		result_ = FRONT;

	return result_;
}

int assignDB_RIGHT (void)	{
	int result_;
	printf ("\nassign DB Right\n");

	float db_sample [ELE_OF_DB][AXIS_NUM][WMA_SAMPLE_NUM] = {
			{ //right
					-0.0100 , 0.0000 , 0.0450 , 0.1220 , 0.2690 , 0.2770 , 0.0830 , -0.2460 , -0.4840 , -0.5380 , -0.4800 , -0.4140 , -0.3120 , -0.2090 , -0.0460 , 0.0900 , 0.1740 , 0.1880 ,
					0.0130 , 0.0120 , -0.0040 , -0.0260 , -0.0740 , -0.1260 , -0.1430 , -0.1010 , -0.0420 , -0.0180 , -0.0500 , -0.0800 , -0.1060 , -0.1090 , -0.1030 , -0.0860 , -0.0590 , -0.0290 ,
					0.5090 , 0.5110 , 0.5160 , 0.5380 , 0.5600 , 0.5410 , 0.4820 , 0.4200 , 0.4170 , 0.4460 , 0.4940 , 0.5000 , 0.4980 , 0.4610 , 0.4970 , 0.4610 , 0.4930 , 0.4480

			},
			{ //left
					-0.0160 , -0.0650 , -0.1210 , -0.1890 , -0.1490 , -0.0330 , 0.1620 , 0.3200 , 0.3900 , 0.4390 , 0.3930 , 0.3720 , 0.3270 , 0.2780 , 0.1800 , 0.1090 , 0.0830 , 0.0940 ,
					0.0000 , 0.0010 , 0.0100 , 0.0220 , 0.0340 , 0.0130 , -0.0310 , -0.0850 , -0.1280 , -0.1320 , -0.1220 , -0.1080 , -0.1260 , -0.1410 , -0.1240 , -0.0990 , -0.0900 , -0.0960 ,
					0.5110 , 0.5040 , 0.5410 , 0.5250 , 0.4790 , 0.4000 , 0.3860 , 0.4380 , 0.4780 , 0.4820 , 0.4650 , 0.4630 , 0.4760 , 0.5030 , 0.5070 , 0.5120 , 0.4920 , 0.4780

			}
	};
	float db_average [ELE_OF_DB][AXIS_NUM] = {
			-0.0828 , -0.0628 , 0.4884 ,
			0.1430 , -0.0668 , 0.4800
	};

	float db_cov [ELE_OF_DB][AXIS_NUM] = {
			0.073668 , 0.002257 ,0.001597 ,
			0.042386 , 0.004026 , 0.001603
	};


	if (result_ = crossCorrelation(db_sample, db_average, db_cov))
		result_ =LEFT ;
	else
		result_ = RIGHT;
	return result_;
}

int assignDB_UP (void)	{
	int result_;
	printf ("\nassign DB UP\n");

	float db_sample [ELE_OF_DB][AXIS_NUM][WMA_SAMPLE_NUM] = {
			{	//UP

					-0.0090 , -0.0100 , -0.0050 , 0.0030 , -0.0670 , -0.0880 , -0.0770 , -0.0160 , -0.0110 , -0.0490 , -0.0620 , -0.0310 , 0.0090 , 0.0310 , 0.0010 , -0.0430 , -0.0780 , -0.1000 ,
					-0.0370 , -0.0280 , -0.0270 , -0.0230 , -0.0820 , -0.1380 , -0.1510 , -0.0860 , -0.0140 , 0.0170 , 0.0210 , 0.0290 , 0.0330 , 0.0120 , -0.0380 , -0.0830 , -0.1300 , -0.1780 ,
					0.4940 , 0.4900 , 0.5170 , 0.5860 , 0.7540 , 0.8310 , 0.7290 , 0.4730 , 0.2900 , 0.2590 , 0.2300 , 0.1750 , 0.2000 , 0.2100 , 0.3050 , 0.5680 , 0.6870 , 0.8090
			},
			{	//down

					-0.0420 , -0.0400 , -0.0370 , -0.0510 , -0.0640 , -0.0580 , -0.0050 , -0.0040 , 0.0000 , -0.0250 , 0.0040 , 0.0070 , 0.0100 , 0.0050 , -0.0090 , -0.0220 , -0.0230 , -0.0130 ,
					0.0090 , 0.0130 , 0.0140 , 0.0320 , 0.0120 , -0.1070 , -0.1630 , -0.1650 , -0.1090 , -0.1360 , -0.1910 , -0.1950 , -0.1980 , -0.1630 , -0.1000 , -0.0240 , 0.0090 , -0.0100 ,
					0.5110 , 0.5050 , 0.4630 , 0.3400 , 0.1830 , 0.1080 , 0.2530 , 0.5350 , 0.7660 , 0.8550 , 0.8860 , 0.8650 , 0.8090 , 0.6570 , 0.4080 , 0.2330 , 0.2030 , 0.2950
			}
	};
	float db_average [ELE_OF_DB][AXIS_NUM] = {
			-0.0334 , -0.0502 , 0.4782 ,
			-0.0204 , -0.0818 , 0.4931
	};

	float db_cov [ELE_OF_DB][AXIS_NUM] = {
			0.001459 , 0.004353 , 0.050258 ,
			0.000555 , 0.007523 , 0.067750
	};


	if (result_ = crossCorrelation(db_sample, db_average, db_cov))
		result_ = DOWN;
	else
		result_ = UP;
	return result_;

}

int assignDB_CLOCK (void)	{
	int result_;
	printf ("\nassign DB Clk\n");

	float db_sample [ELE_OF_DB][AXIS_NUM][WMA_SAMPLE_NUM] = {
		{ //CLOCK
				0.0100 , 0.0140 , 0.0170 , -0.0380 , -0.1360 , -0.2010 , -0.0910 , 0.1250 , 0.3990 , 0.5950 , 0.6650 , 0.6200 , 0.4310 , 0.1390 , -0.2120 , -0.4710 , -0.4950 , -0.3930 ,
				-0.0170 , -0.0080 , 0.0150 , 0.0180 , -0.0210 , -0.0960 , -0.1260 , -0.1120 , -0.0620 , -0.0100 , 0.0190 , 0.0180 , -0.0370 , -0.0760 , -0.0690 , -0.0540 , -0.0470 , -0.0670 ,
				0.5350 , 0.5310 , 0.4590 , 0.3940 , 0.4020 , 0.5330 , 0.6930 , 0.7740 , 0.7310 , 0.5750 , 0.3580 , 0.1660 , 0.0380 , -0.0360 , 0.0230 , 0.1800 , 0.4420 , 0.6060
		},
		{ //ANTI-CLOCK
				0.0840 , 0.0690 , 0.0770 , 0.0810 , 0.1210 , 0.1830 , 0.2970 , 0.3620 , 0.1660 , -0.2600 , -0.5180 , -0.5380 , -0.3340 , -0.1820 , -0.0050 , 0.1710 , 0.2670 , 0.4010 ,
				-0.0030 , 0.0070 , 0.0210 , 0.0340 , 0.0430 , 0.0330 , -0.0320 , -0.1710 , -0.2660 , -0.3210 , -0.3010 , -0.3040 , -0.2710 , -0.2280 , -0.1270 , -0.0360 , 0.0450 , 0.1080 ,
				0.5700 , 0.5760 , 0.5270 , 0.4280 , 0.3120 , 0.2500 , 0.4100 , 0.6760 , 0.9240 , 0.9910 , 0.8450 , 0.6480 , 0.3560 , 0.1540 , -0.0170 , -0.0790 , -0.0220 , 0.0480
		}
	};
	float db_average [ELE_OF_DB][AXIS_NUM] = {
			0.0543, -0.0407 ,0.4113 ,
			0.0246 , -0.0983 ,0.4221
	};

	float db_cov [ELE_OF_DB][AXIS_NUM] = {
			0.131934 , 0.002080 , 0.060740 ,
			0.078812 , 0.022021 , 0.107395
	};


	if (result_ = crossCorrelation(db_sample, db_average, db_cov))
		result_ = ANTI_CLOCK;
	else
		result_ = CLOCK;
	return result_;

}



int assignDB_LOW (void)	{
	int result_;
	printf ("\nassign DB Low\n");

	float db_sample [ELE_OF_DB][AXIS_NUM][WMA_SAMPLE_NUM] = {
		{ //LOW CLOCK
				-0.0240 , -0.0310 , -0.1030 , -0.3040 , -0.3470 , -0.1390 , 0.2450 , 0.4740 , 0.5350 , 0.4880 , 0.3350 , 0.1080 , -0.1040 , -0.2850 , -0.3940 , -0.4280 , -0.3530 , -0.2200 ,
				0.0230 , 0.0230 , 0.0360 , 0.0430 , 0.0500 , 0.0940 , 0.1570 , 0.1710 , 0.0640 , -0.1620 , -0.4060 , -0.5230 , -0.4870 , -0.3610 , -0.2250 , -0.0750 , 0.0750 , 0.1590 ,
				0.5060 , 0.5020 , 0.4680 , 0.4570 , 0.5360 , 0.6960 , 0.7410 , 0.6820 , 0.5290 , 0.4710 , 0.4070 , 0.3550 , 0.3310 , 0.3310 , 0.3960 , 0.5460 , 0.5280 , 0.5890
		},
		{ //LOW ANTI
				-0.0410 , -0.0390 , -0.0270 , -0.0100 , 0.0610 , 0.1620 , 0.1750 , 0.0460 , -0.2520 , -0.4400 , -0.5390 , -0.4230 , -0.2530 , -0.0340 , 0.1580 , 0.3070 , 0.4000 , 0.4410 ,
				0.0080 , 0.0110 , 0.0100 , -0.0220 , -0.0400 , -0.0250 , 0.1210 , 0.1880 , 0.1690 , -0.0330 , -0.2430 , -0.4120 , -0.4870 , -0.4940 , -0.4700 , -0.3890 , -0.2460 , -0.0890 ,
				0.5190 , 0.5110 , 0.4690 , 0.4290 , 0.4250 , 0.5420 , 0.6840 , 0.7210 , 0.6310 , 0.4870 , 0.4370 , 0.4220 , 0.4200 , 0.3630 , 0.3600 , 0.3410 , 0.3960 , 0.3580
		},
	};
	float db_average [ELE_OF_DB][AXIS_NUM] = {
			-0.0304 , -0.0747 , 0.5039 ,
			-0.0171 , -0.1357 , 0.4731
	};

	float db_cov [ELE_OF_DB][AXIS_NUM] = {
			0.103770 , 0.052496 , 0.014333 ,
			0.078052 , 0.053042 , 0.012466
	};


	if (result_ = crossCorrelation(db_sample, db_average, db_cov))
		result_ = LOW_ANTI;
	else
		result_ = LOW_CLOCK;
	return result_;


}
/*
int assignDB_SIDE ()	{

	int result_;
	printf ("\nassign DB Side\n");

	float db_sample [ELE_OF_DB][AXIS_NUM][WMA_SAMPLE_NUM] = {
		{	//SIDE-CLK
			{0.0110 , 0.0720 , 0.1490 , 0.1750 , 0.1200 , 0.0560 , 0.0450 , 0.1130 , 0.1660 , 0.1770 , 0.1330 , 0.1240 , 0.1910 , 0.2280 , 0.2730 , 0.2270 , 0.1580 , 0.0660 },
			{0.0520 , 0.0350 , -0.1210 , -0.3610 , -0.5450 , -0.5440 , -0.3260 , 0.0290 , 0.3610 , 0.5520 , 0.4910 , 0.3100 , 0.1120 , -0.0080 , -0.0760 , -0.2050 , -0.3110 , -0.3360 },
			{0.4590 , 0.6430 , 0.8420 , 0.9320 , 0.6840 , 0.3330 , -0.0450 , -0.1830 , -0.2640 , -0.1010 , 0.1440 , 0.5300 , 0.8140 , 0.9220 , 0.8160 , 0.6400 , 0.4800 , 0.4310 }

		},
		{	//SIDE-ANTI
			{0.0110 , 0.0100 , 0.0210 , 0.0520 , 0.1120 , 0.1620 , 0.1790 , 0.1020 , -0.0460 , -0.1480 , -0.1840 , -0.1070 , -0.0430 , 0.0100 , 0.0450 , 0.0860 , 0.1370 , 0.1500 },
			{-0.0190 , -0.0400 , -0.1110 , -0.2290 , -0.3390 , -0.3480 , -0.2930 , -0.1610 , -0.0380 , 0.2160 , 0.4010 , 0.5070 , 0.3370 , 0.0480 , -0.2390 , -0.4040 , -0.4230 , -0.3330 },
			{0.4810 , 0.4110 , 0.3430 , 0.3240 , 0.4810 , 0.6930 , 0.9020 , 0.8350 , 0.7220 , 0.4140 , 0.3500 , 0.1240 , 0.0780 , -0.0810 , 0.0350 , 0.1930 , 0.4090 , 0.5430 }
		}
	};
	float db_average [ELE_OF_DB][AXIS_NUM] = {
			{0.1380, -0.0495, 0.4487},	//side-clk
			{0.0305, -0.0816, 0.4032} 	//side-anti
	};

	float db_cov [ELE_OF_DB][AXIS_NUM] = {
			{0.00490, 0.10865, 0.1516},	//side-clk
			{0.01116, 0.08157, 0.07431}	//side-anti

	};

	if (result_ = crossCorrelation(db_sample, db_average, db_cov))
		result_ = SIDE_ANTI;
	else
		result_ = SIDE_CLOCK;

	return result_;
}




int assignDB_FRONT_RL ()	{

	int result_;
	printf ("\nassign DB FRONT-RightLeft\n");

	float db_sample [ELE_OF_DB][AXIS_NUM][WMA_SAMPLE_NUM] = {
		{	//FRONT-RIGHT
			{-0.001, -0.03, -0.016, 0.01, 0.112, 0.035, 0.061, 0.154, 0.463, 0.606, 0.414, -0.044, -0.423, -0.468, -0.259, -0.072, 0.023, 0.05},
			{-0.023, 0.055, 0.218, 0.259, -0.019, -0.376, -0.498, -0.302, -0.029, 0.087, -0.035, -0.27, -0.352, -0.231, -0.02, 0.03, 0.057, 0.019},
			{0.461, 0.499, 0.531, 0.529, 0.504, 0.479, 0.499, 0.506, 0.536, 0.51, 0.461, 0.499, 0.531, 0.529, 0.504, 0.479, 0.499, 0.506}
		},
		{	//FRONT-LEFT
			{-0.033, -0.049, -0.047, 0.008, 0.131, 0.073, 0.03, -0.119, -0.245, -0.437, -0.29, 0.123, 0.467, 0.438, 0.24, 0.092, 0.021, -0.004},
			{-0.024, 0.053, 0.224, 0.398, 0.255, -0.211, -0.475, -0.431, -0.236, -0.28, -0.208, -0.081, 0.077, 0.011, -0.03, -0.079, -0.085, -0.074},
			{0.509, 0.534, 0.698, 0.721, 0.621, 0.359, 0.365, 0.439, 0.583, 0.553, 0.494, 0.41, 0.425, 0.479, 0.52, 0.532, 0.547, 0.548}

		}
	};
	float db_average [ELE_OF_DB][AXIS_NUM] = {
			{0.0342, -0.0794, 0.5034},	//FRONT-RIGHT
			{0.0222, -0.0664, 0.5187}  	//FRONT-LEFT
	};

	float db_cov [ELE_OF_DB][AXIS_NUM] = {
			{0.073508, 0.04413, 0.00051},	//FRONT-RIGHT
			{0.050656, 0.05078, 0.00996}	//FRONT-LEFT
	};

	if (result_ = crossCorrelation(db_sample, db_average, db_cov))
		result_ = SIDE_ANTI;
	else
		result_ = SIDE_CLOCK;

	return result_;
}





int assignDB_BACK_RL ()	{

	int result_;
	printf ("\nassign DB BACK-RightLeft\n");

	float db_sample [ELE_OF_DB][AXIS_NUM][WMA_SAMPLE_NUM] = {
		{	//BACK-RIGHT
			{-0.016, 0.029, 0.065, 0.011, 0.048, 0.112, 0.277, 0.349, 0.441, 0.403, 0.087, -0.275, -0.428, -0.244, 0.008, 0.11, 0.11, 0.075},
			{0.008, -0.233, -0.462, -0.389, -0.115, 0.236, 0.291, 0.238, 0.06, -0.052, -0.163, -0.267, -0.216, -0.1, 0.037, 0, -0.011, -0.037},
			{0.477, 0.513, 0.574, 0.518, 0.618, 0.632, 0.661, 0.389, 0.37, 0.358, 0.529, 0.537, 0.654, 0.628, 0.563, 0.502, 0.486, 0.5}

		},
		{	//BACK-LEFT
			{0.064, 0.07, 0.057, 0.052, 0.061, 0.076, 0.059, 0.025, -0.086, -0.265, -0.379, -0.238, 0.034, 0.271, 0.312, 0.266, 0.164, 0.077},
			{-0.081, -0.164, -0.272, -0.369, -0.344, -0.136, 0.14, 0.388, 0.413, 0.296, -0.026, -0.202, -0.231, -0.117, -0.088, -0.086, -0.078, -0.069},
			{0.509, 0.506, 0.482, 0.5, 0.478, 0.522, 0.497, 0.556, 0.523, 0.517, 0.5, 0.569, 0.48, 0.494, 0.412, 0.509, 0.47, 0.493}

		}
	};
	float db_average [ELE_OF_DB][AXIS_NUM] = {
		{0.0646, -0.0653, 0.5283}, //BACK-RIGHT
		{0.0344, -0.0570, 0.5009},  //BACK-LEFT
	};

	float db_cov [ELE_OF_DB][AXIS_NUM] = {
		{0.050382, 0.042138, 0.008543},	//BACK-RIGHT
		{0.032978, 0.052047, 0.001137}//BACK-LEFT
	};

	if (result_ = crossCorrelation(db_sample, db_average, db_cov))
		result_ = SIDE_ANTI;
	else
		result_ = SIDE_CLOCK;

	return result_;
}




int assignDB_RIGHT_FB ()	{

	int result_;
	printf ("\nassign DB RIGHT-frontBack\n");

	float db_sample [ELE_OF_DB][AXIS_NUM][WMA_SAMPLE_NUM] = {
		{
			{0.035, 0.244, 0.425, 0.387, 0.016, -0.348, -0.447, -0.312, -0.133, 0.012, 0.102, 0.11, 0.083, 0.051, 0.068, 0.057, 0.041, 0.019},
			{-0.071, -0.023, 0.003, 0.002, -0.098, -0.158, -0.067, 0.121, 0.285, 0.254, 0.014, -0.286, -0.398, -0.324, -0.162, -0.098, -0.07, -0.07},
			{0.53, 0.541, 0.515, 0.508, 0.487, 0.551, 0.638, 0.687, 0.616, 0.54, 0.416, 0.36, 0.38, 0.432, 0.507, 0.493, 0.515, 0.528}

		},
		{
			{0.053, 0.168, 0.353, 0.446, 0.245, -0.164, -0.457, -0.397, -0.147, 0.046, 0.112, 0.107, 0.122, 0.097, 0.069, 0.009, -0.001, 0.007},
			{-0.005, -0.022, 0.024, 0.085, 0.025, -0.108, -0.259, -0.326, -0.396, -0.389, -0.177, 0.115, 0.384, 0.373, 0.233, 0.02, -0.052, -0.064},
			{0.523, 0.525, 0.546, 0.539, 0.546, 0.471, 0.442, 0.411, 0.457, 0.456, 0.523, 0.521, 0.568, 0.544, 0.574, 0.551, 0.519, 0.503}

		}
	};
	float db_average [ELE_OF_DB][AXIS_NUM] = {
		{0.0228, -0.0637, 0.5136}, //RIGHT-front
		{0.0371, -0.0299, 0.5122} 	//RIGHT-back
	};

	float db_cov [ELE_OF_DB][AXIS_NUM] = {
		{0.050592418, 0.030769412, 0.006952261},	//RIGHT-front
		{0.050591163, 0.051681232, 0.002125324}	//RIGHT-back
	};

	if (result_ = crossCorrelation(db_sample, db_average, db_cov))
		result_ = RIGHT_F;
	else
		result_ = RIGHT_B;

	return result_;
}





int assignDB_LEFT_FB ()	{

	int result_;
	printf ("\nassign DB RIGHT-frontBack\n");

	float db_sample [ELE_OF_DB][AXIS_NUM][WMA_SAMPLE_NUM] = {
		{
			{-0.042, -0.232, -0.463, -0.451, -0.108, 0.362, 0.532, 0.359, 0.136, 0.074, 0.123, 0.027, 0.011, 0, 0.083, 0.034, 0.033, 0.031},
			{0.021, 0.005, -0.092, -0.234, -0.229, -0.115, 0.013, 0.121, 0.235, 0.193, -0.064, -0.393, -0.36, -0.254, -0.084, -0.129, -0.083, -0.081},
			{0.511, 0.457, 0.39, 0.456, 0.467, 0.62, 0.579, 0.67, 0.692, 0.665, 0.493, 0.315, 0.435, 0.479, 0.521, 0.4, 0.49, 0.519}

		},
		{
			{-0.032, -0.108, -0.303, -0.415, -0.231, 0.111, 0.384, 0.338, 0.212, 0.097, 0.037, 0.003, -0.047, -0.034, -0.023, -0.016, -0.049, -0.048},
			{-0.022, 0.01, -0.043, -0.14, -0.128, -0.061, 0.049, -0.016, -0.114, -0.234, -0.325, -0.223, -0.029, 0.228, 0.291, 0.211, 0.062, -0.014},
			{0.512, 0.502, 0.521, 0.568, 0.571, 0.515, 0.502, 0.488, 0.535, 0.514, 0.521, 0.485, 0.509, 0.455, 0.49, 0.477, 0.549, 0.55}

		}
	};
	float db_average [ELE_OF_DB][AXIS_NUM] = {
		{0.0283, -0.0850, 0.5088},
		{-0.0069, -0.0277, 0.5147}
	};

	float db_cov [ELE_OF_DB][AXIS_NUM] = {
		{0.062566, 0.029200, 0.010539},
		{0.039284, 0.025894, 0.000969}
	};

	if (result_ = crossCorrelation(db_sample, db_average, db_cov))
		result_ = LEFT_F;
	else
		result_ = LEFT_B;

	return result_;
}




*/





unsigned char gestureRecognition (int16_t sensor [AXIS_NUM][SAMPLE_NUM])	{


	int min_ [AXIS_NUM] = {10000,10000,30000};
	int max_ [AXIS_NUM] = {-10000, -10000,-1000};

	int gaps [AXIS_NUM] = {0,0,0};
	int maxFlag [AXIS_NUM] = {0,0,0};	//# of sample upto 32
	int minFlag [AXIS_NUM] = {0,0,0};	//# of sample upto 32
	uint8_t Motionflag_ = 0;
	int nLoop, nLoop2;
	int result_ = 0 ;

	memset (WEIGHTED_AXIS, 2, sizeof (WEIGHTED_AXIS));

	normalizingFunction (sensor);


#ifdef WMA_FACTOR
	weightedMovingAverage (sensor);
#endif


	int16_t axs_prnt = 0;
	char temp_c [12] = {0, };

	//	printf ("<INPUT>\n");
	for (nLoop = 0; nLoop < AXIS_NUM; nLoop ++)	{
		for (nLoop2 = 0; nLoop2 < WMA_SAMPLE_NUM; nLoop2 ++)	{
			//LOG_NOR ("%d\t",  );
			//LOG_NOR ("%d\t", sensor [nLoop][nLoop2] );
			sprintf (temp_c,"%d ", WMA_SENSOR [nLoop][nLoop2]);
			//axs_prnt = WMA_SENSOR [nLoop][nLoop2];

			USART1_SEND (temp_c, 5);
		}
		//LOG_NOR ("\n");
		USART1_SEND (" / ", 3);
	}
	USART1_SEND (" / ", 3);


	char temp_flag = 0;
		for (nLoop = ACC_X; nLoop <= ACC_Z; nLoop++ )	{

			for (nLoop2 = 0; nLoop2 < WMA_SAMPLE_NUM; nLoop2 ++)	{

				if (min_[nLoop] > WMA_SENSOR [nLoop][nLoop2])	{
					min_[nLoop] = WMA_SENSOR [nLoop][nLoop2];
					minFlag [nLoop] |= 0x1;
				} else if (max_[nLoop] < WMA_SENSOR [nLoop][nLoop2])	{
					max_[nLoop] = WMA_SENSOR [nLoop][nLoop2];
					maxFlag [nLoop] |= 0x1;
				}
				minFlag[nLoop] = minFlag[nLoop] << 1;
				maxFlag[nLoop] = maxFlag[nLoop] << 1;

			}

			gaps [nLoop] = max_ [nLoop] - min_ [nLoop];

			if (gaps [nLoop] > 350)	{
				Motionflag_ |= 0x1;
				temp_flag ++;
				WEIGHTED_AXIS [nLoop] = 90;

			}

			Motionflag_ = Motionflag_ << 1;

		} //for

		if (temp_flag == 3)	{
			int nLoop3 = 0, temp_min=2000, order = 0;
			for (nLoop3=0; nLoop3 < 3; nLoop3 ++ )	{
				if (gaps [nLoop3] < temp_min)	{
					order = nLoop3;
					temp_min = gaps [nLoop3];
				}
			}


			switch (order)	{
			case 0:
				Motionflag_ = 0x6;
				break;
			case 1:
				Motionflag_ = 0xA;
				WEIGHTED_AXIS [order] = 10;
				temp_flag --;
				break;
			case 2:
				Motionflag_ = 0xC;
				WEIGHTED_AXIS [order] = 10;
				temp_flag --;
				break;

			}

			sprintf (temp_c,"%d, %d ! ",temp_flag, order );
			USART1_SEND (temp_c, 6);

		}




		for (nLoop = ACC_X; nLoop <= ACC_Z; nLoop++ )	{
			WEIGHTED_AXIS [nLoop] /= temp_flag;
		}


		if (!Motionflag_ )	{
			result_ = NONE;
		}
		else {
			int temp_result [NUM_OF_DB], max=0;
			memset (temp_result, 0, sizeof(temp_result));

			switch (Motionflag_ & 0x0E)	{
			case 0x0:
				result_ = NONE;
				break;
			case 0x4:	//FRONT
				result_ = FRONT;
				/*WEIGHTED_AXIS [0] = 5;
				WEIGHTED_AXIS [1] = 90;
				WEIGHTED_AXIS [2] = 5;
	*/
				result_ = assignDB_FRONT ();
				break;
			case 0x8:		//SIDE
				result_ = RIGHT;
				/*WEIGHTED_AXIS [0] = 90;
				WEIGHTED_AXIS [1] = 5;
				WEIGHTED_AXIS [2] = 5;
				*/result_ = assignDB_RIGHT ();
				break;
			case 0x2:	//UP
				result_ = UP;
			/*	WEIGHTED_AXIS [0] = 5;
				WEIGHTED_AXIS [1] = 5;
				WEIGHTED_AXIS [2] = 90;
				*/result_ = assignDB_UP ();
				break;
			case 0xA:

				if (gaps[0] + gaps[2] < 1700)	{

					temp_result [0] = assignDB_CLOCK ();
					biggestValue [CLOCK] = biggestValue [0];
					biggestValue [ANTI_CLOCK] = biggestValue [1];

					temp_result [1] = assignDB_UP ();
					biggestValue [UP] = biggestValue [0];
					biggestValue [DOWN] = biggestValue [1];

					temp_result [2] = assignDB_RIGHT ();
					biggestValue [RIGHT] = biggestValue [0];
					biggestValue [LEFT] = biggestValue [1];


					for (nLoop = 0; nLoop < 3; nLoop ++)	{
						if (max < biggestValue [temp_result[nLoop]])	{
							max = biggestValue [temp_result[nLoop]];
							result_ = temp_result[nLoop];
						}
					}
				}
				else	{
					result_ = assignDB_CLOCK ();
				}



				break;
			case 0xC:
				if (gaps[0] + gaps[1] < 1700)	{


					temp_result [0] = assignDB_LOW ();
					biggestValue [LOW_CLOCK] = biggestValue [0];
					biggestValue [LOW_ANTI] = biggestValue [1];

					temp_result [1] = assignDB_FRONT ();
					biggestValue [FRONT] = biggestValue [0];
					biggestValue [BACK] = biggestValue [1];

					temp_result [2] = assignDB_RIGHT ();
					biggestValue [RIGHT] = biggestValue [0];
					biggestValue [LEFT] = biggestValue [1];

					for (nLoop = 0; nLoop < 3; nLoop ++)	{
						if (max < biggestValue [temp_result[nLoop]])	{
							max = biggestValue [temp_result[nLoop]];
							result_ = temp_result[nLoop];
						}
					}
				}
				else 	{
					result_ = assignDB_LOW ();

				}

				break;

			default:
				temp_result [0] = assignDB_CLOCK ();
				biggestValue [CLOCK] = biggestValue [0];
				biggestValue [ANTI_CLOCK] = biggestValue [1];

				temp_result [1] = assignDB_UP ();
				biggestValue [UP] = biggestValue [0];
				biggestValue [DOWN] = biggestValue [1];

				temp_result [2] = assignDB_RIGHT ();
				biggestValue [RIGHT] = biggestValue [0];
				biggestValue [LEFT] = biggestValue [1];

				temp_result [3] = assignDB_LOW ();
				biggestValue [LOW_CLOCK] = biggestValue [0];
				biggestValue [LOW_ANTI] = biggestValue [1];

				temp_result [4] = assignDB_FRONT ();
				biggestValue [FRONT] = biggestValue [0];
				biggestValue [BACK] = biggestValue [1];


				for (nLoop = 0; nLoop < 5; nLoop ++)	{
					if (max < biggestValue [temp_result[nLoop]])	{
						max = biggestValue [temp_result[nLoop]];
						result_ = temp_result[nLoop];
					}
				}
				break;

			} //switch
		}	//else

	return result_;


}

#ifndef LIBMATH_INCLUDE
double mysqrt(unsigned int src)
{
    unsigned int NUM_REPEAT = 16;
    unsigned int k;
    double t;
    double buf = (double)src;
    for(k=0,t=buf;k<NUM_REPEAT;k++)
    {
        if(t<1.0)
            break;
        t = (t*t+buf)/(2.0*t);
    }
    return t;
}
#endif
