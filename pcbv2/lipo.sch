EESchema Schematic File Version 4
LIBS:badge2-cache
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 4 5
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Battery_Management:MCP73831-2-OT U1
U 1 1 5D9D3610
P 5750 3300
F 0 "U1" H 5450 3550 50  0000 C CNN
F 1 "MCP73831-2-OT" H 5750 3650 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-23-5" H 5800 3050 50  0001 L CIN
F 3 "http://ww1.microchip.com/downloads/en/DeviceDoc/20001984g.pdf" H 5600 3250 50  0001 C CNN
	1    5750 3300
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0108
U 1 1 5D9D4362
P 5750 3800
F 0 "#PWR0108" H 5750 3550 50  0001 C CNN
F 1 "GND" H 5755 3627 50  0000 C CNN
F 2 "" H 5750 3800 50  0001 C CNN
F 3 "" H 5750 3800 50  0001 C CNN
	1    5750 3800
	1    0    0    -1  
$EndComp
Wire Wire Line
	5750 3600 5750 3700
$Comp
L power:+5V #PWR0109
U 1 1 5D9D5A8E
P 5750 2650
F 0 "#PWR0109" H 5750 2500 50  0001 C CNN
F 1 "+5V" H 5765 2823 50  0000 C CNN
F 2 "" H 5750 2650 50  0001 C CNN
F 3 "" H 5750 2650 50  0001 C CNN
	1    5750 2650
	1    0    0    -1  
$EndComp
Wire Wire Line
	5750 2650 5750 2750
Wire Wire Line
	6150 3200 6650 3200
$Comp
L Device:Battery_Cell BT2
U 1 1 5D9D64A0
P 6650 3400
F 0 "BT2" H 6768 3496 50  0000 L CNN
F 1 "Battery_Cell" H 6768 3405 50  0000 L CNN
F 2 "Connector_JST:JST_EH_S2B-EH_1x02_P2.50mm_Horizontal" V 6650 3460 50  0001 C CNN
F 3 "~" V 6650 3460 50  0001 C CNN
	1    6650 3400
	1    0    0    -1  
$EndComp
Wire Wire Line
	6650 3500 6650 3700
Connection ~ 5750 3700
Wire Wire Line
	5750 3700 5750 3800
$Comp
L Device:R R3
U 1 1 5D9D776C
P 5200 3550
F 0 "R3" H 5270 3596 50  0000 L CNN
F 1 "2K" H 5270 3505 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric" V 5130 3550 50  0001 C CNN
F 3 "~" H 5200 3550 50  0001 C CNN
	1    5200 3550
	1    0    0    -1  
$EndComp
Wire Wire Line
	5350 3400 5200 3400
Wire Wire Line
	5200 3700 5750 3700
$Comp
L power:GND #PWR0110
U 1 1 5D9D80D2
P 6650 3700
F 0 "#PWR0110" H 6650 3450 50  0001 C CNN
F 1 "GND" H 6655 3527 50  0000 C CNN
F 2 "" H 6650 3700 50  0001 C CNN
F 3 "" H 6650 3700 50  0001 C CNN
	1    6650 3700
	1    0    0    -1  
$EndComp
$Comp
L Device:R R4
U 1 1 5D9D8512
P 6300 3550
F 0 "R4" H 6370 3596 50  0000 L CNN
F 1 "4.7K" H 6370 3505 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric" V 6230 3550 50  0001 C CNN
F 3 "~" H 6300 3550 50  0001 C CNN
	1    6300 3550
	1    0    0    -1  
$EndComp
Wire Wire Line
	6150 3400 6300 3400
Wire Wire Line
	6300 3700 6300 3850
$Comp
L Device:LED D10
U 1 1 5D9D8F23
P 6300 4000
F 0 "D10" V 6247 4078 50  0000 L CNN
F 1 "LED" V 6338 4078 50  0000 L CNN
F 2 "LED_SMD:LED_0603_1608Metric" H 6300 4000 50  0001 C CNN
F 3 "~" H 6300 4000 50  0001 C CNN
	1    6300 4000
	0    1    1    0   
