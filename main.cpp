#include "mbed.h"

BufferedSerial uart0(P1_7, P1_6,115200);  //TX, RX
SPI spi(P0_9, P0_8, P0_6);    //mosi, miso, sclk
I2C i2c(P0_5,P0_4); //SDA,SCL P0_5,P0_4
DigitalOut LE1(P1_4);
DigitalOut LE2(P1_1);
AnalogIn ain0(P0_11);
AnalogIn ain1(P1_0);

//cs control func.
void le_hi(uint8_t num);    //dds
void le_lo(uint8_t num);

//uart read buf and vals
const uint8_t buf_size=15;
void buf_read(uint8_t num); //uart read func.
char read_buf[buf_size];    //uart read buf
void buf2val();             //buf to vals change func. return to 'freq' and 'pha' global var 
uint32_t freq;              //Hz
uint16_t pha,ampl;          //deg. loaded mV

//DDS control
#define res_inv 4           //res=67108864/2^28
uint8_t i;
void waveset(uint8_t ch, uint32_t freq, uint16_t pha, uint16_t ampl);    //waveset func.

//DAC control
#define dac_fs 3300     //DAC full scale Vout
#define dac_res 4096    //dac resolution 2^12
#define g 7             //driver amp gain
const uint8_t dac_addr=0xc0;

//log det
uint16_t a1,a2;
uint32_t a1_all,a2_all;
uint8_t cnt;
const uint8_t avg=10;
void val_send(uint8_t digit, uint16_t val);

int main(){
    spi.format(16,2);   //spi mode setting. 2byte transfer, mode 2
    for(i=1;i<=2;++i) le_hi(i); //LE init
    while(true) {
        for(i=1;i<=2;++i){
            buf_read(buf_size);//uart buf read
            buf2val();
            waveset(i,freq,pha,ampl);
        }

        for(i=1;i<=2;++i) le_lo(i);
        spi.write(0x2000);      //accum. reset
        for(i=1;i<=4;++i) le_hi(i);

        thread_sleep_for(10);
        
        for(cnt=0;cnt<avg;++cnt){
            a1=ain0.read_u16()*3300/65535;  //mV unit
            a2=ain1.read_u16()*3300/65535;  //mV unit
            a1_all=a1_all+a1;
            a2_all=a2_all+a2;
        }
        
        a1=a1_all/avg;
        a2=a2_all/avg;
        a1_all=0;
        a2_all=0;
        val_send(4,a1);
        val_send(4,a2);
    }
}

//uart char number read func.
void buf_read(uint8_t num){
    char local_buf[1];
    uint8_t i;
    for (i=0;i<num;++i){
        uart0.read(local_buf,1);
        read_buf[i]=local_buf[0];
    }
}

//buf to val change func.
void buf2val(){
    uint8_t i,j;
    uint32_t pow10;
    freq=0;
    pha=0;
    ampl=0;
    for(i=0;i<8;++i){
        pow10=1;
        for(j=0;j<7-i;++j){
            pow10=10*pow10;
        }
        freq=freq+(read_buf[i]-48)*pow10;
    }
    for(i=0;i<3;++i){
        pow10=1;
        for(j=0;j<2-i;++j){
            pow10=10*pow10;
        }
        pha=pha+(read_buf[i+8]-48)*pow10;
    }
    for(i=0;i<4;++i){
        pow10=1;
        for(j=0;j<3-i;++j){
            pow10=10*pow10;
        }
        ampl=ampl+(read_buf[i+11]-48)*pow10;
    }
}

//uart send func.
void val_send(uint8_t digit, uint16_t val){
    char local_buf[1];
    char data[4];
    uint8_t i;
    data[3]=0x30+val%10;        //1
    data[2]=0x30+(val/10)%10;   //10
    data[1]=0x30+(val/100)%10;  //100
    data[0]=0x30+(val/1000)%10; //1000
    for(i=0;i<digit;++i){
        local_buf[0]=data[i+4-digit];
        uart0.write(local_buf,1);
    }
}

//le control func.
void le_hi(uint8_t num){
    if(num==1) LE1=1;
    else if(num==2) LE2=1;
}
void le_lo(uint8_t num){
    if(num==1) LE1=0;
    else if(num==2) LE2=0;
}

//wave set func.
void waveset(uint8_t ch, uint32_t freq, uint16_t pha, uint16_t ampl){
    uint16_t buf;
    char set[2];
    if(freq>30000000)freq=30000000;
    if(pha>360)pha=360;
    if(ampl>2100)ampl=2100;

    le_lo(ch);
    spi.write(0x2100);
    buf=((res_inv*freq)&0x3FFF)+0x4000;
    spi.write(buf);
    buf=((res_inv*freq)>>14)+0x4000;
    spi.write(buf);
    buf=(4096*pha/360)+0xC000;
    spi.write(buf);
    le_hi(ch);

    buf=((1200-4*ampl/g)*dac_res/dac_fs);    //(1/res)*(1200/3)*(3-ampl*2/(200*g))
    set[0]=buf>>8;
    set[1]=buf&0xff;
    if(i==1){
        i2c.write(dac_addr,set,2);  //ch1 dac
    }else{
        i2c.write(dac_addr+0x2,set,2);  //ch2 dac
    }
}