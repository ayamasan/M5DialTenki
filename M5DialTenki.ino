#include <M5Dial.h>

#include "tenki.h"
#include "code.h"
#define CODENUM 122
#include "text.h"

#include "images.h"
// mm1-mm12 大画像 110x110
// s1-s12   小画像 50x50
// c1-c24   文字画像 90x48
// cs1      小文字 50x30
#define CLW 90
#define CLH 48
#define CSW 50
#define CSH 30
#define LW 110
#define LH 110
#define SW 50
#define SH 50
#define OFFSETX 25
#define OFFSETY 20
#define SPRITEX 190
#define SPRITEY 200

#define WIFI_SSID     "xxxxx"
#define WIFI_PASSWORD "xxxxxxxxxx"

#include <WiFi.h>
#include "ArduinoJson.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

// 東京の天気を取得（東京＝130000）
const char *apiServer = "https://www.jma.go.jp/bosai/forecast/data/forecast/130000.json";

char buf[100];
int sx, sy;
int bhour = 0;   // 取得時間
int bmin = 0;    // 取得時間
int bsec = 0;    // 取得時間

int dispnum = 0;
int tmonth[3] = {1, 1, 1};
int tday[3] = {1, 1, 1};
int tenki[3] = {0, 0, 0};

int tenkicode = 0;

int bkcolor = ORANGE;
int bknum = 0;

int first = 0;
long oldPosition = -999;

// 初期設定
void setup() {
	// put your setup code here, to run once:
	auto cfg = M5.config();
	M5Dial.begin(cfg, true, true);
	
	M5Dial.Display.fillScreen(BLACK);
	M5Dial.Display.setTextColor(GREEN, BLACK);
	M5Dial.Display.setTextDatum(middle_center);
	M5Dial.Display.setTextFont(4);
	M5Dial.Display.setTextSize(1);
	
	sx = M5Dial.Display.width() / 2;
	sy = M5Dial.Display.height() / 2;
	
	// WiFi接続
	M5Dial.Display.fillScreen(BLACK);
	M5Dial.Display.drawString("WiFi connecting...", sx, sy);
	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
	int tout = 0;
	while(WiFi.status() != WL_CONNECTED){
		delay(500);
		tout++;
		if(tout > 10){  // 5秒でタイムアウト、接続リトライ
			WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
			tout = 0;
		}
	}
	Serial.println("WiFi Connected.");
	M5Dial.Display.fillScreen(BLACK);
	M5Dial.Display.drawString("Connected.", sx, sy);
	
	Serial.printf("sx=%d, sy=%d\n", sx, sy);
	
	int loop = 0;
	while(loop == 0){
		M5Dial.Display.fillScreen(BLACK);
		M5Dial.Display.drawString("Get TENKI...", sx, sy);
		loop = GetTenki();
		M5Dial.Display.fillScreen(BLACK);
		delay(1000);
	}
	
	tenkicode = tenki[dispnum];
	Serial.printf("tenki[%d]=%d, code=%d\n", dispnum, tenkicode);
	disptenki(tenkicode);
}

// メインループ
void loop() {
	// put your main code here, to run repeatedly:
	M5Dial.update();
	int loop = 0;
	
	if(M5Dial.BtnA.wasPressed()){
		while(loop == 0){
			M5Dial.Display.fillScreen(BLACK);
			M5Dial.Display.drawString("Get TENKI...", sx, sy);
			loop = GetTenki();
			M5Dial.Display.fillScreen(BLACK);
			delay(1000);
		}
		dispnum = 0;
		tenkicode = tenki[dispnum];
		Serial.printf("tenki[%d]=%d, code=%d\n", dispnum, tenkicode);
		disptenki(tenkicode);
	}
	
	long newPosition = M5Dial.Encoder.read();
	if(first == 0){
		oldPosition = newPosition;
		first = 1;
	}
	if(newPosition > oldPosition+3){
		// 右回り
		if(dispnum == 2) dispnum = 0;
		else             dispnum++;
		tenkicode = tenki[dispnum];
		Serial.printf("tenki[%d]=%d\n", dispnum, tenkicode);
		disptenki(tenkicode);
		oldPosition = newPosition;
	}
	else if(newPosition < oldPosition-3){
		// 左回り
		if(dispnum == 0) dispnum = 2;
		else             dispnum--;
		tenkicode = tenki[dispnum];
		Serial.printf("tenki[%d]=%d\n", dispnum, tenkicode);
		disptenki(tenkicode);
		oldPosition = newPosition;
	}
	delay(20);
}

