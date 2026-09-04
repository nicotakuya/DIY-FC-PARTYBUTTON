// DIY FC_PARTYBUTTON  by takuya matsubara
// for Atmega168/328  5V/16MHz(Arduino Pro/Pro Mini)

// BUTTON
#define BUTTON_PORT PORTB     // port
#define BUTTON_PIN  PINB      // pin
#define BUTTON_DDR  DDRB      // direction
#define BUTTON_BITPLAYER1  (1<<0) // player1
#define BUTTON_BITPLAYER2  (1<<1) // player2
#define BUTTON_BITPLAYER3  (1<<2) // player3
#define BUTTON_BITPLAYER4  (1<<3) // player4
#define BUTTON_BITPLAYER5  (1<<4) // player5
#define BUTTON_BITPLAYER6  (1<<5) // player6

// FC
#define FC_PORT PORTD       // Data/Clock port
#define FC_PIN  PIND        // Data/Clock pin
#define FC_DDR  DDRD        // Data/Clock direction

#define FC_BITD2B2 (1<<2)   // P2.bit2 data mask
#define FC_BITD2B3 (1<<3)   // P2.bit3 data mask
#define FC_BITD2B4 (1<<4)   // P2.bit4 data mask

#define FC_BITLATCH (1<<7)   // LATCH(OUT0) mask
#define FC_BITCLK2  (1<<6)   // P2 clock mask

#define UNTIL_FCCLK2_H while((FC_PIN&FC_BITCLK2)==0)
#define UNTIL_FCCLK2_L while((FC_PIN&FC_BITCLK2)!=0)
#define FC_D2B2_L  FC_PORT&=~FC_BITD2B2
#define FC_D2B2_H  FC_PORT|=FC_BITD2B2
#define FC_D2B3_L  FC_PORT&=~FC_BITD2B3
#define FC_D2B3_H  FC_PORT|=FC_BITD2B3
#define FC_D2B4_L  FC_PORT&=~FC_BITD2B4
#define FC_D2B4_H  FC_PORT|=FC_BITD2B4
#define UNTIL_LATCH_H while((FC_PIN&FC_BITLATCH)==0)
#define UNTIL_LATCH_L while((FC_PIN&FC_BITLATCH)!=0)

#define CPUHZ 16000000  // CPU frequency[Hz]
#define PRS1  1         // timer1 prescaler
#define T1HZ  (CPUHZ/PRS1)  // timer1 freq.[Hz]
#define TIMER_10USEC   (unsigned int)(0x10000-(T1HZ/100000))
#define TIMER_1MSEC    (unsigned int)(0x10000-(T1HZ/1000))

// timer initialize
void timer_init(void)
{
  // timer1 prescaler
  TCCR1A = 0;
  TCCR1B = 1;
  // 0: No clock source (Timer/Counter stopped).
  // 1: clock /1 (No prescaling)
  // 2: clock /8 (From prescaler)
  // 3: clock /64 (From prescaler)
  // 4: clock /256 (From prescaler)
  // 5: clock /1024 (From prescaler)
}

//----- wait micro second
void timer_uswait(unsigned int limitcnt)
{
  TCNT1 = limitcnt;
  TIFR1 |= (1 << TOV1);  // clear TOV1
  while(!(TIFR1 & (1 << TOV1)));  // TIFR1が1になるまで待つ
}

//----デバッグ用 
void button_test(void)
{
  unsigned char btn;

  while(1){
    btn = ~BUTTON_PIN;  // ボタン読み込み
    if(btn & BUTTON_BITPLAYER1){
      Serial.print("p1\n");
      delay(100);
    }
    if(btn & BUTTON_BITPLAYER2){
      Serial.print("p2\n");
      delay(100);
    }
    if(btn & BUTTON_BITPLAYER3){
      Serial.print("p3\n");
      delay(100);
    }
    if(btn & BUTTON_BITPLAYER4){
      Serial.print("p4\n");
      delay(100);
    }
    if(btn & BUTTON_BITPLAYER5){
      Serial.print("p5\n");
      delay(100);
    } 
    if(btn & BUTTON_BITPLAYER6){
      Serial.print("p6\n");
      delay(100);
    } 
    delay(16);
  }
}

