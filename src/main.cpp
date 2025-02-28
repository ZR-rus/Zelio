//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////           Программа управления  ZELIO SR3-B261BD      ///////////////////////////////////
////////////////////////                                                        //////////////////////////////////
////////////////////////                                                        //////////////////////////////////
////////////////////////                                Разработчик: ZR         //////////////////////////////////
////////////////////////                                РОССИЯ/ЕЛЕЦ 28.01.2025г.//////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////  

#include <avr/io.h>
#include <avr/interrupt.h>
//#include <stdio.h>

#define cs1 PORTD|=1<<4                                                                  // отключиться от дисплея
#define cs0 PORTD&=~(1<<4)                                                               // подключиться к дисплею
///////////////////////////////////////////////////////// дефайним состояние входов ////////////////////////////////
#define I1 PINE&(1<<6)                                                                   
#define I2 PINE&(1<<7)                                                                   
#define I3 PINA&(1<<0)                                                                   
#define I4 PINA&(1<<1)          
#define I5 PINA&(1<<2)
#define I6 PINA&(1<<3)
#define I7 PINA&(1<<4)
#define I8 PINA&(1<<5)
#define I9 PINA&(1<<6)
#define IA PINA&(1<<7)
#define IB PINF&(1<<2)
#define IC PINF&(1<<3)
#define ID PINF&(1<<4)
#define IE PINF&(1<<5)
#define IF PINF&(1<<6)
#define IG PINF&(1<<7)
#define SCL PIND&(1<<0) 
#define SDA PIND&(1<<1) 
//////////////////////////////////////////////////////// дефайним выходы ///////////////////////////////////////////
#define Q1_on PORTE|=1<<0
#define Q2_on PORTE|=1<<1
#define Q3_on PORTE|=1<<2
#define Q4_on PORTE|=1<<3
#define Q5_on PORTE|=1<<4
#define Q6_on PORTE|=1<<5
#define Q7_on PORTB|=1<<4
#define Q8_on PORTB|=1<<5
#define Q9_on PORTB|=1<<6
#define QA_on PORTB|=1<<7

#define Q1_off PORTE&=~(1<<0)
#define Q2_off PORTE&=~(1<<1)
#define Q3_off PORTE&=~(1<<2)
#define Q4_off PORTE&=~(1<<3)
#define Q5_off PORTE&=~(1<<4)
#define Q6_off PORTE&=~(1<<5)
#define Q7_off PORTB&=~(1<<4)
#define Q8_off PORTB&=~(1<<5)
#define Q9_off PORTB&=~(1<<6)
#define QA_off PORTB&=~(1<<7)

#define Q_off Q1_off;Q2_off;Q3_off;Q4_off;Q5_off;Q6_off;Q7_off;Q8_off;Q9_off;QA_off;print_string(4,1,10,"123456789A");

#define Q1 print_symbol(4,1,129);Q1_on;
#define Q2 print_symbol(4,2,130);Q2_on;
#define Q3 print_symbol(4,3,131);Q3_on;
#define Q4 print_symbol(4,4,132);Q4_on;
#define Q5 print_symbol(4,5,133);Q5_on;
#define Q6 print_symbol(4,6,134);Q6_on;
#define Q7 print_symbol(4,7,135);Q7_on;
#define Q8 print_symbol(4,8,136);Q8_on;
#define Q9 print_symbol(4,9,137);Q9_on;
#define QA print_symbol(4,10,145);QA_on; 

unsigned int DAT=0, sec_i2c, cod;
int i,x,a=16,millisecond,second,minute,hour,t,signal;
unsigned long s;

void main_millis (unsigned long m)                                                        // задержка в миллисекундах, типа дэлей
{
  TCCR0=0b00000100;                                                                       // f/64 (115200)
  OCR0=115;
  while(s<m)
  {
    if(TIFR&(1<<OCF0))
    {
       TIFR|=1<<OCF0;s=s+1;TCNT0=0;
    }
  }
  TCCR0=0;s=0;
}

