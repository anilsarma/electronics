EESchema Schematic File Version 4
LIBS:basic_humidity-cache
EELAYER 29 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Wire Wire Line
	7250 700  7250 750 
Text Notes 7500 7500 0    50   ~ 0
Water Monitor/Trigger
$Comp
L Device:R R1
U 1 1 5D256831
P 1550 2400
F 0 "R1" H 1620 2446 50  0000 L CNN
F 1 "R10K" H 1620 2355 50  0000 L CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P15.24mm_Horizontal" V 1480 2400 50  0001 C CNN
F 3 "~" H 1550 2400 50  0001 C CNN
	1    1550 2400
	1    0    0    -1  
$EndComp
$Comp
L Device:LED D1
U 1 1 5D25A7DD
P 1850 2050
F 0 "D1" H 1843 2266 50  0000 C CNN
F 1 "LED" H 1843 2175 50  0000 C CNN
F 2 "LED_THT:LED_D3.0mm_Clear" H 1850 2050 50  0001 C CNN
F 3 "~" H 1850 2050 50  0001 C CNN
	1    1850 2050
	1    0    0    -1  
$EndComp
Wire Wire Line
	1700 2050 1550 2050
Wire Wire Line
	1550 2050 1550 2250
$Comp
L power:GND #PWR0101
U 1 1 5D293088
P 8150 1200
F 0 "#PWR0101" H 8150 950 50  0001 C CNN
F 1 "GND" H 8155 1027 50  0000 C CNN
F 2 "" H 8150 1200 50  0001 C CNN
F 3 "" H 8150 1200 50  0001 C CNN
	1    8150 1200
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR0102
U 1 1 5D293821
P 8150 1100
F 0 "#PWR0102" H 8150 950 50  0001 C CNN
F 1 "+5V" H 8165 1273 50  0000 C CNN
F 2 "" H 8150 1100 50  0001 C CNN
F 3 "" H 8150 1100 50  0001 C CNN
	1    8150 1100
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0106
U 1 1 5D29D272
P 1550 2550
F 0 "#PWR0106" H 1550 2300 50  0001 C CNN
F 1 "GND" H 1555 2377 50  0000 C CNN
F 2 "" H 1550 2550 50  0001 C CNN
F 3 "" H 1550 2550 50  0001 C CNN
	1    1550 2550
	1    0    0    -1  
$EndComp
$Comp
L Connector:Screw_Terminal_01x02 J1
U 1 1 5D29E49A
P 8350 1100
F 0 "J1" H 8430 1092 50  0000 L CNN
F 1 "Screw_Terminal_01x02" H 8430 1001 50  0000 L CNN
F 2 "TerminalBlock:TerminalBlock_bornier-2_P5.08mm" H 8350 1100 50  0001 C CNN
F 3 "~" H 8350 1100 50  0001 C CNN
	1    8350 1100
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR0108
U 1 1 5D2A5559
P 7700 950
F 0 "#PWR0108" H 7700 800 50  0001 C CNN
F 1 "+5V" H 7715 1123 50  0000 C CNN
F 2 "" H 7700 950 50  0001 C CNN
F 3 "" H 7700 950 50  0001 C CNN
	1    7700 950 
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0109
U 1 1 5D2A5CE0
P 7700 1250
F 0 "#PWR0109" H 7700 1000 50  0001 C CNN
F 1 "GND" H 7705 1077 50  0000 C CNN
F 2 "" H 7700 1250 50  0001 C CNN
F 3 "" H 7700 1250 50  0001 C CNN
	1    7700 1250
	1    0    0    -1  
$EndComp
$Comp
L Device:C C1
U 1 1 5D2A488E
P 7700 1100
F 0 "C1" H 7815 1146 50  0000 L CNN
F 1 "C" H 7815 1055 50  0000 L CNN
F 2 "Capacitor_THT:C_Disc_D5.0mm_W2.5mm_P2.50mm" H 7738 950 50  0001 C CNN
F 3 "~" H 7700 1100 50  0001 C CNN
	1    7700 1100
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR0110
U 1 1 5D2BEF37
P 5350 1600
F 0 "#PWR0110" H 5350 1450 50  0001 C CNN
F 1 "+5V" H 5365 1773 50  0000 C CNN
F 2 "" H 5350 1600 50  0001 C CNN
F 3 "" H 5350 1600 50  0001 C CNN
	1    5350 1600
	1    0    0    -1  
