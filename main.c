/*
 * EnergyMeter.c
 *
 * Created: 6/4/2017 11:49:53 PM
 * Author : sshakya
 */ 

 
#ifndef F_CPU
#define F_CPU 1000000UL                    // set the CPU clock
#endif

//FOR LCD
#define RS  6
#define E   7
#define ctrl PORTD


#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>


// for conversion from long to float
char buf[10];


//.............................Begin LCD Funcations......................................................


int lcd_data(char t)
{   
	ctrl |= (1<<RS);
	PORTB=t;
	ctrl |= (1<<E);
	_delay_ms(1);
	ctrl &= ~(1<<E);
	_delay_ms(1);
	t = t << 4;
	PORTB=t;
	ctrl |= (1<<E);
	_delay_ms(1);
	ctrl &= ~(1<<E);
	_delay_ms(1);
	return 0;
}


int writecmd(char z)
{
	ctrl &= ~(1<<RS);
	PORTB=z;
	ctrl |= (1<<E);
	_delay_ms(1);
	ctrl &= ~(1<<E);
	_delay_ms(1);
	z = z << 4;
	PORTB=z;
	ctrl |= (1<<E);
	_delay_ms(1);
	ctrl &= ~(1<<E);
	_delay_ms(1);
	return 0;
}

void lcd_print(char const *str)
{
	unsigned char k=0;
	while (str[k]!=0)
	{
		lcd_data(str[k]);
		k++;
	}
}


void lcd_init(void)
{
	writecmd(0x02);
	writecmd(0x28);
	writecmd(0x0c);
	writecmd(0x01);
	writecmd(0x06);
}

void lcd_gotoxy(unsigned char x, unsigned char y)
{
	unsigned char firstcharadrs[] = {0x80, 0xC0,0x94,0xD4};
	writecmd(firstcharadrs[y-1] + x - 1);
	_delay_us(100);
}

//___________End of LCD Functions___________




/**************** ADC Functions Start ************/
void adc_init()
{
	ADMUX = (1<<REFS0) | (0<<REFS1); 
	// ADC Enable and prescaler of 128
	// 8000000/128 = 62500
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}

// read adc value
int adc_read(int ch)
{
	// select the corresponding channel 0~7
	// ANDing with '7' will always keep the value
	// of 'ch' between 0 and 7
	ch &= 0b00000111;  // AND operation with 7
	ADMUX = (ADMUX & 0xF8)|ch;     // clears the bottom 3 bits before ORing
 
	// start single conversion
	// write '1' to ADSC
	ADCSRA |= (1<<ADSC);
 
	// wait for conversion to complete
	// ADSC becomes '0' again
	// till then, run loop continuously
	while(ADCSRA & (1<<ADSC));
 
	return (ADC);
}

/**************** ADC Functions End ************/

void lcd_print_num(int num)
{
	lcd_data(num/100 + 0x30);
	num = num%100;
	lcd_data(num/10 + 0x30);
	lcd_data(num%10 + 0x30);	
}

void lcd_clear(void)  //funtion to clear data on LCD
{
	writecmd(0x01); //to clear previous data if any
	_delay_ms(2); //approx. process time
}

double read_volt()
{
	int i;
	// ADC Variables
	unsigned long volt = 0.0;
	unsigned long v[10];
	double conv;

	// 10 Samples
	for (i=0; i <= 9; i++){
		v[i] = adc_read(0);
		volt = volt + v[i];
		_delay_us(10);
	}
	volt = volt /10;

	conv = volt * 0.4235; //0.377; // 386/1024, 350//990, 5*0.94*101/1024, 
	return conv;
}

double read_current()
{
	int j;
	// ADC Variables
	unsigned long current = 0.0;
	unsigned long i[10];
	double conv;

	// 10 Samples
	for (j=0; j <= 9; j++){
		j[i] = adc_read(1);
		current = current + i[j];
		_delay_us(10);
	}
	current = current/10;
	conv = current*0.0048828;// 5/1024
	return conv;
}


int main(void)
{
	// PORT initialization
	DDRB = 0xff;
	DDRC = 0x00;
	DDRD = 0b11000000;
 
 
	//__LCD Initialization____
	lcd_init();
	lcd_gotoxy(1,1);

	// initialize ADC
	adc_init();
 
	// loop forever
	while(1)
	{
		lcd_clear();
		double volt_conv;
		volt_conv = read_volt();
		itoa(volt_conv,buf,10);
		lcd_gotoxy(1,1);
		lcd_print("Voltage = ");
		lcd_print(buf);
		lcd_data('V');
		double curr_conv; 
		int curr_conv_rem;
		curr_conv = read_current();
		if (curr_conv < 1.0)
		curr_conv = 2.12*(curr_conv + 0.27); //(0.94/0.47=2) 0.94 % of DC voltage = rms voltage
		else curr_conv = 2*curr_conv;
		curr_conv_rem = curr_conv/1;
		itoa(curr_conv,buf,10);
		curr_conv_rem = (curr_conv - curr_conv_rem)*100;
		lcd_gotoxy(1,2);
		lcd_print("Current = ");
		lcd_print(buf);
		lcd_data('.');
		itoa(curr_conv_rem,buf,10);
		lcd_print(buf);
		//lcd_gotoxy(15,2);
		lcd_data('A');
		_delay_ms(4000);
		//lcd_clear();
		lcd_gotoxy(1,1);
		lcd_print("Power  = ");
		double power;
		int power_rem;
		power = read_volt() * read_current();
		power_rem = power/1;
		itoa(power,buf,10);
		lcd_print(buf);
		power_rem = (power - power_rem)*100;
		lcd_data('.');
		itoa(power_rem,buf,10);
		lcd_print(buf);
		//lcd_gotoxy(16,1);
		lcd_print("W ");
		lcd_gotoxy(1,2);
		lcd_print("Freq   = 50Hz   ");
		_delay_ms(4000);
	} 
}


