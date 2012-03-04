#include <Windows.h>
#include <tchar.h>
#include <math.h>
#include <time.h>
#include <iostream>

using namespace std;

#ifndef UNICODE
#define UNICODE
#define _UNICODE
#endif

#include "../inc/TuringCup9ClientAPI.h"
#pragma comment(lib, "../lib/TuringCup9.lib")

#define CLIENT_EXPORT
#include "Tc_Client.h"

#define STEPLENGTH 8  //用于判断方向的一个去精确量，避免出现晃动 
#define DIS 0			//英雄间的距离
#define TEAMWIDTH 350 //阵型的宽度
#define VY 7				   //纵向速度
#define VX 7				    //横向运动速度
#define VXY 5               //斜向运动的速度
#define FLEEDIST 100   //是否需要避难的尺度 : freeball 和 hero之间的距离
#define FLEEDIST2 100
#define SPELLDIST 160 //施暴的尺度控制量:  离禁区的x距离
#define SINGTIME 30 //施暴吟唱时间
#define DIFLR_BALL 64
#define DIFLR_MAN 128
#define SCOREFIELD 300

const double esp = 10E-9;
const double pi = acos(-1.0);


/*

How to debug?

Goto
Project -. Properties --> Configuration Properties --> Debugging

Copy "$(ProjectDir)TuringCup9.exe" 
to the Command field.(Without quotation marks)

*/


CLIENT_EXPORT_API void __stdcall Init()
{
	//
	// TODO: Set team name(no Chinese) and choose heros here
	//
	Tc_SetTeamName(L"civilian");
	Tc_ChooseHero(TC_MEMBER1, TC_HERO_MALFOY, L"malfoy");
	Tc_ChooseHero(TC_MEMBER2, TC_HERO_GINNY, L"ginny");
}

//
// TODO: Rename your DLL's filename with your team name
//

typedef TC_Position myVector;

//some struct to hold the information
TC_Handle heroHandle[2],freeBall,ghostBall,goldBall,enemyHandle[2];
TC_Hero heroInfo[2],enemyInfo[2],preEnemyInfo[2];
TC_Ball freeInfo,ghostInfo,goldInfo;
TC_GameInfo gameInfo,preInfo;
TC_Gate myGate,enemyGate;
TC_ForbiddenArea forbiddenArea;
TC_Position upPass,downPass;
bool danger[2];
int dangerCount[2] = {-30,-30};
int back;
int curWork;
int throwed=0;
int count = 0;
int spellCount = -100;
int spellEnemy = -1;
int speller = -1;
int mapLeft,mapRight;
int ballTaker;
bool haveball;

TC_Position dest;
TC_Position upThrow,downThrow;
int gateUpper;
int gateLower;
int mid;

void GetGoldBall();
TC_Position GetGoldUpper();
TC_Position GetGoldLower();

void GetCurrInfo();
void GetCurrStat();
void MoveTo(int id,TC_Position dest);
TC_Direction PosRelative(TC_Position pOrign,TC_Position dest);
bool SuperPosition(TC_Position p1,TC_Position p2);
bool GhostInAir();
TC_Position ScorePot(int type = 0);
TC_Position GetDest(int id);
TC_Position GetSaveWay(TC_Position pOrign);
TC_Position NextPosition(TC_Position p,TC_Direction dir);
bool TooClose(TC_Position p1,TC_Position p2,int dis = FLEEDIST);
int dist(TC_Position p1,TC_Position p2);
int CountEnemySurround(int id);
bool Throwing();
bool ShouldThrow(int id);
bool NotOverForbid(TC_Ball ball);
TC_Position GetThrowTarget(int id);
bool ShouldGetGold(int id  = 5);
bool ShouldSpell(int id);
bool ShouldSpell();
bool CanSpell(int id,int enemyId);
bool InMap(TC_Position pos);
int ChooseSpeller();
int EnemyAhead(int id);  //用向量的方式算前方敌人的个数，用于判断是否该向前扔
bool Between(TC_Position pp,TC_Position p1,TC_Position p2);
int sqr(int num);
int NearHero();
bool InLine();
bool ShouldPass(int id);
TC_Position GetPassTarget(int id);
int DistFromGoal(int id);
bool ScoreField();
bool ShouldDrop(int id);
int CountInLine(int id);
TC_Position GetDropPosition(int id);
bool ShouldGetBall(int id);
bool InDoor(int id);
bool EnemyScoreField();
bool OK(int id);
/**************************************************
向量计算相关函数
***************************************************/
TC_Position operator-(const TC_Position &a,const TC_Position &b){
	TC_Position ans;
	ans.x = a.x-b.x;
	ans.y = a.y-b.y;
	return ans;
}

double Length(TC_Position a){
	return sqrt(double(a.x*a.x + a.y*a.y));
}

int DotMul(TC_Position a,TC_Position b)
{
	return a.x*b.x + a.y*b.y;
}
int DotMul(TC_Position a,TC_Position b,TC_Position c)
{
	return DotMul(a-c,b-c);
}
int CrossMul(TC_Position a,TC_Position b)
{
	return a.x*b.y-a.y*b.x;
}
int CrossMul(TC_Position a,TC_Position b,TC_Position c)
{
	return CrossMul(a-c,b-c);
}
double Relation(const TC_Position&p, const TC_Path &l)
{
	return double(DotMul(p,l.pos_end,l.pos_start))/dist(l.pos_end,l.pos_start);
}

TC_Position Rotate(TC_Position &o, TC_Position p,double alpha){
	TC_Position ans;
	p.x -= o.x;
	p.y -= o.y;
	ans.x = int(p.x*cos(alpha)-p.y*sin(alpha)+o.x+0.5);
	ans.y = int(p.y*cos(alpha)+p.x*sin(alpha)+o.y+0.5);
	return ans;
}

//两向量之间的夹角,逆时针为正，大于pi返回正值，小于pi返回负值
double Angle(myVector a, myVector b)
{
	double ret = acos(DotMul(a, b) / (Length(a)*Length(b)));
	if (CrossMul(a, b) < 0)
		return ret;
	else
		return -ret;
}



CLIENT_EXPORT_API void __stdcall AI()
{
	//
	// TODO: Add your AI code here
	//
	GetCurrInfo();
	count++;

	if(count == 1)
	{
		danger[0] = danger[1] = false;
		preInfo.score_self = 100;
		preInfo.score_self = 100;
		haveball = false;
	}

	GetCurrStat();


	if(ShouldGetGold()){
		GetGoldBall();
		return;
	}
	if(throwed>0)
	{
		throwed++;
		if(!Throwing())
			throwed = 0;
	}
	else throwed = 0;
	if(count - spellCount > SINGTIME)
		spellEnemy = -1;

	//int ballEnemy = enemyInfo[0].b_ghostball?0:1;

	if(ShouldSpell()){
		int speller = ChooseSpeller();
		if(speller != -1){
			if(!Tc_Spell(heroHandle[speller],enemyHandle[spellEnemy])){//如果失败则相当于没施法过
				spellEnemy = -1;
			}
			else
			{
				spellCount = count;
				::speller = speller;
				goto endAI;
			}
		}
		else
		{
			spellEnemy = -1;
		}
	}

/*people0:*/
	dest = GetDest(0);
	MoveTo(0,dest);

	if(ShouldPass(0)){
		TC_Position passTarget = GetPassTarget(0);
		if(Tc_PassBall(heroHandle[0],passTarget))
		{
			throwed = 1;
			ballTaker = 1-ballTaker;
			goto people1;
		}
		else
		{
		}
	}

	if(ShouldThrow(0)){
		TC_Position throwTarget = GetThrowTarget(0);
		if(Tc_PassBall(heroHandle[0],throwTarget))
		{
			throwed = 1;
			goto people1;
		}
		else
		{
		}
	}

	if(ShouldDrop(0)){
		TC_Position dropTarget = GetDropPosition(0);
		if(Tc_PassBall(heroHandle[0],dropTarget))
		{
			throwed = 1;
			ballTaker = 1-ballTaker;
			goto people1;
		}
		else
		{
		}
	}

	
	if(ShouldGetBall(0))

		Tc_SnatchBall(heroHandle[0],TC_GHOST_BALL);

	if(heroInfo[0].b_snatch_goldball)
		Tc_SnatchBall(heroHandle[0],TC_GOLD_BALL);


people1:
	dest = GetDest(1);
	MoveTo(1,dest);

	if(ShouldPass(1)){
		TC_Position passTarget = GetPassTarget(1);
		if(Tc_PassBall(heroHandle[1],passTarget))
		{
			throwed = 1;
			ballTaker = 1-ballTaker;
			goto endAI;
		}
		else
		{
		}

	}

	if(ShouldThrow(1)){
		TC_Position throwTarget = GetThrowTarget(1);
		if(Tc_PassBall(heroHandle[1],throwTarget)){
			throwed = 1;
			goto endAI;
		}
		else
		{
		}
	}

	if(ShouldDrop(1)){
		TC_Position dropTarget = GetDropPosition(1);
		if(Tc_PassBall(heroHandle[1],dropTarget)){
			throwed = 1;
			ballTaker = 1-ballTaker;
			goto endAI;
		}
		else
		{
		}
	}
	

	
	if(ShouldGetBall(1))
		Tc_SnatchBall(heroHandle[1],TC_GHOST_BALL);
	if(heroInfo[1].b_snatch_goldball)
		Tc_SnatchBall(heroHandle[1],TC_GOLD_BALL);

endAI:
	preInfo = gameInfo;
	preEnemyInfo[0] = enemyInfo[0];
	preEnemyInfo[1] = enemyInfo[1];

}