ISR (TIMER1_COMPA_vect) 
{                                                                // обработка прерывания (счет времени)
  millisecond=millisecond+1;TCNT1H=0;TCNT1L=0;t=t+1;
  if(millisecond==1000)
  {
    second=second+1;
    millisecond=0;
  }
  if(second==60)
  {
    minute=minute+1;
    second=0;
  }
  if(minute==60)
  {
    hour=hour+1;
    minute=0;
  }
}
///////////////////////////////// драйвер дисплея ////////////////////////////////////////
void init_LCD()
{ 
  cs0;
  SPDR= 0b00110101;     //   Function Set: 8-bit, RE(1), off 5 line
  while (!(SPSR & (1<<SPIF)));
  SPDR= 0b00001001;    //    Extended Function Set: 5-font, no invert cursor, 4-line
  while (!(SPSR & (1<<SPIF)));
  SPDR= 0b00110000;     //   Function Set: 8-bit, RE(0)
  while (!(SPSR & (1<<SPIF)));
  SPDR= 0b00000010;     //   Return Home
  while (!(SPSR & (1<<SPIF)));
  main_millis(2);
  SPDR= 0b00001100;    //  on
  while (!(SPSR & (1<<SPIF)));
  SPDR= 0b00000001;     //   clear
  while (!(SPSR & (1<<SPIF)));
  main_millis(2);
  SPDR= 0b00110101;     //   Function Set: 8-bit, RE(1), off 5 line
  while (!(SPSR & (1<<SPIF)));
  cs1;
}

void print_symbol(unsigned int line, unsigned int position, unsigned int number)// печать символа
{
cs0;
SPDR= 0b00110000;     //   Function Set: 8-bit, RE(0)
while (!(SPSR & (1<<SPIF)));
SPDR=(127+32*(line-1))+position;    //    позиция
while (!(SPSR & (1<<SPIF)));
SPDR= 0b00110101;     //   Function Set: 8-bit, RE(1), off 5 line
while (!(SPSR & (1<<SPIF)));
SPDR= 127+1;    // 127 + сколько печатать символов
while (!(SPSR & (1<<SPIF)));
SPDR=number; // печать числа
while (!(SPSR & (1<<SPIF))); 
cs1;
}

void print_string(unsigned int line, unsigned int position, int site, char str[])// печать строки
{
cs0;
SPDR= 0b00110000;     //   Function Set: 8-bit, RE(0) 
while (!(SPSR & (1<<SPIF)));
SPDR=(127+32*(line-1))+position;    //    позиция
while (!(SPSR & (1<<SPIF)));
SPDR= 0b00110101;     //   Function Set: 8-bit, RE(1), off 5 line
while (!(SPSR & (1<<SPIF)));
SPDR= 127+site;    // 127 + сколько печатать символов
while (!(SPSR & (1<<SPIF)));
for(i=0;i<site;i++)
  {
     SPDR= str[i];     
     while (!(SPSR & (1<<SPIF)));
  }
cs1;
}
///////////////////////// счетчик времени работы контроллера //////////////////////
void time()
{
cs0;
SPDR= 0b00110000;     //   Function Set: 8-bit, RE(0), off 5 line
while (!(SPSR & (1<<SPIF)));
SPDR=0b11101011 ;    //   позиция
while (!(SPSR & (1<<SPIF)));
SPDR= 0b00110101;     //   Function Set: 8-bit, RE(1), off 5 line
while (!(SPSR & (1<<SPIF)));
SPDR= 127+5;    // 127 + сколько печатать символов  
while (!(SPSR & (1<<SPIF)));
SPDR= minute/10+48;     //   
while (!(SPSR & (1<<SPIF)));
SPDR= minute%10+48;     //   
while (!(SPSR & (1<<SPIF)));
SPDR= 58;     //   
while (!(SPSR & (1<<SPIF)));
SPDR= second/10+48;     //   
while (!(SPSR & (1<<SPIF)));
SPDR= second%10+48;     //   
while (!(SPSR & (1<<SPIF)));
cs1;
}

//////////////////////////////////// настройка I2C ///////////////////////////////////

void I2CInit(void)
{
	TWBR = 11; // Настройка частоты шины
	TWSR = (1 << TWPS1)|(1 << TWPS0); // Предделитель на 64
	TWCR |= (1 << TWEN); // Включение модуля TWI
}

//////////////////////////// функции для часов реального времени //////////////////////

void I2CStart(void)// команда СТАРТ
{
	TWCR = (1 << TWINT)|(1 << TWEN)|(1 << TWSTA); // Передача условия СТАРТ
	while(!(TWCR & (1 << TWINT))); // Ожидание установки флага TWINT
}

