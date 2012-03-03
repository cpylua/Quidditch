/*
	@author:	zombie.fml

	@history:	
	*	10/5/2009
		create file

	*	10/7/2009
		add handle related functions

	*	10/9/2009
		add gate related functions

		change some APIs' return type

	*	10/15/2009
		add Tc_CanSpell, Tc_CanBeSpelled, Tc_IsSpelling

		add hero and ball size

	*	10/18/2009
		add Tc_GetGameInfo, Tc_GetForbiddenArea and related structures

		change Tc_CanBeSpelled's signature

	*	10/19/2009
		change TC_Gate

		add TC_Ball.u.visible_time_left

	*	10/20/2009
		delete Tc_CanSpell and Tc_IsSpelling, merge them into TC_Hero. See
		TC_Hero.b_is_spelling and TC_Hero.b_can_spell

		add TC_Hero.spell_interval

	*	10/22/2009
		change Tc_GetEnemyGate's signature.
		It seems most people don't read the	comments when coding. I have no 
		choice but to change its signature. 

	*	10/24/2009
		change Tc_ChooseHero's signature, must specify a hero name.

		delete TC_Ball.handle_owner.

		make API compatible with Visaul C++ 6.0. I guess there's still someone 
		using Visual C++ 6.0.

	*	10/26/2009
		add moving_time_left, steps_before_next_snatch

	*	11/1/2009
		add TC_HERO_SPELL_DISATNCE

		add map size

		add snatch distances

	*	11/3/2009
		add MANA_RECHARGE

	*	11/10/2009
		add harry and voldemort

	*	11/13/2009
		change TC_ForbiddenArea, now it's a rectangle


	@description:
		defines client data structures and APIs
		

	@Remarks:
		With clients loaded as DLLs, several C/C++ run-time libraries can be
		present in a single address space. And this may lead to crashes.
		There's	an easy fix for this problem. When a module allocates memory, the 
		module must also free the allocated memory.
		As a result, none of the APIs allocates memory for callers. When an API 
		has a parameter that points to some places, it is assumed that the 
		caller has allocated the memory. P L E A S E keep this in mind.

		Never step out your AI when debugging if you are not familiar with
		multi-thread debugging. 

*/
#include <Windows.h>
#include <tchar.h>

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

/*
	地图大小
*/
#define TC_MAP_WIDTH	2048
#define TC_MAP_HEIGHT	768

/*
	英雄攻击范围，
	两个英雄中心连线的长度小于这个值说明在攻击范围内
*/
#define TC_HERO_SPELL_DISTANCE	512

/*
	鬼飞球抢球范围，
	人和鬼飞球中心连线在这个范围内时【有可能】可以抢鬼飞球，
	能否抢鬼飞球由b_snatch_ghostball给出
*/
#define TC_SNATCH_DISTANCE_GHOST	64

/*
	飞贼抢球范围，
	人和飞贼中心连线在这个范围内时【有可能】可以抢飞贼，
	能否抢飞贼由b_snatch_goldball给出
*/
#define TC_SNATCH_DISTANCE_GOLD		64

/*
	游走球和人的中心距离小于这个值时会被游走球砸
*/
#define TC_COLLIDE_WITH_FREEBALL	64

/*
	最远的传球距离
*/
#define TC_PASSBALL_DISTANCE		512

/*
	英雄图像大小
*/
#define TC_HERO_WIDTH	128
#define TC_HERO_HEIGHT	128

/*
	球图像大小
*/
#define TC_BALL_WIDTH	64
#define TC_BALL_HEIGHT	64

/*
	飞贼大小
*/
#define TC_GOLDBALL_WIDTH	128
#define TC_GOLDBALL_HEIGHT	128

/*
	回魔速度
	每帧回1点
*/
#define MANA_RECHARGE	1

/* 
	队伍成员。
*/
enum TC_Team_Member
{
	TC_MEMBER1,
	TC_MEMBER2,
};

/* 
	不必关心这个。

	export modifier
*/
#ifdef TC_CLIENT_EXPORTS
#define TC_CLIENT_API __declspec(dllexport)
#else
#define TC_CLIENT_API __declspec(dllimport)
#endif