bool OK(int id){
	if(heroInfo[id].abnormal_type == TC_SPELLED_BY_NONE || heroInfo[id].abnormal_type == TC_SPELLED_BY_VOLDEMORT)
		return true;
	else
		return false;
}
bool InDoor(int id)
{
	if(heroInfo[id].pos.x>=mapRight-TC_HERO_WIDTH || heroInfo[id].pos.x<=mapLeft)
		return true;
	return false;
}

bool EnemyScoreField(){
	TC_Position ball = ghostInfo.pos;
	ball.x+=32;
	ball.y+=32;

	if(abs(ball.x-myGate.x)<300)
		return true;
	else
		return false;

}
bool ShouldGetBall(int id){

	if(!heroInfo[id].b_snatch_ghostball)return false;
	if(danger[id])return false;
	if(heroInfo[1-id].b_ghostball && OK(1-id) && !danger[1-id])
		return false;
	return true;
}

bool ShouldDrop(int id){
	if(heroInfo[id].b_ghostball == false)return false;

	if(danger[id] && count-dangerCount[id]>18)
		return true;
	else
		return false;
}

TC_Position GetDropPosition(int id){

	TC_Position ans;
	if(ScoreField()){

// 		if(danger[1-id] == false){
// 			if(dist(upThrow,heroInfo[1-id].pos)<dist(downThrow,heroInfo[1-id].pos))
// 				ans = upThrow;
// 			else
// 				ans = downThrow;
// 		}
// 		else
// 		{
			if(dist(upThrow,heroInfo[id].pos) < dist(downThrow,heroInfo[id].pos))
				ans = upThrow;
			else
				ans = downThrow;
/*		}*/
	}
	else
	{
		
		ans.x = ghostInfo.pos.x;
		if(ghostInfo.pos.y>350)
		{
			ans.y = ghostInfo.pos.y-=400;
			if(ans.y<64)
				ans.y = 64;
		}
		else{
			ans.y = ghostInfo.pos.y+400;
			if(ans.y >650)
				ans.y = 650;
		}
	}
	return ans;
}
int CountInLine(int id){
	int ans,i;
	ans = 0;
// 	TC_Position hero,enemy1,enemy2;
// 	hero = heroInfo[id].pos;
// 	enemy1 = enemyInfo[0].pos;
// 	enemy1 = enemyInfo[1].pos;
// 	hero.x+=64;
// 	hero.y+=64;
// 	enemy1.x+=64;
// 	enemy1.y+=64;
// 	enemy2.x+=64;
// 	enemy2.y+=64;

	for(i = 0; i<2; i++){
		if(abs(heroInfo[id].pos.y-enemyInfo[i].pos.y)<35)
			ans++;
	}

// 	if(dist(hero,enemy1)<8100 && dist(hero,enemy2)<8100)
// 		return 0;
	return ans;
}
bool ScoreField(){
	TC_Position ballCenter =ghostInfo.pos;
	ballCenter.x+=32;
	ballCenter.y+=32;
	if(back == -1){
		if(ballCenter.x>enemyGate.x-SCOREFIELD)
			return true;
		else
			return false;
	}
	else
	{
		if(ballCenter.x<enemyGate.x+SCOREFIELD)
			return true;
		else
			return false;
	}
}

int DistFromGoal(int id){
	TC_Position p = heroInfo[id].pos;
	int ans = 0;
	if(back == -1)
	{
		ans += (enemyGate.x-p.x);
	}
	else
	{
		ans+= (TC_HERO_WIDTH+p.x-enemyGate.x);
	}

	if(p.y<gateUpper)
		ans+=(gateUpper-p.y);
	if(p.y>gateLower)
		ans+=(p.y-gateLower);

	return ans;
}

int CountEnemyAhead(int id){
	int ans = 0,i;
	TC_Position heroCenter;
	heroCenter.x = heroInfo[id].pos.x+64;
	heroCenter.y = heroInfo[id].pos.y+64;
	for(i = 0; i < 2; i ++){
		TC_Position enemyCenter;
		enemyCenter = enemyInfo[i].pos;
		enemyCenter.x+=64;
		enemyCenter.y+=64;

		if(dist(heroCenter,enemyCenter)>10000)
			continue;

		if(abs(heroCenter.x - enemyCenter.x)>90)
			continue;
		
		if(abs(heroInfo[id].pos.y-enemyInfo[i].pos.y<30))
			ans++;
	}
	return ans;
}

int EnemyAheadII(int id){

	int i,ec = 0;
	for(i = 0; i < 2; i++){
		if(dist(heroInfo[id].pos,enemyInfo[i].pos)>90000)
			continue;
		if(enemyInfo[i].abnormal_type != TC_SPELLED_BY_NONE)
			continue;
		if(enemyInfo[i].steps_before_next_snatch>=10)
			continue;

		myVector v1,v2;
		v1.x = enemyInfo[i].pos.x-heroInfo[id].pos.x;
		v1.y = enemyInfo[i].pos.y-heroInfo[id].pos.y;
		v2.x = -back*300;
		v2.y = 0;
// 		if(Angle(v2,v1)>pi/3 || Angle(v2,v1)<-pi/3)
// 			continue;
// 		ec++;
		if(Angle(v2,v1)<5*pi/12 && Angle(v2,v1)>-pi/4 && heroInfo[1-id].pos.y>heroInfo[id].pos.y)
			ec++;
		if(Angle(v2,v1)>-5*pi/12 && Angle(v2,v1)<pi/4 && heroInfo[1-id].pos.y<heroInfo[id].pos.y)
			ec++;
	}

	return ec;
}

bool ShouldPass(int id){
	if(heroInfo[id].b_ghostball == false)return false;
// 	if(EnemyAhead(id) > EnemyAhead(1-id))
// 		return true;

// 	if(DistFromGoal(id)<200)return false;
// 	if(DistFromGoal(id)>DistFromGoal(1-id)) return true;
	if(!ScoreField()){
		TC_Position heroCenter[2];
		int i;
		for(i = 0; i < 2; i++)
		{
			heroCenter[i] = heroInfo[i].pos;
			heroCenter[i].x+=64;
			heroCenter[i].y+=64;
		}

		if(abs(heroCenter[0].x-heroCenter[1].x)>250)return false;

		if(CountEnemySurround(id) > CountEnemySurround(1-id) )
			return true;
		if(CountInLine(id) == 2)
			return true;
	}
	else{

// 		if(CountInLine(id) == 2)
// 			return true;
	}

/*	if(abs(heroCenter[0].x-heroCenter[1].x)<30 )*/
	return false;
}

