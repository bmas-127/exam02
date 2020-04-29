#include "mbed.h"
#include "fsl_port.h"
#include "fsl_gpio.h"
#include <iostream>

#define UINT14_MAX        16383

// FXOS8700CQ I2C address
#define FXOS8700CQ_SLAVE_ADDR0 (0x1E<<1) // with pins SA0=0, SA1=0
#define FXOS8700CQ_SLAVE_ADDR1 (0x1D<<1) // with pins SA0=1, SA1=0
#define FXOS8700CQ_SLAVE_ADDR2 (0x1C<<1) // with pins SA0=0, SA1=1
#define FXOS8700CQ_SLAVE_ADDR3 (0x1F<<1) // with pins SA0=1, SA1=1

// FXOS8700CQ internal register addresses
#define FXOS8700Q_STATUS 0x00
#define FXOS8700Q_OUT_X_MSB 0x01
#define FXOS8700Q_OUT_Y_MSB 0x03
#define FXOS8700Q_OUT_Z_MSB 0x05
#define FXOS8700Q_M_OUT_X_MSB 0x33
#define FXOS8700Q_M_OUT_Y_MSB 0x35
#define FXOS8700Q_M_OUT_Z_MSB 0x37
#define FXOS8700Q_WHOAMI 0x0D
#define FXOS8700Q_XYZ_DATA_CFG 0x0E
#define FXOS8700Q_CTRL_REG1 0x2A
#define FXOS8700Q_M_CTRL_REG1 0x5B
#define FXOS8700Q_M_CTRL_REG2 0x5C
#define FXOS8700Q_WHOAMI_VAL 0xC7

I2C i2c( PTD9,PTD8);
Serial pc(USBTX, USBRX);

DigitalOut led(LED3);
DigitalOut led2(LED1);
InterruptIn sw2(SW2);
EventQueue queue;
Thread thread;
Timer debounce;

int m_addr = FXOS8700CQ_SLAVE_ADDR1;

void FXOS8700CQ_readRegs(int addr, uint8_t * data, int len);
void FXOS8700CQ_writeRegs(uint8_t * data, int len);
void sample();

float ref_vec[4];
float acc_data[300];

int main() {
   uint8_t *data, *res;
   uint8_t who_am_i;
   int16_t acc16;

   data = new uint8_t [2];
   res = new uint8_t [6];
   led = 1;
   led2 = 1;

   // Enable the FXOS8700Q
   FXOS8700CQ_readRegs( FXOS8700Q_CTRL_REG1, &data[1], 1);

   data[1] |= 0x01;
   data[0] = FXOS8700Q_CTRL_REG1;
   FXOS8700CQ_writeRegs(data, 2);

   // Get the slave address
   FXOS8700CQ_readRegs(FXOS8700Q_WHOAMI, &who_am_i, 1);

   // filter
   for(int i = 0; i < 10; i ++ )
      FXOS8700CQ_readRegs(FXOS8700Q_OUT_X_MSB, res, 6);

   // thread init and interupt signal control
   debounce.start();
   thread.start(callback(&queue, &EventQueue::dispatch_forever));
   sw2.rise(queue.event(sample));

//   pc.printf("Here is %x\r\n", who_am_i);

   while(1);

}

void sample(){
   uint8_t res[6];
   int16_t acc16;
   double delta = 0;

   if(debounce.read_ms() > 1000){
      debounce.reset();
   }else{
      return;
   }
   
   for(int i = 0; i < 100; i ++ ){
      led = !led;
      FXOS8700CQ_readRegs(FXOS8700Q_OUT_X_MSB, res, 6);

      acc16 = (res[0] << 6) | (res[1] >> 2);
      if (acc16 > UINT14_MAX/2)
         acc16 -= UINT14_MAX;
      acc_data[i * 3] = ((float)acc16) / 4096.0f;

      acc16 = (res[2] << 6) | (res[3] >> 2);
      if (acc16 > UINT14_MAX/2)
         acc16 -= UINT14_MAX;
      acc_data[i * 3 + 1] = ((float)acc16) / 4096.0f;

      acc16 = (res[4] << 6) | (res[5] >> 2);
      if (acc16 > UINT14_MAX/2)
         acc16 -= UINT14_MAX;
      acc_data[i * 3 + 2] = ((float)acc16) / 4096.0f - 1;

      wait(0.1);
   }

   for(int i = 0; i < 100; i ++){
      int signal;
      pc.printf("%1.4f\r\n%1.4f\r\n%1.4f\r\n", acc_data[i * 3], acc_data[i * 3 + 1], acc_data[i * 3 + 2]);
      delta += 0.5 * acc_data[i * 3 + 2] * 9.8 * 0.01;
      signal = (delta >= 0.05 || delta <= -0.05) ? 1 : 0;
      pc.printf("%d\n", signal);
   }
}


void FXOS8700CQ_readRegs(int addr, uint8_t * data, int len) {
   char t = addr;
   i2c.write(m_addr, &t, 1, true);
   i2c.read(m_addr, (char *)data, len);
}


void FXOS8700CQ_writeRegs(uint8_t * data, int len) {
   i2c.write(m_addr, (char *)data, len);
}