// 天気取得
int GetTenki() 
{
	int httpCode;
	// json用メモリ確保
	DynamicJsonDocument doc(4096);
	
	Serial.println("Start Tenki...");
	
	if((WiFi.status() == WL_CONNECTED)){
		HTTPClient http;
		
		// HTTP接続開始
		http.begin(apiServer);
		
		// リクエスト作成
		httpCode = http.GET();
		
		// 返信
		if(httpCode > 0){
			// 応答取得
			String payload = http.getString();
			// ペイロードをjson変換
			deserializeJson(doc, payload);
			
			String date = doc[0][String("reportDatetime")]; //[0][String("timeDefines")];
			tenki[0] = doc[0]["timeSeries"][0]["areas"][0]["weatherCodes"][0];
			tenki[1] = doc[0]["timeSeries"][0]["areas"][0]["weatherCodes"][1];
			tenki[2] = doc[0]["timeSeries"][0]["areas"][0]["weatherCodes"][2];
			Serial.printf("TENKI = %d, %d, %d\n", tenki[0], tenki[1], tenki[2]);
			char ss[3];
			String date1 = doc[0]["timeSeries"][0]["timeDefines"][0];
			ss[0] = date1[5];
			ss[1] = date1[6];
			ss[2] = 0;
			tmonth[0] = atoi(ss);
			ss[0] = date1[8];
			ss[1] = date1[9];
			ss[2] = 0;
			tday[0] = atoi(ss);
			String date2 = doc[0]["timeSeries"][0]["timeDefines"][1];
			ss[0] = date2[5];
			ss[1] = date2[6];
			ss[2] = 0;
			tmonth[1] = atoi(ss);
			ss[0] = date2[8];
			ss[1] = date2[9];
			ss[2] = 0;
			tday[1] = atoi(ss);
			String date3 = doc[0]["timeSeries"][0]["timeDefines"][2];
			ss[0] = date3[5];
			ss[1] = date3[6];
			ss[2] = 0;
			tmonth[2] = atoi(ss);
			ss[0] = date3[8];
			ss[1] = date3[9];
			ss[2] = 0;
			tday[2] = atoi(ss);
			
			Serial.printf("DATE = %02d/%02d, %02d/%02d, %02d/%02d\n"
			, tmonth[0], tday[0], tmonth[1], tday[1], tmonth[2], tday[2]);
			
			String weather1 = doc[0]["timeSeries"][0]["areas"][0]["weathers"][0];
			Serial.println(weather1);
			String weather2 = doc[0]["timeSeries"][0]["areas"][0]["weathers"][1];
			Serial.println(weather2);
			String weather3 = doc[0]["timeSeries"][0]["areas"][0]["weathers"][2];
			Serial.println(weather3);
			
			http.end();
			return(1);
		}
		else{
			Serial.println("!x!");
			http.end();
		}
	}
	else{
		Serial.print("WiFi Error!");
		M5Dial.Display.drawString("WiFi connecting...", sx, sy);
		WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
	}
	return(0);
}