void I2CStop(void)// команда СТОП
{
	TWCR = (1 << TWINT)|(1 << TWEN)|(1 << TWSTO); // Передача условия СТОП
	while(TWCR & (1 << TWSTO)); // Ожидание завершения передачи условия СТОП
}

unsigned int I2CWriteByte(unsigned int data)// команда ЗАПИСЬ ДАННЫХ 
{
	TWDR = data; // Загрузка данных в TWDR
	TWCR = (1 << TWEN)|(1 << TWINT); // Сброс флага TWINT для начала передачи данных
	while(!(TWCR & (1 << TWINT))); // Ожидание установки флага TWINT
	if((TWSR & 0xF8) == 0x18 || (TWSR & 0xF8) == 0x40 || (TWSR & 0xF8) == 0x28) return 1; // OK
	else return 0; // ОШИБКА
}

unsigned int I2CReadByte(unsigned int *data,unsigned int ack)// команда ЧТЕНИЕ ДАННЫХ
{
  // Возвращаем "подтверждение" после приема
	if(ack) TWCR |= (1 << TWEA);
	else TWCR &= ~(1 << TWEA);
	// Разрешение приема данных после сброса TWINT
	TWCR |= (1 << TWINT);
	while(!(TWCR & (1 << TWINT))); // Ожидание установки флага TWINT
	if((TWSR & 0xF8) == 0x50 || (TWSR & 0xF8) == 0x58)
	{
		*data = TWDR; // Читаем данные из TWDR
		return 1; // OK
	}
	else return 0; // ОШИБКА
}

unsigned int I2CRead(unsigned int address,unsigned int *data)// чтение регистра "address" по I2C из часов реального времени в "data"
{
	unsigned int res;
	I2CStart(); // СТАРТ
	res = I2CWriteByte(0b11010000); // адрес I2C+W
	if(!res)    return 0; // ОШИБКА
	// Передача адреса необходимого регистра
	res = I2CWriteByte(address);
	if(!res)    return 0; // ОШИБКА
	I2CStart(); // Повторный СТАРТ
	res = I2CWriteByte(0b11010001); // адрес I2C+R
	if(!res)    return 0; // ОШИБКА
	// Чтение данных с "неподтверждением"
	res = I2CReadByte(data,0);
	if(!res)    return 0; // ОШИБКА
	I2CStop(); // СТОП
	return 1; // OK
}

void I2C_SECOND(unsigned char stroka, unsigned char stolbez)// вывести на экран секунды
{
  int ed_sec=0, des_sec=0, n=0x08;
  for (int m=0;m<4;m++)
  { 
    if(DAT&n)
    {
      ed_sec=ed_sec+n;
    }
    n=n/2;
  }
  n=0x40;
  for (int m=0;m<3;m++)
  { 
    if(DAT&n)
    {
      des_sec=des_sec+n;
    }
    n=n/2;
  }
  print_symbol(stroka,stolbez,ed_sec+48);// единицы 
  print_symbol(stroka,stolbez-1,des_sec/16+48);// десятки
}

void I2C_MINUTE(unsigned char stroka, unsigned char stolbez)// вывести на экран минуты
{
 int ed_min=0, des_min=0, n=0x08;
  for (int m=0;m<4;m++)
  { 
    if(DAT&n)
    {
      ed_min=ed_min+n;
    }
    n=n/2;
  }
  n=0x40;
  for (int m=0;m<3;m++)
  { 
    if(DAT&n)
    {
      des_min=des_min+n;
    }
    n=n/2;
  }
  print_symbol(stroka,stolbez,ed_min+48);// единицы
  print_symbol(stroka,stolbez-1,des_min/16+48);// десятки
}

void I2C_HOUR(unsigned char stroka, unsigned char stolbez)
{
int ed_hour=0, des_hour=0, n=0x08;
  for (int m=0;m<4;m++)
  { 
    if(DAT&n)
    {
      ed_hour=ed_hour+n;
    }
    n=n/2;
  }
  n=0x40;
  for (int m=0;m<3;m++)
  { 
    if(DAT&n)
    {
      des_hour=des_hour+n;
    }
    n=n/2;
  }
  print_symbol(stroka,stolbez,ed_hour+48);// единицы
  print_symbol(stroka,stolbez-1,des_hour/16+48);// десятки
}

