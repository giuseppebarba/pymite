#ifndef USART_H
#define USART_H 1
// 
// STM32F4xx USARTバッファ制御プログラム
// 参考資料：
//	RM0090 Reference manual - STM32F4xx advanced ARM-based 32-bit MCUs
//	PM0056 Programming manual - STM32Fxxx Cortex-M3 programming manual
//	UM1472 Users manual - STM32F4DISCOVERY discovery board
// 
#include "stm32f4xx.h"
#include "core_cm4.h"
#include "sysclk_config.h"
#include <stdio.h>

struct USART_BUF_CTRL_t {
	uint8_t err_flag;
	uint8_t not_done;
	char tx_data;
	char rx_data;
	uint32_t rb_r, rb_w;
	uint32_t tb_r, tb_w;
	uint32_t rb_siz;
	uint32_t tb_siz;
	char rb[BUFFER_SIZE];
	char tb[BUFFER_SIZE];
} sci;

#define	UEI	NVIC_EnableIRQ(INT_pos)
#define	UDI	NVIC_DisableIRQ(INT_pos)

#ifndef	ENTER_hndl
#define	ENTER_hndl
#endif
#ifndef	EXIT_hndl
#define	EXIT_hndl
#endif

#define	clear_sr    	{SCI->SR= (USART_SR_TC | USART_SR_TXE);}

#define	cr_set_UE   	{SCI->CR1|= (uint16_t)USART_CR1_UE;} 	// USARTを許可する
#define	cr_clear_RE 	{SCI->CR1&= (uint16_t)~USART_CR1_RE;}	// 受信を禁止する
#define	cr_set_RE   	{SCI->CR1|= (uint16_t)USART_CR1_RE;} 	// 受信を許可する
#define	cr_clear_TE 	{SCI->CR1&= (uint16_t)~USART_CR1_TE;}	// 送信を禁止する
#define	cr_set_TE   	{SCI->CR1|= (uint16_t)USART_CR1_TE;} 	// 送信を許可する

#define	cr_clear_RIE	{SCI->CR1&= (uint16_t)~USART_CR1_RXNEIE;}	// RDE割込みを禁止する
#define	cr_set_RIE  	{SCI->CR1|= (uint16_t)USART_CR1_RXNEIE;}	// RDE割込みを許可する
#define	cr_clear_ERI	{SCI->CR1&= (uint16_t)~USART_CR1_PEIE;SCI->CR3&= (uint16_t)~USART_CR3_EIE;}	// ERI割込みを禁止する
#define	cr_set_ERI  	{SCI->CR1|= (uint16_t)USART_CR1_PEIE; SCI->CR3|= (uint16_t)USART_CR3_EIE;}	// ERI割込みを許可する
#define	cr_clear_TIE	{SCI->CR1&= (uint16_t)~USART_CR1_TXEIE;}	// TDRE割込みを禁止する
#define	cr_set_TIE  	{SCI->CR1|= (uint16_t)USART_CR1_TXEIE;}	// TDRE割込みを許可する
#define	cr_clear_TEIE	{SCI->CR1&= (uint16_t)~USART_CR1_TCIE;}	// TEND割込みを禁止する
#define	cr_set_TEIE 	{SCI->CR1|= (uint16_t)USART_CR1_TCIE;}	// TEND割込みを許可する
#define	is_sr_ERRF  	(SCI->SR & (USART_SR_PE | USART_SR_FE | USART_SR_NE | USART_SR_ORE))
#define	is_sr_RDRF  	(SCI->SR & USART_SR_RXNE)
#define	is_sr_TDRE  	(SCI->SR & USART_SR_TXE)
#define	is_sr_TEND  	(SCI->SR & USART_SR_TC)
#define	BUFFER_FULL 	(0xffffffff)
#define	RX_FLAG_OVFL	(0x80)

static void sci_RDR2RxBuf(void);
static void sci_TxBuf2TDR(void);
static int16_t sci_getRxBuf(void);
static int16_t sci_putTxBuf(char c);

static void sci_init(void);
static void sci_init_RxBuf(void);
static void sci_init_TxBuf(void);

// ****************************************************************************
// 高レベルインターフェイス処理ルーチン群
// ****************************************************************************
// SCIの転送レート設定および初期化を行い，送受信を開始する。
void SCI_init(int brr){
	SCI->GTPR= 0;
	SCI->BRR= (APB1Clock)/brr;
	sci_init();	// buffer clean and set up
	cr_set_UE;
	UEI;
}

//
// SCIに送信文字を1文字直接送る(0で送出成功，-1が返ると送出レジスタに空きがなく失敗)
// return 0:seccess set TDR -1:TDR is busy
//
int16_t SCI_putc_direct(char c){// SCIに文字を送信(バッファリングを無視して割込む外道方式)
	if(0==(is_sr_TDRE))return(-1);
	SCI->DR= c;
	return(0);
}

//
// SCIに送信文字列を直接送る(全て送り込みが成功するまで処理を返さない)
//
void SCI_puts_direct(char *s){
	for(;*s;){
		while(SCI_putc_direct(*s));	// 送信できるまで再試行する
		s++;
	};
}

