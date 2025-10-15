#include <avr/io.h>
#include <stdio.h>
#include <stdint.h>
#include "UART.h"

#define F_CPU 16000000UL

typedef struct{
	uint8_t ucsra;
	uint8_t ucsrb;
	uint8_t ucsrc;
	uint8_t reserved;
	uint16_t ubrr;
	uint8_t udr;
}UART_reg_t;

static volatile UART_reg_t* const uartAdrr[] = {
	(volatile UART_reg_t*)&UCSR0A, 
	(volatile UART_reg_t*)&UCSR1A,
	(volatile UART_reg_t*)&UCSR2A,
	(volatile UART_reg_t*)&UCSR3A
};

// Prototypes
// Initialization
void UART_Ini(uint8_t com, uint32_t baudrate, uint8_t size, uint8_t parity, uint8_t stop)
{
	uint8_t temp;
	volatile UART_reg_t *uart = uartAdrr[com];
	
	uart->ubrr = (F_CPU/(16*baudrate)) - 1;
	temp = (F_CPU/(8*baudrate)) - 1;
	
	if( ((F_CPU/(16*(((uart->ubrr/baudrate) - 1)+1)))-1) > ((F_CPU/(8*(((temp/baudrate) - 1)+1)))-1))
		uart->ubrr=temp;
	
	uart->ucsra &= ~(1 << U2X2);

	uart->ucsrb = (1 << TXEN0) | (1 << RXEN0);
	if(parity>0)
		parity++;
	uart->ucsrc = (parity << UPM00) | ((size - 5)<< UCSZ00) | ((stop - 1)<< USBS0); // Partiy=even size=5 stopbit=1
}

// Send
void UART_puts(uint8_t com, char *str)
{
	uint32_t idx=0;
	uint8_t b=0;
	while(b==0)
	{
		if(str[idx]=='\0'){
			b++;
			break;
		}
		UART_putchar(com,str[idx++]);
	}
}
void UART_putchar(uint8_t com, char data)
{
	volatile UART_reg_t *uart = uartAdrr[com];
	while (!(uart->ucsra & (1<<UDRE2)))
		;
	uart->udr = data;
}
// Received
uint8_t UART_available(uint8_t com)
{
	volatile UART_reg_t *uart = uartAdrr[com];
	return (uart->ucsra & (1<<RXC2));
}
char UART_getchar(uint8_t com )
{
	volatile UART_reg_t *uart = uartAdrr[com];
	while (!UART_available(com))
		;
	return uart->udr;
}
/*
void UART_gets(uint8_t com, char *str)
{
	uint32_t idx=0;
	uint8_t b=0;
	while(b==0)
	{
		str[idx] = UART_getchar(com);
		if(str[idx]=='\r' || str[idx]=='\n'){
			b++;
			str[idx] = '\0';
			UART_putchar(com,'\n');
			UART_putchar(com,'\r');
		}
		else
			UART_putchar(com,str[idx++]);
	}
}
*/

void UART_gets(uint8_t com, char *str)
{
    uint16_t idx = 0;
    char c;
    while (1)
    {
        c = UART_getchar(com); // Wait for next character
        if (c == '\r' || c == '\n') // Enter key pressed
        {
            UART_putchar(com, '\r');
            UART_putchar(com, '\n');
            break;
        }
        else if (c == '\b' && idx > 0) // Handle backspace
        {
            idx--;
            UART_putchar(com, '\b');
            UART_putchar(com, ' ');
            UART_putchar(com, '\b');
        }
        else if (idx < 19)
        {
            str[idx++] = c; // Store character
            UART_putchar(com, c); // Echo
        }
    }
    str[idx] = '\0'; // Null-terminate
}




// Escape sequences
void UART_clrscr( uint8_t com )
{
	UART_puts(com, "\033[2J");
	UART_puts(com, "\033[H");
}
void UART_setColor(uint8_t com, uint8_t color)
{
	char cmd[10];
	sprintf(cmd, "\033[%dm", 30 + color);
	UART_puts(com, cmd);
}
void UART_gotoxy(uint8_t com, uint8_t x, uint8_t y)
{
	char cmd[10];
	sprintf(cmd, "\033[%d;%dH", y, x);
	UART_puts(com, cmd);
}

// Utils
void itoa(uint16_t number, char* str, uint8_t base)
{
    char digits[] = "0123456789ABCDEF";
    char buffer[17];
    uint32_t i = 0;

    if (number == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }

    while (number > 0) {
        uint8_t remainder = number % base;
        buffer[i++] = digits[remainder];
        number /= base;
    }

    uint32_t j = 0;
    while (i > 0) {
        str[j++] = buffer[--i];
    }
    str[j] = '\0';
}
uint16_t atoi(char *str)
{
    uint16_t result = 0;

    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }

    return result;
}