void I2C_DATA(unsigned char stroka, unsigned char stolbez)
{
int ed_data=0, des_data=0, n=0x08;
  for (int m=0;m<4;m++)
  { 
    if(DAT&n)
    {
      ed_data=ed_data+n;
    }
    n=n/2;
  }
  n=0x40;
  for (int m=0;m<3;m++)
  { 
    if(DAT&n)
    {
      des_data=des_data+n;
    }
    n=n/2;
  }
  print_symbol(stroka,stolbez,ed_data+48);// единицы
  print_symbol(stroka,stolbez-1,des_data/16+48);// десятки
}

void I2C_MONTH(unsigned char stroka, unsigned char stolbez)
{
int ed_mon=0, des_mon=0, n=0x08;
  for (int m=0;m<4;m++)
  { 
    if(DAT&n)
    {
      ed_mon=ed_mon+n;
    }
    n=n/2;
  }
  n=0x40;
  for (int m=0;m<3;m++)
  { 
    if(DAT&n)
    {
      des_mon=des_mon+n;
    }
    n=n/2;
  }
  print_symbol(stroka,stolbez,ed_mon+48);// единицы
  print_symbol(stroka,stolbez-1,des_mon/16+48);// десятки
}

void I2C_GOD(unsigned char stroka, unsigned char stolbez)
{
int ed_god=0, des_god=0, n=0x08;
  for (int m=0;m<4;m++)
  { 
    if(DAT&n)
    {
      ed_god=ed_god+n;
    }
    n=n/2;
  }
  n=0x80;
  for (int m=0;m<4;m++)
  { 
    if(DAT&n)
    {
      des_god=des_god+n;
    }
    n=n/2;
  }
  print_symbol(stroka,stolbez,ed_god+48);// единицы
  print_symbol(stroka,stolbez-1,des_god/16+48);// десятки
}

void Opros_Vhodov()
{
    if((I1)!=0)
     {
       signal=1;
       print_symbol(1,1,129);
       print_symbol(4,1,129);
       Q1_on; 
     }
    if((I2)!=0)
     {
       signal=1;
       print_symbol(1,2,130);
       print_symbol(4,2,130);
       Q2_on;
     }
    if((I3)!=0)
     {
       signal=1;
       print_symbol(1,3,131);
       print_symbol(4,3,131);
       Q3_on; 
     }
    if((I4)!=0)
     {
       signal=1;
       print_symbol(1,4,132);
       print_symbol(4,4,132);
       Q4_on;
     }
    if((I5)!=0)
     {
       signal=1;
       print_symbol(1,5,133);
       print_symbol(4,5,133);
       Q5_on;
     }
    if((I6)!=0)
     {
       signal=1;
       print_symbol(1,6,134);
       print_symbol(4,6,134);
       Q6_on; 
     }
    if((I7)!=0)
     {
       signal=1;
       print_symbol(1,7,135);
       print_symbol(4,7,135);
       Q7_on; 
     }
    if((I8)!=0)
     {
       signal=1;
       print_symbol(1,8,136);
       print_symbol(4,8,136);
       Q8_on;
     }
    if((I9)!=0)
     {
       signal=1;
       print_symbol(1,9,137);
       print_symbol(4,9,137);
       Q9_on;
     }
    if((IA)!=0)
     {
       signal=1;
       print_symbol(1,10,145);
       print_symbol(4,10,145);
       QA_on; 
     }       
}

//void(*resetFUNC)(void)=0;