TC_Position GetPassTarget(int id){
	TC_Position ans;
	if(!ScoreField()){
		if(EnemyAhead(id)+EnemyAhead(1-id)>0){
			if(heroInfo[id].pos.y>heroInfo[1-id].pos.y)
			{
				ans.x = ghostInfo.pos.x;
				ans.y = ghostInfo.pos.y-TEAMWIDTH;
				if(ans.y<64)ans.y = 64;  //等待官方给说法
			}
			else
			{
				ans.x  = ghostInfo.pos.x;
				ans.y = ghostInfo.pos.y+TEAMWIDTH;
				if(ans.y>640)
					ans.y = 640;
			}
		}
		else{
			if(heroInfo[id].pos.y>heroInfo[1-id].pos.y)
			{
				ans.y = ghostInfo.pos.y-TEAMWIDTH;
				if(ans.y<64)ans.y = 64;  //等待官方给说法
			}
			else
			{
				ans.y = ghostInfo.pos.y+TEAMWIDTH;
				if(ans.y>640)
					ans.y = 640;
			}

			ans.x = ghostInfo.pos.x-back*TEAMWIDTH;

			if(back == -1)
			{
				if(ans.x>forbiddenArea.right.left-TC_BALL_WIDTH)
					ans.x = forbiddenArea.right.left-TC_BALL_WIDTH;
			}
			else
			{
				if(ans.x<forbiddenArea.left.right)
					ans.x = forbiddenArea.left.right;
			}
		}
	}
	else{
		//ghostInfo.pos.y>enemyGate.y_lower || ghostInfo.pos.y<enemyGate.y_lower-32
		if(ghostInfo.pos.y>enemyGate.y_lower)
			return upPass;
		else
			return downPass;
	}

// 	if(EnemyAheadII(id) == 0){
// 
// 		int temp;
// 		if(back == -1){
// 			temp = forbiddenArea.right.left-TC_BALL_WIDTH-ghostInfo.pos.x;
// 		}
// 		else
// 		{
// 			temp = ghostInfo.pos.x - forbiddenArea.left.right;
// 		}
// 		if(temp>TEAMWIDTH)
// 			temp = TEAMWIDTH;
// 		if(heroInfo[id].pos.y>heroInfo[1-id].pos.y)
// 		{
// 			ans.x = ghostInfo.pos.x-back*temp;
// 		}
// 		else
// 		{
// 			ans.x = ghostInfo.pos.x-back*temp;
// 		}
// 	}
	return ans;
}

int NearHero(TC_Position pos){ //找个近的英雄去抢
	TC_Position heroPos0,heroPos1;
	heroPos0 = heroInfo[0].pos;
	heroPos1 = heroInfo[1].pos;
	heroPos0.x += 64;
	heroPos0.y += 64;
	heroPos1.x += 64;
	heroPos1.y += 64;

// 	if(dist(heroPos0,pos) > 22500 && (heroPos0.x-myGate.x)*(heroPos0.x-pos.x)<0 && (heroPos1.x-myGate.x)*(heroPos1.x-pos.x)>0)
// 		return 0;
// 
// 	if(dist(heroPos1,pos) > 22500 && (heroPos1.x-myGate.x)*(heroPos1.x-pos.x)<0 && (heroPos0.x-myGate.x)*(heroPos0.x-pos.x)>0)
// 		return 1;


	if(dist(heroPos0,pos) > dist(heroPos1,pos))
		return 1;
	else
		return 0;
}

void GetCurrStat(){
	if(preInfo.score_self != gameInfo.score_self || preInfo.score_enemy !=gameInfo.score_enemy)
	{
		haveball = false;
	}

	if(heroInfo[0].b_ghostball || heroInfo[1].b_ghostball){
		haveball = true;
		ballTaker = heroInfo[0].b_ghostball?0:1;
	}
	if(enemyInfo[0].b_ghostball || enemyInfo[1].b_ghostball)
		haveball = false;

	if(count-dangerCount[0]>25)
	{
		danger[0] = false;
	}

	if(count-dangerCount[1]>25)
	{
		danger[1] = false;
	}

	if(enemyInfo[0].type == TC_HERO_RON || enemyInfo[0].type == TC_HERO_GINNY){
		if(preEnemyInfo[0].b_is_spelling == false && enemyInfo[0].b_is_spelling == true){
			if(!danger[0] && heroInfo[0].b_ghostball){
				danger[0] = true;
				dangerCount[0] = count;
			}
			if(!danger[1] && heroInfo[1].b_ghostball){
				danger[1] = true;
				dangerCount[1] = count;
			}
		}
	}

	if(enemyInfo[1].type == TC_HERO_RON || enemyInfo[1].type == TC_HERO_GINNY){

		if(preEnemyInfo[1].b_is_spelling == false && enemyInfo[1].b_is_spelling == true){
			if(!danger[0] && heroInfo[0].b_ghostball){
				danger[0] = true;
				dangerCount[0] = count;
			}
			if(!danger[1] && heroInfo[1].b_ghostball){
				danger[1] = true;
				dangerCount[1] = count;
			}
		}
	}

	if(enemyInfo[0].b_is_spelling == false && enemyInfo[1].b_is_spelling == false)
	{
		danger[0] = false;
		danger[1] = false;
	}

}

bool InLine(){
	TC_Position centerH0,centerH1,ballCenter;
	centerH0.x = heroInfo[0].pos.x;
	centerH0.y = heroInfo[0].pos.y;
	centerH1.x = heroInfo[1].pos.x;
	centerH1.y = heroInfo[1].pos.y;
	ballCenter.x = goldInfo.pos.x;
	ballCenter.y = goldInfo.pos.y;

	return centerH0.x>(ballCenter.x-50)
		&& centerH0.x<(ballCenter.x+50)
		&& centerH1.x>(ballCenter.x-50) 
		&& centerH1.x<(ballCenter.x+50);
}

TC_Position GetGoldUpper(){
	TC_Position ans;
	ans.x = goldInfo.pos.x;
	ans.y = goldInfo.pos.y-300;
	if(ans.y<0)
		ans.y+=640;
	return ans;
}

TC_Position GetGoldLower(){
	TC_Position ans;
	ans.x = goldInfo.pos.x;
	ans.y = goldInfo.pos.y+264;
	if(ans.y>640)
		ans.y-=640;
	return ans;
}

void GetGoldBall(){
	if(heroInfo[0].b_snatch_goldball)
		Tc_SnatchBall(heroHandle[1],TC_GOLD_BALL);
	if(heroInfo[1].b_snatch_goldball)
		Tc_SnatchBall(heroHandle[1],TC_GOLD_BALL);

	TC_Position up = GetGoldUpper();
	TC_Position low = GetGoldLower();

	if(!InLine()){

		MoveTo(0,up);
		MoveTo(1,low);
	}
	else
	{
		MoveTo(0,goldInfo.pos);
		MoveTo(1,goldInfo.pos);
	}
}

TC_Position GetThrowTarget(int id){//to be full filled
	TC_Position ans;

	if(heroInfo[id].pos.y>heroInfo[1-id].pos.y)
		ans = downThrow;
	else
		ans = upThrow;
	if(dist(ghostInfo.pos,ans)<250000)
		return ans;
	ans.x = ghostInfo.pos.x - back*505;
	ans.y = ghostInfo.pos.y;
	if(back == -1){
		if(ans.y <=forbiddenArea.right.top-TC_BALL_HEIGHT || ans.y >=forbiddenArea.right.bottom){
			if(ans.x>=mapRight-TC_BALL_WIDTH)
				ans.x = mapRight-TC_BALL_WIDTH-1;
		}
		else{
			if(ans.x>forbiddenArea.right.left-TC_BALL_WIDTH)
			{
				ans.x = forbiddenArea.right.left-TC_BALL_WIDTH-1;
			}
		}
	}
	if(back == 1)
	{
		if(ans.y <=forbiddenArea.left.top-TC_BALL_HEIGHT || ans.y >=forbiddenArea.left.bottom)
		{
			if(ans.x <= mapLeft)
				ans.x = mapLeft+1;
		}
		else
		{
			if(ans.x <=forbiddenArea.left.right)
				ans.x = forbiddenArea.left.right+1;
		}
	}
	if(ans.y<64)ans.y = 64;//等待官方给说法
	return ans;
}