//
// SCIの受信バッファの滞留バイト数を返す。
//
uint32_t SCI_nbuf(void){
	if(sci.rb_w == sci.rb_r) return(0);
	if(BUFFER_FULL == sci.rb_w) return(sci.rb_siz);
	if(sci.rb_w <  sci.rb_r){
		return(sci.rb_siz - sci.rb_r + sci.rb_w);
	}else{
		return(sci.rb_w - sci.rb_r);
	};
}

//
// SCIの受信文字を1文字受取る
// 取得したデータの値を返す。文字を受け取るまで処理を返さない。
//
int16_t SCI_getc(void){
int16_t c;
	do c=sci_getRxBuf(); while(c < 0);
	return(c);
}

//
// SCIに送信文字を1文字送る
// 書き込み成功まで再試行する。
//
void SCI_putc(char c){
	while(sci_putTxBuf(c)); // 0以外が戻るとエラーなので再度書き込み
}

//
// SCIに送信文字列を送る
// 書き込み文字数が返る。送信バッファへ全て送り込みが成功するまで処理を返さない。
//
uint32_t SCI_puts(char *s){
uint32_t r;
	for(r=0; *s; s++,r++){
		SCI_putc(*s);
	};
	return(r);	// 書き込んだ文字数をリターンする。
}

//
// SCIに送信文字を1文字送る
// 書き込み文字数が返るので0が返る場合は送り出しに失敗している
//
uint32_t SCI_write(char c){
	return(0 == sci_putTxBuf(c));
	// 0以外が戻るとエラーなので書き込み失敗で0をリターン
	// 正常に書き込めたら0が戻るので書き込み文字数の1をリターン
}

//
// SCIに送信文字列を送る
// 書き込み文字数が返るので文字数をチェックして文字列の長さに足りない場合は送り出しに失敗している
//
uint32_t SCI_writes(char *s){
uint32_t r;
	for(r=0; *s && SCI_write(*s); s++,r++);
	return(r);	// 書き込んだ文字数をリターンする。
}


// ****************************************************************************
// 送受信バッファ初期設定用処理ルーチン
// ****************************************************************************
static void sci_init(void){
	cr_clear_RE;
	cr_clear_RIE;
	cr_clear_ERI;
	cr_clear_TE;
	cr_clear_TIE;
	cr_clear_TEIE;
	clear_sr;
	cr_set_UE;
	sci_init_RxBuf();
	clear_sr;
	cr_set_ERI;
	cr_set_RE;
	sci_init_TxBuf();
	clear_sr;
	cr_set_TE;
}
//
// SCIのRX受信バッファを初期化する。
//
static void sci_init_RxBuf(void){
	sci.rb_siz= sizeof(sci.rb);
	sci.rb_r= sci.rb_w= 0;
	sci.err_flag= 0;
	sci.rx_data= 0;

	if(sci.rb_siz){ /* is buffering control? */
		cr_set_RIE;
	};
}

//
// SCIのTX送信バッファを初期化する。
//
static void sci_init_TxBuf(void){
	sci.tb_siz= sizeof(sci.tb);
	sci.tb_r= sci.tb_w= 0;
	sci.not_done= 0;
	sci.tx_data= 0;
}


// ****************************************************************************
// 例外処理ルーチン群
// ****************************************************************************
volatile uint16_t dummy_buffer;
// RXI (受信文字がRDRにあるので受信バッファに移す)
static void hndl_RXI(void){
	sci_RDR2RxBuf();
}
// ERI (受信エラーを受信エラーフラグにセットする)
static void hndl_ERI(void){
	sci.err_flag |= is_sr_ERRF;
	dummy_buffer= SCI->DR;
	dummy_buffer= SCI->SR;	// Error flags clear sequence
}
// TDRE 例外 (TDRが空いたので送信バッファにデータがあればそれを送る)
static void hndl_TXI(void){
	sci_TxBuf2TDR();
}
// TEND 例外 (送信処理中フラグをクリアする)
static void hndl_TEI(void){
	sci.not_done = 0;
	cr_clear_TEIE;
}

void hndl_USART2(void){
	ENTER_hndl;
	if(is_sr_ERRF)hndl_ERI();
	if(is_sr_RDRF)hndl_RXI(); // not bo read occuerd error sometime.
	if(is_sr_TDRE)hndl_TXI();
	if(is_sr_TEND)hndl_TEI();
	EXIT_hndl;
}