/* 
	句柄，NULL表示无效的句柄。

	所有的球和人物都用句柄来唯一标识，基本上所有接口函数都有一个或多
	个此类型的参数。可以把它认为是一个唯一的标识符。
*/
typedef void* TC_Handle;

/*
	表示速度的结构体，
	vx是水平速度（向右为正），vy是垂直速度（向下为正）。
*/
typedef struct TC_Speed_tag
{
	int vx;
	int vy;
} TC_Speed, *PTC_Speed;

/*
	表示坐标的结构体，
	有效的x、y值应该是非负整数。
*/
typedef struct TC_Position_tag
{
	int x;		/* <0 if invalid */
	int y;		/* <0 if invalid */
} TC_Position, *PTC_Position;

/*
	可能的人物类型，
*/
enum TC_Hero_Type
{
	TC_HERO_RON,
	TC_HERO_HERMIONE,
	TC_HERO_MALFOY,
	TC_HERO_HARRY,
	TC_HERO_VOLDEMORT,
	TC_HERO_GINNY,
};

/*
	可能的人物异常状态，
	这里异常状态主要是指被哪个类型的人物或者球攻击了。
*/
enum TC_Abnormal_Type
{
	/* normal state */
	TC_SPELLED_BY_NONE,
	TC_SPELLING,

	/* abmormal states */
	TC_SPELLED_BY_RON,
	TC_SPELLED_BY_HERMIONE,
	TC_SPELLED_BY_MALFOY,
	TC_SPELLED_BY_HARRY,
	TC_SPELLED_BY_VOLDEMORT,
	TC_SPELLED_BY_GINNY,
	TC_SPELLED_BY_FREEBALL,
};

/*
	表示英雄的结构体，
	英雄在屏幕上是以一个矩形表示的，TC_HERO_WIDTH和TC_HERO_HEIGHT分别定义了
	英雄的宽度和高度。

	type表示英雄类型，请参考TC_Hero_Type的注释；

	speed表示英雄当前的速度，英雄被赫敏施法后会减速一段时间；

	pos表示英雄的当前位置，这个坐标是英雄的左上角；

	max_blue表示英雄的最大魔法值；

	curr_blue表示英雄当前的魔法值；

	spell_cost表示英雄施放一次魔法要耗费多少魔法值，不能简单的根据
	spell_cost是否比curr_blue小来判断是否能放魔法，正确的做法是判断
	Tc_Hero.b_can_spell

	spell_interval表示英雄可以施法两次的最小时间间隔。

	b_can_spell表示英雄能不能施放魔法;

	b_is_spelling表示英雄是不是正在施放魔法;

	b_goldball如果是true就表示英雄抓住了金色飞贼，抓住金色飞贼意味着
	比赛结束；

	b_ghostball如果是true就表示英雄抓住了鬼飞球；

	b_snatch_ghostball如果是true就表示英雄能够去抢鬼飞球，注意球在人手里
	时抢球不一定成功，球不在任何人手里或者拿球的人被晕了则一定能抢到；

	b_snatch_goldball如果是true就表示英雄能够去抢金色飞贼。

	abnormal_type表示英雄当前的异常类型；

	steps_before_next_snatch这个域指明了再过多久英雄【有可能】能够抢球，
	因为能否抢球要考虑距离约束。判断当前能否抢球请使用b_snatch_ghostball和
	b_snatch_goldball。
	比赛中两次抢球之间有一个时间间隔，在时间间隔以内不能抢球。
*/
typedef struct TC_Hero_tag
{
	TC_Hero_Type	type;
	TC_Speed		speed;
	TC_Position		pos;
	int				max_blue;
	int				curr_blue;
	int				spell_cost;
	int				spell_interval;
	bool			b_can_spell;
	bool			b_is_spelling;
	bool			b_goldball;	
	bool			b_ghostball;
	bool			b_snatch_ghostball;
	bool			b_snatch_goldball;
	TC_Abnormal_Type	abnormal_type;
	unsigned 		steps_before_next_snatch;
}TC_Hero, *PTC_Hero;