bool Between(TC_Position pp,TC_Position p1,TC_Position p2){

	return (pp.x-p1.x)*(pp.x-p2.x)<0;
}

int EnemyAhead(int id){//判断是否该向前扔球，即扔出球后球是否有可能被对方拿到
	int i,ec = 0;
	for(i = 0; i < 2; i++){
		if(dist(heroInfo[id].pos,enemyInfo[i].pos)>1000000)
			continue;
		if(enemyInfo[i].abnormal_type != TC_SPELLED_BY_NONE && dist(enemyInfo[i].pos,heroInfo[id].pos)<40000)//可以无视的穿过
			continue;

		myVector v1,v2;
		v1.x = enemyInfo[i].pos.x-heroInfo[id].pos.x;
		v1.y = enemyInfo[i].pos.y-heroInfo[id].pos.y;
		v2.x = -back*300;
		v2.y = 0;
		if(Angle(v2,v1)>pi/4 || Angle(v2,v1)<-pi/4)
			continue;
		ec++;
	}

	return ec;
}

bool InMap(TC_Position pos){
	if(pos.y<0 || pos.y>640) return false;
	if(pos.y<gateUpper || pos.y>gateLower){
		if(pos.x<mapLeft || pos.x>mapRight-128)
			return false;
	}

	if(GhostInAir() && (pos.x<mapLeft || pos.x>mapRight-128)){
		return false;
	}
	return true;
}

bool ShouldSpell(){ //origin ShouldSpell fit for ron and ginny
	//enemy got ball
	if(!enemyInfo[0].b_ghostball &&!enemyInfo[1].b_ghostball)
		return false;

	if(heroInfo[0].b_snatch_ghostball || heroInfo[1].b_snatch_ghostball)//抢了先
		return false;

	int ballEnemy = enemyInfo[0].b_ghostball?0:1;
	//status well

	TC_Position enemy = enemyInfo[ballEnemy].pos;
	enemy.x+=64;
	enemy.y+=64;

	if(back==1){
		if(myGate.x-enemy.x>SPELLDIST)
			return false;
		if(myGate.x-enemy.x<10)//太迟了，不要枉费魔法
			return false;
	}
	if(back==-1){
		if(enemy.x-myGate.x>SPELLDIST)
			return false;
		if(enemy.x-myGate.x<10)//太迟了，不要枉费魔法
			return false;
	}

	if(enemyInfo[ballEnemy].abnormal_type != TC_SPELLED_BY_NONE)
		return false;

	//not being spelled
	if(spellEnemy == ballEnemy)
		return false;

	//take him out
	spellEnemy = ballEnemy;
	return true;
}

// bool ShouldSpell(){//malfoy and hermino 对一个英雄施暴 留一个玩。
// 
// 	if(!havaBall)return false;
// 
// 	if(heroInfo[0].b_is_spelling || heroInfo[1].b_is_spelling){
// 		return false;
// 	}
// 	
// 	if(enemyInfo[0].abnormal_type != TC_SPELLED_BY_NONE || enemyInfo[0].abnormal_type != TC_SPELLED_BY_NONE)//是否考虑spelling状态
// 	{
// 		return false;
// 	}
// 
// 	int maxx = 0x7fffffff;
// 	int suckerId = -1;
// 	int i;
// 
// 	for(i = 0; i < 2; i++)
// 	{
// 		if(enemyInfo[i].abnormal_type != TC_SPELLED_BY_NONE)
// 		{
// 			continue;
// 		}
// 		if(dist(enemyInfo[i].pos,ghostInfo.pos)<maxx)
// 		{
// 			suckerId = i;
// 			maxx = dist(enemyInfo[i].pos,ghostInfo.pos);
// 		}
// 	}
// 	spellEnemy = suckerId;
// 	return true;
// }


bool CanSpell(int id,int enemyId){
	//自身条件符合
	if(!heroInfo[id].b_can_spell)return false;
	if(heroInfo[id].abnormal_type != TC_SPELLED_BY_NONE)return false;
	//客观条件成熟
	if(Tc_CanBeSpelled(heroHandle[id],enemyHandle[enemyId]))
		return true;
	else
		return false;
}

int ChooseSpeller(){
	int ans = -1,countSpeller,i;
	countSpeller = 0;
	for(i = 0; i < 2; i++)
		if(CanSpell(i,spellEnemy))
			countSpeller++;
	if(countSpeller == 1)
	{
		if(CanSpell(0,spellEnemy))
			ans = 0;
		else
			ans = 1;
	}
	if(countSpeller == 2){
		return 0;
	}
	return ans;
}

bool ShouldSpell(int id){  

	TC_Position ball,myHero,enemy1,enemy2;
	ball = ghostInfo.pos;
	ball.x+=32;
	ball.y+=32;
	if(id == NearHero(ball))return false;
	myHero = heroInfo[1-id].pos;
	enemy1 = enemyInfo[0].pos;
	enemy2 = enemyInfo[1].pos;
	enemy1.x+=64;
	enemy2.x+=64;
	enemy1.y+=64;
	enemy2.y+=64;
	myHero.x+=64;
	myHero.y+=64;

	if(dist(myHero,ball)>400)return false;
	if(dist(enemy1,enemy2)>900)return false;
	if(dist(enemy1,ball)<400 || dist(enemy2,ball)<400)return true;
	else
		return false;
}


bool ShouldGetGold(int id){
	if(id == 5){//先不组织团队的抢球
		if(goldInfo.b_visible == false)
			return false;
		if(heroInfo[0].b_ghostball || heroInfo[1].b_ghostball)
			return false;
		if(gameInfo.score_self+10<gameInfo.score_enemy)
			return false;
		return false;
	}
	if(dist(heroInfo[id].pos,goldInfo.pos) <6400)
		return true;
	else
		return false;
}



bool NotOverForbid(TC_Ball ball){ //是否未进入禁区

	TC_Position ballPosition = ball.pos;
	ballPosition.x+=32;
	ballPosition.y+=32;

	if(abs(enemyGate.x-ballPosition.x)>400)
		return true;
	else
		return false;
}

bool Throwing(){  //传球过程

	if(throwed<=0)return false;
	if(ghostInfo.u.target.x==-1 || ghostInfo.u.target.x == ghostInfo.pos.x || (ghostInfo.speed.vx==0 &&ghostInfo.speed.vy==0))
		return false;
	if(enemyInfo[0].b_ghostball || enemyInfo[1].b_ghostball)return false;

	return true;
}
bool ShouldThrow(int id){
	TC_Hero hero= heroInfo[id];
	if(hero.b_ghostball==false)return false;
	if(EnemyAhead(id))
		return false;

	if(NotOverForbid(ghostInfo))
		return true;
	else
		return false;
}

int sqr(int num)
{
	return num*num;
}