int disptenki(int code)
{
	int i;
	int yoffset;
	unsigned char *img;
	int size;
	unsigned int dd1 = 0;
	unsigned int dd2 = 0;
	int mm[4];
	int ct[3];
	int bigimg = 0;
	
	M5Canvas canvas(&M5Dial.Display);
	
	
	for(i=0; i<CODENUM; i++){
		if(CODE[i] == code){
			dd1 = TENKI[i][0];
			dd2 = TENKI[i][1];
			Serial.printf("===> %d[%d]=[%s]\n", code, i, text[i]);
			break;
		}
	}
	Serial.printf("CODE[%d]=%d, dd1=0x%08X, dd2=0x%08X\n", i, code, dd1, dd2);
	if(dd1 != 0 || dd2 != 0){
		imgcheck(dd1, dd2, (int *)&mm[0], (int *)&mm[1], (int *)&mm[2], (int *)&mm[3]);
		// 背景色設定
		switch(mm[0]){
			case 1: bkcolor = ORANGE;    bknum = mm[0]; break;
			case 2: bkcolor = DARKCYAN;  bknum = mm[0]; break;
			case 3: bkcolor = LIGHTGREY; bknum = mm[0]; break;
			case 4: bkcolor = WHITE;     bknum = mm[0]; break;
			case 5: bkcolor = NAVY;      bknum = mm[0]; break;
			case 6: bkcolor = DARKGREY;  bknum = mm[0]; break;
			default : bkcolor = WHITE;   bknum = 0; break;
		}
		Serial.printf("Set BKcolor = %X (mm[0]=%d(%d)\n", bkcolor, bknum);
		M5Dial.Display.fillRect(0, 0, 240, 240, bkcolor);
		
		Serial.printf("mm[0]=%d, mm[1]=%d, mm[2]=%d, mm[3]=%d\n", mm[0], mm[1], mm[2], mm[3]);
		// 表示実行
		canvas.createSprite(SPRITEX, SPRITEY);
		canvas.fillRect(0, 0, SPRITEX, SPRITEY, bkcolor);
		
		i = 0;
		if(((dd1 & 0xF000FFFF) == 0) && ((dd2 & 0xF0000FF0) == 0)){
			// 文章無し
			if((mm[0] + mm[1]) == 0 || (mm[2] + mm[3]) == 0){
				// 大画像
				// 1列
				if(mm[0] != 0){
					// 左列
					yoffset = 0;
					if(mm[1] != 0){
						// 2段目
						img = getimg(1, mm[1], &size);
						yoffset = LH/2;
						if(size > 0){
							canvas.pushSprite(0, 0);
							canvas.drawPng(img, size, sx-(LW/2)-OFFSETX, sy-(LH/2)-yoffset-OFFSETY, LW, LH);
						}
						// ライン「か」
						canvas.drawLine(40, sy, sx*2-40, sy, WHITE);
					}
					img = getimg(1, mm[0], &size);
					if(size > 0){
						canvas.drawPng(img, size, sx-(LW/2)-OFFSETX, sy-(LH/2)+yoffset-OFFSETY, LW, LH);
					}
				}
				else{
					// 右列
					yoffset = 0;
					if(mm[3] != 0){
						// 2段目
						img = getimg(1, mm[3], &size);
						yoffset = LH/2;
						if(size > 0){
							canvas.drawPng(img, size, sx-(LW/2)-OFFSETX, sy-(LH/2)-yoffset-OFFSETY, LW, LH);
						}
						// ライン「か」
						canvas.drawLine(40, sy, sx*2-40, sy, WHITE);
					}
					img = getimg(1, mm[2], &size);
					if(size > 0){
						canvas.drawPng(img, size, sx-(LW/2)-OFFSETX, sy-(LH/2)+yoffset-OFFSETY, LW, LH);
					}
				}
			}
		}
		else if(((dd1 & 0x1000FFFF) == 0) && ((dd2 & 0xFFFFFFF0) == 0)){
			// 文章無し
			Serial.printf("mm[0]=%d, mm[1]=%d, mm[2]=%d, mm[3]=%d\n", mm[0], mm[1], mm[2], mm[3]);
			// 大画像
			// 1列
			if(mm[0] != 0){
				// 左列
				yoffset = 0;
				if(mm[1] != 0){
					// 左表示
					img = getimg(1, mm[1], &size);
					yoffset =LH/2;
					if(size > 0){
						canvas.drawPng(img, size, sx-(LW/2)-yoffset-OFFSETX, sy-(LH/2)-OFFSETY, LW, LH);
					}
				}
				// 右表示
				img = getimg(1, mm[0], &size);
				if(size > 0){
					canvas.drawPng(img, size, sx-(LW/2)+yoffset-OFFSETX, sy-(LH/2)-OFFSETY, LW, LH);
					canvas.drawPng(img, size, sx-(LW/2)+yoffset, sy-(LH/2), LW, LH);
				}
			}
		}
		else{
			// 小アイコン表示 
			ct[0] = -1;
			ct[1] = -1;
			ct[2] = -1;
			textcheck(dd1, dd2, &ct[0]);
			// 大文字が無く、画像が1つならば、大画像で表示する
			if(ct[0] < 0 && ct[1] < 0 && ct[2] < 0 && mm[1] == 0 && (dd1 & 0x90000000) == 0){
				// 画像1（大、中央）
				img = getimg(1, mm[0], &size);
				canvas.drawPng(img, size, sx-(LW/2)-OFFSETX, sy-(LH/2)-OFFSETY, LW, LH);
				bigimg = 1;
			}
			else{
				// 1列目（小文字＋画像＋画像）
				yoffset = 0;
				// 画像2
				if(mm[1] != 0){
					img = getimg(0, mm[1], &size);
					yoffset += (SH/2);
					canvas.drawPng(img, size, sx-SW-(CLW/2)-OFFSETX, sy-(SH/2)-yoffset-OFFSETY, SW, SH);
				}
				// 画像1
				img = getimg(0, mm[0], &size);
				canvas.drawPng(img, size, sx-SW-(CLW/2)-OFFSETX, sy-(SH/2)+yoffset-OFFSETY, SW, SH);
				// 朝の内(cs1) 50x30
				if((dd1 & 0x90000000) != 0){
					if(mm[0] == 0 && mm[1] == 0){  // 画像無しなら中央に表示
						canvas.drawPng(cs1, sizeof(cs1), sx-CSW-(CLW/2)-OFFSETX, sy-(CSH/2)-OFFSETY, CSW, CSH);
					}
					else{
						canvas.drawPng(cs1, sizeof(cs1), sx-CSW-(CLW/2)-OFFSETX, sy-(SH/2)-yoffset-CSH-OFFSETY, CSW, CSH);
					}
				}
			}
			
			// 2列目（大文字）
			yoffset = 0;
			if(ct[2] >= 0){
				img = gettext(ct[0], &size);
				canvas.drawPng(img, size, sx-(CLW/2)-OFFSETX, sy-(55/2)-55-OFFSETY, CLW, 55);
				img = gettext(ct[1], &size);
				canvas.drawPng(img, size, sx-(CLW/2)-OFFSETX, sy-(55/2)-OFFSETY, CLW, 55);
				img = gettext(ct[2], &size);
				canvas.drawPng(img, size, sx-(CLW/2)-OFFSETX, sy-(55/2)+55-OFFSETY, CLW, 55);
			}
			else if(ct[1] >= 0){
				img = gettext(ct[0], &size);
				canvas.drawPng(img, size, sx-(CLW/2)-OFFSETX, sy-55-OFFSETY, CLW, 55);
				img = gettext(ct[1], &size);
				canvas.drawPng(img, size, sx-(CLW/2)-OFFSETX, sy-OFFSETY, CLW, 55);
			}
			else if(ct[0] >= 0){
				img = gettext(ct[0], &size);
				canvas.drawPng(img, size, sx-(CLW/2)-OFFSETX, sy-(55/2)-OFFSETY, CLW, 55);
			}
			
			// 3列目（画像＋画像＋小文字）
			// 画像2
			if(mm[3] != 0){
				img = getimg(0, mm[3], &size);
				canvas.drawPng(img, size, sx+(CLW/2)-OFFSETX, sy-(SH/2)-SH-OFFSETY, SW, SH);
			}
			// 画像1
			img = getimg(0, mm[2], &size);
			canvas.drawPng(img, size, sx+(CLW/2)-OFFSETX, sy-(SH/2)-OFFSETY, SW, SH);
			
			if((ct[0] < 0 && ct[1] < 0 && ct[2] < 0) && mm[3] == 0 && bigimg == 0){
				// 大文字（中央）
				if((dd2 & 0x00000400) != 0){  // 強く降る
					canvas.drawPng(c22, sizeof(c22), sx-(CLW/2)-OFFSETX, sy-(CLH/2)-OFFSETY, CLW, CLH);
				}
				else if((dd2 & 0x00000200) != 0){  // 強い
					canvas.drawPng(c21, sizeof(c21), sx-(CLW/2)-OFFSETX, sy-(CLH/2)-OFFSETY, CLW, CLH);
				}
				else if((dd2 & 0x00000100) != 0){  // 止む
					canvas.drawPng(c20, sizeof(c20), sx-(CLW/2)-OFFSETX, sy-(CLH/2)-OFFSETY, CLW, CLH);
				}
				else if((dd2 & 0x00000020) != 0){  // 暴風
					canvas.drawPng(c24, sizeof(c24), sx-(CLW/2)-OFFSETX, sy-(CLH/2)-OFFSETY, CLW, CLH);
				}
				else if((dd2 & 0x00000010) != 0){  // 雷を伴う
					canvas.drawPng(c23, sizeof(c23), sx-(CLW/2)-OFFSETX, sy-(CLH/2)-OFFSETY, CLW, CLH);
				}
			}
			else{
				// 小文字
				if(mm[2] == 0 && mm[3] == 0){
					yoffset = -(CSH/2);
				}
				else{
					yoffset = (SH/2);
				}
				if((dd2 & 0x00000400) != 0){  // 強く降る
					canvas.drawPng(cs22, sizeof(cs22), sx+(CLW/2)-OFFSETX, sy+yoffset-OFFSETY, CSW, CSH);
				}
				if((dd2 & 0x00000200) != 0){  // 強い
					canvas.drawPng(cs21, sizeof(cs21), sx+(CLW/2)-OFFSETX, sy+yoffset-OFFSETY, CSW, CSH);
				}
				if((dd2 & 0x00000100) != 0){  // 止む
					canvas.drawPng(cs20, sizeof(cs20), sx+(CLW/2)-OFFSETX, sy+yoffset-OFFSETY, CSW, CSH);
				}
				if((dd2 & 0x00000020) != 0){  // 暴風
					canvas.drawPng(cs24, sizeof(cs24), sx+(CLW/2)-OFFSETX, sy+yoffset-OFFSETY, CSW, CSH);
				}
				if((dd2 & 0x00000010) != 0){  // 雷を伴う
					canvas.drawPng(cs23, sizeof(cs23), sx+(CLW/2)-OFFSETX, sy+yoffset-OFFSETY, CSW, CSH);
				}
			}
		}
		
		
		canvas.pushSprite(OFFSETX, OFFSETY);
		
		// 日にち表示
		char buf[100];
		Serial.printf("BKcolor = %X\n", bkcolor);
		if(bknum == 2 || bknum == 5 || bknum == 6){
			M5Dial.Display.setTextColor(WHITE);
		}
		else{
			M5Dial.Display.setTextColor(BLACK);
		}
		sprintf(buf, "%d/%d", tmonth[dispnum], tday[dispnum]);
		M5Dial.Display.setTextSize(1);
		M5Dial.Display.drawString(buf, sx, sy-90);
	}
	else{
		return(0);
	}
	return(code);
}