/*
	球的类型，共三种球。

	游走球会在当前屏幕上随机选两个点为起点和终点来回做匀速直线运动，
	出现时刻不定，出现时间长度不定，被此球碰到的英雄会进入眩晕状态；

	鬼飞球可以被人抢来抢去，比赛选手持此球进入对方球门即可得分,
	拥有鬼飞球时人的移动速度会变慢；

	金色飞贼可以被抢到，而且抢到它比赛就结束，随机出现在当前屏幕任意
	位置，出现时间非常短。
*/
enum TC_Ball_Type
{
	TC_FREE_BALL,
	TC_GHOST_BALL,
	TC_GOLD_BALL,
};

/*
	表示一条路径
*/
typedef struct TC_Path_tag
{
	TC_Position	pos_start;
	TC_Position	pos_end;
}TC_Path, *PTC_Path;

/*
	表示球，

	type是球的类型，请参考TC_Ball_Type的注释；

	speed是球当前的速度，球不可见时无效；

	pos是球当前的位置（球的左上角坐标），球不可见时无效；

	b_visible如果是true就表示球是可见的；

	u.path只有当球是游走球时才有效，其他情况下不能使用这个域。
	表示游走球的运动路径。

	u.moving_time_left只有当球是游走球时才有效，其他情况下不能使用这个域。
	表示游走球剩余可见时间。

	u.target只有当球是鬼飞球时并且鬼飞球正在移动（速度不为0）才有效，其
	他情况下不应当使用这个域。
	表示传球的目的地坐标。

	u.slow_down_steps_left飞贼穿屏后的减速时间。

Microsoft Specific：
	Structure declarations can also be specified without a declarator when they 
	are members of another structure or union. The field names are promoted into
	the enclosing structure.
*/
#pragma warning(push)
#pragma warning(disable:201)
typedef struct TC_Ball_tag
{
	TC_Ball_Type		type;
	TC_Speed			speed;
	TC_Position			pos;
	bool				b_visible;
	union
	{
		/* free ball only */
		struct{
			TC_Path			path;		
			unsigned		moving_time_left;
		};

		/* ghost ball only */
		struct{
			TC_Position		target;	
		};

		/* gold ball only */
		struct{
			int				slow_down_steps_left;
		};
	} u;
}TC_Ball, *PTC_Ball;
#pragma warning(pop)

/*
	英雄移动的方向
*/
enum TC_Direction
{
	TC_DIRECTION_BOTTOM,
	TC_DIRECTION_LEFTBOTTOM,
	TC_DIRECTION_LEFT,
	TC_DIRECTION_LEFTTOP,
	TC_DIRECTION_TOP,
	TC_DIRECTION_RIGHTTOP,
	TC_DIRECTION_RIGHT,
	TC_DIRECTION_RIGHTBOTTOM,
};

/*
	球门位置.

	y_upper 和 y_lower是球门在树枝方向上的两个界限

 '直线X = x'
   /|\
    |
	
	|
	|
	|
+++++		--> '直线Y = y_upper'
|
|			人只能从Y = y_upper 和 Y = y_lower所夹的区域穿过球门线。
|
+++++		--> '直线Y = y_lower'
	|
	|
	|

	英雄带球越过球门线(X = x)就算得分。注意，整个英雄必须都在球门线以外
	才算得分
*/
typedef struct TC_Gate_tag
{
	int x;
	int y_upper;
	int y_lower;
} TC_Gate, *PTC_Gate;

/*
	定义一个矩形区域
*/
typedef struct TC_Rect_tag
{
	int top;
	int left;
	int bottom;
	int right;
} TC_Rect, *PTC_Rect;

/*
	传球时有效目的地必须保证：
	【整个球】都不在禁区内

	left是左边的禁区，right是右边的禁区。
*/
typedef struct TC_ForbiddenArea_tag
{
	TC_Rect left;		
	TC_Rect right;
} TC_ForbiddenArea, *PTC_ForbiddenArea;

/*
	游戏相关信息

	score_self己方得分；

	score_enemy对方得分；

	game_time_left游戏剩余时间，单位毫秒。
*/
typedef struct TC_GameInfo_tag
{
	unsigned score_self;
	unsigned score_enemy;

	unsigned game_time_left;	/* in ms */
} TC_GameInfo, *PTC_GameInfo;