void GetCurrInfo(){
	TC_Team_Member team_member;
	team_member = TC_MEMBER1;
	heroHandle[0] = Tc_GetHeroHandle(team_member);
	enemyHandle[0] = Tc_GetEnemyHandle(team_member);
	team_member = TC_MEMBER2;
	heroHandle[1] = Tc_GetHeroHandle(team_member);
	enemyHandle[1] = Tc_GetEnemyHandle(team_member);
	//获得四个人的信息
	Tc_GetHeroInfo(heroHandle[0],&heroInfo[0]);
	Tc_GetHeroInfo(heroHandle[1],&heroInfo[1]);
	Tc_GetEnemyInfo(enemyHandle[0],&enemyInfo[0]);
	Tc_GetEnemyInfo(enemyHandle[1],&enemyInfo[1]);

	TC_Ball_Type ball_type = TC_FREE_BALL;
	freeBall = Tc_GetBallHandle(ball_type);
	ball_type = TC_GHOST_BALL;
	ghostBall = Tc_GetBallHandle(ball_type);
	ball_type = TC_GOLD_BALL;
	goldBall = Tc_GetBallHandle(ball_type);
	//得到三种球的信息
	Tc_GetBallInfo(freeBall,&freeInfo);
	Tc_GetBallInfo(ghostBall,&ghostInfo);
	Tc_GetBallInfo(goldBall,&goldInfo);

	//比赛和球门的信息
	Tc_GetGameInfo(heroHandle[0],&gameInfo);
	Tc_GetOwnGate(heroHandle[0],&myGate);
	Tc_GetEnemyGate(enemyHandle[0],&enemyGate);
	Tc_GetForbiddenArea(&forbiddenArea);

	gateUpper = myGate.y_upper+STEPLENGTH;
	gateLower = myGate.y_lower-128-STEPLENGTH;
	mid= (gateUpper + gateLower)/2;


	if(myGate.x<enemyGate.x)
	{
		back = -1;
		mapLeft = myGate.x;
		mapRight = enemyGate.x;
		upThrow.y = forbiddenArea.right.top-TC_BALL_HEIGHT;
		upThrow.x = forbiddenArea.right.right-TC_BALL_WIDTH-32;
		downThrow.x = forbiddenArea.right.right-TC_BALL_WIDTH-32;
		downThrow.y = forbiddenArea.right.bottom;

		upPass.x = forbiddenArea.right.left-TC_BALL_WIDTH;
		upPass.y = gateUpper;
		downPass.x = upPass.x;
		downPass.y = gateLower;
// 		upThrow.y = enemyGate.y_upper;
// 		upThrow.x = forbiddenArea.right.left-TC_BALL_WIDTH;
// 		downThrow.x = forbiddenArea.right.left-TC_BALL_WIDTH;
// 		downThrow.y = enemyGate.y_lower-TC_HERO_HEIGHT;
	}
	else{
		back = 1;
		mapLeft = enemyGate.x;
		mapRight = myGate.x;
		upThrow.y = forbiddenArea.left.top-TC_BALL_HEIGHT;
		upThrow.x = forbiddenArea.left.left+1;
		downThrow.x = forbiddenArea.left.left+1;
		downThrow.y = forbiddenArea.left.bottom;

		upPass.x = forbiddenArea.left.right;
		downPass.x = upPass.x;
		upPass.y = gateUpper;
		downPass.x = gateLower;
// 		upThrow.y = enemyGate.y_upper;
// 		upThrow.x = forbiddenArea.left.right+32;
// 		downThrow.x = forbiddenArea.left.right+32;
// 		downThrow.y = enemyGate.y_lower;
	}
}

int CountEnemySurround(int id){//useless
	TC_Position heroCenter = heroInfo[id].pos;
	heroCenter.x+=64;
	heroCenter.y+=64;
	int count = 0,i;
	for(i = 0; i < 2; i++)
	{
		TC_Position enemyCenter;
		enemyCenter = enemyInfo[i].pos;
		enemyCenter.x += 64;
		enemyCenter.y += 64;
		if((dist(heroCenter,enemyCenter)<8100 && abs(enemyCenter.y-heroCenter.y)<65) || dist(heroCenter,enemyCenter)<6400)
			count++;
	}
// 	if(	dist(hero.pos,enemyInfo[0].pos)<40000 && abs(enemyInfo[0].pos.y-heroInfo[id].pos.y)<30 || dist(hero.pos,enemyInfo[0].pos)<6400)
// 		count++;
// 	if(	dist(hero.pos,enemyInfo[1].pos)<40000 && abs(enemyInfo[1].pos.y-heroInfo[id].pos.y)<30 || dist(hero.pos,enemyInfo[0].pos)<6400) 
//		count++;

	return count;
}

TC_Position GetSaveWay(TC_Position pOrign){ //从pOrigin到dest，找到一个能避开freeball的点
	TC_Position ans,temp1,temp2;
	TC_Path ballRoad;
	int i = 0;
	while(i<30){
		i++;
		if((sqr(freeInfo.speed.vx*i)+sqr(freeInfo.speed.vy*i))>40000)
			break;
	}
	TC_Position p,pp;
	p.x = freeInfo.pos.x+i*freeInfo.speed.vx;
	p.y = freeInfo.pos.y+i*freeInfo.speed.vy;
	pp.x = freeInfo.pos.x+5*freeInfo.speed.vx;
	pp.y = freeInfo.pos.y+5*freeInfo.speed.vy;
	ballRoad.pos_end = pp;
	ballRoad.pos_start = freeInfo.pos;

	double r = Relation(pOrign,ballRoad);
	//double length = sqrt(double(dist(freeInfo.u.path.pos_end,freeInfo.u.path.pos_start)));
	if(r>1){
		temp1 = Rotate(p,freeInfo.pos,pi/2);
		temp2 = Rotate(p,freeInfo.pos,pi*3/2);
		if(dist(temp1,pOrign)>dist(temp2,pOrign))
			ans = temp2;
		else 
			ans = temp1;
	}
	else{
		if(r<-esp){
			temp1 = Rotate(freeInfo.pos,p,pi/2);
			temp2 = Rotate(freeInfo.pos,p,pi*3/2);
			if(dist(temp1,pOrign)>dist(temp2,pOrign))
				ans = temp2;
			else 
				ans = temp1;
		}
		else{
			temp1 = Rotate(p,freeInfo.pos,pi*3/2);
			temp2 = Rotate(freeInfo.pos,p,pi/2);
			TC_Position temp3,temp4;
			temp3 = Rotate(p,freeInfo.pos,pi/2);
			temp4 = Rotate(freeInfo.pos,p,pi*3/2);
			temp1.x = (temp1.x+temp2.x)/2;
			temp1.y = (temp1.y+temp2.y)/2;
			temp3.x = (temp3.x+temp4.x)/2;
			temp3.y = (temp3.y+temp4.y)/2;

			if(dist(temp1,pOrign)>dist(temp3,pOrign))
				ans = temp3;
			else
				ans = temp1;
		}
	}

	return ans;
}

int dist(TC_Position p1,TC_Position p2){
	return (p1.x-p2.x)*(p1.x-p2.x) + (p1.y-p2.y)*(p1.y-p2.y)+1;
}

