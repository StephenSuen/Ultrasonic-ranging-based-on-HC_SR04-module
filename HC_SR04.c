#include <reg52.h>
#include <intrins.h>

#define gan_gao 29 //杆子高度

unsigned char code seg_value[] = {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x00};//0-9
unsigned char code wei_value[] = {0xfe,0xfd,0xfb,0xf7,0xef,0xdf,0xbf,0x7f};
unsigned char dspbuf[] = {10,10,10,10,10,8,8,8};
sbit duan_s = P2^0;
sbit wei_s = P2^1;
sbit tx = P2^2;
sbit rx = P2^3;
bit flag = 0;
unsigned int distance = 0;
unsigned int ren_hight = 0;//人高度
void display()
{
	static unsigned char i = 0;
	duan_s = 1;
	P0 = 0;
	duan_s = 0;
	
	duan_s = 1;
	P0 = seg_value[dspbuf[i]];
	duan_s = 0;
	
	wei_s = 1;
	P0 = wei_value[i];
	wei_s = 0;
	
	i++;
	if(i == 8)
		i = 0;
}

void send_wave()
{
	 tx=1;			                 
	  _nop_(); 
	  _nop_(); 
	  _nop_(); 
	  _nop_(); 
	  _nop_(); 
	  _nop_(); 
	  _nop_(); 
	  _nop_(); 
	  _nop_(); 
	  _nop_(); 
	  _nop_(); 
	  _nop_(); 
	  _nop_(); 
	  _nop_(); 
	  _nop_(); 
	  _nop_(); 
	  _nop_();
	  _nop_(); 
	  _nop_(); 
	  _nop_(); 
	  _nop_();
	  tx=0;
}

void timer0_init()//定时器0用来数码管显示
{
	TMOD |= 0x01;
	TH0 = (65536 - 2000)/256;
	TL0 = (65536 - 2000)%256;
	EA = 1;
	ET0 = 1;
	TR0 = 1;
}

void timer1_init()//定时器1用来计算时间
{
	TMOD |= 0x10;
	TH1 = 0;
	TL1 = 0;
	//ET1 = 1;//注意这个地方的中断1千万不能打开，否则定时器1溢出后将进入中断服务处理函数中，但是我们并没有为他写中断处理函数因为他仅仅作为计时作用，如果溢出我们将判断他接收不到回波信号
}
void timer0_isr() interrupt 1
{
	static unsigned char i = 0;
	TH0 = (65536 - 2000)/256;
	TL0 = (65536 - 2000)%256;
	display();
	
	i++;
	if(i == 50)
			flag = 1;
}

void main()
{
	unsigned int t;
	timer1_init();
	timer0_init();
	while(1)
	{
		if(flag == 1)//400ms超声波检测一次
		{
			flag = 0;
			send_wave();
// 			TR1 = 1;
// 			while((rx == 1)&&(TF1 == 0));
// 			TR1 = 0;
			
			while(rx == 0);
			TR1 = 1;
			//while(rx);//将这句变为下面这条指令更好，可做超出量程判断
			while((rx == 1)&&(TF1 == 0));//这句是测量rx高电平时间，如果因为TF1溢出而跳出循环，
			//那么代表他测的距离已经超过(65536us/2*340 =11.14m)11.14m,又因为我们所选用的超声波量程大多在四五米左右
			//，所以我们这样写，好处在于下面我们可做超出量程判断
			TR1 = 0;
			
			if(TF1)//超出量程判断，溢出,在65536us内仍接受不到回波信号则判断为无效距离
			{
				TF1 = 0;
				distance = 888;
			}
			else
			{
				t = TH1;
				t <<= 8;
				t |= TL1;//it's time
				
				distance = (unsigned int)(t*0.017);//cm,从浮点数转换成正整数，340m/s
				ren_hight = (gan_gao) - distance;
				
			}
			TL1 = 0;
			TH1 = 0;
			
			dspbuf[5] = ren_hight/100;//百位
			dspbuf[6] = ren_hight%100/10;//十位
			dspbuf[7] = ren_hight%10;//个位
		}
	}
}