/*

	需要说明的是，注释中可能会有一些错误，一切以函数的原型为准。
	所有参数的名字都是一个很好的注释。

*/

/*
	设置队伍名称，最长16个字符，包括结尾的'\0'。
	名字太长会被截断。
	队名是UINCIODE字符，e.g. L"Team name"。

	无返回值
*/
extern "C" TC_CLIENT_API void  __stdcall Tc_SetTeamName(IN PCWSTR pszTeamName );



/*
	选择英雄类型，

	p_index指定是队伍中的第几成员；
	p_Type指定英雄的类型。

	无返回值。

*/
extern "C" TC_CLIENT_API void  __stdcall Tc_ChooseHero(IN TC_Team_Member index, IN TC_Hero_Type Type, IN PCWSTR pszHeroName);



/*
	移动，

	handle_self是【自己】队伍中的某个成员的句柄，这个句柄通过
	Tc_GetHeroHandle取得。

	direction指定方向。

	如果成功，返回true；否则返回false。
*/
TC_CLIENT_API bool  __stdcall Tc_Move(IN TC_Handle handle_self, IN TC_Direction direction);




/*
	停止移动，原地不动。

	handle_self是【自己】队伍中的某个成员的句柄，这个句柄通过
	Tc_GetHeroHandle取得。

	如果成功，返回true；否则返回false。
*/
TC_CLIENT_API bool __stdcall Tc_Stop(IN TC_Handle handle_self);




/*
	施放魔法

	哈利只能自己队友放魔法，效果是两个人都有。

	如果成功，返回true；否则返回false。
	失败的原因可能是传入了无效的句柄、当前自己不能施放魔法，或者当前不
	能对对方施放魔法。
*/
TC_CLIENT_API bool  __stdcall Tc_Spell(IN TC_Handle handle_self, IN TC_Handle handle_target);


/*
	是否能够被施法

	handle_self是【自己】队伍中某个成员的句柄，这个成员是将要施法的人。
	这个句柄通过Tc_GetHeroHandle取得。

	handle_enemy是【对方】队伍中的某个成员的句柄，这个句柄通过
	Tc_GetEnemyHandle取得。

	如果能够被施法则返回true；否则返回false。
	如果目标正在放魔法、目标已经中了某个魔法而且魔法效果还没有失效、
	目标距离太远，则当前不能对其施法。

*/
TC_CLIENT_API bool __stdcall Tc_CanBeSpelled(IN TC_Handle handle_self, IN TC_Handle handle_enemy);




/*
	传球，只能传鬼飞球。

	handle_self是【自己】队伍中的某个成员的句柄，这个句柄通过
	Tc_GetHeroHandle取得。

	pos_target是传球的目标位置。

	如果成功把球【仍出去】则返回true；否则返回false。
	返回true时只表示把球扔了出去，至于传球能不能成功这是【不能确定】的。
	因为球被扔出去之后对手也可以在中途抢球，从而导致传球失败。

	注意传球位置【不能任意】，禁区内不能传球。
*/
TC_CLIENT_API bool  __stdcall Tc_PassBall(IN TC_Handle handle_self, IN TC_Position &pos_target);




/*
	抢球，鬼飞球和金色飞贼。

	handle_self是【自己】队伍中的某个成员的句柄，这个句柄通过
	Tc_GetHeroHandle取得。

	ball_type是球的类型，比赛中每种球只有一个。只能抢鬼飞球和金色飞贼。

	如果抢到球则返回true；否则返回false。
*/
TC_CLIENT_API bool  __stdcall Tc_SnatchBall(IN TC_Handle handle_self, IN TC_Ball_Type ball_type);



/*
	取得【自己】队伍中某个成员的信息。

	handle_self是【自己】队伍中的某个成员的句柄，这个句柄通过
	Tc_GetHeroHandle取得。

	p_hero_info是指向TC_Hero类型变量的一个指针。

	如果所有参数都有效则返回true；否则返回false。
	返回的信息在p_hero_info指向的变量中，这个变量的内存空间是调用者申请的。
*/
TC_CLIENT_API bool  __stdcall Tc_GetHeroInfo(IN TC_Handle handle_self, IN OUT PTC_Hero p_hero_info);