TC_Position GetDest(int id){ //根据当前的状态得到跑位的目标

	TC_Hero hero = heroInfo[id];
	TC_Position dest;
	dest.x = 0;
	dest.y = 0;


	if(dist(heroInfo[id].pos,heroInfo[1-id].pos)<170&&heroInfo[id].b_ghostball && danger[id] && DistFromGoal(id)>60&&(heroInfo[1-id].abnormal_type == TC_SPELLED_BY_NONE||heroInfo[1-id].abnormal_type == TC_SPELLED_BY_VOLDEMORT))
	{
		dest = heroInfo[1-id].pos;
		return dest;
	}

	if(spellEnemy != -1){
		curWork = 0;
// 		if(speller == 0){
// 			if(dist(heroInfo[id].pos,ghostInfo.pos)<100000)
// 				dest = ScorePot();
// 		}
//		else{
			if(dist(heroInfo[id].pos,ghostInfo.pos)<100000){
				dest.y = heroInfo[id].pos.y;
				dest.x = (myGate.x+enemyGate.x)/2;
			}
//		}
		if(dest.x != 0 && dest.y!=0)
			return dest;
	}

	if(GhostInAir() && InDoor(id)){

		if(heroInfo[id].pos.x>=mapRight-TC_HERO_WIDTH){
			dest = heroInfo[id].pos;
			dest.x-=100;
		}
		else
		{
			dest = heroInfo[id].pos;
			dest.x +=100;
		}

		return dest;
	}

	TC_Hero company = heroInfo[1-id];
	if(hero.b_ghostball){
		curWork = 1;//得分阶段
		if(ScoreField()){
			dest = ScorePot();
			if(hero.pos.y>=gateUpper && hero.pos.y<=gateLower)
				dest.y =hero.pos.y;
			return dest;
		}
		else
		{
			dest.x = enemyGate.x;
			dest.y = hero.pos.y;
			return dest;
		}
	}

	if(company.b_ghostball){
		curWork = 1;
		if(company.abnormal_type == TC_SPELLED_BY_NONE &&!danger[1-id]){
			if(ScoreField())
			{
				dest = company.pos;
			}
			else
			{
				dest.x = company.pos.x;
				if(heroInfo[id].pos.y>company.pos.y)
				{
					dest.y = company.pos.y+TEAMWIDTH;
					if(company.pos.y>440) //本路太狭窄了，换路吧
						dest.y = company.pos.y-TEAMWIDTH;
				}
				else
				{
					dest.y = company.pos.y-TEAMWIDTH;
					if(company.pos.y<200)//换路
						dest.y = company.pos.y+TEAMWIDTH;
				}

			}
		}
		else
		{
			dest = ghostInfo.pos;
			dest.y-=TC_BALL_HEIGHT/2;
		}

		return dest;
	}

	//如果对方持球
	if(!haveball)
	{
		curWork = 0;
		TC_Position heropos,ballPos;
		heropos = heroInfo[id].pos;
		ballPos = ghostInfo.pos;
		heropos.x+=64;
		heropos.y+=64;
		ballPos.x+=32;
		ballPos.y+=32;

		if(EnemyScoreField() || ScoreField())
		{
			dest = ghostInfo.pos;
			dest.y-=32;
			return dest;
		}


// 		if(dist(heropos,ballPos)>TEAMWIDTH*TEAMWIDTH)
// 			dest = ghostInfo.pos;
// 		else
// 		{
		if(!OK(0) || !OK(1)){
			if(enemyInfo[0].b_ghostball || enemyInfo[1].b_ghostball)
				dest = enemyInfo[0].b_ghostball?enemyInfo[0].pos:enemyInfo[1].pos;
			else{
				dest = ballPos;
				dest.x -= 64;
				dest.y -= 64;
			}
			return dest;
		}
		if(id == NearHero(ballPos))
		{
			if(enemyInfo[0].b_ghostball || enemyInfo[1].b_ghostball)
				dest = enemyInfo[0].b_ghostball?enemyInfo[0].pos:enemyInfo[1].pos;
			else{
				dest = ballPos;
				dest.x -= 64;
				dest.y -= 64;
			}
			return dest;
		}
		dest.x = company.pos.x;
		if(hero.pos.y>company.pos.y){
			dest.y = company.pos.y+TEAMWIDTH;
			if(company.pos.y<400)return dest;
			else
			{
				dest.y = company.pos.y-TEAMWIDTH;
				return dest;
			}
		}
		else
		{
			dest.y = company.pos.y-TEAMWIDTH;
			if(company.pos.y>250) return dest;
			else
			{
				dest.y = company.pos.y+TEAMWIDTH;
				return dest;
			}
		}
	}
	if(haveball)
	{
		if(ScoreField())
		{
			dest = ghostInfo.pos;
			return dest;
		}
		if(id == ballTaker){
			if(ghostInfo.u.target.x!=-1)
			{
				dest = ghostInfo.u.target;
				dest.y -= 32;
				if(abs(enemyGate.x-hero.pos.x-64)<100 && hero.pos.y>gateUpper)
					dest.y-=8;
			}
			else
			{
				dest = ghostInfo.pos;
				dest.y-=32;
				if(abs(enemyGate.x-hero.pos.x-64)<100 && hero.pos.y>gateUpper)
					dest.y-=8;
			}
		}
		else
		{
			dest.x = heroInfo[ballTaker].pos.x;
			if(heroInfo[id].pos.y>heroInfo[ballTaker].pos.y)
			{
				dest.y = heroInfo[ballTaker].pos.y+TEAMWIDTH;
				if(dest.y>640)dest.y = 640;
				if(heroInfo[ballTaker].pos.y>440)
					dest.y = heroInfo[ballTaker].pos.y-TEAMWIDTH;
			}
			else
			{
				dest.y = heroInfo[ballTaker].pos.y-TEAMWIDTH;
				if(dest.y<0)dest.y = 0;
				if(heroInfo[ballTaker].pos.y<200)
					dest.y = heroInfo[ballTaker].pos.y+TEAMWIDTH;
			}
		}
	}

	return dest;
}


TC_Position ScorePot(int type){
	TC_Position ans;
	if(enemyGate.x>myGate.x)
	{
		ans.x = enemyGate.x+20;
		//ans.y = (enemyGate.y_upper+enemyGate.y_lower)/2-80;
	}
	else
	{
		ans.x = enemyGate.x-150;
	}
	if(type == 0)
		ans.y = (gateLower+gateUpper)/2;
	if(type==1)
		ans.y = gateUpper;
	if(type == 2)
		ans.y = gateLower;
	return ans;
}
bool GhostInAir(){
	return !heroInfo[0].b_ghostball &&
		!heroInfo[1].b_ghostball &&
		!enemyInfo[0].b_ghostball &&
		!enemyInfo[1].b_ghostball;
}

TC_Position NextPosition(TC_Position p,TC_Direction dir){
	TC_Position ans = p;
	if(dir == TC_DIRECTION_BOTTOM){
		ans.x = p.x;
		ans.y = p.y+VY;
	}
	if(dir == TC_DIRECTION_LEFT){
		ans.x = p.x-VX;
		ans.y = p.y;
	}
	if(dir == TC_DIRECTION_RIGHT){
		ans.x = p.x+VX;
		ans.y = p.y;
	}
	if(dir == TC_DIRECTION_TOP){
		ans.x = p.x;
		ans.y = p.y-VY;
	}
	if(dir == TC_DIRECTION_LEFTBOTTOM){
		ans.x = p.x-VXY;
		ans.y = p.y+VXY;
	}
	if(dir == TC_DIRECTION_RIGHTBOTTOM)
	{
		ans.x = p.x+VXY;
		ans.y = p.y+VXY;
	}
	if(dir == TC_DIRECTION_LEFTTOP){
		ans.x = p.x-VXY;
		ans.y = p.y-VXY;
	}
	if(dir == TC_DIRECTION_RIGHTTOP){
		ans.x = p.x+VXY;
		ans.y = p.y-VXY;
	}
	return ans;
}

bool TooClose(TC_Position p1,TC_Position p2,int dis){ //p1是人物的坐标，p2是球的坐标

	p1.x+=TC_HERO_WIDTH/2;
	p1.y+=TC_HERO_HEIGHT/2;
	p2.x+=TC_BALL_WIDTH/2;
	p2.y+=TC_BALL_HEIGHT/2;
	int dist = int(sqrt(double((p1.x-p2.x)*(p1.x-p2.x) +(p1.y-p2.y)*(p1.y-p2.y))));
	return dist<=dis;
}