$EndComp
$Comp
L Connector:Screw_Terminal_01x03 J2
U 1 1 5D2CB9EB
P 5900 1200
F 0 "J2" H 5980 1242 50  0000 L CNN
F 1 "Screw_Terminal_01x03" H 5980 1151 50  0000 L CNN
F 2 "TerminalBlock:TerminalBlock_bornier-3_P5.08mm" H 5900 1200 50  0001 C CNN
F 3 "~" H 5900 1200 50  0001 C CNN
	1    5900 1200
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR0112
U 1 1 5D2CCF8E
P 5700 1100
F 0 "#PWR0112" H 5700 950 50  0001 C CNN
F 1 "+5V" H 5715 1273 50  0000 C CNN
F 2 "" H 5700 1100 50  0001 C CNN
F 3 "" H 5700 1100 50  0001 C CNN
	1    5700 1100
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0113
U 1 1 5D2CD6BF
P 5700 1300
F 0 "#PWR0113" H 5700 1050 50  0001 C CNN
F 1 "GND" H 5705 1127 50  0000 C CNN
F 2 "" H 5700 1300 50  0001 C CNN
F 3 "" H 5700 1300 50  0001 C CNN
	1    5700 1300
	1    0    0    -1  
$EndComp
Wire Wire Line
	5700 1200 4700 1200
$Comp
L Sensor:DHT11 U2
U 1 1 5D2BE7D6
P 5350 1900
F 0 "U2" H 5106 1946 50  0000 R CNN
F 1 "DHT11" H 5106 1855 50  0000 R CNN
F 2 "Sensor:Aosong_DHT11_5.5x12.0_P2.54mm" H 5350 1500 50  0001 C CNN
F 3 "http://akizukidenshi.com/download/ds/aosong/DHT11.pdf" H 5500 2150 50  0001 C CNN
	1    5350 1900
	1    0    0    -1  
$EndComp
Text Notes 5900 1450 0    50   ~ 0
Relay
Wire Wire Line
	4700 1200 4700 3000
Wire Wire Line
	4100 3000 4700 3000
Wire Wire Line
	3100 3000 3050 3000
$Comp
L power:GND #PWR0103
U 1 1 5D2BFDAA
P 5350 2200
F 0 "#PWR0103" H 5350 1950 50  0001 C CNN
F 1 "GND" H 5355 2027 50  0000 C CNN
F 2 "" H 5350 2200 50  0001 C CNN
F 3 "" H 5350 2200 50  0001 C CNN
	1    5350 2200
	1    0    0    -1  
$EndComp
$Comp
L MCU_Module:Arduino_Nano_v2.x A1
U 1 1 5D224824
P 3600 2900
F 0 "A1" H 3600 1811 50  0000 C CNN
F 1 "Arduino_Nano_v2.x" H 3600 1720 50  0000 C CNN
F 2 "Module:Arduino_Nano_WithMountingHoles" H 3750 1950 50  0001 L CNN
F 3 "https://www.arduino.cc/en/uploads/Main/ArduinoNanoManual23.pdf" H 3600 1900 50  0001 C CNN
	1    3600 2900
	1    0    0    -1  
$EndComp
Wire Wire Line
	2000 2800 2000 2050
Wire Wire Line
	3100 2800 2000 2800
Wire Wire Line
	5650 3600 4100 3600
Wire Wire Line
	5650 1900 5650 3600
$Comp
L power:+5V #PWR0104
U 1 1 5D2EE184
P 3500 1900
F 0 "#PWR0104" H 3500 1750 50  0001 C CNN
F 1 "+5V" H 3515 2073 50  0000 C CNN
F 2 "" H 3500 1900 50  0001 C CNN
F 3 "" H 3500 1900 50  0001 C CNN
	1    3500 1900
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 5D2F0890
P 3600 3900
F 0 "#PWR?" H 3600 3650 50  0001 C CNN
F 1 "GND" H 3605 3727 50  0000 C CNN
F 2 "" H 3600 3900 50  0001 C CNN
F 3 "" H 3600 3900 50  0001 C CNN
	1    3600 3900
	1    0    0    -1  
$EndComp
$EndSCHEMATC