// 画像テーブルから左右上下の画像番号取得
// ma1, ma2 : 左画像、mb1, mb2 : 右画像
int imgcheck(unsigned int dd1, unsigned int dd2, int *ma1, int *ma2, int *mb1, int *mb2){
	int i = 0;
	unsigned int dd;
	*ma1 = 0;
	*ma2 = 0;
	*mb1 = 0;
	*mb2 = 0;
	
	dd = (dd1 >> 24 & 0x03);
	if(dd == 1) *ma1 = 1; else if(dd == 2) *ma2 = 1;
	dd = (dd1 >> 26 & 0x03);
	if(dd == 1) *ma1 = 2; else if(dd == 2) *ma2 = 2;
	dd = (dd1 >> 20 & 0x03);
	if(dd == 1) *ma1 = 3; else if(dd == 2) *ma2 = 3;
	dd = (dd1 >> 22 & 0x03);
	if(dd == 1) *ma1 = 4; else if(dd == 2) *ma2 = 4;
	dd = (dd1 >> 16 & 0x03);
	if(dd == 1) *ma1 = 5; else if(dd == 2) *ma2 = 5;
	dd = (dd1 >> 18 & 0x03);
	if(dd == 1) *ma1 = 6; else if(dd == 2) *ma2 = 6;
	dd = (dd1 >> 29 & 0x01);
	if(dd != 0) *ma2 = 7;
	dd = (dd1 >> 30 & 0x01);
	if(dd != 0) *ma2 = 8;
	dd = (dd1 >> 31 & 0x01);
	if(dd != 0) *ma1 = 10;
	
	dd = (dd2 >> 24 & 0x03);
	if(dd == 1) *mb1 = 1; else if(dd == 2) *mb2 = 1;
	dd = (dd2 >> 26 & 0x03);
	if(dd == 1) *mb1 = 2; else if(dd == 2) *mb2 = 2;
	dd = (dd2 >> 20 & 0x03);
	if(dd == 1) *mb1 = 3; else if(dd == 2) *mb2 = 3;
	dd = (dd2 >> 22 & 0x03);
	if(dd == 1) *mb1 = 4; else if(dd == 2) *mb2 = 4;
	dd = (dd2 >> 16 & 0x03);
	if(dd == 1) *mb1 = 9; else if(dd == 2) *mb2 = 9;
	dd = (dd2 >> 18 & 0x03);
	if(dd == 1) *mb1 = 10; else if(dd == 2) *mb2 = 10;
	dd = (dd2 >> 12 & 0x03);
	if(dd == 1) *mb1 = 11; else if(dd == 2) *mb2 = 11;
	dd = (dd2 >> 14 & 0x03);
	if(dd == 1) *mb1 = 12; else if(dd == 2) *mb2 = 12;
	
	i = 0;
	if(*ma1 != 0) i++;
	if(*ma2 != 0) i++;
	if(*mb1 != 0) i++;
	if(*mb2 != 0) i++;
	
	return(i);
}