void MoveTo(int id,TC_Position dest){
	TC_Hero hero = heroInfo[id];

	if(SuperPosition(hero.pos,dest))
	{
		Tc_Stop(heroHandle[id]);
	}
	else{
		TC_Direction dir= PosRelative(hero.pos,dest);
		TC_Position nextHero = NextPosition(hero.pos,dir);
		TC_Position nextFree;
		nextFree.x = freeInfo.pos.x+freeInfo.speed.vx;
		nextFree.y = freeInfo.pos.y+freeInfo.speed.vy;
		if(/*dist(nextHero,nextFree)<dist(hero.pos,nextFree) && */TooClose(nextHero,nextFree) && freeInfo.b_visible && hero.abnormal_type==TC_SPELLED_BY_NONE)
		{

			TC_Position destNow;           //路上有自由球阻挡，找个好地方避避
			TC_Direction dirNow;
			TC_Position nextHeroNow;
			bool found = false;
			if(curWork == 1){
				int i;
				for(i = 1; i <= 2; i++){
					destNow = ScorePot(i);
					dirNow = PosRelative(hero.pos,destNow);
					nextHeroNow = NextPosition(hero.pos,dirNow);
					if(dist(nextHeroNow,nextFree)>dist(nextHero,nextFree)){
						dest = destNow;
						dir = dirNow;
						found = true;
						break;
					}
				}
			}
			if(!found){
				destNow = GetSaveWay(hero.pos);           //路上有自由球阻挡，找个好地方避避
				dirNow = PosRelative(hero.pos,destNow);
				nextHeroNow = NextPosition(hero.pos,dirNow);
				if(dist(nextHeroNow,nextFree)>dist(nextHero,nextFree)){
					dest = destNow;
					dir = dirNow;
				}
			}
		}

		Tc_Move(heroHandle[id],dir);
	}
}

bool SuperPosition(TC_Position p1,TC_Position p2){ //为了避免人物晃动
	int difx = abs(p1.x-p2.x);
	int dify = abs(p1.y-p2.y);

	return difx<STEPLENGTH && dify<STEPLENGTH;
}

