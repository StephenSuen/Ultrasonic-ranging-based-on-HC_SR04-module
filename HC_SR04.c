#include <reg52.h>
#include <intrins.h>

#define gan_gao 29 //���Ӹ߶�

unsigned char code seg_value[] = {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x00};//0-9
unsigned char code wei_value[] = {0xfe,0xfd,0xfb,0xf7,0xef,0xdf,0xbf,0x7f};
unsigned char dspbuf[] = {10,10,10,10,10,8,8,8};
sbit duan_s = P2^0;
sbit wei_s = P2^1;
sbit tx = P2^2;
sbit rx = P2^3;
bit flag = 0;
unsigned int distance = 0;
unsigned int ren_hight = 0;//�˸߶�
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

void timer0_init()//��ʱ��0�����������ʾ
{
	TMOD |= 0x01;
	TH0 = (65536 - 2000)/256;
	TL0 = (65536 - 2000)%256;
	EA = 1;
	ET0 = 1;
	TR0 = 1;
}

void timer1_init()//��ʱ��1��������ʱ��
{
	TMOD |= 0x10;
	TH1 = 0;
	TL1 = 0;
	//ET1 = 1;//ע������ط����ж�1ǧ���ܴ򿪣�����ʱ��1����󽫽����жϷ��������У��������ǲ�û��Ϊ��д�жϴ�������Ϊ��������Ϊ��ʱ���ã����������ǽ��ж������ղ����ز��ź�
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
		if(flag == 1)//400ms���������һ��
		{
			flag = 0;
			send_wave();
// 			TR1 = 1;
// 			while((rx == 1)&&(TF1 == 0));
// 			TR1 = 0;
			
			while(rx == 0);
			TR1 = 1;
			//while(rx);//������Ϊ��������ָ����ã��������������ж�
			while((rx == 1)&&(TF1 == 0));//����ǲ���rx�ߵ�ƽʱ�䣬�����ΪTF1���������ѭ����
			//��ô��������ľ����Ѿ�����(65536us/2*340 =11.14m)11.14m,����Ϊ������ѡ�õĳ��������̴��������������
			//��������������д���ô������������ǿ������������ж�
			TR1 = 0;
			
			if(TF1)//���������жϣ����,��65536us���Խ��ܲ����ز��ź����ж�Ϊ��Ч����
			{
				TF1 = 0;
				distance = 888;
			}
			else
			{
				t = TH1;
				t <<= 8;
				t |= TL1;//it's time
				
				distance = (unsigned int)(t*0.017);//cm,�Ӹ�����ת������������340m/s
				ren_hight = (gan_gao) - distance;
				
			}
			TL1 = 0;
			TH1 = 0;
			
			dspbuf[5] = ren_hight/100;//��λ
			dspbuf[6] = ren_hight%100/10;//ʮλ
			dspbuf[7] = ren_hight%10;//��λ
		}
	}
}