// ****************************************************************************
// 受信バッファ操作処理ルーチン(内部呼出専用)
// ****************************************************************************
//
// SCIのRDRからRX受信バッファへ文字を送る (送信バッファリング時に限り呼ばれる)
// 割り込み処理内部から呼ばれる処理であることに留意すること
//
static void sci_RDR2RxBuf(void){
	sci.rx_data= SCI->DR;
	if(BUFFER_FULL == sci.rb_w){  // 満タンだった場合のオーバフロー処理
		sci.err_flag |= RX_FLAG_OVFL;
		return;
	};
	sci.rb[sci.rb_w]= sci.rx_data;
	sci.rb_w++;
	if(sci.rb_w >= sci.rb_siz)sci.rb_w=0;
	if(sci.rb_w == sci.rb_r)sci.rb_w=BUFFER_FULL;
}
//
// SCIのRX受信バッファから文字を取り出す
// RX受信バッファから正常に取り出せたならば，その文字を返す
// RX受信バッファが空の場合は，-1を返す
//
static int16_t sci_getRxBuf(void){
    printf("                   \n");
    int16_t r;
	if(0 == sci.rb_siz){  // 非バッファリングの場合
		if(is_sr_RDRF){ // RDRにデータが入っていればその値をリターンする
			r= SCI->DR; return(r);
		};
		return(-1);
	};

	if(sci.rb_r == sci.rb_w) return(-1); // 空の場合

	UDI; // enter the critical section
		r= (int16_t)(sci.rb[sci.rb_r]);
		if(BUFFER_FULL == sci.rb_w) sci.rb_w= sci.rb_r; // 満タンだった場合
		sci.rb_r++;
		if( sci.rb_r >= sci.rb_siz )sci.rb_r= 0;
	UEI; // exit the critical section
//	if(SCI_nbuf()<(sci.rb_siz/2))SCI_set_RTS; // バッファが半分以上空いたらRTSをon
    printf("                 \n");
	return(r);
}

// ****************************************************************************
// 送信バッファ操作処理ルーチン(内部呼出専用)
// ****************************************************************************
//
// SCIのTX送信バッファからTDRへ文字を送る (送信バッファリング時に限り呼ばれる)
// 割り込み処理内部からも呼ばれる処理であることに留意すること
//
static void sci_TxBuf2TDR(void){
	if(sci.tb_r == sci.tb_w){ // 送信バッファが空の場合
		cr_clear_TIE;	// TDRE割込みを禁止してリターン
		return;
	};

	if(is_sr_TDRE){ // TDRが空いている場合にバッファから1文字送る
		sci.tx_data =sci.tb[sci.tb_r];
		dummy_buffer= SCI->SR;	// TDRE/TEND flags clear sequence1
		SCI->DR= sci.tx_data;	// TDRE/TEND flags clear sequence2

		sci.not_done= 1;	// 送信中
		cr_set_TEIE;	// 送信完了割込みを許可
		cr_set_TIE; 	// TDRE割込みを許可

		if(BUFFER_FULL == sci.tb_w) sci.tb_w= sci.tb_r; // 満タンだった場合の後始末
		sci.tb_r++;
		if( sci.tb_r >= sci.tb_siz) sci.tb_r= 0;
	};
	return;
}

//
// SCIのTX送信バッファへ文字を送り込む
// TX送信バッファへ正常に送り込めたならば0を返す
// TX送信バッファオーバフローの場合は -1を返す
//
static int16_t sci_putTxBuf(char c){
	if(0 == sci.tb_siz){  // 非バッファリングの場合
		cr_clear_TIE;	// TDRE割込みを禁止しておく
		if(is_sr_TDRE){ // TDRに空きがあればそのままセットして正常終了する
			SCI->DR= sci.tx_data= c;	// set SR_TXE automaticaly
			sci.not_done= 1;	// 送信中
			cr_set_TEIE;	// 送信完了割込みを許可
			return(0);
		};
		return(-1); // TDRに空きがないとバッファフルでエラーリターンする
	};

	if(BUFFER_FULL == sci.tb_w) return(-1); // 満タンの場合

	UDI; // enter the critical section
		sci.tb[sci.tb_w]= c;
		sci.tb_w++;
		if(sci.tb_w >= sci.tb_siz)sci.tb_w= 0;
		if(sci.tb_r == sci.tb_w){ // 満タンになった場合
			sci.tb_w = BUFFER_FULL;
		};
		sci_TxBuf2TDR();
	UEI; // exit the critical section
	return(0);
}

//
// リングバッファのインプリメント方法についてのメモ
//
#if 0

rは0～sz-1までの値をとるバッファの読み出しポインタ
wは0～sz-1までの値をとるバッファへの書き込みポインタ
ただし、バッファへの書き込みを終えた結果バッファがちょうど一杯になった場合には
wがBUFFER_SIZ(szの取れる最大値=0xffffffff)となる

バッファに入れる場合には
	wがMAX_BUF_SIZなら何もせずOVFセットのみでリターン
	wの位置に文字を置いて, wに1を加える
	w==rならばw=MAX_BUF_SIZとする

バッファから取り出す場合には
	r==wなら何もせず エラー値をリターン
	w==MAX_BUF_SIZならば，w=rとしてから
	rの位置の文字を取得して, rに1を加える
	取得した文字をリターン

nbuf探針で
	r==w なら0をリターン
	w==-1ならszをリターン
	r<w  ならw-rをリターン
	r>w  ならsz-r+wをリターン

szはMAXINT-1までが有効 16bitならば65535まで。0はバッファ不使用を表す。

#endif
#endif /* USART_H */