TC_Direction PosRelative(TC_Position pOrign,TC_Position dest){//判断要到达目标点这一步要走的方向

	srand(unsigned int(time(NULL)));
	int num = rand();
	num = abs(num);
	//bool foundDir =false;
	TC_Position freePos,heroPos;
	heroPos.x = 0;
	heroPos.y = 0;
	freePos.x = freeInfo.pos.x + freeInfo.speed.vx;
	freePos.y = freeInfo.pos.y + freeInfo.speed.vy;
	//带安全检测
	if(num%2){   //随机的先判x或者先走y
		if(pOrign.x+STEPLENGTH <dest.x){
			if(pOrign.y+STEPLENGTH < dest.y){
				heroPos = NextPosition(pOrign,TC_DIRECTION_RIGHTBOTTOM);
				if(!TooClose(heroPos,freePos,FLEEDIST2) && InMap(heroPos))
					return TC_DIRECTION_RIGHTBOTTOM;
			}
			if(pOrign.y-STEPLENGTH > dest.y){
				heroPos = NextPosition(pOrign,TC_DIRECTION_RIGHTTOP);
				if(!TooClose(heroPos,freePos,FLEEDIST2) && InMap(heroPos))
					return TC_DIRECTION_RIGHTTOP;
			}

			heroPos = NextPosition(pOrign,TC_DIRECTION_RIGHT);
			if(!TooClose(heroPos,freePos,FLEEDIST2) && InMap(heroPos))
			{   
				return TC_DIRECTION_RIGHT;
			}

		}
		if(pOrign.x-STEPLENGTH > dest.x){
			if(pOrign.y+STEPLENGTH < dest.y){
				heroPos = NextPosition(pOrign,TC_DIRECTION_LEFTBOTTOM);
				if(!TooClose(heroPos,freePos,FLEEDIST2) && InMap(heroPos))
					return TC_DIRECTION_LEFTBOTTOM;
			}
			if(pOrign.y-STEPLENGTH >dest.y ){
				heroPos = NextPosition(pOrign,TC_DIRECTION_LEFTTOP);
				if(!TooClose(heroPos,freePos,FLEEDIST2) && InMap(heroPos)){
					return TC_DIRECTION_LEFTTOP;
				}
			}
			heroPos = NextPosition(pOrign,TC_DIRECTION_LEFT);
			if(!TooClose(heroPos,freePos,FLEEDIST2) && InMap(heroPos))
				return TC_DIRECTION_LEFT;
		}

		if(pOrign.y+STEPLENGTH < dest.y)
		{
			if(pOrign.x+STEPLENGTH < dest.x){
				heroPos = NextPosition(pOrign,TC_DIRECTION_RIGHTBOTTOM);
				if(!TooClose(heroPos,freePos,FLEEDIST2) && InMap(heroPos))
					return TC_DIRECTION_RIGHTBOTTOM;
			}
			if(pOrign.x-STEPLENGTH >dest.x){
				heroPos = NextPosition(pOrign,TC_DIRECTION_LEFTBOTTOM);
				if(!TooClose(heroPos,freePos,FLEEDIST2) && InMap(heroPos))
					return TC_DIRECTION_LEFTBOTTOM;
			}

			heroPos = NextPosition(pOrign,TC_DIRECTION_BOTTOM);
			if(!TooClose(heroPos,freePos,FLEEDIST2) && InMap(heroPos))
				return TC_DIRECTION_BOTTOM;
		}

		if(pOrign.y - STEPLENGTH > dest.y){
			if(pOrign.x+STEPLENGTH < dest.x){
				heroPos = NextPosition(pOrign,TC_DIRECTION_RIGHTTOP);
				if(!TooClose(heroPos,freePos,FLEEDIST2) && InMap(heroPos))
					return TC_DIRECTION_RIGHTTOP;
			}
			if(pOrign.x-STEPLENGTH > dest.x){
				heroPos = NextPosition(pOrign,TC_DIRECTION_LEFTTOP);
				if(!TooClose(heroPos,freePos,FLEEDIST2) && InMap(heroPos))
					return TC_DIRECTION_LEFTTOP;
			}
			heroPos = NextPosition(pOrign,TC_DIRECTION_TOP);
			if(!TooClose(heroPos,freePos,FLEEDIST2) && InMap(heroPos))
				return TC_DIRECTION_TOP;
		}
	}
	else
	{
		if(pOrign.y+STEPLENGTH < dest.y)
		{
			if(pOrign.x+STEPLENGTH < dest.x){
				heroPos = NextPosition(pOrign,TC_DIRECTION_RIGHTBOTTOM);
				if(!TooClose(heroPos,freePos,FLEEDIST2) && InMap(heroPos))
					return TC_DIRECTION_RIGHTBOTTOM;
			}
			if(pOrign.x-STEPLENGTH >dest.x){
				heroPos = NextPosition(pOrign,TC_DIRECTION_LEFTBOTTOM);
				if(!TooClose(heroPos,freePos,FLEEDIST2) && InMap(heroPos))
					return TC_DIRECTION_LEFTBOTTOM;
			}

			heroPos = NextPosition(pOrign,TC_DIRECTION_BOTTOM);
			if(!TooClose(heroPos,freePos,FLEEDIST2) && InMap(heroPos))
				return TC_DIRECTION_BOTTOM;
		}

		if(pOrign.y - STEPLENGTH > dest.y){
			if(pOrign.x+STEPLENGTH < dest.x){
				heroPos = NextPosition(pOrign,TC_DIRECTION_RIGHTTOP);
				if(!TooClose(heroPos,freePos,FLEEDIST2) && InMap(heroPos))
					return TC_DIRECTION_RIGHTTOP;
			}
			if(pOrign.x-STEPLENGTH > dest.x){
				heroPos = NextPosition(pOrign,TC_DIRECTION_LEFTTOP);
				if(!TooClose(heroPos,freePos,FLEEDIST2) && InMap(heroPos))
					return TC_DIRECTION_LEFTTOP;
			}
			heroPos = NextPosition(pOrign,TC_DIRECTION_TOP);
			if(!TooClose(heroPos,freePos,FLEEDIST2) && InMap(heroPos))
				return TC_DIRECTION_TOP;
		}

		if(pOrign.x+STEPLENGTH <dest.x){
			if(pOrign.y+STEPLENGTH < dest.y){
				heroPos = NextPosition(pOrign,TC_DIRECTION_RIGHTBOTTOM);
				if(!TooClose(heroPos,freePos,FLEEDIST2) && InMap(heroPos))
					return TC_DIRECTION_RIGHTBOTTOM;
			}
			if(pOrign.y-STEPLENGTH > dest.y){
				heroPos = NextPosition(pOrign,TC_DIRECTION_RIGHTTOP);
				if(!TooClose(heroPos,freePos,FLEEDIST2) && InMap(heroPos))
					return TC_DIRECTION_RIGHTTOP;
			}

			heroPos = NextPosition(pOrign,TC_DIRECTION_RIGHT);
			if(!TooClose(heroPos,freePos,FLEEDIST2) && InMap(heroPos)){
				return TC_DIRECTION_RIGHT;
			}

		}
		if(pOrign.x-STEPLENGTH > dest.x){
			if(pOrign.y+STEPLENGTH < dest.y){
				heroPos = NextPosition(pOrign,TC_DIRECTION_LEFTBOTTOM);
				if(!TooClose(heroPos,freePos,FLEEDIST2) && InMap(heroPos))
					return TC_DIRECTION_LEFTBOTTOM;
			}
			if(pOrign.y-STEPLENGTH >dest.y ){
				heroPos = NextPosition(pOrign,TC_DIRECTION_LEFTTOP);
				if(!TooClose(heroPos,freePos,FLEEDIST2) && InMap(heroPos))
				{
					return TC_DIRECTION_LEFTTOP;
				}
			}
			heroPos = NextPosition(pOrign,TC_DIRECTION_LEFT);
			if(!TooClose(heroPos,freePos,FLEEDIST2) && InMap(heroPos))
				return TC_DIRECTION_LEFT;
		}
	}

	//不带安全检测
	if(num%2){   //随机的先走x或者先走y
		if(pOrign.x+STEPLENGTH <dest.x){
			if(pOrign.y+STEPLENGTH < dest.y){
				heroPos = NextPosition(pOrign,TC_DIRECTION_RIGHTBOTTOM);
				if(InMap(heroPos))
					return TC_DIRECTION_RIGHTBOTTOM;
			}
			if(pOrign.y-STEPLENGTH > dest.y){
				heroPos = NextPosition(pOrign,TC_DIRECTION_RIGHTTOP);
				if(InMap(heroPos))
					return TC_DIRECTION_RIGHTTOP;
			}

			heroPos = NextPosition(pOrign,TC_DIRECTION_RIGHT);
			if(InMap(heroPos))
				return TC_DIRECTION_RIGHT;

		}
		if(pOrign.x-STEPLENGTH > dest.x){
			if(pOrign.y+STEPLENGTH < dest.y){
				heroPos = NextPosition(pOrign,TC_DIRECTION_LEFTBOTTOM);
				if(InMap(heroPos))
					return TC_DIRECTION_LEFTBOTTOM;
			}
			if(pOrign.y-STEPLENGTH >dest.y ){
				heroPos = NextPosition(pOrign,TC_DIRECTION_LEFTTOP);
				if(InMap(heroPos))
					return TC_DIRECTION_LEFTTOP;
			}
			heroPos = NextPosition(pOrign,TC_DIRECTION_LEFT);
			if(InMap(heroPos))
				return TC_DIRECTION_LEFT;
		}

		if(pOrign.y+STEPLENGTH < dest.y)
		{
			if(pOrign.x+STEPLENGTH < dest.x){
				heroPos = NextPosition(pOrign,TC_DIRECTION_RIGHTBOTTOM);
				if(InMap(heroPos))
					return TC_DIRECTION_RIGHTBOTTOM;
			}
			if(pOrign.x-STEPLENGTH >dest.x){
				heroPos = NextPosition(pOrign,TC_DIRECTION_LEFTBOTTOM);
				if(InMap(heroPos))
					return TC_DIRECTION_LEFTBOTTOM;
			}

			heroPos = NextPosition(pOrign,TC_DIRECTION_BOTTOM);
			if(InMap(heroPos))
				return TC_DIRECTION_BOTTOM;
		}

		if(pOrign.y - STEPLENGTH > dest.y){
			if(pOrign.x+STEPLENGTH < dest.x){
				heroPos = NextPosition(pOrign,TC_DIRECTION_RIGHTTOP);
				if(InMap(heroPos))
					return TC_DIRECTION_RIGHTTOP;
			}
			if(pOrign.x-STEPLENGTH > dest.x){
				heroPos = NextPosition(pOrign,TC_DIRECTION_LEFTTOP);
				if(InMap(heroPos))
					return TC_DIRECTION_LEFTTOP;
			}
			
			heroPos = NextPosition(pOrign,TC_DIRECTION_TOP);
			if(InMap(heroPos))
				return TC_DIRECTION_TOP;
		}
	}
	else
	{
		if(pOrign.y+STEPLENGTH < dest.y)
		{
			if(pOrign.x+STEPLENGTH < dest.x){
				heroPos = NextPosition(pOrign,TC_DIRECTION_RIGHTBOTTOM);
				if(InMap(heroPos))
					return TC_DIRECTION_RIGHTBOTTOM;
			}
			if(pOrign.x-STEPLENGTH >dest.x){
				heroPos = NextPosition(pOrign,TC_DIRECTION_LEFTBOTTOM);
				if(InMap(heroPos))
					return TC_DIRECTION_LEFTBOTTOM;
			}

			heroPos = NextPosition(pOrign,TC_DIRECTION_BOTTOM);
			if(InMap(heroPos))
				return TC_DIRECTION_BOTTOM;
		}

		if(pOrign.y - STEPLENGTH > dest.y){
			if(pOrign.x+STEPLENGTH < dest.x){
				heroPos = NextPosition(pOrign,TC_DIRECTION_RIGHTTOP);
				if(InMap(heroPos))
					return TC_DIRECTION_RIGHTTOP;
			}
			if(pOrign.x-STEPLENGTH > dest.x){
				heroPos = NextPosition(pOrign,TC_DIRECTION_LEFTTOP);
				if(InMap(heroPos))
					return TC_DIRECTION_LEFTTOP;
			}

			heroPos = NextPosition(pOrign,TC_DIRECTION_TOP);
			if(InMap(heroPos))
				return TC_DIRECTION_TOP;
		}

		if(pOrign.x+STEPLENGTH <dest.x){
			if(pOrign.y+STEPLENGTH < dest.y){
				heroPos = NextPosition(pOrign,TC_DIRECTION_RIGHTBOTTOM);
				if(InMap(heroPos))
					return TC_DIRECTION_RIGHTBOTTOM;
			}
			if(pOrign.y-STEPLENGTH > dest.y){
				heroPos = NextPosition(pOrign,TC_DIRECTION_RIGHTTOP);
				if(InMap(heroPos))
					return TC_DIRECTION_RIGHTTOP;
			}

			heroPos = NextPosition(pOrign,TC_DIRECTION_RIGHT);
			if(InMap(heroPos))
			{
				return TC_DIRECTION_RIGHT;
			}

		}
		if(pOrign.x-STEPLENGTH > dest.x){
			if(pOrign.y+STEPLENGTH < dest.y){
				heroPos = NextPosition(pOrign,TC_DIRECTION_LEFTBOTTOM);
				if(InMap(heroPos))
					return TC_DIRECTION_LEFTBOTTOM;
			}
			if(pOrign.y-STEPLENGTH >dest.y ){
				heroPos = NextPosition(pOrign,TC_DIRECTION_LEFTTOP);
				if(InMap(heroPos))
					return TC_DIRECTION_LEFTTOP;
			}
			heroPos = NextPosition(pOrign,TC_DIRECTION_LEFT);
			if(InMap(heroPos))
				return TC_DIRECTION_LEFT;
		}
	}


	//不带误差
		int difx = pOrign.x-dest.x;
		difx = difx>0?difx:-difx;
		int dify = pOrign.y-dest.y;
		dify = dify>0?dify:-dify;

		if(dify>STEPLENGTH){
			if(pOrign.y<dest.y)
				return TC_DIRECTION_BOTTOM;
			else
				return TC_DIRECTION_TOP;
			
		}
		else
		{
			if(pOrign.x>dest.x)
				return TC_DIRECTION_LEFT;
			else
				return TC_DIRECTION_RIGHT;
		}
}