$EndComp
$Comp
L power:+5V #PWR0111
U 1 1 5D9D9257
P 6300 4350
F 0 "#PWR0111" H 6300 4200 50  0001 C CNN
F 1 "+5V" H 6315 4523 50  0000 C CNN
F 2 "" H 6300 4350 50  0001 C CNN
F 3 "" H 6300 4350 50  0001 C CNN
	1    6300 4350
	-1   0    0    1   
$EndComp
Wire Wire Line
	6300 4150 6300 4350
$Comp
L RT9013:RT9013-XXXB U8
U 1 1 5D9DA7E0
P 8000 3400
F 0 "U8" H 8000 3886 59  0000 C CNN
F 1 "RT9013-XXXB" H 8000 3781 59  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-23-5" H 8000 3400 50  0001 C CNN
F 3 "" H 8000 3400 50  0001 C CNN
	1    8000 3400
	1    0    0    -1  
$EndComp
Connection ~ 6650 3200
Wire Wire Line
	7450 3200 7450 3400
Wire Wire Line
	7450 3400 7600 3400
Connection ~ 7450 3200
Wire Wire Line
	7450 3200 7600 3200
$Comp
L power:GND #PWR0112
U 1 1 5D9DC919
P 8000 3950
F 0 "#PWR0112" H 8000 3700 50  0001 C CNN
F 1 "GND" H 8005 3777 50  0000 C CNN
F 2 "" H 8000 3950 50  0001 C CNN
F 3 "" H 8000 3950 50  0001 C CNN
	1    8000 3950
	1    0    0    -1  
$EndComp
Wire Wire Line
	8000 3950 8000 3850
$Comp
L power:+3.3V #PWR0113
U 1 1 5D9DD807
P 8600 3100
F 0 "#PWR0113" H 8600 2950 50  0001 C CNN
F 1 "+3.3V" H 8615 3273 50  0000 C CNN
F 2 "" H 8600 3100 50  0001 C CNN
F 3 "" H 8600 3100 50  0001 C CNN
	1    8600 3100
	1    0    0    -1  
$EndComp
Wire Wire Line
	8600 3100 8600 3200
$Comp
L Device:C C2
U 1 1 5D9E2DE4
P 8600 3450
F 0 "C2" H 8715 3496 50  0000 L CNN
F 1 "4.7uF" H 8715 3405 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 8638 3300 50  0001 C CNN
F 3 "~" H 8600 3450 50  0001 C CNN
	1    8600 3450
	1    0    0    -1  
$EndComp
Wire Wire Line
	8600 3600 8600 3850
Wire Wire Line
	8600 3850 8000 3850
Wire Wire Line
	6650 3200 7450 3200
Connection ~ 8000 3850
Wire Wire Line
	8000 3850 8000 3800
$Comp
L Device:C C1
U 1 1 5D9E3D42
P 5900 2750
F 0 "C1" V 5648 2750 50  0000 C CNN
F 1 "4.7uF" V 5739 2750 50  0000 C CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 5938 2600 50  0001 C CNN
F 3 "~" H 5900 2750 50  0001 C CNN
	1    5900 2750
	0    1    1    0   
$EndComp
Connection ~ 5750 2750
Wire Wire Line
	5750 2750 5750 3000
$Comp
L power:GND #PWR0114
U 1 1 5D9E4CEC
P 6250 2750
F 0 "#PWR0114" H 6250 2500 50  0001 C CNN
F 1 "GND" H 6255 2577 50  0000 C CNN
F 2 "" H 6250 2750 50  0001 C CNN
F 3 "" H 6250 2750 50  0001 C CNN
	1    6250 2750
	1    0    0    -1  
$EndComp
Wire Wire Line
	6250 2750 6050 2750
Wire Wire Line
	8400 3200 8600 3200
Connection ~ 8600 3200
Wire Wire Line
	8600 3200 8600 3300
Wire Notes Line
	5100 3250 5100 3800
Wire Notes Line
	5100 3800 3350 3800
Wire Notes Line
	3350 3800 3350 3250
Wire Notes Line
	3350 3250 5100 3250
Text Notes 3400 3750 0    50   ~ 0
R3 determines the charge rate of the LiPo:\nR(Ω) = 1000 / max current (A) \n\nExamples:\n• 2kΩ for 500mA\n• 10kΩ for 100mA
$EndSCHEMATC
