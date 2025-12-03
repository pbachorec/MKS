/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */


#include "lwip/opt.h"


#if LWIP_NETCONN

#include "lwip/sys.h"
#include "lwip/api.h"

#include "main.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <strings.h>

#define TELNET_THREAD_PRIO  ( tskIDLE_PRIORITY + 4 )
#define CMD_BUFFER_LEN 256


static void telnet_byte_available(uint8_t c, struct netconn *conn);
static void telnet_process_command(char *cmd, struct netconn *conn);

/*-----------------------------------------------------------------------------------*/
static void telnet_thread(void *arg)
{
  struct netconn *conn, *newconn;
  err_t err, accept_err;
  struct netbuf *buf;
  uint8_t *data;
  u16_t len;
      
  LWIP_UNUSED_ARG(arg);

  /* Create a new connection identifier. */
  conn = netconn_new(NETCONN_TCP);
  
  if (conn!=NULL)
  {  
    /* Bind connection to well known port number 7. */
    err = netconn_bind(conn, NULL, 23);
    
    if (err == ERR_OK)
    {
      /* Tell connection to go into listening mode. */
      netconn_listen(conn);
    
      while (1) 
      {
        /* Grab new connection. */
         accept_err = netconn_accept(conn, &newconn);
    
        /* Process the new connection. */
        if (accept_err == ERR_OK) 
        {

          while (netconn_recv(newconn, &buf) == ERR_OK) 
          {
        	  do
        	  {
        		  netbuf_data(buf, (void**)&data, &len);
        		  while (len--) telnet_byte_available(*data++, newconn);
        	  }
            while (netbuf_next(buf) >= 0);
          
            netbuf_delete(buf);
          }
        
          /* Close connection and discard connection identifier. */
          netconn_close(newconn);
          netconn_delete(newconn);
        }
      }
    }
    else
    {
      netconn_delete(newconn);
    }
  }
}
/*-----------------------------------------------------------------------------------*/

void telnet_init(void)
{
  sys_thread_new("telnet_thread", telnet_thread, NULL, DEFAULT_THREAD_STACKSIZE, TELNET_THREAD_PRIO);
}
/*-----------------------------------------------------------------------------------*/

static void telnet_byte_available(uint8_t c, struct netconn *conn)
{
	static uint16_t cnt;
	static char data[CMD_BUFFER_LEN];

	if (cnt < CMD_BUFFER_LEN && c >= 32 && c <= 127) data[cnt++] = c;
	if (c == '\n' || c == '\r') {
		data[cnt] = '\0';
		telnet_process_command(data, conn);
		cnt = 0;
	}
}

static void telnet_process_command(char *cmd, struct netconn *conn)
{
    char *token;
    char *saveptr;
    char out[128];

    token = strtok_r(cmd, " ", &saveptr);
    if (token == NULL) return;

    // HELLO
    if (strcasecmp(token, "HELLO") == 0) {
        sprintf(out, "Communication OK\r\n");
        netconn_write(conn, out, strlen(out), NETCONN_COPY);
    }

    // LED1 / LED2 / LED3 ON|OFF
    else if (strcasecmp(token, "LED1") == 0 ||
             strcasecmp(token, "LED2") == 0 ||
             strcasecmp(token, "LED3") == 0)
    {
        GPIO_TypeDef *port = NULL;
        uint16_t pin = 0;

        if (strcasecmp(token, "LED1") == 0) {
            port = LED1_GPIO_Port;
            pin  = LED1_Pin;
        } else if (strcasecmp(token, "LED2") == 0) {
            port = LED2_GPIO_Port;
            pin  = LED2_Pin;
        } else if (strcasecmp(token, "LED3") == 0) {
            port = LED3_GPIO_Port;
            pin  = LED3_Pin;
        }

        token = strtok_r(NULL, " ", &saveptr);
        if (token && port) {
            if (strcasecmp(token, "ON") == 0) {
                HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
                sprintf(out, "OK\r\n");
            } else if (strcasecmp(token, "OFF") == 0) {
                HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
                sprintf(out, "OK\r\n");
            } else {
                sprintf(out, "ERR Unknown param\r\n");
            }
        } else {
            sprintf(out, "ERR Missing param\r\n");
        }

        netconn_write(conn, out, strlen(out), NETCONN_COPY);
    }

    // STATUS
    else if (strcasecmp(token, "STATUS") == 0) {
        sprintf(out, "LED1=%s LED2=%s LED3=%s\r\n",
                HAL_GPIO_ReadPin(LED1_GPIO_Port, LED1_Pin) ? "ON" : "OFF",
                HAL_GPIO_ReadPin(LED2_GPIO_Port, LED2_Pin) ? "ON" : "OFF",
                HAL_GPIO_ReadPin(LED3_GPIO_Port, LED3_Pin) ? "ON" : "OFF");
        netconn_write(conn, out, strlen(out), NETCONN_COPY);
    }


}


#endif /* LWIP_NETCONN */