//----デバッグ用 パルスカウント
void pulse_test(void){
#define TARGET FC_BITLATCH
  char edge = 1;
  unsigned char pulsecnt=0;
  unsigned int tempcnt=0;

  cli(); //割り込み禁止
  TCNT1 = TIMER_1MSEC;  // timer reset
  TIFR1 |= (1 << TOV1);  // clear TOV1

  while(1){
    if(edge==0){
      if(FC_PIN & TARGET){ // 立ち上がり検出
        edge=1;
        pulsecnt++;
      }
    }else{
      if((FC_PIN & TARGET)==0){ // 立ち下がり検出
        edge=0;
      }
    }
    if(TIFR1 & (1 << TOV1)){  // 1mSec経過
      TCNT1 = TIMER_1MSEC;  // timer reset
      TIFR1 |= (1 << TOV1);  // clear TOV1
      tempcnt++;
      if(tempcnt >= 1000){ // 1Sec経過

        sei(); // 割り込み許可
        Serial.print(pulsecnt);
        Serial.print(" cnt\n");
        Serial.flush();
        cli(); // 割り込み禁止

        tempcnt = 0;
        pulsecnt=0;
      }
    }
  }
}

//----
void pad_control(void)
{
  unsigned char btn;
  char loopcnt;

  cli(); //割り込み禁止

  while(1){
    btn = ~BUTTON_PIN;  // ボタン読み込み
    
    UNTIL_LATCH_H;  // ラッチ=Hを待つ
    UNTIL_LATCH_L;  // ラッチ=Lを待つ

    if(btn & BUTTON_BITPLAYER1){
      FC_D2B2_L; // ON
    }else{
      FC_D2B2_H; // OFF 
    }
    if(btn & BUTTON_BITPLAYER2){
      FC_D2B3_L; // ON
    }else{
      FC_D2B3_H; // OFF
    }
    if(btn & BUTTON_BITPLAYER3){
      FC_D2B4_L; // ON  
    }else{
      FC_D2B4_H; // OFF
    }
    UNTIL_FCCLK2_L; // CLK2=Lowを待つ
    UNTIL_FCCLK2_H; // CLK2=Highを待つ

    if(btn & BUTTON_BITPLAYER4){
      FC_D2B2_L; // ON
    }else{
      FC_D2B2_H; // OFF
    }
    if(btn & BUTTON_BITPLAYER5){
      FC_D2B3_L; // ON
    }else{
      FC_D2B3_H; // OFF
    }
    if(btn & BUTTON_BITPLAYER6){
      FC_D2B4_L; // ON
    }else{
      FC_D2B4_H; // OFF
    }
    UNTIL_FCCLK2_L; // CLK2=Lowを待つ
    UNTIL_FCCLK2_H; // CLK2=Highを待つ

    // ID送信
    FC_D2B2_L;
    FC_D2B3_H;
    FC_D2B4_L;

    UNTIL_FCCLK2_L; // CLK2=Lowを待つ
    UNTIL_FCCLK2_H; // CLK2=Highを待つ

    FC_D2B2_H;
    FC_D2B3_H;
    FC_D2B4_H;

    loopcnt = 5;
    while(loopcnt--){
      UNTIL_FCCLK2_L; // CLK2=Lowを待つ
      UNTIL_FCCLK2_H; // CLK2=Highを待つ
    }
  } 
}

//----
void setup() {
  BUTTON_DDR &= ~(BUTTON_BITPLAYER1 + BUTTON_BITPLAYER2 + BUTTON_BITPLAYER3
   + BUTTON_BITPLAYER4 + BUTTON_BITPLAYER5 + BUTTON_BITPLAYER6);

  // PULL UP
  BUTTON_PORT |= (BUTTON_BITPLAYER1 + BUTTON_BITPLAYER2 + BUTTON_BITPLAYER3
   + BUTTON_BITPLAYER4 + BUTTON_BITPLAYER5 + BUTTON_BITPLAYER6);

  FC_DDR |= (FC_BITD2B2 + FC_BITD2B3 + FC_BITD2B4);

  FC_DDR &= ~(FC_BITCLK2 + FC_BITLATCH);
  FC_PORT |= (FC_BITCLK2 + FC_BITLATCH + FC_BITD2B2 + FC_BITD2B3 + FC_BITD2B4);

  // デバッグ用
  Serial.begin(115200);
  while (!Serial) {
  }
  delay(50);
  Serial.print("start\n");
  Serial.flush();
}

//----
void loop() {
//  button_test(); // デバッグ用　ボタンのテスト
//  pulse_test();  // デバッグ用　パルスのカウント

  pad_control();
}