// 画像番号から画像ポインタとサイズ取得
unsigned char *getimg(int lm, int num, int *size){
	*size = 0;
	unsigned char *img;
	
	if(lm == 1){  // 大画像
		switch(num){
			case  1 : img = (unsigned char *)&mm1[0]; *size=sizeof(mm1); break;
			case  2 : img = (unsigned char *)&mm2[0]; *size=sizeof(mm2); break;
			case  3 : img = (unsigned char *)&mm3[0]; *size=sizeof(mm3); break;
			case  4 : img = (unsigned char *)&mm4[0]; *size=sizeof(mm4); break;
			case  5 : img = (unsigned char *)&mm5[0]; *size=sizeof(mm5); break;
			case  6 : img = (unsigned char *)&mm6[0]; *size=sizeof(mm6); break;
			case  7 : img = (unsigned char *)&mm7[0]; *size=sizeof(mm7); break;
			case  8 : img = (unsigned char *)&mm8[0]; *size=sizeof(mm8); break;
			case  9 : img = (unsigned char *)&mm9[0]; *size=sizeof(mm9); break;
			case 10 : img = (unsigned char *)&mm10[0]; *size=sizeof(mm10); break;
			case 11 : img = (unsigned char *)&mm11[0]; *size=sizeof(mm11); break;
			case 12 : img = (unsigned char *)&mm12[0]; *size=sizeof(mm12); break;
			default : break;
		}
	}
	else{  // 小画像
		switch(num){
			case  1 : img = (unsigned char *)&s1[0]; *size=sizeof(s1); break;
			case  2 : img = (unsigned char *)&s2[0]; *size=sizeof(s2); break;
			case  3 : img = (unsigned char *)&s3[0]; *size=sizeof(s3); break;
			case  4 : img = (unsigned char *)&s4[0]; *size=sizeof(s4); break;
			case  5 : img = (unsigned char *)&s5[0]; *size=sizeof(s5); break;
			case  6 : img = (unsigned char *)&s6[0]; *size=sizeof(s6); break;
			case  7 : img = (unsigned char *)&s7[0]; *size=sizeof(s7); break;
			case  8 : img = (unsigned char *)&s8[0]; *size=sizeof(s8); break;
			case  9 : img = (unsigned char *)&s9[0]; *size=sizeof(s9); break;
			case 10 : img = (unsigned char *)&s10[0]; *size=sizeof(s10); break;
			case 11 : img = (unsigned char *)&s11[0]; *size=sizeof(s11); break;
			case 12 : img = (unsigned char *)&s12[0]; *size=sizeof(s12); break;
			default : break;
		}
	}
	
	return(img);
}