int main()
{ 
                                                                                         //      ***НАСТРОЙКА ПОРТОВ, СЧЕТЧИКА И SPI***                                                            
// входы на плате подтянуты к земле, НЕ ПОДТЯГИВАТЬ К "+" ПРОГРАМНО!!! Сгорит контроллер
DDRE=0b00111111;                                                                         // выходы 0,1,2,3,4,5
PORTC=0;                                                                                 // не используется   
DDRB=0b11110111;                                                                         // выходы контроллера 4,5,6,7 и SPI(0-SS,1-SCK,2-MOSI,3-MISO)
DDRD|=(1<<4)|(1<<5);                                                                     // D4- сигнал "cs" 
DDRD&=~((1<<0)|(1<<1));                                                                  // D0-SCL, D1-SDA настроить как ВХОДЫ!!!
PORTD|=(1<<4);
PORTB=0b00001111;
DDRG=0b00010000;
PORTG=0b00010111;

SPCR=0b01011110;                                                                         // настройка SPI(enable,master,отправлять сначала старшие байты!!!,скорость-f/32)
SPSR=0b00000001;                                                                         // удвоение частоты SPI()
TCCR1B=0b00000001;                                                                       // включаем 16 битный таймер с предделителем 1(7372800 тиков в 1секунду)
OCR1AH=0x1c;                                                                             // устанавливаем число равное кол-ву тиков за 1 миллисек.(старший байт) (7372)
OCR1AL=0xcc;                                                                             // устанавливаем число равное кол-ву тиков за 1 миллисек.(старший байт) (7372)
                                                                                         //       ***НАСТРОЙКА ПРЕРЫВАНИЙ***
TIMSK|=1<<OCIE1A;                                                                        // маска прерывания по сравнению Т1 
SREG|=1<<SREG_I;                                                                         // разрешение глобальных прерываний   
                                                                                         //       ***НАСТРОЙКА UART***
UBRR1H=0;                                                                                //  9600 бод - старший байт       
UBRR1L=47;                                                                               //  9600 бод - младший байт                                                  
UCSR1B=0b00011000;                                                                       //  подключить RX и TX к соответствующим ножкам
UCSR1C=0b00000110;                                                                       //  8 бит во фрейме

//ACSR|=1<<ACD;                                                                          // отключение аналогового компаратора(обязательно для освобождения ножек in0 и in1 !!!)                                                                                      //       ***НАСТРОЙКА АЦП***
//ADCSRA|=(1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);                                    // включить АЦП, задать частоту АЦП=125кГц 
//ADMUX|=(0<<REFS1)|(1<<REFS0)|(1<<ADLAR);                                               // подать внутреннее VCC на AREF, вход АЦП АС0(по умолчанию)
//ADCSRA|=(1<<ADSC);                                                                     // запуск преобразований АЦП

main_millis(100);                                                                        // пауза перед запуском
init_LCD();
print_string(1,1,16,"123456789ABCDEFG");
print_string(2,1,16,"  /  /    RUN ZR");
print_string(3,1,16,"          :  :  ");
print_string(4,1,10,"123456789A");

I2CInit();

 while(1)                                                                                // бесконечный цикл
  { 
    I2CRead(0,&sec_i2c);
    if(sec_i2c>DAT or sec_i2c==0)
    {
       I2CRead(6,&DAT);
       I2C_MONTH(2,8);// месяц
       I2CRead(5,&DAT);
       I2C_MONTH(2,5);// месяц
       I2CRead(4,&DAT);
       I2C_DATA(2,2);// дата
       I2CRead(2,&DAT);
       I2C_HOUR(3,10);// часы
       I2CRead(1,&DAT);
       I2C_MINUTE(3,13);// минуты
       I2CRead(0,&DAT);
       I2C_SECOND(3,16);// секунды    
    }
    if(x==10)x=0;
    if(signal==1)main_millis(1000);
    signal=0;
    time();
    Opros_Vhodov();
  
    ////////////////////////////////// кнопки ////////////////////////////////

    if((PING&(1<<0))==0)// третья слева
    {
      signal=1;
      cod=0;
      x=0;
      //second=0;minute=0;
    } 
    if((PING&(1<<1))==0)// вторая слева
    {
      signal=1;
      //second=second+1;
    } 
    if((PING&(1<<2))==0)// первая слева
    {
      signal=1;
      x=x+1;
      cod=x;
      //minute=minute+1;
    } 

    switch (cod)
    {
    case 0:
      Q_off;
      break;
    case 1:
      Q1;
      break;
    case 2:
      Q2;
      break;
    case 3:
      Q3;
      break;
    case 4:
      Q4;
      break;
    case 5:
      Q5;
      break;
    case 6:
      Q6;
      break;
    case 7:
      Q7;
      break;
    case 8:
      Q8;
      break;
    case 9:
      Q9;
      break;
    case 10:
      QA;
      break;
    
    default:
      break;
    }
/////////////////////////////////////////////////////////////////////////////////////
// User code...
/////////////////////////////////////////////////////////////////////////////////////
//////////////////////// возврат в исходное состояние ///////////////////////////////    
     if (signal==0)
     { 
      //print_string(1,1,16,"123456789ABCDEFG");
      //print_string(4,1,10,"123456789A");
      //Q_off;
      cod=0;
      signal=2;
     }    
  }
}