/*
	取得【对方】队伍中某个成员的信息。

	handle_enemy是【对方】队伍中的某个成员的句柄，这个句柄通过
	Tc_GetEnemyHandle取得。

	p_enemy_info是指向TC_Hero类型变量的一个指针。

	如果所有参数都有效则返回true；否则返回false。
	返回的信息在p_enemy_info指向的变量中，这个变量的内存空间是调用者申请的。
*/
TC_CLIENT_API bool  __stdcall Tc_GetEnemyInfo(IN TC_Handle handle_enemy, IN OUT PTC_Hero p_enemy_info);




/*
	取得球的信息。

	handle_ball是球的句柄，这个句柄通过Tc_GetBallHandle函数取得。

	p_ball_info是指向TC_Ball类型变量的一个指针。

	如果所有参数都有效则返回true；否则返回false。
	返回的信息在p_ball_info指向的变量中，这个变量的内存空间是调用者申请的。
*/
TC_CLIENT_API bool  __stdcall Tc_GetBallInfo(IN TC_Handle handle_ball, IN OUT PTC_Ball p_ball_info);




/*
	取得【自己】队伍中某个英雄的句柄。

	member指定是那个成员。

	返回对应成员的句柄。
*/
TC_CLIENT_API TC_Handle __stdcall Tc_GetHeroHandle(IN TC_Team_Member member);




/*
	取得【对方】队伍中某个英雄的句柄。

	member指定是那个成员。

	返回对应成员的句柄。
*/
TC_CLIENT_API TC_Handle __stdcall Tc_GetEnemyHandle(IN TC_Team_Member member);




/*
	取得某个球的句柄。

	ball_type指定是那个球，比赛中每种球只有一个。

	返回指定球的句柄。
*/
TC_CLIENT_API TC_Handle __stdcall Tc_GetBallHandle(IN TC_Ball_Type ball_type);




/*
	取得自己球门的位置。

	handle_self是【自己】队伍中的某个成员的句柄，这个句柄通过
	Tc_GetHeroHandle取得。

	p_own_gate是指向TC_Gate类型变量的一个指针。

	如果所有参数都有效则返回true；否则返回false。
	返回的信息在p_own_gate指向的变量中，这个变量的内存空间是调用者申请的。
*/
TC_CLIENT_API bool __stdcall Tc_GetOwnGate(IN TC_Handle handle_self, IN OUT PTC_Gate p_own_gate);




/*
	取得对方球门的位置。

	handle_enemy是【对方】队伍中的某个成员的句柄，这个句柄通过
	Tc_GetEnemyHandle取得。

	p_enemy_gate是指向TC_Gate类型变量的一个指针。

	如果所有参数都有效则返回true；否则返回false。
	返回的信息在p_enemy_gate指向的变量中，这个变量的内存空间是调用者申请。

*/
TC_CLIENT_API bool __stdcall Tc_GetEnemyGate(IN TC_Handle handle_enemy, IN OUT PTC_Gate p_enemy_gate);



/*
	取得比赛相关的信息

	handle_self是【自己】队伍中末各成员的句柄，这个句柄通过
	Tc_GetHeroHandle取得。

	p_game_info指向TC_GameInfo类型变量的一个指针。

	如果所有参数都有效则返回true；否则返回false。
	返回的信息在p_game_info指向的变量中，这个变量的内存空间是调用者申请的。
*/
TC_CLIENT_API bool __stdcall Tc_GetGameInfo(TC_Handle handle_self, PTC_GameInfo p_game_info);



/*
	取得传球禁区位置

	p_forbidden_area指向TC_ForbiddenArea类型变量的一个指针。

	如果所有参数都有效则返回true；否则返回false。
	返回的信息在p_forbidden_area指向的变量中，这个变量的内存空间是调用者
	申请的。
*/
TC_CLIENT_API bool __stdcall Tc_GetForbiddenArea(PTC_ForbiddenArea p_forbidden_area);