// テキスト画像データ番号取得
int textcheck(unsigned int dd1, unsigned int dd2, int *ct){
	int i;
	int j = 0;
	
	for(i= 0; i<20; i++){
		if(i < 16){
			if((dd1 << i) & 0x00008000){
				ct[j] = i;
				j++;
				if(j >= 3) break;
			}
		}
		else{
			if((dd2 >> (i-16)) & 0x10000000){
				ct[j] = i;
				j++;
				if(j >= 3) break;
			}
		}
	}
	
	Serial.printf("dd1=%08X, dd2=%08X, ct[0]=%d, ct[1]=%d, ct[3]=%d\n", dd1, dd2, ct[0], ct[1], ct[2]);
	return(j);
}


// テキスト画像取得
unsigned char *gettext(int ct, int *size){
	*size = 0;
	unsigned char *img;
	
	switch(ct){
		case   0 : img = (unsigned char *)&c4[0]; *size=sizeof(c4); break;
		case   1 : img = (unsigned char *)&c3[0]; *size=sizeof(c3); break;
		case   2 : img = (unsigned char *)&c1[0]; *size=sizeof(c1); break;
		case   3 : img = (unsigned char *)&c2[0]; *size=sizeof(c2); break;
		case   5 : img = (unsigned char *)&c7[0]; *size=sizeof(c7); break;
		case   6 : img = (unsigned char *)&c6[0]; *size=sizeof(c6); break;
		case   7 : img = (unsigned char *)&c5[0]; *size=sizeof(c5); break;
		case   8 : img = (unsigned char *)&c11[0]; *size=sizeof(c11); break;
		case   9 : img = (unsigned char *)&c10[0]; *size=sizeof(c10); break;
		case  10 : img = (unsigned char *)&c9[0]; *size=sizeof(c9); break;
		case  11 : img = (unsigned char *)&c8[0]; *size=sizeof(c8); break;
		case  12 : img = (unsigned char *)&c15[0]; *size=sizeof(c15); break;
		case  13 : img = (unsigned char *)&c14[0]; *size=sizeof(c14); break;
		case  14 : img = (unsigned char *)&c13[0]; *size=sizeof(c13); break;
		case  15 : img = (unsigned char *)&c12[0]; *size=sizeof(c12); break;
		case  16 : img = (unsigned char *)&c16[0]; *size=sizeof(c16); break;
		case  17 : img = (unsigned char *)&c17[0]; *size=sizeof(c17); break;
		case  18 : img = (unsigned char *)&c18[0]; *size=sizeof(c18); break;
		case  19 : img = (unsigned char *)&c19[0]; *size=sizeof(c19); break;
		default : *size = 0; break;
	
	}
	
	return(img);
}
